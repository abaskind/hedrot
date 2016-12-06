//
//  hedrotReceiverDemo_cmd.c
//
//  Created by Alexis Baskind on 16/10/16.
//  Copyright (c) 2016 Alexis Baskind. All rights reserved.
//

#include <stdio.h>

#include "libhedrot.h"
#include "hedrot_comm_protocol.h"

// time constants
#define TICK_PERIOD             .1 // time period in seconds between two ticks

//=====================================================================================================
// definitions and includes for clocking
//=====================================================================================================
#ifdef __MACH__ // if mach (mac os X)
#include <mach/clock.h>
#include <mach/mach.h>
double getTime() {
    clock_serv_t cclock;
    mach_timespec_t mts;
    host_get_clock_service(mach_host_self(), SYSTEM_CLOCK, &cclock);
    clock_get_time(cclock, &mts);
    mach_port_deallocate(mach_task_self(), cclock);
    return mts.tv_sec + mts.tv_nsec*1e-9;
}
#else if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
double getTime() {
	LARGE_INTEGER frequency;
    LARGE_INTEGER time;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&time);
    return time.QuadPart / (double)frequency.QuadPart;
}
#endif


int main(int argc, const char * argv[]) {
    double currentTime1, currentTime2, previousTime;
    char messageNumber;
	int i;

    headtrackerData* trackingData = headtracker_new();
    
    // change baudrate (for some reason the command-line version does not accept higher baud rates than 57600)
    trackingData->serialcomm->baud = 57600;
    
    // set verbose to 1
    setVerbose(trackingData,1);
    
    // set autodiscover to 1
    setAutoDiscover(trackingData,1);
    
    // switch on the headtracker
    setHeadtrackerOn(trackingData,1);
    //headtracker_open(trackingData,1); // if autodiscover = 0
    
    previousTime = getTime();
    
    while(1) { //infinite loop
        currentTime1 = getTime();
        
        // next tick
        headtracker_tick(trackingData);
        
        // process messages to notify
        while( (messageNumber = pullNotificationMessage(trackingData)) ) {
            switch( messageNumber ) {
                case NOTIFICATION_MESSAGE_COMM_PORT_LIST_UPDATED:
                    printf("port list updated, %d ports found\r\n", trackingData->serialcomm->numberOfAvailablePorts);
					for(i = 0;i<trackingData->serialcomm->numberOfAvailablePorts; i++) {
						printf("\t port %d: %s \r\n", i, trackingData->serialcomm->availablePorts[i]);
					}
                    break;
                case NOTIFICATION_MESSAGE_PORT_OPENED:
                    printf("port %s opened\r\n", trackingData->serialcomm->availablePorts[trackingData->serialcomm->portNumber]);
                    break;
                case NOTIFICATION_MESSAGE_WRONG_FIRMWARE_VERSION:
                    printf("Wrong Headtracker Firmware Version - expected version: %i - actual version: %i\r\n",HEDROT_FIRMWARE_VERSION,trackingData->firmwareVersion);
                    break;
                case NOTIFICATION_MESSAGE_HEADTRACKER_STATUS_CHANGED:
                    printf("new reception status: %d\r\n",trackingData->infoReceptionStatus);
                    break;
                case NOTIFICATION_MESSAGE_SETTINGS_DATA_READY:
                    printf("headtracking settings received\r\n");
                    break;
                case NOTIFICATION_MESSAGE_CALIBRATION_NOT_VALID:
                    printf("calibration not valid\r\n");
                    break;
                case NOTIFICATION_MESSAGE_GYRO_CALIBRATION_STARTED:
                    printf("gyroscope calibration started\r\n");
                    break;
                case NOTIFICATION_MESSAGE_GYRO_CALIBRATION_FINISHED:
                    printf("gyroscope calibration finished\r\n");
                    break;
                case NOTIFICATION_MESSAGE_BOARD_OVERLOAD:
                    printf("board too slow, reduce samplerate\r\n");
                    break;
            }
        }
        currentTime2 = getTime();
        
        
        // print estimated angles if transmitting
        if(trackingData->infoReceptionStatus == COMMUNICATION_STATE_HEADTRACKER_TRANSMITTING)
            printf("estimated angles: yaw %f - pitch %f - roll %f. time elapsed since last tick = %f sec\r\n", trackingData->yaw, trackingData->pitch, trackingData->roll, currentTime2 - previousTime);
        
        previousTime = currentTime2;
        
        // sleep so that the next tick starts TICK_PERIOD later
#if defined(_WIN32) || defined(_WIN64)
        Sleep((DWORD) (TICK_PERIOD*1000));
#else /* #if defined(_WIN32) || defined(_WIN64) */
        usleep((int) ((TICK_PERIOD + currentTime2 - currentTime1) * 1000000));
#endif /* #if defined(_WIN32) || defined(_WIN64) */
    }
    
    return 0;
}
