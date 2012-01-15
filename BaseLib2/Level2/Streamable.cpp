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

#include "System.h"
#include "Streamable.h"
#include "Object.h"
#include "HRT.h"
#include "Sleep.h"

OBJECTREGISTER(Streamable,"$Id$")

#if defined(_VXWORKS)

/** will write any text using printf format
    {BUFFERED if buffering activated} */
bool Streamable::VPrintf(const char *format,va_list argList){
    if (csbOut == NULL){
        char stackBuffer[StreamableBufferingSize];
        CStreamBuffering csb(this,stackBuffer,sizeof(stackBuffer));
        return VCPrintf(csb.UseAsOutput(),format,argList);
    }
    return VCPrintf(csbOut->UseAsOutput(),format,argList);
}



#endif

bool StreamCompleteRead(Streamable &s,void* buffer, uint32 &size, TimeoutType msecTimeout, int32 pollMs){
    int64 maxTime = 0;
    if (msecTimeout.IsFinite()){
        maxTime = HRTRead64();
        maxTime += msecTimeout.HRTTicks();
    }
    char *pb = (char *)buffer;
    uint32 left = size;
    uint32 sz = left;
    if (!s.Read(pb,sz)) {
        return False;
    }
    if (sz == 0) {
        size -= left;
        return False;
    }
    left   -= sz;
    pb     += sz;
    while(left > 0){
        if (msecTimeout.IsFinite()){
            int64 dT = maxTime - HRTRead64();
            if (dT < 0){
                size -= left;
                return False;
            }
        }
        if (pollMs > 0){
            SleepMsec(pollMs);
        }
        sz = left;
        if (!s.Read(pb,sz)) return False;
        if (sz == 0) {
            size -= left;
            return False;
        }
        left   -= sz;
        pb     += sz;
    }
    return True;
}

bool StreamCompleteWrite(Streamable &s,const void* buffer, uint32 &size, TimeoutType msecTimeout, int32 pollMs){
    int64 maxTime = 0;
    if (msecTimeout.IsFinite()){
        maxTime = HRTRead64();
        maxTime += msecTimeout.HRTTicks();
    }
    char *pb = (char *)buffer;
    uint32 left = size;
    uint32 sz = left;
    if (!s.Write(pb,sz)) return False;
    if (sz == 0) {
        size -= left;
        return False;
    }
    left   -= sz;
    pb     += sz;
    while (left > 0){
        if (msecTimeout.IsFinite()){
            int64 dT = maxTime - HRTRead64();
            if (dT<0){
                size -= left;
                return False;
            }

        }
        if (pollMs > 0) SleepMsec(pollMs);
        sz = left;
        if (!s.Write(pb,sz)) return False;
        if (sz == 0) {
            size -= left;
            return False;
        }
        left   -= sz;
        pb     += sz;
    }
    return True;
}


