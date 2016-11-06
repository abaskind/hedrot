//
//  libheadtracker_serialcomm.h
//
// Part of code is derived from "comport", (c) 1998-2005  Winfried Ritsch, Institute for Electronic Music - Graz
//
// Copyright 2016 Alexis Baskind

#ifndef __libheadtracker_serialcomm__
#define __libheadtracker_serialcomm__

#include <stdio.h>

// for serial communication
#include <sys/time.h>
#include <fcntl.h>
#include <sys/ioctl.h> /* for ioctl DTR */
#include <termios.h> /* for TERMIO ioctl calls */
#include <unistd.h>
#include <glob.h>
#include <errno.h>

// other includes
#include "hedrot_comm_protocol.h"

// internal constants
#define HANDLE int
#define INVALID_HANDLE_VALUE -1
#define MAX_NUMBER_OF_PORTS 100

// serial constants and tables
#define OPENPARAMS (O_RDWR|O_NDELAY|O_NOCTTY)
#define BAUDRATE_230400 B230400
#define BAUDRATE_115200 B115200
#define BAUDRATE_57600  B57600
#define BAUDRATE_38400  B38400


//=====================================================================================================
// structure definition: headtrackerSerialcomm (all infos for serial communication with the headtracker)
//=====================================================================================================

typedef struct _headtrackerSerialcomm {
    fd_set          com_rfds;

    char**          availablePorts;
    int             numberOfAvailablePorts;
    
    // information about the opened port
    char            *serial_device_name;
    struct termios	com_termio; /* save the com config */
    int             portNumber; // port number in the list "availablePorts"
    int				comhandle; /* holds the headtracker_rcv handle */
    
    int				baud; /* holds the current baud rate */
    
    char            verbose;
} headtrackerSerialcomm;

struct timeval tv;

//=====================================================================================================
// function declarations
//=====================================================================================================

void serial_comm_init(headtrackerSerialcomm *x);
void list_comm_ports(headtrackerSerialcomm *x);
void init_read_serial(headtrackerSerialcomm *x);
int is_data_available(headtrackerSerialcomm *x);
int write_serial(headtrackerSerialcomm *x, unsigned char serial_byte);
int open_serial(headtrackerSerialcomm *x,  char* portName);
int close_serial(headtrackerSerialcomm *x);
long get_baud_ratebits(float *baud);

#endif /* defined(__libheadtracker_serialcomm__) */
