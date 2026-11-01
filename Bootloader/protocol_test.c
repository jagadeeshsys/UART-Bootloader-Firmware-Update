
#include <stdint.h>

#include "uart.h"
#include "protocol.h"

#define PCLK_HZ     15000000U
#define BAUD_RATE   115200U

int main(void)
{
    Protocol_Packet packet;

    uint16_t i;

    /* Valid packet containing HELLO */

    uint8_t valid_packet[] =
    {
        0xAA, 0x55,       /* Start of frame */

        0x01,             /* Protocol version */
        0x03,             /* WRITE_DATA command */

        0x00, 0x01,       /* Sequence number */

        0x00, 0x05,       /* Payload length */

        0x48, 0x45,       /* H E */
        0x4C, 0x4C,       /* L L */
        0x4F,             /* O */

        0xDA, 0x3F,       /* CRC32 */
        0xD7, 0x29
    };

    /* Corrupted packet: HELLO changed to JELLO */

    uint8_t corrupted_packet[] =
    {
        0xAA, 0x55,

        0x01,
        0x03,

        0x00, 0x01,

        0x00, 0x05,

        0x4A, 0x45,       /* J E */
        0x4C, 0x4C,       /* L L */
        0x4F,             /* O */

        0xDA, 0x3F,       /* Original CRC */
        0xD7, 0x29
    };

    /* Initialize UART0 */

    UART0_Init(PCLK_HZ, BAUD_RATE);

    UART0_TxString("\r\n");
    UART0_TxString("==============================\r\n");
    UART0_TxString("LPC2129 PACKET VALIDATION TEST\r\n");
    UART0_TxString("==============================\r\n");

    /* TEST 1: Valid packet */

    UART0_TxString("\r\nTEST 1: VALID PACKET\r\n");

    if (Protocol_ParseBuffer(
            valid_packet,
            sizeof(valid_packet),
            &packet) == PROTOCOL_OK)
    {
        UART0_TxString("PASS: Valid packet accepted\r\n");

        UART0_TxString("Command: ");
        UART0_TxHex(packet.command);
        UART0_TxString("\r\n");

        UART0_TxString("Sequence: ");
        UART0_TxHex(packet.sequence);
        UART0_TxString("\r\n");

        UART0_TxString("Payload: ");

        for (i = 0; i < packet.length; i++)
        {
            UART0_TxChar((char)packet.payload[i]);
        }

        UART0_TxString("\r\n");
    }
    else
    {
        UART0_TxString("FAIL: Valid packet rejected\r\n");
    }

    /* TEST 2: Corrupted packet */

    UART0_TxString("\r\nTEST 2: CORRUPTED PACKET\r\n");

    if (Protocol_ParseBuffer(
            corrupted_packet,
            sizeof(corrupted_packet),
            &packet) == PROTOCOL_ERROR)
    {
        UART0_TxString("PASS: Corrupted packet rejected\r\n");
    }
    else
    {
        UART0_TxString("FAIL: Corrupted packet accepted\r\n");
    }

    /* TEST 3: Incorrect start marker */

    UART0_TxString("\r\nTEST 3: INVALID START MARKER\r\n");

    valid_packet[0] = 0xBB;

    if (Protocol_ParseBuffer(
            valid_packet,
            sizeof(valid_packet),
            &packet) == PROTOCOL_ERROR)
    {
        UART0_TxString("PASS: Invalid SOF rejected\r\n");
    }
    else
    {
        UART0_TxString("FAIL: Invalid SOF accepted\r\n");
    }

    /* TEST 4: Restore packet and test invalid version */

    valid_packet[0] = 0xAA;
    valid_packet[2] = 0x02;

    if (Protocol_ParseBuffer(
            valid_packet,
            sizeof(valid_packet),
            &packet) == PROTOCOL_ERROR)
    {
        UART0_TxString("PASS: Invalid version rejected\r\n");
    }
    else
    {
        UART0_TxString("FAIL: Invalid version accepted\r\n");
    }

    UART0_TxString("\r\n==============================\r\n");
    UART0_TxString("PACKET TESTING COMPLETED\r\n");
    UART0_TxString("==============================\r\n");

    while (1)
    {
    }
}
