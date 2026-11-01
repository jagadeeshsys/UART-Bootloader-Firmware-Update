#include <stdint.h>
#include "timer0.h"
#include "protocol.h"


/* LPC2129 Timer0 registers */
#define T0IR       (*(volatile uint32_t *)0xE0004000)
#define T0TCR      (*(volatile uint32_t *)0xE0004004)
#define T0PR       (*(volatile uint32_t *)0xE000400C)
#define T0MCR      (*(volatile uint32_t *)0xE0004014)
#define T0MR0      (*(volatile uint32_t *)0xE0004018)

/* Initialize Timer0 for a 1 ms match period */
void Timer0_Init(void)
{
    /* Stop Timer0 */
    T0TCR = 0x02U;

    /* PCLK = 15 MHz
       Prescaler = 14
       Timer counter frequency = 1 MHz */
    T0PR = 14U;

    /* 1000 timer counts = 1 ms */
    T0MR0 = 999U;

    /* Interrupt flag + reset on MR0 match */
    T0MCR = 0x03U;

    /* Clear pending match flag */
    T0IR = 0x01U;

    /* Enable Timer0 */
    T0TCR = 0x01U;
}

/* Poll Timer0 match flag */
void Timer0_Poll(void)
{
    if ((T0IR & 0x01U) != 0U)
    {
        /* Clear match flag */
        T0IR = 0x01U;

        /* Notify protocol receiver: 1 ms elapsed */
        Protocol_RxTick1ms();

    }
}
