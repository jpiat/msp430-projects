#include <msp430g2553.h>
#include "my_types.h"
#include "uart.h"
#include "spi.h"
#include "mf522.h"
#include "display.h"

spi_slave mf522_spi ;
uchar buffer[24] ;
char printBuffer[16];

char * msg1 = "no card detected\n";

void uart_rx(unsigned char byte){
	return ;
}


int main(){
	uchar status;
	uint length ;	
	WDTCTL = WDTPW + WDTHOLD ;
	BCSCTL1 = CALBC1_16MHZ ;
	DCOCTL = CALDCO_16MHZ ;
	mf522_spi.csPin = 3;
	setup_uart_9600();
	setupSpiSlave(&mf522_spi);	
	MFRC522_Init(&mf522_spi);
	while(1){
		__delay_cycles(100000);	
		status = MFRC522_Request(PICC_REQIDL, buffer);	
		if (status == MI_OK)
		{
			length = sprintf(printBuffer, "Card nb :%d%d \n",buffer[0], buffer[1] );                   
			uart_send_data((unsigned char*) printBuffer, length);
		}else{
			uart_send_data((unsigned char*) msg1, 17);
		}
		
	}
}
