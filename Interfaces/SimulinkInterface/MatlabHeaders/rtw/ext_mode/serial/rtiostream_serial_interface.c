/*
 * Copyright 2011-2012 The MathWorks, Inc.
 *
 * File: rtiostream_serial_interface.c     
 *
 * Abstract:
 *  The External Mode Serial Port is a logical object providing a standard
 *  interface between the external mode code and the physical serial port 
 *  through the rtiostream interface.
 *  The prototypes in the 'Visible Functions' section of this file provide
 *  the consistent front-end interface to external mode code.  The
 *  implementations of these functions provide the back-end interface to the
 *  physical serial port.  This layer of abstraction allows for minimal
 *  modifications to external mode code when the physical serial port is
 *  changed. The physical serial port functions are implemented by the 
 *  rtiostream serial interface.
 *
 *     ----------------------------------
 *     | Host/Target external mode code |
 *     ----------------------------------
 *                   / \
 *                  /| |\
 *                   | |
 *                   | |
 *                  \| |/
 *                   \ /  Provides a standard, consistent interface to extmode
 *     ----------------------------------
 *     | External Mode Serial Port      |
 *     ----------------------------------
 *                   / \  Function definitions specific to physical serial port
 *                  /| |\ (implemented by rtiostream interface)
 *                   | |
 *                   | |
 *                  \| |/
 *                   \ /
 *     ----------------------------------
 *     | HW/OS/Physical serial port     |
 *     ----------------------------------
 *
 *  See also ext_serial_pkt.c.
 */

#include <string.h>

#ifdef MATLAB_MEX_FILE
   #include "tmwtypes.h"
#else
   #include "rtwtypes.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include "ext_types.h"
#include "ext_share.h"
#include "ext_serial_port.h"
#include "ext_serial_pkt.h"
#include "rtiostream.h"
#include "rtiostream_utils.h"
#include "mex.h"
#include "version.h"
        
#define PENDING_DATA_CACHE_SIZE 8
      
typedef struct UserData_tag {
    int streamID;
    char pendingRecvdData[PENDING_DATA_CACHE_SIZE];
    uint8_T numPending;
    uint8_T startIdxPending;
} UserData;

static UserData *UD;
   
static libH_type libH;

/* Function: loadSharedLib ===========================================================
 * Abstract:
 */
PRIVATE int loadSharedLib(void) {

    #define RTIOSTREAM_SHARED_LIB "libmwrtiostreamserial"
            
    static char libName[] = RTIOSTREAM_SHARED_LIB SL_EXT ;
    int error;
    
    error = rtIOStreamLoadLib( &(libH), libName);

    if (error != 0) {
        mexPrintf("Failed to load rtIOStream shared library %s. Check that the library "
                  "is located in a directory on your system path.", libName);/*lint !e534 
                                                                              * ignore return value of mexPrintf */
    }

    return error;
}


/***************** VISIBLE FUNCTIONS ******************************************/

/* Function: ExtSerialPortCreate ===============================================
 * Abstract:
 *  Creates an External Mode Serial Port object.  The External Mode Serial Port
 *  is an abstraction of the physical serial port providing a standard
 *  interface for external mode code.  A pointer to the created object is
 *  returned.
 *
 */
PUBLIC ExtSerialPort *ExtSerialPortCreate(void)
{
    static ExtSerialPort serialPort;
    ExtSerialPort *portDev = &serialPort;

    /* Determine and save Endianess. */
    {
        union Char2Integer_tag
        {
            int IntegerMember;
            char CharMember[sizeof(int)];
        } temp;

        temp.IntegerMember = 1;
        if (temp.CharMember[0] != 1)
            portDev->isLittleEndian = false;
        else
            portDev->isLittleEndian = true;
    }

    portDev->fConnected = false;

    return portDev;

} /* end ExtSerialPortCreate */


/* Function: ExtSerialPortConnect ==============================================
 * Abstract:
 *  Performs a logical connection between the external mode code and the
 *  External Mode Serial Port object and a real connection between the External
 *  Mode Serial Port object and the physical serial port.
 *
 *  EXT_NO_ERROR is returned on success, EXT_ERROR on failure.
 */
PUBLIC boolean_T ExtSerialPortConnect(ExtSerialPort *portDev,
                                      const int argc,
                                      const char ** argv)
{
    boolean_T error = EXT_NO_ERROR;
        
    if (portDev->fConnected) {
        error = EXT_ERROR;
        goto EXIT_POINT;
    }

    /* allocate memory for UserData */
    UD = (UserData *)calloc(1UL, sizeof(UserData));
    if (UD==NULL) {
        error = EXT_ERROR;
        goto EXIT_POINT;
    }
    
    /* Initialize number of pending (cached) units of data */
    UD->numPending = 0;
    UD->startIdxPending = 0;

    portDev->fConnected = true;
    
    {
        /* Load the rtIOStream shared library */
        int errorSharedLib;
        errorSharedLib = loadSharedLib();
        if (errorSharedLib != RTIOSTREAM_NO_ERROR) {
            
            /* Error out immediately */
            mexErrMsgIdAndTxt("rtiostream_serial_interface:ExtProcessArgs:"
                    "LoadSharedLibFailure",
                    "Error loading rtIOStream shared library; "
                    "see command window for details.");
            
        } else {
            
            /* Call rtIOStreamOpen */
            UD->streamID = ( *(libH.openFn) ) (argc, (void *)argv);
            if (UD->streamID == RTIOSTREAM_ERROR) {
                portDev->fConnected = false;
                error = EXT_ERROR;
                goto EXIT_POINT;
            }
        }
    }

  EXIT_POINT:
    return error;

} /* end ExtSerialPortConnect */


