#include <math.h>
#include "gps.h"

//$GPRMC,235502.475,A,4338.6080,N,00126.7317,E,1.25,63.77,171111,,,A*5B
#define PI 3.14159265358f
#define PREC 5

unsigned int i = 0;
unsigned int field = 0;
struct gps_fix * pfix;

float mySin(float x);
float myCos(float x);
float mySqrt(float number);
float radians(float x);
float sq(float x);
void distance_between(float lat1, const float long1, float lat2,
		const float long2, float * distance, int * bearing);

#define PI_FLOAT     3.14159265f
#define PIBY2_FLOAT  1.5707963f
// |error| < 0.005


enum gps_parser_state {
	SYNC,
	HEADER,
	TIME,
	VALID,
	LAT,
	LAT_DIR,
	LONG,
	LONG_DIR,
	SPEED,
	COURSE,
	DATE,
	MAGN,
	MAGN_DIR,
	CHECKSUM,
};
enum gps_parser_state parserState;
unsigned int counter;
unsigned char frame_checksum;
char buffer[12];



float fast_atan2f(float y, float x) {
	if (x == 0.0f) {
		if (y > 0.0f)
			return PIBY2_FLOAT;
		if (y == 0.0f)
			return 0.0f;
		return -PIBY2_FLOAT;
	}
	float atan;
	float z = y / x;
	if (fabsf(z) < 1.0f) {
		atan = z / (1.0f + 0.28f * z * z);
		if (x < 0.0f) {
			if (y < 0.0f)
				return atan - PI_FLOAT;
			return atan + PI_FLOAT;
		}
	} else {
		atan = PIBY2_FLOAT - z / (z * z + 0.28f);
		if (y < 0.0f)
			return atan - PI_FLOAT;
	}
	return atan;
}

float mySin(const float x) {
	unsigned char i;
	float denum = 1;
	float res = 0;
	float x_2 = x * x;
	float num = x;
	int s = 1;
	for (i = 0; i < PREC; i++) {
		res += s * (num / denum);
		denum = denum * (denum + 1) * (denum + 2);
		num = num * x_2;
		s = s * -1;
	}
	return res;
	/*
	 float x_3 = x * x_2;
	 float x_5 = x_3 * x_2;
	 float x_7 = x_5 * x_2;
	 float res = (x - x_3/6.0 + x_5/120.0 - x_7/5040.0 );
	 return res;//+ x_9/362880.0); */
}

float myCos(const float x) {
	unsigned char i;
	float denum = 2;
	float res = 1;
	float x_2 = x * x;
	float num = x_2;
	int s = -1;
	for (i = 0; i < PREC; i++) {
		res += s * (num / denum);
		denum = denum * (denum + 1) * (denum + 2);
		num = num * x_2;
		s = s * -1;
	}
	return res;
	/*

	 float x_2 = x * x;
	 float x_4 = x_2 * x_2;
	 float x_6 = x_4 * x_2;
	 float res = 1 - x_2 / 2.0 + x_4 / 24.0 - x_6 / 720.0 ;
	 //float x_8 = x_6 * x_2;
	 return res ;//+ x_8/40320.0;
	 */
}

float mySqrt(const float number) {
	unsigned char i = 0;
	float x0, sqx0, error;
	if (number < 1) {
		x0 = number * 2;
	} else {
		x0 = number / 2;
	}
	do {
		x0 = (x0 + (number / x0)) / 2;
		sqx0 = x0 * x0;
		error = (number - sqx0) / number;
		i++;
	} while (i < 20
			&& ((error > 0 && error > 0.01) || (error < 0 && error < -0.01)));

	return x0;
}

float radians(const float x) {
	return PI * x / 180.0f;
}

float sq(const float x) {
	return x * x;
}

void distance_between2(float lat1, const float long1, float lat2,
		const float long2, float * distance, float * bearing) {
	//courtesy of http://www.movable-type.co.uk/scripts/latlong.html
	float dLat = radians(lat1 - lat2);
	float dLong = radians(long1 - long2);
	float sindLong = mySin(dLong / 2);
	float sindLat = mySin(dLat / 2);
	lat1 = radians(lat1);
	lat2 = radians(lat2);
	float cosLat1 = myCos(lat1);
	float cosLat2 = myCos(lat2);
	float a = (sindLat * sindLat) + (sindLong * sindLong * cosLat1 * cosLat2);
	float sa = mySqrt(a);
	float c = 2 * fast_atan2f(mySqrt(a), mySqrt(1 - a));
	*distance = c * 6372.795;
}

void distance_between(float lat1, const float long1, float lat2,
		const float long2, float * distance, int * bearing) {
	//courtesy of http://arduiniana.org/libraries/tinygps/
	float delta = radians(long1 - long2);
	float sdlong = mySin(delta);
	float cdlong = myCos(delta);
	lat1 = radians(lat1);
	lat2 = radians(lat2);
	float slat1 = mySin(lat1);
	float clat1 = myCos(lat1);
	float slat2 = mySin(lat2);
	float clat2 = myCos(lat2);
	delta = (clat1 * slat2) - (slat1 * clat2 * cdlong);
	float x = delta ;
	float y = sdlong * clat2;
	delta = sq(delta);
	delta += sq(clat2 * sdlong);
	delta = mySqrt(delta);
	float denom = (slat1 * slat2) + (clat1 * clat2 * cdlong);
	delta = fast_atan2f(delta, denom);
	*distance =  delta * 6372.795;
	x = (180.0 * (fast_atan2f(y, x)/PI)) ;
	*bearing = ((int) -x + 360)%360 ;
}

void distance_from(float lat2, const float long2, float * distance, int * bearing) {
	 distance_between(pfix->latitude, pfix->longitude, lat2, long2, distance, bearing);
}


