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


#include "WebStatisticGAM.h"
#include "ConfigurationDataBase.h"
#include "FString.h"
#include "CDBExtended.h"
#include "DDBInputInterface.h"
#include "DDBOutputInterface.h"
#include "DDBSignalDescriptor.h"
#include "BasicTypes.h"


bool WebStatisticGAM::Initialise(ConfigurationDataBase& cdbData){

    float decayRate;
    int32 maxSamples;

    CDBExtended cdb(cdbData);

    cdb.ReadFloat(decayRate,"DecayRate",0.999);
    AssertErrorCondition(Information,"WebStatisticGAM::Initialise: %s using DecayRate=%f",Name(),decayRate);

    cdb.ReadInt32(maxSamples,"MaxSamples",-1);
    AssertErrorCondition(Information,"WebStatisticGAM::Initialise: %s using MaxSamples=%d",Name(),maxSamples);

    FString tmp;
    cdb.ReadFString(tmp,"EOPOutput","True");
    if(tmp=="True") {
        EOPOutput = True;
    }else{
        EOPOutput = False;
    }

    cdb.ReadFString(tmp,"EOPMemoryInformation","True");
    if(tmp=="True") {
        EOPMemInfo = True;
    }else{
        EOPMemInfo = False;
    }

    /////////////////
    // Add signals //
    /////////////////

    if(!AddInputInterface(inputData,"StatisticSignalList")){
        AssertErrorCondition(InitialisationError,"WebStatisticGAM::Initialise: %s failed to add input interface InputInterface StatisticSignalList",Name());
        return False;
    }

    // Read Signal Names //
    if(!cdb->Move("Signals")){
        AssertErrorCondition(InitialisationError,"WebStatisticGAM::Initialise: %s did not specify Signals entry",Name());
        return False;
    }

    int32 nOfSignals = cdb->NumberOfChildren();

    if(!inputData->ObjectLoadSetup(cdb,NULL)){
        AssertErrorCondition(InitialisationError,"WebStatisticGAM::Initialise: %s: ObjectLoadSetup Failed DDBInterface %s ",Name(),inputData->InterfaceName());
        return False;
    }

    cdb->MoveToFather();

    if(!AddOutputInterface(statistics,"Statistics")){
        AssertErrorCondition(InitialisationError,"WebStatisticGAM::Initialise: %s failed to add input interface OutputInterface Statistics",Name());
        return False;
    }

    // Find total number of signals
    const DDBSignalDescriptor *descriptor = inputData->SignalsList();
    uint32 signalCounter = 0;

    //Must do this in order to take into account the signals inside arrays
    int i = 0;
    for(i = 0; ((i < nOfSignals)&&(descriptor != NULL)); i++){
        signalCounter += descriptor->SignalSize();
        descriptor = descriptor->Next();
    }
    if(signalCounter == 0) {
        AssertErrorCondition(InitialisationError,"WebStatisticGAM::Initialise: %s: No Signal has been specified", Name());
        return False;
    }

    numberOfSignals = signalCounter;
    statInfo = new StatSignalInfo[numberOfSignals];
    for (i=0; i<numberOfSignals; i++) {
        statInfo[i].decayRate = decayRate;
        statInfo[i].maxSamples = maxSamples;
    }

    if(statInfo == NULL){
        AssertErrorCondition(InitialisationError,"WebStatisticGAM::Initialise: %s: Failed allocating memory for %d StatSignalInfo structures", Name(), signalCounter);
        return False;
    }

    descriptor = inputData->SignalsList();
    signalCounter = 0;
    for(i = 0; ((i < nOfSignals)&&(descriptor != NULL)); i++){
        FString signalName;
        signalName         = descriptor->SignalName();
        uint32 signalSize  = descriptor->SignalSize();
        bool isInt = False;
        BasicTypeDescriptor btd = descriptor->SignalTypeCode();
        // If it is an array!
        if(signalSize > 1){
            for(int j = 0; j < signalSize; j++){
                FString  newName;
                FString  typeSize;
                btd.ConvertToString(typeSize);
                newName.Printf("%s[%d]",signalName.Buffer(),j);
                FString temp;
                temp.Printf("%sMean",newName.Buffer());
                statistics->AddSignal(temp.Buffer(),"float");
                temp.SetSize(0);
                temp.Printf("%sVariance",newName.Buffer());
                statistics->AddSignal(temp.Buffer(),"float");
                temp.SetSize(0);
                temp.Printf("%sMinimum",newName.Buffer());
                statistics->AddSignal(temp.Buffer(),"float");
                temp.SetSize(0);
                temp.Printf("%sMaximum",newName.Buffer());
                statistics->AddSignal(temp.Buffer(),"float");
                // Increment offset
                statInfo[signalCounter].signalType = btd;
                signalCounter++;
            }
        }else{
            FString  newName;
            FString  typeSize;
            btd.ConvertToString(typeSize);
            newName.Printf("%s",signalName.Buffer());
            
            FString temp;
            temp.Printf("%sMean",newName.Buffer());
            statistics->AddSignal(temp.Buffer(),"float");
            temp.SetSize(0);
            temp.Printf("%sVariance",newName.Buffer());
            statistics->AddSignal(temp.Buffer(),"float");
            temp.SetSize(0);
            temp.Printf("%sMinimum",newName.Buffer());
            statistics->AddSignal(temp.Buffer(),"float");
            temp.SetSize(0);
            temp.Printf("%sMaximum",newName.Buffer());
            statistics->AddSignal(temp.Buffer(),"float");
            // Increment offset
            statInfo[signalCounter].signalType = btd;
            signalCounter++;
        }
        descriptor = descriptor->Next();
    }


    AssertErrorCondition(Information,"WebStatisticGAM::Initialise: %s: Correctly Initialized", Name());

    return True;
}


