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
#ifndef HTTP_SESSION_H
#define HTTP_SESSION_H

/**
 * @file classes which extend from SessionState will be able to track and persist an
 * HTTP session. This usually done by adding it to a Container and for 
 * every ProcessHttpMessage either add it or retrieve it from the container.
 */

#include "GCNamedObject.h"
#include "FString.h"

class HttpSession : public GCNamedObject{
private:
    /**
     * Last time this object was accessed. Must be updated with SessionUpdated()
     */
    int32 lastAccessedTime;
    /**
     * Number of times this session was accessed. Must be updated with SessionUpdated()
     */
    int32 numberOfAccesses;
    /**
     * Absolute time of when session was created
     */
    int32 creationTime;
    /**
     * When set to true the session can be deleted and resources freed
     */
    bool  disposable;
public:

    HttpSession(){
        disposable   = False;
        creationTime = time(NULL);

        FString id;
        id.Printf("%d", HRT::HRTCounter());
        SetObjectName(id.Buffer());
    }

    /**
     * @return the session unique id
     */ 
    const char *GetSessionID(){
        return Name();
    }

    /**
     * @return the creation time as an absolute value since the Epoch in seconds
     */
    int32 GetCreationTime(){
        return creationTime;
    }

    /**
     * @return last time of access, as an absolute value since the Epoch in seconds
     */
    int32 GetLastAccessedTime(){
        return lastAccessedTime;
    }

    /**
     * Must be called by any subclass, or HttpInterface subclass, every time the session is changed
     */
    void SessionUpdated(){
        numberOfAccesses++;
        lastAccessedTime = time(NULL);
    }

    /**
     * Must be called by any subclass, or HttpInterface subclass, when the session is no longer needed
     */
    void SetDisposable(){
        disposable = True;   
    }
    
    /**
     * @true if disposable 
     */
    bool IsDisposable(){
        return disposable;
    }
};

#endif


