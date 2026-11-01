
import argparse
from pathlib import Path

from firmware import load_firmware, build_update_packets
from serial_link import BootloaderSerial

PORT = "/dev/cu.usbserial-0001"
BAUDRATE = 115200

COMMAND_NAMES = {
    0x01: "START_UPDATE",
    0x03: "WRITE_DATA",
    0x04: "VERIFY",
    0x05: "JUMP_APP",
}


def upload_firmware(port, firmware_path):

    firmware = load_firmware(firmware_path)
    packets = build_update_packets(firmware)

    bootloader = None

    try:
        bootloader = BootloaderSerial(
            port=port,
            baudrate=BAUDRATE,
            timeout=2.0
        )

        print("\nUART BOOTLOADER FIRMWARE UPDATE")
        print("--------------------------------")
        print(f"Port: {port}")
        print(f"Baud rate: {BAUDRATE}")
        print(f"Firmware: {firmware_path}")
        print(f"Firmware size: {len(firmware)} bytes")
        print(f"Total packets: {len(packets)}")
        print("--------------------------------")

        for index, packet in enumerate(packets):

            command = packet[3]

            sequence = int.from_bytes(
                packet[4:6], "big"
            )

            command_name = COMMAND_NAMES.get(
                command,
                f"UNKNOWN_0x{command:02X}"
            )

            print(
                f"\n[{index + 1}/{len(packets)}] "
                f"{command_name} | Sequence {sequence}"
            )

            response = bootloader.send_and_wait_ack(packet)

            print(
                f"ACK received | Sequence "
                f"{response['sequence']}"
            )

        print("\n--------------------------------")
        print("FIRMWARE UPDATE COMPLETED")
        print("All packets acknowledged.")
        print("--------------------------------")

    except Exception as error:
        print("\nFIRMWARE UPDATE FAILED")
        print(f"Reason: {error}")
        raise

    finally:
        if bootloader is not None:
            bootloader.close()


if __name__ == "__main__":

    parser = argparse.ArgumentParser(
        description="LPC2129 UART Bootloader Firmware Uploader"
    )

    parser.add_argument(
        "--port",
        default=PORT,
        help="UART serial port"
    )

    parser.add_argument(
        "--firmware",
        default="firmware.bin",
        help="Firmware binary file"
    )

    args = parser.parse_args()

    upload_firmware(
        args.port,
        Path(args.firmware)
    )
