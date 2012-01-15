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
 */
#if !defined _EXECUTE_MENU_ENTRY
#define _EXECUTE_MENU_ENTRY

#include "System.h"
#include "MenuEntry.h"
#include "CDBExtended.h"
#include "FString.h"
#include "LoadableLibrary.h"

class ExecuteMenuEntry;
typedef bool (*Action)(StreamInterface &in,StreamInterface &out,void *userData);


extern "C"{
    bool ExecuteMenuAction(StreamInterface &in,StreamInterface &out,void *userData);
}

OBJECT_DLL(ExecuteMenuEntry)
class ExecuteMenuEntry: public MenuEntry{
private:
    /** Contains the Menu Action */
    friend bool   ExecuteMenuAction(StreamInterface &in,StreamInterface &out,void *userData);

    /** User specified DLL containing the function to be executed*/
    LoadableLibrary      dll;

    /** Pointer to the user specified action */
    Action             action;

public:

    /** Constructor */
    ExecuteMenuEntry(){
        action = NULL;
    };

    /** Destructor */
    virtual ~ExecuteMenuEntry(){};

    virtual     bool                ObjectLoadSetup(
                        ConfigurationDataBase &         info,
                        StreamInterface *               err)
    {

        CDBExtended cdb(info);
        if(!MenuEntry::ObjectLoadSetup(cdb, err)){
            AssertErrorCondition(InitialisationError,"ExecuteMenuEntry::ObjectLoadSetup: MenuEntry ObjectLoadSetup Failed");
            return False;
        }
        
        if(!cdb->Move("Function")){
            AssertErrorCondition(InitialisationError,"ExecuteMenuEntry::ObjectLoadSetup: No Function has been specified");
            return False;
        }

        FString dllName;
        if(!cdb.ReadFString(dllName,"Dll")){
            AssertErrorCondition(InitialisationError,"ExecuteMenuEntry::ObjectLoadSetup: Failed reading Dll Name");
            return False;
        }
        
        FString functionName;
        if(!cdb.ReadFString(functionName,"Function")){
            AssertErrorCondition(InitialisationError,"ExecuteMenuEntry::ObjectLoadSetup: Failed Reading Function Name");
            return False;
        }

        if(!dll.Open(dllName.Buffer())){
            AssertErrorCondition(InitialisationError,"ExecuteMenuEntry::ObjectLoadSetup: Failed opening dll %s", dllName.Buffer());
            return False;
        }

        if((action = (Action)dll.Function(functionName.Buffer())) == NULL){
            AssertErrorCondition(InitialisationError,"ExecuteMenuEntry::ObjectLoadSetup: Failed finding function %s in dll %s", functionName.Buffer(), dllName.Buffer());
            return False;
        }
        
        SetUp(ExecuteMenuAction, NULL, NULL, this);

        return True;
    }



OBJECT_DLL_STUFF(ExecuteMenuEntry)

};

#endif

