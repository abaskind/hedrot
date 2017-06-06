//
//  libhedrot_RTmagCalibration.c
//  hedrot_receiver
//
//  (yet still experimental) real-time magnetometer routine, based on an algorithm by Matthieu Aussal
//
//  Created by Alexis Baskind on 06/05/17.
//
//  Part of code is derived from Matthieu Aussal, CMAP - Ecole Polytechnique / CNRS
//


#include "libhedrot_RTmagCalibration.h"


RTmagCalData* newRTmagCalData() {
    RTmagCalData* data = (RTmagCalData*) malloc(sizeof(RTmagCalData));
    
    data->calData = (calibrationData*) malloc(sizeof(calibrationData));
    
    data->TMPcalData = (calibrationData*) malloc(sizeof(calibrationData));
    
    data->zoneData = NULL;
    
    computeFibonnaciMapping(data);
    return data;
}

void freeRTmagCalData(RTmagCalData* data) {
    free(data->zoneData);
    data->zoneData = NULL;
    free(data->calData);
    free(data->TMPcalData);
    free(data);
}


void initRTmagCalData(RTmagCalData* data, float* initalEstimatedOffset, float* initalEstimatedScaling, float RTmagMaxDistanceError, short calibrationRateFactor, long maxNumberOfSamples){
    int i;
    
    // erase (if already exists), allocates and inits zoneData
    if(data->zoneData) free(data->zoneData);
    data->zoneData = (RTmagZoneData*) malloc(RT_CAL_NUMBER_OF_ZONES * sizeof(RTmagZoneData));
    for( i=0;i<RT_CAL_NUMBER_OF_ZONES; i++) {
        data->zoneData[i].numberOfPoints = 0;
        data->zoneData[i].indexOfCurrentPoint = 0;
    }
    
    RTmagCalibration_setRTmagMaxDistanceError(data, RTmagMaxDistanceError);
    
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
    
    data->sampleIndexStep1 = 0;
    data->maxNumberOfSamples = maxNumberOfSamples;
    
    data->calData->numberOfSamples = 0;
    data->calData->normStdDev = 100000; // very big number, to force update on the first pass
    
    data->TMPcalData->numberOfSamples = 0;
    data->TMPcalData->normStdDev = 100000; // very big number, to force update on the first pass
    
    data->previousConditionNumber = MAX_CONDITION_NUMBER_REALTIME; // very big number, to force update on the first pass
    data->previousEstimatedOffset[0] = initalEstimatedOffset[0];
    data->previousEstimatedOffset[1] = initalEstimatedOffset[1];
    data->previousEstimatedOffset[2] = initalEstimatedOffset[2];
    data->previousEstimatedScaling[0] = initalEstimatedScaling[0];
    data->previousEstimatedScaling[1] = initalEstimatedScaling[1];
    data->previousEstimatedScaling[2] = initalEstimatedScaling[2];
    
    data->numberOfFilledZones = 0;
    
    data->calibrationRateFactor = calibrationRateFactor;
    data->calibrationRateCounter = data->calibrationRateFactor;
    
    data->proportionOfRejectedPoints_LPcoeff = 1.0 - exp(-1.0/TIME_CONSTANT_INDICATOR_OF_REJECTED_POINTS);
}



void RTmagCalibration_setmaxNumberOfSamples(RTmagCalData* data, long maxNumberOfSamples) {
    data->maxNumberOfSamples = maxNumberOfSamples;
}


void RTmagCalibration_setCalibrationRateFactor(RTmagCalData* data, short calibrationRateFactor) {
    data->calibrationRateFactor = calibrationRateFactor;
    data->calibrationRateCounter = data->calibrationRateFactor;
}

