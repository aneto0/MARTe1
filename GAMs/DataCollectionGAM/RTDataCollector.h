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

#if !defined (_RTDATACOLLECTOR)
#define _RTDATACOLLECTOR

#include "System.h"
#include "GCNamedObject.h"

#include "RTDelaySystem.h"
#include "RTDataStorageSystem.h"
#include "RTDataPool.h"
#include "DataCollectionSignalsTable.h"
#include "SignalInterface.h"
#include "BString.h"
#include "HttpStream.h"

extern "C"{
    bool ModifyTimeBase(StreamInterface &in,StreamInterface &out,void *userData);
}


OBJECT_DLL(RTDataCollector)

/// RT Data Collector System
class RTDataCollector: public GCNamedObject,public LinkedListable{
    OBJECT_DLL_STUFF(RTDataCollector)

    friend  bool ModifyTimeBase(StreamInterface &in,StreamInterface &out,void *userData);

private:

    /** The container of buffers available for use */
    RTDataPool                        freeDataBuffersPool;

    /** The data collected in a pulse */
    RTDataStorageSystem               dataStorage;

    /** A storage to delay the acquisition */
    RTDelaySystem                     dataCollectionDelaySystem;

    /** Number of channels to copy from the RTDataBuffer */
    int32                             nOfChannels;

    /** SignalTable Database */
    DataCollectionSignalsTable        signalTable;

private:

    /** Memory deallocation and list cleaning */
    void CleanUp();

    /*******************************************************************************************************
    /*
    /* Avoid the user from making copies of the RTDataCollector and forgetting to handle the memory allocation
    /*
     ********************************************************************************************************/

    /** Copy constructors (since it is defined private it won't allow a public use!!) */
    RTDataCollector(const RTDataCollector&){};

    /** Operator=  (since it is defined private it won't allow a public use!!) */
    RTDataCollector& operator=(const RTDataCollector&){};

    /** Returns nOfChannels */
    int32 NOfChannels(){ return nOfChannels; }

public:

    /** */
    RTDataCollector():nOfChannels(0){};

    /** */
    ~RTDataCollector(){CleanUp();}

    /** */
    bool ObjectLoadSetup(ConfigurationDataBase &info,StreamInterface *err, const DDBInterface *ddbInterface);

    /** */
    bool ObjectSaveSetup(ConfigurationDataBase &info,StreamInterface *err=NULL);

    /** */
    bool ObjectDescription(StreamInterface &s,bool full=False,StreamInterface *err=NULL);

    /** Stores Data.
        Try to store the input RT Data Buffer.
        Returns rtBuffer if an error happens. */
    bool StoreData(const uint32 *ddbInterfaceBuffer,uint32 usecTime, bool fastTrigger = False);

    /** Flush the delay system and prepares the system for data retrievel */
    bool CompleteDataCollection();

    /** Rebuild list of buffers and clean all other lists */
    bool PrepareForNextPulse();

    /** Get SignalData */
    GCRTemplate<SignalInterface>  GetSignalData(const FString &jpfSignalName){

        // Create the signal
        GCRTemplate<SignalInterface>  signal("Signal");
        if(signal.IsValid()){
            int signalOffset = signalTable.FindOffsetAndInitSignalType(jpfSignalName, signal, dataStorage.Size());
            if((signalOffset < 0)||(signalOffset > nOfChannels)){
                // Remove the reference to invalidate the signal
                signal.RemoveReference();
                AssertErrorCondition(FatalError,"RTDataCollector::GetSignalData: Signal Offset of of boundary [0-%d]: %d",nOfChannels, signalOffset);
                return signal;
            }

            dataStorage.GetSignalData(signalOffset, signal);

            /* Name the Signal */
            GCRTemplate<GCNamedObject>    namedSignal = signal;
            if(!namedSignal.IsValid()){
                // Remove the reference to invalidate the signal
                signal.RemoveReference();
                AssertErrorCondition(FatalError,"RTDataCollector::GetSignalData: Signal is not of GCNamedObject Type");
                return signal;
            }

            namedSignal->SetObjectName(jpfSignalName.Buffer());
        }
        return signal;
    }

public:

    void TimeWindowsMenu(StreamInterface &in, StreamInterface &out){
        dataStorage.TimeWindowsMenu(in,out);
    }

    void HTMLInfo(HttpStream &hStream);
};

#endif
