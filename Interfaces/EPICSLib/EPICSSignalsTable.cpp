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
#include "EPICSSignalsTable.h"
#include "GlobalObjectDataBase.h"
#include "GCRTemplate.h"
#include "DDBInterface.h"
#include "CDBExtended.h"
#include "SignalInterface.h"

#define DEFAULT_SERVER_SUBSAMPLING 1000

// we do not need to sort the list in any order
// so we remove it (at least for now)

/*
class AlphaSorterClass : public SortFilter {

public:

    virtual ~AlphaSorterClass(){};

    virtual int32 Compare(LinkedListable *data1,LinkedListable *data2){
        const EPICSSignal *p1 = (const EPICSSignal *)data1;
        const EPICSSignal *p2 = (const EPICSSignal *)data2;
        if (p2 == NULL) return 1;
        if (p1 == NULL) return -1;

        return strcmp(p2->EPICSName(),p1->EPICSName());
    }

} AlphaSorter;
*/

/*
 * Initialize has to link signals in the DDB with
 * EPICS descriptors (sending messages ?)
 * EPICS is message based so doesn't care
 */
bool EPICSSignalsTable::Initialize (const DDBInterface &ddbInterface, ConfigurationDataBase &info) {
    // CleanUp
    CleanUp();

    // check the number of signals
    int32 nOfSignals = ddbInterface.NumberOfEntries();
    if(nOfSignals == 0)
    	return False;
 
    CDBExtended cdb(info);
    unsigned dataOffset = 0;
    int i = 0;
   
    // Find the Signal Server object
    FString SignalsServer;
    if (!cdb.ReadFString(SignalsServer,"SignalsServer")) {
        CStaticAssertErrorCondition(FatalError,
        		"EPICSSignalsTable::Initialize: SignalsServer not defined in the configuration"
        		);
        return False;
    }
    
	GCReference gc = GODBFindByName( SignalsServer.Buffer() );
	if ( !gc.IsValid() ) {
		CStaticAssertErrorCondition(FatalError,
				"EPICSSignalsTable::Initialize cannot find %s in the GlobalObjectDataBase",
				SignalsServer.Buffer() );
		return False;
	} 
        
    // GCRTemplate<PubSubInterface> epics_hnd = gc; 
	// TODO more generic
	gcrEPICSHnd = gc;
	if ( !gcrEPICSHnd.IsValid() ) {
		CStaticAssertErrorCondition(FatalError,
				"EPICSSignalsTable::Initialize cannot cast %s as an EPICSHandler",
				SignalsServer.Buffer() );
		return False;
	}         
	//------------------------------------------------------------------------- end server initialization

    // ddbInterface signal list
    const DDBSignalDescriptor *descriptor = ddbInterface.SignalsList();
	
    // searching for signals
    if(!cdb->Move("Signals")){
            CStaticAssertErrorCondition(FatalError,
            		"EPICSSignalsTable::Initialise: GAM did not specify Signals entry"
            		);
            return False;
    }    
        
    if ( signalsTable )
    	delete [] signalsTable;
    signalsTable = new EPICSSignal * [nOfSignals];
    
    for(i = 0; ((i < nOfSignals)&&(descriptor != NULL)); i++) {
    	// move to children
        cdb->MoveToChildren(i);

        // retrive descriptor description
        FString ddbName = descriptor->SignalName();
        uint32 ddbSize  = descriptor->SignalSize();
        BasicTypeDescriptor ddbDesc = descriptor->SignalTypeCode();
        
        // Get the server signal name
        FString EPICSSignalName;
        if ( !cdb.ReadFString(EPICSSignalName,"ServerName") ) {
            CStaticAssertErrorCondition(FatalError,
            		"EPICSSignalsTable::Initialize: ServerName not specified for DDB signal %s",
            		ddbName.Buffer());
            return False;
        }
        // Get the server signal subsampling
        int32 EPICSSignalSubSample;
        if ( !cdb.ReadInt32(EPICSSignalSubSample,"ServerSubSampling", DEFAULT_SERVER_SUBSAMPLING) ) {
            CStaticAssertErrorCondition(Information,
            		"EPICSSignalsTable::Initialize: ServerSubSampling not specified for signal %s assuming %d",
            		ddbName.Buffer(), EPICSSignalSubSample);
        } //------------------------------------------------------------------- end fetching configuration data
        
        
        // subscribe for service in the server
        unsigned EPICSId = -1;
        if ( !(gcrEPICSHnd->subscribe(EPICSSignalName.Buffer(), ddbDesc, ddbSize, EPICSId)) ) {
            CStaticAssertErrorCondition(FatalError,
            		"EPICSSignalsTable::Initialize: %s subscribe fails for signal %s (server's signal name %s)",
            		SignalsServer.Buffer(), ddbName.Buffer(), EPICSSignalName.Buffer() );
            return False;
        }
        
        // NOTA differently from the JpfSignalTable we are not interested in the calibration data (if needed to be implemented)
        // in the previous version (jpf) if you have an array of signals in MARTe than each signals
        // will be mapped to a different jpf signal and also EPICSSignal, in this implementation
        // EPICS (and also MDSplus) support array of data so we also need to support array of data
        // now it is possible to create the Server's signal
        
        EPICSSignal * newSignal = new EPICSSignal(
        		ddbName, ddbDesc, ddbSize, dataOffset,
        		EPICSSignalName, EPICSId, EPICSSignalSubSample);
        if (newSignal == NULL) {
            CStaticAssertErrorCondition(FatalError,
            		"EPICSSignalsTable::Initialize: Failed allocating space for signal %s (%s)",
            		ddbName.Buffer(), EPICSSignalName.Buffer());
            return False;
        }
        else
        	CStaticAssertErrorCondition(Information,
        		"EPICSSignalsTable::Initialize: Added Signal %s (EPICS: %s)",
        		ddbName.Buffer(), EPICSSignalName.Buffer());
    

        //ListInsert(newSignal,&AlphaSorter); //previous implementation
        ListInsert(newSignal);
        // copy the pointer in the table
        signalsTable[i] = newSignal;
        
        // Increment offset
        dataOffset += newSignal->GetDDBSize();

        // return to parent
        cdb->MoveToFather();
        // move to another signal
        descriptor = descriptor->Next();
    }
    // end "Signals" configuration
    cdb->MoveToFather();
    
    if( (ListSize() == 0) || (List() == NULL) ) {
        CStaticAssertErrorCondition(FatalError,
        		"EPICSSignalsTable::Initialize: No signal added to the list"
        		);
        return False;
    }

    return True;
} //--------------------------------------------------------------------------- ObjectLoadSetup

