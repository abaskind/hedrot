// hedrot_receiver.c
//
// Copyright 2016 Alexis Baskind


#include "hedrot_receiver.h"

#include "libhedrot_utils.h"

/* ------------------- main methods --------------------------- */

void hedrot_receiver_tick(t_hedrot_receiver *x) {
    t_atom output;
    t_ptr_size len;
    
    char messageNumber;
    
    if(x->trackingData->headtracker_on) {
        headtracker_tick(x->trackingData);
        
        // if the com port list has been updated, update the menu accordingly
        // if a port has been opened, set the menu entry accordingly
        
        
        // if tracking data is ready, output raw, cooked and debug data
        if(x->trackingData->trackingDataReady) {
            x->trackingData->trackingDataReady = 0; // set the flag to 0 since we read the data
            
            hedrot_receiver_output_data(x);
            
            
            // output debug variables
            atom_setfloat(&output, x->trackingData->beta);
            outlet_anything( x->x_debug_outlet, gensym("beta"), 1, &output);
            
            
            // if we're recording the data in a text file, add a new line
            if(x->recordingDataFlag != 0) {
                sprintf(x->tmpStr,
                        "%li, %i %i %i %i %i %i %i %i %i %.15f %.15f %.15f %.15f %.15f %.15f %.15f %.15f %.15f %.15f %.15f %.15f %.15f %.15f %.15f %.15f %.15f %.15f %.15f;\n",
                        x->recsampleCount,
                        x->trackingData->magRawData[0], x->trackingData->magRawData[1], x->trackingData->magRawData[2],
                        x->trackingData->accRawData[0], x->trackingData->accRawData[1], x->trackingData->accRawData[2],
                        x->trackingData->gyroRawData[0], x->trackingData->gyroRawData[1], x->trackingData->gyroRawData[2],
                        x->trackingData->magCalData[0], x->trackingData->magCalData[1], x->trackingData->magCalData[2],
                        x->trackingData->accCalData[0], x->trackingData->accCalData[1], x->trackingData->accCalData[2],
                        x->trackingData->accCalDataLP[0], x->trackingData->accCalDataLP[1], x->trackingData->accCalDataLP[2],
                        x->trackingData->gyroCalData[0], x->trackingData->gyroCalData[1], x->trackingData->gyroCalData[2],
                        x->trackingData->q1, x->trackingData->q2, x->trackingData->q3, x->trackingData->q4,
                        x->trackingData->yaw, x->trackingData->pitch, x->trackingData->roll);
                len = strlen(x->tmpStr);
                if(sysfile_write(x->fh_write, &len, x->tmpStr)!=MAX_ERR_NONE)
                    error("[hedrot_receiver] : write error on text file");
                x->recsampleCount++;
            }
        }
    }
    
    // process messages to notify
    while( (messageNumber = pullNotificationMessage(x->trackingData)) ) {
        switch( messageNumber ) {
            case NOTIFICATION_MESSAGE_COMM_PORT_LIST_UPDATED:
                hedrot_receiver_outputPortList(x);
                break;
            case NOTIFICATION_MESSAGE_PORT_OPENED:
                hedrot_receiver_selectOpenedPortInList(x);
                break;
            case NOTIFICATION_MESSAGE_WRONG_FIRMWARE_VERSION:
                hedrot_receiver_outputWrongFirmwareVersionNotice(x);
                break;
            case NOTIFICATION_MESSAGE_HEADTRACKER_STATUS_CHANGED:
                hedrot_receiver_outputReceptionStatus(x);
                break;
            case NOTIFICATION_MESSAGE_SETTINGS_DATA_TRANSMISSION_FAILED:
                if(x->verbose) post("[hedrot_receiver] : settings not correctly transmitted by head tracker");
                break;
            case NOTIFICATION_MESSAGE_SETTINGS_DATA_READY:
                hedrot_receiver_mirrorHeadtrackerInfo(x);
                break;
            case NOTIFICATION_MESSAGE_EXPORT_SETTINGS_FAILED:
                if(x->verbose) post("[hedrot_receiver] : could not save settings into file");
                break;
            case NOTIFICATION_MESSAGE_IMPORT_SETTINGS_FAILED:
                if(x->verbose) post("[hedrot_receiver] : could not read settings from file");
                break;
            case NOTIFICATION_MESSAGE_CALIBRATION_NOT_VALID:
                hedrot_receiver_outputCalibrationNotValidNotice(x);
                break;
            case NOTIFICATION_MESSAGE_GYRO_CALIBRATION_STARTED:
                hedrot_receiver_outputGyroCalibrationStartedNotice(x);
                break;
            case NOTIFICATION_MESSAGE_GYRO_CALIBRATION_FINISHED:
                hedrot_receiver_outputGyroCalibrationFinishedNotice(x);
                break;
            case NOTIFICATION_MESSAGE_MAG_CALIBRATION_STARTED:
                hedrot_receiver_outputMagCalibrationStartedNotice(x);
                break;
            case NOTIFICATION_MESSAGE_MAG_CALIBRATION_SUCCEEDED:
                hedrot_receiver_outputMagCalibrationSucceededNotice(x);
                break;
            case NOTIFICATION_MESSAGE_MAG_CALIBRATION_FAILED:
                hedrot_receiver_outputMagCalibrationFailedNotice(x);
                break;
            case NOTIFICATION_MESSAGE_EXPORT_MAGCALDATARAWSAMPLES_FAILED:
                if(x->verbose) post("[hedrot_receiver] : could not save mag raw calibration data");
                break;
            case NOTIFICATION_MESSAGE_ACC_CALIBRATION_STARTED:
                hedrot_receiver_outputAccCalibrationStartedNotice(x);
                break;
            case NOTIFICATION_MESSAGE_ACC_CALIBRATION_SUCCEEDED:
                hedrot_receiver_outputAccCalibrationSucceededNotice(x);
                break;
            case NOTIFICATION_MESSAGE_ACC_CALIBRATION_FAILED:
                hedrot_receiver_outputAccCalibrationFailedNotice(x);
                break;
            case NOTIFICATION_MESSAGE_ACC_CALIBRATION_PAUSED:
                hedrot_receiver_outputAccCalibrationPausedNotice(x);
                break;
            case NOTIFICATION_MESSAGE_ACC_CALIBRATION_RESUMED:
                hedrot_receiver_outputAccCalibrationResumedNotice(x);
                break;
            case NOTIFICATION_MESSAGE_EXPORT_ACCCALDATARAWSAMPLES_FAILED:
                if(x->verbose) post("[hedrot_receiver] : could not save acc raw calibration data");
                break;
            case NOTIFICATION_MESSAGE_MAG_RT_CALIBRATION_SUCCEEDED:
                if(x->verbose) post("[hedrot_receiver] : magnetometer new calibrated");
                hedrot_receiver_dumpRTmagCalInfo(x);
                break;
            case NOTIFICATION_MESSAGE_EXPORT_RTMAGCALDATARAWSAMPLES_FAILED:
                if(x->verbose) post("[hedrot_receiver] : could not save raw real-time mag calibration data");
                break;
            case NOTIFICATION_MESSAGE_BOARD_OVERLOAD:
                if(x->verbose) post("[hedrot_receiver] : board too slow, reduce samplerate");
                break;
            default:
                post("[hedrot_receiver] : unknown message %ld from libhedrot", messageNumber);
                break;
                
        }
    }
    
    if(x->verbose == VERBOSE_STATE_ALL_MESSAGES) post("[hedrot_receiver]: tick");
    
    // schedule the next clock
    // if the head tracker is transmitting, schedule the next tick with the faster clock
    // in any other modes, schedule the next tick with the slower clock
    if(x->trackingData->infoReceptionStatus == COMMUNICATION_STATE_HEADTRACKER_TRANSMITTING) {
        clock_fdelay(x->receive_and_output_clock, x->outputDataPeriod);
        if(x->verbose == VERBOSE_STATE_ALL_MESSAGES) post("[hedrot_receiver]: next tick scheduled in %ld ms", x->outputDataPeriod);
    } else if((x->trackingData->infoReceptionStatus == COMMUNICATION_STATE_AUTODISCOVERING_STARTED)
              || (x->trackingData->infoReceptionStatus == COMMUNICATION_STATE_AUTODISCOVERING_WAITING_FOR_RESPONSE)) {
        // the receiver is in autodiscover mode and still no headtracker has been found
        // the next tick should be sent in defered mode in order to avoid to block Max
        defer((t_object *)x, (method)hedrot_receiver_autodiscover_deferedTick, NULL, 0, NULL);
    } else {
        clock_fdelay(x->receive_and_output_clock, SEARCHING_DEVICE_TIME);
        if(x->verbose == VERBOSE_STATE_ALL_MESSAGES) post("[hedrot_receiver]: next tick scheduled in %ld ms", SEARCHING_DEVICE_TIME);
    }
    
}

void hedrot_receiver_autodiscover_deferedTick(t_hedrot_receiver *x, t_symbol *s, long argc, t_atom *argv) {
    clock_fdelay(x->receive_and_output_clock, SEARCHING_DEVICE_TIME);
    if(x->verbose == VERBOSE_STATE_ALL_MESSAGES) post("[hedrot_receiver]: next tick scheduled and defered in %ld ms", SEARCHING_DEVICE_TIME);
}


void hedrot_receiver_output_data(t_hedrot_receiver *x)
{
    int i;
    
    for (i=0; i < 3; i++) {
        //prepare the lists (raw and calibrated)
        atom_setlong(x->rawData+i,x->trackingData->magRawData[i]);
        atom_setlong(x->rawData+i+3,x->trackingData->accRawData[i]);
        atom_setlong(x->rawData+i+6,x->trackingData->gyroRawData[i]);
        
        atom_setfloat(x->calData+i,x->trackingData->magCalData[i]);
        atom_setfloat(x->calData+i+3,x->trackingData->accCalDataLP[i]);
        atom_setfloat(x->calData+i+6,x->trackingData->gyroCalData[i]);
    }
    
    // output data, right-to-left order
    if(x->trackingData->calibrationValid) {
        atom_setfloat(x->t_estimatedQuaternion,(float)x->trackingData->qcent1);
        atom_setfloat(x->t_estimatedQuaternion+1,(float)x->trackingData->qcent2);
        atom_setfloat(x->t_estimatedQuaternion+2,(float)x->trackingData->qcent3);
        atom_setfloat(x->t_estimatedQuaternion+3,(float)x->trackingData->qcent4);
        
        
        atom_setfloat(x->t_estimatedAngles,(float)x->trackingData->yaw);
        atom_setfloat(x->t_estimatedAngles+1,(float)x->trackingData->pitch);
        atom_setfloat(x->t_estimatedAngles+2,(float)x->trackingData->roll);
        
        outlet_list(x->x_cookedQuaternions_outlet,NULL,4,x->t_estimatedQuaternion);
        outlet_list(x->x_cookedAngles_outlet,NULL,3,x->t_estimatedAngles);
        outlet_list(x->x_calData_outlet,NULL,9,x->calData);
    }
    
    outlet_list(x->x_rawData_outlet,NULL,9,x->rawData);
    
}

void hedrot_receiver_init(t_hedrot_receiver *x) {
    if(x->verbose) post("initializing hedrot_receiver \n");
    
    // initialization of the receiver object
    headtracker_init(x->trackingData);
    
    
    // init variables related to data recording
    strcpy(x->filename,"");
    x->fh_write = NULL;
    x->recordingDataFlag = 0;
    
    // init clocks
    
    hedrot_receiver_free_clock(x);
    // prepare a clock for the main tick method and start it at low rate first
    x->receive_and_output_clock = (t_object*) clock_new(x, (method)hedrot_receiver_tick);
    clock_fdelay(x->receive_and_output_clock, SEARCHING_DEVICE_TIME);
}

