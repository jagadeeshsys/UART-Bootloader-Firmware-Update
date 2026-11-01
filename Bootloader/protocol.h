
#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>

#define PROTOCOL_SOF1          0xAAU
#define PROTOCOL_SOF2          0x55U

#define PROTOCOL_VERSION       0x01U

#define PROTOCOL_MAX_PAYLOAD  256U
#define PROTOCOL_HEADER_SIZE   6U
#define PROTOCOL_CRC_SIZE      4U

/* Bootloader Commands */

#define CMD_START_UPDATE       0x01U
#define CMD_ERASE_FLASH        0x02U
#define CMD_WRITE_DATA         0x03U
#define CMD_VERIFY             0x04U
#define CMD_JUMP_APP           0x05U

#define CMD_ACK                0x06U
#define CMD_NACK               0x07U

typedef struct
{
    uint8_t version;
    uint8_t command;

    uint16_t sequence;
    uint16_t length;

    uint8_t payload[PROTOCOL_MAX_PAYLOAD];

    uint32_t crc;

} Protocol_Packet;

/* Return status */

#define PROTOCOL_OK            1
#define PROTOCOL_ERROR         0

void Protocol_Init(void);

int Protocol_ReceivePacket(Protocol_Packet *packet);

void Protocol_SendPacket(
    uint8_t command,
    uint16_t sequence,
    const uint8_t *payload,
    uint16_t length
);

int Protocol_ParseBuffer(
    const uint8_t *buffer,
    uint32_t buffer_length,
    Protocol_Packet *packet
);


#define RX_PENDING       0
#define RX_PACKET_OK     1
#define RX_PACKET_ERROR -1

void Protocol_RxReset(void);

int Protocol_RxProcessByte(
    uint8_t byte,
    Protocol_Packet *packet
);

void Protocol_RxTick1ms(void);

#endif