bool WebStatisticGAM::Execute(GAM_FunctionNumbers functionNumber){

    inputData->Read();
	int i = 0;
    switch(functionNumber){
        case GAMOffline:
        case GAMOnline:{
            lastDataUpdateTime = HRT::HRTCounter();
            //Discard first second of data
            if(!voidCyclesDone){
                if (voidCycles == 0) voidCycles = HRT::HRTCounter();
                if (((HRT::HRTCounter() - voidCycles) * HRT::HRTPeriod()) < 1) return True;
                voidCyclesDone = True;
            }

            sampleNumber++;
            size_t inputBuffer = (size_t)inputData->Buffer();
            for(i = 0; i < numberOfSignals; i++){
                if ((statInfo[i].signalType == BTDInt32) || (statInfo[i].signalType == BTDUint32)) {
                    int32 tmp = *((int32*)inputBuffer);
                    statInfo[i].Update(tmp);
                    float tmp2 = (float)tmp;
                    statInfo[i].Update(tmp2);
                    inputBuffer+=sizeof(int32);
                } else if (statInfo[i].signalType == BTDFloat){
                    float tmp = *((float*)inputBuffer);
                    statInfo[i].Update(tmp);
                    inputBuffer+=sizeof(float);
                } else if (statInfo[i].signalType == BTDDouble){
                    double tmp = *((double*)inputBuffer);
                    statInfo[i].Update((float)tmp);
                    inputBuffer+=sizeof(double);
                } else if ((statInfo[i].signalType == BTDInt64 ) || (statInfo[i].signalType == BTDUint64)) {
                    int64 tmp = *((int64 *)inputBuffer);
                    statInfo[i].Update(tmp);
                    float tmp2 = (float)tmp;
                    statInfo[i].Update(tmp2);
                    inputBuffer+=sizeof(int64);
                }
            }
        }break;

        case GAMPrepulse:{
            sampleNumber   = 0;
            voidCycles     = HRT::HRTCounter();
            voidCyclesDone = False;
            for(i = 0; i < numberOfSignals; i++) statInfo[i].Init();
        }break;

        case GAMPostpulse:{
            if (EOPOutput){
                const DDBSignalDescriptor *descriptor = inputData->SignalsList();
                int counter = 0;
                while(descriptor != NULL){
                    FString signalName;
                    signalName         = descriptor->SignalName();
                    uint32 signalSize  = descriptor->SignalSize();
                    if(signalSize > 1){
                        for(int j=0; j < signalSize; j++){
                            FString output;
                            output.Printf("WebStatisticGAM::GAMPostpulse: Signal %s[%i]: Mean: %e Variance: %e Max: %e Min: %e",signalName.Buffer(),j,statInfo[counter].Mean(sampleNumber), statInfo[counter].Variance(sampleNumber), statInfo[counter].AbsMax(), statInfo[counter].AbsMin());
                            AssertErrorCondition(Information,"%s",output.Buffer());
                            counter++;
                        }
                    }else{
                        FString output;
                        output.Printf("WebStatisticGAM::GAMPostpulse: Signal[%d] %s: Mean: %e Variance: %e Max: %e Min: %e",sampleNumber, signalName.Buffer(),statInfo[counter].Mean(sampleNumber), statInfo[counter].Variance(sampleNumber), statInfo[counter].AbsMax(), statInfo[counter].AbsMin());
                        AssertErrorCondition(Information,"%s",output.Buffer());
                        counter++;
                    }
                    descriptor = descriptor->Next();
                }
            }

            if (EOPMemInfo){
#ifdef _RTAI
                AssertErrorCondition(Information, "WebStatisticGAM::GAMPostPulse: Memory information. BaseLib2's heap used: %d bytes, total: %d bytes",fcomm_get_baselib2_allocated_mem(),fcomm_get_baselib2_total_mem());
                AssertErrorCondition(Information, "WebStatisticGAM::GAMPostPulse: Memory information. FCOMM's heap used: %d bytes, total: %d bytes",fcomm_get_fcomm_allocated_mem(),fcomm_get_fcomm_total_mem());
#endif
                FString tmp;
                FString buffer;
                DisplayRegisteredClasses(&tmp,True);
                tmp.Seek(0);
                AssertErrorCondition(Information, "WebStatisticGAM::GAMPostPulse: Allocated classes dump:");
                while (tmp.GetLine(buffer)) {
                    AssertErrorCondition(Information, "%s",buffer.Buffer());
                }
            }

        }break;
    };

    return True;
}

