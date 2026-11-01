#ifndef UART_H
#define UART_H

#include<stdint.h>

void UART0_Init(uint32_t pclk, uint32_t baudrate);
void UART0_TxChar(char data);
char UART0_RxChar(void);
void UART0_TxString(const char *str);
void UART0_TxHex(uint32_t value);
uint8_t UART0_RxAvailable(void);
uint8_t UART0_RxCharNonBlocking(char *data);

#endif
