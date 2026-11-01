
import struct
import zlib

# Protocol constants
SOF1 = 0xAA
SOF2 = 0x55

PROTOCOL_VERSION = 0x01

CMD_START_UPDATE = 0x01
CMD_ERASE_FLASH = 0x02
CMD_WRITE_DATA = 0x03
CMD_VERIFY = 0x04
CMD_JUMP_APP = 0x05
CMD_ACK = 0x06
CMD_NACK = 0x07

MAX_PAYLOAD = 256

def calculate_crc32(data: bytes) -> int:
    """Calculate standard CRC32."""
    return zlib.crc32(data) & 0xFFFFFFFF


def build_packet(command: int,
                 sequence: int,
                 payload: bytes = b"") -> bytes:
    """
    Build a bootloader packet.

    Format:
    SOF(2) + Header(6) + Payload + CRC32(4)

    CRC is calculated over header + payload.
    Multi-byte fields use big-endian format.
    """

    if not 0 <= command <= 0xFF:
        raise ValueError("Invalid command")

    if not 0 <= sequence <= 0xFFFF:
        raise ValueError("Invalid sequence")

    if len(payload) > MAX_PAYLOAD:
        raise ValueError("Payload exceeds maximum size")

    # Header: version, command, sequence, payload length
    header = struct.pack(
        ">BBHH",
        PROTOCOL_VERSION,
        command,
        sequence,
        len(payload)
    )

    # CRC covers header and payload, excluding SOF
    crc_data = header + payload
    crc = calculate_crc32(crc_data)

    # CRC is transmitted big-endian
    crc_bytes = struct.pack(">I", crc)

    return bytes([SOF1, SOF2]) + crc_data + crc_bytes
