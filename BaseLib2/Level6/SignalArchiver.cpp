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

#include "SignalArchiver.h"
#include "Directory.h"
#include "GlobalObjectDataBase.h"
#include "MatlabConverter.h"
#include "GCNString.h"
#include "BasicTypes.h"
#include "SignalInterface.h"
#include "Signal.h"
#include "Matrix.h"

OBJECTLOADREGISTER(SignalArchiver, "$Id$")

void SignalArchivingFn(void *args){
    SignalArchiver *sa = (SignalArchiver *)args;
    while(sa->running){
        sa->archiveSem.ResetWait();
        if(sa->running){
            sa->StoreSignals();
        }
    }
    sa->running = True;
}


bool SignalArchiver::SaveInMatlab(GCRTemplate<SignalInterface> s, FString &originalSignalName, FString &matlabVarName, File &file){
    // Find signal type
    int numberOfSamples = s->NumberOfSamples();
    MatlabConverter mc(&file);
    if(s->Type() == BTDInt32 || s->Type() == BTDUint32){
        MatrixI m;
        m.Allocate(1, numberOfSamples);
        int* data = m.Data();
        memcpy((void*)data, (void*)s->Buffer(), numberOfSamples*sizeof(int));
        return mc.Save(m,matlabVarName.Buffer());
    }else if(s->Type() == BTDFloat){
        MatrixF m;
        m.Allocate(1, numberOfSamples);
        float* data = m.Data();
        memcpy((void*)data, (void*)s->Buffer(), numberOfSamples*sizeof(float));
        return mc.Save(m, matlabVarName.Buffer());
    }else{
        AssertErrorCondition(FatalError,"%s:SaveInMatlab: unhandled signal type for signal:%s",Name(), originalSignalName.Buffer());
    }
    return False;
}

void SignalArchiver::GetDateTimeString(FString &timeStr){
    //Get the current time and store in string format (could have used localtime_r, but it is not supported
    //in fcomm at this time...
    char   stime[64];
    uint32 errorTime = time(NULL);
#ifndef _RTAI
    sprintf(stime,"%s",ctime((const time_t*)&errorTime));
#else
 ctime(stime, 64, (const long int*)errorTime);    
#endif
    stime[strlen(stime)-1]=0;
    char *c = stime;
    while (*c != 0){
        if (*c == ':') *c = '.';
        if (*c == ' ') *c = '_';
        c++;
    }
    timeStr = stime;
}

bool SignalArchiver::StoreSignals(){
    //Query the current date/time
    FString currentTime;
    GetDateTimeString(currentTime);
    //Create the directory
    FString directoryName;
    if(relativeDirectoryPath.Size() == 0){
        directoryName.Printf("%s%c%s", archiveDirectoryPath.Buffer(), DIRECTORY_SEPARATOR, currentTime.Buffer());
    }
    else{
        directoryName.Printf("%s%c%s", archiveDirectoryPath.Buffer(), DIRECTORY_SEPARATOR, relativeDirectoryPath.Buffer());
    }
    if(!Directory::DirectoryExists(directoryName.Buffer())){
        if(!Directory::Create(directoryName.Buffer())){
            AssertErrorCondition(FatalError, "%s::StoreSignals: Could not create directory %s", Name(), directoryName.Buffer());
            return False;
        }
    }
    
    //Loop through all the signals and store them in the disk
    CDBExtended cdbx(signalDataBase);

    if (cdbx->Move("Signals")){
        int32 i = 0;
        for(i = 0; i<cdbx->NumberOfChildren();i++){
            if(cdbx->MoveToChildren(i)){
                //Get the original signal name from the node in the CDB tree
                FString originalSignalName;
                if(cdbx->NodeName(originalSignalName)){
                    //Retrieve the signal
                    GCRTemplate<SignalInterface> signal = GetSignal(originalSignalName.Buffer());
                    if(!signal.IsValid()){
                        AssertErrorCondition(FatalError, "%s:StoreSignals: invalid signal name %s", Name(), originalSignalName.Buffer());
                        return False;
                    }

                    //Remove illegal/strange character from the signal name
                    FString signalName;
                    RemoveIllegalCharacters(signalName, originalSignalName, "/<:");
                    FString filename;

                    //Create the full path for the signal
                    filename.Printf("%s%c%s", directoryName.Buffer(), DIRECTORY_SEPARATOR, signalName.Buffer());

                    File outputFile;
                    //Add the correct extension
                    if(writeExtension){
                        if(storageMode == MATLAB){
                            filename.Printf(".mat");
                        }
                        else if(storageMode == TEXT){
                            filename.Printf(".txt");
                        }
                        else{
                            filename.Printf(".bin");
                        }
                    }
                    if(!outputFile.OpenWrite(filename.Buffer())){
                        AssertErrorCondition(FatalError, "%s::StoreSignals: Could not open file %s", Name(), filename.Buffer());
                        return False;
                    }
            
                    //Store the date accordingly to the mode
                    if(storageMode == MATLAB){
                        if(!SaveInMatlab(signal, originalSignalName, signalName, outputFile)){
                            AssertErrorCondition(FatalError, "%s::StoreSignals: Could not store signal %s in matlab format in file %s", Name(), originalSignalName.Buffer(), filename.Buffer());
                            return False;
                        }
                    }
                    else if(storageMode == TEXT){
                        //The ObjectSaveSetup is provided by the GCNamedObject interface...
                        GCRTemplate<GCNamedObject> signalGCO = signal;
                        //Saves a CDB to a stream without buffering to memory
                        ConfigurationDataBase content("CDBOS");
                        //Assigns the stream to write to
                        content->WriteToStream(outputFile);
                        if(!signalGCO->ObjectSaveSetup(content, NULL)){
                            AssertErrorCondition(FatalError, "%s::StoreSignals: Could not store signal %s in text format in file %s", Name(), originalSignalName.Buffer(), filename.Buffer());
                            return False;
                        }
                    }
                    else{
                        uint32 size = signal->NumberOfSamples() * signal->Type().ByteSize();
                        if(!outputFile.Write(signal->Buffer(), size)){
                            AssertErrorCondition(FatalError, "%s::StoreSignals: Could not store signal %s in binary format in file %s", Name(), originalSignalName.Buffer(), filename.Buffer());
                            return False;
                        }
                    }
                    outputFile.Close();
                }
                cdbx->MoveToFather();
            }
        }
        cdbx->MoveToFather();
    }

    return True;
}


