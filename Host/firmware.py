
from pathlib import Path
import struct

from protocol import (
    build_packet,
    calculate_crc32,
    CMD_START_UPDATE,
    CMD_WRITE_DATA,
    CMD_VERIFY,
    CMD_JUMP_APP,
)

MAX_PAYLOAD = 256
WRITE_OFFSET_SIZE = 4
MAX_CHUNK_SIZE = MAX_PAYLOAD - WRITE_OFFSET_SIZE

MOCK_FLASH_SIZE = 4096


def load_firmware(file_path):
    """Load and validate a firmware binary."""

    path = Path(file_path)

    if not path.is_file():
        raise FileNotFoundError(
            f"Firmware file not found: {path}"
        )

    firmware = path.read_bytes()

    if not firmware:
        raise ValueError("Firmware file is empty")

    if len(firmware) > MOCK_FLASH_SIZE:
        raise ValueError(
            f"Firmware size {len(firmware)} exceeds "
            f"mock flash capacity of {MOCK_FLASH_SIZE} bytes"
        )

    return firmware


def build_update_packets(firmware):
    """Build START, WRITE, VERIFY and JUMP packets."""

    if not isinstance(firmware, (bytes, bytearray)):
        raise TypeError("Firmware must be bytes or bytearray")

    firmware = bytes(firmware)

    if not firmware:
        raise ValueError("Firmware cannot be empty")

    firmware_size = len(firmware)

    if firmware_size > MOCK_FLASH_SIZE:
        raise ValueError("Firmware exceeds mock flash capacity")

    packets = []

    firmware_crc = calculate_crc32(firmware)

    # 1. START_UPDATE
    start_payload = struct.pack(
        ">II",
        firmware_size,
        firmware_crc
    )

    sequence = 0

    packets.append(
        build_packet(
            CMD_START_UPDATE,
            sequence,
            start_payload
        )
    )

    # 2. WRITE_DATA
    sequence = 1
    offset = 0

    while offset < firmware_size:

        chunk = firmware[
            offset:offset + MAX_CHUNK_SIZE
        ]

        write_payload = struct.pack(
            ">I",
            offset
        ) + chunk

        packets.append(
            build_packet(
                CMD_WRITE_DATA,
                sequence,
                write_payload
            )
        )

        offset += len(chunk)
        sequence += 1

    # 3. VERIFY
    packets.append(
        build_packet(
            CMD_VERIFY,
            sequence,
            b""
        )
    )

    sequence += 1

    # 4. JUMP_APP
    packets.append(
        build_packet(
            CMD_JUMP_APP,
            sequence,
            b""
        )
    )

    return packets
