
#include "i2c.h"


volatile unsigned char i2cBusy = 0;
volatile unsigned char i2cBufferLength = 0 ;
unsigned char * i2cBufferPtr ; 
volatile char txDone = 1 ;
volatile char rxDone = 1 ;




void initi2c(unsigned int divider){
	
	P1SEL |= BIT6 + BIT7 ;
	P1SEL2 |= BIT6 + BIT7 ;
	UCB0CTL1 |= UCSWRST ;
	UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC ;
	UCB0CTL1 = UCSSEL_2 + UCSWRST ;
	UCB0BR0 = divider & 0x00FF ;
	UCB0BR1 = ((divider & 0xFF00) >> 8) ;
	UCB0I2CSA = 0x20 ; 
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
unsigned char readi2c(unsigned char addr, unsigned char * data, unsigned char nbData){
	UCB0I2CSA = addr ;
	i2cBusy = 0 ;
	i2cBufferPtr = data ;
	i2cBufferLength = nbData ;
	rxDone = 0 ;
	UCB0CTL1 &= ~UCTR;
	UCB0CTL1 |= UCTXSTT;
	if(nbData == 1){
	 	while(UCB0CTL1 & UCTXSTT);
		UCB0CTL1 |= UCTXSTP ;
		
	}
	while(rxDone == 0) ;
	return rxDone ;
}


inline void i2cDataInterruptService(void){
	if(UCB0CTL1 & UCTR){
		if(i2cBusy >=  i2cBufferLength){
			UCB0CTL1 |= UCTXSTP ;
			i2cBusy = 0 ;	
			txDone = 1 ;
			IFG2 &= ~UCB0TXIFG;
		}else{
			UCB0TXBUF = i2cBufferPtr[i2cBusy] ;
			i2cBusy ++ ;
		}
	}else{
		if((i2cBufferLength - i2cBusy) == 1){ // may generate a repeated stop condition
			UCB0CTL1 |= UCTXSTP ;
		}
		i2cBufferPtr[i2cBusy] = UCB0RXBUF;
		i2cBusy ++ ;
		if(i2cBusy >= i2cBufferLength){
			i2cBusy = 0 ;	
			rxDone = 1 ;
		}
	}
}

inline void i2cErrorInterruptService(void){
	  if (UCB0STAT & UCNACKIFG) {
		UCB0CTL1 |= UCTXSTP;
		UCB0STAT &= ~UCNACKIFG;
		i2cBusy = -1;
		txDone = -1 ;
		rxDone = -1 ;
	}else if (UCB0STAT & UCALIE) {
                UCB0CTL1 |= UCTXSTP;
                UCB0STAT &= ~UCALIE;
                i2cBusy = -1;
		txDone = -1 ;
		rxDone = -1 ;
        }
}





