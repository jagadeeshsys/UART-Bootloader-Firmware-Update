
#include "bootloader.h"
#include "timer0.h"
#include "uart.h"
#include "protocol.h"
#include "crc32.h"
#include "flash_mock.h"

#define BL_PCLK_HZ             15000000U
#define BL_BAUD_RATE           115200U

#define BL_METADATA_SIZE      8U
#define BL_WRITE_OFFSET_SIZE  4U

#define BL_CRC_POLYNOMIAL     0xEDB88320UL

static uint8_t bl_state = BL_STATE_IDLE;

static uint32_t firmware_size = 0U;
static uint32_t expected_crc = 0U;
static uint32_t received_size = 0U;

static uint32_t running_crc = 0xFFFFFFFFUL;

static uint16_t expected_sequence = 1U;

/* Cache the last successfully written packet */
static uint16_t last_write_sequence;
static uint16_t last_write_length;
static uint32_t last_write_crc;
static uint8_t last_write_valid;

static Protocol_Packet rx_packet;



/* Read 32-bit big-endian value */

static uint32_t ReadBE32(const uint8_t *data)
{
    return ((uint32_t)data[0] << 24) |
           ((uint32_t)data[1] << 16) |
           ((uint32_t)data[2] << 8)  |
           (uint32_t)data[3];
}

/* Update running CRC32 */

static uint32_t CRC32_UpdateByte(
    uint32_t crc,
    uint8_t data)
{
    uint32_t i;

    crc ^= data;

    for (i = 0U; i < 8U; i++)
    {
        if (crc & 1UL)
        {
            crc = (crc >> 1) ^ BL_CRC_POLYNOMIAL;
        }
        else
        {
            crc >>= 1;
        }
    }

    return crc;
}

/* Send ACK */

static void SendACK(uint16_t sequence)
{
    Protocol_SendPacket(
        CMD_ACK,
        sequence,
        0,
        0U
    );
}

/* Send NACK */

static void SendNACK(uint16_t sequence)
{
    Protocol_SendPacket(
        CMD_NACK,
        sequence,
        0,
        0U
    );
}

/* Erase simulated application flash */

static int Bootloader_EraseFlash(void)
{
    uint32_t sector;

    for (sector = 0U;
         sector < MOCK_SECTOR_COUNT;
         sector++)
    {
        if (FlashMock_EraseSector(sector) != FLASH_OK)
        {
            return BL_ERROR;
        }
    }

    return BL_OK;
}

/* Process START_UPDATE command */

static void HandleStartUpdate(Protocol_Packet *packet)
{
    uint32_t sector;

    if (packet->length != BL_METADATA_SIZE)
    {
        SendNACK(packet->sequence);
        return;
    }

    if (bl_state == BL_STATE_RECEIVING)
    {
        SendNACK(packet->sequence);
        return;
    }

    firmware_size = ReadBE32(&packet->payload[0]);
    expected_crc = ReadBE32(&packet->payload[4]);

    if ((firmware_size == 0U) ||
        (firmware_size > MOCK_FLASH_SIZE))
    {
        SendNACK(packet->sequence);
        return;
    }

    if (Bootloader_EraseFlash() != BL_OK)
    {
        bl_state = BL_STATE_ERROR;
        SendNACK(packet->sequence);
        return;
    }

    received_size = 0U;
    running_crc = 0xFFFFFFFFUL;
    expected_sequence = 1U;
    
    last_write_sequence = 0U;
    last_write_length = 0U;
    last_write_crc = 0U;
    last_write_valid = 0U;


    bl_state = BL_STATE_RECEIVING;

    /* Acknowledge successful initialization */

    SendACK(packet->sequence);

    /* sector variable intentionally retained
       for easier debugger inspection */
    sector = MOCK_SECTOR_COUNT;
    (void)sector;
}

/* Process WRITE_DATA command */


