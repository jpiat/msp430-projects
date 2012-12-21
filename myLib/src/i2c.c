
#include "i2c.h"


volatile unsigned char i2cBusy = 0;
volatile unsigned char i2cBufferLength = 0 ;
unsigned char * i2cBufferPtr ; 
volatile char txDone = 1 ;

void initi2c(unsigned int divider){
	
	P1SEL |= BIT6 + BIT7 ;
	P1SEL2 |= BIT6 + BIT7 ;
	P1DIR |= BIT6 + BIT7;
	UCB0CTL1 |= UCSWRST ;
	UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC ;
	UCB0CTL1 = UCSSEL_2 + UCSWRST ;
	UCB0BR0 = divider ;
	UCB0BR1 = 0;
	UCB0I2CSA = 0x00 ; 
	UCB0I2CIE = UCNACKIE + UCALIE ;
	UCB0CTL1 &= ~UCSWRST ;
	IE2 |= UCB0TXIE | UCB0RXIE ;
}
void writei2c(unsigned char addr, unsigned char * data, unsigned char nbData){	
	UCB0I2CSA = addr ;
	i2cBusy = 0 ;
	i2cBufferPtr = data ;
	i2cBufferLength = nbData ;
	txDone = 0 ;
	UCB0CTL1 |= UCTR + UCTXSTT;
	while(txDone == 0) ;
}
unsigned char readi2c(unsigned char addr){
	return 0x00 ;
}


inline void i2cTxInterruptService(void){
	if(i2cBusy >=  i2cBufferLength){
		UCB0CTL1 |= UCTXSTP ;
		i2cBusy = 0 ;	
		txDone = 1 ;
		IFG2 &= ~UCB0TXIFG;
	}else{
		UCB0TXBUF = i2cBufferPtr[i2cBusy] ;
		i2cBusy ++ ;
	}
}

inline void i2cRxInterruptService(void){
	  if (UCB0STAT & UCNACKIFG) {
		UCB0CTL1 |= UCTXSTP;
		UCB0STAT &= ~UCNACKIFG;
		i2cBusy = -1;
		txDone = -1 ;
	}else if (UCB0STAT & UCALIE) {
                UCB0CTL1 |= UCTXSTP;
                UCB0STAT &= ~UCALIE;
                i2cBusy = -1;
		txDone = -1 ;
        }
}





