//
// libheadtracker_serialcomm.c
// root functions for serial communication with the headtracker
//
// Part of code is derived from "comport", (c) 1998-2005  Winfried Ritsch, Institute for Electronic Music - Graz
//
// Copyright 2016 Alexis Baskind


#include "libhedrot_serialcomm.h"
#include "stdlib.h"
#include "string.h"

// global tables
long baudspeedbittable[] =
{
    BAUDRATE_230400,
    BAUDRATE_115200,  /* CPU SPECIFIC */
    BAUDRATE_57600,   /* CPU SPECIFIC */
    BAUDRATE_38400,   /* CPU SPECIFIC */
    B19200,
    B9600,
    B4800,
    B2400,
    B1800,
    B1200,
    B600,
    B300,
    B200,
    B150,
    B134,
    B110,
    B75,
    B50,
    B0
};

#define BAUDRATETABLE_LEN 19

long baudratetable[] =
{
    230400L,
    115200L,
    57600L,
    38400L,
    19200L,
    9600L,
    4800L,
    2400L,
    1800L,
    1200L,
    600L,
    300L,
    200L,
    150L,
    134L,
    110L,
    75L,
    50L,
    0L
    
}; /* holds the baud rate selections */



/* ----------------- Serial methods ------------------------------ */

// initialization routine
void serial_comm_init(headtrackerSerialcomm *x) {
    // init the parameters for serial communication
    
    x->serial_device_name = NULL;
    
    x->comhandle = INVALID_HANDLE_VALUE; //still no com port open
    
    x->baud = BAUDRATE; //default
    
    x->numberOfAvailablePorts = 0;
    x->availablePorts = NULL;
    
    x->portNumber = -1;
}


// list all available comm ports
void list_comm_ports(headtrackerSerialcomm *x) {
    
    unsigned int   i;
    int            fd;
    struct termios test;
    
    glob_t         glob_buffer;
    
    char*          tmpPortsNames[MAX_NUMBER_OF_PORTS];
    
    // reset port number (for autodiscovering)
    x->portNumber=-1;
    
    /* first look for registered devices in the filesystem */
#ifdef __APPLE__
    const char *glob_pattern = "/dev/cu.*";
#endif
    
    switch( glob( glob_pattern, GLOB_ERR, NULL, &glob_buffer ) ) {
        case 0:
            printf("[hedrot]: %ld possible ports found\r\n",glob_buffer.gl_pathc);
            break;
        case GLOB_NOSPACE:
            printf("[hedrot] out of memory for \"%s\"\r\n",glob_pattern);
            break;
# ifdef GLOB_ABORTED
        case GLOB_ABORTED:
            printf("[hedrot] aborted \"%s\"\r\n",glob_pattern);
            break;
# endif /* GLOB_ABORTED */
# ifdef GLOB_NOMATCH
        case GLOB_NOMATCH:
            printf("[hedrot] no serial devices found for \"%s\"\r\n",glob_pattern);
            break;
# endif /* GLOB_NOMATCH */
    }
    
    // free the previous port list
    for (i = 0; i < x->numberOfAvailablePorts; i++) {
        free(x->availablePorts[i]);
    }
    if(x->availablePorts) {
        free(x->availablePorts);
        x->availablePorts = NULL;
    }
    x->numberOfAvailablePorts = 0;
    
    // now check which ports are really available and has attributes, and update the port list accordingly
    for(i=0; i<glob_buffer.gl_pathc; i++) {
        // now try to open the device
        if((fd = open(glob_buffer.gl_pathv[i], OPENPARAMS)) != INVALID_HANDLE_VALUE) {
            // now see if it has attributes. Only if yes, add it to the list
            if ((tcgetattr(fd, &test)) != -1) {
                tmpPortsNames[x->numberOfAvailablePorts] = strdup(glob_buffer.gl_pathv[i]);
                x->numberOfAvailablePorts++;
                
                if(x->verbose) printf("[hedrot]: port %u (%s) available\r\n",i, glob_buffer.gl_pathv[i]);
                
            }
            close (fd);
        }
    }
    
    // update the list of available ports
    x->availablePorts = malloc(x->numberOfAvailablePorts*sizeof(char*));
    for(i=0; i<x->numberOfAvailablePorts; i++)
        x->availablePorts[i] = strdup(tmpPortsNames[i]);
    
    globfree( &(glob_buffer) );
}


// open serial port by its name
// returns INVALID_HANDLE_VALUE if fails
// returns handle value if it succeeds

