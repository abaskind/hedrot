//
//  libhedrot_serialcomm.h
//
// Part of code is derived from "comport", (c) 1998-2005  Winfried Ritsch, Institute for Electronic Music - Graz
//
// Copyright 2016 Alexis Baskind

#ifndef __libheadtracker_serialcomm__
#define __libheadtracker_serialcomm__

#include <stdio.h>

// for serial communication
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include <commctrl.h>
#else
#include <sys/time.h>
#include <fcntl.h>
#include <sys/ioctl.h> /* for ioctl DTR */
#include <termios.h> /* for TERMIO ioctl calls */
#include <unistd.h>
#include <glob.h>
#include <errno.h>
#define INVALID_HANDLE_VALUE -1
#endif /* #if defined(_WIN32) || defined(_WIN64) */

// other includes
#include "hedrot_comm_protocol.h"

// internal constants
#define MAX_NUMBER_OF_PORTS 99
#define READ_BUFFER_SIZE 10000

//=====================================================================================================
// structure definition: headtrackerSerialcomm (all infos for serial communication with the headtracker)
//=====================================================================================================

typedef struct _headtrackerSerialcomm {
    char**          availablePorts;
    int				numberOfAvailablePorts;

	unsigned long	numberOfReadBytes;
	unsigned char	readBuffer[READ_BUFFER_SIZE];
    
    // information about the opened port
    char            *serial_device_name;
    int             portNumber; // port number in the list "availablePorts"
#if defined(_WIN32) || defined(_WIN64)
    HANDLE			comhandle; /* holds the comport handle */
    DCB				dcb; /* holds the comm pars */
    DCB				dcb_old; /* holds the comm pars */
    COMMTIMEOUTS	old_timeouts;
#else /* #if defined(_WIN32) || defined(_WIN64) */
    struct termios	com_termio; /* save the com config */
    int				comhandle; /* holds the headtracker_rcv handle */
	fd_set          com_rfds;
#endif /* #if defined(_WIN32) || defined(_WIN64) */
    
    int				baud; /* holds the current baud rate */
    
    char            verbose;
} headtrackerSerialcomm;

//=====================================================================================================
// function declarations
//=====================================================================================================

void serial_comm_init(headtrackerSerialcomm *x);
void list_comm_ports(headtrackerSerialcomm *x);
void init_read_serial(headtrackerSerialcomm *x);
int is_data_available(headtrackerSerialcomm *x);
int write_serial(headtrackerSerialcomm *x, unsigned char *serial_byte, unsigned long numberOfBytesToWrite);
#if defined(_WIN32) || defined(_WIN64)
HANDLE open_serial(headtrackerSerialcomm *x,  char* portName);
HANDLE close_serial(headtrackerSerialcomm *x);
#else /* #if defined(_WIN32) || defined(_WIN64) */
int open_serial(headtrackerSerialcomm *x,  char* portName);
int close_serial(headtrackerSerialcomm *x);
#endif


#endif /* defined(__libheadtracker_serialcomm__) */
