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

/* define SIZE_MAX if not already 
 * defined (e.g. by a C99 compiler) */
#ifndef SIZE_MAX
#define SIZE_MAX ((size_t)-1)
#endif

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

/* This is used by ExtSerialPortDataPending to read and cache this number 
 * of bytes in case there is data on the comm line. Even the smallest 
 * external mode message (like the connect message) will at least be of 
 * size 8 bytes. Do not increase this value since the implementation of 
 * rtIOStreamRecv on the target side might block until it receives this 
 * number of bytes.
 */
#define PENDING_DATA_CACHE_SIZE 8
      
typedef struct UserData_tag {
    int streamID;
    char pendingRecvdData[PENDING_DATA_CACHE_SIZE];
    uint8_T numPending;
    uint8_T startIdxPending;
} UserData;

static UserData *UD;
    
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

    /* Determine and save endianess. */
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

    UD->streamID = rtIOStreamOpen(argc, (void *)argv);        
    if (UD->streamID == RTIOSTREAM_ERROR) {
        portDev->fConnected = false;
        error = EXT_ERROR;
        goto EXIT_POINT;
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
   
   result = rtIOStreamClose(UD->streamID);         

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
    boolean_T error = EXT_NO_ERROR;
    int_T result;
    size_t sizeSent;
    size_t transferAmount;
    /* use a variable to avoid SIZE_MAX being treated as a constant
     * which leads to compiler warnings for "MIN" on platforms where 
     * SIZE_MAX > UINT32_MAX */
    size_t sizeMax = SIZE_MAX;

    if (!portDev->fConnected) return EXT_ERROR;

    while (size > 0) {
       /* support full uint32 size */
       transferAmount = (size_t) MIN(sizeMax, size);
       result = rtIOStreamSend(UD->streamID,
             data,
             transferAmount,
             &sizeSent);    
       if (result == RTIOSTREAM_ERROR) {
          error = EXT_ERROR;
          return error;
       }
       else {
         size -= (uint32_T) sizeSent;
         data += sizeSent;
       }
    }
    return(error);

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
      result = rtIOStreamRecv(UD->streamID,
                                UD->pendingRecvdData,
                                (const size_t)PENDING_DATA_CACHE_SIZE,
                                &sizeRecvd);
               
    if (result == RTIOSTREAM_ERROR) {
        errorCode = EXT_ERROR;
        return errorCode;
    }

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
    size_t sizeRecvd = 0;
    uint8_T nBytesToGet = 1; /* we are reading one char */
    
    if (!portDev->fConnected) return EXT_ERROR;
    
    if (UD->numPending > 0) {
        memcpy(dst, &(UD->pendingRecvdData[UD->startIdxPending]), nBytesToGet);
        UD->numPending -= nBytesToGet;
        UD->startIdxPending += nBytesToGet;
        *bytesRead = (uint32_T) nBytesToGet;
        return errorCode;
    }
    else {
        /* Call rtIOStreamRecv */
        while(sizeRecvd!=1) {
            result = rtIOStreamRecv(UD->streamID, dst, (size_t) nBytesToGet, &sizeRecvd);
            if (result == RTIOSTREAM_ERROR) {                
                errorCode = EXT_ERROR;
                break;
            }
        }
    }
    
    *bytesRead = (uint32_T) (sizeRecvd);     
    return errorCode;        

} /* end ExtSerialPortGetRawChar */


/* [EOF] rtiostream_serial_interface.c */