void *hedrot_receiver_new(t_symbol *s, short ac, t_atom *av)
{
    t_hedrot_receiver *x;
    
    t_atom temp_atom[2];
    
    x = (t_hedrot_receiver *)object_alloc(hedrot_receiver_class);
    
    // create outlets
    x->x_debug_outlet = outlet_new(x, 0);
    x->x_error_outlet = outlet_new(x, 0);
    x->x_status_outlet = outlet_new(x, 0);
    x->x_cookedQuaternions_outlet = outlet_new(x, 0);
    x->x_cookedAngles_outlet = outlet_new(x, 0);
    x->x_calData_outlet = outlet_new(x, 0);
    x->x_rawData_outlet = outlet_new(x, 0);
    
    // create the headtracker object with default values
    // those default values will be updated afterwards by the patch
    x->trackingData = headtracker_new();
    
    // mirror all default values to the max object
    hedrot_receiver_mirrorHeadtrackerInfo(x);
    
    // default output data rate
    x->outputDataPeriod = 5;
    
    // dictionaries and buffers for calibration data
    x->magCalInfoDict = dictionary_new();
    x->magCalInfoDictName = symbol_unique();
    x->magCalInfoDict = dictobj_register(x->magCalInfoDict, &x->magCalInfoDictName);
    // creates the embedded buffer (for norm of the mag cal data)
    x->magCalDataCalNorm_BufferObjName = symbol_unique();
    atom_setsym(temp_atom, x->magCalDataCalNorm_BufferObjName);
    x->magCalDataCalNorm_BufferObj = (t_buffer_obj*) object_new_typed(CLASS_BOX, gensym("buffer~"), 1, temp_atom);
    if(x->magCalDataCalNorm_BufferObj) {
        // resize the buffer
        atom_setlong(temp_atom, MAX_NUMBER_OF_SAMPLES_FOR_CALIBRATION);
        if(!object_method_typed (x->magCalDataCalNorm_BufferObj, gensym("sizeinsamps"), 1L, temp_atom, temp_atom + 1) == MAX_ERR_NONE) {
            error("[hedrot_receiver] (method hedrot_receiver_new): problem while changing buffer size");
        }
    } else {
        object_error( (t_object *)x, "problem while changing buffer size");
    }

    
    
    x->accCalInfoDict = dictionary_new();
    x->accCalInfoDictName = symbol_unique();
    x->accCalInfoDict = dictobj_register(x->accCalInfoDict, &x->accCalInfoDictName);
    // creates the embedded buffer (for norm of the acc cal data)
    x->accCalDataCalNorm_BufferObjName = symbol_unique();
    atom_setsym(temp_atom, x->accCalDataCalNorm_BufferObjName);
    x->accCalDataCalNorm_BufferObj = (t_buffer_obj*) object_new_typed(CLASS_BOX, gensym("buffer~"), 1, temp_atom);
    if(x->accCalDataCalNorm_BufferObj) {
        // resize the buffer
        atom_setlong(temp_atom, MAX_NUMBER_OF_SAMPLES_FOR_CALIBRATION);
        if(!object_method_typed (x->accCalDataCalNorm_BufferObj, gensym("sizeinsamps"), 1L, temp_atom, temp_atom + 1) == MAX_ERR_NONE) {
            error("[hedrot_receiver] (method hedrot_receiver_new): problem while changing buffer size");
        }
    } else {
        object_error( (t_object *)x, "problem while changing buffer size");
    }
    
    x->RTmagCalInfoDict = dictionary_new();
    x->RTmagCalInfoDictName = symbol_unique();
    x->RTmagCalInfoDict = dictobj_register(x->RTmagCalInfoDict, &x->RTmagCalInfoDictName);
    // creates the embedded buffer (for norm of the RT mag cal data)
    x->RTmagCalDataCalNorm_BufferObjName = symbol_unique();
    atom_setsym(temp_atom, x->RTmagCalDataCalNorm_BufferObjName);
    x->RTmagCalDataCalNorm_BufferObj = (t_buffer_obj*) object_new_typed(CLASS_BOX, gensym("buffer~"), 1, temp_atom);
    if(x->RTmagCalDataCalNorm_BufferObj) {
        // resize the buffer
        atom_setlong(temp_atom, MAX_NUMBER_OF_SAMPLES_FOR_CALIBRATION);
        if(!object_method_typed (x->RTmagCalDataCalNorm_BufferObj, gensym("sizeinsamps"), 1L, temp_atom, temp_atom + 1) == MAX_ERR_NONE) {
            error("[hedrot_receiver] (method hedrot_receiver_new): problem while changing buffer size");
        }
    } else {
        object_error( (t_object *)x, "problem while changing buffer size");
    }
    hedrot_receiver_init(x);
    
    
    return x;
}

void hedrot_receiver_free_clock(t_hedrot_receiver *x) {
    if(x->verbose) post("[hedrot_receiver] free clock...");
    if(x->receive_and_output_clock!=NULL)
        clock_free(x->receive_and_output_clock);
}


void hedrot_receiver_free(t_hedrot_receiver *x)
{
    if(x->verbose) post("[hedrot_receiver] free memory...");
    
    hedrot_receiver_close(x);
    hedrot_receiver_free_clock(x);
    
    object_free((t_object *)x->magCalInfoDict); // will call object_unregister
    object_free((t_object *)x->accCalInfoDict); // will call object_unregister
    
    object_free(x->magCalDataCalNorm_BufferObj);
    object_free(x->accCalDataCalNorm_BufferObj);
}


/* ---------------- METHODS ------------------------- */

void hedrot_receiver_open(t_hedrot_receiver *x, int n)
{
    // only transmit the open command from outside if autodiscover is not on
    if(!x->trackingData->autoDiscover)
        headtracker_open(x->trackingData, n);
}

void hedrot_receiver_close(t_hedrot_receiver *x)
{
    if(x->verbose) post("[hedrot_receiver] closing port...");
    
    headtracker_close(x->trackingData);
    
    hedrot_receiver_init(x);
}

void hedrot_receiver_devices(t_hedrot_receiver *x) {
    headtracker_list_comm_ports(x->trackingData);
}

void hedrot_receiver_assist(t_hedrot_receiver *x, void *b, long m, long a, char *s)
{
    if (m == ASSIST_INLET) {	// Inlets
        sprintf(s, "commands to be sent to the headtracker, configuration of the receiver");
    }
    else {						// Outlets
        switch (a) {
            case 0: sprintf(s, "raw headtracker data"); break;
            case 1: sprintf(s, "calibrated headtracker data"); break;
            case 2: sprintf(s, "cooked angles"); break;
            case 3: sprintf(s, "cooked quaternion"); break;
            case 4: sprintf(s, "status"); break;
            case 5: sprintf(s, "error messages"); break;
        }
    }
}


void hedrot_receiver_center_angles(t_hedrot_receiver *x) {
    center_angles(x->trackingData);
}


/* ------------------- methods for importing/exporting headtracker settings --------------------------- */

void hedrot_receiver_export_settings(t_hedrot_receiver *x, t_symbol *s) {
    defer((t_object *)x, (method)hedrot_receiver_defered_export_settings, s, 0, NULL);
}


void hedrot_receiver_defered_export_settings(t_hedrot_receiver *x, t_symbol *s) {
    t_fourcc filetype = FOUR_CHAR_CODE('TEXT'), outtype;
    char filename[MAX_PATH_CHARS], fullfilename[MAX_PATH_CHARS];
    short path=0;
    
    if (s == gensym("")) {      // if no argument supplied, ask for file
        sprintf(filename, "headtrackerSettings.txt");
        
        saveas_promptset("Save hedrot settings as...");
        if (saveasdialog_extended(filename, &path, &outtype, &filetype, 1))
            // non-zero: user cancelled
            return;
    } else {
        strcpy(filename, s->s_name);
    }
    
    path_toabsolutesystempath( path, filename, fullfilename);
    
    post("[hedrot_receiver]: try to open file %s for storing settings", fullfilename);
    
    if(!export_headtracker_settings(x->trackingData, fullfilename))
        error("[hedrot_receiver] Error while exporting settings");
    
}


void hedrot_receiver_import_settings(t_hedrot_receiver *x, t_symbol *s) {
    defer((t_object *)x, (method)hedrot_receiver_defered_import_settings, s, 0, NULL);
}


void hedrot_receiver_defered_import_settings(t_hedrot_receiver *x, t_symbol *s) {
    t_fourcc filetype = FOUR_CHAR_CODE('TEXT'), outtype;
    char filename[MAX_PATH_CHARS], fullfilename[MAX_PATH_CHARS];
    short path=0;
    
    if (s == gensym("")) {      // if no argument supplied, ask for file
        sprintf(filename, "");
        
        open_promptset("Open hedrot settings from...");
        if (open_dialog(filename, &path, &outtype, &filetype, 1))
            // non-zero: user cancelled
            return;
    } else {
        strcpy(filename, s->s_name);
    }
    
    path_toabsolutesystempath( path, filename, fullfilename);
    
    post("[hedrot_receiver]: try to open file %s for retrieving settings", fullfilename);
    
    if(!import_headtracker_settings(x->trackingData, fullfilename))
        error("[hedrot_receiver] Error while importing settings");
    
}

void hedrot_receiver_printVersion(t_hedrot_receiver *x, t_symbol *s) {
    t_atom message_version;
    
    post("[hedrot_receiver] Max object based on hedrot version %s, compiled on "__DATE__, HEDROT_VERSION);
    post("[hedrot_receiver] Required firmware version %d", HEDROT_FIRMWARE_VERSION);
    
    // output version number on status outlet
    atom_setsym(&message_version, gensym(HEDROT_VERSION));
    outlet_anything(x->x_status_outlet, gensym("version"), 1, &message_version);
    
    
}

/* ------------------- methods for mag calibration --------------------------- */

void hedrot_receiver_startMagCalibration(t_hedrot_receiver *x) {
    setMagCalibratingFlag(x->trackingData, 1);
}

void hedrot_receiver_stopMagCalibration(t_hedrot_receiver *x) {
    setMagCalibratingFlag(x->trackingData, 0);
}

void hedrot_receiver_redoMagCalibration(t_hedrot_receiver *x) {
    calibrateMag(x->trackingData);
}

void hedrot_receiver_saveMagCalibration(t_hedrot_receiver *x) {
    setMagScaling(x->trackingData, x->magScaling, 0);
    setMagOffset(x->trackingData, x->magOffset, 1);
    if(x->verbose) post("[hedrot_receiver]: mag calibration data saved");
}


void hedrot_receiver_dumpMagCalInfo(t_hedrot_receiver *x) {
    // dump extended calibration info for the magnetometer as a dictionary
    t_atom output[3];
    
    if( !hedrot_receiver_createCalDataDictionary( x->magOffset, x->magScaling, x->trackingData->magCalibrationData,
                                                 x->magCalInfoDict, x->magCalDataCalSamples_Matrix,
                                                 x->magCalDataCalNorm_BufferObj, x->magCalDataCalNorm_BufferObjName) ) {
        // output the dictionary
        atom_setsym(output, gensym("calData"));
        atom_setsym(output+1, _sym_dictionary);
        atom_setsym(output+2, x->magCalInfoDictName);
        outlet_anything( x->x_status_outlet, gensym("calibrating_mag"), 3, output);
    }
}

