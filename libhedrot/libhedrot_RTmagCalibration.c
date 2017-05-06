//
//  libhedrot_RTmagCalibration.c
//  hedrot_receiver
//
//  Created by Alexis Baskind on 06/05/17.
//
//

#include "libhedrot_RTmagCalibration.h"


RTmagCalData* newRTmagCalData(float* initalEstimatedOffset, float* initalEstimatedScaling) {
    RTmagCalData* data = (RTmagCalData*) malloc(sizeof(RTmagCalData));
    
    data->matrixA = (double*) malloc(RT_CAL_NUMBER_OF_ZONES*6*sizeof(double)); // internal input matrix A
    
    computeFibonnaciMapping(data);
return data;
}

void freeRTmagCalData(RTmagCalData* data) {
    free(data->zoneData);
    free(data->matrixA);
    free(data);
}


void initRTmagCalData(RTmagCalData* data, float* initalEstimatedOffset, float* initalEstimatedScaling, float RTmagMaxDistanceError){
    int i;
    
    // erase (if already exists), allocates and inits zoneData
    if(data->zoneData) free(data->zoneData);
    data->zoneData = (RTmagZoneData*) malloc(RT_CAL_NUMBER_OF_ZONES * sizeof(RTmagZoneData));
    for( i=0;i<RT_CAL_NUMBER_OF_ZONES; i++) {
        data->zoneData[i].numberOfPoints = 0;
        data->zoneData[i].indexOfFirstPoint = 0;
    }
    
    setRTmagMaxDistanceError(data, RTmagMaxDistanceError);
        
    data->estimatedOffset[0] = initalEstimatedOffset[0];
    data->estimatedOffset[1] = initalEstimatedOffset[1];
    data->estimatedOffset[2] = initalEstimatedOffset[2];
    
    data->estimatedScaling[0] = initalEstimatedScaling[0];
    data->estimatedScaling[1] = initalEstimatedScaling[1];
    data->estimatedScaling[2] = initalEstimatedScaling[2];
    
    data->estimatedScalingFactor[0] = 1/data->estimatedScaling[0];
    data->estimatedScalingFactor[1] = 1/data->estimatedScaling[1];
    data->estimatedScalingFactor[2] = 1/data->estimatedScaling[2];
}



void setRTmagMaxDistanceError(RTmagCalData* data, float RTmagMaxDistanceError) {
    data->RTmagMinDistance2 = (1-RTmagMaxDistanceError*RTmagMaxDistanceError)*(1-RTmagMaxDistanceError*RTmagMaxDistanceError);
    data->RTmagMaxDistance2 = (1+RTmagMaxDistanceError*RTmagMaxDistanceError)*(1+RTmagMaxDistanceError*RTmagMaxDistanceError);
}

//=====================================================================================================
// function computeFibonnaciMapping
//=====================================================================================================
//
// compute the cartesian coordinates of a Fibonacci mapping of the unit sphere
//
//
void computeFibonnaciMapping( RTmagCalData* data) {
    short i;
    double theta, sinPhi;
    
    for( i=0; i<RT_CAL_NUMBER_OF_ZONES; i++) {
        theta 	 = 2*M_PI/M_GOLDEN_RATIO*i;
        sinPhi     = 1 - (2*i+1)/RT_CAL_NUMBER_OF_ZONES;
        
        data->Fibonacci_Points[i][0] = cos(theta)*sinPhi;      // x
        data->Fibonacci_Points[i][1] = sin(theta)*sinPhi;      // y
        data->Fibonacci_Points[i][1] = sqrt(1-sinPhi*sinPhi);  // z
    }
}

//=====================================================================================================
// function getClosestFibonacciPoint
//=====================================================================================================
//
// looks for the closest point among the Fibonacci set
//
short getClosestFibonacciPoint( RTmagCalData* data, float calPoint[3]) {
    short i;
    short zoneNumber;
    float dist2;
    float minDistance2;
    
    
    // get the zone number corresponding to the closest point in the Fibonacci set
    zoneNumber = -1;
    minDistance2 = 10e8; // very large number
    for( i=0; i<RT_CAL_NUMBER_OF_ZONES; i++) {
        dist2 = (calPoint[0]-data->Fibonacci_Points[i][0])*(calPoint[0]-data->Fibonacci_Points[i][0])
        + (calPoint[1]-data->Fibonacci_Points[i][1])*(calPoint[1]-data->Fibonacci_Points[i][1])
        + (calPoint[2]-data->Fibonacci_Points[i][2])*(calPoint[2]-data->Fibonacci_Points[i][2]);
        
        if(dist2 < minDistance2) {
            minDistance2 = dist2;
            zoneNumber = i;
        }
    }
    
    return zoneNumber;
}