void RTmagCalibration_setRTmagMaxDistanceError(RTmagCalData* data, float RTmagMaxDistanceError) {
    data->RTmagMaxDistanceError = RTmagMaxDistanceError;
    data->RTmagMinDistance2 = (1-RTmagMaxDistanceError)*(1-RTmagMaxDistanceError);
    data->RTmagMaxDistance2 = (1+RTmagMaxDistanceError)*(1+RTmagMaxDistanceError);
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
short getClosestFibonacciPoint( RTmagCalData* data, double calPoint[3]) {
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
}


//=====================================================================================================
// function RTmagCalibrationUpdateDirect
//=====================================================================================================
//
// main function (direct method): updates the calibration corpus and computes the new offsets and scaling factors
//
// returns result status:
//  0: too many points out of bounds, restart calibration
//  1: point added, no calibration done
//  2: point added, calibration failed
//  3: point added, calibration succeeded
short RTmagCalibrationUpdateDirect( RTmagCalData* data, short rawPoint[3]) {
    double calPoint[3];
    double normPoint2;
    
    short pointRejectedStatus;
    double proportionOfRejectedPoints;
    
    float previousAverage, previousStandardDeviation;
    float newEstimatedOffset[3], newEstimatedScaling[3];
    
    short err;
    
    double quadricCoefficients6[6]; // not used here, needed however to call nonRotatedEllipsoidFit
    
    float X0[3]; // average value
    
    
    // store the new point anyway
    data->TMPcalData->rawSamples[data->sampleIndexStep1][0] = rawPoint[0];
    data->TMPcalData->rawSamples[data->sampleIndexStep1][1] = rawPoint[1];
    data->TMPcalData->rawSamples[data->sampleIndexStep1][2] = rawPoint[2];
    data->TMPcalData->numberOfSamples = min(data->TMPcalData->numberOfSamples+1,data->maxNumberOfSamples);
    data->sampleIndexStep1++;
    if(data->sampleIndexStep1 == data->maxNumberOfSamples)
        data->sampleIndexStep1 = 0; // maximum number of samples reached, erase the oldest ones
    
    if(data->calibrationValid) {
        // check if calibration is still ok, otherwise restart it
        
        // scale the new point
        calPoint[0] = (rawPoint[0]-data->estimatedOffset[0])*data->estimatedScalingFactor[0];
        calPoint[1] = (rawPoint[1]-data->estimatedOffset[1])*data->estimatedScalingFactor[1];
        calPoint[2] = (rawPoint[2]-data->estimatedOffset[2])*data->estimatedScalingFactor[2];
        
        // checks if the new point is close enough to the unit sphere, i.e. if abs(norm(Point)-1) <= RTmagMaxDistanceError
        // this is equivalent to: (1-RTmagMaxDistanceError)^2 <= norm(Point)^2 <= (1+RTmagMaxDistanceError)^2
        // i.e. RTmagMinDistance2 <= norm(Point)^2 <= RTmaxMinDistance2
        // this is done only if a first successful calibration has been done
        normPoint2 = calPoint[0]*calPoint[0]+calPoint[1]*calPoint[1]+calPoint[2]*calPoint[2];
        if((normPoint2 >= data->RTmagMinDistance2) && (normPoint2 <= data->RTmagMaxDistance2)) {// point inside allowed bounds
            pointRejectedStatus = 0;
        } else {
            pointRejectedStatus = 1;
        }
        
        // update the indicator of the proportion of rejected points in the last time with moving average (duration defined by TIME_CONSTANT_INDICATOR_OF_REJECTED_POINTS)
        proportionOfRejectedPoints = data->proportionOfRejectedPoints_LPcoeff * pointRejectedStatus + (1 - data->proportionOfRejectedPoints_LPcoeff) * data->proportionOfRejectedPoints_State;
        if(proportionOfRejectedPoints > MAX_ALLOWED_PROPORTION_OF_REJECTED_POINTS) {
            // too many rejected points in the last time, restart calibration
            printf("Too many rejected points, restart calibration\r\n");
            initRTmagCalData(data, data->estimatedOffset, data->estimatedScaling, data->RTmagMaxDistanceError, data->calibrationRateFactor, data->maxNumberOfSamples);
            return 0;
        } else {
            data->proportionOfRejectedPoints_State = proportionOfRejectedPoints;
            // go further
        }
    }
    
    // try to calibrate according to the "brute force" method (with all raw points)
    
    // if this is time for calibration, try to calibrate
    data->calibrationRateCounter--;
    
    if(!data->calibrationRateCounter) {
        data->calibrationRateCounter = data->calibrationRateFactor;
        
        // filter out most extreme values
        // compute average of the raw data (rough estimation of the center)
        getMean3(&data->TMPcalData->rawSamples[0][0], data->TMPcalData->numberOfSamples, X0);
        
        // filter out aberrant raw samples
        filterCalData(data->TMPcalData, data->calData, X0);
        
        // apply ellipsoid fit
        err = nonRotatedEllipsoidFit(data->calData, data->estimatedOffset, data->estimatedScaling, quadricCoefficients6, MAX_CONDITION_NUMBER_REALTIME);
        if(!err) {
            if(data->calData->normStdDev <= MAX_ALLOWED_STANDARD_DEVIATION_ERROR) {
                // calibration succeded, compare it with the previous one
                newEstimatedOffset[0] = data->estimatedOffset[0];
                newEstimatedOffset[1] = data->estimatedOffset[1];
                newEstimatedOffset[2] = data->estimatedOffset[2];
                newEstimatedScaling[0] = data->estimatedScaling[0];
                newEstimatedScaling[1] = data->estimatedScaling[1];
                newEstimatedScaling[2] = data->estimatedScaling[2];
                
                // recompute standard dev with previous estimated parameters
                computeCalNormStatistics(data->calData, data->previousEstimatedOffset, data->previousEstimatedScaling, &previousAverage, &previousStandardDeviation);
                
                // compare new and old standard deviation and condition number
                if((data->calData->normStdDev <= previousStandardDeviation) && (data->calData->conditionNumber <= data->previousConditionNumber)) {
                    // the new estimation is better than the previous one, go for it
                    data->calibrationValid = 1;
                    
                    
                    // update scaling factor
                    data->estimatedScalingFactor[0] = 1/data->estimatedScaling[0];
                    data->estimatedScalingFactor[1] = 1/data->estimatedScaling[1];
                    data->estimatedScalingFactor[2] = 1/data->estimatedScaling[2];
                    
                    // initialize indicator of proportion of rejected points
                    data->proportionOfRejectedPoints_State = 0;
                    
                    
                    // store condition number and estimated parameters to compare later
                    data->previousConditionNumber = data->calData->conditionNumber;
                    data->previousEstimatedOffset[0] = data->estimatedOffset[0];
                    data->previousEstimatedOffset[1] = data->estimatedOffset[1];
                    data->previousEstimatedOffset[2] = data->estimatedOffset[2];
                    data->previousEstimatedScaling[0] = data->estimatedScaling[0];
                    data->previousEstimatedScaling[1] = data->estimatedScaling[1];
                    data->previousEstimatedScaling[2] = data->estimatedScaling[2];
                    
                    return 3;
                } else {
                    printf("previous calibration was better, calibration rejected\r\n");
                    printf("previous condition number %f, new condition number %f\r\n", data->previousConditionNumber, data->calData->conditionNumber);
                    printf("previous standard deviation %f, new standard deviation %f\r\n", previousStandardDeviation, data->calData->normStdDev);
                    
                    // the previous estimation was better, keep it
                    data->estimatedOffset[0] = data->previousEstimatedOffset[0];
                    data->estimatedOffset[1] = data->previousEstimatedOffset[1];
                    data->estimatedOffset[2] = data->previousEstimatedOffset[2];
                    data->estimatedScaling[0] = data->previousEstimatedScaling[0];
                    data->estimatedScaling[1] = data->previousEstimatedScaling[1];
                    data->estimatedScaling[2] = data->previousEstimatedScaling[2];
                    
                    return 2;
                }
                
            } else {
                printf("standard deviation too high, calibration rejected\r\n");
                // the previous estimation was better, keep it
                data->estimatedOffset[0] = data->previousEstimatedOffset[0];
                data->estimatedOffset[1] = data->previousEstimatedOffset[1];
                data->estimatedOffset[2] = data->previousEstimatedOffset[2];
                data->estimatedScaling[0] = data->previousEstimatedScaling[0];
                data->estimatedScaling[1] = data->previousEstimatedScaling[1];
                data->estimatedScaling[2] = data->previousEstimatedScaling[2];
                return 2;
            }
        } else { // calibration failed
            return 2;
        }
    } else {
        return 1;
    }
}


//=====================================================================================================
// function RTmagCalibrationUpdateIterative
//=====================================================================================================
//
// main function (iterative method): updates the calibration corpus and computes the new offsets and scaling factors
//
// returns result status:
//  0: point out of bounds (absurd measure), not accepted
//  1: point added, no calibration done
//  2: point added, calibration failed
//  3: point added, calibration succeeded
short RTmagCalibrationUpdateIterative( RTmagCalData* data, short rawPoint[3]) {
    double calPoint[3];
    short rawTMPpoint[3];
    double normPoint2;
    
    short i, zoneNumber, err;
    
    double quadricCoefficients6[6]; // not used here, needed however to call nonRotatedEllipsoidFit
    
    float X0[3]; // average value
    
    if(!data->calibrationValid) {
        // Step 1: no calibration has been done already, try to calibrate according to the "brute force" method (with all raw points)
        
        // store the new point
        data->TMPcalData->rawSamples[data->sampleIndexStep1][0] = rawPoint[0];
        data->TMPcalData->rawSamples[data->sampleIndexStep1][1] = rawPoint[1];
        data->TMPcalData->rawSamples[data->sampleIndexStep1][2] = rawPoint[2];
        data->TMPcalData->numberOfSamples = min(data->TMPcalData->numberOfSamples+1,data->maxNumberOfSamples);
        data->sampleIndexStep1++;
        if(data->sampleIndexStep1 == data->maxNumberOfSamples)
            data->sampleIndexStep1 = 0; // maximum number of samples reached, erase the oldest ones
        
        
        // if this is time for calibration, try to calibrate
        data->calibrationRateCounter--;
        
        if(!data->calibrationRateCounter) {
            data->calibrationRateCounter = data->calibrationRateFactor;
            
            // filter out most extreme values
            // compute average of the raw data (rough estimation of the center)
            getMean3(&data->TMPcalData->rawSamples[0][0], data->TMPcalData->numberOfSamples, X0);
            
            // filter out aberrant raw samples
            filterCalData(data->TMPcalData, data->calData, X0);
            
            // apply ellipsoid fit
            err = nonRotatedEllipsoidFit(data->calData, data->estimatedOffset, data->estimatedScaling, quadricCoefficients6, MAX_CONDITION_NUMBER_REALTIME);
            if(!err) {
                if(data->calData->normStdDev <= MAX_ALLOWED_STANDARD_DEVIATION_ERROR) {
                    // calibration succeded
                    data->calibrationValid = 1;
                    
                    // update scaling factor
                    data->estimatedScalingFactor[0] = 1/data->estimatedScaling[0];
                    data->estimatedScalingFactor[1] = 1/data->estimatedScaling[1];
                    data->estimatedScalingFactor[2] = 1/data->estimatedScaling[2];
                    
                    // cluster all samples on Fibonacci Mapping
                    for (i=0; i<data->calData->numberOfSamples; i++) {
                        // get the zone number corresponding to the point
                        zoneNumber = getClosestFibonacciPoint(data, data->calData->calSamples[i]);
                        
                        // adds the point to the corresponding zone number
                        rawTMPpoint[0] = data->calData->rawSamples[i][0];
                        rawTMPpoint[1] = data->calData->rawSamples[i][1];
                        rawTMPpoint[2] = data->calData->rawSamples[i][2];
                        addPoint2FibonnaciZone( data, zoneNumber, rawTMPpoint);
                    }
                    
                    // initialize indicator of proportion of rejected points
                    data->proportionOfRejectedPoints_State = 0;
                    
                    return 3;
                    
                } else {
                    printf("standard deviation too high, calibration rejected");
                    return 2;
                }
            } else { // calibration failed
                return 2;
            }
        } else {
            return 1;
        }
        
    } else {
        // Step 2: a first calibration has been done, try to optimize it with Fibonacci mapping
        
        // scale the new point
        calPoint[0] = (rawPoint[0]-data->estimatedOffset[0])*data->estimatedScalingFactor[0];
        calPoint[1] = (rawPoint[1]-data->estimatedOffset[1])*data->estimatedScalingFactor[1];
        calPoint[2] = (rawPoint[2]-data->estimatedOffset[2])*data->estimatedScalingFactor[2];
        
        // checks if the new point is close enough to the unit sphere, i.e. if abs(norm(Point)-1) <= RTmagMaxDistanceError
        // this is equivalent to: (1-RTmagMaxDistanceError)^2 <= norm(Point)^2 <= (1+RTmagMaxDistanceError)^2
        // i.e. RTmagMinDistance2 <= norm(Point)^2 <= RTmaxMinDistance2
        // this is done only if a first successful calibration has been done
        normPoint2 = calPoint[0]*calPoint[0]+calPoint[1]*calPoint[1]+calPoint[2]*calPoint[2];
        
        if(normPoint2<UPPER_NORM2_FOR_ABSURD_VALUES) {
            if((normPoint2 < data->RTmagMinDistance2) || (normPoint2 > data->RTmagMaxDistance2)) {// point outside tolerance => warning only, but accept point anyway
                printf("Warning: significant deviation of norm \r\n");
            }
            
            // get the zone number corresponding to the point
            zoneNumber = getClosestFibonacciPoint(data, calPoint);
            
            // adds the point to the corresponding zone number
            addPoint2FibonnaciZone( data, zoneNumber, rawPoint);
            
            // if this is time for calibration, try to calibrate
            data->calibrationRateCounter--;
            if(!data->calibrationRateCounter) {
                data->calibrationRateCounter = data->calibrationRateFactor;
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
                err = nonRotatedEllipsoidFit(data->calData, data->estimatedOffset, data->estimatedScaling, quadricCoefficients6, 100000000000);
                if(!err) {
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
            printf("Absurd measure, point rejected\r\n");
            return 0;
        }
    }
}


