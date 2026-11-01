
import time
import serial

from protocol import (
    SOF1,
    SOF2,
    PROTOCOL_VERSION,
    CMD_WRITE_DATA,
    CMD_ACK,
    CMD_NACK,
    calculate_crc32,
)

MAX_PAYLOAD = 256
MAX_WRITE_RETRIES = 3


class BootloaderSerial:

    def __init__(self, port, baudrate=115200, timeout=2.0):

        self.serial = serial.Serial(
            port=port,
            baudrate=baudrate,
            bytesize=serial.EIGHTBITS,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            timeout=timeout,
            write_timeout=timeout
        )

        time.sleep(0.2)

    def close(self):
        if self.serial.is_open:
            self.serial.close()

    def send_packet(self, packet):
        written = self.serial.write(packet)

        if written != len(packet):
            raise IOError("Incomplete UART packet transmission")

        self.serial.flush()

    def _read_exact(self, length):
        data = self.serial.read(length)

        if len(data) != length:
            raise TimeoutError(
                f"Expected {length} bytes, received {len(data)}"
            )

        return data

    def _find_start_of_frame(self):
        """Scan UART input until AA 55 is received."""

        previous = None

        while True:
            current = self.serial.read(1)

            if not current:
                raise TimeoutError("Response start-of-frame timeout")

            value = current[0]

            if previous == SOF1 and value == SOF2:
                return

            previous = value if value == SOF1 else None

    def receive_response(self):
        """Read and validate one complete ACK/NACK packet."""

        self._find_start_of_frame()

        header = self._read_exact(6)

        version = header[0]
        command = header[1]

        sequence = int.from_bytes(header[2:4], "big")
        length = int.from_bytes(header[4:6], "big")

        if version != PROTOCOL_VERSION:
            raise ValueError("Invalid response protocol version")

        if length > MAX_PAYLOAD:
            raise ValueError("Response payload exceeds maximum size")

        payload = self._read_exact(length)
        crc_bytes = self._read_exact(4)

        received_crc = int.from_bytes(crc_bytes, "big")

        calculated_crc = calculate_crc32(header + payload)

        if received_crc != calculated_crc:
            raise ValueError("Response CRC mismatch")

        if command == CMD_ACK:
            status = "ACK"
        elif command == CMD_NACK:
            status = "NACK"
        else:
            raise ValueError(
                f"Unexpected response command: 0x{command:02X}"
            )

        return {
            "status": status,
            "sequence": sequence,
            "payload": payload
        }

    def send_and_wait_ack(self, packet):
        """Send a packet and wait for its matching ACK."""

        if len(packet) < 12:
            raise ValueError("Invalid packet length")

        if packet[0:2] != bytes([SOF1, SOF2]):
            raise ValueError("Invalid packet start-of-frame")

        if packet[2] != PROTOCOL_VERSION:
            raise ValueError("Invalid packet protocol version")

        command = packet[3]

        expected_sequence = int.from_bytes(
            packet[4:6], "big"
        )

        retries = MAX_WRITE_RETRIES if command == CMD_WRITE_DATA else 0

        for attempt in range(retries + 1):

            self.send_packet(packet)

            try:
                response = self.receive_response()

            except TimeoutError:
                if attempt < retries:
                    print(
                        f"ACK timeout for WRITE_DATA sequence "
                        f"{expected_sequence}; retry "
                        f"{attempt + 1}/{retries}"
                    )
                    continue

                raise TimeoutError(
                    f"No ACK received for sequence "
                    f"{expected_sequence} after {retries + 1} attempts"
                )

            if response["sequence"] != expected_sequence:
                raise ValueError(
                    f"Sequence mismatch: expected "
                    f"{expected_sequence}, received "
                    f"{response['sequence']}"
                )

            if response["status"] == "NACK":
                raise RuntimeError(
                    f"Bootloader rejected command 0x{command:02X}, "
                    f"sequence {expected_sequence}"
                )

            return response

        raise RuntimeError("Packet transmission failed")
