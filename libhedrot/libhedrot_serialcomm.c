//
// libhedrot_serialcomm.c
// root functions for serial communication with the headtracker
//
// Part of code is derived from "comport", (c) 1998-2005  Winfried Ritsch, Institute for Electronic Music - Graz
//
// Copyright 2016 Alexis Baskind


#include "libhedrot_serialcomm.h"
#include "stdlib.h"
#include "string.h"

// global tables
#if defined(_WIN32) || defined(_WIN64)
/* we don't use the  table for windos cos we can set the number directly. */
/* This may result in more possible baud rates than the table contains. */
#else /* #if defined(_WIN32) || defined(_WIN64) */
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
#endif /* #if defined(_WIN32) || defined(_WIN64) */

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
    
    int				i;    
	char*			tmpPortsNames[MAX_NUMBER_OF_PORTS];

#if defined(_WIN32) || defined(_WIN64)
	HANDLE			fd;
	char            device_name[10];
    DWORD           dw;
#else /* #if defined(_WIN32) || defined(_WIN64) */
    int            fd;
	struct termios test;
    glob_t         glob_buffer;

	const char		*glob_pattern = "/dev/cu.*";
#endif /* #if defined(_WIN32) || defined(_WIN64) */

	// free the previous port list
    for (i = 0; i < x->numberOfAvailablePorts; i++) {
        free(x->availablePorts[i]);
    }
    if(x->availablePorts) {
        free(x->availablePorts);
        x->availablePorts = NULL;
    }
    x->numberOfAvailablePorts = 0;

	// reset port number (for autodiscovering)
    x->portNumber=-1;

#if defined(_WIN32) || defined(_WIN64)
    for(i = 1; i < MAX_NUMBER_OF_PORTS; i++) {
        sprintf_s(device_name, 10, "\\\\.\\COM%d", i);/* the recommended way to specify COMs above 9 */
        fd = CreateFile( device_name,
                GENERIC_READ | GENERIC_WRITE,
                0,
                0,
                OPEN_EXISTING,
                FILE_FLAG_OVERLAPPED,
                0);
        dw = 0L;
        if(fd == INVALID_HANDLE_VALUE)
            dw = GetLastError();
        else
            CloseHandle(fd);

        if (dw == 0) {
			//port available
			tmpPortsNames[x->numberOfAvailablePorts] = _strdup(device_name);
            x->numberOfAvailablePorts++;

			if(x->verbose) 
				printf("[hedrot]: port %s available\r\n", device_name);
		}

    }

    // update the list of available ports
    x->availablePorts = (char**) malloc(x->numberOfAvailablePorts*sizeof(char*));
    for(i=0; i<x->numberOfAvailablePorts; i++)
        x->availablePorts[i] = _strdup(tmpPortsNames[i]);

#else /* #if defined(_WIN32) || defined(_WIN64) */
    /* first look for registered devices in the filesystem */
   
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
    x->availablePorts = (char**) malloc(x->numberOfAvailablePorts*sizeof(char*));
    for(i=0; i<x->numberOfAvailablePorts; i++)
        x->availablePorts[i] = strdup(tmpPortsNames[i]);


    globfree( &(glob_buffer) );

#endif /* #if defined(_WIN32) || defined(_WIN64) */
}


// open serial port by its name
// returns INVALID_HANDLE_VALUE if fails
// returns handle value if it succeeds

