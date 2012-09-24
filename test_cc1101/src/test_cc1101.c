#include <msp430g2553.h>
#include "cc1101.h"


int main(){
	uchar tx_buffer [64] ;
	uint i ;	
	WDTCTL = WDTPW + WDTHOLD ;
	BCSCTL1 = CALBC1_16MHZ ;
	DCOCTL = CALDCO_16MHZ ;
	setup_cc1101_spi();	
	setup_cc1101(cc1101_433_cfg, 2);	
	P1DIR |= BIT0 ;
	P1OUT &= ~BIT0 ;
	for(i = 0 ; i < 64 ; i ++){
		tx_buffer[i] = i ;	
	}
	while(1){
		P1OUT ^= BIT0 ;
		write_cc1101_buffer(CC1101_TXFIFO , tx_buffer, NULL, 2);
		__delay_cycles(100000);		
	}
}
