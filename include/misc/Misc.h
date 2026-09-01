#ifndef MISC_H_
#define MISC_H_

#define _USE_MATH_DEFINES
#include <cmath>

inline float radian2degrees(float radian)
{
	return radian * 180.0f / M_PI;
}

inline float degrees2radian(float degrees)
{
	return M_PI * degrees / 180.0f;
}

inline int power_of_two(int input)
{
	int value = 1;

	while ( value < input ) {
		value <<= 1;
	}
	return value;
}


#endif // MISC_H_