double myAtoF(char * buffer){
	double result = 0 ;
	unsigned char decPos = 0;
	unsigned char i = 0;
	while(buffer[i] != '\0'){
		if(buffer[i] == '.'){
			decPos = i ;
			i ++ ;
			continue ;
		}
		result = result * 10 ;
		result += buffer[i] - 48 ;
		i ++ ;
	}
	for(; decPos < (i-1) ; decPos ++){
		result = result/10 ;
	}
	return result ;
}

int myAtoI(char * buffer){
		int result = 0 ;
		unsigned char i = 0;
		while(buffer[i] != '\0'){
			if(buffer[i] == '.') return result ;
			result = result * 10 ;
			result += buffer[i] - 48 ;
			i ++ ;
		}
		return result ;
}

unsigned char asciiToHex(char * buffer){
	unsigned char result = 0 ;
	unsigned char i = 0;
	while(buffer[i] != '\0'){
		result = (result << 4) ;
		if(buffer[i] < 'A'){
			result += buffer[i] - 48 ;
		}else{
			result += buffer[i] - ('A' - 10) ;
		}
		i ++ ;
	}
	return result ;
}

char parseIntField(char c, char * buffer, unsigned int count, int * val) {
	buffer[count] = c;
	if (c == ',') {
		buffer[count] = '\0';
		*val = myAtoI(buffer);
		if (*val < 0) {
			return -1;
		} else {
			return 1;
		}
	}
	return 0;
}

char parseFloatField(char c, char * buffer, unsigned int count, float * val) {
	buffer[count] = c;
	if (c == ',') {
		buffer[count] = '\0';
		*val = myAtoF(buffer);
		if (*val < 0) {
			return -1;
		} else {
			return 1;
		}
	}
	return 0;
}

int parseGPS(char c, struct gps_data * dataP) {
	unsigned char ret;
	frame_checksum ^= c;
	switch (parserState) {
	case SYNC:
		counter = 0;
		if (c == '$') {
			parserState = HEADER;
			frame_checksum = 0;
		}
		break;
	case HEADER:
		if (c == ',') {
			parserState = TIME;
			counter = 0;
		} else if (c != header[counter]) {
			parserState = SYNC;
		} else {
			counter++;
		}
		break;
	case TIME:
		ret = parseIntField(c, buffer, counter, &dataP->time);
		if (ret == 1) {
			counter = 0;
			parserState = VALID;
			return 0;
		} else if (ret == -1) {
			counter = 0;
			parserState = SYNC;
			return 0;

		} else {
			counter++;
		}
		break;
	case VALID:
		if (c == ',') {
			parserState = LAT;
		} else if (c == 'A') {
			dataP->valid = 1;
		} else {
			dataP->valid = -1;
		}
		break;
	case LAT:
		ret = parseFloatField(c, buffer, counter, &dataP->lat);
		if (ret == 1) {
			counter = 0;
			parserState = LAT_DIR;
			return 0;
		} else if (ret == -1) {
			counter = 0;
			parserState = SYNC;
			return 0;

		} else {
			counter++;
		}
		break;
	case LAT_DIR:
		if (c == ',') {
			parserState = LONG;
		} else if (c == 'W') {
			dataP->lat = -dataP->lat;
		}
		break;
	case LONG:
		ret = parseFloatField(c, buffer, counter, &dataP->lng);
		if (ret == 1) {
			counter = 0;
			parserState = LONG_DIR;
			return 0;
		} else if (ret == -1) {
			counter = 0;
			parserState = SYNC;
			return 0;

		} else {
			counter++;
		}
		break;
	case LONG_DIR:
		if (c == ',') {
			parserState = SPEED;
		} else if (c == 'S') {
			dataP->lng = -dataP->lng;
		}
		break;
	case SPEED:
		ret = parseFloatField(c, buffer, counter, &dataP->speed);
		if (ret == 1) {
			counter = 0;
			parserState = COURSE;
			return 0;
		} else if (ret == -1) {
			counter = 0;
			parserState = SYNC;
			return 0;
		} else {
			counter++;
		}
		break;
	case COURSE:
		ret = parseFloatField(c, buffer, counter, &dataP->course);
		if (ret == 1) {
			counter = 0;
			parserState = DATE;
			return 0;
		} else if (ret == -1) {
			counter = 0;
			parserState = SYNC;
			return 0;
		} else {
			counter++;
		}
		break;
	case DATE:
		ret = parseIntField(c, buffer, counter, &dataP->date);
		if (ret == 1) {
			counter = 0;
			parserState = MAGN;
			return 0;
		} else if (ret == -1) {
			counter = 0;
			parserState = SYNC;
			return 0;
		} else {
			counter++;
		}
		break;
	case MAGN:
		ret = parseFloatField(c, buffer, counter, &dataP->magn_var);
		if (ret == 1) {
			counter = 0;
			parserState = MAGN_DIR;
			return 0;
		} else if (ret == -1) {
			counter = 0;
			parserState = SYNC;
			return 0;
		} else {
			counter++;
		}
		break;
	case MAGN_DIR:
		if (c == '*') {
			parserState = CHECKSUM;
		} else if (c == 'W') {
			dataP->magn_var = -dataP->magn_var;
		} else {
			dataP->checksum = frame_checksum;
		}
		break;
	case CHECKSUM:
		buffer[counter] = c;
		if (c == '\n') {
			unsigned char sum;
			buffer[counter] = '\0';
			sum = asciiToHex(buffer);
			if (sum != dataP->checksum) {
				dataP->valid = -1;
			}
			gps_callback(dataP);
			parserState = SYNC;
			return 1;

		} else {
			counter++;
		}
		break;
	default:
		break;

	}
	return 0;
}







