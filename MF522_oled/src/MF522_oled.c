#include <msp430g2553.h>
#include "my_types.h"
#include "i2c.h"
#include "spi.h"
#include "mf522.h"
#include "CourierNew_5x7.h"
#include "CourierNew_14x15.h"
#include <legacymsp430.h>
#include <stdarg.h>

#define MAX_LENGTH	40
#define LINE_JUMP	9
unsigned char mf522_buffer[16] ;
unsigned char oledBuffer[15] ;
unsigned char pageBuffer[MAX_LENGTH] ;

spi_slave mf522_spi ;

const char * msg1 = "no card \ndetected\n";

/*
#define font_table data_table_SMALL
#define FONT_WIDTH 5
#define FONT_SPAN 1
*/
#define font_table data_table_LARGE
#define FONT_WIDTH 14
#define FONT_SPAN 2

//#define OLED_ADDR 0x7A
#define OLED_ADDR 0x3D



unsigned int itoa(int n, char * buffer) {
        unsigned char i = 0;
        unsigned int div = 10000;
        unsigned int f;
        unsigned char leading_zero = 1;
        char c;
        if (n < 0) {
                buffer[i] = '-';
                i++;
                n = -n;
        }
        for (div = 10000; div >= 1; div = div / 10) {
                f = n / div;
                if (leading_zero) {
                        if (f != 0 || div == 10) {
                                leading_zero = 0;
                                c = f + 48;
                                buffer[i] = c;
                                i += 1;
                        }
                } else {
                        c = f + 48;
                        buffer[i] = c;
                        i++;
                }
                n = n - (f * div);
        }
        return (i);
}

unsigned int ftoa(float n, char * buffer) {
        unsigned char i = 0;
        float div = 1000;
        unsigned int f;
        unsigned char leading_zero = 1;
        char c;
        if (n < 0) {
                buffer[i] = '-';
                i++;
                n = -n;
        }
        if(n == 0){
                buffer[i] = 48 ;
                i ++ ;
                return i ;
        }
        for (div = 1000; div >= 0.001 && i < 4; div = div / 10) {
                if (div == 0.1) {
                        if(leading_zero){
                                buffer[i] = '0';
                                i += 1;
                        }
                        buffer[i] = '.';
                        i += 1;
                        leading_zero = 0;
                }
                f = n / div;
                if (leading_zero) {
                        if (f != 0) {
                                leading_zero = 0;
                                c = f + 48;
                                buffer[i] = c;
                                i += 1;
                        }
                } else {
                        c = f + 48;
                        buffer[i] = c;
                        i++;
                }
                n = n - (f * div);
        }
        return (i);
}


unsigned int sprintf(char *buffer, const char * txt, ...) {
        va_list arguments;
        unsigned int length = 0;
        unsigned int wr_index = 0;
        va_start ( arguments, txt );
        while(length < MAX_LENGTH && txt[length] != 0) {
               if(txt[length] =='\n') {
                        while(wr_index%LINE_JUMP) {
                                buffer[wr_index] = ' ';
                                wr_index ++;
                        }
                        length ++;
                }
                if(txt[length] =='%') {
                        int nb;
                        float fb;
                        switch(txt[length + 1]) {
                                case 'd':
		                        nb = va_arg( arguments, int );
		                        wr_index += itoa(nb, (char *) &buffer[wr_index]);
		                        length += 2;
		                        break;
                                case 'f':
		                        fb = va_arg( arguments, double );
		                        wr_index += ftoa(fb, (char *) &buffer[wr_index]);
		                        length += 2;
		                        break;
                                default:
		                        buffer[wr_index] = txt[length];
		                        length ++;
		                        wr_index ++;
		                        break;
                        }
                } else {
                        buffer[wr_index] = txt[length];
                        length ++;
                        wr_index ++;
                }

        }
	buffer[wr_index] = '\0';
        va_end(arguments);
        return wr_index;
}




void oledSendDataBuffer(unsigned char * dataBuffer, unsigned char bufferLength){
	unsigned char i ;	
	oledBuffer[0] = 0x40;
	for(i = 0 ; i < bufferLength ; i++){
		oledBuffer[i+1] = dataBuffer[i];	
	}
	writei2c(OLED_ADDR, oledBuffer, bufferLength+1);
}


void oledSendData(unsigned char data){
	oledBuffer[0] = 0x40;
	oledBuffer[1] = data ;
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
			oledSendDataBuffer(&font_table[cIndex + (k*FONT_WIDTH)], FONT_WIDTH);				
			/*for(j=0 ; j < FONT_WIDTH ; j++){
				oledSendData(font_table[cIndex + j + (k*FONT_WIDTH) ]);
			}*/
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
	unsigned int count = 0;
	unsigned char status ;
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
	
	mf522_spi.interface_type = BITBANG ;
	mf522_spi.csPin = 3;
	mf522_spi.misoPin = 1;
	mf522_spi.mosiPin = 2;
	mf522_spi.clkPin = 0;
	setupSpiSlave(&mf522_spi);
	P1DIR |= BIT0;
	P2DIR |= BIT4; // reset pin of mf522
	P2OUT |= BIT4;
	MFRC522_Init(&mf522_spi);
	initi2c(40);
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
	
	P1OUT &= ~BIT0 ;
	count = 0 ;
	while(1){
		__delay_cycles(100000);
		__delay_cycles(100000);
		__delay_cycles(100000);		
		status = MFRC522_Request(PICC_REQIDL, mf522_buffer);	
			
		if (status == MI_OK)
		{
			P1OUT |= BIT0 ;
			sprintf(pageBuffer, "Card nb :%d%d \n",mf522_buffer[0], mf522_buffer[1] ); 
			count = 50 ;                  
		}else{
			P1OUT &= ~BIT0 ;
			if(count == 0){
				sprintf(pageBuffer, msg1 );
			}else{
				count -- ;
			}  			
		}
		print_screen(7, 0, pageBuffer);		
	}
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



