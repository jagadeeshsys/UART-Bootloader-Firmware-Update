
#include "protocol.h"
#include "uart.h"
#include "crc32.h"

#define PACKET_DATA_SIZE \
    (PROTOCOL_HEADER_SIZE + PROTOCOL_MAX_PAYLOAD)

static uint8_t ReceiveByte(void)
{
    return (uint8_t)UART0_RxChar();
}

static void SendByte(uint8_t data)
{
    UART0_TxChar((char)data);
}

static uint16_t ReadBigEndian16(uint8_t high, uint8_t low)
{
    return (uint16_t)(((uint16_t)high << 8) | low);
}

static void WriteBigEndian16(uint8_t *buffer, uint16_t value)
{
    buffer[0] = (uint8_t)(value >> 8);
    buffer[1] = (uint8_t)value;
}

static void WriteBigEndian32(uint8_t *buffer, uint32_t value)
{
    buffer[0] = (uint8_t)(value >> 24);
    buffer[1] = (uint8_t)(value >> 16);
    buffer[2] = (uint8_t)(value >> 8);
    buffer[3] = (uint8_t)value;
}

void Protocol_Init(void)
{
    /* UART0 must be initialized before protocol use. */
}

/* Receive and validate one packet */

int Protocol_ReceivePacket(Protocol_Packet *packet)
{
    uint8_t byte;
    uint8_t previous;
    uint8_t crc_data[PACKET_DATA_SIZE];
    uint8_t crc_bytes[4];

    uint16_t i;
    uint32_t received_crc;
    uint32_t calculated_crc;

    if (packet == 0)
        return PROTOCOL_ERROR;

    /* Search for start-of-frame AA 55 */

    previous = 0U;

    while (1)
    {
        byte = ReceiveByte();

        if ((previous == PROTOCOL_SOF1) &&
            (byte == PROTOCOL_SOF2))
        {
            break;
        }

        previous = byte;
    }

    /* Receive version, command, sequence and length */

    for (i = 0; i < PROTOCOL_HEADER_SIZE; i++)
    {
        crc_data[i] = ReceiveByte();
    }

    packet->version = crc_data[0];
    packet->command = crc_data[1];

    packet->sequence =
        ReadBigEndian16(crc_data[2], crc_data[3]);

    packet->length =
        ReadBigEndian16(crc_data[4], crc_data[5]);

    /* Check payload length */

    if (packet->length > PROTOCOL_MAX_PAYLOAD)
    {
        return PROTOCOL_ERROR;
    }

    /* Receive payload */

    for (i = 0; i < packet->length; i++)
    {
        packet->payload[i] = ReceiveByte();
        crc_data[PROTOCOL_HEADER_SIZE + i] =
            packet->payload[i];
    }

    /* Receive transmitted CRC32 */

    for (i = 0; i < PROTOCOL_CRC_SIZE; i++)
    {
        crc_bytes[i] = ReceiveByte();
    }

    received_crc =
        ((uint32_t)crc_bytes[0] << 24) |
        ((uint32_t)crc_bytes[1] << 16) |
        ((uint32_t)crc_bytes[2] << 8) |
        (uint32_t)crc_bytes[3];

    packet->crc = received_crc;

    /* Validate protocol version */

    if (packet->version != PROTOCOL_VERSION)
    {
        return PROTOCOL_ERROR;
    }

    /* Calculate CRC over header and payload */

    calculated_crc =
        CRC32_Calculate(
            crc_data,
            PROTOCOL_HEADER_SIZE + packet->length
        );

    if (calculated_crc != received_crc)
    {
        return PROTOCOL_ERROR;
    }

    return PROTOCOL_OK;
}

/* Transmit one packet */

