#include <msp430g2553.h>

#ifndef UART_H
#define UART_H

extern void uart_rx(unsigned char byte);
void UartRxInterruptService(void);
void uart_send_data(unsigned char * data, unsigned char length);
void uart_send_char(unsigned char val);
void setup_uart_9600();

#endif

