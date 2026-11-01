
#include <stdint.h>

#include "bootloader.h"
#include "protocol.h"
#include "crc32.h"
#include "flash_mock.h"

#define FIRMWARE_SIZE 32U

static const uint8_t firmware[FIRMWARE_SIZE] =
{
    0x10U, 0x11U, 0x12U, 0x13U,
    0x14U, 0x15U, 0x16U, 0x17U,
    0x18U, 0x19U, 0x1AU, 0x1BU,
    0x1CU, 0x1DU, 0x1EU, 0x1FU,

    0x20U, 0x21U, 0x22U, 0x23U,
    0x24U, 0x25U, 0x26U, 0x27U,
    0x28U, 0x29U, 0x2AU, 0x2BU,
    0x2CU, 0x2DU, 0x2EU, 0x2FU
};

/* Debugger result variables */
volatile uint8_t start_state = 0xFFU;
volatile uint8_t first_write_state = 0xFFU;
volatile uint8_t retry_state = 0xFFU;
volatile uint8_t next_write_state = 0xFFU;
volatile uint8_t verify_state = 0xFFU;

volatile uint8_t flash_read_result = 0xFFU;
volatile uint8_t flash_match = 0U;
volatile uint8_t retransmission_test_complete = 0U;

volatile uint16_t expected_sequence_result = 0U;

/* Inject one complete protocol packet */
static void InjectPacket(
    uint8_t command,
    uint16_t sequence,
    const uint8_t *payload,
    uint16_t length)
{
    uint8_t packet_data[PROTOCOL_HEADER_SIZE +
                        PROTOCOL_MAX_PAYLOAD];

    uint8_t crc_bytes[4];

    uint32_t crc;
    uint32_t i;

    if (length > PROTOCOL_MAX_PAYLOAD)
    {
        return;
    }

    packet_data[0] = PROTOCOL_VERSION;
    packet_data[1] = command;

    packet_data[2] = (uint8_t)(sequence >> 8);
    packet_data[3] = (uint8_t)sequence;

    packet_data[4] = (uint8_t)(length >> 8);
    packet_data[5] = (uint8_t)length;

    for (i = 0U; i < length; i++)
    {
        packet_data[PROTOCOL_HEADER_SIZE + i] = payload[i];
    }

    crc = CRC32_Calculate(
        packet_data,
        PROTOCOL_HEADER_SIZE + length
    );

    crc_bytes[0] = (uint8_t)(crc >> 24);
    crc_bytes[1] = (uint8_t)(crc >> 16);
    crc_bytes[2] = (uint8_t)(crc >> 8);
    crc_bytes[3] = (uint8_t)crc;

    Bootloader_TestInjectByte(PROTOCOL_SOF1);
    Bootloader_TestInjectByte(PROTOCOL_SOF2);

    for (i = 0U; i < PROTOCOL_HEADER_SIZE + length; i++)
    {
        Bootloader_TestInjectByte(packet_data[i]);
    }

    for (i = 0U; i < 4U; i++)
    {
        Bootloader_TestInjectByte(crc_bytes[i]);
    }
}

int main(void)
{
    uint8_t start_payload[8];
    uint8_t first_payload[20];
    uint8_t second_payload[20];
    uint8_t readback[FIRMWARE_SIZE];

    uint32_t firmware_crc;
    uint32_t i;

    Bootloader_Init();

    /* TEST 1: START_UPDATE */

    firmware_crc = CRC32_Calculate(firmware, FIRMWARE_SIZE);

    start_payload[0] = 0x00U;
    start_payload[1] = 0x00U;
    start_payload[2] = 0x00U;
    start_payload[3] = FIRMWARE_SIZE;

    start_payload[4] = (uint8_t)(firmware_crc >> 24);
    start_payload[5] = (uint8_t)(firmware_crc >> 16);
    start_payload[6] = (uint8_t)(firmware_crc >> 8);
    start_payload[7] = (uint8_t)firmware_crc;

    InjectPacket(CMD_START_UPDATE, 0U, start_payload, 8U);

    start_state = Bootloader_GetState();

    /* TEST 2: First WRITE_DATA packet, sequence 1 */

    first_payload[0] = 0U;
    first_payload[1] = 0U;
    first_payload[2] = 0U;
    first_payload[3] = 0U;

    for (i = 0U; i < 16U; i++)
    {
        first_payload[4U + i] = firmware[i];
    }

    InjectPacket(CMD_WRITE_DATA, 1U, first_payload, 20U);

    first_write_state = Bootloader_GetState();

    /* TEST 3: Simulate lost ACK by retransmitting sequence 1 */

    InjectPacket(CMD_WRITE_DATA, 1U, first_payload, 20U);

    retry_state = Bootloader_GetState();

    /* TEST 4: Send next packet using sequence 2, offset 16 */

    second_payload[0] = 0U;
    second_payload[1] = 0U;
    second_payload[2] = 0U;
    second_payload[3] = 16U;

    for (i = 0U; i < 16U; i++)
    {
        second_payload[4U + i] = firmware[16U + i];
    }

    InjectPacket(CMD_WRITE_DATA, 2U, second_payload, 20U);

    next_write_state = Bootloader_GetState();

    /* TEST 5: Verify complete firmware */

    InjectPacket(CMD_VERIFY, 3U, 0, 0U);

    verify_state = Bootloader_GetState();

    /* TEST 6: Read back simulated flash */

    flash_read_result = (uint8_t)FlashMock_Read(
        0U,
        readback,
        FIRMWARE_SIZE
    );

    flash_match = 1U;

    for (i = 0U; i < FIRMWARE_SIZE; i++)
    {
        if (readback[i] != firmware[i])
        {
            flash_match = 0U;
        }
    }

    expected_sequence_result = 4U;

    if ((start_state == BL_STATE_RECEIVING) &&
        (first_write_state == BL_STATE_RECEIVING) &&
        (retry_state == BL_STATE_RECEIVING) &&
        (next_write_state == BL_STATE_RECEIVING) &&
        (verify_state == BL_STATE_COMPLETE) &&
        (flash_read_result == FLASH_OK) &&
        (flash_match == 1U))
    {
        retransmission_test_complete = 1U;
    }

    while (1)
    {
    }
}
