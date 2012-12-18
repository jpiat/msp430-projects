
#include "i2c.h"


unsigned char i2cBusy = 0;
unsigned char i2cBufferLength = 0 ;
unsigned char * i2cBufferPtr ; 

void initi2c(){
	P1SEL |= BIT6 + BIT7 ;
	P1SEL2 |= BIT6 + BIT7 ;
	P1REN |= BIT6 + BIT7;
	P1OUT |= BIT6 + BIT7 ;
	UCB0CTL1 |= UCSWRST ;
	UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC ;
	UCB0CTL1 = UCSSEL_2 + UCSWRST ;
	UCB0BR0 = 144 ;
	UCB0BR1 = 0;
	//UCB0I2CSA = 0x3C ; 
	UCB0I2CIE = /*UCNACKIE +*/ UCALIE ;
	UCB0CTL1 &= ~UCSWRST ;
	IE2 |= UCB0TXIE ;//| UCB0RXIE ;
}
void writei2c(unsigned char addr, unsigned char * data, unsigned char nbData){
	UCB0I2CSA = 0x3C ;
	i2cBusy = 0 ;
	i2cBufferPtr = data ;
	i2cBufferLength = nbData ;
	UCB0CTL1 |= UCTR + UCTXSTT;
	while(i2cBusy) ;
}
unsigned char readi2c(unsigned char addr){
	return 0x00 ;
}


void i2cInterruptService(void){
	if(i2cBusy >=  i2cBufferLength){
		UCB0CTL1 |= UCTXSTP ;
		i2cBusy = 0 ;	
	}else{
		UCB0TXBUF = i2cBufferPtr[i2cBusy] ;
		i2cBusy ++ ;
	}
	IFG2 &= ~UCB0TXIFG ;
}