void Protocol_SendPacket(
    uint8_t command,
    uint16_t sequence,
    const uint8_t *payload,
    uint16_t length)
{
    uint8_t crc_data[PACKET_DATA_SIZE];
    uint32_t crc;
    uint16_t i;

    if (length > PROTOCOL_MAX_PAYLOAD)
        return;

    if ((length != 0U) && (payload == 0))
        return;

    crc_data[0] = PROTOCOL_VERSION;
    crc_data[1] = command;

    WriteBigEndian16(&crc_data[2], sequence);
    WriteBigEndian16(&crc_data[4], length);

    for (i = 0; i < length; i++)
    {
        crc_data[PROTOCOL_HEADER_SIZE + i] = payload[i];
    }

    crc = CRC32_Calculate(
        crc_data,
        PROTOCOL_HEADER_SIZE + length
    );

    /* Send start-of-frame */

    SendByte(PROTOCOL_SOF1);
    SendByte(PROTOCOL_SOF2);

    /* Send header and payload */

    for (i = 0;
         i < (PROTOCOL_HEADER_SIZE + length);
         i++)
    {
        SendByte(crc_data[i]);
    }

    /* Send CRC32 in big-endian format */

    {
        uint8_t crc_bytes[4];

        WriteBigEndian32(crc_bytes, crc);

        for (i = 0; i < 4; i++)
        {
            SendByte(crc_bytes[i]);
        }
    }
}

/* Parse and validate a complete packet from memory */

int Protocol_ParseBuffer(
    const uint8_t *buffer,
    uint32_t buffer_length,
    Protocol_Packet *packet)
{
    uint16_t payload_length;
    uint32_t expected_length;
    uint32_t received_crc;
    uint32_t calculated_crc;
    uint32_t i;

    if ((buffer == 0) || (packet == 0))
        return PROTOCOL_ERROR;

    /* Minimum packet size:
       SOF(2) + Header(6) + CRC(4) */

    if (buffer_length < 12U)
        return PROTOCOL_ERROR;

    /* Check start of frame */

    if ((buffer[0] != PROTOCOL_SOF1) ||
        (buffer[1] != PROTOCOL_SOF2))
    {
        return PROTOCOL_ERROR;
    }

    /* Read header */

    packet->version = buffer[2];
    packet->command = buffer[3];

    packet->sequence =
        ((uint16_t)buffer[4] << 8) | buffer[5];

    payload_length =
        ((uint16_t)buffer[6] << 8) | buffer[7];

    packet->length = payload_length;

    /* Check protocol version */

    if (packet->version != PROTOCOL_VERSION)
        return PROTOCOL_ERROR;

    /* Check maximum payload */

    if (payload_length > PROTOCOL_MAX_PAYLOAD)
        return PROTOCOL_ERROR;

    /* Check exact packet size */

    expected_length =
        2U + PROTOCOL_HEADER_SIZE +
        payload_length + PROTOCOL_CRC_SIZE;

    if (buffer_length != expected_length)
        return PROTOCOL_ERROR;

    /* Copy payload */

    for (i = 0; i < payload_length; i++)
    {
        packet->payload[i] = buffer[8U + i];
    }

    /* Read received CRC32 */

    i = 8U + payload_length;

    received_crc =
        ((uint32_t)buffer[i] << 24) |
        ((uint32_t)buffer[i + 1U] << 16) |
        ((uint32_t)buffer[i + 2U] << 8) |
        ((uint32_t)buffer[i + 3U]);

    packet->crc = received_crc;

    /* Calculate CRC over version through payload */

    calculated_crc =
        CRC32_Calculate(
            &buffer[2],
            PROTOCOL_HEADER_SIZE + payload_length
        );

    /* Compare CRC values */

    if (calculated_crc != received_crc)
        return PROTOCOL_ERROR;

    return PROTOCOL_OK;
}

/* ==========================================
   LIVE UART PACKET RECEIVER
   ========================================== */

#define RX_WAIT_SOF1       0
#define RX_WAIT_SOF2       1
#define RX_READ_HEADER     2
#define RX_READ_PAYLOAD    3
#define RX_READ_CRC        4

static uint8_t rx_state = RX_WAIT_SOF1;

static uint8_t rx_header_index = 0;
static uint16_t rx_payload_index = 0;
static uint8_t rx_crc_index = 0;

static uint8_t rx_buffer[
    PROTOCOL_HEADER_SIZE + PROTOCOL_MAX_PAYLOAD
];

static uint16_t rx_payload_length;
static uint32_t rx_received_crc;

/* Packet receive timeout configuration */
#define PROTOCOL_RX_TIMEOUT_MS    100U

static volatile uint32_t rx_elapsed_ms = 0U;


