// hedrot_comm_protocol.h: all constants related to communication protocol betwenn headtracker and receiver
//
// Copyright 2016 Alexis Baskind



// "R2H_" constants correspond to messages sent from receiver to headtracker
// "H2R_" constants correspond to messages sent from headtracker to receiver


#ifndef _HEADTRACKER_COMM_PROTOCOL_H_
#define _HEADTRACKER_COMM_PROTOCOL_H_

#define HEDROT_FIRMWARE_VERSION                        10

//serial communication settings
#define BAUDRATE                                       230400

#define NUMBER_OF_BYTES_IN_RAWDATA_FRAME               21


// reserved bytes (corresponding to ASCII codes, and cannot be used for codes):
// . 44 (ASCII code corresponding to ',')
// . 45 (ASCII code corresponding to '-')
// . 46 (ASCII code corresponding to '.')
// . from 48 to 57 (ASCII codes corresponding to digits 0-9)

// standard ASCII codes for transmission of decimal values
#define ASCII_0                                        48
#define ASCII_9                                        57
#define CARRIAGE_RETURN                                13
#define SPACE                                          32
#define COMMA                                          44
#define MINUS                                          45
#define DOT                                            46


// data sent from receiver to headtracker
#define R2H_STOP_TRANSMISSION_CHAR                     0 

#define R2H_SEND_INFO_CHAR                             1

#define R2H_START_TRANSMIT_ACCEL_OFFSET_DATA_CHAR      2 
#define R2H_STOP_TRANSMIT_ACCEL_OFFSET_DATA_CHAR       3

#define R2H_START_TRANSMIT_ACCEL_SCALING_DATA_CHAR     4 
#define R2H_STOP_TRANSMIT_ACCEL_SCALING_DATA_CHAR      5

#define R2H_START_TRANSMIT_MAG_OFFSET_DATA_CHAR        6 
#define R2H_STOP_TRANSMIT_MAG_OFFSET_DATA_CHAR         7

#define R2H_START_TRANSMIT_MAG_SCALING_DATA_CHAR       8 
#define R2H_STOP_TRANSMIT_MAG_SCALING_DATA_CHAR        9

#define R2H_TRANSMIT_SAMPLERATE                        11

#define R2H_TRANSMIT_GYRO_RATE                         21
#define R2H_TRANSMIT_GYRO_CLOCK_SOURCE                 22
#define R2H_TRANSMIT_GYRO_LPF_BANDWIDTH                23

#define R2H_TRANSMIT_ACCEL_RANGE                       31
#define R2H_START_TRANSMIT_ACCEL_HARD_OFFSET           32
#define R2H_STOP_TRANSMIT_ACCEL_HARD_OFFSET            33
#define R2H_TRANSMIT_ACCEL_FULL_RESOLUTION_BIT         34
#define R2H_TRANSMIT_ACCEL_DATARATE                    35

#define R2H_TRANSMIT_MAG_MEASUREMENT_BIAS              51
#define R2H_TRANSMIT_MAG_SAMPLE_AVERAGING              52
#define R2H_TRANSMIT_MAG_DATA_RATE                     53
#define R2H_TRANSMIT_MAG_GAIN                          54
#define R2H_TRANSMIT_MAG_MEASUREMENT_MODE              55

#define R2H_AREYOUTHERE_CHAR                           126
#define R2H_PING_CHAR                                  127

// data sent from headtracker to receiver
// must be between 0 and 127 so that the MSB is always 0
// (MSB = 1 is reserved for transmitting headtracking data)
#define H2R_END_OF_RAWDATA_FRAME                       0
#define H2R_BOARD_OVERLOAD                             1 // the teensy is too slow (the samplerate is too high)


#define H2R_START_TRANSMIT_INFO_CHAR                   11
#define H2R_STOP_TRANSMIT_INFO_CHAR                    12

#define H2R_DATA_RECEIVE_ERROR_CHAR                    21

#define H2R_IAMTHERE_CHAR                              126
#define H2R_PING_CHAR                                  127



#endif /* _HEADTRACKER_COMM_PROTOCOL_H_ */
