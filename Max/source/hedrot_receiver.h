// hedrot_receiver.h
//
// Copyright 2016 Alexis Baskind

#ifndef hedrot_receiver_h
#define hedrot_receiver_h

#include "ext.h"
#include "ext_common.h"
#include "ext_strings.h"
#include "ext_proto.h"
#include "ext_support.h"
#include "ext_maxtypes.h"
#include "ext_mess.h"
#include "ext_user.h"
#include "ext_critical.h"

#include <string.h>
#include <errno.h>
#include <stdio.h>

#include "libhedrot.h"
#include "hedrot_comm_protocol.h"


// libhedrot software version
#define MAX_hedrot_receiver_VERSION 2

// time constants
#define SEARCHING_DEVICE_TIME          200  //time period between two ticks when the headtracker has still not been found

typedef struct hedrot_receiver
{
    t_object		x_obj;
    
    //outlets
    void			*x_rawData_outlet;
    void			*x_calData_outlet;
    void			*x_cookedAngles_outlet;
    void			*x_cookedQuaternions_outlet;
    void			*x_status_outlet;
    void			*x_error_outlet;
    void			*x_debug_outlet;
    
    
    // general variables about information reception
    t_object		*receive_and_output_clock;
    
    headtrackerData *trackingData;
    
    // mirror of the internal settings of the structure trackingData that are used as attributes by Max
    // unfortunately necessary, since attributes can be only linked to variables on the top level of the Max object struct
    char            verbose;
    char            headtracker_on;
    char            autoDiscover;
    char            outputCenteredAngles;
    long            samplerate;
    char            gyroDataRate;
    char            gyroClockSource;
    char            gyroDLPFBandwidth;
    char            gyroOffsetAutocalOn;
    char            accRange;
    char            accHardOffset[3];
    char            accFullResolutionBit;
    char            accDataRate;
    char            magMeasurementBias;
    char            magSampleAveraging;
    char            magDataRate;
    char            magGain;
    char            magMeasurementMode;
    float           gyroOffsetAutocalTime; // in ms
    long            gyroOffsetAutocalThreshold; //in LSB units
    float           accOffset[3];
    float           accScaling[3];
    float           magOffset[3];
    float           magScaling[3];
    float           MadgwickBetaMax;
    float           MadgwickBetaGain;
    float           accLPtimeConstant;
    
    
    // output data
    t_atom          t_estimatedAngles[3];
    t_atom          t_estimatedQuaternion[4];
    
    // (control) output data rate
    long            outputDataPeriod;
    
    // list to be send after formating...
    t_atom          rawData[9];
    // ... and after calibration
    t_atom          calData[9];
    
    // rec into text file
    char            filename[MAX_PATH_CHARS];
    t_filehandle	fh_write;
    char            recordingDataFlag;
    long            recsampleCount;
    char            tmpStr[1024];
    
} t_hedrot_receiver;


t_class *hedrot_receiver_class;


void *hedrot_receiver_new(t_symbol *s, short ac, t_atom *av);
void hedrot_receiver_init(t_hedrot_receiver *x);
void hedrot_receiver_tick(t_hedrot_receiver *x);
void hedrot_receiver_free_clock(t_hedrot_receiver *x);
void hedrot_receiver_free(t_hedrot_receiver *x);

// methods
void hedrot_receiver_open(t_hedrot_receiver *x, int n);
void hedrot_receiver_close(t_hedrot_receiver *x);
void hedrot_receiver_devices(t_hedrot_receiver *x);
void hedrot_receiver_assist(t_hedrot_receiver *x, void *b, long m, long a, char *s);
void hedrot_receiver_center_angles(t_hedrot_receiver *x);
void hedrot_receiver_export_settings(t_hedrot_receiver *x, t_symbol *s);
void hedrot_receiver_defered_export_settings(t_hedrot_receiver *x, t_symbol *s);
void hedrot_receiver_import_settings(t_hedrot_receiver *x, t_symbol *s);
void hedrot_receiver_defered_import_settings(t_hedrot_receiver *x, t_symbol *s);
void hedrot_receiver_printVersion(t_hedrot_receiver *x, t_symbol *s);


void hedrot_receiver_opendestinationtextfile(t_hedrot_receiver *x, t_symbol *s);
void hedrot_receiver_doopenforwrite(t_hedrot_receiver *x, t_symbol *s);
void hedrot_receiver_startrec(t_hedrot_receiver *x);
void hedrot_receiver_stoprec(t_hedrot_receiver *x);

void hedrot_receiver_output_data(t_hedrot_receiver *x);


void hedrot_receiver_autodiscover_deferedTick(t_hedrot_receiver *x, t_symbol *s, long argc, t_atom *argv);


