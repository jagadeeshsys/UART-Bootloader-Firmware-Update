
#include <stdint.h>

#include "uart.h"
#include "protocol.h"

#define PCLK_HZ       15000000U
#define BAUD_RATE     115200U

int main(void)
{
    char received_byte;

    int status;

    Protocol_Packet packet;

    UART0_Init(PCLK_HZ, BAUD_RATE);

    Protocol_RxReset();

    UART0_TxString("\r\n");
    UART0_TxString("==============================\r\n");
    UART0_TxString("LPC2129 LIVE PACKET RECEIVER\r\n");
    UART0_TxString("==============================\r\n");

    UART0_TxString("Waiting for firmware packet...\r\n");

    while (1)
    {
        /* Check UART without blocking */

        if (UART0_RxCharNonBlocking(&received_byte))
        {
            status = Protocol_RxProcessByte(
                (uint8_t)received_byte,
                &packet
            );

            if (status == RX_PACKET_OK)
            {
                UART0_TxString("\r\nPACKET RECEIVED\r\n");

                UART0_TxString("Command: ");
                UART0_TxHex(packet.command);
                UART0_TxString("\r\n");

                UART0_TxString("Sequence: ");
                UART0_TxHex(packet.sequence);
                UART0_TxString("\r\n");

                UART0_TxString("Payload: ");

                {
                    uint16_t i;

                    for (i = 0; i < packet.length; i++)
                    {
                        UART0_TxChar(
                            (char)packet.payload[i]
                        );
                    }
                }

                UART0_TxString("\r\nCRC: VALID\r\n");

                /* Send ACK */

                Protocol_SendPacket(
                    CMD_ACK,
                    packet.sequence,
                    0,
                    0
                );

                UART0_TxString("ACK SENT\r\n");
            }
            else if (status == RX_PACKET_ERROR)
            {
                UART0_TxString("\r\nINVALID PACKET\r\n");

                /* Send NACK with sequence zero */

                Protocol_SendPacket(
                    CMD_NACK,
                    0,
                    0,
                    0
                );

                UART0_TxString("NACK SENT\r\n");
            }
        }
    }
}
