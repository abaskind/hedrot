//
// libhedrot.c
// all functions for communicating with the headtracker, receiving the raw data and cook the angles
//
// Part of code is derived from Sebastian Madgwick's open-source gradient descent angle estimation algorithm
//
// Copyright 2016 Alexis Baskind


//---------------------------------------------------------------------------------------------------
// Header files

#include "libhedrot.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


//=====================================================================================================
// definitions and includes for clocking
//=====================================================================================================
#ifdef __MACH__ // if mach (mac os X)
#include <mach/clock.h>
#include <mach/mach.h>
double get_monotonic_time() {
    clock_serv_t cclock;
    mach_timespec_t mts;
    host_get_clock_service(mach_host_self(), SYSTEM_CLOCK, &cclock);
    clock_get_time(cclock, &mts);
    mach_port_deallocate(mach_task_self(), cclock);
    return mts.tv_sec + mts.tv_nsec*1e-9;
}
#else 
#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
double get_monotonic_time() {
	LARGE_INTEGER frequency;
    LARGE_INTEGER time;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&time);
    return time.QuadPart / (double)frequency.QuadPart;
}
#endif /* #if defined(_WIN32) || defined(_WIN64) */
#endif /* #ifdef __MACH__ */


//=====================================================================================================
// "public" function declarations
//=====================================================================================================

//=====================================================================================================
// function headtracker_new
//=====================================================================================================
//
// create a new headtracker structure
//
headtrackerData* headtracker_new() {
    // allocate memory for the main structure
    headtrackerData* trackingData = (headtrackerData*) malloc(sizeof(headtrackerData));
    
    // allocate memory for the serial comm structure
    trackingData->serialcomm = (headtrackerSerialcomm*) malloc(sizeof(headtrackerSerialcomm));
    
    headtracker_init(trackingData);
    
    // default values (headtracker)
    trackingData->verbose = 0;
    trackingData->headtracker_on = 0;
    trackingData->autoDiscover = 0;
    trackingData->samplerate = 1000;
    trackingData->samplePeriod = .001f; // 1 / trackingData->samplerate
    
    trackingData->gyroDataRate = 0;
    trackingData->gyroClockSource = 1;
    trackingData->gyroDLPFBandwidth = 1;
    
    trackingData->accRange = 2;
    trackingData->accHardOffset[0] = 0;
    trackingData->accHardOffset[1] = 0;
    trackingData->accHardOffset[2] = 0;
    trackingData->accFullResolutionBit = 1;
    trackingData->accDataRate = 13;
    
    trackingData->magMeasurementBias = 0;
    trackingData->magSampleAveraging = 0;
    trackingData->magDataRate = 6;
    trackingData->magGain = 4;
    trackingData->magMeasurementMode = 1;
    
    // default calibration attributes (receiver but saved in the headtracker)
    trackingData->accOffset[0] = 0;
    trackingData->accOffset[1] = 0;
    trackingData->accOffset[2] = 0;
    trackingData->accScaling[0] = 1;
    trackingData->accScaling[1] = 1;
    trackingData->accScaling[2] = 1;
    trackingData->accScalingFactor[0] = 1;
    trackingData->accScalingFactor[1] = 1;
    trackingData->accScalingFactor[2] = 1;
    trackingData->magOffset[0] = 0;
    trackingData->magOffset[1] = 0;
    trackingData->magOffset[2] = 0;
    trackingData->magScaling[0] = 1;
    trackingData->magScaling[1] = 1;
    trackingData->magScaling[2] = 1;
    trackingData->magScalingFactor[0] = 1;
    trackingData->magScalingFactor[1] = 1;
    trackingData->magScalingFactor[2] = 1;
    
    // default values (receiver)
    // gyro autocalibration attributes
    trackingData->gyroOffsetAutocalTime = 2000; //ms
    trackingData->gyroOffsetAutocalThreshold = 200; //LSB
    trackingData->gyroOffsetAutocalOn = 1;
    
    
    // default filter coefficients and internal variables
    trackingData->MadgwickBetaMax = 2.5;
    trackingData->MadgwickBetaGain = 1;
    setAccLPtimeConstant(trackingData, .01f); // default time constant 10 ms
    trackingData->axesReference = 0;
    trackingData->rotationOrder = 0;
    trackingData->invertRotation = 0;
    
    trackingData->gyroOffsetCalibratedState = 0;
    resetGyroOffsetCalibration(trackingData);
    
    headtracker_setReceptionStatus(trackingData, COMMUNICATION_STATE_NO_CONNECTED_HEADTRACKER);
    
    return trackingData;
}


//=====================================================================================================
// function headtracker_init
//=====================================================================================================
//
// initialize the headtracker receiver object
//
void headtracker_init(headtrackerData *trackingData) {
    
    headtracker_setReceptionStatus(trackingData,COMMUNICATION_STATE_NO_CONNECTED_HEADTRACKER);
    
    serial_comm_init(trackingData->serialcomm);
    
    // init ping scheduler
    trackingData->scheduledNextPingTime = 0;
    
    // init FIFO list of messages to notify
    trackingData->numberOfMessages = 0;
    trackingData->firstMessageToNotify = 0;
    
    // reset reference quaternion
    trackingData->qref1 = 1;
    trackingData->qref2 = 0;
    trackingData->qref3 = 0;
    trackingData->qref4 = 0;
    
    // init the calibration values
    trackingData->calibrationValid = 0;
    
    // reset temporary variables
    trackingData->gyroHalfScaleSensitivity = -1;
    trackingData->gyroBitDepth = -1;
}


//=====================================================================================================
// function headtracker_list_comm_ports
//=====================================================================================================
//
// update comm ports lists and notify the host
//
void headtracker_list_comm_ports(headtrackerData *trackingData) {
    list_comm_ports(trackingData->serialcomm);
    pushNotificationMessage(trackingData, NOTIFICATION_MESSAGE_COMM_PORT_LIST_UPDATED);
}



//=====================================================================================================
// function export_headtracker_settings
//=====================================================================================================
//
// dump all headtracker settings (sensors and receiver) in a text file
//
// the format for each line is:
//      parameter_name, value1 (value2 value3);
//
// returns 1 if no error
// returns 0 if error
//
int export_headtracker_settings(headtrackerData *trackingData, char* filename) {
	FILE *fd;
    // open file for writing, returns 0 if it fails
#if defined(_WIN32) || defined(_WIN64)
    fopen_s( &fd, filename, "w");
#else /* #if defined(_WIN32) || defined(_WIN64) */
	fd = fopen( filename, "w");
#endif /* #if defined(_WIN32) || defined(_WIN64) */
    
    if( fd == NULL) {
        printf("Error: file %s could not be opened for writing", filename);
        pushNotificationMessage(trackingData, NOTIFICATION_MESSAGE_EXPORT_SETTINGS_FAILED);
        return 0;
    }
    
    // dump all values in the text file
    fprintf(fd, "MadgwickBetaMax, %f;\n",           trackingData->MadgwickBetaMax);
    fprintf(fd, "MadgwickBetaGain, %f;\n",          trackingData->MadgwickBetaGain);
    fprintf(fd, "accLPalpha, %f;\n",                trackingData->accLPalpha);
    fprintf(fd, "firmwareVersion, %d;\n",           trackingData->firmwareVersion);
    fprintf(fd, "samplerate, %ld;\n",               trackingData->samplerate);
    fprintf(fd, "gyroDataRate, %d;\n",                  trackingData->gyroDataRate);
    fprintf(fd, "gyroClockSource, %d;\n",           trackingData->gyroClockSource);
    fprintf(fd, "gyroDLPFBandwidth, %d;\n",         trackingData->gyroDLPFBandwidth);
    fprintf(fd, "gyroOffsetAutocalOn, %d;\n",       trackingData->gyroOffsetAutocalOn);
    fprintf(fd, "accRange, %d;\n",                  trackingData->accRange);
    fprintf(fd, "accHardOffset, %d %d %d;\n",       trackingData->accHardOffset[0],trackingData->accHardOffset[1],trackingData->accHardOffset[2]);
    fprintf(fd, "accFullResolutionBit, %d;\n",      trackingData->accFullResolutionBit);
    fprintf(fd, "accDataRate, %d;\n",               trackingData->accDataRate);
    fprintf(fd, "magMeasurementBias, %d;\n",        trackingData->magMeasurementBias);
    fprintf(fd, "magSampleAveraging, %d;\n",        trackingData->magSampleAveraging);
    fprintf(fd, "magDataRate, %d;\n",               trackingData->magDataRate);
    fprintf(fd, "magGain, %d;\n",                   trackingData->magGain);
    fprintf(fd, "magMeasurementMode, %d;\n",        trackingData->magMeasurementMode);
    fprintf(fd, "gyroOffset, %f %f %f;\n",          trackingData->gyroOffset[0],trackingData->gyroOffset[1],trackingData->gyroOffset[2]);
    fprintf(fd, "gyroOffsetAutocalTime, %f;\n",     trackingData->gyroOffsetAutocalTime);
    fprintf(fd, "gyroOffsetAutocalThreshold, %ld;\n",trackingData->gyroOffsetAutocalThreshold);
    fprintf(fd, "accOffset, %f %f %f;\n",           trackingData->accOffset[0],trackingData->accOffset[1],trackingData->accOffset[2]);
    fprintf(fd, "accScaling, %f %f %f;\n",          trackingData->accScaling[0],trackingData->accScaling[1],trackingData->accScaling[2]);
    fprintf(fd, "magOffset, %f %f %f;\n",           trackingData->magOffset[0],trackingData->magOffset[1],trackingData->magOffset[2]);
    fprintf(fd, "magScaling, %f %f %f;\n",          trackingData->magScaling[0],trackingData->magScaling[1],trackingData->magScaling[2]);
    
    
    // close (save) the file, returns 0 if it fails
    if(fclose(fd)) {
        printf("Error: file %s could not be closed properly", filename);
        pushNotificationMessage(trackingData, NOTIFICATION_MESSAGE_EXPORT_SETTINGS_FAILED);
        return 0;
    }
    
    return 1;
}


