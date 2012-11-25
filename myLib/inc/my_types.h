
#ifndef MY_TYPES_H
#define MY_TYPES_H

#define NULL ((void *) 0)

#define TRUE	1
#define FLASE 	0


typedef unsigned char uchar ;
typedef unsigned int uint ;


typedef struct{
	uchar * buffer ; // less than 256 bytes
	uchar * rP ;
	uchar * wP ;
}CHAR_FIFO;
#endif

