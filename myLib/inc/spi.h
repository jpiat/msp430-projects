#include <msp430g2553.h>

#ifndef SPI_H
#define SPI_H

typedef struct{
	unsigned char portNb ;
	unsigned char csPin ;
	unsigned char speed ; // Mhz
}spi_slave ;


void setupSpiSlave(spi_slave * slave);
void spiClearCs(spi_slave * slave);
void spiSetCs(spi_slave * slave);
unsigned char writeByte(unsigned char val);
void spiTxRxSlave(spi_slave * slave, unsigned char * send_buffer, unsigned char * receive_buffer , unsigned char length);

#endif
