//
// libhedrot.h
// all functions for communicating with the headtracker, receiving the raw data and cook the angles
// header file
//
// Part of code is derived from Sebastian Madgwick's open-source gradient descent angle estimation algorithm
//
// Copyright 2016 Alexis Baskind


#ifndef libheadtracker_h
#define libheadtracker_h

//=====================================================================================================
// header includes
//=====================================================================================================

#include "libhedrot_serialcomm.h"
#include "libhedrot_calibration.h"
#include "libhedrot_RTmagCalibration.h"


// hedrot version
#define HEDROT_VERSION "1.2.0beta1"

//=====================================================================================================
// static definitions
//=====================================================================================================

#define RAWDATA_STRING_MAX_SIZE 100 // should be bigger as NUMBER_OF_BYTES_IN_RAWDATA_FRAME

// time constants
#define PINGTIME                0.5  // time delay in seconds between two pings when the headtracker has been found
#define AUTODISCOVER_MAX_TIME   0.1  // max time period in seconds between autodiscover ping and headtracker response

// prototypes
#if defined(_WIN32) || defined(_WIN64)
# define strtok_r strtok_s // strtok_r does not exist on windows, use strtok_s instead
#endif /* #if defined(_WIN32) || defined(_WIN64) */


// communication states
#define COMMUNICATION_STATE_NO_CONNECTED_HEADTRACKER                0 //no connection with the head tracker
#define COMMUNICATION_STATE_AUTODISCOVERING_STARTED                 1 //no connection with the head tracker, autodiscovering started
#define COMMUNICATION_STATE_AUTODISCOVERING_WAITING_FOR_RESPONSE    2 //still no connected headtracker, waiting the response from a candidate
#define COMMUNICATION_STATE_AUTODISCOVERING_NO_HEADTRACKER_THERE    3 //the tested comm port is not a headtracker
#define COMMUNICATION_STATE_AUTODISCOVERING_HEADTRACKER_FOUND       4 //the headtracker has been found by the autodiscover function
#define COMMUNICATION_STATE_WAITING_FOR_INFO                        5 //connected to the head tracker, waiting for the head tracker to respond to the info request
#define COMMUNICATION_STATE_RECEIVING_INFO                          6 //connected to the head tracker, receiving info
#define COMMUNICATION_STATE_HEADTRACKER_TRANSMITTING                7 //connected to the head tracker, head tracker is transmitting raw data

// verbose states
#define VERBOSE_STATE_ONLY_ERROR_MESSAGES               0
#define VERBOSE_STATE_MAIN_MESSAGES                     1
#define VERBOSE_STATE_ALL_MESSAGES                      2

// constants for the notification messages FIFO list
#define MAX_NUMBER_OF_NOTIFICATION_MESSAGES             1000
#define NOTIFICATION_MESSAGE_NONE                       0
#define NOTIFICATION_MESSAGE_COMM_PORT_LIST_UPDATED     1
#define NOTIFICATION_MESSAGE_PORT_OPENED                2
#define NOTIFICATION_MESSAGE_WRONG_FIRMWARE_VERSION     3
#define NOTIFICATION_MESSAGE_HEADTRACKER_STATUS_CHANGED 4
#define NOTIFICATION_MESSAGE_SETTINGS_DATA_TRANSMISSION_FAILED 5
#define NOTIFICATION_MESSAGE_SETTINGS_DATA_READY        6
#define NOTIFICATION_MESSAGE_EXPORT_SETTINGS_FAILED     7
#define NOTIFICATION_MESSAGE_IMPORT_SETTINGS_FAILED     8
#define NOTIFICATION_MESSAGE_CALIBRATION_NOT_VALID      9
#define NOTIFICATION_MESSAGE_GYRO_CALIBRATION_STARTED   10
#define NOTIFICATION_MESSAGE_GYRO_CALIBRATION_FINISHED  11
#define NOTIFICATION_MESSAGE_MAG_CALIBRATION_STARTED    21
#define NOTIFICATION_MESSAGE_MAG_CALIBRATION_SUCCEEDED  22
#define NOTIFICATION_MESSAGE_MAG_CALIBRATION_FAILED     23
#define NOTIFICATION_MESSAGE_EXPORT_MAGCALDATARAWSAMPLES_FAILED 24
#define NOTIFICATION_MESSAGE_ACC_CALIBRATION_STARTED    31
#define NOTIFICATION_MESSAGE_ACC_CALIBRATION_SUCCEEDED  32
#define NOTIFICATION_MESSAGE_ACC_CALIBRATION_FAILED     33
#define NOTIFICATION_MESSAGE_ACC_CALIBRATION_PAUSED     34
#define NOTIFICATION_MESSAGE_ACC_CALIBRATION_RESUMED     35
#define NOTIFICATION_MESSAGE_EXPORT_ACCCALDATARAWSAMPLES_FAILED 36
#define NOTIFICATION_MESSAGE_MAG_RT_CALIBRATION_SUCCEEDED 40
#define NOTIFICATION_MESSAGE_BOARD_OVERLOAD             50


