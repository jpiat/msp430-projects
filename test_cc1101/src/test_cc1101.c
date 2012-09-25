#include <msp430g2553.h>
#include "CC1101_simple_link_reg_config.h"
#include "uart.h"



void uart_rx(unsigned char val){
	uchar stat ;		
		if(val == '\n'){
			stat = strobe_cc1101(CC1101_SFRX);			
			uart_send(stat);	
		}else{
			stat = write_cc1101_reg(CC1101_TXFIFO, val);			
			uart_send(stat);	
		}
}

int main(){
	uchar fifo_count, tx_char ;	
	WDTCTL = WDTPW + WDTHOLD ;
	BCSCTL1 = CALBC1_16MHZ ;
	DCOCTL = CALDCO_16MHZ ;
	setup_cc1101_spi();	
	setup_cc1101(cc1101_cfg, 22);
	setup_uart_9600();
	P1DIR |= BIT0 ;
	P1OUT &= ~BIT0 ;
	__bis_SR_register(GIE);
	while(1){
		P1OUT ^= BIT0 ;
		__delay_cycles(100000);	
		read_cc1101_reg(CC1101_RXBYTES, &fifo_count);
		if(fifo_count & 0x7F){
			read_cc1101_reg(CC1101_RXFIFO, &tx_char);
			uart_send(tx_char);
		}
	}
}
