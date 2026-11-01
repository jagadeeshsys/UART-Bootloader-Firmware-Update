
import tempfile
from pathlib import Path
import struct

from firmware import (
    load_firmware,
    build_update_packets,
    MAX_CHUNK_SIZE
)

from protocol import (
    CMD_START_UPDATE,
    CMD_WRITE_DATA,
    CMD_VERIFY,
    CMD_JUMP_APP,
)


# Create a temporary firmware file for testing
test_firmware = bytes(range(256)) * 4

with tempfile.TemporaryDirectory() as temp_dir:

    firmware_path = Path(temp_dir) / "firmware.bin"

    firmware_path.write_bytes(test_firmware)

    # TEST 1: Load firmware
    firmware = load_firmware(firmware_path)

    assert firmware == test_firmware

    print("TEST 1: Firmware loading - PASS")

    # TEST 2: Packet generation
    packets = build_update_packets(firmware)

    print("TEST 2: Packet generation")

    assert packets[0][3] == CMD_START_UPDATE
    assert packets[-2][3] == CMD_VERIFY
    assert packets[-1][3] == CMD_JUMP_APP

    print("PASS")

    # TEST 3: WRITE_DATA chunking
    write_packets = packets[1:-2]

    expected_chunks = (
        len(firmware) + MAX_CHUNK_SIZE - 1
    ) // MAX_CHUNK_SIZE

    assert len(write_packets) == expected_chunks

    print("TEST 3: Packet chunking - PASS")

    # TEST 4: Offset validation
    expected_offset = 0

    for packet in write_packets:

        payload_length = struct.unpack(
            ">H",
            packet[6:8]
        )[0]

        payload = packet[8:8 + payload_length]

        offset = struct.unpack(
            ">I",
            payload[:4]
        )[0]

        assert offset == expected_offset

        expected_offset += len(payload) - 4

    assert expected_offset == len(firmware)

    print("TEST 4: Firmware offset validation - PASS")

    print("\nALL FIRMWARE TESTS PASSED")
