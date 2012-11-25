#include <spi.h>



void setupSpiSlave(spi_slave * slave){
	P1DIR |= ( 1 << (slave -> csPin)); // CS	
	P1OUT |= ( 1 << (slave -> csPin)); // CS	
	P1SEL |= BIT7 | BIT5 | BIT6 ; //SIMO & SOMI & SMCLK
	P1SEL2 |= BIT7 | BIT5 | BIT7 ;
	UCB0CTL1 |= UCSWRST ;
	UCB0CTL0 |= UCCKPL | UCMSB | UCMST | UCSYNC ;
	UCB0CTL1 |= UCSSEL_2 ;
	UCB0BR0 |= 64; //prescale by 64
	UCB0BR1	|= 0 ;
	UCB0CTL1 &= ~UCSWRST ;
}
void spiClearCs(spi_slave * slave){
	P1OUT &= ~( 1 << (slave -> csPin)) ;
}

void spiSetCs(spi_slave * slave){
	P1OUT |= ( 1 << (slave -> csPin)) ;
}

unsigned char spiWriteByte(unsigned char val){
	UCB0TXBUF = val ;
	while(UCB0STAT & UCBUSY);
	return UCB0RXBUF;
}
void spiTxRx(spi_slave * slave, unsigned char * send_buffer, unsigned char * receive_buffer , unsigned char length){
	unsigned int i ;	
	spiClearCs(slave);
	for(i = 0 ; i < length ; i ++){
		receive_buffer[i] = spiWriteByte(send_buffer[i]);
	}
	spiSetCs(slave);
}