//=====================================================================================================
// structure definition: the headtrackerData "object"
//=====================================================================================================

typedef struct _headtrackerData {
    // structure for serial communication
    headtrackerSerialcomm *serialcomm;
    
    // global receiver infos and settings
    char            calibrationValid;
    char            headtracker_on;
    char            verbose;
    
    // configuration flags
    char            autoDiscover;
    
    
    //------------------------- HEAD TRACKER SETTINGS ------------------------
    
    // angle estimation coefficients
    float           MadgwickBetaMax;
    float           MadgwickBetaGain;
    float           accLPtimeConstant; // lowpass filter time constant in seconds for the accel data
    float           accLPalpha; // lowpass filter coefficient for the accel data (internal)
    
    // axes reference setting
    // 0: X->right, Y->back, Z->down
    // 1: X->right, Y->front, Z->up
    // 2: X->front, Y->left, Z->up
    char            axesReference;
    
    // rotation sequence setting
    // 0: Yaw-Pitch-Roll (YZX)
    // 1: Roll-Pitch-Yaw (XZZ)
    char            rotationOrder;
    
    // invert angles flag
    // 0: don't invert
    // 1: invert
    char            invertRotation;
    
    // sensor infos and settings
    char            firmwareVersion; //headtracker firmware version
    long            samplerate;
    float           samplePeriod; // internal
    
    // calibration settings
    char            offlineCalibrationMethod; //0 = double ellipsoid fit, 1 = Aussal
    
    // gyroscope settings
    unsigned char   gyroDataRate;
    unsigned char   gyroClockSource;
    unsigned char   gyroDLPFBandwidth;
    unsigned char   gyroOffsetAutocalOn;
    
    // accelerometer settings
    unsigned char   accRange;
    char			accHardOffset[3];
    unsigned char   accFullResolutionBit;
    unsigned char   accDataRate;
    
    // magnetometer settings
    unsigned char   magMeasurementBias;
    unsigned char   magSampleAveraging;
    unsigned char   magDataRate;
    unsigned char   magGain;
    unsigned char   magMeasurementMode;
    
    // calibration values und settings
    float           gyroOffset[3];
    float           gyroOffsetAutocalTime; // in ms
    long            gyroOffsetAutocalThreshold; //in LSB units
    long            gyroOffsetAutocalCounter; //internal
    long            gyroOffsetAutocalMin[3]; //internal, in LSB units
    long            gyroOffsetAutocalMax[3]; //internal, in LSB units
    long            gyroOffsetAutocalSum[3]; //internal, in LSB units
    float           gyroscopeCalibrationFactor; //internal, in rad/sec/LSB
    char            gyroOffsetCalibratedState; // internal
    long            gyroHalfScaleSensitivity; // internal
    long            gyroBitDepth; // internal
    
    float           magOffset[3];
    float           magScaling[3], magScalingFactor[3];
    calibrationData* magCalibrationData;
    char            magCalibratingFlag;
    
    float           accOffset[3];
    float           accScaling[3], accScalingFactor[3];
    calibrationData* accCalibrationData;
    char            accCalibratingFlag;
    float           accCalMaxGyroNorm; // maximum allowed norm for the gyroscope when calibrating accelerometer
    char            accCalPauseStatus; // 0 if acquiring samples, 1 if paused because norm for the gyroscope too high
    
    RTmagCalData*   RTmagCalibrationData;
    short           RTMagCalAcquisitionRateFactor;
    short           RTMagCalAcquisitionRateCounter;
    char            RTmagCalOn;
    float           RTmagMaxDistanceError;
    short           RTMagCalibrationRateFactor;
    
    //--------------- OUTPUTS AND STATUS FROM THE HEAD TRACKER -------------------
    
    // info status variables and flags
    char            infoReceptionStatus; //see constants "communication states"
    char            trackingDataReady;
    
    // buffer for raw data
    unsigned char   rawDataBuffer[RAWDATA_STRING_MAX_SIZE];
    int             rawDataBufferIndex;
    
    // raw data pro sensor
    short           magRawData[3];
    short           accRawData[3];
    short           gyroRawData[3];
    
    // calibrated data pro sensor
    float           magCalData[3];
    float           accCalData[3];
    float           gyroCalData[3];
    
    // estimated quaternion
    float           q1, q2, q3, q4; //quaternion values W,X,Y,Z (should be set to zero when chaning method)
    
    // estimated angles (yaw,pitch,roll)
    float           yaw,pitch,roll;
    float           centeredYaw,centeredPitch,centeredRoll;
    
    // reference quaternion for centering
    float           qref1, qref2, qref3, qref4;
    
    // centered quaternion
    float           qcent1, qcent2, qcent3, qcent4;
    
    // internal variables for the computation of the angles
    float           accCalDataLP[3]; // low-pass filtered acc data
    float           accLPstate[3]; // history
    float           beta; //dynamically calculated
    
    // internal variables for timing
    double          scheduledNextPingTime;
    double          autodiscoverResponseTimeLimit;
    
    // notification message FIFO list (implemented as a circular buffer)
    int             numberOfMessages;
    int             firstMessageToNotify;
    char            messagesToNotify[MAX_NUMBER_OF_NOTIFICATION_MESSAGES];
    
} headtrackerData;