// output messages
void hedrot_receiver_outputPortList(t_hedrot_receiver *x);
void hedrot_receiver_selectOpenedPortInList(t_hedrot_receiver *x);
void hedrot_receiver_outputWrongFirmwareVersionNotice(t_hedrot_receiver *x);
void hedrot_receiver_outputReceptionStatus(t_hedrot_receiver *x);
void hedrot_receiver_mirrorHeadtrackerInfo(t_hedrot_receiver *x);
void hedrot_receiver_outputCalibrationNotValidNotice(t_hedrot_receiver *x);
void hedrot_receiver_outputGyroCalibrationStartedNotice(t_hedrot_receiver *x);
void hedrot_receiver_outputGyroCalibrationFinishedNotice(t_hedrot_receiver *x);


//getters and setters
t_max_err hedrot_receiver_verbose_set(t_hedrot_receiver *x, t_object *attr, long argc, t_atom *argv);
t_max_err hedrot_receiver_headtracker_on_set(t_hedrot_receiver *x, t_object *attr, long argc, t_atom *argv);
t_max_err hedrot_receiver_autoDiscover_set(t_hedrot_receiver *x, t_object *attr, long argc, t_atom *argv);
t_max_err hedrot_receiver_samplerate_set(t_hedrot_receiver *x, t_object *attr, long argc, t_atom *argv);
t_max_err hedrot_receiver_outputCenteredAngles_set(t_hedrot_receiver *x, t_object *attr, long argc, t_atom *argv);
t_max_err hedrot_receiver_gyroDataRate_set(t_hedrot_receiver *x, t_object *attr, long argc, t_atom *argv);
t_max_err hedrot_receiver_gyroClockSource_set(t_hedrot_receiver *x, t_object *attr, long argc, t_atom *argv);
t_max_err hedrot_receiver_gyroDLPFBandwidth_set(t_hedrot_receiver *x, t_object *attr, long argc, t_atom *argv);
t_max_err hedrot_receiver_accRange_set(t_hedrot_receiver *x, t_object *attr, long argc, t_atom *argv);
t_max_err hedrot_receiver_accHardOffset_set(t_hedrot_receiver *x, t_object *attr, long argc, t_atom *argv);
t_max_err hedrot_receiver_accFullResolutionBit_set(t_hedrot_receiver *x, t_object *attr, long argc, t_atom *argv);
t_max_err hedrot_receiver_accDataRate_set(t_hedrot_receiver *x, t_object *attr, long argc, t_atom *argv);
t_max_err hedrot_receiver_magMeasurementBias_set(t_hedrot_receiver *x, t_object *attr, long argc, t_atom *argv);
t_max_err hedrot_receiver_magSampleAveraging_set(t_hedrot_receiver *x, t_object *attr, long argc, t_atom *argv);
t_max_err hedrot_receiver_magDataRate_set(t_hedrot_receiver *x, t_object *attr, long argc, t_atom *argv);
t_max_err hedrot_receiver_magGain_set(t_hedrot_receiver *x, t_object *attr, long argc, t_atom *argv) ;
t_max_err hedrot_receiver_magMeasurementMode_set(t_hedrot_receiver *x, t_object *attr, long argc, t_atom *argv);
t_max_err hedrot_receiver_gyroOffsetAutocalOn_set(t_hedrot_receiver *x, t_object *attr, long argc, t_atom *argv);
t_max_err hedrot_receiver_gyroOffsetAutocalTime_set(t_hedrot_receiver *x, t_object *attr, long argc, t_atom *argv);
t_max_err hedrot_receiver_gyroOffsetAutocalThreshold_set(t_hedrot_receiver *x, t_object *attr, long argc, t_atom *argv);
void hedrot_receiver_do_set_gyroOffsetAutocalOn(t_hedrot_receiver *x, char gyroOffsetAutocalOn);
t_max_err hedrot_receiver_accOffset_set(t_hedrot_receiver *x, t_object *attr, long argc, t_atom *argv);
t_max_err hedrot_receiver_accScaling_set(t_hedrot_receiver *x, t_object *attr, long argc, t_atom *argv);
t_max_err hedrot_receiver_magOffset_set(t_hedrot_receiver *x, t_object *attr, long argc, t_atom *argv);
t_max_err hedrot_receiver_magScaling_set(t_hedrot_receiver *x, t_object *attr, long argc, t_atom *argv);
t_max_err hedrot_receiver_outputDataPeriod_set(t_hedrot_receiver *x, t_object *attr, long argc, t_atom *argv);
t_max_err hedrot_receiver_MadgwickBetaGain_set(t_hedrot_receiver *x, t_object *attr, long argc, t_atom *argv);
t_max_err hedrot_receiver_MadgwickBetaMax_set(t_hedrot_receiver *x, t_object *attr, long argc, t_atom *argv);
t_max_err hedrot_receiver_accLPtimeConstant_set(t_hedrot_receiver *x, t_object *attr, long argc, t_atom *argv);

#endif
