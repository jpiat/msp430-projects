#ifndef _GPS_
#define _GPS_
#include  <msp430g2553.h>
#include <stdlib.h>

#define FIFO_SIZE 64
#define FIFO_DIST 10

struct my_fifo{
	unsigned char buffer [FIFO_SIZE] ;
	unsigned int write_index ;
	unsigned int read_index	 ;
	unsigned int distance ;
};

struct gps_fix {
	unsigned char valid ;
	float time ;
	float latitude ;
	float longitude ;
	float speed ;
	float date ;
} ;


extern struct my_fifo gps_fifo ;

extern void new_fix(struct gps_fix fix);

void parseStream(struct my_fifo * fif);
void distance_from (float lat2, const float long2, float * distance, int * bearing);
void init_gps(void);

inline unsigned char fifo_available(struct my_fifo * const fif) {
	return fif->distance > FIFO_DIST;
}



inline unsigned char fifo_peek(struct my_fifo * const fif) {
	while (fif->distance < FIFO_DIST);
	unsigned char c = fif->buffer[fif->read_index];
	return c;
}

inline void fifo_inc(struct my_fifo * const fif) {
	fif->read_index = (fif->read_index + 1) % FIFO_SIZE;
	fif->distance--;
}

inline unsigned char fifo_read(struct my_fifo * const fif) {
	while (fif->distance < FIFO_DIST);
	unsigned char c = fif->buffer[fif->read_index];
	fif->read_index = (fif->read_index + 1) % FIFO_SIZE;
	fif->distance--;
	return c;
}

inline void fifo_write(struct my_fifo * const fif, const unsigned char c) {
	if (fif->distance < FIFO_SIZE) {
		fif->buffer[fif->write_index] = c;
		fif->write_index = (fif->write_index + 1) % FIFO_SIZE;
		fif->distance ++;
	}
}

#endif