void hedrot_receiver_exportMagRawCalData(t_hedrot_receiver *x, t_symbol *s) {
    defer((t_object *)x, (method)hedrot_receiver_defered_exportMagRawCalData, s, 0, NULL);
}


void hedrot_receiver_defered_exportMagRawCalData(t_hedrot_receiver *x, t_symbol *s) {
    t_fourcc filetype = FOUR_CHAR_CODE('TEXT'), outtype;
    char filename[MAX_PATH_CHARS], fullfilename[MAX_PATH_CHARS];
    short path=0;
    
    if (s == gensym("")) {      // if no argument supplied, ask for file
        sprintf(filename, "headtrackerRawMagCalibrationData.txt");
        
        saveas_promptset("Save raw magnetometer calibration data as...");
        if (saveasdialog_extended(filename, &path, &outtype, &filetype, 1))
            // non-zero: user cancelled
            return;
    } else {
        strcpy(filename, s->s_name);
    }
    
    path_toabsolutesystempath( path, filename, fullfilename);
    
    post("[hedrot_receiver]: try to open file %s for storing raw magnetometer calibration data", fullfilename);
    
    if(!export_magCalDataRawSamples(x->trackingData, fullfilename))
        error("[hedrot_receiver] Error while exporting raw magnetometer calibration data");
    
}


/* ------------------- methods for acc calibration --------------------------- */

void hedrot_receiver_startAccCalibration(t_hedrot_receiver *x) {
    setAccCalibratingFlag(x->trackingData, 1);
}

void hedrot_receiver_stopAccCalibration(t_hedrot_receiver *x) {
    setAccCalibratingFlag(x->trackingData, 0);
}

void hedrot_receiver_redoAccCalibration(t_hedrot_receiver *x) {
    calibrateAcc(x->trackingData);
}

void hedrot_receiver_saveAccCalibration(t_hedrot_receiver *x) {
    setAccScaling(x->trackingData, x->accScaling, 0);
    setAccOffset(x->trackingData, x->accOffset, 1);
    if(x->verbose) post("[hedrot_receiver]: acc calibration data saved");
}


void hedrot_receiver_dumpAccCalInfo(t_hedrot_receiver *x) {
    // dump extended calibration info for the accelerometer as a dictionary
    t_atom output[3];
    
    if( !hedrot_receiver_createCalDataDictionary( x->accOffset, x->accScaling, x->trackingData->accCalibrationData,
                                                 x->accCalInfoDict, x->accCalDataCalSamples_Matrix,
                                                 x->accCalDataCalNorm_BufferObj, x->accCalDataCalNorm_BufferObjName) ) {
        // output the dictionary
        atom_setsym(output, gensym("calData"));
        atom_setsym(output+1, _sym_dictionary);
        atom_setsym(output+2, x->accCalInfoDictName);
        outlet_anything( x->x_status_outlet, gensym("calibrating_acc"), 3, output);
    }
}

void hedrot_receiver_exportAccRawCalData(t_hedrot_receiver *x, t_symbol *s) {
    defer((t_object *)x, (method)hedrot_receiver_defered_exportAccRawCalData, s, 0, NULL);
}


void hedrot_receiver_defered_exportAccRawCalData(t_hedrot_receiver *x, t_symbol *s) {
    t_fourcc filetype = FOUR_CHAR_CODE('TEXT'), outtype;
    char filename[MAX_PATH_CHARS], fullfilename[MAX_PATH_CHARS];
    short path=0;
    
    if (s == gensym("")) {      // if no argument supplied, ask for file
        sprintf(filename, "headtrackerRawAccCalibrationData.txt");
        
        saveas_promptset("Save raw accelerometer calibration data as...");
        if (saveasdialog_extended(filename, &path, &outtype, &filetype, 1))
            // non-zero: user cancelled
            return;
    } else {
        strcpy(filename, s->s_name);
    }
    
    path_toabsolutesystempath( path, filename, fullfilename);
    
    post("[hedrot_receiver]: try to open file %s for storing raw accelerometer calibration data", fullfilename);
    
    if(!export_accCalDataRawSamples(x->trackingData, fullfilename))
        error("[hedrot_receiver] Error while exporting raw accelerometer calibration data");
    
}


char hedrot_receiver_createCalDataDictionary( float offset[], float scaling[], calibrationData *calData,
                                             t_dictionary *calDict, t_jit_object *sampleMatrix,
                                             t_buffer_obj *normBufferObj, t_symbol *normBufferObjName) {
    int i;
    t_atom offsetData[3];
    t_atom ScalingData[3];
    t_float *tab; // buffer values
    
    t_jit_matrix_info samplesMatrix_info;
    t_symbol *calDataCalSamples_matrixName;
    void *calDataCalSamples_matrixPointer;
    float *calDataCalSamples_matrixPointer_bp, *samples_matrixPointer;
    long savelock;
    
    dictionary_clear(calDict);
    
    dictionary_appendlong(calDict, gensym("numberOfSamples"), calData->numberOfSamples);
    
    atom_setfloat(offsetData, offset[0]);
    atom_setfloat(offsetData+1, offset[1]);
    atom_setfloat(offsetData+2, offset[2]);
    dictionary_appendatoms(calDict, gensym("offset"), 3, offsetData);
    
    atom_setfloat(ScalingData, scaling[0]);
    atom_setfloat(ScalingData+1, scaling[1]);
    atom_setfloat(ScalingData+2, scaling[2]);
    dictionary_appendatoms(calDict, gensym("scaling"), 3, ScalingData);
    
    dictionary_appendfloat(calDict, gensym("conditionNumber"), calData->conditionNumber);
    
    dictionary_appendfloat(calDict, gensym("normAverage"), calData->normAverage);
    
    dictionary_appendfloat(calDict, gensym("normStdDev"), calData->normStdDev);
    
    dictionary_appendfloat(calDict, gensym("maxNormError"), calData->maxNormError);
    
    // create the matrix that store the calibrated samples
    jit_matrix_info_default(&samplesMatrix_info);
    samplesMatrix_info.dimcount = 1;
    samplesMatrix_info.type = _jit_sym_float32;
    samplesMatrix_info.planecount = 3;
    samplesMatrix_info.dim[ 0 ] = calData->numberOfSamples;
    if(sampleMatrix) jit_object_free(sampleMatrix); // free the matrix if it already exists
    sampleMatrix = jit_object_new( _jit_sym_jit_matrix, &samplesMatrix_info ); // create the matrix
    calDataCalSamples_matrixName = symbol_unique();
    jit_object_register( sampleMatrix, calDataCalSamples_matrixName );
    
    // get the zeroth index from the corresponding list
    calDataCalSamples_matrixPointer    = jit_object_method(sampleMatrix,_jit_sym_getindex,0);
    
    if(calDataCalSamples_matrixPointer) {
        // lock the matrix
        savelock = (long) jit_object_method(calDataCalSamples_matrixPointer, _jit_sym_lock, 1);
        
        // get the matrix data pointer
        jit_object_method(calDataCalSamples_matrixPointer, _jit_sym_getdata, &calDataCalSamples_matrixPointer_bp);
        
        // fill the matrix
        if (calDataCalSamples_matrixPointer_bp) {
            
            // initialization
            samples_matrixPointer = calDataCalSamples_matrixPointer_bp;
            
            for(i = 0; i<calData->numberOfSamples; i++) {
                // samples matrix: fill for each plane (corresponding to x,y and z coordinates)
                *samples_matrixPointer++ = calData->calSamples[i][0]; // x
                *samples_matrixPointer++ = calData->calSamples[i][1]; // y
                *samples_matrixPointer++ = calData->calSamples[i][2]; // z
            }
            
            // at the end of the loop, add the reference to the matrix to the dictionary
            dictionary_appendsym( calDict, gensym("CalData"), calDataCalSamples_matrixName);
        } else {
            error("[hedrot_receiver] (method dumpAccCalInfo): error creating output jitter matrix");
            return 1; // error
        }
        // restore matrix lock state to previous value
        jit_object_method(calDataCalSamples_matrixPointer, _jit_sym_lock, savelock);
        
        tab = buffer_locksamples(normBufferObj);
        
        if (tab) {
            for(i = 0; i < calData->numberOfSamples; i++) {
                tab[i] = calData->dataNorm[i];
            }
            
            for(i = calData->numberOfSamples; i < MAX_NUMBER_OF_SAMPLES_FOR_CALIBRATION; i++) {
                tab[i] = 0;
            }
            
            buffer_unlocksamples(normBufferObj);
            
            // call the dirty flag to refresh the waveform views
            buffer_setdirty(normBufferObj);
            
            // at the end of the loop, add the reference to the buffer to the dictionary and output the dictionary
            dictionary_appendsym( calDict, gensym("calDataNorm"), normBufferObjName);
            
            return 0; // no error
            
        } else
        {
            error("[hedrot_receiver] (method hedrot_receiver_createCalDataDictionary):Write error on buffer");
            
            return 1; // error
        }
    } else {
        error("[hedrot_receiver] (method hedrot_receiver_createCalDataDictionary): error creating output jitter matrix");
        return 1; // error
    }
}

// methods for mag RT calibration


void hedrot_receiver_dumpRTmagCalInfo(t_hedrot_receiver *x) {
    // dump extended real-time calibration info for the magnetometer as a dictionary
    t_atom output[3];
    
    if( !hedrot_receiver_createCalDataDictionary( x->trackingData->RTmagCalibrationData->estimatedOffset,
                                                 x->trackingData->RTmagCalibrationData->estimatedScaling,
                                                 x->trackingData->RTmagCalibrationData->calData,
                                                 x->RTmagCalInfoDict, x->RTmagCalDataCalSamples_Matrix,
                                                 x->RTmagCalDataCalNorm_BufferObj, x->RTmagCalDataCalNorm_BufferObjName) ) {
        // output the dictionary
        atom_setsym(output, gensym("calData"));
        atom_setsym(output+1, _sym_dictionary);
        atom_setsym(output+2, x->RTmagCalInfoDictName);
        outlet_anything( x->x_status_outlet, gensym("RTmag_calibration"), 3, output);
    }
}

void hedrot_receiver_exportRTmagRawCalData(t_hedrot_receiver *x, t_symbol *s) {
    defer((t_object *)x, (method)hedrot_receiver_defered_exportRTmagRawCalData, s, 0, NULL);
}


void hedrot_receiver_defered_exportRTmagRawCalData(t_hedrot_receiver *x, t_symbol *s) {
    t_fourcc filetype = FOUR_CHAR_CODE('TEXT'), outtype;
    char filename[MAX_PATH_CHARS], fullfilename[MAX_PATH_CHARS];
    short path=0;
    
    if (s == gensym("")) {      // if no argument supplied, ask for file
        sprintf(filename, "headtrackerRawRTmagCalibrationData.txt");
        
        saveas_promptset("Save raw real-time magnetometer calibration data as...");
        if (saveasdialog_extended(filename, &path, &outtype, &filetype, 1))
            // non-zero: user cancelled
            return;
    } else {
        strcpy(filename, s->s_name);
    }
    
    path_toabsolutesystempath( path, filename, fullfilename);
    
    post("[hedrot_receiver]: try to open file %s for storing raw real-time magnetometer calibration data", fullfilename);
    
    if(!export_RTmagCalDataRawSamples(x->trackingData, fullfilename))
        error("[hedrot_receiver] Error while exporting raw real-time magnetometer calibration data");
    
}


