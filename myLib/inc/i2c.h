#include <msp430g2553.h>


#ifndef I2C_H
#define I2C_H
void initi2c(void);
void writei2c(unsigned char addr, unsigned char * data, unsigned char nbData);
unsigned char readi2c(unsigned char addr);
inline void i2cTxInterruptService(void);
inline void i2cRxInterruptService(void);

#endif
