#include <cc1101.h>

#define ADD_STATUS 1




void setup_cc1101_spi(){
	CS_PDIR |= CS; // CS	
	CS_POUT |= CS; // CS
	
	GDO0_PDIR &= ~GDO0 ;
	GDO0_PREN |= GDO0 ;
	GDO0_POUT |= GDO0 ;
		
	SPI_PSEL |= SIMO | SOMI | SMCLK ; //SIMO & SOMI & SMCLK
	SPI_PSEL2 |= SIMO | SOMI | SIMO ;
	UCB0CTL1 |= UCSWRST ;
	UCB0CTL0 |= UCCKPL | UCMSB | UCMST | UCSYNC ;
	UCB0CTL1 |= UCSSEL_2 ;
	UCB0BR0 |= 64; //prescale by 64
	UCB0BR1	|= 0 ;
	UCB0CTL1 &= ~UCSWRST ;
}

uchar write_cc1101_reg(uchar addr, uchar data){
	uchar stat ;	
	CS_POUT &= ~CS ;
	while(GDO2_PIN & GDO2); // wait module ready
	UCB0TXBUF = (addr & 0x3F)  ; //clearing read bit
	while(UCB0STAT & UCBUSY);
	UCB0TXBUF = data ;
	while(UCB0STAT & UCBUSY);
	stat = UCB0RXBUF ;
	CS_POUT |= CS ;
	return stat ;
}

uchar strobe_cc1101(uchar cmd){
	uchar stat ;	
	CS_POUT &= ~CS ;
	while(GDO2_PIN & GDO2); // wait module ready
	UCB0TXBUF = (cmd & 0x3F)  ; //clearing read bit
	while(UCB0STAT & UCBUSY);
	stat = UCB0RXBUF ;
	CS_POUT |= CS ;
	return stat ;
}

uchar read_cc1101_reg(const uchar addr, uchar * data){
	uchar stat ;	
	CS_POUT &= ~CS ;
	while(GDO2_PIN & GDO2); // wait module ready
	UCB0TXBUF = (addr | 0x80)   ; //setting write bit
	while(UCB0STAT & UCBUSY);
	stat = UCB0RXBUF ;
	UCB0TXBUF = 0x00 ;
	while(UCB0STAT & UCBUSY);
	*data = UCB0RXBUF ;
	CS_POUT |= CS ;
	return stat ;
}

uchar read_cc1101_status(const uchar addr, uchar * data){
	uchar stat ;	
	CS_POUT &= ~CS ;
	while(GDO2_PIN & GDO2); // wait module ready
	UCB0TXBUF = (addr | 0xC0)   ; //setting write bit
	while(UCB0STAT & UCBUSY);
	stat = UCB0RXBUF ;
	UCB0TXBUF = 0x00 ;
	while(UCB0STAT & UCBUSY);
	*data = UCB0RXBUF ;
	CS_POUT |= CS ;
	return stat ;
}

int write_cc1101_buffer(uchar addr, uchar * tx_data, uchar * rx_data, uint size){
	uint counter, free ;	
	CS_POUT &= ~CS ;
	while(GDO2_PIN & GDO2); // wait module ready
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
	CS_POUT |= CS ;
	return 1 ;
}

int read_cc1101_buffer(uchar addr, uchar * rx_data, uint size){
	uint counter, avail ;	
	CS_POUT &= ~CS ;
	while(GDO2_PIN & GDO2); // wait module ready
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
	CS_POUT |= CS ;
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
	read_cc1101_status(CC1101_RXBYTES, &nb_data_avail);
	if(((nb_data_avail & 0x7F) < 2) && !(nb_data_avail & 0x80)){
		return -1 ;	
	}else if(nb_data_avail & 0x80){
		//fifo overflow
		strobe_cc1101(CC1101_SIDLE); 
		strobe_cc1101(CC1101_SFRX);      // Flush RXFIFO
		switchToRX();
		return -1 ;
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
	uchar nb_data_to_send, cc1101_state ;
	read_cc1101_status(CC1101_TXBYTES, &nb_data_to_send);
	if((nb_data_to_send & 0x7F) > 0){
		strobe_cc1101(CC1101_SFTX);// not the best thing to do but works
	}
	write_cc1101_reg(CC1101_TXFIFO, packet->pkt_length + 1);
	write_cc1101_reg(CC1101_TXFIFO, packet->dst_addr);
	write_cc1101_buffer(CC1101_TXFIFO, packet->pkt_data, NULL, packet->pkt_length);
	strobe_cc1101(CC1101_STX);
	read_cc1101_status(CC1101_MARCSTATE, &cc1101_state);
	cc1101_state = cc1101_state & 0x1F ;
	if((cc1101_state != 0x13) && (cc1101_state != 0x14) && (cc1101_state != 0x15)){
		strobe_cc1101(CC1101_SIDLE);
		strobe_cc1101(CC1101_SFTX);
		switchToRX();
		return -1;
	}

	while (!(GDO0_PIN&GDO0)); // wait synced
  	while (GDO0_PIN&GDO0); // wait TX done
	switchToRX();
	return 0 ;
}

int send_data(uchar addr, uchar * data, uchar length){
	uchar nb_data_to_send, cc1101_state ;
	read_cc1101_status(CC1101_TXBYTES, &nb_data_to_send);
	if((nb_data_to_send & 0x7F) > 0){
		strobe_cc1101(CC1101_SFTX);// not the best thing to do but works
	}
	write_cc1101_reg(CC1101_TXFIFO, length + 1);
	write_cc1101_reg(CC1101_TXFIFO, addr);
	write_cc1101_buffer(CC1101_TXFIFO, data, NULL, length);
	strobe_cc1101(CC1101_STX);
	read_cc1101_status(CC1101_MARCSTATE, &cc1101_state);
	cc1101_state = cc1101_state & 0x1F ;
	 if((cc1101_state != 0x13) && (cc1101_state != 0x14) && (cc1101_state != 0x15)){
		strobe_cc1101(CC1101_SIDLE);
		strobe_cc1101(CC1101_SFTX);
		switchToRX();
		return -1;
	}

	while (!(GDO0_PIN&GDO0)); // wait synced
  	while (GDO0_PIN&GDO0); // wait TX done
	switchToRX();
	return 0 ;
}


void setChannel(uchar chan){
	write_cc1101_reg(CC1101_CHANNR, chan);
}

void setDeviceAddr(uchar addr){
	write_cc1101_reg(CC1101_ADDR, addr);
}

void switchToRX(){
	strobe_cc1101(CC1101_SRX);
}
