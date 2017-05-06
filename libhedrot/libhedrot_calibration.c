//
//  libhedrot_calibration.c
//  libhedrot
//
//  Created by Alexis Baskind on 17/04/17.
//
//

#include "libhedrot_calibration.h"

#include "libhedrot_utils.h"

// include cblas and lapack for matrix operations
#ifdef __MACH__  // if mach (mac os X)
#include <Accelerate/Accelerate.h>
#else
#if defined(_WIN32) || defined(_WIN64)
#include "lapacke.h"
#endif
#endif


//=====================================================================================================
// function accMagCalibration
//=====================================================================================================
//
// general function for calibrating accelerometers and magnetometers
//
//
// returns 1 (error) if calibration failed

int accMagCalibration(calibrationData* calData, float* estimatedOffset, float* estimatedScaling) {
    calibrationData TMPcalData1;
    float TMPestimatedOffset[3], TMPestimatedScaling[3];
    char err = 0;
    double quadricCoefficients6[6]; // not used here, needed however to call ellipsoidFit
    
    // first pass: calls ellipsoidFit a first time
    if( ellipsoidFit(calData, TMPestimatedOffset, TMPestimatedScaling, quadricCoefficients6) ) {
        // if error, quits
        printf( "[libhedrot] (function accMagCalibration) first pass failed.\r\n" );
        err =  1;
        goto end;
    }
    
    // no error, goes on
    printf( "[libhedrot] (function accMagCalibration) first pass succeeded.\r\n" );
    printf( "           radii: %f %f %f\r\n", TMPestimatedScaling[0], TMPestimatedScaling[1], TMPestimatedScaling[2]);
    printf( "           offsets: %f %f %f\r\n", TMPestimatedOffset[0], TMPestimatedOffset[1], TMPestimatedOffset[2]);
    printf( "           condition number for ellipsoid fit: %f\r\n", calData->conditionNumber);
    printf( "           maximum norm error: %f\r\n", calData->maxNormError);
    

    // based on the norm error from the first pass, filter out aberrant raw samples
    filterCalData(calData, &TMPcalData1, TMPestimatedOffset);
    
    // second pass: calls ellipsoidFit a second time
    if( ellipsoidFit(&TMPcalData1, TMPestimatedOffset, TMPestimatedScaling, quadricCoefficients6) ) {
        // if error, quits
        printf( "[libhedrot] (function accMagCalibration) second pass failed.\r\n" );
        err =  1;
        goto end;
    }
    
    // no error, goes on
    printf( "[libhedrot] (function accMagCalibration) second pass succeeded.\r\n" );
    printf( "           radii: %f %f %f\r\n", TMPestimatedScaling[0], TMPestimatedScaling[1], TMPestimatedScaling[2]);
    printf( "           offsets: %f %f %f\r\n", TMPestimatedOffset[0], TMPestimatedOffset[1], TMPestimatedOffset[2]);
    printf( "           condition number for ellipsoid fit: %f\r\n", TMPcalData1.conditionNumber);
    printf( "           maximum norm error: %f\r\n", TMPcalData1.maxNormError);
    
    // save the final estimation values
    estimatedOffset[0] = (float) TMPestimatedOffset[0];
    estimatedOffset[1] = (float) TMPestimatedOffset[1];
    estimatedOffset[2] = (float) TMPestimatedOffset[2];
    estimatedScaling[0] = (float) TMPestimatedScaling[0];
    estimatedScaling[1] = (float) TMPestimatedScaling[1];
    estimatedScaling[2] = (float) TMPestimatedScaling[2];
    
    // cook extra calibration data on initial (unfiltered) data set
    cookCalibrationData(calData, estimatedOffset, estimatedScaling);
    
    // overwrite the stats that have just been computed by the previous one (which ignores the data that had been filtered out)
    calData->maxNormError = TMPcalData1.maxNormError;
    calData->normAverage = TMPcalData1.normAverage;
    calData->normStdDev = TMPcalData1.normStdDev;
    
    // set as condition number the one from the second pass
    calData->conditionNumber = TMPcalData1.conditionNumber;
    
end:

    return err;
}


