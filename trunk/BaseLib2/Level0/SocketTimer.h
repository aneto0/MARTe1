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
 * Enables to specify a maximum wait on socket actions 
 */
#if !defined SocketTimer_
#define SocketTimer_

#include "SocketSelect.h"

/** to simplify some use of select */
class  SocketTimer:public SocketSelect {

    /** the precalculated interval */
    timeval timeWait;

public:
    /** is infinite valid here ? */
    void SetMsecTimeout(TimeoutType msecTimeout = TTInfiniteWait){
        if (msecTimeout != TTInfiniteWait){
            timeWait.tv_sec  = msecTimeout.msecTimeout / 1000;
            timeWait.tv_usec = 1000 * (msecTimeout.msecTimeout - (timeWait.tv_sec * 1000));
        } else {
            timeWait.tv_sec = 0x7FFFFFFF;
            timeWait.tv_usec = 0;
        }
    }
    /** wait for data to read */
    bool WaitRead(){
        readFDS_done   = readFDS;
        readySockets = select(256,&readFDS_done,NULL,NULL,&timeWait);
        return (readySockets > 0);
    }
    /** wait for space to write*/
    bool WaitWrite(){
        writeFDS_done   = readFDS;
        readySockets = select(256,NULL,&writeFDS_done,NULL,&timeWait);
        return (readySockets > 0);
    }

    /** calculates Timeout from the the timeWait structure*/
    uint32 MSecTimeout(){
        uint32 mSecTimeout = timeWait.tv_sec * 1000;
        mSecTimeout += timeWait.tv_usec / 1000 ;
        return mSecTimeout;
    }
};

#endif

