/*
 * Copyright 2011 EFDA | European Fusion Development Agreement
 *
 * Licensed under the EUPL, Version 1.1 or - as soon they 
   will be approved by the European Commission - subsequent  
   versions of the EUPL (the "Licence"); 
 * You may not use this work except in compliance with the 
   Licence. 
 * You may obtain a copy of the Licence at: 
 *  
 * http://ec.europa.eu/idabc/eupl
 *
 * Unless required by applicable law or agreed to in 
   writing, software distributed under the Licence is 
   distributed on an "AS IS" basis, 
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either 
   express or implied. 
 * See the Licence for the specific language governing 
   permissions and limitations under the Licence. 
 *
 * $Id$
 *
**/

/** 
 * @file
 * @brief Processes and handles all the logging messages.
 */
#if !defined (_LOGGERSERVICE_H)
#define _LOGGERSERVICE_H

#include "System.h"
#include "FString.h"

class ErrorSystemInfo;
class CStream;


extern "C" {

    /** */
    bool GetLogStream(CStream *cs);

    /** */
    bool EndLogStream(CStream *cs);

    /** starts the logger service thread */
    void LSStartService();

    /** stops the logger service thread */
    void LSStopService();

    /** cleans the backlog of errors */
    void LSProcessPendingErrors();

//    /** Sets the destination of error messages sent by defauly BaseLib error message processor */
//    bool LSSetRemoteLogger(const char *serverName,int port);

    /** */
    bool LSLastError(ErrorSystemInfo &info,int32 index);

    /** the function that handles the errors */
    void LSAssembleErrorMessage(const char *errorDescription,va_list argList,const char *errorHeader,...);

}

static inline bool LSSetRemoteLogger(const char *serverName,int port){
    FString url;
    url.Printf("http://%s:%i",serverName,port);
    SetLoggerServerURL(url.Buffer());
    return True;
}

#endif