void hedrot_receiver_outputPortList(t_hedrot_receiver *x) {
    t_atom message_clear[2], outptr[4];
    int i;
    
    atom_setsym(message_clear, gensym("available_ports"));
    atom_setsym(message_clear+1, gensym("clear"));
    outlet_anything(x->x_status_outlet, gensym("serial"), 2, message_clear);
    
    if(x->verbose == VERBOSE_STATE_ALL_MESSAGES) post("[hedrot_receiver]: list of available comm ports updated: %ld ports:", x->trackingData->serialcomm->numberOfAvailablePorts);
    
    for(i=0; i<x->trackingData->serialcomm->numberOfAvailablePorts; i++) {
        // now append the list
        atom_setsym(outptr, gensym("available_ports"));
        atom_setsym(outptr+1, gensym("append"));
        atom_setlong(outptr+2, i);
        atom_setsym(outptr+3, gensym(x->trackingData->serialcomm->availablePorts[i]));
        outlet_anything(x->x_status_outlet, gensym("serial"), 4, outptr);
        
        if(x->verbose == VERBOSE_STATE_ALL_MESSAGES) post("[hedrot_receiver]: port %ld, %s", i, x->trackingData->serialcomm->availablePorts[i]);
        
    }
    
}


void hedrot_receiver_selectOpenedPortInList(t_hedrot_receiver *x) {
    t_atom outptr[3];
    atom_setsym(outptr, gensym("available_ports"));
    atom_setsym(outptr+1, gensym("set"));
    atom_setlong(outptr+2, x->trackingData->serialcomm->portNumber);
    outlet_anything(x->x_status_outlet, gensym("serial"), 3, outptr);
}

// update the headtracker settings for the GUI
void hedrot_receiver_outputWrongFirmwareVersionNotice(t_hedrot_receiver *x) {
    t_atom output;
    
    char str[80];
    
    sprintf(str, "Wrong Headtracker Firmware Version - expected version: %i - actual version: %i", HEDROT_FIRMWARE_VERSION, x->trackingData->firmwareVersion);
    
    atom_setsym(&output, gensym(str));
    
    outlet_anything( x->x_error_outlet, gensym("wrong_firmware_version"), 1, &output);
    
    if(x->verbose) error("[hedrot_receiver]: wrong firmware version");
}


void hedrot_receiver_outputGyroCalibrationStartedNotice(t_hedrot_receiver *x) {
    t_atom output;
    atom_setlong(&output, 1);
    outlet_anything( x->x_status_outlet, gensym("calibrating_gyro_offset"), 1, &output);
    
    if(x->verbose) post("[hedrot_receiver]: gyroscope offsetcalibration started");
}


void hedrot_receiver_outputGyroCalibrationFinishedNotice(t_hedrot_receiver *x) {
    t_atom output, output3[3];
    atom_setlong(&output, 0);
    outlet_anything( x->x_status_outlet, gensym("calibrating_gyro_offset"), 1, &output);
    
    atom_setfloat_array( 3, output3, 3, x->trackingData->gyroOffset);
    outlet_anything( x->x_debug_outlet, gensym("gyroOffset"), 3, output3);
    
    if(x->verbose) post("[hedrot_receiver]: gyroscope offset calibration finished");
}

void hedrot_receiver_outputMagCalibrationStartedNotice(t_hedrot_receiver *x) {
    t_atom output[2];
    atom_setsym(output, gensym("status"));
    atom_setsym(output+1, gensym("started"));
    outlet_anything( x->x_status_outlet, gensym("calibrating_mag"), 2, output);
    
    if(x->verbose) post("[hedrot_receiver]: magnetometer calibration started");
}


void hedrot_receiver_outputMagCalibrationSucceededNotice(t_hedrot_receiver *x) {
    t_atom output[2];
    int i;
    
    for(i=0;i<3;i++) x->magOffset[i] = x->trackingData->magOffset[i];
    object_attr_touch( (t_object *)x, gensym("magOffset"));
    
    for(i=0;i<3;i++) x->magScaling[i] = x->trackingData->magScaling[i];
    object_attr_touch( (t_object *)x, gensym("magScaling"));
    
    atom_setsym(output, gensym("status"));
    atom_setsym(output+1, gensym("succeeded"));
    outlet_anything( x->x_status_outlet, gensym("calibrating_mag"), 2, output);
    
    if(x->verbose) post("[hedrot_receiver]: magnetometer calibration succeeded");
}

void hedrot_receiver_outputMagCalibrationFailedNotice(t_hedrot_receiver *x) {
    t_atom output[2];
    atom_setsym(output, gensym("status"));
    atom_setsym(output+1, gensym("failed"));
    outlet_anything( x->x_status_outlet, gensym("calibrating_mag"), 2, output);
    
    if(x->verbose) post("[hedrot_receiver]: magnetometer calibration failed");
}

void hedrot_receiver_outputAccCalibrationStartedNotice(t_hedrot_receiver *x) {
    t_atom output[2];
    atom_setsym(output, gensym("status"));
    atom_setsym(output+1, gensym("started"));
    outlet_anything( x->x_status_outlet, gensym("calibrating_acc"), 2, output);
    
    if(x->verbose) post("[hedrot_receiver]: accelerometer calibration started");
}


void hedrot_receiver_outputAccCalibrationSucceededNotice(t_hedrot_receiver *x) {
    t_atom output[2];
    int i;
    
    for(i=0;i<3;i++) x->accOffset[i] = x->trackingData->accOffset[i];
    object_attr_touch( (t_object *)x, gensym("accOffset"));
    
    for(i=0;i<3;i++) x->accScaling[i] = x->trackingData->accScaling[i];
    object_attr_touch( (t_object *)x, gensym("accScaling"));
    
    atom_setsym(output, gensym("status"));
    atom_setsym(output+1, gensym("succeeded"));
    outlet_anything( x->x_status_outlet, gensym("calibrating_acc"), 2, output);
    
    if(x->verbose) post("[hedrot_receiver]: accelerometer calibration succeeded");
}

void hedrot_receiver_outputAccCalibrationFailedNotice(t_hedrot_receiver *x) {
    t_atom output[2];
    atom_setsym(output, gensym("status"));
    atom_setsym(output+1, gensym("failed"));
    outlet_anything( x->x_status_outlet, gensym("calibrating_acc"), 2, output);
    
    if(x->verbose) post("[hedrot_receiver]: accelerometer calibration failed");
}

void hedrot_receiver_outputAccCalibrationPausedNotice(t_hedrot_receiver *x) {
    t_atom output[2];
    atom_setsym(output, gensym("status"));
    atom_setsym(output+1, gensym("paused"));
    outlet_anything( x->x_status_outlet, gensym("calibrating_acc"), 2, output);
    
    if(x->verbose) post("[hedrot_receiver]: accelerometer calibration paused");
}

void hedrot_receiver_outputAccCalibrationResumedNotice(t_hedrot_receiver *x) {
    t_atom output[2];
    atom_setsym(output, gensym("status"));
    atom_setsym(output+1, gensym("resumed"));
    outlet_anything( x->x_status_outlet, gensym("calibrating_acc"), 2, output);
    
    if(x->verbose) post("[hedrot_receiver]: accelerometer calibration resumed");
}




void hedrot_receiver_outputReceptionStatus(t_hedrot_receiver *x) {
    t_atom sym[2];
    
    if(x->verbose) post("new reception status: %ld",x->trackingData->infoReceptionStatus);
    
    atom_setsym(&sym[0], gensym("connection_status"));
    switch(x->trackingData->infoReceptionStatus) {
        case COMMUNICATION_STATE_NO_CONNECTED_HEADTRACKER:
            atom_setsym(&sym[1], gensym("no headtracker connected"));
            break;
        case COMMUNICATION_STATE_AUTODISCOVERING_STARTED:
            atom_setsym(&sym[1], gensym("no headtracker connected, autodiscovering"));
            break;
        case COMMUNICATION_STATE_AUTODISCOVERING_WAITING_FOR_RESPONSE:
            atom_setsym(&sym[1], gensym("candidate found, waiting for confirm"));
            break;
        case COMMUNICATION_STATE_AUTODISCOVERING_NO_HEADTRACKER_THERE:
            atom_setsym(&sym[1], gensym("candidate is not a headtracker"));
            break;
        case COMMUNICATION_STATE_AUTODISCOVERING_HEADTRACKER_FOUND:
            atom_setsym(&sym[1], gensym("headtracker found"));
            break;
        case COMMUNICATION_STATE_WAITING_FOR_INFO:
            atom_setsym(&sym[1], gensym("headtracker connected, waiting for info"));
            break;
        case COMMUNICATION_STATE_RECEIVING_INFO:
            atom_setsym(&sym[1], gensym("receiving info from headtracker"));
            break;
        case COMMUNICATION_STATE_HEADTRACKER_TRANSMITTING:
            atom_setsym(&sym[1], gensym("headtracker transmitting..."));
            break;
    }
    
    outlet_anything( x->x_status_outlet, gensym("headtracker"), 2, sym);
    
    if(x->verbose) post("[hedrot_receiver]: communication status changed to %ld", x->trackingData->infoReceptionStatus);
}


