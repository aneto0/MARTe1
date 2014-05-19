/*
 * Copyright 1994-2012 The MathWorks, Inc.
 *
 * File: rtiostream_interface.c     
 *
 * Abstract: Provide a host-side communications driver interface for Simulink
 * external mode.
 */


/***************** TRANSPORT-INDEPENDENT INCLUDES *****************************/

#include <ctype.h>
#include <string.h>

#include "version.h"
#include "mex.h"
#include "extsim.h"
#include "extutil.h"
#include "rtiostream.h"
#include "rtiostream_utils.h"
#include "mwutil.h"


#if defined (_WIN32)
#include <windows.h>   /* for Sleep() */
#else
#include <time.h>      /* for nanosleep(), struct timespec */
#endif

#if defined (EXTMODE_SERIAL_TRANSPORT)
    #define RTIOSTREAM_SHARED_LIB "libmwrtiostreamserial"
    #define BAUD_NUM_STR_LEN 7
    #define PORT_NAME_STR_LEN 7    
#elif defined (EXTMODE_TCPIP_TRANSPORT)
    #define RTIOSTREAM_SHARED_LIB "libmwrtiostreamtcpip"
    #define PORT_NUM_STR_LEN 6
#else
    #error Unsupported external mode transport layer
#endif
        
#if defined (EXTMODE_SERIAL_TRANSPORT) && defined (EXTMODE_TCPIP_TRANSPORT)
    #error Cannot have both EXTMODE_SERIAL_TRANSPORT and EXTMODE_TCPIP_TRANSPORT defined
#endif

#define PENDING_DATA_CACHE_SIZE 8

typedef struct UserData_tag {
    int streamID;
    char pendingRecvdData[PENDING_DATA_CACHE_SIZE];
    int numPending;
    size_t startIdxPending;
} UserData;

static libH_type libH;

/* Function: loadSharedLib ===========================================================
 * Abstract:
 */
