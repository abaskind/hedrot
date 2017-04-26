//
//  libhedrot_calibration.h
//  hedrot_receiver
//
//  Created by Alexis Baskind on 17/04/17.
//
//

#ifndef __hedrot_receiver__libhedrot_calibration__
#define __hedrot_receiver__libhedrot_calibration__

#include <stdio.h>

// constants
#define MAX_NUMBER_OF_SAMPLES_FOR_CALIBRATION 100000
#define MAX_CONDITION_NUMBER 1000
#define NORM_ERROR_TOLERANCE .05 // during calibration, all samples which norm is outside (1 +/- NORM_ERROR_TOLERANCE) after first pass of calibration are filtered out for the second pass

//=====================================================================================================
// structure definition: temporary calibration data for magnetometer and accelerometer
//=====================================================================================================

typedef struct _calibrationData {
    long            numberOfSamples;
    short           rawSamples[MAX_NUMBER_OF_SAMPLES_FOR_CALIBRATION][3];  // raw samples
    float           calSamples[MAX_NUMBER_OF_SAMPLES_FOR_CALIBRATION][3];  // calibrated samples
    float           dataNorm[MAX_NUMBER_OF_SAMPLES_FOR_CALIBRATION];       // vector norm after calibration
    float           conditionNumber;
    float           maxNormError;                                          // maximum norm error (absolute value of norm minus 1)
} calibrationData;


int accMagCalibration(calibrationData* calData, float* estimatedOffset, float* estimatedScaling);

int ellipsoidFit(calibrationData* calData, float* estimatedOffset, float* estimatedScaling);

void cookCalibrationData(calibrationData* calData, float* estimatedOffset, float* estimatedScaling);


#endif /* defined(__hedrot_receiver__libhedrot_calibration__) */