// update the headtracker settings for the GUI
void hedrot_receiver_mirrorHeadtrackerInfo(t_hedrot_receiver *x) {
    t_atom sym[2];
    
    int i;
    
    if(x->verbose) post("Headtracker settings retrieved\n");
    
    atom_setsym(sym, gensym("headtracker_settings_retrieved"));
    atom_setsym(sym+1, gensym("bang"));
    outlet_anything( x->x_status_outlet, gensym("headtracker"), 2, sym);
    
    // tell all attributes related to headtracker settings that they have to update their status
    x->verbose = x->trackingData->verbose;
    object_attr_touch( (t_object *)x, gensym("verbose"));
    
    x->headtracker_on = x->trackingData->headtracker_on;
    object_attr_touch( (t_object *)x, gensym("headtracker_on"));
    
    x->autoDiscover = x->trackingData->autoDiscover;
    object_attr_touch( (t_object *)x, gensym("autoDiscover"));
    
    x->samplerate = x->trackingData->samplerate;
    object_attr_touch( (t_object *)x, gensym("samplerate"));
    
    x->gyroDataRate = x->trackingData->gyroDataRate;
    object_attr_touch( (t_object *)x, gensym("gyroDataRate"));
    
    x->gyroClockSource = x->trackingData->gyroClockSource;
    object_attr_touch( (t_object *)x, gensym("gyroClockSource"));
    
    x->gyroDLPFBandwidth = x->trackingData->gyroDLPFBandwidth;
    object_attr_touch( (t_object *)x, gensym("gyroDLPFBandwidth"));
    
    x->gyroOffsetAutocalOn = x->trackingData->gyroOffsetAutocalOn;
    object_attr_touch( (t_object *)x, gensym("gyroOffsetAutocalOn"));
    
    x->accRange = x->trackingData->accRange;
    object_attr_touch( (t_object *)x, gensym("accRange"));
    
    for(i=0;i<3;i++) x->accHardOffset[i] = (long) x->trackingData->accHardOffset[i];
    object_attr_touch( (t_object *)x, gensym("accHardOffset"));
    
    x->accFullResolutionBit = x->trackingData->accFullResolutionBit;
    object_attr_touch( (t_object *)x, gensym("accFullResolutionBit"));
    
    x->accDataRate = x->trackingData->accDataRate;
    object_attr_touch( (t_object *)x, gensym("accDataRate"));
    
    x->magMeasurementBias = x->trackingData->magMeasurementBias;
    object_attr_touch( (t_object *)x, gensym("magMeasurementBias"));
    
    x->magSampleAveraging = x->trackingData->magSampleAveraging;
    object_attr_touch( (t_object *)x, gensym("magSampleAveraging"));
    
    x->magDataRate = x->trackingData->magDataRate;
    object_attr_touch( (t_object *)x, gensym("magDataRate"));
    
    x->magGain = x->trackingData->magGain;
    object_attr_touch( (t_object *)x, gensym("magGain"));
    
    x->magMeasurementMode = x->trackingData->magMeasurementMode;
    object_attr_touch( (t_object *)x, gensym("magMeasurementMode"));
    
    x->gyroOffsetAutocalTime = x->trackingData->gyroOffsetAutocalTime;
    object_attr_touch( (t_object *)x, gensym("gyroOffsetAutocalTime"));
    
    x->gyroOffsetAutocalThreshold = x->trackingData->gyroOffsetAutocalThreshold;
    object_attr_touch( (t_object *)x, gensym("gyroOffsetAutocalThreshold"));
    
    for(i=0;i<3;i++) x->accOffset[i] = x->trackingData->accOffset[i];
    object_attr_touch( (t_object *)x, gensym("accOffset"));
    
    for(i=0;i<3;i++) x->accScaling[i] = x->trackingData->accScaling[i];
    object_attr_touch( (t_object *)x, gensym("accScaling"));
    
    for(i=0;i<3;i++) x->magOffset[i] = x->trackingData->magOffset[i];
    object_attr_touch( (t_object *)x, gensym("magOffset"));
    
    for(i=0;i<3;i++) x->magScaling[i] = x->trackingData->magScaling[i];
    object_attr_touch( (t_object *)x, gensym("magScaling"));
    
    x->MadgwickBetaMax = x->trackingData->MadgwickBetaMax;
    object_attr_touch( (t_object *)x, gensym("MadgwickBetaMax"));
    
    x->MadgwickBetaGain = x->trackingData->MadgwickBetaGain;
    object_attr_touch( (t_object *)x, gensym("MadgwickBetaGain"));
    
    x->accLPtimeConstant = x->trackingData->accLPtimeConstant;
    object_attr_touch( (t_object *)x, gensym("accLPtimeConstant"));
    
    x->axesReference = x->trackingData->axesReference;
    object_attr_touch( (t_object *)x, gensym("axesReference"));
    
    x->rotationOrder = x->trackingData->rotationOrder;
    object_attr_touch( (t_object *)x, gensym("rotationOrder"));
    
    x->invertRotation = x->trackingData->invertRotation;
    object_attr_touch( (t_object *)x, gensym("invertRotation"));
    
    x->accCalMaxGyroNorm = x->trackingData->accCalMaxGyroNorm;
    object_attr_touch( (t_object *)x, gensym("accCalMaxGyroNorm"));
    
    x->offlineCalibrationMethod = x->trackingData->offlineCalibrationMethod;
    object_attr_touch( (t_object *)x, gensym("offlineCalibrationMethod"));

    x->RTmagCalibrationMethod = x->trackingData->RTmagCalibrationMethod;
    object_attr_touch( (t_object *)x, gensym("RTmagCalibrationMethod"));

    x->RTmagCalOn = x->trackingData->RTmagCalOn;
    object_attr_touch( (t_object *)x, gensym("RTmagCalOn"));
    
    x->RTmagMaxMemoryDuration = x->trackingData->RTmagMaxMemoryDuration;
    object_attr_touch( (t_object *)x, gensym("RTmagMaxMemoryDuration"));
    
    x->RTmagMaxDistanceError = x->trackingData->RTmagMaxDistanceError;
    object_attr_touch( (t_object *)x, gensym("RTmagMaxDistanceError"));
    
    x->RTMagCalibrationPeriod = x->trackingData->RTMagCalibrationPeriod;
    object_attr_touch( (t_object *)x, gensym("RTMagCalibrationPeriod"));

}

void hedrot_receiver_outputCalibrationNotValidNotice(t_hedrot_receiver *x) {
    t_atom output;
    atom_setsym(&output, gensym("Headtracker's accelerometer not calibrated or bad calibrated, please calibrate it first"));
    
    outlet_anything( x->x_error_outlet, gensym("calibration_not_valid"), 1, &output);
}




/* ---------------- REC DATA INTO TEXT FILE ------------------------- */
void hedrot_receiver_opendestinationtextfile(t_hedrot_receiver *x, t_symbol *s) {
    defer((t_object *)x, (method)hedrot_receiver_doopenforwrite, s, 0, NULL);
}


void hedrot_receiver_doopenforwrite(t_hedrot_receiver *x, t_symbol *s) {
    t_fourcc filetype = FOUR_CHAR_CODE('TEXT'), outtype;
    char filename[MAX_PATH_CHARS];
    short path=0;
    long err;
    t_filehandle fh_write;
    char str[1024];
    t_ptr_size len;
    
    
    if (s == gensym("")) {      // if no argument supplied, ask for file
        filename[0] = 0;
        if (saveasdialog_extended(filename, &path, &outtype, &filetype, 1))
            // non-zero: user cancelled
            return;
    } else {
        strcpy(filename, s->s_name);
    }
    
    post("t_hedrot_receiver: try to open file %s (path ID %d) for recording data", filename, path);
    
    if ((err = path_createsysfile(filename, path, 'TEXT', &fh_write))) {
        error("t_hedrot_receiver: %s: error %d creating file", filename, err);
        return;
    }
    
    post("t_hedrot_receiver: file %s opened for recording data", filename);
    
    // the file is opened, save the infos
    x->fh_write = fh_write;
    path_topathname( path, filename, x->filename);
    
    // write the header
    len = strlen("<header>\n");
    sysfile_write(x->fh_write, &len, "<header>\n");
    
    sprintf(str, "samplerate, %li;\n", x->trackingData->samplerate);
    len = strlen(str);
    sysfile_write(x->fh_write, &len, str);
    
    sprintf(str, "gyroDataRate, %hhi;\n", x->trackingData->gyroDataRate);
    len = strlen(str);
    sysfile_write(x->fh_write, &len, str);
    
    sprintf(str, "gyroClockSource, %hhi;\n", x->trackingData->gyroClockSource);
    len = strlen(str);
    sysfile_write(x->fh_write, &len, str);
    
    sprintf(str, "gyroDLPFBandwidth, %hhi;\n", x->trackingData->gyroDLPFBandwidth);
    len = strlen(str);
    sysfile_write(x->fh_write, &len, str);
    
    sprintf(str, "gyroOffset, %f %f %f;\n", x->trackingData->gyroOffset[0], x->trackingData->gyroOffset[1], x->trackingData->gyroOffset[2]);
    len = strlen(str);
    sysfile_write(x->fh_write, &len, str);
    
    sprintf(str, "accRange, %hhi;\n", x->trackingData->accRange);
    len = strlen(str);
    sysfile_write(x->fh_write, &len, str);
    
    sprintf(str, "accFullResolutionBit, %hhi;\n", x->trackingData->accFullResolutionBit);
    len = strlen(str);
    sysfile_write(x->fh_write, &len, str);
    
    sprintf(str, "accDataRate, %hhi;\n", x->trackingData->accDataRate);
    len = strlen(str);
    sysfile_write(x->fh_write, &len, str);
    
    sprintf(str, "magSampleAveraging, %hhi;\n", x->trackingData->magSampleAveraging);
    len = strlen(str);
    sysfile_write(x->fh_write, &len, str);
    
    sprintf(str, "magDataRate, %hhi;\n", x->trackingData->magDataRate);
    len = strlen(str);
    sysfile_write(x->fh_write, &len, str);
    
    sprintf(str, "magGain, %hhi;\n", x->trackingData->magGain);
    len = strlen(str);
    sysfile_write(x->fh_write, &len, str);
    
    sprintf(str, "magMeasurementMode, %hhi;\n", x->trackingData->magMeasurementMode);
    len = strlen(str);
    sysfile_write(x->fh_write, &len, str);
    
    sprintf(str, "magOffset, %f %f %f;\n", x->trackingData->magOffset[0], x->trackingData->magOffset[1], x->trackingData->magOffset[2]);
    len = strlen(str);
    sysfile_write(x->fh_write, &len, str);
    
    sprintf(str, "magScaling, %f %f %f;\n", x->trackingData->magScaling[0], x->trackingData->magScaling[1], x->trackingData->magScaling[2]);
    len = strlen(str);
    sysfile_write(x->fh_write, &len, str);
    
    sprintf(str, "accOffset, %f %f %f;\n", x->trackingData->accOffset[0], x->trackingData->accOffset[1], x->trackingData->accOffset[2]);
    len = strlen(str);
    sysfile_write(x->fh_write, &len, str);
    
    sprintf(str, "accScaling, %f %f %f;\n", x->trackingData->accScaling[0], x->trackingData->accScaling[1], x->trackingData->accScaling[2]);
    len = strlen(str);
    sysfile_write(x->fh_write, &len, str);
    
    sprintf(str, "MadgwickBetaGain, %f;\n", x->trackingData->MadgwickBetaGain);
    len = strlen(str);
    sysfile_write(x->fh_write, &len, str);
    
    sprintf(str, "MadgwickBetaMax, %f;\n", x->trackingData->MadgwickBetaMax);
    len = strlen(str);
    sysfile_write(x->fh_write, &len, str);
    
    sprintf(str, "accLPtimeConstant, %f;\n", x->trackingData->accLPtimeConstant);
    len = strlen(str);
    sysfile_write(x->fh_write, &len, str);
    
    sprintf(str, "axesReference, %hhi;\n", x->trackingData->axesReference);
    len = strlen(str);
    sysfile_write(x->fh_write, &len, str);
    
    sprintf(str, "rotationOrder, %hhi;\n", x->trackingData->rotationOrder);
    len = strlen(str);
    sysfile_write(x->fh_write, &len, str);
    
    sprintf(str, "invertRotation, %hhi;\n", x->trackingData->invertRotation);
    len = strlen(str);
    sysfile_write(x->fh_write, &len, str);
    
    sprintf(str, "accCalMaxGyroNorm, %f;\n", x->trackingData->accCalMaxGyroNorm);
    len = strlen(str);
    sysfile_write(x->fh_write, &len, str);
    
    sprintf(str, "offlineCalibrationMethod, %hhi;\n", x->trackingData->offlineCalibrationMethod);
    len = strlen(str);
    sysfile_write(x->fh_write, &len, str);
    
    sprintf(str, "RTmagCalibrationMethod, %hhi;\n", x->trackingData->RTmagCalibrationMethod);
    len = strlen(str);
    sysfile_write(x->fh_write, &len, str);
    
    sprintf(str, "RTmagCalOn, %hhi;\n", x->trackingData->RTmagCalOn);
    len = strlen(str);
    sysfile_write(x->fh_write, &len, str);
    
    sprintf(str, "RTmagMaxMemoryDuration, %f;\n", x->trackingData->RTmagMaxMemoryDuration);
    len = strlen(str);
    sysfile_write(x->fh_write, &len, str);
    
    sprintf(str, "RTmagMaxDistanceError, %f;\n", x->trackingData->RTmagMaxDistanceError);
    len = strlen(str);
    sysfile_write(x->fh_write, &len, str);
    
    sprintf(str, "RTMagCalibrationPeriod, %f;\n", x->trackingData->RTMagCalibrationPeriod);
    len = strlen(str);
    sysfile_write(x->fh_write, &len, str);
    
    len = strlen("</header>\n");
    sysfile_write(x->fh_write, &len, "</header>\n");
}