//=====================================================================================================
// function myCalibrationOffline
//=====================================================================================================
//
// general function for calibrating accelerometers and magnetometers
// adapted from Matlab code "MyCalibration.m" by Matthieu Aussal, first part (offline)
//
//
// returns 1 (error) if calibration failed

int myCalibrationOffline(calibrationData* calData, float* estimatedOffset, float* estimatedScaling) {
    calibrationData TMPcalData1;
    float TMPestimatedOffset[3], TMPestimatedScaling[3];
    int i;
    char err = 0;
    double quadricCoefficients6[6], quadricCoefficients9[9];
    float X0[3];
    double *distance;
    double tmpMax1, tmpMax2, normFactor;
    
    distance = (double*) malloc(calData->numberOfSamples * sizeof(double));
    
    // compute average of the raw data (rough estimation of the center)
    getMean3(&calData->rawSamples[0][0], calData->numberOfSamples, X0);
    
    // filter out aberrant raw samples
    filterCalData(calData, &TMPcalData1, X0);
    
    // first pass: calls quadricFit (rotated ellipsoid)
    if( quadricFit(&TMPcalData1, quadricCoefficients9) ) {
        // if error, quits
        printf( "[libhedrot] (function myCalibrationOffline) first pass failed.\r\n" );
        err =  1;
        goto end;
    }
    
    // no error, goes on
    printf( "[libhedrot] (function myCalibrationOffline) first pass succeeded.\r\n" );
    printf( "debug output: %f, %f, %f, %f, %f, %f, %f, %f, %f\r\n", quadricCoefficients9[0], quadricCoefficients9[1], quadricCoefficients9[2],
           quadricCoefficients9[3], quadricCoefficients9[4], quadricCoefficients9[5],
           quadricCoefficients9[6], quadricCoefficients9[7], quadricCoefficients9[8] );

    // second pass: calls ellipsoidFit a first time (forcing non-rotated ellipsoid)
    if( ellipsoidFit(&TMPcalData1, TMPestimatedOffset, TMPestimatedScaling, quadricCoefficients6) ) {
        // if error, quits
        printf( "[libhedrot] (function myCalibrationOffline) second pass failed.\r\n" );
        err =  1;
        goto end;
    }
    
    
    printf( "[libhedrot] (function myCalibrationOffline) second pass succeeded.\r\n" );
    printf( "debug output: %f, %f, %f, %f, %f, %f\r\n", quadricCoefficients6[0], quadricCoefficients6[1], quadricCoefficients6[2],
           quadricCoefficients6[3], quadricCoefficients6[4], quadricCoefficients6[5] );
    
    // check if the possible rotation (calculated from the first pass) can be neglected
    // 1° check if the coefficients 4-6 for the cross products in the quadric are small with respect to the other ones
    // quadric coefficients are normalized with respect to the maximum average data
    // (quadratic coefficients, i.e. 7-9, are normalized with the quadratic norm)
    normFactor = max(fabs(X0[0]), max(fabs(X0[1]), fabs(X0[2])));
    tmpMax1 = 0;
    tmpMax2 = 0;
    for( i=0; i<3; i++)
        tmpMax2 = max(tmpMax2, fabs(quadricCoefficients9[i]*normFactor*normFactor));
    for( i=3; i<6; i++) {
        tmpMax1 = max(tmpMax1, fabs(quadricCoefficients9[i]*normFactor*normFactor));
        tmpMax2 = max(tmpMax2, fabs(quadricCoefficients9[i]*normFactor*normFactor));
    }
    for( i=6; i<9; i++)
        tmpMax2 = max(tmpMax2, fabs(quadricCoefficients9[i]*normFactor));
    
    if( tmpMax1 >= .1*tmpMax2 ) { //10% error allowed
        // if error, quits
        printf( "[libhedrot] (function myCalibrationOffline) first check of non rotated ellipsoid failed.\r\n" );
        printf( "debug output: %f, %f\r\n", tmpMax1, tmpMax2 );
        err =  1;
        goto end;
    } else {
        printf( "[libhedrot] (function myCalibrationOffline) first check of non rotated ellipsoid succeeded.\r\n" );
    }
    
    // 2° check if the coefficients 1-3,7-9 of the estimated quadrics with and without cross products (i.e. with and without rotation)
    // are close to each other
    // quadric coefficients are normalized with respect to the maximum average data
    // (quadratic coefficients, i.e. 7-9, are normalized with the quadratic norm)
    tmpMax1 = 0;
    tmpMax2 = 0;
    for( i=0; i<3; i++) { // quadric coefficients 1-3
        tmpMax1 = max(tmpMax1, fabs(quadricCoefficients6[i]-quadricCoefficients9[i])*normFactor*normFactor);
        tmpMax2 = max(tmpMax2, fabs(quadricCoefficients6[i])*normFactor*normFactor);
    }
    
    for( i=6; i<8; i++) { // quadric coefficients 7-9
        tmpMax1 = max(tmpMax1, fabs(quadricCoefficients6[i-3]-quadricCoefficients9[i])*normFactor);
        tmpMax2 = max(tmpMax2, fabs(quadricCoefficients6[i-3])*normFactor);
    }
    
    if( tmpMax1 >= .1*tmpMax2 ) { //10% error allowed
        // if error, quits
        printf( "[libhedrot] (function myCalibrationOffline) second check of non rotated ellipsoid failed.\r\n" );
        printf( "debug output: %f, %f\r\n", tmpMax1, tmpMax2 );
        err =  1;
        goto end;
    } else {
        printf( "[libhedrot] (function myCalibrationOffline) second check of non rotated ellipsoid succeeded.\r\n" );
    }
    
    // no error, goes on
    // save the final estimation values
    estimatedOffset[0] = TMPestimatedOffset[0];
    estimatedOffset[1] = TMPestimatedOffset[1];
    estimatedOffset[2] = TMPestimatedOffset[2];
    estimatedScaling[0] = TMPestimatedScaling[0];
    estimatedScaling[1] = TMPestimatedScaling[1];
    estimatedScaling[2] = TMPestimatedScaling[2];
    
    printf( "           radii: %f %f %f\r\n", estimatedScaling[0], estimatedScaling[1], estimatedScaling[2]);
    printf( "           offsets: %f %f %f\r\n", estimatedOffset[0], estimatedOffset[1], estimatedOffset[2]);
    printf( "           condition number for ellipsoid fit: %f\r\n", TMPcalData1.conditionNumber);
    printf( "           maximum norm error: %f\r\n", TMPcalData1.maxNormError);
    
    
    
    // cook extra calibration data on initial (unfiltered) data set
    cookCalibrationData(calData, estimatedOffset, estimatedScaling);
    
    // overwrite the max norm error that has been just computed by the previous one (which ignores the data that had been filtered out)
    calData->maxNormError = TMPcalData1.maxNormError;
    
    // set as condition number the one from the second pass
    calData->conditionNumber = TMPcalData1.conditionNumber;
    
end:

    return err;
}



