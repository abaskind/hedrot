//
//  libhedrot_utils.h
//  hedrot_receiver
//
//  Created by Alexis Baskind on 17/04/17.
//
//

#ifndef __hedrot_receiver__libhedrot_utils__
#define __hedrot_receiver__libhedrot_utils__

#include <stdio.h>
#include <math.h>

// double floating point modulo
double mod(double a, double N);

// round (not defined in VS2012)
#if defined(_WIN32) || defined(_WIN64)
double round(double value);
#endif /* #if defined(_WIN32) || defined(_WIN64) */

// math utils

// min and max not standard in Mac
#ifdef __MACH__  // if mach (mac os X)
#ifndef min
#define min(a,b) (((a)<(b))?(a):(b))
#endif

#ifndef max
#define max(a,b) (((a)>(b))?(a):(b))
#endif
#endif

#define M_PI_float (float)      3.14159265358979323846264338327950288
#define	DEGREE_TO_RAD           M_PI_float / 180.0f
#define	RAD_TO_DEGREE           180.0f / M_PI_float

#ifndef sign
#define sign(a) (((a)>=0.0)*2-1)
#endif

// prototypes
#if defined(_WIN32) || defined(_WIN64)
# define strtok_r strtok_s // strtok_r does not exist on windows, use strtok_s instead
#endif /* #if defined(_WIN32) || defined(_WIN64) */

//=====================================================================================================
// utils
//=====================================================================================================
double mod(double a, double N);
float invSqrt(float x);
void quaternion2YawPitchRoll(float q1, float q2, float q3, float q4, float *yaw, float *pitch, float *roll);
void quaternion2RollPitchYaw(float q1, float q2, float q3, float q4, float *yaw, float *pitch, float *roll);
void quaternionComposition(float q01, float q02, float q03, float q04, float q11, float q12, float q13, float q14, float *q21, float *q22, float *q23, float *q24);
int stringToFloats(char* valueBuffer, float* data, int nvalues);
int stringToChars(char* valueBuffer, char* data, int nvalues);

void getMean3(double *samples, long numberOfSamples, float* mean);
float getMean1f(float *samples, long numberOfSamples);
float getStdDev1f(float *samples, long numberOfSamples, float mean);


#endif /* defined(__hedrot_receiver__libhedrot_utils__) */