void hedrot_receiver_startrec(t_hedrot_receiver *x) {
    //if the handle x->fh_write is NULL but if x->filename does not point to NULL, try to reopen the file
    if(x->fh_write == NULL) {
        if(x->filename != NULL) {
            hedrot_receiver_opendestinationtextfile(x,gensym(x->filename));
        } else {
            error("t_hedrot_receiver: no file opened");
            return;
        }
    }
    
    //sets the flag to 1 (and the main tick method will record into it)
    x->recordingDataFlag = 1;
    x->recsampleCount = 0;
}


void hedrot_receiver_stoprec(t_hedrot_receiver *x) {
    if(x->recordingDataFlag != 0) {
        // sets the flag to 0
        x->recordingDataFlag = 0;
        
        // closes the file
        sysfile_close(x->fh_write);
        
        // sets the handle x->fh_write to NULL
        x->fh_write = NULL;
    } else {
        error("t_hedrot_receiver: rec not started");
        return;
    }
}

/* ---------------- CUSTOM GETTERS AND SETTERS ------------------------- */
t_max_err hedrot_receiver_verbose_set(t_hedrot_receiver *x, t_object *attr, long argc, t_atom *argv) {
    if (argc && argv) {
        x->verbose = (char) atom_getlong(argv);
        
        setVerbose(x->trackingData, x->verbose);
    }
    
    return MAX_ERR_NONE;
}

t_max_err hedrot_receiver_headtracker_on_set(t_hedrot_receiver *x, t_object *attr, long argc, t_atom *argv) {
    if (argc && argv) {
        char headtrackeronVal = (char) atom_getlong(argv);
        
        if (headtrackeronVal < 0)       // bad value, don’t change anything
            return MAX_ERR_NONE;
        
        x->headtracker_on = headtrackeronVal;
        
        setHeadtrackerOn(x->trackingData, headtrackeronVal);
    }
    
    return MAX_ERR_NONE;
}


t_max_err hedrot_receiver_autoDiscover_set(t_hedrot_receiver *x, t_object *attr, long argc, t_atom *argv) {
    if (argc && argv) {
        x->autoDiscover = (char) atom_getlong(argv);
        
        setAutoDiscover(x->trackingData, x->autoDiscover);
    }
    
    return MAX_ERR_NONE;
}

t_max_err hedrot_receiver_samplerate_set(t_hedrot_receiver *x, t_object *attr, long argc, t_atom *argv) {
    if (argc && argv) {
        x->samplerate = (long) max(min(atom_getlong(argv),65535),2);
        setSamplerate(x->trackingData, x->samplerate, 1);
    }
    return MAX_ERR_NONE;
}

t_max_err hedrot_receiver_gyroDataRate_set(t_hedrot_receiver *x, t_object *attr, long argc, t_atom *argv) {
    if (argc && argv) {
        x->gyroDataRate = (unsigned char) max(min(atom_getlong(argv),255),0);
        setgyroDataRate(x->trackingData, x->gyroDataRate, 1);
    }
    return MAX_ERR_NONE;
}


t_max_err hedrot_receiver_gyroClockSource_set(t_hedrot_receiver *x, t_object *attr, long argc, t_atom *argv) {
    if (argc && argv) {
        x->gyroClockSource = (unsigned char) max(min(atom_getlong(argv),5),0);
        setGyroClockSource(x->trackingData, x->gyroClockSource, 1);
    }
    return MAX_ERR_NONE;
}


t_max_err hedrot_receiver_gyroDLPFBandwidth_set(t_hedrot_receiver *x, t_object *attr, long argc, t_atom *argv) {
    if (argc && argv) {
        x->gyroDLPFBandwidth = (unsigned char) max(min(atom_getlong(argv),6),0);
        setGyroDLPFBandwidth(x->trackingData, x->gyroDLPFBandwidth, 1);
    }
    return MAX_ERR_NONE;
}


t_max_err hedrot_receiver_accRange_set(t_hedrot_receiver *x, t_object *attr, long argc, t_atom *argv) {
    if (argc && argv) {
        x->accRange = (unsigned char) atom_getlong(argv);
        setAccRange(x->trackingData, x->accRange, 1);
    }
    return MAX_ERR_NONE;
}


t_max_err hedrot_receiver_accHardOffset_set(t_hedrot_receiver *x, t_object *attr, long argc, t_atom *argv) {
    int i;
    
    char accHardOffsetTMP[3];
    
    if (!argc)
        return MAX_ERR_GENERIC;
    
    for(i=0;i<3;i++,argv++) {
        x->accHardOffset[i] = max(min(atom_getlong(argv),127),-128);
        accHardOffsetTMP[i] = (char) x->accHardOffset[i];
    }
    
    setAccHardOffset(x->trackingData, accHardOffsetTMP, 1);
    
    return MAX_ERR_NONE;
}


t_max_err hedrot_receiver_accFullResolutionBit_set(t_hedrot_receiver *x, t_object *attr, long argc, t_atom *argv) {
    if (argc && argv) {
        x->accFullResolutionBit = (unsigned char) atom_getlong(argv);
        setAccFullResolutionBit(x->trackingData, x->accFullResolutionBit, 1);
    }
    return MAX_ERR_NONE;
}


t_max_err hedrot_receiver_accDataRate_set(t_hedrot_receiver *x, t_object *attr, long argc, t_atom *argv) {
    if (argc && argv) {
        x->accDataRate = (unsigned char) atom_getlong(argv);
        setAccDataRate(x->trackingData, x->accDataRate, 1);
    }
    return MAX_ERR_NONE;
}


t_max_err hedrot_receiver_magMeasurementBias_set(t_hedrot_receiver *x, t_object *attr, long argc, t_atom *argv) {
    if (argc && argv) {
        x->magMeasurementBias = (unsigned char) max(min(atom_getlong(argv),2),0);
        setMagMeasurementBias(x->trackingData, x->magMeasurementBias, 1);
    }
    return MAX_ERR_NONE;
}


t_max_err hedrot_receiver_magSampleAveraging_set(t_hedrot_receiver *x, t_object *attr, long argc, t_atom *argv) {
    if (argc && argv) {
        x->magSampleAveraging = (unsigned char) max(min(atom_getlong(argv),3),0);
        setMagSampleAveraging(x->trackingData, x->magSampleAveraging, 1);
    }
    return MAX_ERR_NONE;
}



t_max_err hedrot_receiver_magDataRate_set(t_hedrot_receiver *x, t_object *attr, long argc, t_atom *argv) {
    if (argc && argv) {
        x->magDataRate = (unsigned char) max(min(atom_getlong(argv),6),0);
        setMagDataRate(x->trackingData, x->magDataRate, 1);
    }
    return MAX_ERR_NONE;
}


t_max_err hedrot_receiver_magGain_set(t_hedrot_receiver *x, t_object *attr, long argc, t_atom *argv) {
    if (argc && argv) {
        x->magGain = (unsigned char) max(min(atom_getlong(argv),7),0);
        setMagGain(x->trackingData, x->magGain, 1);
    }
    return MAX_ERR_NONE;
}


t_max_err hedrot_receiver_magMeasurementMode_set(t_hedrot_receiver *x, t_object *attr, long argc, t_atom *argv) {
    if (argc && argv) {
        x->magMeasurementMode = (unsigned char) max(min(atom_getlong(argv),1),0);
        setMagMeasurementMode(x->trackingData, x->magMeasurementMode, 1);
    }
    return MAX_ERR_NONE;
}


t_max_err hedrot_receiver_gyroOffsetAutocalOn_set(t_hedrot_receiver *x, t_object *attr, long argc, t_atom *argv) {
    if (!argc)
        return MAX_ERR_GENERIC;
    
    x->gyroOffsetAutocalOn = (unsigned char) max(min(atom_getlong(argv),1),0);
    setGyroOffsetAutocalOn(x->trackingData, x->gyroOffsetAutocalOn);
    
    return MAX_ERR_NONE;
}


t_max_err hedrot_receiver_gyroOffsetAutocalTime_set(t_hedrot_receiver *x, t_object *attr, long argc, t_atom *argv) {
    if (!argc)
        return MAX_ERR_GENERIC;
    
    x->gyroOffsetAutocalTime = (float) atom_getfloat(argv);
    setGyroOffsetAutocalTime(x->trackingData, x->gyroOffsetAutocalTime);
    
    return MAX_ERR_NONE;
}


t_max_err hedrot_receiver_gyroOffsetAutocalThreshold_set(t_hedrot_receiver *x, t_object *attr, long argc, t_atom *argv) {
    if (!argc)
        return MAX_ERR_GENERIC;
    
    x->gyroOffsetAutocalThreshold = (long) max(min(atom_getlong(argv),10000),0);
    setGyroOffsetAutocalThreshold(x->trackingData, x->gyroOffsetAutocalThreshold);
    
    return MAX_ERR_NONE;
}


t_max_err hedrot_receiver_accOffset_set(t_hedrot_receiver *x, t_object *attr, long argc, t_atom *argv) {
    int i;
    
    if (!argc)
        return MAX_ERR_GENERIC;
    
    for(i=0;i<3;i++,argv++)
        x->accOffset[i] = (float) atom_getfloat(argv);
    
    setAccOffset(x->trackingData, x->accOffset, 1);
    
    return MAX_ERR_NONE;
}


t_max_err hedrot_receiver_accScaling_set(t_hedrot_receiver *x, t_object *attr, long argc, t_atom *argv) {
    int i;
    
    if (!argc)
        return MAX_ERR_GENERIC;
    
    
    for(i=0;i<3;i++,argv++)
        x->accScaling[i] = (float) atom_getfloat(argv);
    
    setAccScaling(x->trackingData, x->accScaling, 1);
    
    return MAX_ERR_NONE;
}


t_max_err hedrot_receiver_magOffset_set(t_hedrot_receiver *x, t_object *attr, long argc, t_atom *argv) {
    int i;
    
    if (!argc)
        return MAX_ERR_GENERIC;
    
    for(i=0;i<3;i++,argv++)
        x->magOffset[i] = (float) atom_getfloat(argv);
    
    setMagOffset(x->trackingData, x->magOffset, 1);
    
    return MAX_ERR_NONE;
}


t_max_err hedrot_receiver_magScaling_set(t_hedrot_receiver *x, t_object *attr, long argc, t_atom *argv) {
    int i;
    
    if (!argc)
        return MAX_ERR_GENERIC;
    
    for(i=0;i<3;i++,argv++)
        x->magScaling[i] = (float) atom_getfloat(argv);
    
    setMagScaling(x->trackingData, x->magScaling, 1);
    
    return MAX_ERR_NONE;
}



t_max_err hedrot_receiver_MadgwickBetaGain_set(t_hedrot_receiver *x, t_object *attr, long argc, t_atom *argv) {
    if (argc && argv) {
        x->MadgwickBetaGain = (float) atom_getfloat(argv);
        setMadgwickBetaGain(x->trackingData, x->MadgwickBetaGain);
    }
    return MAX_ERR_NONE;
}


t_max_err hedrot_receiver_MadgwickBetaMax_set(t_hedrot_receiver *x, t_object *attr, long argc, t_atom *argv) {
    if (argc && argv) {
        x->MadgwickBetaMax = (float) atom_getfloat(argv);
        setMadgwickBetaMax(x->trackingData, x->MadgwickBetaMax);
    }
    return MAX_ERR_NONE;
}


t_max_err hedrot_receiver_accLPtimeConstant_set(t_hedrot_receiver *x, t_object *attr, long argc, t_atom *argv) {
    if (argc && argv) {
        x->accLPtimeConstant = (float) atom_getfloat(argv);
        setAccLPtimeConstant(x->trackingData, x->accLPtimeConstant);
    }
    return MAX_ERR_NONE;
}

