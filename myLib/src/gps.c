#include <math.h>
#include "gps.h"

//$GPRMC,235502.475,A,4338.6080,N,00126.7317,E,1.25,63.77,171111,,,A*5B
#define PI 3.14159265358f
#define PREC 5

struct float_parse {
	float value;
	short int multiply;
	short int inc;
	unsigned int divide;
}* pFloat;

const char * fix_sequence = "$GPRMC,f,%,f,%,f,%,f,f,f,f,f,%*h";

unsigned int i = 0;
unsigned int field = 0;
struct my_fifo gps_fifo;
struct gps_fix * pfix;

float mySin(float x);
float myCos(float x);
float mySqrt(float number);
float radians(float x);
float sq(float x);
void distance_between(float lat1, const float long1, float lat2,
		const float long2, float * distance, int * bearing);
int isFloat(unsigned char c);
int isInteger(unsigned char c);
int isUpperCase(unsigned char c);
void init_float(struct float_parse * myFloat);
float addDigit(struct float_parse * myFloat, unsigned char digit);
void addToField(unsigned int fieldNumber, unsigned char c);
void init_gps();
void init_fix(struct gps_fix * fix_pointer);
void parseStream(struct my_fifo * fif);

#define PI_FLOAT     3.14159265f
#define PIBY2_FLOAT  1.5707963f
// |error| < 0.005
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

int isFloat(unsigned char c) {
	return (c > 47 && c < 58) || (c == '.');
}

int isInteger(unsigned char c) {
	return (c > 47 && c < 58);
}

int isHex(unsigned char c) {
	return (c > 47 && c < 58) || (c > 64 && c < 71);
}

int isUpperCase(unsigned char c) {
	return (c > 64 && c < 91);
}

void init_float(struct float_parse * myFloat) {
	myFloat->value = 0.0;
	myFloat->divide = 1;
	myFloat->multiply = 10;
	myFloat->inc = 1;
}

float addDigit(struct float_parse * myFloat, unsigned char digit) {
	if (digit == '.') {
		myFloat->divide = 10;
		myFloat->inc = 10;
		myFloat->multiply = 1;
	} else {
		myFloat->value = myFloat->value * myFloat->multiply;
		myFloat->value += (float) (digit - '0') / (float) myFloat->divide;
		myFloat->divide = myFloat->divide * myFloat->inc;
	}
	return myFloat->value;
}

inline float Dmtod(float Dm) {
	int D = Dm / 100;
	float m = Dm - (D * 100);
	return D + m / 60;
}

void addToField(unsigned int fieldNumber, unsigned char c) {
	if (c == ',') {
		init_float(pFloat);
		return;
	}
	switch (fieldNumber) {
	case 0:
		break;
	case 1:
		pfix->time = addDigit(pFloat, c);
		break;
	case 2:

		if (c == 'A') {
			pfix->valid = 1;
		}
		break;
	case 3:
		pfix->latitude = addDigit(pFloat, c);
		break;
	case 4:
		if (c != 'N') {
			pfix->latitude = -pfix->latitude;
		}
		break;
	case 5:
		pfix->longitude = addDigit(pFloat, c);
		break;
	case 6:
		if (c != 'E') {
			pfix->longitude = -pfix->longitude;
		}
		break;
	case 7:
		pfix->speed = addDigit(pFloat, c);
		break;
	case 8:
		break;
	case 9:
		pfix->date = addDigit(pFloat, c);
		break;
	default:
		break;
	}

}

void init_gps() {
	pFloat = (struct float_parse *) malloc(sizeof(struct float_parse));
	gps_fifo.write_index = FIFO_DIST;
	gps_fifo.read_index = 0;
	gps_fifo.distance = FIFO_DIST;
	pfix = (struct gps_fix *) malloc(sizeof(struct gps_fix));
	init_fix(pfix);
	init_float(pFloat);
}

void init_fix(struct gps_fix * fix_pointer) {
	fix_pointer->latitude = 0.0;
	fix_pointer->longitude = 0.0;
	fix_pointer->latitude = 0.0;
	fix_pointer->longitude = 0.0;
	fix_pointer->valid = 0.0;
}

void parseStream(struct my_fifo * fif) {
	unsigned char new_char;
	unsigned char char_consumed = 1;
	while (fifo_available(fif)) {
		new_char = fifo_peek(fif);
		switch (fix_sequence[i]) {
		case 'f':
			if (isFloat(new_char)) {
				fifo_inc(fif);
				char_consumed = 1;
			} else {
				i++;
			}
			break;
		case ',':
			if (new_char == fix_sequence[i]) {
				char_consumed = 1;
				fifo_inc(fif);
				i++;
				field++;
			} else {
				char_consumed = 1;
				fifo_inc(fif);
				i = 0;
				field = 0;
			}
			break;
		case 'd':
			if (isInteger(new_char)) {
				char_consumed = 1;
				fifo_inc(fif);
			} else {
				i++;
			}
			break;
		case 'h':
			if (isHex(new_char)) {
				char_consumed = 1;
				fifo_inc(fif);
			} else {
				i++;
			}
			break;
		case '%':
			if (isUpperCase(new_char)) {
				char_consumed = 1;
				fifo_inc(fif);
			} else {
				i++;
			}
			break;
		default:
			if (new_char == fix_sequence[i]) {
				char_consumed = 1;
				fifo_inc(fif);
				i++;
			} else {
				char_consumed = 1;
				fifo_inc(fif);
				i = 0;
				field = 0;
			}
			break;
		}
		if (char_consumed) {
			addToField(field, new_char);
		}
		if (fix_sequence[i] == '\0') {
			i = 0;
			field = 0;
			if (pfix->valid) {
				pfix->longitude = Dmtod(pfix->longitude);
				pfix->latitude = Dmtod(pfix->latitude);
				new_fix(*pfix);
				init_fix(pfix);
			}
		}
	}
}

