#include <cc1101.h>


void setup_cc1101_spi(){
	P1DIR |= BIT3; // CS	
	P1OUT |= BIT3; // CS	
	P1SEL |= BIT7 | BIT5 | BIT6 ; //SIMO & SOMI & SMCLK
	P1SEL2 |= BIT7 | BIT5 | BIT7 ;
	UCB0CTL1 |= UCSWRST ;
	UCB0CTL0 |= UCCKPL | UCMSB | UCMST | UCSYNC ;
	UCB0CTL1 |= UCSSEL_2 ;
	UCB0BR0 |= 64; //prescale by 64
	UCB0BR1	|= 0 ;
	UCB0CTL1 &= ~UCSWRST ;
}

uchar write_cc1101_reg(uchar addr, uchar data){
	uchar stat ;	
	P1OUT &= ~BIT3 ;
	while(P1IN & BIT4); // wait module ready
	UCB0TXBUF = (addr & 0x3F)  ; //clearing read bit
	while(UCB0STAT & UCBUSY);
	UCB0TXBUF = data ;
	while(UCB0STAT & UCBUSY);
	stat = UCB0RXBUF ;
	P1OUT |= BIT3 ;
	return stat ;
}

uchar strobe_cc1101(uchar cmd){
	uchar stat ;	
	P1OUT &= ~BIT3 ;
	while(P1IN & BIT4); // wait module ready
	UCB0TXBUF = (cmd & 0x3F)  ; //clearing read bit
	while(UCB0STAT & UCBUSY);
	stat = UCB0RXBUF ;
	P1OUT |= BIT3 ;
	return stat ;
}

uchar read_cc1101_reg(const uchar addr, uchar * data){
	uchar stat ;	
	P1OUT &= ~BIT3 ;
	while(P1IN & BIT4); // wait module ready
	UCB0TXBUF = (addr | 0x80)   ; //setting write bit
	while(UCB0STAT & UCBUSY);
	stat = UCB0RXBUF ;
	UCB0TXBUF = 0x00 ;
	while(UCB0STAT & UCBUSY);
	*data = UCB0RXBUF ;
	P1OUT |= BIT3 ;
	return stat ;
}

int write_cc1101_buffer(uchar addr, uchar * tx_data, uchar * rx_data, uint size){
	uint counter, free ;	
	P1OUT &= ~BIT3 ;
	while(P1IN & BIT4); // wait module ready
	UCB0TXBUF = ((addr | 0x40) & 0x3F) ; //burst bit set, read bit cleared
	while(UCB0STAT & UCBUSY);
	free = (UCB0RXBUF & 0x0F) ;
	if(free < size) return -1 ;
	for(counter = 0 ; counter < size ; counter ++){
		UCB0TXBUF = tx_data[counter] ;
		while(UCB0STAT & UCBUSY);
		if(rx_data != NULL){
			rx_data[counter] = UCB0RXBUF ;
		}
	}
	P1OUT |= BIT3 ;
	return 1 ;
}

int read_cc1101_buffer(uchar addr, uchar * rx_data, uint size){
	uint counter, avail ;	
	P1OUT &= ~BIT3 ;
	while(P1IN & BIT4); // wait module ready
	UCB0TXBUF = ((addr | 0xC0)) ; //burst bit set, read bit set
	while(UCB0STAT & UCBUSY);
	avail = (UCB0RXBUF & 0x0F) ;
	if(avail < size) return -1 ;
	for(counter = 0 ; counter < size ; counter ++){
		UCB0TXBUF = 0x00 ;
		while(UCB0STAT & UCBUSY);
		if(rx_data != NULL){
			rx_data[counter] = UCB0RXBUF ;
		}
	}
	P1OUT |= BIT3 ;
	return 1 ;
}

void setup_cc1101(const uchar cfg[][2], uint nb_regs){
	uint i ;
	strobe_cc1101(CC1101_SRES);
	for(i = 0 ; i < nb_regs ; i++){
		write_cc1101_reg(cfg[i][0], cfg[i][1]);			
	}
}

int receive_packet(cc1101_pkt * packet){
	uchar nb_data_avail ;
	uchar status [2] ;
	read_cc1101_reg(CC1101_RXBYTES, &nb_data_avail);
	if(nb_data_avail < 2){
		return -1 ;	
	}else if(nb_data_avail & 0x80){
		//fifo overflow
		strobe_cc1101(CC1101_SFRX);      // Flush RXFIFO
	}
	read_cc1101_reg(CC1101_RXFIFO, &packet->pkt_length);
	read_cc1101_reg(CC1101_RXFIFO, &packet->dst_addr);
	while(nb_data_avail < packet->pkt_length) read_cc1101_reg(CC1101_RXBYTES, &nb_data_avail);
	read_cc1101_buffer(CC1101_RXFIFO, packet->pkt_data, packet->pkt_length - 1) ;
	#ifdef ADD_STATUS
		read_cc1101_buffer(CC1101_RXFIFO, status, 2) ;
		return status[0] ;
	#else
		return 0 ;
	#endif
	
}	


int send_packet(cc1101_pkt * packet){
	uchar nb_data_free ;
	read_cc1101_reg(CC1101_TXBYTES, &nb_data_free);
	if(nb_data_free <  packet->pkt_length){
		return -1 ;	
	}
	write_cc1101_reg(CC1101_TXFIFO, packet->dst_addr);
	write_cc1101_buffer(CC1101_TXFIFO, packet->pkt_data, NULL, packet->pkt_length);
	strobe_cc1101(CC1101_STX);
	return 0 ;
}

int send_data(uchar addr, uchar * data, uchar length){
	uchar nb_data_free ;
	read_cc1101_reg(CC1101_TXBYTES, &nb_data_free);
	if(nb_data_free <  length){
		return -1 ;	
	}
	write_cc1101_reg(CC1101_TXFIFO, addr);
	write_cc1101_buffer(CC1101_TXFIFO, data, NULL, length);
	strobe_cc1101(CC1101_STX);
	return 0 ;
}