bool WebStatisticGAM::ProcessHttpMessageTextMode(HttpStream &hStream){
    hStream.SSPrintf("OutputHttpOtions.Content-Type","text/plain");
    hStream.keepAlive = False;
    
    FString requestedSignal;
    requestedSignal.SetSize(0);
    if (hStream.Switch("InputCommands.signal")){
        hStream.Seek(0);
        hStream.GetToken(requestedSignal, "");
        hStream.Switch((uint32)0);
    }

    const DDBSignalDescriptor *descriptor = inputData->SignalsList();
    int counter = 0;
    while(descriptor != NULL){
        FString signalName;
        signalName         = descriptor->SignalName();
        uint32 signalSize  = descriptor->SignalSize();
        FString signalTypeName;
        statInfo[counter].signalType.ConvertToString(signalTypeName);

        if((requestedSignal.Size() == 0) || (requestedSignal == signalName)){
            if(signalSize > 1){
                for(int j=0; j < signalSize; j++){
                    FString compoundName;
                    compoundName.Printf("%s(%d)",signalName.Buffer(),j+descriptor->SignalFromIndex());
                    hStream.Printf("%s ",compoundName.Buffer());
                    hStream.Printf("%e %e %e %e %e %e %e %s\n",statInfo[counter].LastValue(), statInfo[counter].Mean(sampleNumber), statInfo[counter].Variance(sampleNumber), statInfo[counter].AbsMax(), statInfo[counter].AbsMin(), statInfo[counter].RelMax(), statInfo[counter].RelMin(),signalTypeName.Buffer());
                    counter++;
                }
            }else{
                hStream.Printf("%s ",signalName.Buffer());
                hStream.Printf("%e %e %e %e %e %e %e %s\n",statInfo[counter].LastValue(), statInfo[counter].Mean(sampleNumber), statInfo[counter].Variance(sampleNumber), statInfo[counter].AbsMax(), statInfo[counter].AbsMin(), statInfo[counter].RelMax(), statInfo[counter].RelMin(),signalTypeName.Buffer());
                counter++;
            }
        }
        descriptor = descriptor->Next();
    }


    hStream.WriteReplyHeader(False);
    return True;
}