//=====================================================================================================
// "public" functions declarations
//=====================================================================================================
headtrackerData* headtracker_new();
void headtracker_free(headtrackerData* trackingData);
void headtracker_init(headtrackerData *trackingData);
void headtracker_tick(headtrackerData *trackingData);
void center_angles(headtrackerData *trackingData);
void headtracker_open(headtrackerData *trackingData, int portnum);
void headtracker_close(headtrackerData *trackingData);
int  pullNotificationMessage(headtrackerData *trackingData);
void headtracker_list_comm_ports(headtrackerData *trackingData);
int export_headtracker_settings(headtrackerData *trackingData, char* filename);
int import_headtracker_settings(headtrackerData *trackingData, char* filename);
int export_magCalDataRawSamples(headtrackerData *trackingData, char* filename);
int export_accCalDataRawSamples(headtrackerData *trackingData, char* filename);

int calibrateAcc(headtrackerData *trackingData);
int calibrateMag(headtrackerData *trackingData);

//=====================================================================================================
// "public" setters for receiver parameters
//=====================================================================================================
void setHeadtrackerOn(headtrackerData *trackingData, char headtrackeronVal);
void setVerbose(headtrackerData *trackingData, char verboseVal);
void setAutoDiscover(headtrackerData *trackingData, char autoDiscover);
void setGyroOffsetAutocalOn(headtrackerData *trackingData, char gyroOffsetAutocalOn);
void setGyroOffsetAutocalTime(headtrackerData *trackingData, float gyroOffsetAutocalTime);
void setGyroOffsetAutocalThreshold(headtrackerData *trackingData, long gyroOffsetAutocalThreshold);
void setMadgwickBetaGain(headtrackerData *trackingData, float MadgwickBetaGain);
void setMadgwickBetaMax(headtrackerData *trackingData, float MadgwickBetaMax);
void setAccLPtimeConstant(headtrackerData *trackingData, float accLPtimeConstant);
void setAxesReference(headtrackerData *trackingData, char axesReference);
void setRotationOrder(headtrackerData *trackingData, char rotationOrder);
void setInvertRotation(headtrackerData *trackingData, char invertRotation);
void setMagCalibratingFlag(headtrackerData *trackingData, char magCalibratingFlag);
void setAccCalibratingFlag(headtrackerData *trackingData, char accCalibratingFlag);
void setAccCalMaxGyroNorm(headtrackerData *trackingData, float accCalMaxGyroNorm);
void setOfflineCalibrationMethod(headtrackerData *trackingData, char offlineCalibrationMethod);




