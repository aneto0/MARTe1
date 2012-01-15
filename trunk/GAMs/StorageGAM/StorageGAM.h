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
#if !defined(_STORAGE_GAM_)
#define _STORAGE_GAM_

#include "GAM.h"
#include "DDBInputInterface.h"
#include "EventTrigger.h"
#include "Signal.h"
#include "GCNString.h"

OBJECT_DLL(StorageGAM)
class StorageGAM : public GAM, public MessageHandler, public HttpInterface {
OBJECT_DLL_STUFF(StorageGAM)

private:

    /// Array of references to EventTrigger objecte
    GCRTemplate<EventTrigger>               *eventTriggerObject;

    /// Total number of EventTrigger objects
    uint32                                   numberOfEventTriggerObjects;

    /// Array of DDBInputInterfaces
    DDBInputInterface                      **input;

    /// DDBInputInterface for usecTime signal
    DDBInputInterface                       *usecTimeInput;
    
    /// Total number of signals to store
    uint32                                   numberOfSignals2Store;

    /// Array of pointers to all the partial signal info container objects
    SignalInfoContainer                    **signalInfoContainer;

    /// Total number of partial signal info container objects
    uint32                                   numberOfSignalInfoContainers;

    /// The master signal info container object
    SignalInfoContainer                      master;

public:

    /// Flag to control the dealing with external messages
    bool                                     acceptingMessages;

    /// Sem used to control the Discombobulation thread
    EventSem                                 StartStopSem;

public:

    /// Constructor
    StorageGAM() {
        StartStopSem.Create();
        eventTriggerObject  = NULL;
        input               = NULL;
        usecTimeInput       = NULL;
        signalInfoContainer = NULL;

        acceptingMessages = False;
    }

    /// Destructor
    ~StorageGAM() {
        StartStopSem.Close();
        if(eventTriggerObject != NULL) {
            delete [] eventTriggerObject;
            eventTriggerObject = NULL;
        }
        if(input != NULL) {
            free((void*&)input);
            input = NULL;
        }
        if(signalInfoContainer != NULL) {
            free((void*&)signalInfoContainer);
            signalInfoContainer = NULL;
        }
    }
    
    /// Initialise the module
    bool Initialise(ConfigurationDataBase& cdbData);

    /** Execute the module functionalities */
    bool Execute(GAM_FunctionNumbers functionNumber);

    /** Retrieve signal */
    bool RetrieveSignal(const FString &sigName, GCRTemplate<Signal> &sig, MemoryAllocationFlags allocFlags = MEMORYStandardMemory);

    /// Discombobulate data from partial data containers to the master
    bool Discombobulate();

    /// Process message
    bool ProcessMessage(GCRTemplate<MessageEnvelope> envelope);

    /// Process Http message
    bool ProcessHttpMessage(HttpStream &hStream);
};

#endif
