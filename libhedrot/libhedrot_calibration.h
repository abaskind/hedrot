//
//  libhedrot_calibration.h
//  hedrot_receiver
//
//  Created by Alexis Baskind on 17/04/17.
//
//  Part of code is derived from Matthieu Aussal, CMAP - Ecole Polytechnique / CNRS
//



#ifndef __hedrot_receiver__libhedrot_calibration__
#define __hedrot_receiver__libhedrot_calibration__

// constants
#define MAX_NUMBER_OF_SAMPLES_FOR_CALIBRATION 100000
#define MAX_CONDITION_NUMBER_OFFLINE 100000
#define NORM_ERROR_TOLERANCE .05 // during calibration, all samples which norm is outside (1 +/- NORM_ERROR_TOLERANCE) after first pass of calibration are filtered out for the second pass

#define MAX_ALLOWED_OFFSET 10000
#define MAX_ALLOWED_SCALING 10000

//=====================================================================================================
// structure definition: temporary calibration data for magnetometer and accelerometer
//=====================================================================================================

typedef struct _calibrationData {
    long            numberOfSamples;
    double          rawSamples[MAX_NUMBER_OF_SAMPLES_FOR_CALIBRATION][3];  // raw samples
    double          calSamples[MAX_NUMBER_OF_SAMPLES_FOR_CALIBRATION][3];  // calibrated samples
    float           dataNorm[MAX_NUMBER_OF_SAMPLES_FOR_CALIBRATION];       // vector norm after calibration
    float           conditionNumber;
    float           normAverage;                                           // average norm to the center
    float           normStdDev;                                            // standard deviation of the norm to the center
    float           maxNormError;                                          // maximum norm error (absolute value of norm minus averageNorm)
} calibrationData;


int accMagCalibration(calibrationData* calData, float* estimatedOffset, float* estimatedScaling);

int myCalibration1(calibrationData* calData, float* estimatedOffset, float* estimatedScaling);

int nonRotatedEllipsoidFit(calibrationData* calData, float* estimatedOffset, float* estimatedScaling, double *quadricCoefficients, double maxConditionNumber);
int rotatedEllipsoidFit(calibrationData* calData, double *quadricCoefficients, double maxConditionNumber);

int filterCalData(calibrationData *inCalData, calibrationData *outCalData, float center[3]);

void cookCalibrationData(calibrationData* calData, float* estimatedOffset, float* estimatedScaling);
void computeCalNormStatistics(calibrationData* calData, float* estimatedOffset, float* estimatedScaling, float* normAverage, float* normStdDev);


#endif /* defined(__hedrot_receiver__libhedrot_calibration__) */
