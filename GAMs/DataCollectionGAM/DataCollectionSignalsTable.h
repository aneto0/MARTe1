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


#if !defined(_DATA_COLLECTION_SIGNALS_TABLE_)
#define _DATA_COLLECTION_SIGNALS_TABLE_

#include "System.h"
#include "LinkedListable.h"

#include "FString.h"
#include "BasicTypes.h"

#include "GCRTemplate.h"

class  DDBInterface;
class  SignalInterface;

class DataCollectionSignal: public LinkedListable{

private:

    /**  SignalName in the DDB */
    FString                       ddbSignalName;

    /**  SignalName in the JPF database */
    FString                       jpfSignalName;

    /** SignalType Descriptor */
    BasicTypeDescriptor           typeDescriptor;

    /** Offset From Beginning of the buffer */
    uint32                        dataOffset;

    /** Cal 0. To be used by the GAP interface for integer signals */
    float                         cal0;

    /** Cal 1. To be used by the GAP interface for integer signals */
    float                         cal1;

public:

    /** Next */
    DataCollectionSignal *Next(){ return (DataCollectionSignal *)LinkedListable::Next();}


    /** Constructor  */
    DataCollectionSignal():dataOffset(0){
        cal0 = 0.0;
        cal1 = 1.0;
    };

    /** Constructor  */
    DataCollectionSignal(FString &ddbName, FString &jpfName, uint32 offset, BasicTypeDescriptor  typeDescriptor){
        Initialize(ddbName, jpfName, offset, typeDescriptor);
    };

    /** Destructor */
    virtual ~DataCollectionSignal(){};

    /** Initialize data information */
    bool Initialize(const FString &ddbName, const FString &jpfName, uint32 offset, BasicTypeDescriptor  typeDescriptor);

    /////////////////
    //             //
    /////////////////

    /** Get copy of the Name*/
    const char *DDBName()  const {return ddbSignalName.Buffer();}

    /** Get copy of the Name*/
    const char *JPFName()  const {return jpfSignalName.Buffer();}

    /** Offset */
    uint32     Offset() const {return dataOffset;}

    /** Type */
    BasicTypeDescriptor   Type()const {return typeDescriptor;}

    /** Set Calibration factors*/
    bool                  SetCalibrations(float cal0, float cal1){
        this->cal0 = cal0;
        this->cal1 = cal1;
        return True;
    }

    /** Returns the cal0 factor */
    float                Cal0(){return cal0;}

    /** Returns the cal1 factor */
    float                Cal1(){return cal1;}
};

class DataCollectionSignalsTable: public LinkedListHolder{
private:
  
    // Check if, when calling CopyData() in Level5/Signal.cpp through FindOffsetAndInitSignalType(), standard (lower) or extra (upper) memory allocation is required
    MemoryAllocationFlags           useUpperMemory2CopySignal;

public:
    //Force the signal names to be in upper case? (default = True)
    bool                            forceSignalNamesUpperCase;

    /** Constructor */
    DataCollectionSignalsTable(){
        forceSignalNamesUpperCase = True;
    }

    /** Destructor */
    virtual ~DataCollectionSignalsTable(){};

    /** Initialize the signal table */
    bool Initialize(const DDBInterface &interfacex, ConfigurationDataBase &info);

    /** Find the offset for the signal */
    int32 FindOffsetAndInitSignalType(const FString &signalName, GCRTemplate<SignalInterface> signal, int32 nOfSamples);

    /** Object Description. Saves information on a StreamInterface. The information
        is used by the DataCollectionGAM to process the GAP message LISTSIGNALS. */
    virtual bool ObjectDescription(StreamInterface &s,bool full=False,StreamInterface *err=NULL);
    
    /** */
    bool SetUseUpperMemory2CopySignal(MemoryAllocationFlags allocFlags = MEMORYStandardMemory){
        if((allocFlags != MEMORYStandardMemory) && (allocFlags != MEMORYExtraMemory)){
	    CStaticAssertErrorCondition(Warning, "DataCollectionSignalsTable: SetUseUpperMemory2CopySignal: Trying to set useUpperMemory2CopySignal = 0x%x -> Invalid! Valid values are 0x0 and 0x1.", (int32)useUpperMemory2CopySignal);
	    return False;
	}else{
	    useUpperMemory2CopySignal = allocFlags;
	}

	return True;
    }
    
    /** */
    MemoryAllocationFlags GetUseUpperMemory2CopySignal(){return useUpperMemory2CopySignal;}
};


#endif
