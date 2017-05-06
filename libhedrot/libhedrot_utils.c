//
//  libhedrot_utils.c
//  hedrot_receiver
//
//  Created by Alexis Baskind on 17/04/17.
//
//

#include "libhedrot_utils.h"


//=====================================================================================================
// utils
//=====================================================================================================

// double floating point modulo
double mod(double a, double N) {return a - N*floor(a/N);} //return in range [0, N]


//---------------------------------------------------------------------------------------------------
// Fast inverse square-root

// single precision version
// See: http://en.wikipedia.org/wiki/Fast_inverse_square_root
float invSqrt(float x) {
    float halfx = 0.5f * x;
    float y = x;
    int32_t i = *(int32_t*)&y;
    i = 0x5f3759df - (i>>1);
    y = *(float*)&i;
    y = y * (1.5f - (halfx * y * y));
    return y;
}


/* double precision version
 // See: https://tbach.web.cern.ch/tbach/thesis/literature/fastinvsquare_robertson.pdf
 double invSqrt(double x) {
 uint64_t i;
 double x2, y;
 x2 = x * 0.5;
 y = x;
 i = *(uint64_t *) &y;
 i = 0x5fe6eb50c7b537a9 - (i >> 1);
 y = *(double *) &i;
 y = y * (1.5 - (x2 * y * y));
 return y;
 }
 */

/*
 // not optimized (but more precise) version
 double invSqrt(double x) {
 if(x==0) return 1e40; // arbitrary very big number
 else return 1./sqrt(x);
 }*/

void quaternion2YawPitchRoll(float q1, float q2, float q3, float q4, float *yaw, float *pitch, float *roll) {
    // zyx Talt-Bryan rotation sequence
    
    *yaw = (float) (RAD_TO_DEGREE * atan2(2.0f * (q1*q4 + q2*q3),
                                          1.0f - 2.0f * (q3*q3 + q4*q4) ));
    
    *pitch = (float) (RAD_TO_DEGREE * asin(min(max(2.0f * (q1*q3 - q4*q2),-1),1)));
    
    
    *roll= (float) (RAD_TO_DEGREE * atan2(2.0f * (q1*q2 + q3*q4),
                                          1.0f - 2.0f * (q2*q2 + q3*q3)));
}

void quaternion2RollPitchYaw(float q1, float q2, float q3, float q4, float *yaw, float *pitch, float *roll) {
    // xyz Talt-Bryan rotation sequence
    
    *roll = (float) (RAD_TO_DEGREE * atan2(2.0f * (q1*q2 - q3*q4),
                                           1.0f - 2.0f * (q2*q2 + q3*q3) ));
    
    *pitch = (float) (RAD_TO_DEGREE * asin(min(max(2.0f * (q1*q3 + q4*q2),-1),1)));
    
    
    *yaw= (float) (RAD_TO_DEGREE * atan2(2.0f * (q1*q4 - q2*q3),
                                         1.0f - 2.0f * (q3*q3 + q4*q4)));
}

void quaternionComposition(float q01, float q02, float q03, float q04, float q11, float q12, float q13, float q14, float *q21, float *q22, float *q23, float *q24) {
    // compose two quaternions (Hamilton product)
    // the quaternion (q11, q12, q13, q14) is rotated according to the reference quaternion (q01, q02, q03, q04)
    *q21 = q01 * q11 - q02 * q12 - q03 * q13 - q04 * q14;
    *q22 = q01 * q12 + q02 * q11 + q03 * q14 - q04 * q13;
    *q23 = q01 * q13 - q02 * q14 + q03 * q11 + q04 * q12;
    *q24 = q01 * q14 + q02 * q13 - q03 * q12 + q04 * q11;
}

int stringToFloats(char* valueBuffer, float* data, int nvalues) {
    int i=0;
    
    char *delim = " "; // input separated by spaces
    char *token = NULL;
    char *brkt;
    
    for (token = strtok_r(valueBuffer, delim, &brkt); token != NULL; token = strtok_r(NULL, delim, &brkt))
    {
        char *unconverted;
        data[i] = (float) strtod(token, &unconverted);
        i++;
    }
    
    if(i<nvalues-1) {
        return 0; //error
    } else {
        return 1; //ok
    }
}


int stringToChars(char* valueBuffer, char* data, int nvalues) {
    int i=0;
    
    char *delim = " "; // input separated by spaces
    char *token = NULL;
    char *brkt;
    
    for (token = strtok_r(valueBuffer, delim, &brkt); token != NULL; token = strtok_r(NULL, delim, &brkt))
    {
        char *unconverted;
        data[i] = (char) strtol(token, &unconverted,10);
        i++;
    }
    
    if(i<nvalues-1) {
        return 0; //error
    } else {
        return 1; //ok
    }
}


//=====================================================================================================
// function getMean3
//=====================================================================================================
//
// get average point in a set of data (short format) over 3 dimensions
// careful: memory for double* rawMean has to be allocated first!!
void getMean3(double *samples, long numberOfSamples, float* mean) {
    int n;
    
    // initialization
    mean[0] = 0;
    mean[1] = 0;
    mean[2] = 0;
    n = 3*numberOfSamples;
    
    while(n--) {
        mean[0] += *samples++;
        mean[1] += *samples++;
        mean[2] += *samples++;
    }
    
    // normalization
    mean[0] /= numberOfSamples;
    mean[1] /= numberOfSamples;
    mean[2] /= numberOfSamples;
}


//=====================================================================================================
// function getMean1f
//=====================================================================================================
//
// get average point in a set of data (float format), 1 dimension
// careful: memory for double* rawMean has to be allocated first!!
float getMean1f(float *samples, long numberOfSamples) {
    int n;
    float mean = 0;

    n = numberOfSamples;
    while(n--)
        mean += *samples++;
    
    // normalization
    return (mean / numberOfSamples);
}


//=====================================================================================================
// function getStdDev1f
//=====================================================================================================
//
// get variance (centered moment 2nd order, square of the std deviation) per axis in a set of data (double version)
float getStdDev1f(float *samples, long numberOfSamples, float mean) {
    int n;
    double TMPval;
    float var = 0;

    n = numberOfSamples;
    while(n--) {
        TMPval = *samples++ - mean;
        var += TMPval*TMPval;
    }

    // normalization and square root
    return sqrt(var/numberOfSamples);
}

