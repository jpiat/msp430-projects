#include <msp430g2553.h>
#include "my_types.h"
#include "i2c.h"
#include "uart.h"
#include <legacymsp430.h>
#include <stdarg.h>
#include <math.h>

#define MAX_LENGTH	40
#define LINE_JUMP	100

#define HMC5883L_ADDR	0x1E

const uchar setup_hmc5883[] = {0x00, 0x70};
const uchar gain_hmc5883[] = {0x01, 0xA0};
const uchar continuous_hmc5883[] = {0x02, 0x00};
const uchar base_hmc5883[] = {0x03};

uchar readBuffer [6];

uchar printBuffer [25];




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
        for (div = 1000; div >= 0.001 && i < 8; div = div / 10) {
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







void uart_rx(unsigned char byte){
	return ;
}

int main(){
	unsigned long int i ;
	unsigned int count = 0;
	unsigned char status ;
	int x, y, z ; 
	float heading ;
	float headingDegrees ;
	WDTCTL = WDTPW + WDTHOLD ;
	BCSCTL1 = CALBC1_16MHZ ;
	DCOCTL = CALDCO_16MHZ ;
	P1DIR |= BIT0 ;
	for(i = 0 ; i < 3000 ; i++){
		nop();
	}
	P1OUT &= ~BIT0 ;
	initi2c(100);
	setup_uart_9600();
	__bis_SR_register(GIE);
	P1OUT |= BIT0 ;
	count = 0 ;
	writei2c(HMC5883L_ADDR, setup_hmc5883, 2);
	writei2c(HMC5883L_ADDR, gain_hmc5883, 2);
	writei2c(HMC5883L_ADDR, continuous_hmc5883, 2);
	while(1){
		writei2c(HMC5883L_ADDR, base_hmc5883, 1);
		__delay_cycles(100000);
		readi2c(HMC5883L_ADDR, readBuffer, 6);
		x = readBuffer[0] ;
		x |= readBuffer[1] << 8 ;
		z = readBuffer[2] ;
		z |= readBuffer[3] << 8 ;
		y = readBuffer[4] ;
		y |= readBuffer[5] << 8 ;	
		heading = atan2(y, x);
		if(heading < 0) heading += 2*M_PI;
		headingDegrees = heading * 180/M_PI;
		count = sprintf(printBuffer, "%d , %d, %d h = %f \n\r", x, y, z, headingDegrees);	
		uart_send_data(printBuffer, count);
	}
}

interrupt(USCIAB0TX_VECTOR) USCI0TX_ISR(void){
	if ((IFG2 & UCB0TXIFG) || (IFG2 & UCB0RXIFG)) {	
		i2cDataInterruptService();
	}
}

interrupt(USCIAB0RX_VECTOR) USCI0RX_ISR(void) {
	if (IFG2 & UCA0RXIFG) {
		UartRxInterruptService();
		IFG2 &= ~UCA0RXIFG;
	}else if(IFG2 & UCB0RXIFG){
		i2cErrorInterruptService();
		IFG2 &= ~UCB0RXIFG;
	}
	
}