t_max_err hedrot_receiver_outputDataPeriod_set(t_hedrot_receiver *x, t_object *attr, long argc, t_atom *argv) {
    if (argc && argv) {
        x->outputDataPeriod = (long) max(min(atom_getlong(argv),500),1);
        if(x->verbose) post("[hedrot_receiver]: outputDataPeriod set to %ld ms", x->outputDataPeriod);
    }
    return MAX_ERR_NONE;
}

t_max_err hedrot_receiver_axesReference_set(t_hedrot_receiver *x, t_object *attr, long argc, t_atom *argv) {
    if (argc && argv) {
        x->axesReference = (char) max(min(atom_getlong(argv),2),0);
        setAxesReference(x->trackingData, x->axesReference);
        if(x->verbose) post("[hedrot_receiver]: axesReference set to %ld", x->axesReference);
    }
    return MAX_ERR_NONE;
}

t_max_err hedrot_receiver_rotationOrder_set(t_hedrot_receiver *x, t_object *attr, long argc, t_atom *argv) {
    if (argc && argv) {
        x->rotationOrder = (char) max(min(atom_getlong(argv),1),0);
        setRotationOrder(x->trackingData, x->rotationOrder);
        if(x->verbose) post("[hedrot_receiver]: rotationOrder set to %ld", x->rotationOrder);
    }
    return MAX_ERR_NONE;
}


t_max_err hedrot_receiver_invertRotation_set(t_hedrot_receiver *x, t_object *attr, long argc, t_atom *argv) {
    if (argc && argv) {
        x->invertRotation = (char) max(min(atom_getlong(argv),1),0);
        setInvertRotation(x->trackingData, x->invertRotation);
        if(x->verbose) post("[hedrot_receiver]: invertRotation set to %ld", x->invertRotation);
    }
    return MAX_ERR_NONE;
}


t_max_err hedrot_receiver_accCalMaxGyroNorm_set(t_hedrot_receiver *x, t_object *attr, long argc, t_atom *argv) {
    if (argc && argv) {
        x->accCalMaxGyroNorm = (float) max(atom_getfloat(argv),0);
        setAccCalMaxGyroNorm(x->trackingData, x->accCalMaxGyroNorm);
    }
    return MAX_ERR_NONE;
}


t_max_err hedrot_receiver_offlineCalibrationMethod_set(t_hedrot_receiver *x, t_object *attr, long argc, t_atom *argv) {
    if (argc && argv) {
        x->offlineCalibrationMethod = (char) max(min(atom_getlong(argv),1),0);
        setOfflineCalibrationMethod(x->trackingData, x->offlineCalibrationMethod);
    }
    return MAX_ERR_NONE;
}


t_max_err hedrot_receiver_RTmagCalibrationMethod_set(t_hedrot_receiver *x, t_object *attr, long argc, t_atom *argv) {
    if (argc && argv) {
        x->RTmagCalibrationMethod = (char) max(min(atom_getlong(argv),1),0);
        setRTmagCalibrationMethod(x->trackingData, x->RTmagCalibrationMethod);
    }
    return MAX_ERR_NONE;
}


t_max_err hedrot_receiver_RTmagCalOn_set(t_hedrot_receiver *x, t_object *attr, long argc, t_atom *argv) {
    if (argc && argv) {
        x->RTmagCalOn = (char) max(min(atom_getlong(argv),1),0);
        setRTmagCalOn(x->trackingData, x->RTmagCalOn);
    }
    return MAX_ERR_NONE;
}


t_max_err hedrot_receiver_RTmagMaxMemoryDuration_set(t_hedrot_receiver *x, t_object *attr, long argc, t_atom *argv){
    if (argc && argv) {
        x->RTmagMaxMemoryDuration = (float) max(atom_getfloat(argv),0);
        setRTmagMaxMemoryDuration(x->trackingData, x->RTmagMaxMemoryDuration);
    }
    return MAX_ERR_NONE;
}


t_max_err hedrot_receiver_RTmagMaxDistanceError_set(t_hedrot_receiver *x, t_object *attr, long argc, t_atom *argv){
    if (argc && argv) {
        x->RTmagMaxDistanceError = (float) max(atom_getfloat(argv),0);
        setRTmagMaxDistanceError(x->trackingData, x->RTmagMaxDistanceError);
    }
    return MAX_ERR_NONE;
}


t_max_err hedrot_receiver_RTMagCalibrationPeriod_set(t_hedrot_receiver *x, t_object *attr, long argc, t_atom *argv){
    if (argc && argv) {
        x->RTMagCalibrationPeriod = (float) max(atom_getfloat(argv),0);
        setRTMagCalibrationPeriod(x->trackingData, x->RTMagCalibrationPeriod);
    }
    return MAX_ERR_NONE;
}



