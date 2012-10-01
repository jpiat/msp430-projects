#include <msp430g2553.h>

unsigned char modulo ;

int main(){
	WDTCTL = WDTPW + WDTHOLD ;
	P1DIR |= BIT0 | BIT6;
	P1OUT &= ~ (BIT0 | BIT6) ;
	modulo = 0 ;
	while(1){
		P1OUT ^= BIT0 ;
		if(modulo & 0x08){
			 P1OUT |= BIT6 ;
		}
		else{
			P1OUT &= ~BIT6 ;
		}
		modulo ++ ;
		__delay_cycles(100000);		
	}
}
