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


#if !defined(_DATA_COLLECTION_GAM_)
#define _DATA_COLLECTION_GAM_

#include "System.h"
#include "GAM.h"
#include "MessageHandler.h"

#include "GCRTemplate.h"
#include "FString.h"

#include "DDBInputInterface.h"
#include "DDBIOInterface.h"
#include "RTDataCollector.h"

#include "SignalInterface.h"

#include "HttpInterface.h"

OBJECT_DLL(DataCollectionGAM)


class DataCollectionGAM:public GAM, public MessageHandler, public HttpInterface{

OBJECT_DLL_STUFF(DataCollectionGAM)

private:

    ///////////////////////////////
    // MESSAGE HANDLER INTERFACE //
    ///////////////////////////////

    virtual bool                                   ProcessMessage(GCRTemplate<MessageEnvelope> envelope);

    /** Get Data Collected during the pulse */
    GCRTemplate<SignalInterface>                   GetSignal(const FString &jpfSignalName);

private:

    /** DDB Interface for the Time */
    DDBInputInterface                              *usecTime;

    /** Fast Trigger Request signal. */
    DDBIOInterface                                 *fastTrigger;

    /** Jpf Data Collection */
    DDBInputInterface                              *jpfData;

private:

    /** Has trigger signal */
    bool                                           hasTriggerSignal;

    /** Data Collector */
    RTDataCollector                                dataCollector;

private:

    /** flag to avoid processing messages during pulse */
    bool                                           acceptingMessages;

public:

    // Constructor
    DataCollectionGAM();

    // Destructor
    virtual ~DataCollectionGAM(){};

    // Initialise the module
    virtual bool Initialise(ConfigurationDataBase& cdbData);

    /** Execute the module functionalities */
    virtual bool Execute(GAM_FunctionNumbers functionNumber);

    /** Implements the Saving of the parameters to Configuration Data Base */
    virtual bool ObjectSaveSetup(ConfigurationDataBase &info, StreamInterface *err)
    {
        return True;
    };

    /** Menu Interface */
    virtual bool MenuInterface(StreamInterface &in,StreamInterface &out,void *userData){
        return dataCollector.ObjectDescription(in, True);
    }

    /**
    * Builds the webpage.
    * @param hStream The HttpStream to write to.
    * @return False on error, True otherwise.
    */
    virtual bool ProcessHttpMessage(HttpStream &hStream);
};

#endif
