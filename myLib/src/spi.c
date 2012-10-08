#include <spi.h>



void setup_spi_slave(spi_slave * slave){
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

void txrx_slave(spi_slave * slave, unsigned char * send_buffer, unsigned char * receive_buffer , unsigned char length){
	unsigned char stat, i ;	
	P1OUT &= ~( 1 << (slave -> csPin)) ;
	for(i = 0 ; i < length ; i ++){
		UCB0TXBUF = send_buffer[i] ;
		while(UCB0STAT & UCBUSY);
		receive_buffer[i] = UCB0RXBUF;
	}
	P1OUT |= ( 1 << (slave -> csPin)) ;
}
