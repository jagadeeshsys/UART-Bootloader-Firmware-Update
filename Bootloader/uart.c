
#include "uart.h"

/* LPC2129 Register Addresses */

#define PINSEL0     (*(volatile uint32_t *)0xE002C000)

#define U0RBR       (*(volatile uint32_t *)0xE000C000)
#define U0THR       (*(volatile uint32_t *)0xE000C000)
#define U0DLL       (*(volatile uint32_t *)0xE000C000)

#define U0DLM       (*(volatile uint32_t *)0xE000C004)
#define U0IER       (*(volatile uint32_t *)0xE000C004)

#define U0FCR       (*(volatile uint32_t *)0xE000C008)
#define U0LCR       (*(volatile uint32_t *)0xE000C00C)
#define U0LSR       (*(volatile uint32_t *)0xE000C014)

/* UART Line Status Bits */

#define LSR_RDR     (1U << 0)
#define LSR_THRE    (1U << 5)

/* UART0 Initialization */

void UART0_Init(uint32_t pclk, uint32_t baudrate)
{
    uint32_t divisor;

    if (baudrate == 0U)
        return;

    /*
     * P0.0 = TXD0
     * P0.1 = RXD0
     */

    PINSEL0 = (PINSEL0 & ~0x0FU) | 0x05U;

    /*
     * Calculate baud divisor.
     * Rounded to nearest integer.
     */

    divisor = (pclk + (8U * baudrate)) /
              (16U * baudrate);

    if (divisor == 0U || divisor > 65535U)
        return;

    /* Disable UART interrupts */

    U0IER = 0x00U;

    /* Enable Divisor Latch Access */

    U0LCR = 0x83U;

    /* Configure baud rate divisor */

    U0DLL = divisor & 0xFFU;
    U0DLM = (divisor >> 8) & 0xFFU;

    /*
     * 8-bit data
     * No parity
     * 1 stop bit
     * Disable DLAB
     */

    U0LCR = 0x03U;

    /* Enable and reset FIFO */

    U0FCR = 0x07U;
}

/* Transmit one character */

void UART0_TxChar(char data)
{
    while ((U0LSR & LSR_THRE) == 0U)
    {
        /* Wait until transmit holding register is empty */
    }

    U0THR = (uint8_t)data;
}

/* Receive one character */

char UART0_RxChar(void)
{
    while ((U0LSR & LSR_RDR) == 0U)
    {
        /* Wait until data is received */
    }

    return (char)(U0RBR & 0xFFU);
}

/* Transmit a string */

void UART0_TxString(const char *str)
{
    while (*str != '\0')
    {
        UART0_TxChar(*str);
        str++;
    }
}

/* Transmit hexadecimal value */

void UART0_TxHex(uint32_t value)
{
    const char hex[] = "0123456789ABCDEF";
    int i;

    for (i = 7; i >= 0; i--)
    {
        UART0_TxChar(hex[(value >> (i * 4)) & 0x0FU]);
    }
}

/* Check whether UART0 has received data */

uint8_t UART0_RxAvailable(void)
{
    return (uint8_t)(U0LSR & LSR_RDR);
}

/* Receive one character without blocking */

uint8_t UART0_RxCharNonBlocking(char *data)
{
    if (data == 0)
    {
        return 0U;
    }

    if (UART0_RxAvailable())
    {
        *data = (char)(U0RBR & 0xFFU);
        return 1U;
    }

    return 0U;
}
