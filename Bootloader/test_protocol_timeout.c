
#include <stdint.h>
#include "protocol.h"
#include "crc32.h"

/* Test result variables: inspect these in Keil Watch window */
volatile uint32_t timeout_recovery_test_complete = 0U;
volatile uint32_t packet_received = 0U;
volatile uint32_t received_command = 0U;
volatile uint32_t received_sequence = 0U;

int main(void)
{
    uint8_t frame[12];
    uint32_t crc;
    uint32_t i;
    int status;
    Protocol_Packet received_packet;

    /* Reset receiver */
    Protocol_RxReset();

    /* Send only the first SOF byte */
    Protocol_RxProcessByte(PROTOCOL_SOF1, &received_packet);

    /* Simulate 100 milliseconds passing */
    for (i = 0U; i < 100U; i++)
    {
        Protocol_RxTick1ms();
    }

    /*
       Send the second SOF byte after timeout.
       The incomplete packet should already be discarded.
    */
    Protocol_RxProcessByte(PROTOCOL_SOF2, &received_packet);

    /* Construct a valid zero-payload packet */
    frame[0] = PROTOCOL_SOF1;
    frame[1] = PROTOCOL_SOF2;
    frame[2] = PROTOCOL_VERSION;
    frame[3] = CMD_START_UPDATE;
    frame[4] = 0x00U;
    frame[5] = 0x01U;
    frame[6] = 0x00U;
    frame[7] = 0x00U;

    /* Calculate CRC over header */
    crc = CRC32_Calculate(&frame[2], 6U);

    frame[8]  = (uint8_t)(crc >> 24);
    frame[9]  = (uint8_t)(crc >> 16);
    frame[10] = (uint8_t)(crc >> 8);
    frame[11] = (uint8_t)crc;

    /* Feed the complete valid packet */
    for (i = 0U; i < 12U; i++)
    {
        status = Protocol_RxProcessByte(
            frame[i],
            &received_packet
        );
    }

    /* Record test result */
    if (status == RX_PACKET_OK)
    {
        packet_received = 1U;
        received_command = received_packet.command;
        received_sequence = received_packet.sequence;
        timeout_recovery_test_complete = 1U;
    }

    while (1)
    {
        /* Hold results for debugging */
    }
}
