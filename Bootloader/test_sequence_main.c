
#include <stdint.h>

#include "bootloader.h"
#include "flash_mock.h"

/* Valid Python-generated START_UPDATE packet */
static const uint8_t start_packet[20] =
{
    0xAAU, 0x55U,
    0x01U, 0x01U,
    0x00U, 0x00U,
    0x00U, 0x08U,
    0x00U, 0x00U, 0x00U, 0x10U,
    0xF4U, 0xA7U, 0xFDU, 0x67U,
    0xEDU, 0xEDU, 0xD5U, 0xB0U
};

/* WRITE_DATA packet with incorrect sequence = 5.
   Correct expected sequence = 1.
   CRC is valid for this incorrect sequence packet.
*/
static const uint8_t wrong_sequence_packet[32] =
{
    0xAAU, 0x55U,
    0x01U, 0x03U,
    0x00U, 0x05U,
    0x00U, 0x14U,

    0x00U, 0x00U, 0x00U, 0x00U,

    0x10U, 0x11U, 0x12U, 0x13U,
    0x14U, 0x15U, 0x16U, 0x17U,
    0x18U, 0x19U, 0x1AU, 0x1BU,
    0x1CU, 0x1DU, 0x1EU, 0x1FU,

    0xA7U, 0x32U, 0x45U, 0xD4U
};

/* Debugger result variables */
volatile uint8_t start_state = 0xFFU;
volatile uint8_t bad_sequence_state = 0xFFU;
volatile uint8_t flash_read_result = 0xFFU;
volatile uint8_t flash_unchanged = 0U;
volatile uint8_t sequence_test_complete = 0U;

static void InjectPacket(
    const uint8_t *packet,
    uint32_t length)
{
    uint32_t i;

    for (i = 0U; i < length; i++)
    {
        Bootloader_TestInjectByte(packet[i]);
    }
}

int main(void)
{
    uint8_t readback[16];
    uint32_t i;

    Bootloader_Init();

    /* TEST 1: Send valid START_UPDATE */
    InjectPacket(start_packet, 20U);

    start_state = Bootloader_GetState();

    /* TEST 2: Send WRITE_DATA with sequence 5 */
    InjectPacket(wrong_sequence_packet, 32U);

    bad_sequence_state = Bootloader_GetState();

    /* TEST 3: Check that flash remains erased */
    flash_read_result = (uint8_t)FlashMock_Read(
        0U,
        readback,
        16U
    );

    flash_unchanged = 1U;

    for (i = 0U; i < 16U; i++)
    {
        if (readback[i] != 0xFFU)
        {
            flash_unchanged = 0U;
        }
    }

    sequence_test_complete = 1U;

    while (1)
    {
    }
}