//=====================================================================================================
// function ellipsoidFit
//=====================================================================================================
//
// find the center and raddii of a set of raw data (Nx3), assumed to be on a non rotated ellipsoid
// The calculation is done in double precision, the result is converted to single.
//
// method:
// classical ellipsoid fit algorithm without rotation = least-squares optimization on the linearized problem (after variable changes):
// a*X^2 + b*Y^2 + c*Z^2 + d*2*X + e*2*Y + f*2*Z = 1;
//
// the radii and offset are calculated afterwards as follows:
// offsets = [-d/a -e/b -f/c]
// radii = [sqrt(gamma/a) sqrt(gamma/b) sqrt(gamma/c)]
// ... with gamma = 1 + ( d^2/a + e^2/b + f^2/c );
//
// returns 1 (error) if calibration failed
int ellipsoidFit(calibrationData* calData, float* estimatedOffset, float* estimatedScaling, double *quadricCoefficients) {
    int i;
    char err = 0;
    double gamma;
    
    // definitions
    double *matrixA = (double*) malloc(calData->numberOfSamples*6*sizeof(double)); // internal input matrix A (corresponding to the problem to solve A.X = B)
    double *matrixB = (double*) malloc(calData->numberOfSamples*sizeof(double)); // internal input matrix B (column mit ones), output = solution
    double vectorS[6]; //output = singular values
    
    // constants
    double rcond = 1/MAX_CONDITION_NUMBER; // reverse maximum condition number
    
#ifdef __MACH__  // if mach (mac os X)
    // constants
    __CLPK_integer one = 1, six=6;
    __CLPK_integer rank; // effective rank of the matrix
    double wkopt;
    double *work;
    __CLPK_integer n = calData->numberOfSamples;
    __CLPK_integer lda = calData->numberOfSamples, ldb = calData->numberOfSamples;
    __CLPK_integer lwork, info;
#else
#if defined(_WIN32) || defined(_WIN64)
    // constants
    lapack_int rank; // effective rank of the matrix
    int n = calData->numberOfSamples;
    int lda = calData->numberOfSamples, ldb = calData->numberOfSamples;
#endif
#endif
    
    
    // Build the matrix D (rows = X^2, Y^2, Z^2, 2*X, 2*Y, 2*Z) and the matrix ONES (N*1)
    for(i=0; i<calData->numberOfSamples; i++) {
        matrixA[0*calData->numberOfSamples+i] = calData->rawSamples[i][0] * calData->rawSamples[i][0];
        matrixA[1*calData->numberOfSamples+i] = calData->rawSamples[i][1] * calData->rawSamples[i][1];
        matrixA[2*calData->numberOfSamples+i] = calData->rawSamples[i][2] * calData->rawSamples[i][2];
        
        matrixA[3*calData->numberOfSamples+i] = 2 * calData->rawSamples[i][0];
        matrixA[4*calData->numberOfSamples+i] = 2 * calData->rawSamples[i][1];
        matrixA[5*calData->numberOfSamples+i] = 2 * calData->rawSamples[i][2];
        
        matrixB[i] = 1;
    }
    
    
#ifdef __MACH__  // if mach (mac os X)
    /* Query and allocate the optimal workspace */
    lwork = -1;
    dgelss_(&n, &six, &one, matrixA, &lda, matrixB, &ldb, vectorS, &rcond, &rank, &wkopt, &lwork, &info);
    lwork = (int)wkopt;
    work = (double*) malloc( lwork*sizeof(double) );
    /* Solve the equations A*X = B */
    dgelss_(&n, &six, &one, matrixA, &lda, matrixB, &ldb, vectorS, &rcond, &rank, work, &lwork, &info);
    
    /* Check for the full rank */
    /*if( info > 0 ) {
     printf( "The diagonal element %i of the triangular factor ", (int) info );
     printf( "of A is zero, so that A does not have full rank;\r\n" );
     printf( "the least squares solution could not be computed.\r\n" );
     return 1;
     }*/
#else
#if defined(_WIN32) || defined(_WIN64)
    LAPACKE_dgelss( LAPACK_COL_MAJOR, n, 6, 1, matrixA, lda, matrixB, ldb, vectorS, rcond, &rank );
#endif
#endif
    
    // check if the rank is different than 6
    if( rank != 6 ) {
        printf( "The matrix is not of full rank\r\n ");
        printf( "The least squares solution could not be computed.\r\n" );
        err =  1;
        goto end;
    }
    
    
    // check if the condition number is bigger than the allowed maximum
    if( vectorS[0]/vectorS[5] >= MAX_CONDITION_NUMBER ) {
        printf( "The condition number (%f) is bigger than the allowed maximum (%d)\r\n ", vectorS[0]/vectorS[5], MAX_CONDITION_NUMBER );
        err =  1;
        goto end;
    }
    
    // check if the estimated quadric is an ellipsoid
    if( abs(sign(matrixB[0])+sign(matrixB[1])+sign(matrixB[2]))!=3 ) {
        // if the 3 first coefficients of the estimated quadric are not of the same sign
        // the estimated quadric is not an ellipsoid
        printf( "The estimated quadric is not an ellipsoid\r\n");
        printf( "debug output: %f, %f, %f, %f, %f, %f\r\n", matrixB[0], matrixB[1], matrixB[2],
               matrixB[3], matrixB[4], matrixB[5]);
        err =  1;
        goto end;
    }
    
    // compute the radii and offsets from the results
    estimatedOffset[0] = (float) -matrixB[3]/matrixB[0];
    estimatedOffset[1] = (float) -matrixB[4]/matrixB[1];
    estimatedOffset[2] = (float) -matrixB[5]/matrixB[2];
    
    gamma = 1 + ( matrixB[3]*matrixB[3] / matrixB[0] + matrixB[4]*matrixB[4] / matrixB[1] + matrixB[5]*matrixB[5] / matrixB[2] );
    estimatedScaling[0] = (float) sqrt(gamma/matrixB[0]);
    estimatedScaling[1] = (float) sqrt(gamma/matrixB[1]);
    estimatedScaling[2] = (float) sqrt(gamma/matrixB[2]);
    
    // compute the condition number
    calData->conditionNumber = vectorS[0]/vectorS[5];
    
    // cook extra data (for displaying/debugging purposes)
    cookCalibrationData(calData, estimatedOffset, estimatedScaling);
    
    // copy the coefficients of the direct solution in the output
    for( i=0; i<6; i++) quadricCoefficients[i] = matrixB[i];
    
end:
    /* Free workspace */
#ifdef __MACH__  // if mach (mac os X)
    free( (void*)work );
#else
#if defined(_WIN32) || defined(_WIN64)
    free(matrixA);
    free(matrixB);
#endif
#endif
    
    return err;
}


