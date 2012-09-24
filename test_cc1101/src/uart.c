#include "uart.h"
#include <legacymsp430.h>

void setup_uart_9600(){
	P1SEL |= BIT1 | BIT2 ;
	P1SEL2 |= BIT1 | BIT2 ;
	UCA0CTL1 |= UCSSEL_2 ;
	UCA0BR0 = 130 ;
	UCA0BR1 = 6 ;
	UCA0MCTL = UCBRS1 + UCBRS2;
	UCA0CTL1 &= ~UCSWRST ;
	IE2 |= UCA0RXIE ;
}

void uart_send(unsigned char val){
	while(UCA0STAT & UCBUSY);
	UCA0TXBUF = val ;
}

interrupt(USCIAB0RX_VECTOR) USCI0RX_ISR(void){
	while(!(IFG2&UCA0TXIFG));
	uart_rx(UCA0RXBUF);
}
