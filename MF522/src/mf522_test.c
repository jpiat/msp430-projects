#include <msp430g2553.h>
#include "my_types.h"
#include "uart.h"
#include "spi.h"
#include "mf522.h"


spi_slave mf522_spi ;
uchar buffer[24] ;

void uart_rx(){
	return ;
}


int main(){
	uchar dummy, index, status;	
	WDTCTL = WDTPW + WDTHOLD ;
	BCSCTL1 = CALBC1_16MHZ ;
	DCOCTL = CALDCO_16MHZ ;
	setupSpiSlave(&mf522_spi);	
	MFRC522_Init(&mf522_spi);
	while(1){
		__delay_cycles(100000);	
		status = MFRC522_Request(PICC_REQIDL, buffer);	
		if (status == MI_OK)
		{
                        //Serial.println("Card detected");
			//Serial.print(str[0],BIN);
                        //Serial.print(" , ");
			//Serial.print(str[1],BIN);
                        //Serial.println(" ");
		}
		
	}
}