//=====================================================================================================
// function quadricFit
//=====================================================================================================
//
// find the 9 coefficients of a set of raw data (Nx3), assumed to be on a quadric
// The calculation is done in double precision, the result is converted to single.
// this corresponds to the general form of a rotated ellipsoid, except that quadrics
// are not necessarily ellipsoids
//
// Do not compute radii and offsets.
//
// method:
// least-squares optimization on the linearized problem (after variable changes):
// a*X^2 + b*Y^2 + c*Z^2 + d*X*Y + e*X*Z + f*Y*Z + g*2*X + h*2*Y + i*2*Z = 1;
//
//
// returns 1 (error) if calibration failed
int quadricFit(calibrationData* calData, double *quadricCoefficients) {
    int i;
    char err = 0;
    
    // definitions
#ifdef __MACH__  // if mach (mac os X)
    // constants
    __CLPK_integer one = 1, nine=9;
    double rcond = 1/MAX_CONDITION_NUMBER; // reverse maximum condition number
    
    double matrixA[calData->numberOfSamples*9]; // internal input matrix A
    double matrixB[calData->numberOfSamples]; // internal input matrix B (column mit ones), output = solution
    double vectorS[9]; //output = singular values
    __CLPK_integer rank; // effective rank of the matrix
    double wkopt;
    double *work;
    __CLPK_integer n = calData->numberOfSamples;
    __CLPK_integer lda = calData->numberOfSamples, ldb = calData->numberOfSamples;
    __CLPK_integer lwork, info;
#else
#if defined(_WIN32) || defined(_WIN64)
    // constants
    double rcond = 1/MAX_CONDITION_NUMBER; // reverse maximum condition number
    
    double *matrixA = (double*) malloc(calData->numberOfSamples*9*sizeof(double)); // internal input matrix A
    double *matrixB = (double*) malloc(calData->numberOfSamples*sizeof(double)); // internal input matrix B (column mit ones), output = solution
    double vectorS[9]; //output = singular values
    lapack_int rank; // effective rank of the matrix
    int n = calData->numberOfSamples;
    int lda = calData->numberOfSamples, ldb = calData->numberOfSamples;
#endif
#endif
    
    
    // Build the matrix D (rows = X^2, Y^2, Z^2, X*Y, X*Z, Y*Z, 2*X, 2*Y, 2*Z) and the matrix ONES (N*1)
    for(i=0; i<calData->numberOfSamples; i++) {
        matrixA[0*calData->numberOfSamples+i] = calData->rawSamples[i][0] * calData->rawSamples[i][0];
        matrixA[1*calData->numberOfSamples+i] = calData->rawSamples[i][1] * calData->rawSamples[i][1];
        matrixA[2*calData->numberOfSamples+i] = calData->rawSamples[i][2] * calData->rawSamples[i][2];
        
        matrixA[3*calData->numberOfSamples+i] = calData->rawSamples[i][0] * calData->rawSamples[i][1];
        matrixA[4*calData->numberOfSamples+i] = calData->rawSamples[i][0] * calData->rawSamples[i][2];
        matrixA[5*calData->numberOfSamples+i] = calData->rawSamples[i][1] * calData->rawSamples[i][2];
        
        matrixA[6*calData->numberOfSamples+i] = 2 * calData->rawSamples[i][0];
        matrixA[7*calData->numberOfSamples+i] = 2 * calData->rawSamples[i][1];
        matrixA[8*calData->numberOfSamples+i] = 2 * calData->rawSamples[i][2];
        
        matrixB[i] = 1;
    }
    
    
#ifdef __MACH__  // if mach (mac os X)
    /* Query and allocate the optimal workspace */
    lwork = -1;
    dgelss_(&n, &nine, &one, matrixA, &lda, matrixB, &ldb, vectorS, &rcond, &rank, &wkopt, &lwork, &info);
    lwork = (int)wkopt;
    work = (double*) malloc( lwork*sizeof(double) );
    /* Solve the equations A*X = B */
    dgelss_(&n, &nine, &one, matrixA, &lda, matrixB, &ldb, vectorS, &rcond, &rank, work, &lwork, &info);
    
    /* Check for the full rank */
    /*if( info > 0 ) {
     printf( "The diagonal element %i of the triangular factor ", (int) info );
     printf( "of A is zero, so that A does not have full rank;\r\n" );
     printf( "the least squares solution could not be computed.\r\n" );
     return 1;
     }*/
#else
#if defined(_WIN32) || defined(_WIN64)
    LAPACKE_dgelss( LAPACK_COL_MAJOR, n, 6, 1, matrixA, lda, matrixB, ldb, vectorS, rcond, &rank );
#endif
#endif
    
    // check if the rank is different than 6
    if( rank != 9 ) {
        printf( "The matrix is not of full rank\r\n ");
        printf( "The least squares solution could not be computed.\r\n" );
        err =  1;
        goto end;
    }
    
    
    // check if the condition number is bigger than the allowed maximum
    if( vectorS[0]/vectorS[5] >= MAX_CONDITION_NUMBER ) {
        printf( "The condition number (%f) is bigger than the allowed maximum (%d)\r\n ", vectorS[0]/vectorS[5], MAX_CONDITION_NUMBER );
        err =  1;
        goto end;
    }
    
    // check if the estimated quadric is an ellipsoid
    if( abs(sign(matrixB[0])+sign(matrixB[1])+sign(matrixB[2]))!=3 ) {
        // if the 3 first coefficients of the estimated quadric are not of the same sign
        // the estimated quadric is not an ellipsoid
        printf( "The estimated quadric is not an ellipsoid\r\n");
        printf( "debug output: %f, %f, %f, %f, %f, %f, %f, %f, %f\r\n", matrixB[0], matrixB[1], matrixB[2],
               matrixB[3], matrixB[4], matrixB[5],
               matrixB[6], matrixB[7], matrixB[8] );
        err =  1;
        goto end;
    }

    // copy the coefficients of the direct solution in the output
    for( i=0; i<9; i++) quadricCoefficients[i] = matrixB[i];
    
end:
    /* Free workspace */
#ifdef __MACH__  // if mach (mac os X)
    free( (void*)work );
#else
#if defined(_WIN32) || defined(_WIN64)
    free(matrixA);
    free(matrixB);
#endif
#endif
    
    return err;
}

