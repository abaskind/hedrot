//
//  libhedrot_RTmagCalibration.c
//  hedrot_receiver
//
//  (yet still experimental) real-time magnetometer routine, based on an algorithm by Matthieu Aussal
//
//  Created by Alexis Baskind on 06/05/17.
//
//

#include "libhedrot_RTmagCalibration.h"


RTmagCalData* newRTmagCalData() {
    RTmagCalData* data = (RTmagCalData*) malloc(sizeof(RTmagCalData));
    
    data->calData = (calibrationData*) malloc(sizeof(calibrationData));
    
    data->zoneData = NULL;
    
    computeFibonnaciMapping(data);
return data;
}

void freeRTmagCalData(RTmagCalData* data) {
    free(data->zoneData);
    data->zoneData = NULL;
    free(data->calData);
    free(data);
}


void initRTmagCalData(RTmagCalData* data, float* initalEstimatedOffset, float* initalEstimatedScaling, float RTmagMaxDistanceError, short calibrationRateFactor){
    int i;
    
    // erase (if already exists), allocates and inits zoneData
    if(data->zoneData) free(data->zoneData);
    data->zoneData = (RTmagZoneData*) malloc(RT_CAL_NUMBER_OF_ZONES * sizeof(RTmagZoneData));
    for( i=0;i<RT_CAL_NUMBER_OF_ZONES; i++) {
        data->zoneData[i].numberOfPoints = 0;
        data->zoneData[i].indexOfCurrentPoint = 0;
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
    
    data->calibrationValid = 0;
    data->numberOfFilledZones = 0;
    
    data->calibrationRateFactor = calibrationRateFactor;
    data->calibrationRateCounter = data->calibrationRateFactor;
}



void setRTmagMaxDistanceError(RTmagCalData* data, float RTmagMaxDistanceError) {
    data->RTmagMinDistance2 = (1-RTmagMaxDistanceError*RTmagMaxDistanceError)*(1-RTmagMaxDistanceError*RTmagMaxDistanceError);
    data->RTmagMaxDistance2 = (1+RTmagMaxDistanceError*RTmagMaxDistanceError)*(1+RTmagMaxDistanceError*RTmagMaxDistanceError);
}


void setCalibrationRateFactor(RTmagCalData* data, short calibrationRateFactor) {
    data->calibrationRateFactor = calibrationRateFactor;
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
        theta   = 2.0*M_PI*mod(M_INV_GOLDEN_RATIO*i,1);
        sinPhi  = 1.0 - (2*i+1.0)/RT_CAL_NUMBER_OF_ZONES;
        
        data->Fibonacci_Points[i][0] = cos(theta)*sinPhi;      // x
        data->Fibonacci_Points[i][1] = sin(theta)*sinPhi;      // y
        data->Fibonacci_Points[i][2] = sqrt(1-sinPhi*sinPhi);  // z
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
        
        /*printf("point %i, XYZ= %f %f %f, calpoint_XYZ %f %f %f, distance2 = %f\r\n",i,
               data->Fibonacci_Points[i][0], data->Fibonacci_Points[i][1], data->Fibonacci_Points[i][2],
               calPoint[0], calPoint[1], calPoint[2],
               dist2);
        dist2 = 0;*/
    }
    
    return zoneNumber;
}

//=====================================================================================================
// function addPoint2FibonnaciZone
//=====================================================================================================
//
// adds a new point in a given zone
//
void addPoint2FibonnaciZone( RTmagCalData* data, short zoneNumber, short rawPoint[3]) {
    short i;

    // if the zone was previously empty, add 1 to data->numberOfFilledZones
    if(!data->zoneData[zoneNumber].numberOfPoints) {
        data->numberOfFilledZones++;
    }
    
    // adds the point in the corresponding zone
    //ring buffer: does not accept more than RT_CAL_NUMBER_OF_POINTS_PER_ZONE points
    data->zoneData[zoneNumber].numberOfPoints = min(data->zoneData[zoneNumber].numberOfPoints+1, RT_CAL_NUMBER_OF_POINTS_PER_ZONE);
    
    data->zoneData[zoneNumber].points[data->zoneData[zoneNumber].indexOfCurrentPoint][0] = rawPoint[0];
    data->zoneData[zoneNumber].points[data->zoneData[zoneNumber].indexOfCurrentPoint][1] = rawPoint[1];
    data->zoneData[zoneNumber].points[data->zoneData[zoneNumber].indexOfCurrentPoint][2] = rawPoint[2];
    
    data->zoneData[zoneNumber].indexOfCurrentPoint++;
    if(data->zoneData[zoneNumber].indexOfCurrentPoint == RT_CAL_NUMBER_OF_POINTS_PER_ZONE)
        data->zoneData[zoneNumber].indexOfCurrentPoint = 0; // ring buffer: returns to 0 if overload
    
    // compute new average
    data->zoneData[zoneNumber].averagePoint[0] = 0;
    data->zoneData[zoneNumber].averagePoint[1] = 0;
    data->zoneData[zoneNumber].averagePoint[2] = 0;
    for(i=0; i<data->zoneData[zoneNumber].numberOfPoints; i++) {
        data->zoneData[zoneNumber].averagePoint[0] += data->zoneData[zoneNumber].points[i][0];
        data->zoneData[zoneNumber].averagePoint[1] += data->zoneData[zoneNumber].points[i][1];
        data->zoneData[zoneNumber].averagePoint[2] += data->zoneData[zoneNumber].points[i][2];
    }
    data->zoneData[zoneNumber].averagePoint[0] /= data->zoneData[zoneNumber].numberOfPoints;
    data->zoneData[zoneNumber].averagePoint[1] /= data->zoneData[zoneNumber].numberOfPoints;
    data->zoneData[zoneNumber].averagePoint[2] /= data->zoneData[zoneNumber].numberOfPoints;
    
    /*printf("adding a point to zone number %d, number of points %d, new average %f %f %f\r\n", zoneNumber, data->zoneData[zoneNumber].numberOfPoints,
           data->zoneData[zoneNumber].averagePoint[0],
           data->zoneData[zoneNumber].averagePoint[1],
           data->zoneData[zoneNumber].averagePoint[2]); // for debugging*/
}