//=====================================================================================================
// public setters to send attributes to the headtracker
//=====================================================================================================
void setSamplerate(headtrackerData *trackingData, long sr, char requestSettingsFlag);
void setgyroDataRate(headtrackerData *trackingData, unsigned char gyroDataRate, char requestSettingsFlag);
void setGyroClockSource(headtrackerData *trackingData, unsigned char gyroClockSource, char requestSettingsFlag);
void setGyroDLPFBandwidth(headtrackerData *trackingData, unsigned char gyroDLPFBandwidth, char requestSettingsFlag);
void setAccRange(headtrackerData *trackingData, unsigned char accRange, char requestSettingsFlag);
void setAccHardOffset(headtrackerData *trackingData, char* accHardOffset, char requestSettingsFlag);
void setAccFullResolutionBit(headtrackerData *trackingData, unsigned char accFullResolutionBit, char requestSettingsFlag);
void setAccDataRate(headtrackerData *trackingData, unsigned char accDataRate, char requestSettingsFlag);
void setMagMeasurementBias(headtrackerData *trackingData, unsigned char magMeasurementBias, char requestSettingsFlag);
void setMagSampleAveraging(headtrackerData *trackingData, unsigned char magSampleAveraging, char requestSettingsFlag);
void setMagDataRate(headtrackerData *trackingData, unsigned char magDataRate, char requestSettingsFlag);
void setMagGain(headtrackerData *trackingData, unsigned char magGain, char requestSettingsFlag);
void setMagMeasurementMode(headtrackerData *trackingData, unsigned char magMeasurementMode, char requestSettingsFlag);

//=====================================================================================================
// public setters to send calibration attributes to the headtracker
//=====================================================================================================
void setAccOffset(headtrackerData *trackingData, float* accOffset, char requestSettingsFlag);
void setAccScaling(headtrackerData *trackingData, float* accScaling, char requestSettingsFlag);
void setMagOffset(headtrackerData *trackingData, float* magOffset, char requestSettingsFlag);
void setMagScaling(headtrackerData *trackingData, float* magScaling, char requestSettingsFlag);


//=====================================================================================================
// public setters for mag real-time calibration
//=====================================================================================================
void setRTmagCalOn(headtrackerData *trackingData, char RTmagCalOn);

//=====================================================================================================
// "private" setters
//=====================================================================================================
void headtracker_setReceptionStatus(headtrackerData *trackingData, int n);

//=====================================================================================================
// "private" functions declarations
//=====================================================================================================
void headtracker_requestHeadtrackerSettings(headtrackerData *trackingData);
int processInfoFromHeadtracker(headtrackerData *trackingData, int offset, int numberOfBytes);
void gyroOffsetCalibration(headtrackerData *trackingData);
void headtracker_autodiscover(headtrackerData *trackingData);
void headtracker_autodiscover_tryNextPort(headtrackerData *trackingData);
void headtracker_compute_data(headtrackerData *trackingData);
void convert_7bytes_to_3int16(unsigned char *rawDataBuffer,int baseIndex,short *rawDataToSend);
char MadgwickAHRSupdateModified(headtrackerData *trackingData);
void pushNotificationMessage(headtrackerData *trackingData, char messageNumber);
void headtracker_sendFloatArray2Headtracker(headtrackerData *trackingData, float* data, int numValues, unsigned char StartTransmitChar, unsigned char StopTransmitChar);
void headtracker_sendSignedCharArray2Headtracker(headtrackerData *trackingData, char* data, int numValues, unsigned char StartTransmitChar, unsigned char StopTransmitChar);
void resetGyroOffsetCalibration(headtrackerData *trackingData);
int  processKeyValueSettingPair(headtrackerData *trackingData, char *key, char *value, char UpdateHeadtrackerFlag);
void changeQuaternionReference(headtrackerData *trackingData);
void changeRTMagCalAcquisitionRateFactor(headtrackerData *trackingData);


#endif
//=====================================================================================================
// End of file
//=====================================================================================================
