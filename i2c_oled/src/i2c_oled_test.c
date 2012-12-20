
#include <msp430g2553.h>
#include "my_types.h"
#include "i2c.h"
#include "CourierNew_5x7.h"
#include "CourierNew_14x15.h"
#include <legacymsp430.h>

unsigned char oledBuffer[2] ;
unsigned char pageBuffer[] = {0xFF, 0x3F, 0x0F, 0x03, 0x00};
/*
#define font_table data_table_SMALL
#define FONT_WIDTH 5
#define FONT_SPAN 1
*/
#define font_table data_table_LARGE
#define FONT_WIDTH 14
#define FONT_SPAN 2

//const char * testPhrase = "CE QUI SE CONCOIT BIEN S'ENONCE CLAIREMENT ET LES MOTS POUR LE DIRE VOUS VIENNENT AISEMENT";
const char * testPhrase = "THIS IS A TEST !";

//#define OLED_ADDR 0x7A
#define OLED_ADDR 0x3D


void oledSendData(unsigned char command){
	oledBuffer[0] = 0x40;
	oledBuffer[1] = command ;
	writei2c(OLED_ADDR, oledBuffer, 2);
}


void oledSendCommand(unsigned char command){
	oledBuffer[0] = 0x80;
	oledBuffer[1] = command ;
	writei2c(OLED_ADDR, oledBuffer, 2);
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

void print_screen(unsigned char page, unsigned char column, unsigned char * txt){
	unsigned int i = 0,j,k,c ;
	unsigned int curPage = page, curCol=column ;
	unsigned int cIndex ;
	c = txt[i];	
	while(c!='\0'){
		cIndex = (c - 32)*FONT_WIDTH*FONT_SPAN;
		for(k=0 ; k < FONT_SPAN ; k++){	
			Set_Start_Page(curPage - k);
			Set_Start_Column(curCol);
			for(j=0 ; j < FONT_WIDTH ; j++){
				oledSendData(font_table[cIndex + j + (k*FONT_WIDTH) ]);
			}
		}
		i ++ ;
		curCol = (curCol + (FONT_WIDTH));
		if(curCol >= (128-FONT_WIDTH)){
			curPage = curPage - FONT_SPAN ;
			curCol = column ;
		}
		c = txt[i];
	}
}

void fill_screen(unsigned char data)
{
   unsigned char i, j;
	for(i = 0; i < 8 ; i ++){
	   Set_Start_Page(i);
	   Set_Start_Column(0x00);
	   for(j=0;j<128;j++)
	   {
	      oledSendData(data);
	   }
	}
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
	P1OUT |= BIT0 ;
	fill_screen(0x00) ;
	print_screen(5, 0, testPhrase);
	/*	fill_oled_page(0, &data_table_SMALL[0], 128);
	fill_oled_page(1, &data_table_SMALL[128], 128);
	fill_oled_page(2, &data_table_SMALL[256], 128);
	fill_oled_page(3, &data_table_SMALL[384], 128);
	fill_oled_page(4, &data_table_SMALL[0], 128);
	fill_oled_page(5, &data_table_SMALL[128], 128);
	fill_oled_page(6, &data_table_SMALL[256], 128);
	fill_oled_page(7, &data_table_SMALL[384], 128);*/
	P1OUT &= ~BIT0 ;
	while(1);
}

interrupt(USCIAB0TX_VECTOR) USCI0TX_ISR(void){
	if (IFG2 & UCB0TXIFG) {	
		i2cTxInterruptService();
	}
	
}

interrupt(USCIAB0RX_VECTOR) USCI0RX_ISR(void) {
      	i2cRxInterruptService();
	IFG2 &= ~UCB0RXIFG;
}