//=====================================================================================================
// function filterCalData
//=====================================================================================================
//
// filter a data set assuming a gaussian law for the distribution of the distances to a precomputed center
// all samples for which the difference between the distance to the center and the average distance
// is bigger than 3 times the standard deviation
int filterCalData(calibrationData *inCalData, calibrationData *outCalData, float center[3]) {
    int i;
    float *distance;
    float dist_mean, dist_STD;
    
    // compute distance to the average
    distance = (float*) malloc(inCalData->numberOfSamples*sizeof(float));
    for( i =0; i<inCalData->numberOfSamples; i++)
        distance[i] = sqrt((inCalData->rawSamples[i][0]-center[0])*(inCalData->rawSamples[i][0]-center[0])
                           +(inCalData->rawSamples[i][1]-center[1])*(inCalData->rawSamples[i][1]-center[1])
                           +(inCalData->rawSamples[i][2]-center[2])*(inCalData->rawSamples[i][2]-center[2]));
    
    // compute standard deviation and mean value of the distance
    dist_mean = getMean1f( distance, inCalData->numberOfSamples);
    dist_STD = getStdDev1f( distance, inCalData->numberOfSamples, dist_mean);
    
    
    // filter out all points from raw data which distance to average X0 is bigger than 3 times
    // the standard deviation of the distance (if it were gaussian, keeps 99.7% of the data)
    outCalData->numberOfSamples = 0;
    for (i=0;i<inCalData->numberOfSamples; i++) {
        if(abs(distance[i]-dist_mean) < 3*dist_STD) {
            outCalData->rawSamples[outCalData->numberOfSamples][0] = inCalData->rawSamples[i][0];
            outCalData->rawSamples[outCalData->numberOfSamples][1] = inCalData->rawSamples[i][1];
            outCalData->rawSamples[outCalData->numberOfSamples][2] = inCalData->rawSamples[i][2];
            outCalData->numberOfSamples++;
        }
    }
    
    return 0;
}




