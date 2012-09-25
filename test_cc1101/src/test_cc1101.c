#include <msp430g2553.h>
#include "CC1101_simple_link_reg_config.h"
#include "uart.h"


cc1101_pkt send_packet, receive_packet ;


void uart_rx(unsigned char val){
	uchar stat ;		
		if(val == '\n'){
			send_packet(&send_packet);			
			uart_send(stat);	
		}else{
			send_packet.pkt_data[send_packet.pkt_length] = val ;	
			send_packet.pkt_length ++ ;			
		}
}


void init_packets(){
	send_packet.dst_addr = 0 ;
	send_packet.pkt_length = 0 ;

	receive_packet.dst_addr = 0 ;
	receive_packet.pkt_length = 0 ;
}

int main(){
	uchar dummy;	
	WDTCTL = WDTPW + WDTHOLD ;
	BCSCTL1 = CALBC1_16MHZ ;
	DCOCTL = CALDCO_16MHZ ;
	setup_cc1101_spi();	
	setup_cc1101(cc1101_cfg, 22);
	setup_uart_9600();
	init_packets();
	P1DIR |= BIT0 ;
	P1OUT &= ~BIT0 ;
	__bis_SR_register(GIE);
	while(1){
		P1OUT ^= BIT0 ;
		__delay_cycles(100000);	
		dummy = receive_packet(&receive_packet);
		if(dummy == 0){
			uart_send(receive_packet.pkt_data, receive_packet.pkt_length);
		}
	}
}
