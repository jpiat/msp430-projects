#include <msp430g2553.h>

#ifndef SPI_H
#define SPI_H

typedef struct{
	unsigned char portNb ;
	unsigned char csPin ;
	unsigned char speed ; // Mhz
}spi_slave ;


void setup_spi_slave(spi_slave * slave);

void txrx_slave(spi_slave * slave, unsigned char * send_buffer, unsigned char * receive_buffer , unsigned char length);

#endif