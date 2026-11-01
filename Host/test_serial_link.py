
from serial_link import BootloaderSerial
from protocol import CMD_ACK, CMD_NACK, build_packet


class FakeSerial:
    """Simulates serial input for offline testing."""

    def __init__(self, data):
        self.data = bytearray(data)

    def read(self, size=1):
        result = self.data[:size]
        del self.data[:size]
        return bytes(result)


def create_test_receiver(packet):
    receiver = BootloaderSerial.__new__(BootloaderSerial)
    receiver.serial = FakeSerial(packet)
    return receiver


# TEST 1: Valid ACK
print("TEST 1: Valid ACK")

ack_packet = build_packet(CMD_ACK, 7, b"")
receiver = create_test_receiver(ack_packet)

response = receiver.receive_response()

assert response["status"] == "ACK"
assert response["sequence"] == 7

print("PASS")


# TEST 2: Valid NACK
print("\nTEST 2: Valid NACK")

nack_packet = build_packet(CMD_NACK, 8, b"")
receiver = create_test_receiver(nack_packet)

response = receiver.receive_response()

assert response["status"] == "NACK"
assert response["sequence"] == 8

print("PASS")


# TEST 3: Corrupted CRC
print("\nTEST 3: Corrupted CRC")

corrupted_packet = bytearray(
    build_packet(CMD_ACK, 9, b"")
)

corrupted_packet[-1] ^= 0x01

receiver = create_test_receiver(corrupted_packet)

try:
    receiver.receive_response()
    raise AssertionError("Corrupted CRC was accepted")

except ValueError as error:
    assert str(error) == "Response CRC mismatch"
    print("PASS")


print("\nALL ACK/NACK TESTS PASSED")
