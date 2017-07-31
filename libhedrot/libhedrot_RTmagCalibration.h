//
//  libhedrot_RTmagCalibration.h
//  hedrot_receiver
//
//  Created by Alexis Baskind on 06/05/17.
//
//  Part of code is derived from Matthieu Aussal, CMAP - Ecole Polytechnique / CNRS
//


#ifndef __hedrot_receiver__libhedrot_RTmagCalibration__
#define __hedrot_receiver__libhedrot_RTmagCalibration__

#include <stdio.h>
#include <stdlib.h>
#define _USE_MATH_DEFINES // for C+Windows
#include <math.h>
#include "libhedrot_utils.h"
#include "libhedrot_calibration.h"

#define M_GOLDEN_RATIO 1.61803398874989484820 // (1+sqrt(5))/2
#define M_INV_GOLDEN_RATIO 0.61803398874989484820 // 1/M_GOLDEN_RATIO = M_GOLDEN_RATIO - 1

#define RT_CAL_NUMBER_OF_ZONES 100 // number of zones in the unit sphere (defined by Fibonaci mapping)
#define RT_CAL_NUMBER_OF_POINTS_PER_ZONE 100 // number of points per zone

#define MAX_CONDITION_NUMBER_REALTIME 100000

#define TIME_CONSTANT_INDICATOR_OF_REJECTED_POINTS 1600 // in samples
#define MAX_ALLOWED_PROPORTION_OF_REJECTED_POINTS .5

#define MAX_ALLOWED_STANDARD_DEVIATION_ERROR .1

#define UPPER_NORM_FOR_ABSURD_VALUES 1.5 // for method RTmagCalibrationUpdateIterative
#define UPPER_NORM2_FOR_ABSURD_VALUES UPPER_NORM_FOR_ABSURD_VALUES*UPPER_NORM_FOR_ABSURD_VALUES // for method RTmagCalibrationUpdateIterative

//=====================================================================================================
// structure definition: RTmagZoneData (structure that stores information per zone)
//=====================================================================================================
typedef struct _RTmagZoneData {
    short               points[RT_CAL_NUMBER_OF_POINTS_PER_ZONE][3];
    double              averagePoint[3];
    short               numberOfPoints;
    short               indexOfCurrentPoint;
} RTmagZoneData;

//=====================================================================================================
// structure definition: RTmagCalData (structure that stores RT calibration data)
//=====================================================================================================
typedef struct _RTmagCalData {
    // parameters
    short               calibrationRateFactor; // calibration rate
    char                calibrationValid;       // 0 => step 1 ("brute force" method), 1=> step 2 (with Fibonacci mapping)
    float               RTmagMaxDistanceError;
    
    // for step 1 (with raw samples)
    long                maxNumberOfSamples;
    long                sampleIndexStep1; // internal
    
    // for step 2 (with Fibonacci mapping)
    float               Fibonacci_Points[RT_CAL_NUMBER_OF_ZONES][3];
    RTmagZoneData       *zoneData;
    float               RTmagMinDistance2; //minimum squared distance error to accept or not a new point
    float               RTmagMaxDistance2; //maximum squared distance error to accept or not a new point
    short               numberOfFilledZones;
    
    // estimated values
    float               estimatedOffset[3];
    float               estimatedScaling[3], estimatedScalingFactor[3];
    
    // container for calibrating with ellipsoid fit and displaying purposes
    calibrationData*    calData;
    
    // temporary container for storing data before filtering
    calibrationData*    TMPcalData;
    
    // internal data
    short               calibrationRateCounter;
    float               proportionOfRejectedPoints_State;
    float               proportionOfRejectedPoints_LPcoeff; // coefficient alpha of the low pass filter estimating the proportion of rejected points
    float               previousEstimatedOffset[3], previousEstimatedScaling[3];
    float               previousConditionNumber;
} RTmagCalData;

//=====================================================================================================
// functions
//=====================================================================================================
RTmagCalData* newRTmagCalData();
void freeRTmagCalData(RTmagCalData* data);
void initRTmagCalData(RTmagCalData* data, float* initalEstimatedOffset, float* initalEstimatedScaling, float RTmagMaxDistanceError, short calibrationRateFactor, long maxNumberOfSamples);

void RTmagCalibration_setCalibrationRateFactor(RTmagCalData* data, short calibrationRateFactor);

void RTmagCalibration_setmaxNumberOfSamples(RTmagCalData* data, long maxNumberOfSamples);

void RTmagCalibration_setRTmagMaxDistanceError(RTmagCalData* data, float RTmagMaxDistanceError);

void computeFibonnaciMapping( RTmagCalData* data);
short getClosestFibonacciPoint( RTmagCalData* data, double calPoint[3]);
void addPoint2FibonnaciZone( RTmagCalData* data, short zoneNumber, short point[3]);

short RTmagCalibrationUpdateDirect( RTmagCalData* data, short rawPoint[3]);
short RTmagCalibrationUpdateIterative( RTmagCalData* data, short rawPoint[3]);

#endif /* defined(__hedrot_receiver__libhedrot_RTmagCalibration__) */
