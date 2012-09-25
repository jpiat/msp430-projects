#include <msp430g2553.h>


extern void uart_rx(unsigned char val);
void uart_send_data(unsigned char * data, unsigned char length);
void uart_send_char(unsigned char val);
void setup_uart_9600();