bool WebStatisticGAM::ProcessHttpMessage(HttpStream &hStream){
	int i = 0;
    FString mode;
    mode.SetSize(0);
    if (hStream.Switch("InputCommands.mode")){
        hStream.Seek(0);
        hStream.GetToken(mode, "");
        hStream.Switch((uint32)0);
    }
    if(mode == "text"){
        return WebStatisticGAM::ProcessHttpMessageTextMode(hStream);
    }

    const DDBSignalDescriptor *descriptor = inputData->SignalsList();
    double dataAge = (HRT::HRTCounter() - lastDataUpdateTime) * HRT::HRTPeriod();

    hStream.SSPrintf("OutputHttpOtions.Content-Type","text/html");
    hStream.keepAlive = False;
    //copy to the client
    hStream.WriteReplyHeader(False);

    hStream.Printf("<html><head><title>WebStatisticGAM - Statistics</title></head><body>");
    hStream.Printf("<h1>Current statistics:</h1><br />");
    hStream.Printf("<p>Data was updated %f seconds ago<p/>", dataAge);

    hStream.Printf("<table border=\"1\"><tr><th></th><th>Last value</th><th>Mean</th><th>Variance</th><th>Abs Max</th><th>Abs Min</th><th>Rel Max</th><th>Rel Min</th><th>Type</th></tr>");

    int counter = 0;
    while(descriptor != NULL){
        FString signalName;
        signalName         = descriptor->SignalName();
        uint32 signalSize  = descriptor->SignalSize();
        FString signalTypeName;
        statInfo[counter].signalType.ConvertToString(signalTypeName);

        if(signalSize > 1){
            for(int j=0; j < signalSize; j++){
                FString compoundName;
                compoundName.Printf("%s(%d)",signalName.Buffer(),j+descriptor->SignalFromIndex());
                hStream.Printf("<tr><th>%s</th>",compoundName.Buffer());

                hStream.Printf("<td>%.3e</td><td>%.3e</td><td>%.3e</td><td>%.3e</td><td>%.3e</td><td>%.3e</td><td>%.3e</td><td>%s</td></tr>",statInfo[counter].LastValue(), statInfo[counter].Mean(sampleNumber), statInfo[counter].Variance(sampleNumber), statInfo[counter].AbsMax(), statInfo[counter].AbsMin(), statInfo[counter].RelMax(), statInfo[counter].RelMin(),signalTypeName.Buffer());
                counter++;
            }
        }else{
            hStream.Printf("<tr><th>%s</th>",signalName.Buffer());
            hStream.Printf("<td>%.3e</td><td>%.3e</td><td>%.3e</td><td>%.3e</td><td>%.3e</td><td>%.3e</td><td>%.3e</td><td>%s</td></tr>",statInfo[counter].LastValue(), statInfo[counter].Mean(sampleNumber), statInfo[counter].Variance(sampleNumber), statInfo[counter].AbsMax(), statInfo[counter].AbsMin(), statInfo[counter].RelMax(), statInfo[counter].RelMin(),signalTypeName.Buffer());
            counter++;
        }

        descriptor = descriptor->Next();
    }
    hStream.Printf("</table><br />");

    hStream.Printf("<h3>Integer 32 bits signals</h3>\n");
    hStream.Printf("<table border=\"1\"><tr><th></th><th>Decimal</th><th>Hex</th>");
    for(i=(sizeof(int32) * 8 - 1); i>-1; i--){
    	hStream.Printf("<th>%02d</th>", i);
    }
    hStream.Printf("</tr>");

    counter = 0;
    descriptor = inputData->SignalsList();    
    while(descriptor != NULL){
        FString signalName;
        signalName         = descriptor->SignalName();
        uint32 signalSize  = descriptor->SignalSize();
        if ((statInfo[counter].signalType == BTDInt32) || (statInfo[counter].signalType == BTDUint32)){
            if(signalSize > 1){
                for(int j=0; j < signalSize; j++){
                    FString compoundName;
                    compoundName.Printf("%s(%d)",signalName.Buffer(),j+descriptor->SignalFromIndex());
                    hStream.Printf("<tr><th>%s</th>",compoundName.Buffer());

                    hStream.Printf("<td>%d</td><td>0x%x</td>",statInfo[counter].lastIntSample, statInfo[counter].lastIntSample);
    		        statInfo[counter].PrintLastIntSampleAsBinary(hStream);
	        	    hStream.Printf("</tr>");
                    counter++;
                }
            }else{
                hStream.Printf("<tr><th>%s</th>",signalName.Buffer());
    	        hStream.Printf("<td>%d</td><td>0x%x</td>",statInfo[counter].lastIntSample, statInfo[counter].lastIntSample);
        	    statInfo[counter].PrintLastIntSampleAsBinary(hStream);
                hStream.Printf("</tr>");
                counter++;
            }
        }else{
    	    counter+=signalSize;
	    }
        descriptor = descriptor->Next();
    }
    hStream.Printf("</table><br />");

    hStream.Printf("<h3>Integer 64 bits signals</h3>\n");
    hStream.Printf("<table border=\"1\"><tr><th></th><th>Decimal</th><th>Hex</th>");
    for(i=(sizeof(int64) * 8 - 1); i>-1; i--){
    	hStream.Printf("<th>%02d</th>", i);
    }
    hStream.Printf("</tr>");
    counter = 0;
    descriptor = inputData->SignalsList();    
    while(descriptor != NULL){
        FString signalName;
        signalName         = descriptor->SignalName();
        uint32 signalSize  = descriptor->SignalSize();
        if ((statInfo[counter].signalType == BTDInt64) || (statInfo[counter].signalType == BTDUint64)){
            if(signalSize > 1){
                for(int j=0; j < signalSize; j++){
                    FString compoundName;
                    compoundName.Printf("%s(%d)",signalName.Buffer(),j+descriptor->SignalFromIndex());
                    hStream.Printf("<tr><th>%s</th>",compoundName.Buffer());

                    hStream.Printf("<td>%lld</td><td>0x%x</td>",statInfo[counter].lastInt64Sample, statInfo[counter].lastInt64Sample);
    		        statInfo[counter].PrintLastInt64SampleAsBinary(hStream);
	        	    hStream.Printf("</tr>");
                    counter++;
                }
            }else{
                hStream.Printf("<tr><th>%s</th>",signalName.Buffer());
    	        hStream.Printf("<td>%lld</td><td>0x%x</td>",statInfo[counter].lastInt64Sample, statInfo[counter].lastInt64Sample);
                statInfo[counter].PrintLastInt64SampleAsBinary(hStream);
                hStream.Printf("</tr>");
                counter++;
            }
        }else{
    	    counter+=signalSize;
	    }
        descriptor = descriptor->Next();
    }
    hStream.Printf("</table><br />");


#ifdef _RTAI
    hStream.Printf("<h1>Memory usage:</h1><br />");
    hStream.Printf("<table border=\"1\"><tr><th></th><th>Used</th><th>Total</th><th>%% Used</th></tr>");
    float t1 = fcomm_get_baselib2_allocated_mem();
    float t2 = fcomm_get_baselib2_total_mem();
    float t3 = (t1 / t2) * 100;
    hStream.Printf("<tr><td><b>BaseLib2</b></td><td>%d</td><td>%d</td><td>%f</td></tr>",(int32)t1,(int32)t2,t3);
    t1 = fcomm_get_fcomm_allocated_mem();
    t2 = fcomm_get_fcomm_total_mem();
    t3 = (float)(t1 * 100)/(float)t2;
    hStream.Printf("<tr><td><b>FCOMM</b></td><td>%d</td><td>%d</td><td>%f</td></tr>",(int32)t1,(int32)t2,t3);
    t1 = fcomm_get_large_heap_allocated_mem();
    t2 = fcomm_get_large_heap_total_mem();
    t3 = (float)(t1 * 100)/(float)t2;
    hStream.Printf("<tr><td><b>Large</b></td><td>%d</td><td>%d</td><td>%f</td></tr>",(int32)t1,(int32)t2,t3);
    hStream.Printf("</table>");

    hStream.Printf("<table border=\"1\"><tr><th></th><th>Max block size</th></tr>");
    hStream.Printf("<tr><td><b>fcomm</b></td><td>%d</td></tr>", fcomm_get_max_block_size_fcomm());
    hStream.Printf("<tr><td><b>BaseLib2</b></td><td>%d</td></tr>", fcomm_get_max_block_size_baselib2());
    hStream.Printf("<tr><td><b>Large</b></td><td>%d</td></tr>", fcomm_get_max_block_size_large_heap());
    hStream.Printf("</table>");

#endif
    hStream.Printf("</body></html>");
    hStream.WriteReplyHeader(True);

    return True;
}


OBJECTLOADREGISTER(WebStatisticGAM,"$Id$")