PRIVATE int loadSharedLib(void) {

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

/* Function: SleepMilliseconds =========================================================
 * Abstract:
 *    Sleep for specified number of milliseconds. 
 *
 */
PRIVATE void SleepMilliseconds(const int t_msecs)
{
#if defined (_WIN32)

    Sleep( (DWORD)(t_msecs) );

#else

    struct timespec clkSleep;
    const int t_secs = t_msecs/1000;
    const int t_nsecs = (t_msecs - 1000 * t_secs) * 1e6; /*lint !e790 loss of precision double to int */
    
    clkSleep.tv_sec  = (time_t) t_secs;
    clkSleep.tv_nsec = (long)   t_nsecs;
    
    nanosleep( &clkSleep, NULL ); /*lint !e534 ignoring return value */

#endif
}


/* Function: ExtOpenConnection ===========================================================
 * Abstract:
 */
PUBLIC void ExtOpenConnection(ExternalSim *ES)
{
    UserData * const userData = (UserData *)esGetUserData(ES);

    /* rtIOStreamOpen has already been called from ExtProcessArgs */


    /* Initialize number of pending (cached) units of data */
    userData->numPending = 0;
    userData->startIdxPending = 0;

}

        



/* Function: ExtProcessArgs ===========================================================
 * Abstract:
 */
PUBLIC void ExtProcessArgs(
    ExternalSim   *ES,
    int           nrhs,
    const mxArray *prhs[])
{
#define ARGC_MAX 10
    int argc = 0;
    char * argv[ARGC_MAX]={NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
    char * hostName = NULL;
    UserData  * const userData = (UserData *)esGetUserData(ES);

#if defined(EXTMODE_TCPIP_TRANSPORT)

    /* ... Argument 1 - host name */
    if (nrhs >= 1 && !mxIsEmpty(prhs[0])) {
        /* host name specified */
        const mxArray * const mat = prhs[0];
        const size_t        len  = mxGetN(mat) + 1;

        if (mxGetM(mat) != 1) {
            esSetError(ES, "Expected 1xN string array for host name\n");
            return;
        }

        hostName = (char *)calloc(len, sizeof(char));
        if (hostName == NULL) {
            esSetError(ES, "Memory allocation error.");
            return;
        }
        
        if (mxGetString(mat, hostName, (mwSize) len)) {
            esSetError(ES, "Error converting matlab string to C string.");
            free(hostName);
            return;
        }
        {
            /* Use static storage: data must persist outside this scope */
            static char hostNameStr[] = "-hostname"; 

            argv[argc++] = (void *)hostNameStr;
            argv[argc++] = (void *)hostName;
        }
    } /* else default to local host in ExtOpenConnection */

    if(nrhs >= 2) {
        const mxArray * const mat = prhs[1];
        const size_t         m    = mxGetM(mat);
        const size_t         n    = mxGetN(mat);
        static int     verbosity; /* Use static: must persist outside this scope */
        const double   matVal = *mxGetPr(mat);

        const char msg[] =
            "Verbosity argument must be a real, scalar, integer value in the "
            "range: [0-1].";

        /* verify that we've got a real, scalar integer */
        if (!mxIsNumeric(mat) || mxIsComplex(mat) || mxIsSparse(mat) ||
            !mxIsDouble(mat) || (!(m ==1 && n ==1)) || !IS_INT(matVal)) {
            esSetError(ES, msg);
            return;
        }
        verbosity = (int) matVal;

        /* verify that it's 0 or 1 */
        if ((verbosity != 0) && (verbosity != 1)) {
            esSetError(ES, msg);
            return;
        }
        
        esSetVerbosity(ES, verbosity);/*lint !e734 loss of precision 31 bits to 8 bits*/
        
    }

    /* ... Argument 3 - TCP port value */
    if (nrhs >= 3) {
        unsigned int port; 
        const mxArray * const mat = prhs[2];
        const size_t         m    = mxGetM(mat);
        const size_t         n    = mxGetN(mat);
        const double   matVal = *mxGetPr(mat);

        const char msg[] = 
            "Port argument must be a real, scalar, integer value in the range: "
            "[256-65535].";

        /* verify that we've got a real, scalar integer */
        if (!mxIsNumeric(mat) || mxIsComplex(mat) || mxIsSparse(mat) ||
            !mxIsDouble(mat) || (!(m ==1 && n ==1)) || !IS_INT(matVal)) {
            esSetError(ES, msg);
            return;
        }
        port = (unsigned int) matVal;

        /* check port range */
        if (port < 256 || port > 65535) {
            esSetError(ES, msg);
            return;
        }

        {
            static char portStr[] = "-port"; /* Use static: must persist 
                                                * outside this scope */
            static char portNumStr[PORT_NUM_STR_LEN]; /* Use static: must 
                                                       * persist outside 
                                                       * this scope */
            sprintf(portNumStr, "%5d", (int) port);

            argv[argc++] = (void *)portStr;
            argv[argc++] = (void *)portNumStr;
        }
    } 

    /* ... Argument 4 - open rtIOStream as TCP/IP client */
    {
        static char isClientStr[] = "-client"; /* Use static: must persist 
                                                  * outside this scope */

        static char isClientStrVal[] = "1";    /* Use static: must 
                                                  * persist outside 
                                                  * this scope */
        argv[argc++] = (void *)isClientStr;
        argv[argc++] = (void *)isClientStrVal;
    }
    
#endif

#if defined(EXTMODE_SERIAL_TRANSPORT)
/* ... Argument 1 - verbosity */
    if(nrhs >= 1 && !mxIsEmpty(prhs[0])) {
        const mxArray * const mat = prhs[0];
        const size_t         m    = mxGetM(mat);
        const size_t         n    = mxGetN(mat);
        static int     verbosity; /* Use static: must persist outside this scope */
        const double   matVal = *mxGetPr(mat);

        const char msg[] =
            "Verbosity argument must be a real, scalar, integer value in the "
            "range: [0-1].";

        /* verify that we've got a real, scalar integer */
        if (!mxIsNumeric(mat) || mxIsComplex(mat) || mxIsSparse(mat) ||
            !mxIsDouble(mat) || (!(m ==1 && n ==1)) || !IS_INT(matVal)) {
            esSetError(ES, msg);
            return;
        }
        verbosity = (int) matVal;

        /* verify that it's 0 or 1 */
        if ((verbosity != 0) && (verbosity != 1)) {
            esSetError(ES, msg);
            return;
        }
        
        esSetVerbosity(ES, verbosity);/*lint !e734 loss of precision 31 bits to 8 bits*/
        
    }    

    /* ... Argument 2 - serial comm port value */
    if (nrhs >= 2) {
        uint16_T      port;
        const mxArray *mat = prhs[1];
        size_t         m    = mxGetM(mat);
        size_t         n    = mxGetN(mat);
        const double   matVal = *mxGetPr(mat);

        const char msg[] = 
            "Port argument must be a real, scalar, integer value in the range: "
            "[1-32].";
         
        /* verify that we've got a real, scalar integer */
        if (!mxIsNumeric(mat) || mxIsComplex(mat) || mxIsSparse(mat) ||
            !mxIsDouble(mat) || (!(m ==1 && n ==1)) || !IS_INT(matVal)) {
            esSetError(ES, msg);
            return;
        }
     
        port = (unsigned int) matVal;

        /* check port range */
        if (port < 1 || port > 32) {
            esSetError(ES, msg);
            return;
        }

        {
            static char portStr[] = "-port"; /* Use static: must persist 
                                                * outside this scope */

            static char portName[PORT_NAME_STR_LEN];
            sprintf(portName, "COM%d", port);                    
            
            argv[argc++] = (void *)portStr;
            argv[argc++] = (void *)portName;
        }
    } /* else default to COM1 in rtiostream_serial */

     /* ... Argument 3 - serial baud rate value */
    if (nrhs >= 3) {        
        unsigned int baud; 
        const mxArray * const mat = prhs[2];
        const size_t         m    = mxGetM(mat);
        const size_t         n    = mxGetN(mat);        
        const double       matVal = *mxGetPr(mat);
        
        const char msg[] = 
                "Baud argument must be a real, scalar, integer value.";
        
        /* verify that we've got a real, scalar integer */
        if (!mxIsNumeric(mat) || mxIsComplex(mat) || mxIsSparse(mat) ||
            !mxIsDouble(mat) || (!(m ==1 && n ==1)) || !IS_INT(matVal)) {
            esSetError(ES, msg);
            return;
        }

        baud = (unsigned int) matVal;       
        
        {
            static char baudStr[] = "-baud"; /* Use static: must persist 
                                                * outside this scope */
            static char baudNumStr[BAUD_NUM_STR_LEN]; /* Use static: must 
                                                       * persist outside 
                                                       * this scope */
            
            /* 6 digits is enough to hold the maximum baud setting (128000) */
            sprintf(baudNumStr, "%6d", baud);

            argv[argc++] = (void *)baudStr;
            argv[argc++] = (void *)baudNumStr;
        }
    } /* else default to 57600 in rtiostream_serial */

#endif

    assert(argc < ARGC_MAX);

    {
        /* Load the rtIOStream shared library */
        int error;
        error = loadSharedLib();
        if (error != RTIOSTREAM_NO_ERROR) {

            /* Error out immediately */
            mexErrMsgIdAndTxt("rtiostream_interface:ExtProcessArgs:"
                              "LoadSharedLibFailure",
                              "Error loading rtIOStream shared library; "
                              "see command window for details.");

        } else {

            /* Call rtIOStreamOpen */
            userData->streamID = ( *(libH.openFn) ) (argc, (void *)argv);
            if (userData->streamID == RTIOSTREAM_ERROR) {
                const char msg[] = 
                    "An error occurred attempting to open an rtIOStream. More detail "
                    "may be reported in the MATLAB command window\n";
                esSetError(ES, msg);
            }
        }
    }
    if (hostName!=NULL) {
        free(hostName);
    }
}

/* Function: ExtUserDataCreate =================================================
 * Abstract:
 *  Create the user data.
 */
PUBLIC UserData *ExtUserDataCreate(void)
{
    UserData *ud = (UserData *)calloc(1UL, sizeof(UserData));
    return(ud);
} /* end ExtUserDataCreate */


/* Function: ExtUserDataDestroy ================================================
 * Abstract:
 *  Destroy the user data.
 */
PUBLIC void ExtUserDataDestroy(UserData *userData)
{
    if (userData != NULL) {
        free(userData);
    }
} /* end ExtUserDataDestroy */


/* Function: ExtGetTargetPkt ===================================================
 * Abstract:
 *  Attempts to get the specified number of bytes from the comm line.  The
 *  number of bytes read is returned via the 'nBytesGot' parameter.
 *  EXT_NO_ERROR is returned on success, EXT_ERROR is returned on failure.
 *
 * NOTES:
 *  o it is not an error for 'nBytesGot' to be returned as 0
 *  o it is o.k. for this function to block if no data is available (e.g.,
 *    a recv call on a blocking socket)
 */
PUBLIC boolean_T ExtGetTargetPkt(
    const ExternalSim *ES,
    const int         totalNBytesToGet,
    int               *nBytesGot, /* out */
    char              *dst)       /* out */
{
    int retVal;
    boolean_T error;
    size_t sizeRecvd;
    size_t nBytesToGet = (size_t) totalNBytesToGet;
    size_t numFromCache = 0;
    UserData  *userData = (UserData *)esGetUserData(ES);

    if (userData->numPending > 0) {

        numFromCache = MIN((size_t)nBytesToGet, (size_t)userData->numPending);

        memcpy(dst, &(userData->pendingRecvdData[userData->startIdxPending]), numFromCache);

        userData->numPending -= (int)numFromCache;
        userData->startIdxPending += numFromCache;

        dst += numFromCache;
        nBytesToGet -= numFromCache;

    }

    /* Call rtIOStreamRecv */
    retVal = ( *(libH.recvFn) ) ((const int) userData->streamID,
                            dst,
                            (size_t) nBytesToGet,
                            &sizeRecvd);

    *nBytesGot = (int) (sizeRecvd + numFromCache);

    if (retVal==RTIOSTREAM_ERROR) {
        error = EXT_ERROR;
    } else {
        error = EXT_NO_ERROR;
    }

    return error;
} /* end ExtGetTargetPkt */

/* Function: ExtSetTargetPkt ===================================================
 * Abstract:
 *  Sets (sends) the specified number of bytes on the comm line.  As long as
 *  an error does not occur, this function is guaranteed to set the requested
 *  number of bytes.  The number of bytes set is returned via the 'nBytesSet'
 *  parameter.  EXT_NO_ERROR is returned on success, EXT_ERROR is returned on
 *  failure.
 *
 * NOTES:
 *  o it is o.k. for this function to block if no room is available (e.g.,
 *    a send call on a blocking socket)
 */
PUBLIC boolean_T ExtSetTargetPkt(
    const ExternalSim *ES,
    const int         nBytesToSet,
    const char        *src,
    int               *nBytesSet) /* out */
{   
	UserData *userData = (UserData *)esGetUserData(ES);
    boolean_T errorCode = EXT_NO_ERROR;
	int retVal;	
    #if defined (LEGO_NXT)
    const int sendTimeOutSecs = 10;
    #else
    const int sendTimeOutSecs = 120;
    #endif
    int timeoutOccurred = 0;
	
    *nBytesSet = 0;  /* assume */
    retVal = rtIOStreamBlockingSend( &(libH) ,
                                    userData->streamID,
                                    (const uint8_T *) src,
                                    (size_t) nBytesToSet,
                                    sendTimeOutSecs, 
                                    &timeoutOccurred);
    if (retVal) {
        errorCode = EXT_ERROR;
        return errorCode;
    }
    
    *nBytesSet = nBytesToSet;
    return errorCode;

    
} /* end ExtSetTargetPkt */


/* Function: ExtTargetPktPending ===============================================
 * Abstract:
 *  Returns true, via the 'pending' arg, if data is pending on the comm line.
 *  Returns false otherwise.  If the timeout is 0, do simple polling (i.e.,
 *  return immediately).  Otherwise, wait the specified amount of seconds.
 *
 *  EXT_NO_ERROR is returned on success, EXT_ERROR on failure (reaching
 *  a nonzero timeout is considered a failure).
 */
PUBLIC boolean_T ExtTargetPktPending(
    const ExternalSim *ES,
    boolean_T         *pending,
    long int          timeOutSecs,
    long int          timeOutUSecs)
{
#define SLEEP_MSECS 10
    boolean_T retVal = EXT_NO_ERROR;
    int result;
    size_t sizeRecvd=0;
    int num_attempts;
    UserData *userData = (UserData *)esGetUserData(ES);

    num_attempts = (timeOutSecs*1000 + timeOutUSecs/1000)/SLEEP_MSECS;/*lint !e712 
                                                                       * loss of precision
                                                                       * long to int */

    /* Check for pending data at least once */
    if (num_attempts <= 0) {
        num_attempts = 1;
    }

    if (userData->numPending > 0) {
        *pending = 1;
        return retVal;
    } else {
        *pending = 0;
    }

    while (num_attempts>0 && sizeRecvd==0) {
        /* Call rtIOStreamRecv */
        result = ( *(libH.recvFn) ) (userData->streamID,
                                userData->pendingRecvdData,
                                (const size_t)PENDING_DATA_CACHE_SIZE,
                                &sizeRecvd);

        if (result==RTIOSTREAM_ERROR) {
            retVal = EXT_ERROR;
            return retVal;
        }

        if (sizeRecvd==0) {
            SleepMilliseconds(SLEEP_MSECS);
        }
        num_attempts--;
    }
        
    if (sizeRecvd>0) {
        *pending = 1;
        userData->numPending = (int)sizeRecvd;
        userData->startIdxPending = 0;
    }

    return retVal;

} /* end ExtTargetPktPending */

/* Function: ExtCloseConnection ================================================
 * Abstract:
 *  Close the connection with the target.  In some cases, this may be 
 *  trivial (e.g., shared memory, serial cable).  In the case of sockets it
 *  takes a little work.
 *
 * NOTES:
 *  o It is assumed that this function is always successful.
 *  o It is possible that user data will be NULL (due to a shutdown
 *    caused by an error early in the connect process).
 */
PUBLIC void ExtCloseConnection(const ExternalSim *ES)
{
    UserData           *userData = (UserData *)esGetUserData(ES);

    /* Call rtIOStreamClose */
    ( *(libH.closeFn) ) (userData->streamID);

    return;
} /* end ExtCloseConnection */

/* LocalWords:  nanosleep timespec rtiostreamserial rtiostreamtcpip libmw
 * LocalWords:  Process nanosleep timespec rtiostreamserial rtiostreamtcpip
 * LocalWords:  libmw Process TCP recv
 */