static void HandleWriteData(Protocol_Packet *packet)
{
    uint32_t offset;
    uint16_t data_length;
    uint16_t i;

    if (bl_state != BL_STATE_RECEIVING)
    {
        SendNACK(packet->sequence);
        return;
    }

    /* Detect retransmission of the last accepted packet */
    if ((last_write_valid != 0U) &&
        (packet->sequence == last_write_sequence))
    {
        if ((packet->length == last_write_length) &&
            (packet->crc == last_write_crc))
        {
            SendACK(packet->sequence);
        }
        else
        {
            SendNACK(packet->sequence);
        }

        return;
    }

    if (packet->sequence != expected_sequence)
    {
        SendNACK(packet->sequence);
        return;
    }

    if (packet->length <= BL_WRITE_OFFSET_SIZE)
    {
        SendNACK(packet->sequence);
        return;
    }

    offset = ReadBE32(&packet->payload[0]);

    data_length =
        packet->length - BL_WRITE_OFFSET_SIZE;

    if (offset != received_size)
    {
        SendNACK(packet->sequence);
        return;
    }

    /* Prevent subtraction underflow */
    if (received_size > firmware_size)
    {
        SendNACK(packet->sequence);
        return;
    }

    if ((data_length > firmware_size - received_size) ||
        (offset > MOCK_FLASH_SIZE) ||
        (data_length > MOCK_FLASH_SIZE - offset))
    {
        SendNACK(packet->sequence);
        return;
    }

    /* Write data into simulated flash */
    if (FlashMock_Write(
            offset,
            &packet->payload[BL_WRITE_OFFSET_SIZE],
            data_length) != FLASH_OK)
    {
        bl_state = BL_STATE_ERROR;
        SendNACK(packet->sequence);
        return;
    }

    /* Update firmware CRC */
    for (i = 0U; i < data_length; i++)
    {
        running_crc = CRC32_UpdateByte(
            running_crc,
            packet->payload[BL_WRITE_OFFSET_SIZE + i]
        );
    }

    received_size += data_length;

    /* Save last accepted packet information */
    last_write_sequence = packet->sequence;
    last_write_length = packet->length;
    last_write_crc = packet->crc;
    last_write_valid = 1U;

    expected_sequence++;

    SendACK(packet->sequence);
}

/* Process VERIFY command */

static void HandleVerify(Protocol_Packet *packet)
{
    uint32_t final_crc;

    if (bl_state != BL_STATE_RECEIVING)
    {
        SendNACK(packet->sequence);
        return;
    }

    if (packet->length != 0U)
    {
        SendNACK(packet->sequence);
        return;
    }

    if (received_size != firmware_size)
    {
        SendNACK(packet->sequence);
        return;
    }

    final_crc = running_crc ^ 0xFFFFFFFFUL;

    if (final_crc != expected_crc)
    {
        bl_state = BL_STATE_ERROR;
        SendNACK(packet->sequence);
        return;
    }

    bl_state = BL_STATE_COMPLETE;

    SendACK(packet->sequence);
}

/* Process application handover command */

static void HandleJumpApp(Protocol_Packet *packet)
{
    if ((bl_state != BL_STATE_COMPLETE) ||
        (packet->length != 0U))
    {
        SendNACK(packet->sequence);
        return;
    }

    SendACK(packet->sequence);

    /*
     * Simulation only:
     * Actual LPC2129 application handover
     * will be implemented separately.
     */

    UART0_TxString(
        "\r\nSIMULATED APPLICATION HANDOVER\r\n"
    );
}

/* Process a validated packet */

static void Bootloader_HandlePacket(Protocol_Packet *packet)
{
    switch (packet->command)
    {
        case CMD_START_UPDATE:
            HandleStartUpdate(packet);
            break;

        case CMD_WRITE_DATA:
            HandleWriteData(packet);
            break;

        case CMD_VERIFY:
            HandleVerify(packet);
            break;

        case CMD_JUMP_APP:
            HandleJumpApp(packet);
            break;

        default:
            SendNACK(packet->sequence);
            break;
    }
}

/* Initialize bootloader */

void Bootloader_Init(void)
{
    bl_state = BL_STATE_IDLE;

    firmware_size = 0U;
    expected_crc = 0U;
    received_size = 0U;

    running_crc = 0xFFFFFFFFUL;

    expected_sequence = 1U;

    last_write_sequence = 0U;
    last_write_length = 0U;
    last_write_crc = 0U;
    last_write_valid = 0U;

    Protocol_RxReset();

    FlashMock_Init();

    UART0_Init(BL_PCLK_HZ, BL_BAUD_RATE);
		Timer0_Init();

    UART0_TxString(
        "\r\nLPC2129 UART BOOTLOADER\r\n"
    );

    UART0_TxString(
        "Waiting for firmware update...\r\n"
    );
}

/* Main bootloader processing loop */

void Bootloader_Process(void)
{
    char received_byte;
    int status;

		Timer0_Poll();
	
    if (UART0_RxCharNonBlocking(&received_byte))
    {
        status = Protocol_RxProcessByte(
            (uint8_t)received_byte,
            &rx_packet
        );

        if (status == RX_PACKET_OK)
        {
            Bootloader_HandlePacket(&rx_packet);
        }
        else if (status == RX_PACKET_ERROR)
        {
            SendNACK(0U);
        }
    }
}

/* Return current bootloader state */

uint8_t Bootloader_GetState(void)
{
    return bl_state;
}
/* Inject one simulated UART byte for testing */
void Bootloader_TestInjectByte(uint8_t byte)
{
    int status;

    status = Protocol_RxProcessByte(byte, &rx_packet);

    if (status == RX_PACKET_OK)
    {
        Bootloader_HandlePacket(&rx_packet);
    }
    else if (status == RX_PACKET_ERROR)
    {
        SendNACK(0U);
    }
}
