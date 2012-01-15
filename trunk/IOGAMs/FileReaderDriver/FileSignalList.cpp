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

#include "FileSignalList.h"
#include "File.h"

bool FileSignalList::LoadData(){
    //Open the file
    File f;
    if(!f.OpenRead(filename.Buffer())){
        AssertErrorCondition(InitialisationError,"FileSignalList::ObjectLoadSetup: %s failed to open file: %s", Name(), filename.Buffer());
        return False;
    }
    //Read the first line
    FString line;
    FString token;
    numberOfSignals = 0;
    //Count the number of samples and check if all the lines are consistent
    f.Seek(0);
    line.SetSize(0);
    while(f.GetLine(line)){
        line.Seek(0);
        //Ignore commented lines
        if(IsComment(line)){
            line.SetSize(0);
            continue;
        } 
        //Check signal size consistency
        int32 signalCounter = 0;
        while(line.GetToken(token, " ")){
            signalCounter++;
            token.SetSize(0);
        }
        //Only to initialise once the numberOfSignals
        if(numberOfSignals == 0){
            numberOfSignals = signalCounter;
        }
        if(signalCounter != numberOfSignals){
            AssertErrorCondition(InitialisationError,"FileSignalList::ObjectLoadSetup: %s number of signals is not consistent. )Error at line %d", Name(), numberOfSamples + 1);
            f.Close();
            return False;
        }
        numberOfSamples++;
        line.SetSize(0);
    }

    if(numberOfSignals < 2){
        AssertErrorCondition(InitialisationError,"FileSignalList::ObjectLoadSetup: %s must specify at least a time vector and a signal vector", Name());
        f.Close();
        return False;
    }
    if(numberOfSamples < 1){
        AssertErrorCondition(InitialisationError,"FileSignalList::ObjectLoadSetup: %s Must specify at least one sample", Name());
        f.Close();
        return False;
    }
    
    //The time is not counted as a signal...
    numberOfSignals--;
    AssertErrorCondition(Information,"FileSignalList::ObjectLoadSetup: %s Going to read %d signals, with %d samples", Name(), numberOfSignals, numberOfSamples); 
    //Allocate all vectors
    data = (void *)malloc(numberOfSignals * numberOfSamples * signalType.ByteSize());
    if(data == NULL){
        AssertErrorCondition(InitialisationError,"FileSignalList::ObjectLoadSetup: %s Failed allocating data pointer vector with size %d", Name(), numberOfSignals * numberOfSamples * signalType.ByteSize());
        f.Close();
        return False;
    }
    time = new int64[numberOfSamples];
    if(time == NULL){
        AssertErrorCondition(InitialisationError,"FileSignalList::ObjectLoadSetup: %s Failed allocating time 64 bit vector with size %d", Name(), numberOfSamples);
        f.Close();
        return False;
    }
    //Finally populate all the vectors
    f.Seek(0);
    line.SetSize(0);
    int32 sample = 0;
    int32 signal = 0;
    while(f.GetLine(line)){
        line.Seek(0);
        //Ignore commented lines
        if(IsComment(line)){
            line.SetSize(0);
            continue;
        } 
        //Read the time
        token.SetSize(0);
        line.GetToken(token, separator.Buffer());
        token.Seek(0);
        //atoll and BTConvert don't like int64 if this is in eng. format... Get from double
        double tmp;
        BasicTypeDescriptor sourceBTD(token.Size(), BTDTString, BTDSTCArray);
        if(!BTConvert(1, BTDDouble, &tmp, sourceBTD, token.Buffer())){
            AssertErrorCondition(InitialisationError,"FileSignalList::ObjectLoadSetup: %s Failed converting signal at column %d", Name(), signal + 1);
            f.Close();
            return False;
        }
        time[sample] = (int64)tmp;
        token.SetSize(0);
        signal = 0;
        while(line.GetToken(token, separator.Buffer())){
            token.Seek(0);
            //Automatically convert each of the string tokens
            BasicTypeDescriptor sourceBTD(token.Size(), BTDTString, BTDSTCArray);
            char *p = (char *)data;
            p      += (sample * numberOfSignals + signal) * signalType.ByteSize();
            if(!BTConvert(1, signalType, p, sourceBTD, token.Buffer())){
                AssertErrorCondition(InitialisationError,"FileSignalList::ObjectLoadSetup: %s Failed converting signal at column %d", Name(), signal + 1);
                f.Close();
                return False;
            }
            token.SetSize(0);
            signal++;
        }
        sample++;
        line.SetSize(0);
    }

    f.Close();
    AssertErrorCondition(Information,"FileSignalList::ObjectLoadSetup: %s Successfully read %d signals, with %d samples", Name(), numberOfSignals, numberOfSamples);
    return True;
}

bool FileSignalList::ObjectLoadSetup(ConfigurationDataBase &info,StreamInterface *err){
    GCNamedObject::ObjectLoadSetup(info, err);
    AssertErrorCondition(Information, "FileSignalList::ObjectLoadSetup: %s Loading signals", Name());

    CDBExtended cdb(info);

    //Read the data type
    FString signalTypeStr;
    if(!cdb.ReadFString(signalTypeStr, "SignalType", "float")){
        AssertErrorCondition(Warning, "FileSignalList::ObjectLoadSetup: %s did not specify SignalType. Assuming %s", Name(), signalTypeStr.Buffer());
    }
    if(!BTConvertFromString(signalType, signalTypeStr.Buffer())){
        AssertErrorCondition(InitialisationError,"FileSignalList::ObjectLoadSetup: %s failed to convert to signalType %s", Name(), signalTypeStr.Buffer());
        return False;
    }
    if(!cdb.ReadFString(separator, "Separator", " ")){
        AssertErrorCondition(Warning,"FileSignalList::ObjectLoadSetup: %s no separator specificed. Using default: \"%s\"", Name(), separator.Buffer());
    }
    //Get the filename
    if(!cdb.ReadFString(filename, "Filename")){
        AssertErrorCondition(InitialisationError,"FileSignalList::ObjectLoadSetup: %s did not specify Filename", Name());
        return False;
    }

    return LoadData();
}

void *FileSignalList::GetNextSample(uint32 usecTime){
    if(data == NULL || time == NULL){
        AssertErrorCondition(FatalError,"FileSignalList::GetNextSample: %s Data is not ready or is NULL", Name()); 
        return NULL;
    }
    //Moves the internal counter to the next sample after the specified time
    //and returns the previous sample
    while(time[sampleCounter] <= usecTime){
        sampleCounter++;
        if(sampleCounter == numberOfSamples){
            break;
        }
    }
    sampleCounter--;
    return (sampleCounter == -1) ? data : ((char *) data) + sampleCounter * (numberOfSignals *  signalType.ByteSize());
}

OBJECTLOADREGISTER(FileSignalList,"$Id$")