/* ---------------- SETUP OBJECT ------------------ */
int C74_EXPORT main()
{
    
    t_class *c;
    
    common_symbols_init();
    
    c = class_new("hedrot_receiver", (method)hedrot_receiver_new,
                  (method)hedrot_receiver_free, sizeof(t_hedrot_receiver),
                  0L, A_GIMME, 0);
    
    // --------------------------------------- define methods -----------------------------------------------------------
    class_addmethod(c, (method)hedrot_receiver_open,            "open",			A_LONG, 0);
    class_addmethod(c, (method)hedrot_receiver_close,           "close",		0);
    class_addmethod(c, (method)hedrot_receiver_devices,         "devices",      0);
    class_addmethod(c, (method)hedrot_receiver_assist,			"assist",		A_CANT, 0);
    class_addmethod(c, (method)hedrot_receiver_center_angles,   "center",      0);
    
    // methods for importing/exporting headtracker settings
    class_addmethod(c, (method)hedrot_receiver_export_settings, "export_settings", A_DEFSYM, 0);
    class_addmethod(c, (method)hedrot_receiver_import_settings, "import_settings", A_DEFSYM, 0);
    class_addmethod(c, (method)hedrot_receiver_printVersion, "version", A_DEFSYM, 0);
    
    // methods for recording data into a text file
    class_addmethod(c, (method)hedrot_receiver_opendestinationtextfile,	"opendestinationtextfile",		A_DEFSYM, 0);
    class_addmethod(c, (method)hedrot_receiver_startrec,         "startrec", 0);
    class_addmethod(c, (method)hedrot_receiver_stoprec,          "stoprec", 0);
    
    // methods for mag calibration
    class_addmethod(c, (method)hedrot_receiver_startMagCalibration,  "startMagCalibration", 0);
    class_addmethod(c, (method)hedrot_receiver_stopMagCalibration,   "stopMagCalibration", 0);
    class_addmethod(c, (method)hedrot_receiver_redoMagCalibration,   "redoMagCalibration", 0);
    class_addmethod(c, (method)hedrot_receiver_saveMagCalibration,   "saveMagCalibration", 0);
    class_addmethod(c, (method)hedrot_receiver_dumpMagCalInfo,       "dumpMagCalData", 0);
    class_addmethod(c, (method)hedrot_receiver_exportMagRawCalData,  "exportMagRawCalData", A_DEFSYM, 0);
    
    // methods for acc calibration
    class_addmethod(c, (method)hedrot_receiver_startAccCalibration,  "startAccCalibration", 0);
    class_addmethod(c, (method)hedrot_receiver_stopAccCalibration,   "stopAccCalibration", 0);
    class_addmethod(c, (method)hedrot_receiver_redoAccCalibration,   "redoAccCalibration", 0);
    class_addmethod(c, (method)hedrot_receiver_saveAccCalibration,   "saveAccCalibration", 0);
    class_addmethod(c, (method)hedrot_receiver_dumpAccCalInfo,       "dumpAccCalData", 0);
    class_addmethod(c, (method)hedrot_receiver_exportAccRawCalData,  "exportAccRawCalData", A_DEFSYM, 0);
    
    
    // methods for real-time mag calibration
    class_addmethod(c, (method)hedrot_receiver_exportRTmagRawCalData,  "exportRTmagRawCalData", A_DEFSYM, 0);
    
    
    
    // --------------------------------------- define attributes --------------------------------------------------------
    CLASS_ATTR_CHAR(c,    "verbose",    0,  t_hedrot_receiver,  verbose);
    CLASS_ATTR_ENUMINDEX(c, "verbose", 0, "\"no messages\" \"some messages\" \"all messages\"");
    CLASS_ATTR_STYLE(c, "verbose", 0, "enumindex");
    CLASS_ATTR_ACCESSORS(c, "verbose", NULL, hedrot_receiver_verbose_set);
    CLASS_ATTR_SAVE(c,    "verbose",   0);
    
    CLASS_ATTR_CHAR(c,    "headtracker_on",    0,  t_hedrot_receiver, headtracker_on);
    CLASS_ATTR_STYLE_LABEL(c, "headtracker_on", 0, "onoff", "headtracker_on");
    CLASS_ATTR_ACCESSORS(c, "headtracker_on", NULL, hedrot_receiver_headtracker_on_set);
    
    CLASS_ATTR_CHAR(c,    "autoDiscover",    0,  t_hedrot_receiver, autoDiscover);
    CLASS_ATTR_STYLE_LABEL(c, "autoDiscover", 0, "onoff", "autoDiscover");
    CLASS_ATTR_ACCESSORS(c, "autoDiscover", NULL, hedrot_receiver_autoDiscover_set);
    CLASS_ATTR_SAVE(c,    "autoDiscover",   0);
    
    //global settings
    CLASS_ATTR_LONG(c,    "samplerate",    0,  t_hedrot_receiver,  samplerate);
    CLASS_ATTR_ACCESSORS(c, "samplerate", NULL, hedrot_receiver_samplerate_set);
    
    //gyroscope settings
    CLASS_ATTR_CHAR(c,    "gyroDataRate",    0,  t_hedrot_receiver,  gyroDataRate);
    CLASS_ATTR_ACCESSORS(c, "gyroDataRate", NULL, hedrot_receiver_gyroDataRate_set);
    
    CLASS_ATTR_CHAR(c,    "gyroClockSource",    0,  t_hedrot_receiver,  gyroClockSource);
    CLASS_ATTR_ACCESSORS(c, "gyroClockSource", NULL, hedrot_receiver_gyroClockSource_set);
    
    CLASS_ATTR_CHAR(c,    "gyroDLPFBandwidth",    0,  t_hedrot_receiver,  gyroDLPFBandwidth);
    CLASS_ATTR_ACCESSORS(c, "gyroDLPFBandwidth", NULL, hedrot_receiver_gyroDLPFBandwidth_set);
    
    //accelerometer settings
    CLASS_ATTR_CHAR(c,"accRange", 0, t_hedrot_receiver, accRange);
    CLASS_ATTR_ENUMINDEX(c, "accRange", 0, "2G 4G 8G 16G");
    CLASS_ATTR_ACCESSORS(c, "accRange", NULL, hedrot_receiver_accRange_set);
    
    CLASS_ATTR_LONG_ARRAY(c, "accHardOffset",    0,  t_hedrot_receiver,  accHardOffset,  3);
    CLASS_ATTR_ACCESSORS(c, "accHardOffset", NULL, hedrot_receiver_accHardOffset_set);
    
    CLASS_ATTR_CHAR(c,    "accFullResolutionBit",    0,  t_hedrot_receiver,  accFullResolutionBit);
    CLASS_ATTR_STYLE_LABEL(c, "accFullResolutionBit", 0, "onoff", "accFullResolutionBit");
    CLASS_ATTR_ACCESSORS(c, "accFullResolutionBit", NULL, hedrot_receiver_accFullResolutionBit_set);
    
    CLASS_ATTR_CHAR(c, "accDataRate", 0, t_hedrot_receiver, accDataRate);
    CLASS_ATTR_ENUMINDEX(c, "accDataRate", 0, "\".1 Hz\" \".2 Hz\" \".39 Hz\" \".78 Hz\" \"1.56 Hz\" \"3.13 Hz\" \"6.25 Hz\" \"12.5 Hz\" \"25 Hz\" \"50 Hz\" \"100 Hz\" \"200 Hz\" \"400 Hz\" \"800 Hz\" \"1600 Hz\" \"3200 Hz\"");
    CLASS_ATTR_STYLE(c, "accDataRate", 0, "enumindex");
    CLASS_ATTR_ACCESSORS(c, "accDataRate", NULL, hedrot_receiver_accDataRate_set);
    
    //magnetometer settings
    CLASS_ATTR_CHAR(c,    "magMeasurementBias",    0,  t_hedrot_receiver,  magMeasurementBias);
    CLASS_ATTR_ACCESSORS(c, "magMeasurementBias", NULL, hedrot_receiver_magMeasurementBias_set);
    
    CLASS_ATTR_CHAR(c,    "magSampleAveraging",    0,  t_hedrot_receiver,  magSampleAveraging);
    CLASS_ATTR_ACCESSORS(c, "magSampleAveraging", NULL, hedrot_receiver_magSampleAveraging_set);
    
    CLASS_ATTR_CHAR(c,    "magDataRate",    0,  t_hedrot_receiver,  magDataRate);
    CLASS_ATTR_ENUMINDEX(c, "magDataRate", 0, "\"0.75 Hz\" \"1.5 Hz\" \"3 Hz\" \"7.5 Hz\" \"15 Hz\" \"30 Hz\" \"75 Hz\"");
    CLASS_ATTR_STYLE(c, "magDataRate", 0, "enumindex");
    CLASS_ATTR_ACCESSORS(c, "magDataRate", NULL, hedrot_receiver_magDataRate_set);
    
    CLASS_ATTR_CHAR(c,    "magGain",    0,  t_hedrot_receiver,  magGain);
    CLASS_ATTR_ACCESSORS(c, "magGain", NULL, hedrot_receiver_magGain_set);
    
    CLASS_ATTR_CHAR(c,    "magMeasurementMode",    0,  t_hedrot_receiver,  magMeasurementMode);
    CLASS_ATTR_ACCESSORS(c, "magMeasurementMode", NULL, hedrot_receiver_magMeasurementMode_set);
    
    
    
    //calibration settings
    CLASS_ATTR_CHAR(c,    "gyroOffsetAutocalOn",    0,  t_hedrot_receiver,  gyroOffsetAutocalOn);
    CLASS_ATTR_STYLE_LABEL(c, "gyroOffsetAutocalOn", 0, "onoff", "gyroOffsetAutocalOn");
    CLASS_ATTR_ACCESSORS(c, "gyroOffsetAutocalOn", NULL, hedrot_receiver_gyroOffsetAutocalOn_set);
    CLASS_ATTR_SAVE(c,    "gyroOffsetAutocalOn",   0);
    
    CLASS_ATTR_FLOAT(c,    "gyroOffsetAutocalTime",    0,  t_hedrot_receiver,  gyroOffsetAutocalTime);
    CLASS_ATTR_ACCESSORS(c, "gyroOffsetAutocalTime", NULL, hedrot_receiver_gyroOffsetAutocalTime_set);
    CLASS_ATTR_SAVE(c,    "gyroOffsetAutocalTime",   0);
    
    CLASS_ATTR_LONG(c,    "gyroOffsetAutocalThreshold",    0,  t_hedrot_receiver,  gyroOffsetAutocalThreshold);
    CLASS_ATTR_ACCESSORS(c, "gyroOffsetAutocalThreshold", NULL, hedrot_receiver_gyroOffsetAutocalThreshold_set);
    CLASS_ATTR_SAVE(c,    "gyroOffsetAutocalThreshold",   0);
    
    CLASS_ATTR_FLOAT_ARRAY(c,    "accOffset",    0,  t_hedrot_receiver,  accOffset,  3);
    CLASS_ATTR_ACCESSORS(c, "accOffset", NULL, hedrot_receiver_accOffset_set);
    
    CLASS_ATTR_FLOAT_ARRAY(c,    "accScaling",    0,  t_hedrot_receiver,  accScaling,  3);
    CLASS_ATTR_ACCESSORS(c, "accScaling", NULL, hedrot_receiver_accScaling_set);
    
    CLASS_ATTR_FLOAT_ARRAY(c,    "magOffset",    0,  t_hedrot_receiver,  magOffset,  3);
    CLASS_ATTR_ACCESSORS(c, "magOffset", NULL, hedrot_receiver_magOffset_set);
    
    CLASS_ATTR_FLOAT_ARRAY(c,    "magScaling",    0,  t_hedrot_receiver,  magScaling,  3);
    CLASS_ATTR_ACCESSORS(c, "magScaling", NULL, hedrot_receiver_magScaling_set);
    
    CLASS_ATTR_CHAR(c,    "offlineCalibrationMethod",    0,  t_hedrot_receiver,  offlineCalibrationMethod);
    CLASS_ATTR_ENUMINDEX(c, "offlineCalibrationMethod", 0, "\"double ellipsoid fit\" \"Aussal\"");
    CLASS_ATTR_ACCESSORS(c, "offlineCalibrationMethod", NULL, hedrot_receiver_offlineCalibrationMethod_set);
    CLASS_ATTR_SAVE(c,    "offlineCalibrationMethod",   0);
    
    
    // real-time magnetometer calibration settings
    CLASS_ATTR_CHAR(c,    "RTmagCalibrationMethod",    0,  t_hedrot_receiver,  RTmagCalibrationMethod);
    CLASS_ATTR_ENUMINDEX(c, "RTmagCalibrationMethod", 0, "\"direct\" \"iterative\"");
    CLASS_ATTR_ACCESSORS(c, "RTmagCalibrationMethod", NULL, hedrot_receiver_RTmagCalibrationMethod_set);
    CLASS_ATTR_SAVE(c,    "RTmagCalibrationMethod",   0);
    
    
    CLASS_ATTR_CHAR(c,    "RTmagCalOn",    0,  t_hedrot_receiver,  RTmagCalOn);
    CLASS_ATTR_STYLE_LABEL(c, "RTmagCalOn", 0, "onoff", "RTmagCalOn");
    CLASS_ATTR_ACCESSORS(c, "RTmagCalOn", NULL, hedrot_receiver_RTmagCalOn_set);
    CLASS_ATTR_SAVE(c,    "RTmagCalOn",   0);
    
    CLASS_ATTR_FLOAT(c,    "RTmagMaxMemoryDuration",    0,  t_hedrot_receiver,  RTmagMaxMemoryDuration);
    CLASS_ATTR_ACCESSORS(c, "RTmagMaxMemoryDuration", NULL, hedrot_receiver_RTmagMaxMemoryDuration_set);
    CLASS_ATTR_SAVE(c,    "RTmagMaxMemoryDuration",   0);
    
    CLASS_ATTR_FLOAT(c,    "RTmagMaxDistanceError",    0,  t_hedrot_receiver,  RTmagMaxDistanceError);
    CLASS_ATTR_ACCESSORS(c, "RTmagMaxDistanceError", NULL, hedrot_receiver_RTmagMaxDistanceError_set);
    CLASS_ATTR_SAVE(c,    "RTmagMaxDistanceError",   0);
    
    CLASS_ATTR_FLOAT(c,    "RTMagCalibrationPeriod",    0,  t_hedrot_receiver,  RTMagCalibrationPeriod);
    CLASS_ATTR_ACCESSORS(c, "RTMagCalibrationPeriod", NULL, hedrot_receiver_RTMagCalibrationPeriod_set);
    CLASS_ATTR_SAVE(c,    "RTMagCalibrationPeriod",   0);
    
    
    // angle estimation
    CLASS_ATTR_FLOAT(c,    "MadgwickBetaGain",    0,  t_hedrot_receiver,  MadgwickBetaGain);
    CLASS_ATTR_ACCESSORS(c, "MadgwickBetaGain", NULL, hedrot_receiver_MadgwickBetaGain_set);
    CLASS_ATTR_SAVE(c,    "MadgwickBetaGain",   0);
    
    CLASS_ATTR_FLOAT(c,    "MadgwickBetaMax",    0,  t_hedrot_receiver,  MadgwickBetaMax);
    CLASS_ATTR_ACCESSORS(c, "MadgwickBetaMax", NULL, hedrot_receiver_MadgwickBetaMax_set);
    CLASS_ATTR_SAVE(c,    "MadgwickBetaMax",   0);
    
    CLASS_ATTR_FLOAT(c,    "accLPtimeConstant",    0,  t_hedrot_receiver,  accLPtimeConstant);
    CLASS_ATTR_ACCESSORS(c, "accLPtimeConstant", NULL, hedrot_receiver_accLPtimeConstant_set);
    CLASS_ATTR_SAVE(c,    "accLPtimeConstant",   0);
    
    CLASS_ATTR_CHAR(c,    "axesReference",    0,  t_hedrot_receiver,  axesReference);
    CLASS_ATTR_ENUMINDEX(c, "axesReference", 0, "\"X->right, Y->back, Z->down\" \"X->right, Y->front, Z->up\" \"X->front, Y->left, Z->up\"");
    CLASS_ATTR_ACCESSORS(c, "axesReference", NULL, hedrot_receiver_axesReference_set);
    CLASS_ATTR_SAVE(c,    "axesReference",   0);
    
    CLASS_ATTR_CHAR(c,    "rotationOrder",    0,  t_hedrot_receiver,  rotationOrder);
    CLASS_ATTR_ENUMINDEX(c, "rotationOrder", 0, "\"Yaw-Pitch-Roll (ZYX)\" \"Roll-Pitch-Yaw (XYZ)\"");
    CLASS_ATTR_ACCESSORS(c, "rotationOrder", NULL, hedrot_receiver_rotationOrder_set);
    CLASS_ATTR_SAVE(c,    "rotationOrder",   0);
    
    CLASS_ATTR_CHAR(c,    "invertRotation",    0,  t_hedrot_receiver,  invertRotation);
    CLASS_ATTR_ENUMINDEX(c, "invertRotation", 0, "\"Don't invert rotation\" \"Invert rotation\"");
    CLASS_ATTR_ACCESSORS(c, "invertRotation", NULL, hedrot_receiver_invertRotation_set);
    CLASS_ATTR_SAVE(c,    "invertRotation",   0);
    
    CLASS_ATTR_FLOAT(c,    "accCalMaxGyroNorm",    0,  t_hedrot_receiver,  accCalMaxGyroNorm);
    CLASS_ATTR_ACCESSORS(c, "accCalMaxGyroNorm", NULL, hedrot_receiver_accCalMaxGyroNorm_set);
    CLASS_ATTR_SAVE(c,    "accCalMaxGyroNorm",   0);
    
    // output settings
    CLASS_ATTR_LONG(c,    "outputDataPeriod",    0,  t_hedrot_receiver,  outputDataPeriod);
    CLASS_ATTR_ACCESSORS(c, "outputDataPeriod", NULL, hedrot_receiver_outputDataPeriod_set);
    CLASS_ATTR_SAVE(c,    "outputDataPeriod",   0);
    
    
    // --------------------------------------- register class --------------------------------------------------------
    class_register(CLASS_BOX, c);
    
    hedrot_receiver_class = c;
    
    return 0;
}