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
// EPICS Lib include
#include "EPICSHandler.h"
//#include "exServer.h"




#include "EPICSSignalsTable.h"


#include "GlobalObjectDataBase.h"
#include "GCRTemplate.h"

#include "DDBInterface.h"
#include "CDBExtended.h"
#include "SignalInterface.h"


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


bool EPICSSignal::Initialize(const FString &ddbName, const FString &jpfName, uint32 offset, BasicTypeDescriptor  descriptor) {
    ddbSignalName        = ddbName;
    EPICSSignalName        = jpfName;
    dataOffset           = offset;
    typeDescriptor       = descriptor;
    data_size = typeDescriptor.ByteSize();
    data_buffer = new char[data_size];
    memset(data_buffer, 0, data_size);
    return True;
}

/*
 * Initialize has to link signals in the DDB with
 * EPICS descriptors (sending messages ?)
 * EPICS is message based so doesn't care
 */
bool EPICSSignalsTable::Initialize(const DDBInterface &ddbInterface, ConfigurationDataBase &info) {
    // CleanUp
    CleanUp();

    int32 nOfSignals = ddbInterface.NumberOfEntries();
    if(nOfSignals == 0)return False;

    // ddbInterface signal list
    CDBExtended cdb(info);
    const DDBSignalDescriptor *descriptor = ddbInterface.SignalsList();
    uint32 wordOffset = 0;
    int i = 0;
 
    // EPICS interface signal list
#define EPICS_LIBRARY "EPICSLib"
        GCReference gc = GODBFindByName(EPICS_LIBRARY);
        if ( !gc.IsValid() ) {
            CStaticAssertErrorCondition(FatalError,
            		"EPICSSignalsTable::Initialize cannot find %s in the GlobalObjectDataBase",
            		EPICS_LIBRARY);
            return False;
        } 
        
        GCRTemplate<EPICSHandler> epics_hnd = gc;
        if ( !epics_hnd.IsValid() ) {
            CStaticAssertErrorCondition(FatalError,
            		"EPICSSignalsTable::Initialize cannot cast %s as an EPICSHandler",
            		EPICS_LIBRARY);
            return False;
        } 
        
        exServer *es = const_cast<exServer*>( epics_hnd->getCaServer() );
    
    for(i = 0; ((i < nOfSignals)&&(descriptor != NULL)); i++){
        cdb->MoveToChildren(i);

        FString  ddbName;
        ddbName = descriptor->SignalName();

        // Get the JPF name of the signal
        FString EPICSSignalName;
        if (!cdb.ReadFString(EPICSSignalName,"EPICSName")) {
            CStaticAssertErrorCondition(FatalError,
            		"EPICSSignalsTable::Initialize: EPICSName not specified for DDB signal %s",
            		ddbName.Buffer());
            return False;
        }

        
// probably this must be done also in EPICS... conversion to uppercase letters        
/*        
        char *temp = EPICSSignalName.Buffer();
        for(int j =0; j<EPICSSignalName.Size(); j++){
            temp[j] = toupper(temp[j]);
        }
*/
        
        //Locate the signal by message .. 
        // honestly I do not know which is the best way to implement it bu...
        // as a first prototype we do things in this way...
        //searching the GODB for the EPICSLib (one instance I hope)
        // and getting the signals by        
        
        uint32 signalSize  = descriptor->SignalSize();
        // If it is an array add the :xxx number at the end of the JPF name
        if (signalSize > 1) {

            float *cal0 = NULL;
            float *cal1 = NULL;

            if(descriptor->SignalTypeCode() == BTDInt32){
                // Read Calibration Factors
                cal0 = (float *)malloc(signalSize*sizeof(float));
                if(cal0 == NULL){
                    CStaticAssertErrorCondition(FatalError,
                    		"EPICSSignalsTable::Initialize: Failed allocating space cal0 for signal %s (%s)",
                    		ddbName.Buffer(), EPICSSignalName.Buffer());
                    return False;
                }
                cal1 = (float *)malloc(signalSize*sizeof(float));
                if(cal1 == NULL){
                    CStaticAssertErrorCondition(FatalError,
                    		"EPICSSignalsTable::Initialize: Failed allocating space cal1 for signal %s (%s)",
                    		ddbName.Buffer(), EPICSSignalName.Buffer());
                    free((void*&)cal0);
                    return False;
                }

                int dims    = 1;
                int size[2] = {signalSize,1};
                if(!cdb.ReadFloatArray(cal0,size,dims,"Cal0")){
                    CStaticAssertErrorCondition(FatalError,"EPICSSignalsTable::Initialize: No Cal0 has been specified for signal %s (%s). Assuming 0.0", ddbName.Buffer(), EPICSSignalName.Buffer());
                    for(int i = 0; i < signalSize; i++){
                        cal0[i] = 0.0;
                    }
                }
                dims = 1;
                size[0] = signalSize;
                size[1] = 1;
                if(!cdb.ReadFloatArray(cal1,size,dims,"Cal1")){
                    CStaticAssertErrorCondition(FatalError,
                    		"EPICSSignalsTable::Initialize: No Cal1 has been specified for signal %s (%s). Assuming 1.0",
                    		ddbName.Buffer(), EPICSSignalName.Buffer());
                    for(int i = 0; i < signalSize; i++){
                        cal1[i] = 1.0;
                    }
                }
            }

            for(int j = 0; j < signalSize; j++){
                FString  newJpfName;
                newJpfName.Printf("%s:%03d",EPICSSignalName.Buffer(),j);
                EPICSSignal *newSignal = new EPICSSignal(ddbName, newJpfName,  wordOffset, descriptor->SignalTypeCode());
                if(newSignal == NULL){
                    CStaticAssertErrorCondition(FatalError,"EPICSSignalsTable::Initialize: Failed allocating space for signal %s (%s)", ddbName.Buffer(), EPICSSignalName.Buffer());
                    if(cal0 != NULL)free((void*&)cal0);
                    if(cal1 != NULL)free((void*&)cal1);
                    return False;
                }

                newSignal->SetCalibrations(cal0[j],cal1[j]);

                ListInsert(newSignal,&AlphaSorter);
                // Increment offset
                wordOffset += descriptor->SignalTypeCode().Word32Size();
            }

            if(cal0 != NULL)free((void*&)cal0);
            if(cal1 != NULL)free((void*&)cal1);

        } //------------------------------------------------------------------- array values
        else {

            EPICSSignal *newSignal = new EPICSSignal(ddbName, EPICSSignalName,  wordOffset, descriptor->SignalTypeCode());
            if(newSignal == NULL){
                CStaticAssertErrorCondition(FatalError,"EPICSSignalsTable::Initialize: Failed allocating space for signal %s (%s)", ddbName.Buffer(), EPICSSignalName.Buffer());
                return False;
            }

            // Set Calibrations
            float cal0 = 0.0;
            float cal1 = 1.0;
            cdb.ReadFloat(cal0,"Cal0",0.0);
            cdb.ReadFloat(cal1,"Cal1",1.0);
            newSignal->SetCalibrations(cal0, cal1);

            ListInsert(newSignal,&AlphaSorter);
            // Increment offset
            wordOffset += descriptor->SignalTypeCode().Word32Size();


        pvExistReturn _exist = es->pvExistTest( EPICSSignalName.Buffer(),  newSignal->DataBuffer(), newSignal->Type());
        if ( _exist.getStatus() == pverDoesNotExistHere )  {
             CStaticAssertErrorCondition(FatalError,
                    		"EPICSSignalsTable::Initialize: EPICS Signal %s cannot be located in EPICSLib",
                    		EPICSSignalName.Buffer());
             return False;                
       }

        } //------------------------------------------------------------------- scalar values        
        
        descriptor = descriptor->Next();
        cdb->MoveToFather();
    }
    
    if((ListSize() == 0) ||(List() == NULL)) {
        CStaticAssertErrorCondition(FatalError,"EPICSSignalsTable::Initialize: No sighal added to the list");
        return False;
    }

    return True;
} //--------------------------------------------------------------------------- ObjectLoadSetup


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


bool EPICSSignalsTable::ObjectDescription(StreamInterface &s,bool full,StreamInterface *err) {

    EPICSSignal *sig = (EPICSSignal *)List();
    if(sig == NULL) return False;

    s.Printf("Signals = {\n");
    for(int i = 0; i < ListSize(); i++){

        FString bType;
        FString oType;

        s.Printf("Signal%d = {\n",i);
        sig->Type().ConvertToString(bType);
        s.Printf("    Name = \"%s\" \n"  , sig->EPICSName());
        s.Printf("    DataType = %s \n"  , bType.Buffer());
        //	s.Printf("    Cal0 = %f \n", sortedSignalVector[i]->Cal0());
        //	s.Printf("    Cal1 = %f \n", sortedSignalVector[i]->Cal1());
        s.Printf("}\n");
        sig = (EPICSSignal *)sig->Next();
    }

    s.Printf("}\n");
    return True;
}