//=====================================================================================================
// function import_headtracker_settings
//=====================================================================================================
//
// update all headtracker settings (sensors and receiver) from a text file
//
// the format for each line is:
//      parameter_name, value1 (value2 value3);
// returns 1 if no error
// returns 0 if error
//
int import_headtracker_settings(headtrackerData *trackingData, char* filename) {
    char   lineBuffer[200];
    char   *keyBuffer =  NULL;
    char   *valueBuffer = NULL;
    char   *brkt;

	FILE *fd;
    
    // open file for reading, returns 0 if it fails
#if defined(_WIN32) || defined(_WIN64)
    fopen_s( &fd, filename, "r");
#else /* #if defined(_WIN32) || defined(_WIN64) */
	fd = fopen( filename, "r");
#endif /* #if defined(_WIN32) || defined(_WIN64) */
    
    if( fd == NULL) {
        printf("Error: file %s could not be opened for reading", filename);
        pushNotificationMessage(trackingData, NOTIFICATION_MESSAGE_IMPORT_SETTINGS_FAILED);
        return 0;
    }
    
    // get all values from the text file with fprintf, returns 0 if it fails
    while (fgets(lineBuffer, 200, fd) != NULL) {
        if((keyBuffer=strtok_r(lineBuffer, ", ", &brkt)) == NULL) {
            printf("Error: syntax error for line <<%s>> (1)\r\n", lineBuffer);
            pushNotificationMessage(trackingData, NOTIFICATION_MESSAGE_IMPORT_SETTINGS_FAILED);
            return 0;
        } else {
            if((valueBuffer=strtok_r(NULL, ";", &brkt)) == NULL) {
                printf("Error: syntax error for line <<%s>> (2)\r\n", lineBuffer);
                pushNotificationMessage(trackingData, NOTIFICATION_MESSAGE_IMPORT_SETTINGS_FAILED);
                return 0;
            } else {
                if(trackingData->verbose) printf("key: %s, value: %s\r\n", keyBuffer, valueBuffer);
                
                // process key/value pair
                if(!processKeyValueSettingPair(trackingData, keyBuffer, valueBuffer, 1)) {
                    if(trackingData->verbose) printf("parsing error on key/value pair: %s/%s\r\n",keyBuffer, valueBuffer);
                    pushNotificationMessage(trackingData, NOTIFICATION_MESSAGE_IMPORT_SETTINGS_FAILED);
                    return 0;
                }
            }
        }
    }
    
    // close the file, returns 0 if it fails
    if(fclose(fd)) {
        printf("Error: file %s could not be closed properly", filename);
        pushNotificationMessage(trackingData, NOTIFICATION_MESSAGE_IMPORT_SETTINGS_FAILED);
        return 0;
    }
    
    // file read successfully, request head tracker info for double-checking
    headtracker_requestHeadtrackerSettings(trackingData);
    
    return 1;
}



//=====================================================================================================
// function headtracker_tick
//=====================================================================================================
//
// main routine of the headtracker receiver
//
void headtracker_tick(headtrackerData *trackingData) {
    int readBufferInfoOffset = 0; // index of the first value in the read buffer to be taken into account (the one right after H2R_START_TRANSMIT_INFO_CHAR) by processInfoFromHeadtracker
    
    
    if(trackingData->infoReceptionStatus < COMMUNICATION_STATE_AUTODISCOVERING_HEADTRACKER_FOUND){ //headtracker not connected to the receiver yet
        if(trackingData->autoDiscover) {
            // if no ports are found, make a new list of it
            if(trackingData->serialcomm->numberOfAvailablePorts == 0) {
                if(trackingData->verbose) printf("[hedrot]: looking for available ports\r\n");
                headtracker_list_comm_ports(trackingData);
            }
            
            // then autodiscover
            headtracker_autodiscover(trackingData);
        }
    } else { // trackingData->infoReceptionStatus >= COMMUNICATION_STATE_AUTODISCOVERING_HEADTRACKER_FOUND => a headtracker is connected
        // get current time
        
        double current_time = get_monotonic_time();

		unsigned int i;
        
		unsigned char message; // for single bytes to be sent to the head tracker

        // check if a new scheduled ping is necessary
        if(current_time >= trackingData->scheduledNextPingTime) {
			message = R2H_PING_CHAR;
            if(write_serial(trackingData->serialcomm,&message, 1) == 1) {
                //printf("ping sent, delay since last ping %f sec \r\n", current_time - trackingData->scheduledNextPingTime + PINGTIME);
                
                // write ping successful, schedule a new one
                trackingData->scheduledNextPingTime = current_time + PINGTIME;
            } else { // error while sending ping => init
                if(trackingData->verbose) printf("error while sending ping \r\n");
                headtracker_init(trackingData);
            }
        }
        
        if( trackingData->serialcomm->comhandle == INVALID_HANDLE_VALUE) return; //error
        init_read_serial(trackingData->serialcomm);
        
        while(is_data_available(trackingData->serialcomm)) { // if bytes are available for reading
            //printf("%ld bytes read\r\n", numberOfReadBytes);
            for(i = 0; i<trackingData->serialcomm->numberOfReadBytes; i++) {
                if(trackingData->verbose == VERBOSE_STATE_ALL_MESSAGES) {
                    printf( "[hedrot] : byte received = %c\r\n",trackingData->serialcomm->readBuffer[i]);
                }
                if(trackingData->serialcomm->readBuffer[i]==H2R_DATA_RECEIVE_ERROR_CHAR) { //if the headtracker report an error by receiving
                    printf("[hedrot] : the headtracker reports a receive error\r\n");
                } else {
                    switch (trackingData->infoReceptionStatus) {
                        case COMMUNICATION_STATE_WAITING_FOR_INFO:
                            //check if the headtracker has started transmitting the info
                            if(trackingData->serialcomm->readBuffer[i]==H2R_START_TRANSMIT_INFO_CHAR) {
                                headtracker_setReceptionStatus(trackingData,COMMUNICATION_STATE_RECEIVING_INFO);
                                readBufferInfoOffset = i+1;
                            }
                            break;
                        case COMMUNICATION_STATE_RECEIVING_INFO:
                            //check if the headtracker has finished transmitting the info
                            if(trackingData->serialcomm->readBuffer[i]==H2R_STOP_TRANSMIT_INFO_CHAR) {
                                if(processInfoFromHeadtracker(trackingData, readBufferInfoOffset, i+1)) { // is the info stream sent by the headtracker valid?
                                    trackingData->rawDataBufferIndex = 0; //reset the counter
                                } else {
                                    //change back to state 1, which means that we will request the info once more at the end of the loop
                                    headtracker_setReceptionStatus(trackingData,COMMUNICATION_STATE_WAITING_FOR_INFO);
                                }
                            } else if(trackingData->serialcomm->readBuffer[i]==H2R_PING_CHAR) {
                                //if the headtracker responds to the ping, it means we can start to transmit
                                headtracker_setReceptionStatus(trackingData,COMMUNICATION_STATE_HEADTRACKER_TRANSMITTING);
                            } else {
                                // info still transmitting, do nothing
                            }
                            break;
                        case COMMUNICATION_STATE_HEADTRACKER_TRANSMITTING:
                            //check if the MSB of the current byte is 0 (meaning "frame end" or any other message)
                            if((trackingData->serialcomm->readBuffer[i]&128)==0) {
                                //is it the end of the frame?
                                if(trackingData->serialcomm->readBuffer[i]==H2R_END_OF_RAWDATA_FRAME) {
                                    //check that the number of received bytes is correct
                                    if(trackingData->rawDataBufferIndex==NUMBER_OF_BYTES_IN_RAWDATA_FRAME) {
                                            
                                        headtracker_compute_data(trackingData);
                                            
                                        trackingData->trackingDataReady = 1;
                                    } else {
                                        if(trackingData->verbose) {
                                            printf( "[hedrot] : bad stream (%d elements instead of %d)\r\n",trackingData->rawDataBufferIndex, NUMBER_OF_BYTES_IN_RAWDATA_FRAME);
                                        }
                                    }
                                    trackingData->rawDataBufferIndex = 0; //reset the counter
                                } else if(trackingData->serialcomm->readBuffer[i]==H2R_BOARD_OVERLOAD) {
                                    // error: teensy overloaded
                                    pushNotificationMessage(trackingData, NOTIFICATION_MESSAGE_BOARD_OVERLOAD);
                                }
                            } else { //MSB = 1, raw headtracking data, store in the buffer
                                trackingData->rawDataBuffer[trackingData->rawDataBufferIndex++]=trackingData->serialcomm->readBuffer[i];
                            }
                            break;
                        default: //error
                            break;
                    }
                }
            }
        }
    }
}


//=====================================================================================================
// function headtracker_requestHeadtrackerSettings
//=====================================================================================================
//
// request the settings from the head tracker (stored in EEPROM)
//
void headtracker_requestHeadtrackerSettings(headtrackerData *trackingData) {
	unsigned char message; // for single byte to be sent to the head tracker
    // if it's not the case, set the state to 1:
    if (trackingData->infoReceptionStatus!=COMMUNICATION_STATE_WAITING_FOR_INFO)
        headtracker_setReceptionStatus(trackingData,COMMUNICATION_STATE_WAITING_FOR_INFO);
    
    if(trackingData->verbose) printf("[hedrot] requesting info\r\n");
	message = R2H_SEND_INFO_CHAR;
    write_serial(trackingData->serialcomm, &message, 1);
}




//=====================================================================================================
// function processInfoFromHeadtracker
//=====================================================================================================
//
// process and store the received head tracker settings
// returns 1 if no error
// returns 0 if error
//
int processInfoFromHeadtracker(headtrackerData *trackingData, int offset, int numberOfBytes) {
    
    char   keyBuffer[RAWDATA_STRING_MAX_SIZE];
    char   valueBuffer[RAWDATA_STRING_MAX_SIZE];
	int n,i;
    
    int keyBufferIndex = 0;
    int valueBufferIndex = 0;
    int isValueFlag = 0; //0 means "still aquiring the key name", 1 means "acquiring the value name"
    
	for(n=0;n<RAWDATA_STRING_MAX_SIZE;n++) {
        keyBuffer[n] = '\0';
        valueBuffer[n] = '\0';
    }

    trackingData->calibrationValid = 1; //initialize the "calibration valid" flag
    
    if(trackingData->verbose) printf("headtracker info (%d bytes): \r\n", numberOfBytes);
    
    for(i=offset;i<numberOfBytes;i++) {
        if(isValueFlag==0) { //acquiring the key name
            if(trackingData->serialcomm->readBuffer[i] == SPACE) {
                //stop acquiring the key name
                isValueFlag = 1;
            } else {
                keyBuffer[keyBufferIndex]=trackingData->serialcomm->readBuffer[i];
                keyBufferIndex++;
            }
        } else if(isValueFlag==1) { //acquiring the value(s)
            if((trackingData->serialcomm->readBuffer[i] == COMMA) || (i == numberOfBytes-1)) { // the comma at the end is not mandatory
                //stop acquiring the key/value pair, process it and go to the next one
                
                if(trackingData->verbose) printf("key received: %s - value received %s\r\n",keyBuffer, valueBuffer);
                
                if(!processKeyValueSettingPair(trackingData, keyBuffer, valueBuffer, 0)) {
                    if(trackingData->verbose) printf("parsing error on key/value pair: %s/%s\r\n",keyBuffer, valueBuffer);
                    pushNotificationMessage(trackingData, NOTIFICATION_MESSAGE_SETTINGS_DATA_TRANSMISSION_FAILED);
                    return 0;
                }
                    
                
                isValueFlag = 0;
                keyBufferIndex = 0;
                valueBufferIndex = 0;
                for(n=0;n<RAWDATA_STRING_MAX_SIZE;n++) {
                    keyBuffer[n]='\0';
                    valueBuffer[n]='\0';
                }
            } else {
                valueBuffer[valueBufferIndex]=trackingData->serialcomm->readBuffer[i];
                valueBufferIndex++;
            }
        }
    }
    
    if((trackingData->gyroHalfScaleSensitivity!=-1) & (trackingData->gyroBitDepth!=-1)) {
        trackingData->gyroscopeCalibrationFactor =  trackingData->gyroHalfScaleSensitivity * M_PI_float / 180.0f / (float) pow((double) 2,(int) trackingData->gyroBitDepth-1);
        if(trackingData->verbose) printf("gyroscopeCalibrationFactor: %f\r\n", trackingData->gyroscopeCalibrationFactor);
    } else {
        
        if(trackingData->verbose) printf("cannot determine gyroscopeCalibrationFactor\r\n");
        pushNotificationMessage(trackingData, NOTIFICATION_MESSAGE_SETTINGS_DATA_TRANSMISSION_FAILED);
        return 0;
    }
    
    pushNotificationMessage(trackingData, NOTIFICATION_MESSAGE_SETTINGS_DATA_READY);
    
    return 1;
}



