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

//=====================================================================================================
// structure definition: temporary calibration data for magnetometer and accelerometer
//=====================================================================================================

typedef struct _calibrationData {
    long            numberOfSamples;
    short           rawSamples[MAX_NUMBER_OF_SAMPLES_FOR_CALIBRATION][3];  // raw samples
    float           calSamples[MAX_NUMBER_OF_SAMPLES_FOR_CALIBRATION][3];  // calibrated samples
    float           dataNorm[MAX_NUMBER_OF_SAMPLES_FOR_CALIBRATION];       // norm error
    float           conditionNumber;
} calibrationData;


int ellipsoidFit(calibrationData* calData, float* accOffset, float* accScaling);


#endif /* defined(__hedrot_receiver__libhedrot_calibration__) */
