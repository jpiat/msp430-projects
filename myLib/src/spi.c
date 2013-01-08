#include <spi.h>
#include <legacymsp430.h>

#define SPI_BB_CYCLE 20


void setupSpiSlave(spi_slave * slave){
	
	if(slave->interface_type == NATIVE){
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
	}else{
		P2DIR |= ( 1 << (slave -> csPin)); // CS	
		P2OUT |= ( 1 << (slave -> csPin)); // CS		
		P2DIR |= ( 1 << (slave -> mosiPin)) | ( 1 << (slave -> clkPin)); // mosi, clk	
		P2OUT |= ( 1 << (slave -> mosiPin)) | ( 1 << (slave -> clkPin)); // clk
		P2OUT &= ~( 1 << (slave -> mosiPin)) ; // mosi
		P2DIR &= ~( 1 << (slave -> misoPin)) ; // miso
		//P2OUT |= ( 1 << (slave -> misoPin));
		//P2REN |= ( 1 << (slave -> misoPin)); // pull-up
	}
}
void spiClearCs(spi_slave * slave){
	if(slave->interface_type == NATIVE){
		P1OUT &= ~( 1 << (slave -> csPin)) ;
	}else{
		P2OUT &= ~( 1 << (slave -> csPin)) ;
	}
}

void spiSetCs(spi_slave * slave){
	if(slave->interface_type == NATIVE){
		P1OUT |= ( 1 << (slave -> csPin)) ;
	}else{
		P2OUT |= ( 1 << (slave -> csPin)) ;
	}
}

unsigned char spiWriteByte(unsigned char val, spi_slave * slave){
	if(slave->interface_type == NATIVE){
		UCB0TXBUF = val ;
		while(UCB0STAT & UCBUSY);
		return UCB0RXBUF;	
	}else{
		unsigned char i = 0 ;
		unsigned char valBuf = val ;
		unsigned char inBuf = 0 ;
		__delay_cycles(SPI_BB_CYCLE);
		for(i = 0 ; i < 8 ; i ++){ //MODE3
			if(valBuf & 0x80){
				P2OUT &= ~( 1 << (slave->clkPin)); // clk down
				P2OUT |= ( 1 << (slave->mosiPin)); // mosi set
			}else{
				P2OUT &= ~( 1 << (slave->clkPin)) ; // clk down
				P2OUT &= ~( 1 << (slave->mosiPin)); // mosi clear
			}
			valBuf = (valBuf << 1);
			__delay_cycles(SPI_BB_CYCLE);
			inBuf = (inBuf << 1) ;
			if((P2IN & (1 << slave->misoPin))){
				inBuf |= 0x01 ;
			}else{
				nop();			
			}
			P2OUT |= ( 1 << (slave->clkPin)); // clk up
			__delay_cycles(SPI_BB_CYCLE);		
		}
		__delay_cycles(SPI_BB_CYCLE);
		/*for(i = 0 ; i < 8 ; i ++){ //MODE2
			if(valBuf & 0x80){
				P2OUT |= ( 1 << (slave->mosiPin)); // mosi set
			}else{
				P2OUT &= ~( 1 << (slave->mosiPin)); // mosi clear
			}
			valBuf = (valBuf << 1);
			P2OUT &= ~( 1 << (slave->clkPin)); // clk down	
			__delay_cycles(SPI_BB_CYCLE);
			inBuf = (inBuf << 1) ;
			if((P2IN & (slave->misoPin))){
				inBuf |= 0x01 ;
			}
			P2OUT |= ( 1 << (slave->clkPin)); // clk up
			__delay_cycles(SPI_BB_CYCLE);	
		}*/
		
		return inBuf ;
	}
}
void spiTxRx(spi_slave * slave, unsigned char * send_buffer, unsigned char * receive_buffer , unsigned char length){
	unsigned int i ;	
	spiClearCs(slave);
	for(i = 0 ; i < length ; i ++){
		receive_buffer[i] = spiWriteByte(send_buffer[i], slave);
	}
	spiSetCs(slave);
}