//=====================================================================================================
// function resetGyroOffsetCalibration
//=====================================================================================================
//
// reset the internal variables for the gyro auto calibration
//
void resetGyroOffsetCalibration(headtrackerData *trackingData) {
    trackingData->gyroOffsetAutocalCounter = 0;
    trackingData->gyroOffsetAutocalMin[0] = 10000000;
    trackingData->gyroOffsetAutocalMin[1] = 10000000;
    trackingData->gyroOffsetAutocalMin[2] = 10000000;
    trackingData->gyroOffsetAutocalMax[0] = -10000000;
    trackingData->gyroOffsetAutocalMax[1] = -10000000;
    trackingData->gyroOffsetAutocalMax[2] = -10000000;
    trackingData->gyroOffsetAutocalSum[0] = 0;
    trackingData->gyroOffsetAutocalSum[1] = 0;
    trackingData->gyroOffsetAutocalSum[2] = 0;
    
    trackingData->gyroOffset[0] = 0;
    trackingData->gyroOffset[1] = 0;
    trackingData->gyroOffset[2] = 0;
}



//=====================================================================================================
// function gyroOffsetCalibration
//=====================================================================================================
//
// calibrate the zero of the gyroscope
//
void gyroOffsetCalibration(headtrackerData *trackingData) {
    //gyro offset autocalibration
    // update min, max, sum and counter
    trackingData->gyroOffsetAutocalMax[0] = max(trackingData->gyroOffsetAutocalMax[0], trackingData->gyroRawData[0]);
    trackingData->gyroOffsetAutocalMax[1] = max(trackingData->gyroOffsetAutocalMax[1], trackingData->gyroRawData[1]);
    trackingData->gyroOffsetAutocalMax[2] = max(trackingData->gyroOffsetAutocalMax[2], trackingData->gyroRawData[2]);
    trackingData->gyroOffsetAutocalMin[0] = min(trackingData->gyroOffsetAutocalMin[0], trackingData->gyroRawData[0]);
    trackingData->gyroOffsetAutocalMin[1] = min(trackingData->gyroOffsetAutocalMin[1], trackingData->gyroRawData[1]);
    trackingData->gyroOffsetAutocalMin[2] = min(trackingData->gyroOffsetAutocalMin[2], trackingData->gyroRawData[2]);
    trackingData->gyroOffsetAutocalSum[0] += trackingData->gyroRawData[0];
    trackingData->gyroOffsetAutocalSum[1] += trackingData->gyroRawData[1];
    trackingData->gyroOffsetAutocalSum[2] += trackingData->gyroRawData[2];
    trackingData->gyroOffsetAutocalCounter++;
    
    // if the max-min differences are above the threshold, reset the counter and values
    if((trackingData->gyroOffsetAutocalMax[0]-trackingData->gyroOffsetAutocalMin[0]>trackingData->gyroOffsetAutocalThreshold)
       ||(trackingData->gyroOffsetAutocalMax[1]-trackingData->gyroOffsetAutocalMin[1]>trackingData->gyroOffsetAutocalThreshold)
       ||(trackingData->gyroOffsetAutocalMax[2]-trackingData->gyroOffsetAutocalMin[2]>trackingData->gyroOffsetAutocalThreshold)) {
        resetGyroOffsetCalibration(trackingData);
    }
    
    // check if there are enough stable samples. If yes, update the offsets and the "calibrate" flag
    if(trackingData->gyroOffsetAutocalCounter >= (trackingData->gyroOffsetAutocalTime / 1000. * trackingData->samplerate) ) {
        trackingData->gyroOffset[0] = (trackingData->gyroOffsetAutocalSum[0] * 1.0f)/ trackingData->gyroOffsetAutocalCounter;
        trackingData->gyroOffset[1] = (trackingData->gyroOffsetAutocalSum[1] * 1.0f)/ trackingData->gyroOffsetAutocalCounter;
        trackingData->gyroOffset[2] = (trackingData->gyroOffsetAutocalSum[2] * 1.0f)/ trackingData->gyroOffsetAutocalCounter;
        trackingData->gyroOffsetCalibratedState = 2;
    }
}


void headtracker_compute_data(headtrackerData *trackingData) {
    convert_7bytes_to_3int16(trackingData->rawDataBuffer,0,trackingData->magRawData);
    convert_7bytes_to_3int16(trackingData->rawDataBuffer,7,trackingData->accRawData);
    convert_7bytes_to_3int16(trackingData->rawDataBuffer,14,trackingData->gyroRawData);
    
    // angle estimation
    if(trackingData->calibrationValid) {
        MadgwickAHRSupdateModified(trackingData); //compute cooked data and angles only if calibration is valid
    }
    
    // check if the gyro calibration is done
    if(trackingData->gyroOffsetAutocalOn & trackingData->calibrationValid) {
        switch(trackingData->gyroOffsetCalibratedState) {
            case 0:
                // gyro calibration not started yet, start it now
                trackingData->gyroOffsetCalibratedState = 1;
                gyroOffsetCalibration(trackingData);
                
                // send a message to the output to ask the user to stay still while calibrating the gyro
                pushNotificationMessage(trackingData, NOTIFICATION_MESSAGE_GYRO_CALIBRATION_STARTED);
                break;
            case 1:
                gyroOffsetCalibration(trackingData);
                break;
            case 2:
                // the calibration is finished
                trackingData->gyroOffsetCalibratedState = 3;
                
                // send a message to the output to notify that the calibration is finished
                pushNotificationMessage(trackingData, NOTIFICATION_MESSAGE_GYRO_CALIBRATION_FINISHED);
                break;
        }
    }
    // center according to reference1
    quaternionComposition(trackingData->qref1, trackingData->qref2, trackingData->qref3, trackingData->qref4,
                          trackingData->q1, trackingData->q2, trackingData->q3, trackingData->q4,
                          &trackingData->qcent1, &trackingData->qcent2, &trackingData->qcent3, &trackingData->qcent4);
    
    // change the axes references of the quaternion if it does not fit the standard (X->right, Y->back, Z->down)
    changeQuaternionReference(trackingData);
    
    
    // invert rotation if requested
    if(trackingData->invertRotation) {
        trackingData->qcent2 = - trackingData->qcent2;
        trackingData->qcent3 = - trackingData->qcent3;
        trackingData->qcent4 = - trackingData->qcent4;
    }
    
    // compute euler angles
    switch(trackingData->rotationOrder) {
        case 0:
            quaternion2YawPitchRoll(trackingData->qcent1, trackingData->qcent2, trackingData->qcent3, trackingData->qcent4, &trackingData->yaw, &trackingData->pitch, &trackingData->roll);
            break;
        case 1:
            quaternion2RollPitchYaw(trackingData->qcent1, trackingData->qcent2, trackingData->qcent3, trackingData->qcent4, &trackingData->yaw, &trackingData->pitch, &trackingData->roll);
            break;
    }

}

//=====================================================================================================
// function convert_7bytes_to_3int16
//=====================================================================================================
//
// decode the raw data flow from the headtracker
//
void convert_7bytes_to_3int16(unsigned char *rawDataBuffer,int baseIndex,short *rawDataToSend) {
    // for each sensor with values vx, vy, vz in 16 bits:
    // each of the 7 bytes starts with the bit 1 (it's a unique marker of raw data info)
    // 1/vx_1-vx_7
    // 1/vx_8-vx_14
    // 1/vx_15/vx_16/vy_1-vy_5
    // 1/vy_6-vx_12
    // 1/vy_13-vy_16/vz_1-vz_3
    // 1/vz_4-vy_10
    // 1/vz_11-vy_16/0
    rawDataToSend[0] = ((rawDataBuffer[baseIndex]&127)<<9) + ((rawDataBuffer[baseIndex+1]&127)<<2) + ((rawDataBuffer[baseIndex+2]&127)>>5);
    rawDataToSend[1] = ((rawDataBuffer[baseIndex+2]&31)<<11) + ((rawDataBuffer[baseIndex+3]&127)<<4) + ((rawDataBuffer[baseIndex+4]&127)>>3);
    rawDataToSend[2] = ((rawDataBuffer[baseIndex+4]&7)<<13) + ((rawDataBuffer[baseIndex+5]&127)<<6) + ((rawDataBuffer[baseIndex+6]&127)>>1);
}


//=====================================================================================================
// function MadgwickAHRSupdateModified
//=====================================================================================================
//
// AHRS algorithm update
//
// Modified version of Madgwick's angle estimation algorithm
// See: http://www.x-io.co.uk/node/8#open_source_ahrs_and_imu_algorithms
// Main modifications:
//  . low-pass filter applied to accelerometer data, with variable time constant depending on the movement (gyroscope activity)
//  . beta coefficient (balance between gyroscope and accel/compass incremental estimation) made variable, depending on the movement (gyroscope activity)
//
//=====================================================================================================


