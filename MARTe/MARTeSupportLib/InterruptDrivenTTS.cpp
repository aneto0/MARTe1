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

#include "GenericAcqModule.h"
#include "InterruptDrivenTTS.h"

bool   InterruptDrivenTTS::ObjectLoadSetup(ConfigurationDataBase &info,StreamInterface *err){
    bool ret = True; 
    ret = TimeTriggeringServiceInterface::ObjectLoadSetup(info, err);
    //timeOut = 10*tsOfflineUsecPeriod/1000;
    if(tsTimeOutMsecTime == -1) {
      AssertErrorCondition(Warning, "InterruptDrivenTTS assuming default TsTimeOutMsecTime = 10*tsOfflineUsecPeriod/1000");
      timeOut = 10*tsOfflineUsecPeriod/1000;
    } else {
      timeOut = tsTimeOutMsecTime;
    }
    return ret;
}

bool   InterruptDrivenTTS::ObjectDescription(StreamInterface &s,bool full,StreamInterface *err){
    return TimeTriggeringServiceInterface::ObjectDescription(s, full, err);
}


OBJECTLOADREGISTER(InterruptDrivenTTS,"$Id: InterruptDrivenTTS.cpp,v 1.5 2009/07/22 14:25:23 ppcc_dev Exp $")
