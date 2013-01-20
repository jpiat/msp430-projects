
#include "fifo.h"

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