int open_serial(headtrackerSerialcomm *x, char* portName) {
    int             fd;
    struct termios  *tios = &(x->com_termio);
    int             *baud = &(x->baud);
    
    /* the communication is through USB, so:
     * The number of bits is always 8
     * no stop bits
     * no hardware/software handshake
     * parity bits are not used */
    
    if((fd = open(portName, OPENPARAMS)) == INVALID_HANDLE_VALUE) {
        printf("[hedrot] ** ERROR ** could not open device %s: failure(%d): %s\r\n",
              portName,errno, strerror(errno));
        return INVALID_HANDLE_VALUE;
    }
    
    /* set no wait on any operation */
    fcntl(fd, F_SETFL, FNDELAY);
    
    /*   Save the Current Port Configuration  */
    if(tcgetattr(fd, tios) < 0) {
        printf("[hedrot] ** ERROR ** could not get termios-structure of device %s\r\n", portName);
        close(fd);
        return INVALID_HANDLE_VALUE;
    }

    if (cfsetspeed(tios, *baud) < 0) {
        printf("error in cfsetspeed\n\n");
        return INVALID_HANDLE_VALUE;
    }
    
    if (tcsetattr(fd, TCSANOW, tios) < 0) {
        printf("unable to set baud rate\n");
        return INVALID_HANDLE_VALUE;
    }
    
    /* Setup the new port configuration...NON-CANONICAL INPUT MODE
     .. as defined in termios.h */
    
    /* enable input and ignore modem controls */
    tios->c_cflag |= (CREAD | CLOCAL);
    
    /* always nocanonical, this means raw i/o no terminal */
    tios->c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    
    /* no post processing */
    tios->c_oflag &= ~OPOST;
    
    //set_baudrate(x, *baud);
    
    if(tcsetattr(fd, TCSAFLUSH, tios) != -1) {
        if(x->verbose) printf("[hedrot] opened serial line device %s\r\n", portName);
        
        // update structure info
        x->serial_device_name = strdup(portName);
        x->comhandle = fd;
        
        return fd;
    } else {
        printf("[hedrot] ** ERROR ** could not set attributes to ioctl of device %s\r\n", portName);
        close(fd);
        return INVALID_HANDLE_VALUE;
    }
}

// close serial port
// returns INVALID_HANDLE_VALUE in any case

int close_serial(headtrackerSerialcomm *x)
{
    struct termios *tios = &(x->com_termio);
    
    if(x->comhandle != INVALID_HANDLE_VALUE) {
        tcsetattr(x->comhandle, TCSANOW, tios);
        close(x->comhandle);
        if(x->verbose) printf("[hedrot] closed %s\r\n",x->serial_device_name);
        x->serial_device_name = NULL;
        x->comhandle = INVALID_HANDLE_VALUE;
    }
    return INVALID_HANDLE_VALUE;
}

// clear file descriptors before reading data on opened comm port
void init_read_serial(headtrackerSerialcomm *x) {
    FD_ZERO(&x->com_rfds);
    FD_SET(x->comhandle,&x->com_rfds);
    tv.tv_sec = 0;
    tv.tv_usec = 0;
}


// check with select() if a data is available for reading on opened comm port
inline int is_data_available(headtrackerSerialcomm *x) {
    return select(x->comhandle+1,&x->com_rfds,NULL,NULL,&tv);
}



// write byte on serial port
// return 0 if fails
// return 1 if it succeeds

int write_serial(headtrackerSerialcomm *x, unsigned char serial_byte)
{
    if(x->verbose >= 2) printf("write_serial on comhandle %i: %x\r\n", x->comhandle, serial_byte);
    int result = (int) write(x->comhandle,(char *) &serial_byte,1);
    if (result != 1) {
        printf ("[hedrot] write on comhandle %i returned %d, errno is %d\r\n", x->comhandle, result, errno);
    } 
    return result;
}


long get_baud_ratebits(float *baud)
{
    int i = 0;
    
    while(i < BAUDRATETABLE_LEN && baudratetable[i] > *baud) i++;
    
    /* nearest Baudrate finding */
    if(i==BAUDRATETABLE_LEN ||  baudspeedbittable[i] < 0)
    {
        printf("*Warning* The baud rate %f is not supported or out of range, using 9600\r\n",*baud);
        i = 8;
    }
    *baud =  baudratetable[i];
    
    return baudspeedbittable[i];
}
