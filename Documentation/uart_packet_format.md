
# UART Packet Format

## Overview

The UART bootloader uses a custom packet-based communication protocol between the Python host application and the LPC2129 bootloader.

## Packet Structure

```text
+----------------------+----------------+
| Field                | Size           |
+----------------------+----------------+
| Start of Frame       | 2 Bytes        |
| Protocol Version     | 1 Byte         |
| Command              | 1 Byte         |
| Sequence Number      | 2 Bytes        |
| Payload Length       | 2 Bytes        |
| Payload              | 0-256 Bytes    |
| CRC32                | 4 Bytes        |
+----------------------+----------------+
```

## Field Description

| Field | Description |
|---|---|
| SOF | Identifies the beginning of a packet (0xAA 0x55) |
| Version | Protocol version (0x01) |
| Command | Identifies the requested operation |
| Sequence | Packet sequence number |
| Length | Payload size in bytes |
| Payload | Command-specific data |
| CRC32 | Detects packet corruption |

## Supported Commands

| Command | Value | Purpose |
|---|---|---|
| START_UPDATE | 0x01 | Starts firmware update |
| ERASE_FLASH | 0x02 | Defined command; not independently implemented |
| WRITE_DATA | 0x03 | Transfers firmware data |
| VERIFY | 0x04 | Verifies firmware image |
| JUMP_APP | 0x05 | Requests simulated application handover |
| ACK | 0x06 | Acknowledgement |
| NACK | 0x07 | Negative acknowledgement |

## Byte Ordering

Multi-byte protocol header fields and payload offsets use Big Endian byte ordering.

CRC32 uses the IEEE CRC32 algorithm with reflected polynomial 0xEDB88320.

## CRC32 Validation

The receiver calculates CRC32 over:

- Protocol Version
- Command
- Sequence Number
- Payload Length
- Payload

The calculated CRC32 is compared with the CRC32 value received in the packet.

If the values match, the packet passes CRC validation. Otherwise, the packet is rejected.
