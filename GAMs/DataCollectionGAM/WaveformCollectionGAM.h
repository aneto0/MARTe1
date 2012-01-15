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



#if !defined(_WAVEFORM_COLLECTION_GAM_)
#define _WAVEFORM_COLLECTION_GAM_

#include "System.h"
#include "GAM.h"
#include "MessageHandler.h"

#include "GCRTemplate.h"
#include "FString.h"

#include "DDBInputInterface.h"
#include "RTDataCollector.h"

#include "SignalInterface.h"


OBJECT_DLL(WaveformCollectionGAM)


class WaveformCollectionGAM:public GAM, public MessageHandler{

OBJECT_DLL_STUFF(WaveformCollectionGAM)

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

    /** Jpf Data Collection */
    DDBInputInterface                              *jpfData;

    /** Data Collector */
    RTDataCollector                                dataCollector;

private:

    /** State Vector */
    float                                          *inputState;

    /** Differences at the previous step */
    float                                          *differencesState;

    /** Time at the previous step */
    int32                                          lastSampleUsecTime;

    /** State Size */
    int32                                           inputStateByteSize;

    /** Sample number */
    int32                                           sampleNumber;

private:

    /** flag to avoid processing messages during pulse */
    bool                                           acceptingMessages;

    /** Fast Trigger Requirement */
    bool                                           triggerAcquisition;

private:
    /** | x'(T) - x'ref| < maxPropVar |x'ref|+ maxVar; */
    double                                          maxProportionalVariation;

    /** | x'(T) - x'ref| < maxPropVar |x'ref|+ maxVar; */
    double                                          maxAbsoluteVariation;

public:

    // Constructor
    WaveformCollectionGAM();

    // Destructor
    virtual ~WaveformCollectionGAM(){
        if(inputState != NULL) free((void *&)inputState);
        if(differencesState != NULL) free((void *&)differencesState);
    };

    // Initialise the module
    virtual bool Initialise(ConfigurationDataBase& cdbData);

    /** Execute the module functionalities */
    virtual bool Execute(GAM_FunctionNumbers functionNumber);

    /** Implements the Saving of the parameters to Configuration Data Base */
    virtual bool ObjectSaveSetup(ConfigurationDataBase &info, StreamInterface *err)
    {
        return True;
    };
};

#endif /*EVENTCOLLECTIONGAM_H_*/
