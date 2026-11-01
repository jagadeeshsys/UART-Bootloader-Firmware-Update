# LPC2129 UART Bootloader for Firmware Update

A UART-based firmware update bootloader developed using Embedded C for the LPC2129 ARM7 microcontroller. The project demonstrates serial communication, packet-based firmware transfer, CRC32 validation, sequence control, error handling, and firmware verification.

> **Development status:** Implemented and tested using Keil µVision simulation and Python host-side tests. Flash operations currently use a RAM-based mock implementation. Physical LPC2129 hardware testing has not yet been performed.

## 1. Project Objectives

- Develop a UART-based firmware update mechanism.
- Implement a custom packet communication protocol.
- Validate firmware integrity using CRC32.
- Handle packet sequencing and duplicate transmissions.
- Detect corrupted packets and invalid commands.
- Develop a Python-based host utility for firmware transfer.

## 2. Technologies Used

| Category | Technology |
|---|---|
| Microcontroller | LPC2129 |
| Architecture | ARM7TDMI-S |
| Programming language | Embedded C |
| IDE | Keil µVision4 |
| Communication | UART0 |
| Host programming | Python |
| Integrity checking | CRC32 |
| Simulation | Keil µVision |
| Version control | Git and GitHub |

## 3. System Architecture

The project consists of two main components:

**Embedded Bootloader**
- UART driver
- Packet protocol parser
- CRC32 validation
- Bootloader state management
- RAM-based flash simulation
- Timer0-based receive timeout tracking

**Host Application**
- Firmware loading
- Packet generation
- Firmware chunking
- Serial communication
- ACK/NACK handling
- Firmware upload utility

## 4. UART Communication Protocol

The custom protocol uses the following packet structure:

| Field | Size |
|---|---:|
| Start of Frame | 2 bytes |
| Version | 1 byte |
| Command | 1 byte |
| Sequence Number | 2 bytes |
| Payload Length | 2 bytes |
| Payload | Variable |
| CRC32 | 4 bytes |

**Start of Frame:** `0xAA 0x55`

**Maximum payload:** 256 bytes

**CRC:** IEEE CRC32 using polynomial `0xEDB88320`.

### Supported Commands

| Command | Description |
|---|---|
| `0x01` | START_UPDATE |
| `0x02` | ERASE_FLASH (defined, not independently implemented) |
| `0x03` | WRITE_DATA |
| `0x04` | VERIFY |
| `0x05` | JUMP_APP |
| `0x06` | ACK |
| `0x07` | NACK |

## 5. Firmware Update Sequence

1. Host loads the firmware binary.
2. Host calculates firmware CRC32.
3. Host sends START_UPDATE.
4. Bootloader prepares the simulated flash memory.
5. Host transfers firmware in data packets.
6. Bootloader checks packet sequence and CRC.
7. Bootloader writes received data into simulated flash.
8. Host sends VERIFY.
9. Bootloader verifies the complete firmware image.
10. Host sends JUMP_APP.
11. Bootloader displays a simulated application handover message.

## 6. Error Handling

The implementation includes:

- Packet CRC validation.
- Invalid packet rejection.
- Sequence number validation.
- Duplicate WRITE_DATA packet detection.
- Modified duplicate packet rejection.
- Firmware size validation.
- Flash boundary checking.
- Illegal flash bit-transition checking.
- Receive timeout recovery.
- ACK/NACK responses.

## 7. Testing and Validation

The following tests have been performed:

- UART transmission and reception tests.
- CRC32 validation tests.
- Protocol parser tests.
- Valid and corrupted packet tests.
- Flash mock read/write/erase tests.
- Invalid flash operation tests.
- Firmware transfer integration tests.
- Duplicate packet and retransmission tests.
- Protocol timeout recovery tests.
- Python host-side protocol and firmware tests.

The embedded protocol and Python host implementation were tested together using generated packet streams and direct test-byte injection.

## 8. Project Limitations

- No physical LPC2129 board was available for testing.
- Flash operations currently use a RAM-based mock implementation.
- Actual LPC2129 flash programming is not implemented.
- Application handover is simulated; no real application jump is performed.
- UART communication has not yet been validated with physical hardware.

## 9. Future Improvements

- Implement actual LPC2129 flash programming.
- Test firmware transfer using a physical development board.
- Implement a real application handover mechanism.
- Improve protocol resynchronization and timeout handling.
- Validate UART communication using a USB-to-TTL adapter.
- Add hardware-level firmware update testing.

## 10. Repository Structure

```text
UART-Bootloader-Firmware-Update/
│
├── Bootloader/
│   ├── bootloader.c
│   ├── bootloader.h
│   ├── main.c
│   ├── uart.c
│   ├── uart.h
│   ├── protocol.c
│   ├── protocol.h
│   ├── crc32.c
│   ├── crc32.h
│   ├── flash_mock.c
│   ├── flash_mock.h
│   ├── timer0.c
│   └── timer0.h
│
├── Host/
│   ├── protocol.py
│   ├── firmware.py
│   ├── serial_link.py
│   ├── upload_firmware.py
│   └── test files
│
├── Application/
├── Documentation/
├── Tests/
└── README.md
```

## 11. Author

**Jagadeesh**

GitHub: https://github.com/jagadeeshsys

Project Repository: https://github.com/jagadeeshsys/UART-Bootloader-Firmware-Update
