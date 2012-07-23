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

#include "DataCollectionSignalsTable.h"
#include "DDBInterface.h"
#include "GCRTemplate.h"
#include "CDBExtended.h"
#include "SignalInterface.h"

class AlphaSorterClass : public SortFilter {

public:

    virtual ~AlphaSorterClass(){};

    virtual int32 Compare(LinkedListable *data1,LinkedListable *data2){
        const DataCollectionSignal *p1 = (const DataCollectionSignal *)data1;
        const DataCollectionSignal *p2 = (const DataCollectionSignal *)data2;
        if (p2 == NULL) return 1;
        if (p1 == NULL) return -1;

        return strcmp(p2->JPFName(),p1->JPFName());
    }

} AlphaSorter;


bool DataCollectionSignal::Initialize(const FString &ddbName, const FString &jpfName, uint32 offset, BasicTypeDescriptor  descriptor){
    ddbSignalName             = ddbName;
    jpfSignalName             = jpfName;
    dataOffset                = offset;
    typeDescriptor            = descriptor;

    return True;
}


bool DataCollectionSignalsTable::Initialize(const DDBInterface &ddbInterface, ConfigurationDataBase &info){
    // CleanUp
    CleanUp();

    int32 nOfSignals = ddbInterface.NumberOfEntries();
    if(nOfSignals == 0)return False;

    CDBExtended cdb(info);
    const DDBSignalDescriptor *descriptor = ddbInterface.SignalsList();
    uint32 wordOffset = 0;
    int i = 0;
    for(i = 0; ((i < nOfSignals)&&(descriptor != NULL)); i++){
        cdb->MoveToChildren(i);

        FString  ddbName;
        ddbName = descriptor->SignalName();

        // Get the JPF name of the signal
        FString jpfSignalName;
        if(!cdb.ReadFString(jpfSignalName,"JPFName")){
            CStaticAssertErrorCondition(FatalError,"DataCollectionSignalsTable::Initialize: JPFName not specified for DDB signal %s", ddbName.Buffer());
            return False;
        }

        ///////////////////////////////////
        // Convert to upper case letters //
        ///////////////////////////////////
        char *temp = jpfSignalName.BufferReference();
        if(forceSignalNamesUpperCase){
            for(int j =0; j<jpfSignalName.Size(); j++){
                temp[j] = toupper(temp[j]);
            }
        }

        uint32 signalSize  = descriptor->SignalSize();
        // If it is an array add the :xxx number at the end of the JPF name
        if(signalSize > 1){

            float *cal0 = NULL;
            float *cal1 = NULL;

            
                // Read Calibration Factors
            cal0 = (float *)malloc(signalSize*sizeof(float));
            if(cal0 == NULL){
                    CStaticAssertErrorCondition(FatalError,"DataCollectionSignalsTable::Initialize: Failed allocating space cal0 for signal %s (%s)", ddbName.Buffer(), jpfSignalName.Buffer());
                    return False;
            }
            cal1 = (float *)malloc(signalSize*sizeof(float));
            if(cal1 == NULL){
                    CStaticAssertErrorCondition(FatalError,"DataCollectionSignalsTable::Initialize: Failed allocating space cal1 for signal %s (%s)", ddbName.Buffer(), jpfSignalName.Buffer());
                    free((void*&)cal0);
                    return False;
            }

            int dims    = 1;
            int size[2] = {signalSize,1};
            if(!cdb.ReadFloatArray(cal0,size,dims,"Cal0")){
                CStaticAssertErrorCondition(FatalError,"DataCollectionSignalsTable::Initialize: No Cal0 has been specified for signal %s (%s). Assuming 0.0", ddbName.Buffer(), jpfSignalName.Buffer());
                for(int i = 0; i < signalSize; i++){
                       cal0[i] = 0.0;
                  }
            }
            dims = 1;
            size[0] = signalSize;
            size[1] = 1;
            if(!cdb.ReadFloatArray(cal1,size,dims,"Cal1")){
                   CStaticAssertErrorCondition(FatalError,"DataCollectionSignalsTable::Initialize: No Cal1 has been specified for signal %s (%s). Assuming 1.0", ddbName.Buffer(), jpfSignalName.Buffer());
                   for(int i = 0; i < signalSize; i++){
                        cal1[i] = 1.0;
                    }
            }
            

            for(int j = 0; j < signalSize; j++){
                FString  newJpfName;
                newJpfName.Printf("%s:%03d",jpfSignalName.Buffer(),j);
                DataCollectionSignal *newSignal = new DataCollectionSignal(ddbName, newJpfName,  wordOffset, descriptor->SignalTypeCode());
                if(newSignal == NULL){
                    CStaticAssertErrorCondition(FatalError,"DataCollectionSignalsTable::Initialize: Failed allocating space for signal %s (%s)", ddbName.Buffer(), jpfSignalName.Buffer());
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

        }else{

            DataCollectionSignal *newSignal = new DataCollectionSignal(ddbName, jpfSignalName,  wordOffset, descriptor->SignalTypeCode());
            if(newSignal == NULL){
                CStaticAssertErrorCondition(FatalError,"DataCollectionSignalsTable::Initialize: Failed allocating space for signal %s (%s)", ddbName.Buffer(), jpfSignalName.Buffer());
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
        }


        descriptor = descriptor->Next();
        cdb->MoveToFather();
    }
    
    if((ListSize() == 0) ||(List() == NULL)) {
        CStaticAssertErrorCondition(FatalError,"DataCollectionSignalsTable::Initialize: No sighal added to the list");
        return False;
    }

    //Initialise flag to zero => use standard memory instead of upper memory
    useUpperMemory2CopySignal = MEMORYStandardMemory;

    return True;
}


int32 DataCollectionSignalsTable::FindOffsetAndInitSignalType(const FString &signalName, GCRTemplate<SignalInterface> signal, int32 nOfSamples){

    DataCollectionSignal *sig = (DataCollectionSignal *)List(); 
    for(int i = 0; i < ListSize(); i++){
      if((strcmp(sig->JPFName(),signalName.Buffer()) == 0) || (strcmp(sig->DDBName(),signalName.Buffer()) == 0)) {
	    signal->CopyData(sig->Type(), nOfSamples, NULL, useUpperMemory2CopySignal);
            return sig->Offset();
        }
        sig = (DataCollectionSignal *)sig->Next();
    }
    
    return -1;
}


bool DataCollectionSignalsTable:: ObjectDescription(StreamInterface &s,bool full,StreamInterface *err){

    DataCollectionSignal *sig = (DataCollectionSignal *)List();
    if(sig == NULL) return False;

    s.Printf("Signals = {\n");
    for(int i = 0; i < ListSize(); i++){

        FString bType;
        FString oType;

        s.Printf("Signal%d = {\n",i);
        sig->Type().ConvertToString(bType);
        s.Printf("    Name = \"%s\" \n"  , sig->JPFName());
        s.Printf("    DDBName = \"%s\" \n"  , sig->DDBName());
        s.Printf("    DataType = %s \n"  , bType.Buffer());
        //	s.Printf("    Cal0 = %f \n", sortedSignalVector[i]->Cal0());
        //	s.Printf("    Cal1 = %f \n", sortedSignalVector[i]->Cal1());
        s.Printf("}\n");
        sig = (DataCollectionSignal *)sig->Next();
    }

    s.Printf("}\n");
    return True;
}