//=====================================================================================================
// function RTmagCalibrationUpdate
//=====================================================================================================
//
// main function: updates the calibration corpus and computes the new offsets and scaling factors
//
// returns result status:
//  0: point out of bounds, non added
//  1: point added, no calibration done
//  2: point added, calibration failed
//  3: point added, calibration succeeded
short RTmagCalibrationUpdate( RTmagCalData* data, short rawPoint[3]) {
    float calPoint[3];
    float normPoint2;
    
    short i, zoneNumber, err;
    
    double quadricCoefficients6[6]; // not used here, needed however to call ellipsoidFit
    
    // scale the new point
    calPoint[0] = (rawPoint[0]-data->estimatedOffset[0])*data->estimatedScalingFactor[0];
    calPoint[1] = (rawPoint[1]-data->estimatedOffset[1])*data->estimatedScalingFactor[1];
    calPoint[2] = (rawPoint[2]-data->estimatedOffset[2])*data->estimatedScalingFactor[2];
    
    // checks if the new point is close enough to the unit sphere, i.e. if abs(norm(Point)-1) <= RTmagMaxDistanceError
    // this is equivalent to: (1-RTmagMaxDistanceError)^2 <= norm(Point)^2 <= (1+RTmagMaxDistanceError)^2
    // i.e. RTmagMinDistance2 <= norm(Point)^2 <= RTmaxMinDistance2
    // this is done only if a first successful calibration has been done
    if(data->calibrationValid) {
        normPoint2 = calPoint[0]*calPoint[0]+calPoint[1]*calPoint[1]+calPoint[2]*calPoint[2];
        //printf("normPoint2 = %f, RTmagMinDistance2 = %f, RTmagMaxDistance2 = %f\r\n", normPoint2, data->RTmagMinDistance2, data->RTmagMaxDistance2); // for debugging
        if((normPoint2 < data->RTmagMinDistance2) || (normPoint2 > data->RTmagMaxDistance2)) // point out of allowed bounds, does nothing and returns error
            return 0;
    }
    
    // get the zone number corresponding to the point
    zoneNumber = getClosestFibonacciPoint(data, calPoint);
    
    // adds the point to the corresponding zone number
    addPoint2FibonnaciZone( data, zoneNumber, rawPoint);
    
    // if this is time for calibration and if there is information for enough zones, try to calibrate
    data->calibrationRateCounter--;
    if(!data->calibrationRateCounter) {
        data->calibrationRateCounter = data->calibrationRateFactor;
        
        if(data->numberOfFilledZones >= RT_CAL_MIN_NUMBER_OF_ZONES) {
            // builds the matrix for ellipsoid fit
            data->calData->numberOfSamples = 0;
            for(i=0; i<RT_CAL_NUMBER_OF_ZONES; i++) {
                if(data->zoneData[i].numberOfPoints) { // if there are points in the zone, adds the average point to the list and update the number of samples
                    data->calData->rawSamples[data->calData->numberOfSamples][0] = data->zoneData[i].averagePoint[0];
                    data->calData->rawSamples[data->calData->numberOfSamples][1] = data->zoneData[i].averagePoint[1];
                    data->calData->rawSamples[data->calData->numberOfSamples][2] = data->zoneData[i].averagePoint[2];
                    
                    data->calData->numberOfSamples++;
                }
            }
            
            // apply ellipsoid fit
            err = ellipsoidFit(data->calData, data->estimatedOffset, data->estimatedScaling, quadricCoefficients6);
            if(!err) {
                // calibration succeded
                if(!data->calibrationValid)
                    data->calibrationValid = 1;
                // update scaling factor
                data->estimatedScalingFactor[0] = 1/data->estimatedScaling[0];
                data->estimatedScalingFactor[1] = 1/data->estimatedScaling[1];
                data->estimatedScalingFactor[2] = 1/data->estimatedScaling[2];
                
                return 3;
            } else { // calibration failed
                return 2;
            }
        } else {
            return 1;
        }
    } else {
        return 1;
    }
}

