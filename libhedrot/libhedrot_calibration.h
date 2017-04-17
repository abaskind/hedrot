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


int ellipsoidFit(short rawData[][3], int numberOfSamples, float* accOffset, float* accScaling);


#endif /* defined(__hedrot_receiver__libhedrot_calibration__) */