char MadgwickAHRSupdateModified(headtrackerData *trackingData) {
    float recipNorm;
    float s1, s2, s3, s4;
    float qDot1, qDot2, qDot3, qDot4;
    float hx, hy;
    float _2q1mx, _2q1my, _2q1mz, _2q2mx, _2bx, _2bz, _4bx, _4bz, _2q1, _2q2, _2q3, _2q4, _2q1q3, _2q3q4, q1q1, q1q2, q1q3, q1q4, q2q2, q2q3, q2q4, q3q3, q3q4, q4q4;
    float a_norm2,m_norm2, gyro_norm2;
    
    float accDataNorm[3], magDataNorm[3];
    
    //scale the gyro data
    trackingData->gyroCalData[0] = (trackingData->gyroRawData[0]-trackingData->gyroOffset[0]) * trackingData->gyroscopeCalibrationFactor;
    trackingData->gyroCalData[1] = (trackingData->gyroRawData[1]-trackingData->gyroOffset[1]) * trackingData->gyroscopeCalibrationFactor;
    trackingData->gyroCalData[2] = (trackingData->gyroRawData[2]-trackingData->gyroOffset[2]) * trackingData->gyroscopeCalibrationFactor;
    
    //scale mag data
    trackingData->magCalData[0]=(trackingData->magRawData[0]-trackingData->magOffset[0]) * trackingData->magScalingFactor[0];
    trackingData->magCalData[1]=(trackingData->magRawData[1]-trackingData->magOffset[1]) * trackingData->magScalingFactor[1];
    trackingData->magCalData[2]=(trackingData->magRawData[2]-trackingData->magOffset[2]) * trackingData->magScalingFactor[2];
    
    //scale acc data
    trackingData->accCalData[0]=(trackingData->accRawData[0]-trackingData->accOffset[0]) * trackingData->accScalingFactor[0];
    trackingData->accCalData[1]=(trackingData->accRawData[1]-trackingData->accOffset[1]) * trackingData->accScalingFactor[1];
    trackingData->accCalData[2]=(trackingData->accRawData[2]-trackingData->accOffset[2]) * trackingData->accScalingFactor[2];
    
    
    // compute the squared norm of the gyro data => rough estimation of the movement
    gyro_norm2 = trackingData->gyroCalData[0] * trackingData->gyroCalData[0]
    + trackingData->gyroCalData[1] * trackingData->gyroCalData[1]
    + trackingData->gyroCalData[2] * trackingData->gyroCalData[2];
    
    // low-pass the accelerometer data with a variable coefficient. If movement, alpha tends to 1 (no smoothing), if no movement, alpha tends to its min (smoothing)
    trackingData->accCalDataLP[0] = trackingData->accLPalpha * trackingData->accCalData[0] + (1 - trackingData->accLPalpha) * trackingData->accLPstate[0];
    trackingData->accCalDataLP[1] = trackingData->accLPalpha * trackingData->accCalData[1] + (1 - trackingData->accLPalpha) * trackingData->accLPstate[1];
    trackingData->accCalDataLP[2] = trackingData->accLPalpha * trackingData->accCalData[2] + (1 - trackingData->accLPalpha) * trackingData->accLPstate[2];
    trackingData->accLPstate[0] = trackingData->accCalDataLP[0]; // filter state update
    trackingData->accLPstate[1] = trackingData->accCalDataLP[1]; // filter state update
    trackingData->accLPstate[2] = trackingData->accCalDataLP[2]; // filter state update
    
    
    // compute squared norms
    m_norm2 =  trackingData->magCalData[0] *  trackingData->magCalData[0] + trackingData->magCalData[1] *  trackingData->magCalData[1] + trackingData->magCalData[2] *  trackingData->magCalData[2];
    a_norm2 =  trackingData->accCalDataLP[0] *  trackingData->accCalDataLP[0] + trackingData->accCalDataLP[1] *  trackingData->accCalDataLP[1] + trackingData->accCalDataLP[2] *  trackingData->accCalDataLP[2];
    
    // return an error if magnetometer or accelerometer measurement invalid (avoids NaN in magnetometer normalisation)
    if(m_norm2==0.0 || a_norm2==0.0) return 1; //returns error
    
    // Normalise accelerometer measurement
    recipNorm = invSqrt(a_norm2);
    accDataNorm[0] = trackingData->accCalDataLP[0] * recipNorm;
    accDataNorm[1] = trackingData->accCalDataLP[1] * recipNorm;
    accDataNorm[2] = trackingData->accCalDataLP[2] * recipNorm;
    
    // Normalise magnetometer measurement
    recipNorm = invSqrt(m_norm2);
    magDataNorm[0] = trackingData->magCalData[0] * recipNorm;
    magDataNorm[1] = trackingData->magCalData[1] * recipNorm;
    magDataNorm[2] = trackingData->magCalData[2] * recipNorm;
    
    // Auxiliary variables to avoid repeated arithmetic
    _2q1mx = 2.0f * trackingData->q1 * magDataNorm[0];
    _2q1my = 2.0f * trackingData->q1 * magDataNorm[1];
    _2q1mz = 2.0f * trackingData->q1 * magDataNorm[2];
    _2q2mx = 2.0f * trackingData->q2 * magDataNorm[0];
    _2q1 = 2.0f * trackingData->q1;
    _2q2 = 2.0f * trackingData->q2;
    _2q3 = 2.0f * trackingData->q3;
    _2q4 = 2.0f * trackingData->q4;
    _2q1q3 = 2.0f * trackingData->q1 * trackingData->q3;
    _2q3q4 = 2.0f * trackingData->q3 * trackingData->q4;
    q1q1 = trackingData->q1 * trackingData->q1;
    q1q2 = trackingData->q1 * trackingData->q2;
    q1q3 = trackingData->q1 * trackingData->q3;
    q1q4 = trackingData->q1 * trackingData->q4;
    q2q2 = trackingData->q2 * trackingData->q2;
    q2q3 = trackingData->q2 * trackingData->q3;
    q2q4 = trackingData->q2 * trackingData->q4;
    q3q3 = trackingData->q3 * trackingData->q3;
    q3q4 = trackingData->q3 * trackingData->q4;
    q4q4 = trackingData->q4 * trackingData->q4;
    
    // Reference direction of Earth's magnetic field
    hx =  magDataNorm[0] * q1q1 - _2q1my * trackingData->q4 + _2q1mz * trackingData->q3 + magDataNorm[0] * q2q2 + _2q2 *  magDataNorm[1] * trackingData->q3 + _2q2 *  magDataNorm[2] * trackingData->q4 - magDataNorm[0] * q3q3 - magDataNorm[0] * q4q4;
    hy = _2q1mx * trackingData->q4 + magDataNorm[1] * q1q1 - _2q1mz * trackingData->q2 + _2q2mx * trackingData->q3 - magDataNorm[1] * q2q2 + magDataNorm[1] * q3q3 + _2q3 *  magDataNorm[2] * trackingData->q4 - magDataNorm[1] * q4q4;
    _2bx = (float) sqrt(hx * hx + hy * hy);
    _2bz = -_2q1mx * trackingData->q3 + _2q1my * trackingData->q2 + magDataNorm[2] * q1q1 + _2q2mx * trackingData->q4 - magDataNorm[2] * q2q2 + _2q3 *  magDataNorm[1] * trackingData->q4 - magDataNorm[2] * q3q3 + magDataNorm[2] * q4q4;
    _4bx = 2.0f * _2bx;
    _4bz = 2.0f * _2bz;
    
    // Gradient decent algorithm corrective step
    s1 = -_2q3 * (2.0f * q2q4 - _2q1q3 - accDataNorm[0]) + _2q2 * (2.0f * q1q2 + _2q3q4 - accDataNorm[1]) - _2bz * trackingData->q3 * (_2bx * (0.5f - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - magDataNorm[0]) + (-_2bx * trackingData->q4 + _2bz * trackingData->q2) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - magDataNorm[1]) + _2bx * trackingData->q3 * (_2bx * (q1q3 + q2q4) + _2bz * (0.5f - q2q2 - q3q3) - magDataNorm[2]);
    s2 = _2q4 * (2.0f * q2q4 - _2q1q3 - accDataNorm[0]) + _2q1 * (2.0f * q1q2 + _2q3q4 - accDataNorm[1]) - 4.0f * trackingData->q2 * (1 - 2.0f * q2q2 - 2.0f * q3q3 - accDataNorm[2]) + _2bz * trackingData->q4 * (_2bx * (0.5f - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - magDataNorm[0]) + (_2bx * trackingData->q3 + _2bz * trackingData->q1) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - magDataNorm[1]) + (_2bx * trackingData->q4 - _4bz * trackingData->q2) * (_2bx * (q1q3 + q2q4) + _2bz * (0.5f - q2q2 - q3q3) - magDataNorm[2]);
    s3 = -_2q1 * (2.0f * q2q4 - _2q1q3 - accDataNorm[0]) + _2q4 * (2.0f * q1q2 + _2q3q4 - accDataNorm[1]) - 4.0f * trackingData->q3 * (1 - 2.0f * q2q2 - 2.0f * q3q3 - accDataNorm[2]) + (-_4bx * trackingData->q3 - _2bz * trackingData->q1) * (_2bx * (0.5f - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - magDataNorm[0]) + (_2bx * trackingData->q2 + _2bz * trackingData->q4) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - magDataNorm[1]) + (_2bx * trackingData->q1 - _4bz * trackingData->q3) * (_2bx * (q1q3 + q2q4) + _2bz * (0.5f - q2q2 - q3q3) - magDataNorm[2]);
    s4 = _2q2 * (2.0f * q2q4 - _2q1q3 - accDataNorm[0]) + _2q3 * (2.0f * q1q2 + _2q3q4 - accDataNorm[1]) + (-_4bx * trackingData->q4 + _2bz * trackingData->q2) * (_2bx * (0.5f - q3q3 - q4q4) + _2bz * (q2q4 - q1q3) - magDataNorm[0]) + (-_2bx * trackingData->q1 + _2bz * trackingData->q3) * (_2bx * (q2q3 - q1q4) + _2bz * (q1q2 + q3q4) - magDataNorm[1]) + _2bx * trackingData->q2 * (_2bx * (q1q3 + q2q4) + _2bz * (0.5f - q2q2 - q3q3) - magDataNorm[2]);
    recipNorm = invSqrt(s1 * s1 + s2 * s2 + s3 * s3 + s4 * s4);
    
    // normalise step magnitude
    s1 *= recipNorm;
    s2 *= recipNorm;
    s3 *= recipNorm;
    s4 *= recipNorm;
    
    // Rate of change of quaternion from gyroscope
    qDot1 = 0.5f * (-trackingData->q2 * trackingData->gyroCalData[0] - trackingData->q3 * trackingData->gyroCalData[1] - trackingData->q4 * trackingData->gyroCalData[2]);
    qDot2 = 0.5f * (trackingData->q1 * trackingData->gyroCalData[0] + trackingData->q3 * trackingData->gyroCalData[2] - trackingData->q4 * trackingData->gyroCalData[1]);
    qDot3 = 0.5f * (trackingData->q1 * trackingData->gyroCalData[1] - trackingData->q2 * trackingData->gyroCalData[2] + trackingData->q4 * trackingData->gyroCalData[0]);
    qDot4 = 0.5f * (trackingData->q1 * trackingData->gyroCalData[2] + trackingData->q2 * trackingData->gyroCalData[1] - trackingData->q3 * trackingData->gyroCalData[0]);
    
    // Apply feedback step
    // compute the dynamic parameter beta: no movement => beta maximum, lot of movement => beta tends to 0
    trackingData->beta = trackingData->MadgwickBetaMax * (1 - min(max(trackingData->MadgwickBetaGain * gyro_norm2,0),1));
    qDot1 -= trackingData->beta * s1;
    qDot2 -= trackingData->beta * s2;
    qDot3 -= trackingData->beta * s3;
    qDot4 -= trackingData->beta * s4;
    
    
    // Integrate rate of change of quaternion to yield quaternion
    trackingData->q1 += qDot1 * trackingData->samplePeriod;
    trackingData->q2 += qDot2 * trackingData->samplePeriod;
    trackingData->q3 += qDot3 * trackingData->samplePeriod;
    trackingData->q4 += qDot4 * trackingData->samplePeriod;
    printf("estimated quaternion 1 : %f %f %f %f\r\n", trackingData->q1, trackingData->q2, trackingData->q3, trackingData->q4);
    
    // Normalise quaternion
    recipNorm = invSqrt(trackingData->q1 * trackingData->q1 + trackingData->q2 * trackingData->q2 + trackingData->q3 * trackingData->q3 + trackingData->q4 * trackingData->q4);
    printf("%f, recipNorm = %f\r\n", trackingData->q1 * trackingData->q1 + trackingData->q2 * trackingData->q2 + trackingData->q3 * trackingData->q3 + trackingData->q4 * trackingData->q4, recipNorm);
    trackingData->q1 *= recipNorm;
    trackingData->q2 *= recipNorm;
    trackingData->q3 *= recipNorm;
    trackingData->q4 *= recipNorm;
    
    printf("estimated quaternion 2 : %f %f %f %f\r\n", trackingData->q1, trackingData->q2, trackingData->q3, trackingData->q4);
    
    return 0;
}




