//
//  libhedrot_RTmagCalibration.h
//  hedrot_receiver
//
//  Created by Alexis Baskind on 06/05/17.
//
//

#ifndef __hedrot_receiver__libhedrot_RTmagCalibration__
#define __hedrot_receiver__libhedrot_RTmagCalibration__

#include <stdio.h>
#include "libhedrot_utils.h"
#include "libhedrot_calibration.h"

#define M_GOLDEN_RATIO 1.61803398874989484820 // (1+sqrt(5))/2
#define M_INV_GOLDEN_RATIO 0.61803398874989484820 // 1/M_GOLDEN_RATIO = M_GOLDEN_RATIO - 1

#define RT_CAL_NUMBER_OF_ZONES 100 // number of zones in the unit sphere (defined by Fibonaci mapping)
#define RT_CAL_NUMBER_OF_POINTS_PER_ZONE 100 // number of points per zone

//=====================================================================================================
// structure definition: RTmagZoneData (structure that stores information per zone)
//=====================================================================================================
typedef struct _RTmagZoneData {
    float points[RT_CAL_NUMBER_OF_POINTS_PER_ZONE][3];
    float averagePoint[3];
    short numberOfPoints;
    short indexOfFirstPoint;
} RTmagZoneData;

//=====================================================================================================
// structure definition: RTmagCalData (structure that stores RT calibration data)
//=====================================================================================================
typedef struct _RTmagCalData {
    // parameters
    float RTmagMinDistance2; //minimum squared distance error to accept or not a new point
    float RTmagMaxDistance2; //minimum squared distance error to accept or not a new point
    
    float Fibonacci_Points[RT_CAL_NUMBER_OF_ZONES][3];
    float estimatedOffset[3];
    float estimatedScaling[3], estimatedScalingFactor[3];
    RTmagZoneData *zoneData;
    
    // internal data
    double *matrixA; // internal input matrix A (in the problem A.X = B)
    float           conditionNumber;
} RTmagCalData;

//=====================================================================================================
// functions
//=====================================================================================================
RTmagCalData* newRTmagCalData(float* initalEstimatedOffset, float* initalEstimatedScaling);
void freeRTmagCalData(RTmagCalData* data);
void initRTmagCalData(RTmagCalData* data, float* initalEstimatedOffset, float* initalEstimatedScaling, float RTmagMaxDistanceError);

void setRTmagMaxDistanceError(RTmagCalData* data, float RTmagMaxDistanceError);

void computeFibonnaciMapping( RTmagCalData* data);
short getClosestFibonacciPoint( RTmagCalData* data, float calPoint[3]);
void addPoint2FibonnaciZone( RTmagCalData* data, short zoneNumber, float point[3]);

short RTmagCalibrationUpdate( RTmagCalData* data, float rawPoint[3]);

#endif /* defined(__hedrot_receiver__libhedrot_RTmagCalibration__) */