/* Called once every 1 ms by the hardware timer */
void Protocol_RxTick1ms(void)
{
    if (rx_state != RX_WAIT_SOF1)
    {
        if (rx_elapsed_ms < PROTOCOL_RX_TIMEOUT_MS)
        {
            rx_elapsed_ms++;
        }
    }
    else
    {
        rx_elapsed_ms = 0U;
    }
}

/* Reset receiver state */

void Protocol_RxReset(void)
{
    rx_state = RX_WAIT_SOF1;

    rx_header_index = 0;
    rx_payload_index = 0;
    rx_crc_index = 0;
	
		rx_elapsed_ms = 0U;

    rx_payload_length = 0;
    rx_received_crc = 0;
}

/* Process one incoming UART byte */

int Protocol_RxProcessByte(
    uint8_t byte,
    Protocol_Packet *packet)
{
    uint32_t calculated_crc;
    uint16_t i;
	
		/* Check for incomplete packet timeout */
		if ((rx_state != RX_WAIT_SOF1) &&
    (rx_elapsed_ms >= PROTOCOL_RX_TIMEOUT_MS))
		{
    Protocol_RxReset();
		}

    if (packet == 0)
        return RX_PACKET_ERROR;
	
			/* Reset inter-byte timeout for the new byte */
			rx_elapsed_ms = 0U;
				
    switch (rx_state)
    {
        case RX_WAIT_SOF1:

            if (byte == PROTOCOL_SOF1)
            {
                rx_state = RX_WAIT_SOF2;
            }

            break;

        case RX_WAIT_SOF2:

            if (byte == PROTOCOL_SOF2)
            {
                rx_state = RX_READ_HEADER;
                rx_header_index = 0;
            }
            else if (byte == PROTOCOL_SOF1)
            {
                rx_state = RX_WAIT_SOF2;
            }
            else
            {
                rx_state = RX_WAIT_SOF1;
            }

            break;

        case RX_READ_HEADER:

            rx_buffer[rx_header_index] = byte;
            rx_header_index++;

            if (rx_header_index >= PROTOCOL_HEADER_SIZE)
            {
                /* Extract payload length */

                rx_payload_length =
                    ((uint16_t)rx_buffer[4] << 8) |
                    rx_buffer[5];

                /* Validate version */

                if (rx_buffer[0] != PROTOCOL_VERSION)
                {
                    Protocol_RxReset();
                    return RX_PACKET_ERROR;
                }

                /* Validate payload length */

                if (rx_payload_length > PROTOCOL_MAX_PAYLOAD)
                {
                    Protocol_RxReset();
                    return RX_PACKET_ERROR;
                }

                rx_payload_index = 0;

                if (rx_payload_length == 0U)
                {
                    rx_state = RX_READ_CRC;
                }
                else
                {
                    rx_state = RX_READ_PAYLOAD;
                }
            }

            break;

        case RX_READ_PAYLOAD:

            rx_buffer[
                PROTOCOL_HEADER_SIZE + rx_payload_index
            ] = byte;

            rx_payload_index++;

            if (rx_payload_index >= rx_payload_length)
            {
                rx_state = RX_READ_CRC;
                rx_crc_index = 0;
                rx_received_crc = 0;
            }

            break;

        case RX_READ_CRC:

            rx_received_crc =
                (rx_received_crc << 8) | byte;

            rx_crc_index++;

            if (rx_crc_index >= PROTOCOL_CRC_SIZE)
            {
                /* Calculate CRC */

                calculated_crc =
                    CRC32_Calculate(
                        rx_buffer,
                        PROTOCOL_HEADER_SIZE +
                        rx_payload_length
                    );

                if (calculated_crc != rx_received_crc)
                {
                    Protocol_RxReset();
                    return RX_PACKET_ERROR;
                }

                /* Populate packet structure */

                packet->version = rx_buffer[0];
                packet->command = rx_buffer[1];

                packet->sequence =
                    ((uint16_t)rx_buffer[2] << 8) |
                    rx_buffer[3];

                packet->length = rx_payload_length;

                packet->crc = rx_received_crc;

                for (i = 0; i < rx_payload_length; i++)
                {
                    packet->payload[i] =
                        rx_buffer[
                            PROTOCOL_HEADER_SIZE + i
                        ];
                }

                Protocol_RxReset();

                return RX_PACKET_OK;
            }

            break;

        default:

            Protocol_RxReset();
            return RX_PACKET_ERROR;
    }

    return RX_PENDING;
}