//=====================================================================================================
// function processKeyValueSettingPair
//=====================================================================================================
//
// reads and check a key/value pair (from 2 strings) for one given parameter of the head tracker
// if UpdateHeadtrackerFlag = 0, only updates the corresponding internal variable of the receiver (case where the head tracker sends its settings to the receiver)
// if UpdateHeadtrackerFlag = 1, updates the corresponding internal variable of the receiver AND send the key/value pair to the head tracker (case where the settings are read from a text file)
//
// returns 1 if successful, 0 if error
//
int  processKeyValueSettingPair(headtrackerData *trackingData, char *keyBuffer, char *valueBuffer, char UpdateHeadtrackerFlag) {
    int i;
    
    if(strcmp(keyBuffer,"firmware_version") == 0) {
        trackingData->firmwareVersion=(char) strtol(valueBuffer,NULL,10);
        if(trackingData->verbose) printf("firmware version: %d\r\n",trackingData->firmwareVersion);
        if(trackingData->firmwareVersion!=HEDROT_FIRMWARE_VERSION) {
            printf("wrong firmware version\r\n");
            pushNotificationMessage(trackingData, NOTIFICATION_MESSAGE_WRONG_FIRMWARE_VERSION);
        } else {
            printf("firmware version OK\r\n");
        }
        
    } else if(strcmp(keyBuffer,"samplerate") == 0) {
        trackingData->samplerate=strtol(valueBuffer,NULL,10);
        trackingData->samplePeriod = 1.0f / trackingData->samplerate;
        if(trackingData->verbose) printf("samplerate: %ld\r\n",trackingData->samplerate);
        if(UpdateHeadtrackerFlag) setSamplerate(trackingData, trackingData->samplerate, 0);
        
    } else if(strcmp(keyBuffer,"gyroHalfScaleSensitivity") == 0) {
        trackingData->gyroHalfScaleSensitivity=strtol(valueBuffer,NULL,10);
        if(trackingData->verbose) printf("gyroHalfScaleSensitivity: %ld\r\n",trackingData->gyroHalfScaleSensitivity);
        
    } else if(strcmp(keyBuffer,"gyroBitDepth") == 0) {
        trackingData->gyroBitDepth=strtol(valueBuffer,NULL,10);
        if(trackingData->verbose) printf("gyroBitDepth: %ld\r\n",trackingData->gyroBitDepth);
        
    } else if(strcmp(keyBuffer,"gyroDataRate") == 0) {
        trackingData->gyroDataRate=(char) strtol(valueBuffer,NULL,10);
        if(trackingData->verbose) printf("gyroDataRate: %d\r\n",trackingData->gyroDataRate);
        if(UpdateHeadtrackerFlag) setgyroDataRate(trackingData, trackingData->gyroDataRate, 0);
        
    } else if(strcmp(keyBuffer,"gyroClockSource") == 0) {
        trackingData->gyroClockSource=(char) strtol(valueBuffer,NULL,10);
        if(trackingData->verbose) printf("gyroClockSource: %d\r\n",trackingData->gyroClockSource);
        if(UpdateHeadtrackerFlag) setGyroClockSource(trackingData, trackingData->gyroClockSource, 0);
        
    } else if(strcmp(keyBuffer,"gyroDLPFBandwidth") == 0) {
        trackingData->gyroDLPFBandwidth=(char) strtol(valueBuffer,NULL,10);
        if(trackingData->verbose) printf("gyroDLPFBandwidth: %d\r\n",trackingData->gyroDLPFBandwidth);
        if(UpdateHeadtrackerFlag) setGyroDLPFBandwidth(trackingData, trackingData->gyroDLPFBandwidth, 0);
        
    } else if(strcmp(keyBuffer,"accHardOffset") == 0) {
        if(stringToChars(valueBuffer, trackingData->accHardOffset, 3)) {
            if(trackingData->verbose) printf("accHardOffset: %d %d %d\r\n",trackingData->accHardOffset[0],trackingData->accHardOffset[1],trackingData->accHardOffset[2]);
            if(UpdateHeadtrackerFlag) setAccHardOffset(trackingData, trackingData->accHardOffset, 0);
        } else {
            if(trackingData->verbose) printf("error while reading accHardOffset !!!!\r\n");
            return 0;
        }
        
    } else if(strcmp(keyBuffer,"accFullResolutionBit") == 0) {
        trackingData->accFullResolutionBit=(char) strtol(valueBuffer,NULL,10);
        if(trackingData->verbose) printf("accFullResolutionBit: %d\r\n",trackingData->accFullResolutionBit);
        if(UpdateHeadtrackerFlag) setAccFullResolutionBit(trackingData, trackingData->accFullResolutionBit, 0);
        
    } else if(strcmp(keyBuffer,"accDataRate") == 0) {
        trackingData->accDataRate=(char) strtol(valueBuffer,NULL,10);
        if(trackingData->verbose) printf("accelerometer_data_rate: %d\r\n",trackingData->accDataRate);
        if(UpdateHeadtrackerFlag) setAccDataRate(trackingData, trackingData->accDataRate, 0);
        
    } else if(strcmp(keyBuffer,"accRange") == 0) {
        trackingData->accRange=(char) strtol(valueBuffer,NULL,10);
        if(trackingData->verbose) printf("accRange: %d\r\n",trackingData->accRange);
        if(UpdateHeadtrackerFlag) setAccRange(trackingData, trackingData->accRange, 0);
        
    } else if(strcmp(keyBuffer,"magMeasurementBias") == 0) {
        trackingData->magMeasurementBias=(char) strtol(valueBuffer,NULL,10);
        if(trackingData->verbose) printf("magMeasurementBias: %d\r\n",trackingData->magMeasurementBias);
        if(UpdateHeadtrackerFlag) setMagMeasurementBias(trackingData, trackingData->magMeasurementBias, 0);
        
    } else if(strcmp(keyBuffer,"magSampleAveraging") == 0) {
        trackingData->magSampleAveraging=(char) strtol(valueBuffer,NULL,10);
        if(trackingData->verbose) printf("magSampleAveraging: %d\r\n",trackingData->magSampleAveraging);
        if(UpdateHeadtrackerFlag) setMagSampleAveraging(trackingData, trackingData->magSampleAveraging, 0);
        
    } else if(strcmp(keyBuffer,"magDataRate") == 0) {
        trackingData->magDataRate=(char) strtol(valueBuffer,NULL,10);
        if(trackingData->verbose) printf("magDataRate: %d\r\n",trackingData->magDataRate);
        if(UpdateHeadtrackerFlag) setMagDataRate(trackingData, trackingData->magDataRate, 0);
        
    } else if(strcmp(keyBuffer,"magGain") == 0) {
        trackingData->magGain=(char) strtol(valueBuffer,NULL,10);
        if(trackingData->verbose) printf("magGain: %d\r\n",trackingData->magGain);
        if(UpdateHeadtrackerFlag) setMagGain(trackingData, trackingData->magGain, 0);
        
    } else if(strcmp(keyBuffer,"magMeasurementMode") == 0) {
        trackingData->magMeasurementMode=(char) strtol(valueBuffer,NULL,10);
        if(trackingData->verbose) printf("magMeasurementMode: %d\r\n",trackingData->magMeasurementMode);
        if(UpdateHeadtrackerFlag) setMagMeasurementMode(trackingData, trackingData->magMeasurementMode, 0);
        
    } else if(strcmp(keyBuffer,"accOffset") == 0) {
        if(stringToFloats(valueBuffer, trackingData->accOffset, 3)) {
            if(trackingData->verbose) printf("accOffset: %f %f %f\r\n",trackingData->accOffset[0], trackingData->accOffset[1], trackingData->accOffset[2]);
            if(UpdateHeadtrackerFlag) setAccOffset(trackingData, trackingData->accOffset, 0);
        } else {
            if(trackingData->verbose) printf("error while reading accOffset !!!!\r\n");
            return 0;
        }
        
    } else if(strcmp(keyBuffer,"accScaling") == 0) {
        if(stringToFloats(valueBuffer, trackingData->accScaling, 3)) {
            
            for(i=0;i<3;i++) {
                trackingData->accScalingFactor[i] = 1/trackingData->accScaling[i];
            }
            
            if(trackingData->verbose) {
                printf("accScaling: %f %f %f \r\n",trackingData->accScaling[0],trackingData->accScaling[1],trackingData->accScaling[2]);
                printf("accScalingFactor: %f %f %f \r\n",trackingData->accScalingFactor[0],trackingData->accScalingFactor[1],trackingData->accScalingFactor[2]);
            }
            
            if(UpdateHeadtrackerFlag) setAccScaling(trackingData, trackingData->accScaling, 0);
            if(!((trackingData->accScaling[0]>0) && (trackingData->accScaling[1]>0) && (trackingData->accScaling[2]>0))) { // calibration not valid
                trackingData->calibrationValid = 0;
                pushNotificationMessage(trackingData, NOTIFICATION_MESSAGE_CALIBRATION_NOT_VALID);
            }
        } else {
            if(trackingData->verbose) printf("error while reading accScaling !!!!\r\n");
            return 0;
        }
        
    } else if(strcmp(keyBuffer,"magOffset") == 0) {
        if(stringToFloats(valueBuffer, trackingData->magOffset, 3)) {
            if(trackingData->verbose) printf("magOffset: %f %f %f\r\n",trackingData->magOffset[0], trackingData->magOffset[1], trackingData->magOffset[2]);
            if(UpdateHeadtrackerFlag) setMagOffset(trackingData, trackingData->magOffset, 0);
        } else {
            if(trackingData->verbose) printf("error while reading magOffset !!!!\r\n");
            return 0;
        }
        
    } else if(strcmp(keyBuffer,"magScaling") == 0) {
        if(stringToFloats(valueBuffer, trackingData->magScaling, 3)) {
            
            for(i=0;i<3;i++) {
                trackingData->magScalingFactor[i] = 1/trackingData->magScaling[i];
            }
            
            if(trackingData->verbose) {
                printf("magScaling: %f %f %f \r\n",trackingData->magScaling[0],trackingData->magScaling[1],trackingData->magScaling[2]);
                printf("magScalingFactor: %f %f %f \r\n",trackingData->magScalingFactor[0],trackingData->magScalingFactor[1],trackingData->magScalingFactor[2]);
            }
            
            if(UpdateHeadtrackerFlag) setMagScaling(trackingData, trackingData->magScaling, 0);
            if(!((trackingData->magScaling[0]>0) || (trackingData->magScaling[1]>0) || (trackingData->magScaling[2]>0))) { // calibration not valid
                trackingData->calibrationValid = 0;
                pushNotificationMessage(trackingData, NOTIFICATION_MESSAGE_CALIBRATION_NOT_VALID);
            }
        } else {
            if(trackingData->verbose) printf("error while reading magScaling !!!!\r\n");
            return 0;
        }
    } else {
        if(trackingData->verbose) printf("unknown key <<%s>> !!!! Continuing anyway...\r\n", keyBuffer);
    }
    
    return 1;
}



