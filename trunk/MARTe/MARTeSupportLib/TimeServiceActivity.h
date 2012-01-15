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

#if !defined (_TIME_SERVICE_ACTIVITY)
#define _TIME_SERVICE_ACTIVITY

#include "System.h"
#include "GCReferenceContainer.h"
#include "ConfigurationDataBase.h"

class ExternalTimeTriggeringService;

/** Abstract Time Service Activity Class */
OBJECT_DLL(TimeServiceActivity)
class TimeServiceActivity: public GCReferenceContainer{

OBJECT_DLL_STUFF(TimeServiceActivity)

protected:

    /** Time service activity period in micro seconds */
    int32 aUsecPeriod;

    /** Time service activity phase in micro seconds */
    int32 aUsecPhase;

public:

    /** Constructor */
    TimeServiceActivity(){
        aUsecPeriod = -1;
        aUsecPhase  = -1;
    }

    /** Destructor */
    virtual ~TimeServiceActivity();

    /** Load time service activity parameters */
    bool ObjectLoadSetup(ConfigurationDataBase &info,StreamInterface *err);

    /** Trigger method called by ExternalTimeTriggeringService::Trigger()
        Code in this method is executed only if ((ActualTime % Period) == Phase) */
    virtual bool Trigger(int64 usecTime) = 0;

    /** Start the Time Activity*/
    virtual bool Start(){return True;}

    /** Stop the Time Activity*/
    virtual bool Stop(){return True;}

    /** Get activity period */
    inline int32 GetActivityUsecPeriod(){return aUsecPeriod;}

    /** Get activity phase */
    inline int32 GetActivityUsecPhase(){return aUsecPhase;}

    /** Writes on a Stream all Module's Parameter */
    bool ObjectDescription(StreamInterface &s,bool full=False,StreamInterface *err=NULL);
};

#endif
