
#include <msp430g2553.h>
#include "my_types.h"
//#include "i2c.h"
#include <legacymsp430.h>

unsigned char oledBuffer[2] ;
unsigned char pageBuffer[] = {0x00, 0x03, 0x0F, 0x3F, 0xFF};


volatile unsigned char i2cBusy = 0;
volatile unsigned char i2cBufferLength = 0 ;
unsigned char * i2cBufferPtr ; 
volatile char txDone = 1 ;

void initi2c(){
	P1SEL |= BIT6 + BIT7 ;
	P1SEL2 |= BIT6 + BIT7 ;
	P1REN |= BIT6 + BIT7;
	P1OUT |= BIT6 + BIT7 ;
	UCB0CTL1 |= UCSWRST ;
	UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC ;
	UCB0CTL1 = UCSSEL_2 + UCSWRST ;
	UCB0BR0 = 192 ;
	UCB0BR1 = 0;
	UCB0I2CSA = 0x3D ; 
	UCB0I2CIE = UCNACKIE + UCALIE ;
	UCB0CTL1 &= ~UCSWRST ;
	IE2 |= UCB0TXIE | UCB0RXIE ;
}
void writei2c(unsigned char addr, unsigned char * data, unsigned char nbData){	
	UCB0I2CSA = 0x3C ;
	i2cBusy = 0 ;
	i2cBufferPtr = data ;
	i2cBufferLength = nbData ;
	txDone = 0 ;
	UCB0CTL1 |= UCTR + UCTXSTT;
	while(txDone == 0) ;
}
unsigned char readi2c(unsigned char addr){
	return 0x00 ;
}



void oledSendData(unsigned char command){
	oledBuffer[0] = 0x00;
	oledBuffer[1] = command ;
	writei2c(0x3D, oledBuffer, 2);
}


void oledSendCommand(unsigned char command){
	oledBuffer[0] = 0x80;
	oledBuffer[1] = command ;
	writei2c(0x3D, oledBuffer, 2);
}



/* Function    : Set_Start_Column
 * Description : Sets a start column to start writing at.
 * Input       : column[0-127]
 * Output      : None
 */
void Set_Start_Column(unsigned char d)
{
	// Set Lower Column Start Address for Page Addressing Mode. Default => 0x00
   oledSendCommand(0x00+d%16);		
	
	// Set Higher Column Start Address for Page Addressing Mode. Default => 0x10
   oledSendCommand(0x10+d/16);		
						
}

/* Function    : Set_Start_Page
 * Description : Sets a start page to start writing at.
 * Input       : column[0-7]
 * Output      : None
 */
void Set_Start_Page(unsigned char d)
{
	// Set Page Start Address for Page Addressing Mode. Default => 0xB0 (0x00)
   oledSendCommand(0xB0|d);			
						
}


void fill_oled_page(unsigned char page, unsigned char * data, unsigned char dataSize)
{
   unsigned char j;

   Set_Start_Page(page);
   Set_Start_Column(0x00);

   for(j=0;j<128;j++)
   {
      oledSendData(data[j%dataSize]);
   }
}


int main(){
	unsigned long int i ;	
	WDTCTL = WDTPW + WDTHOLD ;
	BCSCTL1 = CALBC1_16MHZ ;
	DCOCTL = CALDCO_16MHZ ;
	P1DIR |= BIT0 + BIT5;
	P1OUT &= ~BIT5;
	for(i = 0 ; i < 3000 ; i++){
		nop();
	}
	P1OUT |= BIT5 ;
	P1OUT &= ~BIT0 ;
	initi2c();
	__bis_SR_register(GIE);
	oledSendCommand(0xAE);	
	oledSendCommand(0x00);
	oledSendCommand(0x10);
	oledSendCommand(0x40);
	oledSendCommand(0x81);
	oledSendCommand(0xCF);
	oledSendCommand(0xA1);
	oledSendCommand(0xA6);
	oledSendCommand(0xA8);
	oledSendCommand(0x3F);
	oledSendCommand(0xD3);
	oledSendCommand(0x00);
	oledSendCommand(0xD5);
	oledSendCommand(0x80);
	oledSendCommand(0xD9);
	oledSendCommand(0xF1);
	oledSendCommand(0xDA);
	oledSendCommand(0x12);
	oledSendCommand(0xDB);
	oledSendCommand(0x40);	
	oledSendCommand(0x8D);
	//oledSendCommand(0x14);
	oledSendCommand(0x10);
	oledSendCommand(0xAF);
	fill_oled_page(0, pageBuffer, 5);
	fill_oled_page(1, pageBuffer, 5);
	fill_oled_page(2, pageBuffer, 5);
	fill_oled_page(3, pageBuffer, 5);
	fill_oled_page(4, pageBuffer, 5);
	fill_oled_page(5, pageBuffer, 5);
	P1OUT &= ~BIT0 ;
	while(1);
}

interrupt(USCIAB0TX_VECTOR) USCI0TX_ISR(void){
	if (IFG2 & UCB0TXIFG) {	
		if(i2cBusy >=  i2cBufferLength){
			UCB0CTL1 |= UCTXSTP ;
			i2cBusy = 0 ;	
			txDone = 1 ;
			IFG2 &= ~UCB0TXIFG;
		}else{
			UCB0TXBUF = i2cBufferPtr[i2cBusy] ;
			i2cBusy ++ ;
		}
	}
	
}

interrupt(USCIAB0RX_VECTOR) USCI0RX_ISR(void) {
        if (UCB0STAT & UCNACKIFG) {
		UCB0CTL1 |= UCTXSTP;
		UCB0STAT &= ~UCNACKIFG;
		i2cBusy = -1;
		txDone = -1 ;
	}else if (UCB0STAT & UCALIE) {
                UCB0CTL1 |= UCTXSTP;
                UCB0STAT &= ~UCALIE;
                i2cBusy = -1;
		txDone = -1 ;
        }
        P1OUT |= BIT0 ;
	IFG2 &= ~UCB0RXIFG;
}



