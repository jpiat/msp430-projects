#include <msp430.h>


#ifndef I2C_H
#define I2C_H
void initi2c(unsigned int divider);
char writei2c(unsigned char addr, unsigned char * data, unsigned char nbData);
char readi2c(unsigned char addr, unsigned char * data, unsigned char nbData);
inline void i2cDataInterruptService(void);
inline void i2cErrorInterruptService(void);

#endif
