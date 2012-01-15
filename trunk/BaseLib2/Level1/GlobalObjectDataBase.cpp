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

#include "GlobalObjectDataBase.h"
#include "FastPollingMutexSem.h"

class GlobalObjectDataBase: public GCReferenceContainer{

public:

    /* not NULL will enable display of object statistics at class destruction and creation*/
    bool enableDebugging;

    GlobalObjectDataBase(){
        enableDebugging = False;
    }

    virtual ~GlobalObjectDataBase(){
        if (enableDebugging){
            DisplayRegisteredClasses(NULL,True);
        }
    }

};

static FastPollingMutexSem godbInitMux;
GCRTemplate<GlobalObjectDataBase> globalObjectDataBase;

// performs the job of destroying the database
// note that the global destructor is called in a single thread environment
// during the final winding down of an application
// for this reason there is no need to check for the database being in use
// that is the hope at least....
static class GODBDestructor{

public:
    ~GODBDestructor(){
        if (globalObjectDataBase.IsValid()){
	    printf("Potentially Destroying globalObjectDataBase \n");
            globalObjectDataBase.RemoveReference();
        }
    }

} godbDestructor;


void _GetGlobalObjectDataBase(GCRTemplate<GCReferenceContainer> &GODB){
    // if not created first get gglobal lock
    // then check again in case some other thread did the job already
    // if not created still then do the creation
    if (!globalObjectDataBase.IsValid()){
        godbInitMux.FastLock();
        if (!globalObjectDataBase.IsValid()){
            GCRTemplate<GlobalObjectDataBase> GODBTemp(GCFT_Create);
            globalObjectDataBase = GODBTemp;
        }
        godbInitMux.FastUnLock();
    }
    GODB = globalObjectDataBase;
    return ;
}

void GlobalObjectDataBaseEnableDebugging(bool enable){
    GetGlobalObjectDataBase();
    if (globalObjectDataBase.IsValid()){
        globalObjectDataBase->enableDebugging = enable;
    }
}

