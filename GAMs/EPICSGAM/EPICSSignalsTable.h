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



// EPICS Lib include
#include "EPICSHandler.h"


/*
 * DDB Supported Signal types 
 * BTDInt32, BTDInt64 (int32, int64)
 * BTDUint32, BTDUint64 (uint32, uint64)
 * BTDFloat, BTDDouble (float32, float64)
 */



class EPICSSignal :
	public LinkedListable
{
private:
    // Signal name in the DDB
    FString                       ddbSignalName;
    // Signal basic type descriptor in the DDB
    BasicTypeDescriptor           typeDescriptor;
    // Signal number of elements
    int							  numElements;
    // Signal offset from the beginning of the DDB's buffer (byte)
    unsigned	                  dataOffset;

    // Signal name in the external server
    FString                       EPICSSignalName;
    // handle in the external server
    int							  EPICSindex;
    // esternal server required subsample
    int							  EPICSsubsample;
    
public:
	// subsampling public counter
	int							  counter;
	
    // Next
	EPICSSignal *Next () {
    	return (EPICSSignal *)LinkedListable::Next() ;
    	};

    // Constructor
    EPICSSignal (FString &nameIn,  BasicTypeDescriptor descriptorIn, int elementIn, unsigned offsetIn,
    		FString &serverNameIn, int indexIn, int subsampleIn) {
        Initialize(nameIn, descriptorIn, elementIn, offsetIn,
        		serverNameIn, indexIn, subsampleIn);
    };

    // De-constructor
    // no dynamic allocation, so nothing to do
    virtual ~EPICSSignal () {} ;

    // Initialize data information
    inline bool Initialize (FString &nameIn,  BasicTypeDescriptor descriptorIn, int elementIn, unsigned offsetIn,
    		FString &serverNameIn, int indexIn, int subsampleIn)
    {
    	ddbSignalName = nameIn;
	    typeDescriptor = descriptorIn;
	    numElements = elementIn;
	    dataOffset = offsetIn;
	    
	    EPICSSignalName = serverNameIn;
	    EPICSindex = indexIn;
	    EPICSsubsample = subsampleIn;
	    
	    counter = 0;
	    return True;
    }

    // Get copy of the DDB Name
    inline const char * GetDDBName() const {
    	return ddbSignalName.Buffer();
    	}
    // type
    inline BasicTypeDescriptor GetDDBType () const {
    	return typeDescriptor;
    	}
    // whole signal data size
    inline int GetDDBSize() {
    	return ( numElements * typeDescriptor.ByteSize() );
    	}
    // Offset
    inline unsigned GetDDBOffset() const {
    	return dataOffset;
    	}

    // Get copy of the Server Name
    inline const char * GetEPICSName() const {
    	return EPICSSignalName.Buffer();
    	}
    // Get copy of the EPICS index
    inline const int GetEPICSIndex() const {
    	return EPICSindex;
    	}
    // Get copy of the EPICS subsample factor
    inline const int GetEPICSSubSample() const {
    	return EPICSsubsample;
    	}
}; //-------------------------------------------------------------------------- class EPICSSignal



class EPICSSignalsTable :
	public LinkedListHolder
{
	EPICSSignal ** signalsTable;
	GCRTemplate<EPICSHandler> gcrEPICSHnd;
		
public:

    /** Constructor */
    EPICSSignalsTable () {
    	signalsTable = 0;
    };

    /** Destructor */
    virtual ~EPICSSignalsTable () {
    	
    	for (int i=0; i<this->ListSize(); i++)
    		gcrEPICSHnd->unsubscribe( signalsTable[i]->GetEPICSIndex() );
    	
       	// TODO
       	//  delete all the elements in the linked list
       	//  it is done automatically by Filippo's code?
    		
    	if ( signalsTable )
    		delete [] signalsTable;
    	
    	if ( gcrEPICSHnd.IsValid() )
    		gcrEPICSHnd.RemoveReference();
    };

    /** return EPICSSignal table */
    inline EPICSSignal ** GetTable () {
    	return signalsTable;
    };
    
    /** Update all signals in the list */
    bool UpdateSignals (char * buffer, int32 timestamp);
    
    /** Initialize the signal table */
    bool Initialize(const DDBInterface &interfacex, ConfigurationDataBase &info);

    /** Object Description. Saves information on a StreamInterface. The information
        is used by the DataCollectionGAM to process the GAP message LISTSIGNALS. */
    virtual bool ObjectDescription(StreamInterface &s,bool full=False,StreamInterface *err=NULL);

    /** Find the offset for the signal */
//    int32 FindOffsetAndInitSignalType(const FString &signalName, GCRTemplate<SignalInterface> signal, int32 nOfSamples);
    
}; //-------------------------------------------------------------------------- class EPICSSignalsTable

#endif
