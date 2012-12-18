/*
 * display.c
 *
 *  Created on: 3 déc. 2011
 *      Author: jpiat
 */
#include <stdarg.h>
#include "display.h"

#define LINE_JUMP 0

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
	for (div = 1000; div >= 1; div = div / 10) {
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
	while(length < 16 && txt[length] != 0) {
		if(txt[length] =='\n') {
			while(wr_index < LINE_JUMP) {
				buffer[wr_index] = ' ';
				wr_index ++;
			}
			length ++;
		}
		if(txt[length] =='%') {
			int nb;
			double fb;
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
	va_end(arguments);
	return wr_index;
}