/* Function: ExtSerialPortDisconnect ===========================================
 * Abstract:
 *  Performs a logical disconnection between the external mode code and the
 *  External Mode Serial Port object and a real disconnection between the
 *  External Mode Serial Port object and the physical serial port.
 *
 *  EXT_NO_ERROR is returned on success, EXT_ERROR on failure.
 */
PUBLIC boolean_T ExtSerialPortDisconnect(ExtSerialPort *portDev)
{
   boolean_T error = EXT_NO_ERROR;
   int_T result;       

   if (!portDev->fConnected) return EXT_ERROR;
   
   portDev->fConnected = false;
   
   /* Call rtIOStreamClose */
    result = ( *(libH.closeFn) ) (UD->streamID);       

   if (UD != NULL) free(UD);

   if (result == RTIOSTREAM_ERROR) {
        error = EXT_ERROR;
    }
   
    return(error);

} /* end ExtSerialPortDisconnect */


/* Function: ExtSerialPortSetData ==============================================
 * Abstract:
 *  Sets (sends) the specified number of bytes on the comm line. 
 *
 *  EXT_NO_ERROR is returned on success, EXT_ERROR on failure.
 */
PUBLIC boolean_T ExtSerialPortSetData(ExtSerialPort *portDev,
                                      char *data,
                                      uint32_T size)
{    
    boolean_T errorCode = EXT_NO_ERROR;
    int_T result;
    const int sendTimeOutSecs = 120;
    int timeoutOccurred = 0;
    
    if (!portDev->fConnected) return EXT_ERROR;
        	
    result = rtIOStreamBlockingSend( &(libH) ,
                                    UD->streamID,
                                    (const uint8_T *) data,
                                    (size_t) size,
                                    sendTimeOutSecs, 
                                    &timeoutOccurred);    
    if (result) {
        errorCode = EXT_ERROR;
        return errorCode;
    }
    
    return errorCode;
       
} /* end ExtSerialPortSetData */


/* Function: ExtSerialPortDataPending ==========================================
 * Abstract:
 * Returns true, via the 'pending' arg, if data is pending on the comm line.
 *
 *  EXT_NO_ERROR is returned on success, EXT_ERROR on failure.
 */
PUBLIC boolean_T ExtSerialPortDataPending(ExtSerialPort *portDev,
                                          boolean_T *pending)
{
    boolean_T errorCode = EXT_NO_ERROR;
    int_T result;
    size_t sizeRecvd=0;
    if (!portDev->fConnected) return EXT_ERROR;
    
    if (UD->numPending > 0) {
        *pending = 1;
        return errorCode;
    } else {
        *pending = 0;
    }

    /* Call rtIOStreamRecv */
    result = ( *(libH.recvFn) ) (UD->streamID,
                                 UD->pendingRecvdData,
                                (const size_t)PENDING_DATA_CACHE_SIZE,
                                &sizeRecvd);
    
    if (result == RTIOSTREAM_ERROR) return EXT_ERROR;
    
      if (sizeRecvd>0) {
          *pending = 1;
          UD->numPending = (uint8_T)sizeRecvd;
          UD->startIdxPending = 0;
      }

    return errorCode;    

} /* end ExtSerialPortDataPending */


/* Function: ExtSerialPortGetRawChar ===========================================
 * Abstract:
 *  Attempts to get one byte from the comm line.  The number of bytes read is
 *  returned via the 'bytesRead' parameter.
 *
 *  EXT_NO_ERROR is returned on success, EXT_ERROR on failure.
 *
 */
PUBLIC boolean_T ExtSerialPortGetRawChar(ExtSerialPort *portDev,
                                         char *dst,
                                         uint32_T *bytesRead)
{
    boolean_T errorCode = EXT_NO_ERROR;
    int_T result = RTIOSTREAM_NO_ERROR;
    const int recvTimeOutSecs = 120;
    int timeoutOccurred = 0;    
    uint8_T nBytesToGet = 1; /* we are reading one char */
    *bytesRead = 0;
    
    if (!portDev->fConnected) return EXT_ERROR;
    
    if (UD->numPending > 0) {
        memcpy(dst, &(UD->pendingRecvdData[UD->startIdxPending]), nBytesToGet);
        UD->numPending -= nBytesToGet;
        UD->startIdxPending += nBytesToGet;
    }
    else {
        result = rtIOStreamBlockingRecv(&(libH) ,
                (const int) UD->streamID,
                (uint8_T *) dst,
                (const size_t) nBytesToGet,
                recvTimeOutSecs,
                &timeoutOccurred);
        
        if (result == RTIOSTREAM_ERROR) return EXT_ERROR;
    }
    
    *bytesRead = (uint32_T) (nBytesToGet);     
    return errorCode;        

} /* end ExtSerialPortGetRawChar */


/* [EOF] rtiostream_serial_interface.c */

/* LocalWords:  extmode libmwrtiostreamserial Recv
 */
