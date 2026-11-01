
#include <stdint.h>
#include "uart.h"

#define PCLK_HZ    15000000U
#define BAUD_RATE  115200U

int main(void)
{
    char received;

    UART0_Init(PCLK_HZ, BAUD_RATE);

    UART0_TxString("\r\nUART NON-BLOCKING RX TEST\r\n");
    UART0_TxString("Enter a character: ");

    while (1)
    {
        if (UART0_RxCharNonBlocking(&received))
        {
            UART0_TxString("\r\nReceived: ");
            UART0_TxChar(received);
            UART0_TxString("\r\nEnter a character: ");
        }
    }
}