//=====================================================================================================
// function headtracker_open
//=====================================================================================================
//
// open a port by its number
//
void headtracker_open(headtrackerData *trackingData, int portnum)
{
#if defined(_WIN32) || defined(_WIN64)
    HANDLE handle;
#else /* #if defined(_WIN32) || defined(_WIN64)*/
    int handle;
#endif
    
    if(trackingData->serialcomm->comhandle != INVALID_HANDLE_VALUE) {
        if(trackingData->verbose) printf("[hedrot] : closing previously openened port port %s\r\n", trackingData->serialcomm->serial_device_name);
        headtracker_close(trackingData);
    }
    
    if(trackingData->verbose) printf("[hedrot] trying to open port %i\r\n", portnum);
    handle = open_serial(trackingData->serialcomm,trackingData->serialcomm->availablePorts[portnum]);
    
    if(handle != INVALID_HANDLE_VALUE) {
        // the port is open: save the port number
        trackingData->serialcomm->portNumber = portnum;
        
        pushNotificationMessage(trackingData, NOTIFICATION_MESSAGE_PORT_OPENED);
        
        if(trackingData->verbose) printf("[hedrot] port %d with handle %d is valid\r\n", portnum, handle);
        
        // request info
        headtracker_requestHeadtrackerSettings(trackingData);
    } else {
        if(trackingData->verbose) printf("[hedrot] port %d with handle %d is NOT valid\r\n", portnum, handle);
    }
}


//=====================================================================================================
// function headtracker_close
//=====================================================================================================
//
// close the currently opened port
//
void headtracker_close(headtrackerData *trackingData)
{
	unsigned char message; // for single byte to be sent to the head tracker

    if(trackingData->verbose) printf("[hedrot] closing port...\r\n");
    
    if(trackingData->serialcomm->comhandle != INVALID_HANDLE_VALUE) {
		message = R2H_STOP_TRANSMISSION_CHAR;
        write_serial(trackingData->serialcomm,&message, 1); //stops sending raw data
	}

    close_serial(trackingData->serialcomm);
    
    headtracker_setReceptionStatus(trackingData,COMMUNICATION_STATE_NO_CONNECTED_HEADTRACKER);
}


//=====================================================================================================
// function pullNotificationMessage
//=====================================================================================================
//
// get first message to notify in the FIFO queue
// if no messages, returns NOTIFICATION_MESSAGE_NONE
int pullNotificationMessage(headtrackerData *trackingData) {
    if(!trackingData->numberOfMessages) {
        return NOTIFICATION_MESSAGE_NONE;
    } else {
        trackingData->numberOfMessages--;
        trackingData->firstMessageToNotify++;
        if( trackingData->firstMessageToNotify == MAX_NUMBER_OF_NOTIFICATION_MESSAGES)
            trackingData->firstMessageToNotify = 0;
        return trackingData->messagesToNotify[trackingData->firstMessageToNotify-1]; // -1 necessary since we updated the value right before
    }
}

//=====================================================================================================
// function pushNotificationMessage
//=====================================================================================================
//
// put a new message to notify in the FIFO queue
void pushNotificationMessage(headtrackerData *trackingData, char messageNumber){
    // circular buffer: there is no check if the queue is full, old messages are overwritten if necessary
    
    // push the message at the end of the circular buffer
    trackingData->messagesToNotify[(trackingData->firstMessageToNotify + trackingData->numberOfMessages) % MAX_NUMBER_OF_NOTIFICATION_MESSAGES] = messageNumber;
    trackingData->numberOfMessages++;
}




//=====================================================================================================
// function headtracker_autodiscover
//=====================================================================================================
//
// headtracker autodiscovering function
//
void headtracker_autodiscover(headtrackerData *trackingData) {
	unsigned int i;
    
    if(trackingData->verbose) printf("[hedrot] : autodiscovering\r\n");
    
    if(trackingData->serialcomm->comhandle == INVALID_HANDLE_VALUE) { // headtracker still not found
        headtracker_setReceptionStatus(trackingData, COMMUNICATION_STATE_AUTODISCOVERING_STARTED);
        headtracker_autodiscover_tryNextPort(trackingData);
    } else {
        // a port has been opened and a tick has been sent
        // did it respond?
        init_read_serial(trackingData->serialcomm);
        
        if(is_data_available(trackingData->serialcomm)) {
            // read one byte and check that there is no read error
            if(trackingData->serialcomm->numberOfReadBytes == 0) { // if a read error was detected
                if(trackingData->verbose) printf("autodiscover : read error\r\n");
                headtracker_setReceptionStatus(trackingData, COMMUNICATION_STATE_AUTODISCOVERING_NO_HEADTRACKER_THERE);
                // close the port
                close_serial(trackingData->serialcomm);
            } else {
                for( i = 0; i < trackingData->serialcomm->numberOfReadBytes; i++) { //loop on all received bytes
                    if(trackingData->verbose == 3) printf("autodiscover : byte received = %d\r\n",trackingData->serialcomm->readBuffer[i]);
                    if(trackingData->serialcomm->readBuffer[i]==H2R_IAMTHERE_CHAR) {
                        //if the headtracker responded, close and reopen it with the proper headtracker_open function
                        if(trackingData->verbose) printf("Headtracker Found on port %d, open the port\r\n",trackingData->serialcomm->portNumber);
                        
                        close_serial(trackingData->serialcomm);
                        
                        headtracker_setReceptionStatus(trackingData, COMMUNICATION_STATE_AUTODISCOVERING_HEADTRACKER_FOUND);
                        
                        headtracker_open(trackingData,trackingData->serialcomm->portNumber);
                    }
                }
            }
        } else { // no data available
            // if time elapsed, close the current device and go to the next one
            printf("autodiscover : time elapsed since query: %f sec\r\n", get_monotonic_time() - trackingData->autodiscoverResponseTimeLimit + AUTODISCOVER_MAX_TIME);
            if(get_monotonic_time() > trackingData->autodiscoverResponseTimeLimit) {
                if(trackingData->verbose) printf("autodiscover : comm device did not respond on time\r\n");
                close_serial(trackingData->serialcomm);
                headtracker_setReceptionStatus(trackingData, COMMUNICATION_STATE_AUTODISCOVERING_NO_HEADTRACKER_THERE);
            } // otherwise do nothing
        }
    }
}


//=====================================================================================================
// function headtracker_autodiscover_tryNextPort
//=====================================================================================================
//
// if during autodiscover, the previous port does not correspond to the headtracker, try the next one
//
void headtracker_autodiscover_tryNextPort(headtrackerData *trackingData) {
	
	unsigned char message; // for single byte to be sent to the head tracker

    trackingData->serialcomm->portNumber++;
    if(trackingData->serialcomm->portNumber<trackingData->serialcomm->numberOfAvailablePorts) {
        // did we reach the last port? if not, try to open it
        
        // try to open the device
        if(trackingData->verbose) printf("autodiscover: trying to open port %s \r\n",
                                         trackingData->serialcomm->availablePorts[trackingData->serialcomm->portNumber]);
        if((open_serial(trackingData->serialcomm,trackingData->serialcomm->availablePorts[trackingData->serialcomm->portNumber])) != INVALID_HANDLE_VALUE) {
            if(trackingData->verbose) printf("autodiscover: port %s, comhandle %d, opened\r\n",
                                             trackingData->serialcomm->availablePorts[trackingData->serialcomm->portNumber],
                                             trackingData->serialcomm->comhandle);
            
            
            // test if it responds to a ping
            // send the ping data and wait for the next tick to see if the headtracker responds
			message = R2H_AREYOUTHERE_CHAR;
            if(write_serial(trackingData->serialcomm, &message, 1)!=1) {
                headtracker_setReceptionStatus(trackingData, COMMUNICATION_STATE_AUTODISCOVERING_STARTED);
                
                // cannot write a serial, close the port
                close_serial(trackingData->serialcomm);
            } else {
                trackingData->autodiscoverResponseTimeLimit = get_monotonic_time() + AUTODISCOVER_MAX_TIME;
                headtracker_setReceptionStatus(trackingData, COMMUNICATION_STATE_AUTODISCOVERING_WAITING_FOR_RESPONSE);
            }
        } else {
            // open unsuccessful
            if(trackingData->verbose) printf("autodiscover: port %d cannot be opened\r\n",trackingData->serialcomm->portNumber);
            
            trackingData->serialcomm->comhandle = INVALID_HANDLE_VALUE;
        }
    } else {
        // we reach the last port, restart counting and reset list of comm ports
        headtracker_list_comm_ports(trackingData);
    }
}

