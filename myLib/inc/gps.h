#ifndef _GPS_
#define _GPS_
#include  <msp430g2553.h>

struct gps_fix {
	unsigned char valid ;
	float time ;
	float latitude ;
	float longitude ;
	float speed ;
	float course;
	int date;
	float magn_var;
	unsigned char checksum;
} ;

extern void new_fix(struct gps_fix fix);

int parseGPS(char c, struct gps_fix * dataP);
void distance_from (float lat2, const float long2, float * distance, int * bearing);
void init_gps(void);

#endif