//=====================================================================================================
// function cookCalibrationData
//=====================================================================================================
//
// cook extra data (calibrated recorded data, norm, maximum norm error) for visualisation/debugging based on raw data and estimated parameters
void cookCalibrationData(calibrationData* calData, float* estimatedOffset, float* estimatedScaling) {
    int i;
    float invEstimatedScaling[3];
    
    invEstimatedScaling[0] = 1.0f / estimatedScaling[0];
    invEstimatedScaling[1] = 1.0f / estimatedScaling[1];
    invEstimatedScaling[2] = 1.0f / estimatedScaling[2];
    
    
    calData->maxNormError = 0;
    for(i=0;i<calData->numberOfSamples;i++) {
        // calibrated samples
        calData->calSamples[i][0] = (calData->rawSamples[i][0]-estimatedOffset[0]) * invEstimatedScaling[0];
        calData->calSamples[i][1] = (calData->rawSamples[i][1]-estimatedOffset[1]) * invEstimatedScaling[1];
        calData->calSamples[i][2] = (calData->rawSamples[i][2]-estimatedOffset[2]) * invEstimatedScaling[2];
        
        // norm, norm average, norm standard deviation and max norm error update
        calData->dataNorm[i] = sqrt(calData->calSamples[i][0]*calData->calSamples[i][0]
                                    + calData->calSamples[i][1]*calData->calSamples[i][1]
                                    + calData->calSamples[i][2]*calData->calSamples[i][2]);
        
        calData->maxNormError = max( calData->maxNormError, fabsf(calData->dataNorm[i] - 1) );
    }
    
    calData->normAverage = getMean1f( calData->dataNorm, calData->numberOfSamples);
    calData->normStdDev = getStdDev1f( calData->dataNorm, calData->numberOfSamples, calData->normAverage);
}