//=====================================================================================================
// function center_angles
//=====================================================================================================
//
// center headtracker according to current position using inverse quaternion of the current quaternion
//
void center_angles(headtrackerData *trackingData) {
    trackingData->qref1 = trackingData->q1;
    trackingData->qref2 = - trackingData->q2;
    trackingData->qref3 = - trackingData->q3;
    trackingData->qref4 = - trackingData->q4;
}

//=====================================================================================================
// "public" setters for receiver parameters
//=====================================================================================================

void setHeadtrackerOn(headtrackerData *trackingData, char headtrackeronVal) {
    trackingData->headtracker_on = headtrackeronVal;
    
    headtracker_setReceptionStatus(trackingData, COMMUNICATION_STATE_NO_CONNECTED_HEADTRACKER);
    
    if(trackingData->headtracker_on) {
        printf("[hedrot] : switching on...\r\n");
    } else {
        printf("[hedrot] : switching off...\r\n");
        // switch off => close the port
        headtracker_close(trackingData);
    }
}

void setVerbose(headtrackerData *trackingData, char verboseVal) {
    trackingData->verbose = verboseVal;
    trackingData->serialcomm->verbose = verboseVal;
}

void setAutoDiscover(headtrackerData *trackingData, char autoDiscover) {
    trackingData->autoDiscover = autoDiscover;
}


void setGyroOffsetAutocalOn(headtrackerData *trackingData, char gyroOffsetAutocalOn) {
    trackingData->gyroOffsetAutocalOn = gyroOffsetAutocalOn;
    
    // if the flag gyroOffsetAutocalOn is set to 1, restart the gyro calibration
    if(trackingData->gyroOffsetAutocalOn == 1) {
        trackingData->gyroOffsetCalibratedState = 0;
        resetGyroOffsetCalibration(trackingData);
    }
}


void setGyroOffsetAutocalTime(headtrackerData *trackingData, float gyroOffsetAutocalTime) {
    trackingData->gyroOffsetAutocalTime = gyroOffsetAutocalTime;
}


void setGyroOffsetAutocalThreshold(headtrackerData *trackingData, long gyroOffsetAutocalThreshold) {
    trackingData->gyroOffsetAutocalThreshold = gyroOffsetAutocalThreshold;
}


void setMadgwickBetaGain(headtrackerData *trackingData, float MadgwickBetaGain) {
    trackingData->MadgwickBetaGain = MadgwickBetaGain;
}


void setMadgwickBetaMax(headtrackerData *trackingData, float MadgwickBetaMax) {
    trackingData->MadgwickBetaMax = MadgwickBetaMax;
}


void setAccLPtimeConstant(headtrackerData *trackingData, float accLPtimeConstant) {
    trackingData->accLPtimeConstant = accLPtimeConstant;
    trackingData->accLPalpha = 1 - (float) exp(-trackingData->samplePeriod/trackingData->accLPtimeConstant);
}


//=====================================================================================================
// public setter to change axes reference
//=====================================================================================================
void setAxesReference(headtrackerData *trackingData, char axesReference) {
    trackingData->axesReference = axesReference;
}


//=====================================================================================================
// public setter to change rotation sequence
//=====================================================================================================
void setRotationOrder(headtrackerData *trackingData, char rotationOrder) {
    trackingData->rotationOrder = rotationOrder;
}


//=====================================================================================================
// public setter to change invert angles flag
//=====================================================================================================
void setInvertRotation(headtrackerData *trackingData, char invertRotation){
    trackingData->invertRotation = invertRotation;
}


//=====================================================================================================
// public setters to send attributes to the headtracker
//=====================================================================================================
void setSamplerate(headtrackerData *trackingData, long samplerate, char requestSettingsFlag) {
	unsigned char message[3];

    trackingData->samplerate = max(min(samplerate,65535),2);
    trackingData->samplePeriod = 1.0f / trackingData->samplerate;
    
    // recalculate receiver parameters based on samplerate
    trackingData->accLPalpha = 1 - (float) exp(-trackingData->samplePeriod/trackingData->accLPtimeConstant);
    
	message[0] = R2H_TRANSMIT_SAMPLERATE;
	message[1] = (unsigned char) (trackingData->samplerate%256); //least significant byte first, then most significant byte
	message[2] = (unsigned char) (trackingData->samplerate/256);

    write_serial(trackingData->serialcomm, message, 3);
    
    // request settings
    if(requestSettingsFlag) headtracker_requestHeadtrackerSettings(trackingData);
}

void setgyroDataRate(headtrackerData *trackingData, unsigned char gyroDataRate, char requestSettingsFlag) {
    unsigned char message[2];

	trackingData->gyroDataRate = gyroDataRate;
	message[0] = R2H_TRANSMIT_GYRO_RATE;
	message[1] = trackingData->gyroDataRate;
	write_serial(trackingData->serialcomm, message, 2);
    
    // request settings
    if(requestSettingsFlag) headtracker_requestHeadtrackerSettings(trackingData);
}

void setGyroClockSource(headtrackerData *trackingData, unsigned char gyroClockSource, char requestSettingsFlag) {
    unsigned char message[2];

	trackingData->gyroClockSource = gyroClockSource;
	message[0] = R2H_TRANSMIT_GYRO_CLOCK_SOURCE;
	message[1] = trackingData->gyroClockSource;
	write_serial(trackingData->serialcomm, message, 2);
    
    // request settings
    if(requestSettingsFlag) headtracker_requestHeadtrackerSettings(trackingData);
}

void setGyroDLPFBandwidth(headtrackerData *trackingData, unsigned char gyroDLPFBandwidth, char requestSettingsFlag) {
    unsigned char message[2];

	trackingData->gyroDLPFBandwidth = gyroDLPFBandwidth;
	message[0] = R2H_TRANSMIT_GYRO_LPF_BANDWIDTH;
	message[1] = trackingData->gyroDLPFBandwidth;
	write_serial(trackingData->serialcomm, message, 2);
    
    // request settings
    if(requestSettingsFlag) headtracker_requestHeadtrackerSettings(trackingData);
}

void setAccRange(headtrackerData *trackingData, unsigned char accRange, char requestSettingsFlag) {
    unsigned char message[2];

	trackingData->accRange = accRange;
	message[0] = R2H_TRANSMIT_ACCEL_RANGE;
	message[1] = trackingData->accRange;
	write_serial(trackingData->serialcomm, message, 2);
    
    // request settings
    if(requestSettingsFlag) headtracker_requestHeadtrackerSettings(trackingData);
}

void setAccHardOffset(headtrackerData *trackingData, char *accHardOffset, char requestSettingsFlag) {
    int i;
	for(i=0;i<3;i++) {
        trackingData->accHardOffset[i] = accHardOffset[i];
    }
    
    headtracker_sendSignedCharArray2Headtracker(trackingData,trackingData->accHardOffset,3,R2H_START_TRANSMIT_ACCEL_HARD_OFFSET,R2H_STOP_TRANSMIT_ACCEL_HARD_OFFSET);
    
    // request settings
    if(requestSettingsFlag) headtracker_requestHeadtrackerSettings(trackingData);
}

void setAccFullResolutionBit(headtrackerData *trackingData, unsigned char accFullResolutionBit, char requestSettingsFlag) {
    unsigned char message[2];
	
	trackingData->accFullResolutionBit = accFullResolutionBit;
	message[0] = R2H_TRANSMIT_ACCEL_FULL_RESOLUTION_BIT;
	message[1] = trackingData->accFullResolutionBit;
	write_serial(trackingData->serialcomm, message, 2);
    
    // request settings
    if(requestSettingsFlag) headtracker_requestHeadtrackerSettings(trackingData);
}

void setAccDataRate(headtrackerData *trackingData, unsigned char accDataRate, char requestSettingsFlag) {
    unsigned char message[2];
	
	trackingData->accDataRate = accDataRate;
	message[0] = R2H_TRANSMIT_ACCEL_DATARATE;
	message[1] = trackingData->accDataRate;
	write_serial(trackingData->serialcomm, message, 2);
    
    // request settings
    if(requestSettingsFlag) headtracker_requestHeadtrackerSettings(trackingData);
}

void setMagMeasurementBias(headtrackerData *trackingData, unsigned char magMeasurementBias, char requestSettingsFlag) {
    unsigned char message[2];
	
	trackingData->magMeasurementBias = magMeasurementBias;
	message[0] = R2H_TRANSMIT_MAG_MEASUREMENT_BIAS;
	message[1] = trackingData->magMeasurementBias;
	write_serial(trackingData->serialcomm, message, 2);
    
    // request settings
    if(requestSettingsFlag) headtracker_requestHeadtrackerSettings(trackingData);
}

void setMagSampleAveraging(headtrackerData *trackingData, unsigned char magSampleAveraging, char requestSettingsFlag) {
    unsigned char message[2];
	
	trackingData->magSampleAveraging = magSampleAveraging;
	message[0] = R2H_TRANSMIT_MAG_SAMPLE_AVERAGING;
	message[1] = trackingData->magSampleAveraging;
	write_serial(trackingData->serialcomm, message, 2);
    
    // request settings
    if(requestSettingsFlag) headtracker_requestHeadtrackerSettings(trackingData);
}

void setMagDataRate(headtrackerData *trackingData, unsigned char magDataRate, char requestSettingsFlag) {
    unsigned char message[2];
	
	trackingData->magDataRate = magDataRate;
	message[0] = R2H_TRANSMIT_MAG_DATA_RATE;
	message[1] = trackingData->magDataRate;
	write_serial(trackingData->serialcomm, message, 2);
    
    // request settings
    if(requestSettingsFlag) headtracker_requestHeadtrackerSettings(trackingData);
}

void setMagGain(headtrackerData *trackingData, unsigned char magGain, char requestSettingsFlag) {
    unsigned char message[2];
	
	trackingData->magGain = magGain;
	message[0] = R2H_TRANSMIT_MAG_GAIN;
	message[1] = trackingData->magGain;
	write_serial(trackingData->serialcomm, message, 2);
    
    // request settings
    if(requestSettingsFlag) headtracker_requestHeadtrackerSettings(trackingData);
}

void setMagMeasurementMode(headtrackerData *trackingData, unsigned char magMeasurementMode, char requestSettingsFlag) {
    unsigned char message[2];
	
	trackingData->magMeasurementMode = magMeasurementMode;
	message[0] = R2H_TRANSMIT_MAG_MEASUREMENT_MODE;
	message[1] = trackingData->magMeasurementMode;
	write_serial(trackingData->serialcomm, message, 2);
    
    // request settings
    if(requestSettingsFlag) headtracker_requestHeadtrackerSettings(trackingData);
}


