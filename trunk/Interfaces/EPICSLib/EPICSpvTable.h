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
#if !defined(_EPICS_SIGNALS_TABLE)
#define _EPICS_SIGNALS_TABLE

#include "System.h"
#include "LinkedListable.h"

#include "FString.h"
#include "BasicTypes.h"

#include "GCRTemplate.h"

class  DDBInterface;
class  SignalInterface;


class EPICSSignal: public LinkedListable{

private:

    /**  SignalName in the DDB */
    FString                       ddbSignalName;
 //   int ddbSignalSize;

    /**  SignalName in the JPF database */
    FString                       EPICSSignalName;
//    int epicsSignalSize;
    

    /** SignalType Descriptor DDB*/
    BasicTypeDescriptor           typeDescriptor;
    
    /** Singal type descriptor EPICS?!?!? */
    
    

    /** Offset From Beginning of the buffer */
    uint32                        dataOffset;

    /** Cal 0. To be used by the GAP interface for integer signals */
    float                         cal0;

    /** Cal 1. To be used by the GAP interface for integer signals */
    float                         cal1;
    
    
    int data_size;

    /** just for the prototype we allocate a buffer where to store the previous value */
    char* data_buffer;
    
public:

    /** Next */
    EPICSSignal *Next(){ return (EPICSSignal *)LinkedListable::Next();}


    /** Constructor  */
    EPICSSignal():dataOffset(0){
        cal0 = 0.0;
        cal1 = 1.0;
    };

    /** Constructor  */
    EPICSSignal(FString &ddbName, FString &jpfName, uint32 offset, BasicTypeDescriptor  typeDescriptor){
        Initialize(ddbName, jpfName, offset, typeDescriptor);
    };

    /** Destructor */
    virtual ~EPICSSignal(){};

    /** Initialize data information */
    bool Initialize(const FString &ddbName, const FString &jpfName, uint32 offset, BasicTypeDescriptor  typeDescriptor);

    /** Get copy of the Name*/
    const char * DDBName()  const {return ddbSignalName.Buffer();}

    /** Get copy of the Name*/
    const char * EPICSName()  const {return EPICSSignalName.Buffer();}

    /** Offset */
    uint32     Offset() const {return dataOffset;}

    char* DataBuffer() {return data_buffer;}
    int DataSize() {return data_size;}
    
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



class EPICSSignalsTable: public LinkedListHolder{

public:

    /** Constructor */
    EPICSSignalsTable () {};

    /** Destructor */
    virtual ~EPICSSignalsTable () {};

    /** Initialize the signal table */
    bool Initialize(const DDBInterface &interfacex, ConfigurationDataBase &info);

    /** Find the offset for the signal */
    int32 FindOffsetAndInitSignalType(const FString &signalName, GCRTemplate<SignalInterface> signal, int32 nOfSamples);

    /** Object Description. Saves information on a StreamInterface. The information
        is used by the DataCollectionGAM to process the GAP message LISTSIGNALS. */
    virtual bool ObjectDescription(StreamInterface &s,bool full=False,StreamInterface *err=NULL);

};

#endif
