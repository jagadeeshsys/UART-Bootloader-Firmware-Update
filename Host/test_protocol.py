
import struct

from protocol import (
    calculate_crc32,
    build_packet,
    CMD_START_UPDATE,
    PROTOCOL_VERSION
)


# Test 1: Standard CRC32 reference test
test_data = b"123456789"

crc = calculate_crc32(test_data)

print("TEST 1: CRC32")
print(f"Calculated CRC: {crc:08X}")

assert crc == 0xCBF43926
print("PASS\n")


# Test 2: Build START_UPDATE packet
payload = bytes([
    0x00, 0x00, 0x00, 0x10,
    0xCB, 0xF4, 0x39, 0x26
])

packet = build_packet(
    CMD_START_UPDATE,
    0,
    payload
)

print("TEST 2: START_UPDATE packet")
print("Packet:", packet.hex(" ").upper())

assert packet[0:2] == bytes([0xAA, 0x55])
assert packet[2] == PROTOCOL_VERSION
assert packet[3] == CMD_START_UPDATE

sequence, length = struct.unpack(">HH", packet[4:8])

assert sequence == 0
assert length == 8

print("PASS\n")


# Test 3: Verify packet CRC
received_crc = struct.unpack(">I", packet[-4:])[0]
calculated_crc = calculate_crc32(packet[2:-4])

assert received_crc == calculated_crc

print("TEST 3: Packet CRC validation")
print(f"Packet CRC: {received_crc:08X}")
print("PASS\n")


print("ALL PROTOCOL TESTS PASSED")
