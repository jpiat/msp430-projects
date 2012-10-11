#include <msp430g2553.h>
#include "CC1101_simple_link_reg_config.h"
#include "uart.h"


cc1101_pkt packet_to_send, packet_to_receive ;


void uart_rx(unsigned char val){		
		if(val == '\n'){
			send_packet(&packet_to_send);			
		}else{
			packet_to_send.pkt_data[packet_to_send.pkt_length] = val ;	
			packet_to_send.pkt_length ++ ;			
		}
		uart_send_char(val);
}


void init_packets(){
	packet_to_send.dst_addr = 0 ;
	packet_to_send.pkt_length = 0 ;

	packet_to_receive.dst_addr = 0 ;
	packet_to_receive.pkt_length = 0 ;
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
	__bis_SR_register(GIE);
	while(1){
		__delay_cycles(100000);	
		//switchToRX();
		dummy = receive_packet(&packet_to_receive);
		if(dummy == 0){
			uart_send_data(packet_to_receive.pkt_data, packet_to_receive.pkt_length);
		}
	}
}
