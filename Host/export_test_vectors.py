
from firmware import build_update_packets

firmware = bytes([
    0x10, 0x11, 0x12, 0x13,
    0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1A, 0x1B,
    0x1C, 0x1D, 0x1E, 0x1F
])

packets = build_update_packets(firmware)

# Combine complete packets into one byte stream.
stream = b"".join(packets)

print("PYTHON-GENERATED BOOTLOADER TEST STREAM")
print("Total packets:", len(packets))
print("Total bytes:", len(stream))
print()

for index, packet in enumerate(packets):
    print(f"Packet {index + 1}:")
    print(" ".join(f"{byte:02X}" for byte in packet))

print("\nC BYTE ARRAY:")
print("static const unsigned char host_stream[] =")
print("{")

for index, byte in enumerate(stream):
    if index % 8 == 0:
        print("    ", end="")

    print(f"0x{byte:02X}U,", end=" ")

    if index % 8 == 7:
        print()

print("\n};")
print(f"#define HOST_STREAM_SIZE {len(stream)}U")