//=====================================================================================================
// public setters to send calibration attributes to the headtracker
//=====================================================================================================

void setAccOffset(headtrackerData *trackingData, float* accOffset, char requestSettingsFlag) {
    int i;

    for(i=0;i<3;i++)
        trackingData->accOffset[i] = accOffset[i];
    
    headtracker_sendFloatArray2Headtracker(trackingData,trackingData->accOffset,3,R2H_START_TRANSMIT_ACCEL_OFFSET_DATA_CHAR,R2H_STOP_TRANSMIT_ACCEL_OFFSET_DATA_CHAR);
    
    // schedule an information request
    if(requestSettingsFlag) headtracker_requestHeadtrackerSettings(trackingData);
}

void setAccScaling(headtrackerData *trackingData, float* accScaling, char requestSettingsFlag) {
    int i;

    for(i=0;i<3;i++) {
        trackingData->accScaling[i] = accScaling[i];
        trackingData->accScalingFactor[i] = 1/accScaling[i];
    }
    
    headtracker_sendFloatArray2Headtracker(trackingData,trackingData->accScaling,3,R2H_START_TRANSMIT_ACCEL_SCALING_DATA_CHAR,R2H_STOP_TRANSMIT_ACCEL_SCALING_DATA_CHAR);
    
    // schedule an information request
    if(requestSettingsFlag) headtracker_requestHeadtrackerSettings(trackingData);
}

void setMagOffset(headtrackerData *trackingData, float* magOffset, char requestSettingsFlag) {
    int i;

    for(i=0;i<3;i++)
        trackingData->magOffset[i] = magOffset[i];
    
    headtracker_sendFloatArray2Headtracker(trackingData,trackingData->magOffset,3,R2H_START_TRANSMIT_MAG_OFFSET_DATA_CHAR,R2H_STOP_TRANSMIT_MAG_OFFSET_DATA_CHAR);
    
    // schedule an information request
    if(requestSettingsFlag) headtracker_requestHeadtrackerSettings(trackingData);
}

void setMagScaling(headtrackerData *trackingData, float* magScaling, char requestSettingsFlag) {
    int i;

    for(i=0;i<3;i++) {
        trackingData->magScaling[i] = magScaling[i];
        trackingData->magScalingFactor[i] = 1/magScaling[i];
    }
    
    headtracker_sendFloatArray2Headtracker(trackingData,trackingData->magScaling,3,R2H_START_TRANSMIT_MAG_SCALING_DATA_CHAR,R2H_STOP_TRANSMIT_MAG_SCALING_DATA_CHAR);
    
    // schedule an information request
    if(requestSettingsFlag) headtracker_requestHeadtrackerSettings(trackingData);
}


//=====================================================================================================
// "private" setters
//=====================================================================================================
void headtracker_setReceptionStatus(headtrackerData *trackingData, int n) {
    trackingData->infoReceptionStatus = n;
    if(trackingData->verbose) printf("new reception status: %d \r\n",trackingData->infoReceptionStatus);
    
    // notify the host
    pushNotificationMessage(trackingData, NOTIFICATION_MESSAGE_HEADTRACKER_STATUS_CHANGED);
    
    // if transmission starts, reset the necessary internal variables
    if(n == COMMUNICATION_STATE_RECEIVING_INFO) {
        // init of the accelerometer LP filter state
        trackingData->accLPstate[0] = 0;
        trackingData->accLPstate[1] = 0;
        trackingData->accLPstate[2] = 0;
        
        
        //initialization of the quaternions
        trackingData->q1 = 1.0;
        trackingData->q2 = 0.0;
        trackingData->q3 = 0.0;
        trackingData->q4 = 0.0;
        
        trackingData->rawDataBufferIndex = 0;
        
        // reset gyro auto calibration variables
        trackingData->gyroOffsetCalibratedState = 0;
        resetGyroOffsetCalibration(trackingData);
    }
    
    
}

// send a float array to the headtracker
void headtracker_sendFloatArray2Headtracker(headtrackerData *trackingData, float* data, int numValues, unsigned char StartTransmitChar, unsigned char StopTransmitChar) {
    char charData[20];

	unsigned char message[1000]; // the char array won't probably be longer as 1000
	int messageLen;
    
    int i;
	unsigned int n;
    
	messageLen = 0;

	message[messageLen++] = StartTransmitChar;
    for(i=0;i<numValues;i++)
	{
#if defined(_WIN32) || defined(_WIN64)
        sprintf_s(charData, 20, "%.2f", data[i]);
#else /* #if defined(_WIN32) || defined(_WIN64) */
        sprintf(charData,"%.2f",data[i]);
#endif /* #if defined(_WIN32) || defined(_WIN64) */

        for(n=0;n<strlen(charData);n++)
            message[messageLen++] = charData[n];
        
        if(i<numValues-1)
            message[messageLen++] = ' '; // pas d'espace après le dernier
	}
	message[messageLen++] = StopTransmitChar;

	write_serial(trackingData->serialcomm, message, messageLen);
}


// send a signed char array to the headtracker
void headtracker_sendSignedCharArray2Headtracker(headtrackerData *trackingData, char *data, int numValues, unsigned char StartTransmitChar, unsigned char StopTransmitChar) {
    int i;
	unsigned char message[1000];	
	
	message[0] = StartTransmitChar;
    for(i=0;i<numValues;i++)
        message[i+1] = data[i];
	message[numValues+1] = StopTransmitChar;

	write_serial(trackingData->serialcomm, message, numValues+2);

}


//=====================================================================================================
// utils
//=====================================================================================================

// floating point modulo
double mod(double a, double N) {return a - N*floor(a/N);} //return in range [0, N]


//---------------------------------------------------------------------------------------------------
// Fast inverse square-root

// single precision version
// See: http://en.wikipedia.org/wiki/Fast_inverse_square_root
float invSqrt(float x) {
    float halfx = 0.5f * x;
    float y = x;
    int32_t i = *(int32_t*)&y;
    i = 0x5f3759df - (i>>1);
    y = *(float*)&i;
    y = y * (1.5f - (halfx * y * y));
    return y;
}


/* double precision version
 // See: https://tbach.web.cern.ch/tbach/thesis/literature/fastinvsquare_robertson.pdf
 double invSqrt(double x) {
 uint64_t i;
 double x2, y;
 x2 = x * 0.5;
 y = x;
 i = *(uint64_t *) &y;
 i = 0x5fe6eb50c7b537a9 - (i >> 1);
 y = *(double *) &i;
 y = y * (1.5 - (x2 * y * y));
 return y;
 }
 */

/*
 // not optimized (but more precise) version
 double invSqrt(double x) {
 if(x==0) return 1e40; // arbitrary very big number
 else return 1./sqrt(x);
 }*/

void quaternion2YawPitchRoll(float q1, float q2, float q3, float q4, float *yaw, float *pitch, float *roll) {
    // zyx Talt-Bryan rotation sequence
    
    *yaw = (float) (RAD_TO_DEGREE * atan2(2.0f * (q1*q4 + q2*q3),
                                          1.0f - 2.0f * (q3*q3 + q4*q4) ));
    
    *pitch = (float) (RAD_TO_DEGREE * asin(min(max(2.0f * (q1*q3 - q4*q2),-1),1)));
    
    
    *roll= (float) (RAD_TO_DEGREE * atan2(2.0f * (q1*q2 + q3*q4),
                                          1.0f - 2.0f * (q2*q2 + q3*q3)));
}

void quaternion2RollPitchYaw(float q1, float q2, float q3, float q4, float *yaw, float *pitch, float *roll) {
    // xyz Talt-Bryan rotation sequence
    
    *roll = (float) (RAD_TO_DEGREE * atan2(2.0f * (q1*q2 - q3*q4),
                                          1.0f - 2.0f * (q2*q2 + q3*q3) ));
    
    *pitch = (float) (RAD_TO_DEGREE * asin(min(max(2.0f * (q1*q3 + q4*q2),-1),1)));
    
    
    *yaw= (float) (RAD_TO_DEGREE * atan2(2.0f * (q1*q4 - q2*q3),
                                          1.0f - 2.0f * (q3*q3 + q4*q4)));
}

void quaternionComposition(float q01, float q02, float q03, float q04, float q11, float q12, float q13, float q14, float *q21, float *q22, float *q23, float *q24) {
    // compose two quaternions (Hamilton product)
    // the quaternion (q11, q12, q13, q14) is rotated according to the reference quaternion (q01, q02, q03, q04)
    *q21 = q01 * q11 - q02 * q12 - q03 * q13 - q04 * q14;
    *q22 = q01 * q12 + q02 * q11 + q03 * q14 - q04 * q13;
    *q23 = q01 * q13 - q02 * q14 + q03 * q11 + q04 * q12;
    *q24 = q01 * q14 + q02 * q13 - q03 * q12 + q04 * q11;
}

void changeQuaternionReference(headtrackerData *trackingData) {
    // change the axes references of the quaternion if it does not fit the standard (X->right, Y->back, Z->down)
    float qtemp;
    switch (trackingData->axesReference) {
        case 1: // X->right, Y->front, Z->up
            trackingData->qcent3 *= -1.0f; // Y -> -Y
            trackingData->qcent4 *= -1.0f; // Z -> -Z
            break;
        case 2: //X->front, Y->left, Z->up
            qtemp = trackingData->qcent2; // store X
            trackingData->qcent2 = -trackingData->qcent3; // Y -> -X
            trackingData->qcent3 = -qtemp; // X -> -Y
            trackingData->qcent4 *= -1.0f; // Z -> -Z
            break;
            // if 0 does nothing
    }

}


int stringToFloats(char* valueBuffer, float* data, int nvalues) {
    int i=0;
    
    char *delim = " "; // input separated by spaces
    char *token = NULL;
	char *brkt;
    
    for (token = strtok_r(valueBuffer, delim, &brkt); token != NULL; token = strtok_r(NULL, delim, &brkt))
    {
        char *unconverted;
        data[i] = (float) strtod(token, &unconverted);
        i++;
    }
    
    if(i<nvalues-1) {
        return 0; //error
    } else {
        return 1; //ok
    }
}


int stringToChars(char* valueBuffer, char* data, int nvalues) {
    int i=0;
    
    char *delim = " "; // input separated by spaces
    char *token = NULL;
	char *brkt;
    
    for (token = strtok_r(valueBuffer, delim, &brkt); token != NULL; token = strtok_r(NULL, delim, &brkt))
    {
        char *unconverted;
        data[i] = (char) strtol(token, &unconverted,10);
        i++;
    }
    
    if(i<nvalues-1) {
        return 0; //error
    } else {
        return 1; //ok
    }
}