/* UpdateSignals
 * Updates all signals in the list pushing an event message on the queue
 */
bool EPICSSignalsTable::UpdateSignals (char * buffer, int32 timestamp) {
	int _ret;
	
	int nOfSignals = this->ListSize(); // get the number of signals from the list
	for (int i=0; i<nOfSignals; i++) {
		// update the signal on the server only if needed 
		if ( !(signalsTable[i]->counter % signalsTable[i]->GetEPICSSubSample()) ) {
			_ret = gcrEPICSHnd->put( signalsTable[i]->GetEPICSIndex(),
					(buffer + signalsTable[i]->GetDDBOffset()), (unsigned) timestamp );
			if ( _ret == 0 )
		        CStaticAssertErrorCondition(FatalError,
		        		"EPICSSignalsTable::UpdateSignals: ->put return CIRCULAR BUFFER OVERFLOW"
		        		);
		}
		
		signalsTable[i]->counter++;
	}
			
	return True;
} //--------------------------------------------------------------------------- UpdateSignals


// not required
/*
int32 EPICSSignalsTable::FindOffsetAndInitSignalType(const FString &signalName, GCRTemplate<SignalInterface> signal, int32 nOfSamples) {

    EPICSSignal *sig = (EPICSSignal *)List(); 
    for(int i = 0; i < ListSize(); i++){
        if(strcmp(sig->EPICSName(),signalName.Buffer()) == 0){
            signal->CopyData(sig->Type(), nOfSamples);
            return sig->Offset();
        }
        sig = (EPICSSignal *)sig->Next();
    }
    
    return -1;
}
*/


bool EPICSSignalsTable::ObjectDescription(StreamInterface &s,bool full,StreamInterface *err) {

    EPICSSignal *sig = (EPICSSignal *)List();
    if(sig == NULL) return False;

    s.Printf("Signals = {\n");
    for(int i = 0; i < this->ListSize(); i++){

        FString bType;
        FString oType;

        s.Printf("Signal%d = {\n",i);
        sig->GetDDBType().ConvertToString(bType);
        s.Printf("    Name = \"%s\" \n"  , sig->GetEPICSName());
        s.Printf("    DataType = %s \n"  , bType.Buffer());
        //	s.Printf("    Cal0 = %f \n", sortedSignalVector[i]->Cal0());
        //	s.Printf("    Cal1 = %f \n", sortedSignalVector[i]->Cal1());
        s.Printf("}\n");
        sig = (EPICSSignal *)sig->Next();
    }

    s.Printf("}\n");
    return True;
}