#if defined(_WIN32) || defined(_WIN64)
// Windows version
HANDLE open_serial(headtrackerSerialcomm *x, char* portName) {
    HANDLE          fd;
    COMMTIMEOUTS	timeouts;
    DCB				dcb; /* holds the comm params */
    DWORD			dw;

    
    /* the communication is through USB, so:
     * The number of bits is always 8
     * no stop bits
     * no hardware/software handshake
     * parity bits are not used */
    
    fd = CreateFile( portName,
        GENERIC_READ | GENERIC_WRITE,
        0,
        0,
        OPEN_EXISTING,
        FILE_FLAG_OVERLAPPED,
        0);

    if(fd == INVALID_HANDLE_VALUE)
    {
        dw = GetLastError();
		printf("[hedrot] ** ERROR ** could not open device %s: failure(%d): %s\r\n",
              portName,errno, dw);
        return INVALID_HANDLE_VALUE;
    }

    
    /* set no wait on any operation */
	//fcntl(fd, F_SETFL, FNDELAY);
    
    /* Get the Current Port Configuration  */
	memset(&(dcb), 0, sizeof(DCB));

    if (!GetCommState(fd, &(dcb)))
    {
        printf("[hedrot] ** ERROR ** could not get dcb of device %s\r\n", portName);
        CloseHandle(fd);
        return INVALID_HANDLE_VALUE;
    }

	dcb.fBinary = TRUE; /*  binary mode, no EOF check  */
    dcb.fErrorChar = FALSE;       /*  enable error replacement  */

	// set baud rate
	dcb.BaudRate = (DWORD)x->baud;
	dcb.ByteSize = 8;
    dcb.StopBits = ONESTOPBIT;
    dcb.Parity   = NOPARITY;
    
    if(SetCommState(fd, &dcb)) {
        if(x->verbose) printf("[hedrot] opened serial line device %s\r\n", portName);
    } else {
        printf("[hedrot] ** ERROR ** could not set params to control dcb of device %s\r\n", portName);
        CloseHandle(fd);
        return INVALID_HANDLE_VALUE;
    }

	/* setting new timeouts for read to immediately return */
    timeouts.ReadIntervalTimeout = MAXDWORD;
    timeouts.ReadTotalTimeoutMultiplier = 0;
    timeouts.ReadTotalTimeoutConstant = 0;
    timeouts.WriteTotalTimeoutMultiplier = 0;
    timeouts.WriteTotalTimeoutConstant = 0;

    if (!SetCommTimeouts(fd, &timeouts))
    {
        printf("[hedrot] ** ERROR ** Couldn't set timeouts for serial device (%d)\r\n", GetLastError());
		CloseHandle(fd);
        return INVALID_HANDLE_VALUE;
    }

	if (!SetupComm(fd, 4096L, 4096L))/* try to get big buffers to avoid overruns*/
	{
		printf("[hedrot] ** ERROR ** Couldn't do SetupComm (%d)\r\n", GetLastError());
		CloseHandle(fd);
		return INVALID_HANDLE_VALUE;
	}

	// update structure info
    x->serial_device_name = _strdup(portName);
    x->comhandle = fd;
    return fd;

}
#else /* #if defined(_WIN32) || defined(_WIN64) */
// Mac version
int open_serial(headtrackerSerialcomm *x, char* portName) {
    int             fd;

    struct termios  *tios = &(x->com_termio);f
    
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
    
    /* Get the Current Port Configuration  */
    if(tcgetattr(fd, tios) < 0) {
        printf("[hedrot] ** ERROR ** could not get termios-structure of device %s\r\n", portName);
        close(fd);
        return INVALID_HANDLE_VALUE;
    }

	// set baud rate
    if (cfsetspeed(tios, x->baud) < 0) {
        printf("error in cfsetspeed\n");
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
#endif /* #if defined(_WIN32) || defined(_WIN64) */

// close serial port
// returns INVALID_HANDLE_VALUE in any case

#if defined(_WIN32) || defined(_WIN64)
// Windows version
HANDLE close_serial(headtrackerSerialcomm *x) {
    if(x->comhandle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(x->comhandle);
        if(x->verbose) printf("[hedrot] closed %s\r\n",x->serial_device_name);
        x->serial_device_name = NULL;
        x->comhandle = INVALID_HANDLE_VALUE;
    }
    return INVALID_HANDLE_VALUE;
}
#else /* #if defined(_WIN32) || defined(_WIN64) */
// Mac version
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
#endif /* #if defined(_WIN32) || defined(_WIN64) */

// clear file descriptors before reading data on opened comm port
void init_read_serial(headtrackerSerialcomm *x) {
#if defined(_WIN32) || defined(_WIN64)
	// nothing to do here
#else /* #if defined(_WIN32) || defined(_WIN64) */
    FD_ZERO(&x->com_rfds);
    FD_SET(x->comhandle,&x->com_rfds);
    tv.tv_sec = 0;
    tv.tv_usec = 0;
#endif /* #if defined(_WIN32) || defined(_WIN64) */
}


// check if a data is available for reading on opened comm port, if yes, reads it
int is_data_available(headtrackerSerialcomm *x) {
    int          err;
#if defined(_WIN32) || defined(_WIN64)
    OVERLAPPED    osReader = {0};

    osReader.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if(ReadFile(x->comhandle, x->readBuffer, READ_BUFFER_SIZE, &x->numberOfReadBytes, &osReader)) {
		err = 1;
    } else {
        err = 0; // read error
    }
	if(x->numberOfReadBytes ==0)
		err = 0;
    CloseHandle(osReader.hEvent);
#else /* #if defined(_WIN32) || defined(_WIN64) */
    ssize_t numberOfBytes;
    err =  select(x->comhandle+1,&x->com_rfds,NULL,NULL,&tv);
    if(err) { // data is available, read it
        numberOfBytes = (unsigned long) read(x->comhandle, x->readBuffer,READ_BUFFER_SIZE);
        if(numberOfBytes == -1) { // read error
            x->numberOfReadBytes = 0;
            err = 0;
        } else {
            x->numberOfReadBytes = (unsigned long) numberOfBytes;
        }
    }
#endif /* #if defined(_WIN32) || defined(_WIN64) */
	return err;
}




// write byte on serial port
// return 0 if fails
// return 1 if it succeeds
#if defined(_WIN32) || defined(_WIN64)
// Windows version
int write_serial(headtrackerSerialcomm *x, unsigned char *serial_byte, unsigned long numberOfBytesToWrite) {
	OVERLAPPED osWrite = {0};
    DWORD      dwWritten;
    DWORD      dwRes;
    DWORD      dwErr;
	int			fRes;
	unsigned long i;

	if(x->verbose >= 2) 
	{
		printf("write %d bytes on comhandle %i:", numberOfBytesToWrite, x->comhandle);
		for(i = 0;i<numberOfBytesToWrite;i++)
			printf("%d: %u",i,(unsigned int)serial_byte[i]);
		printf("\r\n");
	}

	// Create this write operation's OVERLAPPED structure's hEvent.
    osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (osWrite.hEvent == NULL)
    {
        // error creating overlapped event handle
		printf("Couldn't create event. Transmission aborted.\r\n");
        return 0;
    }

    if (!WriteFile(x->comhandle, serial_byte, numberOfBytesToWrite, &dwWritten, &osWrite))
    {
        dwErr = GetLastError();
        if (dwErr != ERROR_IO_PENDING)
        {
			// WriteFile failed, but isn't delayed. Report error and abort
            printf("WriteFile failed, but isn't delayed. WriteFile error: %d\r\n", (int)dwErr);
            fRes = 0;
        }
	else
	{
         // Write is pending.
         dwRes = WaitForSingleObject(osWrite.hEvent, INFINITE);
         switch(dwRes)
         {
            // OVERLAPPED structure's event has been signaled. 
            case WAIT_OBJECT_0:
                 if (!GetOverlappedResult(x->comhandle, &osWrite, &dwWritten, FALSE))
                       fRes = 0;
                 else
                  // Write operation completed successfully.
                  fRes = 1;
                 break;
            
            default:
                 // An error has occurred in WaitForSingleObject.
                 // This usually indicates a problem with the
                // OVERLAPPED structure's event handle.
                 printf("An error has occurred in WaitForSingleObject.\r\n");
				 fRes = 0;
                 break;
         }
      }
   }
   else
      // WriteFile completed immediately.
      fRes = 1;

   CloseHandle(osWrite.hEvent);
   return fRes;
}

#else /* #if defined(_WIN32) || defined(_WIN64) */
// Mac version
int write_serial(headtrackerSerialcomm *x, unsigned char *serial_byte, unsigned long numberOfBytesToWrite) {
    	if(x->verbose >= 2) 
	{
		printf("write %d bytes on comhandle %i:", numberOfBytesToWrite, x->comhandle);
		for(i = 0;i<numberOfBytesToWrite;i++)
			printf("%d: %u",i,(unsigned int)serial_byte[i]);
		printf("\r\n");
	}

    int result = (int) write(x->comhandle,(char *) serial_byte,numberOfBytesToWrite);
    if (result != 1) {
        printf ("[hedrot] write on comhandle %i returned %d, errno is %d\r\n", x->comhandle, result, errno);
    }
    return result;
}
#endif /* #if defined(_WIN32) || defined(_WIN64) */