//=====================================================================================================
// function addPoint2FibonnaciZone
//=====================================================================================================
//
// adds a new point in a given zone
//
void addPoint2FibonnaciZone( RTmagCalData* data, short zoneNumber, float rawPoint[3]) {
    short i;
    short idx;

    // adds the point in the corresponding zone
    data->zoneData[zoneNumber].numberOfPoints++;
    data->zoneData[zoneNumber].numberOfPoints = max(data->zoneData[zoneNumber].numberOfPoints,RT_CAL_NUMBER_OF_POINTS_PER_ZONE); //ring buffer: does not accept more than RT_CAL_NUMBER_OF_POINTS_PER_ZONE points
    
    data->zoneData[zoneNumber].points[data->zoneData[zoneNumber].indexOfFirstPoint][0] = rawPoint[0];
    data->zoneData[zoneNumber].points[data->zoneData[zoneNumber].indexOfFirstPoint][1] = rawPoint[1];
    data->zoneData[zoneNumber].points[data->zoneData[zoneNumber].indexOfFirstPoint][2] = rawPoint[2];
    
    data->zoneData[zoneNumber].indexOfFirstPoint++;
    if(data->zoneData[zoneNumber].indexOfFirstPoint == RT_CAL_NUMBER_OF_POINTS_PER_ZONE)
        data->zoneData[zoneNumber].indexOfFirstPoint = 0; // ring buffer: returns to 0 if overload
    
    // compute new average
    data->zoneData[zoneNumber].averagePoint[0] = 0;
    data->zoneData[zoneNumber].averagePoint[1] = 0;
    data->zoneData[zoneNumber].averagePoint[2] = 0;
    idx = data->zoneData[zoneNumber].indexOfFirstPoint;
    i = data->zoneData[zoneNumber].numberOfPoints;
    while(i--) {
        data->zoneData[zoneNumber].averagePoint[0] += data->zoneData[zoneNumber].points[idx][0];
        data->zoneData[zoneNumber].averagePoint[1] += data->zoneData[zoneNumber].points[idx][1];
        data->zoneData[zoneNumber].averagePoint[2] += data->zoneData[zoneNumber].points[idx][2];
        
        idx++;
        if(idx == RT_CAL_NUMBER_OF_POINTS_PER_ZONE)
            idx = 0; // ring buffer: returns to 0 if overload
    }
    data->zoneData[zoneNumber].averagePoint[0] /= data->zoneData[zoneNumber].numberOfPoints;
    data->zoneData[zoneNumber].averagePoint[1] /= data->zoneData[zoneNumber].numberOfPoints;
    data->zoneData[zoneNumber].averagePoint[2] /= data->zoneData[zoneNumber].numberOfPoints;
}


//=====================================================================================================
// function update RTmagCalibrationUpdate
//=====================================================================================================
//
// main function: updates the calibration corpus and computes the new offsets and scaling factors
//
short RTmagCalibrationUpdate( RTmagCalData* data, float rawPoint[3]) {
    float calPoint[3];
    float normPoint2;
    
    short i, zoneNumber, numberOfSamples, err;
    
    double quadricCoefficients6[6]; // not used here, needed however to call ellipsoidFit
    
    // scale the new point
    calPoint[0] = (rawPoint[0]-data->estimatedOffset[0])*data->estimatedScalingFactor[0];
    calPoint[1] = (rawPoint[1]-data->estimatedOffset[1])*data->estimatedScalingFactor[1];
    calPoint[2] = (rawPoint[2]-data->estimatedOffset[2])*data->estimatedScalingFactor[2];
    
    // checks if the new point is close enough to the unit sphere, i.e. if abs(norm(Point)-1) <= RTmagMaxDistanceError
    // this is equivalent to: (1-RTmagMaxDistanceError)^2 <= norm(Point)^2 <= (1+RTmagMaxDistanceError)^2
    // i.e. RTmagMinDistance2 <= norm(Point)^2 <= RTmaxMinDistance2
    normPoint2 = calPoint[0]*calPoint[0]+calPoint[1]*calPoint[1]+calPoint[2]*calPoint[2];
    if((normPoint2 < data->RTmagMinDistance2) || (normPoint2 > data->RTmagMaxDistance2)) // point out of allowed bounds, does nothing and returns error
        return 1;

    
    // get the zone number corresponding to the point
    zoneNumber = getClosestFibonacciPoint(data, calPoint);
    
    // adds the point to the corresponding zone number
    addPoint2FibonnaciZone( data, zoneNumber, rawPoint);
    
    // builds the matrix for ellipsoid fit
    numberOfSamples = 0;
    for(i=0; i<RT_CAL_NUMBER_OF_ZONES; i++) {
        if(data->zoneData[i].numberOfPoints) { // if there are points in the zone, adds the average point to the matrix and update the number of Samples
            data->matrixA[0*numberOfSamples+i] = data->zoneData[i].averagePoint[0] * data->zoneData[i].averagePoint[0];
            data->matrixA[1*numberOfSamples+i] = data->zoneData[i].averagePoint[1] * data->zoneData[i].averagePoint[1];
            data->matrixA[2*numberOfSamples+i] = data->zoneData[i].averagePoint[2] * data->zoneData[i].averagePoint[2];
            
            data->matrixA[3*numberOfSamples+i] = 2 * data->zoneData[i].averagePoint[0];
            data->matrixA[4*numberOfSamples+i] = 2 * data->zoneData[i].averagePoint[1];
            data-> matrixA[5*numberOfSamples+i] = 2 * data->zoneData[i].averagePoint[2];
            
            numberOfSamples++;
        }
    }
    
    // apply ellipsoid fit
    err =  ellipsoidFitCore( data->matrixA, numberOfSamples, data->estimatedOffset, data->estimatedScaling, &data->conditionNumber, quadricCoefficients6);
    
    return err;
}

