#include <msp430g2553.h>

int main(){
	WDTCTL = WDTPW + WDTHOLD ;
	P1DIR |= BIT0 ;
	P1OUT &= ~BIT0 ;
	while(1){
		P1OUT ^= BIT0 ;
		__delay_cycles(100000);		
	}
}
