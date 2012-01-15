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

#include "RTDataStorageSystem.h"
#include "ConfigurationDataBase.h"
#include "CDBExtended.h"
#include "GenDefs.h"

/* min(a,b) function defined only in VxWorks
 * or other SOs we redefine it here */

//#if !defined(_VXWORKS)
//static int32 min(int32 a,int32 b){
//	if (a<=b) {
//		return a;
//	} else {
//		return b;
//	}
//}
//#endif


/* min(a,b) function defined only in VxWorks Tornado.
 * or other SOs we redefine it here */
#if !defined(_VXWORKS)
static int32 minimum(int32 a,int32 b){
    if (a<=b)
        return a;
    else
        return b;
}
#else
#if defined(_V6X5100) || defined(_V6X5500)
/* As previously defined in vxWorks.h */
#define min(x, y)	(((x) < (y)) ? (x) : (y))
#endif
#endif


RTDataStorageSystem::RTDataStorageSystem(){

    acquisitionTimesBuffer          = NULL;
    acquisitionTimesBufferIndex     = 0;
    acquisitionTimesBufferSize      = 0;

    maxFastAcquisitionPoints        = 0;
    pointsForSingleFastAcquisition  = 0;
    fastAcquisitionPointsLeft       = 0;
    fastAcquisitionPointsToDo       = 0;

    // Initialize the internal GAP EXTCLOK message header
    CLOCKInit.nOfSequences   = 0;
    CLOCKInit.nOfReadSamples = 0;
    for(int i=0;i<RTDataStorageMaxWindows;i++){
        CLOCKInit.nOfSamples[i]         = 0;
        CLOCKInit.samplingPeriodUsec[i] = 0;
    }

}

RTDataStorageSystem::~RTDataStorageSystem(){
    CleanUpTimeVector();
    Reset();
}

void RTDataStorageSystem::CleanUpTimeVector(){
    // Initialize the timeWindow struct
    if (acquisitionTimesBuffer != NULL) free((void *&)acquisitionTimesBuffer);
    acquisitionTimesBuffer          = NULL;
    acquisitionTimesBufferIndex     = 0;
    acquisitionTimesBufferSize      = 0;
    fastAcquisitionPointsLeft       = 0;
    fastAcquisitionPointsToDo       = 0;

}

void  RTDataStorageSystem::PrepareForNextPulse(){
    Reset();
    CLOCKInit.nOfReadSamples        = 0;
    acquisitionTimesBufferIndex     = 0;
    fastAcquisitionPointsLeft       = maxFastAcquisitionPoints;
    fastAcquisitionPointsToDo       = 0;
}


bool RTDataStorageSystem::CreateTimeVectorFromTimeWindow(){
    CleanUpTimeVector();

    // sets acquisitionTimesBufferSize
    acquisitionTimesBufferSize = 0;
    int i;
    for(i = 0;i<(CLOCKInit.nOfSequences);i++){
        if(CLOCKInit.samplingPeriodUsec[i]>0){
            acquisitionTimesBufferSize += CLOCKInit.nOfSamples[i];
        }
    }

    if(acquisitionTimesBufferSize == 0){
        AssertErrorCondition(Warning,"RTDataStorageSystem::CreateTimeVectorFromTimeWindow: No Acquisition point has been specified");
        return True;
    }

    acquisitionTimesBuffer = (int32 *)malloc(sizeof(int32)* acquisitionTimesBufferSize);
    if(acquisitionTimesBuffer==NULL){
        AssertErrorCondition(FatalError,"RTDataStorageSystem::CreateTimeVectorFromTimeWindow:: Error while allocating requiredSlowSample vector %d",acquisitionTimesBuffer);
        acquisitionTimesBufferSize = 0;
        return False;
    }

    // Builds the new time vector using the message header infos
    int time = 0;
    int j = 0;
    for(i=0;i<(CLOCKInit.nOfSequences);i++){
        uint32 interval = abs(CLOCKInit.samplingPeriodUsec[i]);
        // Check if there is a delay
        // (negative mantissa)
        if(CLOCKInit.samplingPeriodUsec[i]<0){
            // Increase the time in any case
            time += interval*CLOCKInit.nOfSamples[i];
            // else adds samples
        } else {
            for(int k=0;k<(CLOCKInit.nOfSamples[i]);k++){
                // Increase the time
                time += interval;
                acquisitionTimesBuffer[j] = time;
                // Next sample
                j++;
            }
        }
    }

    return True;

}

bool RTDataStorageSystem::ObjectLoadSetup(ConfigurationDataBase &info,StreamInterface *err){

    // Clean up the linked list holder
    CleanUpTimeVector();
    Reset();

    CDBExtended cdb(info);

    // Read number of fast acquisition total samples
    if(!cdb.ReadInt32(maxFastAcquisitionPoints,"MaxFastAcquisitionPoints")){
        AssertErrorCondition(Warning,"RTDataStorageSystem::ObjectLoadSetup: MaxFastAcquisitionPointshas not been specified. Assug 0.");
        maxFastAcquisitionPoints = 0;
    }

    // Copy maxFastAcquisitionPoints even in fastAcquisitionPointsLeft
    fastAcquisitionPointsLeft       = maxFastAcquisitionPoints;
    fastAcquisitionPointsToDo       = 0;

    if(!cdb.ReadInt32(pointsForSingleFastAcquisition,"PointsForSingleFastAcquisition")){
        AssertErrorCondition(Warning,"RTDataStorageSystem::ObjectLoadSetup: PointsForSingleFastAcquisition not been specified. Assuming 500.");
        pointsForSingleFastAcquisition = 500;
    }

    // Read data for slow acquisition time windows
    // DB Entry for the i-th time window
    int i = 0;
    CLOCKInit.nOfSequences   = 0;
    CLOCKInit.nOfReadSamples = 0;

    FString s;
    s.Printf("TimeWindow%d",i);

    while( (cdb->Exists(s.Buffer())) && (i < RTDataStorageMaxWindows) ){

        if(cdb->Move(s.Buffer())){
            // Read number of samples
            cdb.ReadInt32(CLOCKInit.nOfSamples[i],"NOfSamples",1);

            // Read slow data sample rate
            int32 usecPeriod = (int32)ceil(80000000.0/CLOCKInit.nOfSamples[i]);
            // The default value is 80/nOfSamples
            cdb.ReadInt32(CLOCKInit.samplingPeriodUsec[i],"UsecPeriod",usecPeriod);

            // Next time window
            i++;

            s = "";
            s.Printf("TimeWindow%d",i);

            cdb->MoveToFather();
        }
    }

    if(i > 0) CLOCKInit.nOfSequences = i;

    if (!CreateTimeVectorFromTimeWindow()){
        AssertErrorCondition(InitialisationError,"RTDataStorageSystem::ObjectLoadSetup: Failed to Create TimeVector From TimeWindow");
        return False;
    }

    return True;
}


void RTDataStorageSystem::TimeWindowsMenu(StreamInterface &in,StreamInterface &out){
    // empty input
    char buffer[128];
    uint32 size = sizeof(buffer);
    while(1){
        out.Printf(
                "###############################################################################\n"
                "     % 40s                              ##\n"
                "###############################################################################\n"
                ,"JPF TIME WINDOW SETUP MENU");
        out.Printf("S: Number Of Sequences = %d\n",CLOCKInit.nOfSequences);
        out.Printf("              nOfSamples samplingPeriod (usec)\n");
        for(int i=0;i<RTDataStorageMaxWindows;i++){
            char label[32];
            sprintf(label,"%c",i+'A');
            out.Printf("%s:        %10d     %10d\n",label,CLOCKInit.nOfSamples[i],CLOCKInit.samplingPeriodUsec[i]);
        }
        out.Printf("###############################################################################\n");
        out.Printf("0: EXIT\n");

        size = sizeof(buffer);
        if (!in.GetLine(buffer,size,False)) SleepMsec(100);
        else
            if (strlen(buffer)>0){
                char command = toupper(buffer[0]);
                if (command == '0') {
                    CreateTimeVectorFromTimeWindow();
                    return;
                } else{
                    if (command == 'S') {
                        out.Printf("insert new number of sequences\n");
                        in.GetLine(buffer,size,False);
                        int nseq = atoi(buffer);
                        if ((nseq > 0) && (nseq < RTDataStorageMaxWindows)){
                            CLOCKInit.nOfSequences = nseq;
                        } else {
                            out.Printf("number of sequences must be in [%i %i] range\n",0,RTDataStorageMaxWindows);
                        }
                    } else {
                        int index = command - 'A';
                        if ((index >= 0) && (index < RTDataStorageMaxWindows)){
                            out.Printf("insert new number of samples\n");
                            in.GetLine(buffer,size,False);
                            int nsam = atoi(buffer);
                            if ((nsam > RTMinSamplingPeriodUsec) && (nsam < RTMaxSamplingPeriodUsec)){
                                CLOCKInit.nOfSamples[index] = nsam;

                                out.Printf("insert new sampling period as microseconds\n");
                                in.GetLine(buffer,size,False);
                                int period = atoi(buffer);
                                if ((period > RTMinSamplingPeriodUsec) && (period < RTMaxSamplingPeriodUsec)){
                                    CLOCKInit.samplingPeriodUsec[index] = period;
                                } else {
                                    out.Printf("period usec must be in [%i %i] range\n",RTMinSamplingPeriodUsec,RTMaxSamplingPeriodUsec);
                                }
                            } else {
                                out.Printf("number of samples must be in [%i %i] range\n",RTMinSamplingPeriodUsec,RTMaxSamplingPeriodUsec);
                            }
                        }
                    }
                }
            }
    }
    CreateTimeVectorFromTimeWindow();
    return ;
}


bool RTDataStorageSystem::GetSignalData(int32 signalOffset, GCRTemplate<SignalInterface> &signal){

    //////////////////////////////////////////
    // Allocate buffer for copying the data //
    /////////////////////////////////////////

    uint32 dataSize = ListSize();
    if(dataSize == 0){
        AssertErrorCondition(FatalError,"RTDataStorageSystem::GetSignalData: No Data Has been collected");
        return False;
    }

    ////////////////////////////////////////////////////////////
    // Copy Data from RTCollectionBuffers to the local buffer //
    ////////////////////////////////////////////////////////////
    if(!signal.IsValid()){
        AssertErrorCondition(FatalError,"RTDataStorageSystem::GetSignalData: Signal of SignalInterface Type is not valid");
        return False;
    }

    uint32 *output           = (uint32 *)signal->Buffer();
    uint32 *outputEnd        = output + dataSize;
    RTCollectionBuffer *buf = (RTCollectionBuffer *)List();

    while ((output < outputEnd) && (buf!=NULL)){
        // +2 is to skip the local copy of the time and of the fast trigger request
        const uint32 *ddb = (const uint32 *)buf->Data();
        const uint32 *in  =  ddb + signalOffset + 2;
        // Copy Data
        *output          = *in;
        // Increment pointers
        buf              = buf->Next();
        output++;
    }

    return True;
}



RTCollectionBuffer *RTDataStorageSystem::StoreData(RTCollectionBuffer &rtBuffer,bool fastTrigger){

    // Check the fastTrigger flag and set pointsToDo != 0
    // if there are fast acquisition time slots available
    if(fastTrigger){
        // Update fastAcqPoints adding pointsToDo
        fastAcquisitionPointsLeft += fastAcquisitionPointsToDo;
        // Set pointsToDo != if there are fast acq points available
#if !defined(_VXWORKS)
        fastAcquisitionPointsToDo = minimum(fastAcquisitionPointsLeft,pointsForSingleFastAcquisition);
#else
        fastAcquisitionPointsToDo = min(fastAcquisitionPointsLeft,pointsForSingleFastAcquisition);
#endif
        fastAcquisitionPointsLeft -= fastAcquisitionPointsToDo;
    }

    bool acquire = False;
    // If pointsToDo != 0 force the acquisition
    if(fastAcquisitionPointsToDo != 0){
        // Update pointsToDo
        fastAcquisitionPointsToDo--;
        acquire = True;
    }

    // Packet Time
    uint32 usecTime = rtBuffer.PacketUsecTime();

    // slow acquisition
    if(acquisitionTimesBuffer != NULL){
        if((acquisitionTimesBufferIndex < acquisitionTimesBufferSize)&&
           (usecTime >= acquisitionTimesBuffer[acquisitionTimesBufferIndex])&& 
           (usecTime <= acquisitionTimesBuffer[acquisitionTimesBufferSize - 1])){

            acquisitionTimesBufferIndex++;
            acquire = True;
        }
    }

    if (acquire){
        // Store the sample
        FastQueueInsertSingle(rtBuffer);
        CLOCKInit.nOfReadSamples++;
        return NULL;
    }

    // Sample not stored
    return &rtBuffer;
}





bool RTDataStorageSystem::ObjectDescription(StreamInterface &s,bool full,StreamInterface *err){

    s.Printf("MaxFastAcquisitionPoints = %d\n", maxFastAcquisitionPoints);
    s.Printf("PointsForSingleFastAcquisition = %d\n", pointsForSingleFastAcquisition);
    s.Printf("PointsForAcquisition= %d\n", acquisitionTimesBufferSize);
    if(full){
        for(int i = 0; i < CLOCKInit.nOfSequences; i++){
            s.Printf("TimeWindow[%d]: %d Samples @ %d usec \n", i, CLOCKInit.nOfSamples[i], CLOCKInit.samplingPeriodUsec[i]);
        }
    }

    return True;
}

void RTDataStorageSystem::HTMLInfo(HttpStream &hStream) {
    hStream.Printf("<table border=\"1\"><tr><th>Time window n.</th><th>NumOfSamples</th><th>UsecPeriod</th><th>Start</th><th>End</th></tr>\n");

    int i                = 0;
    int totalSamples     = 0;
    int start            = 0;
    int end              = 0;

    FString white  = "#FFFFFF";
    FString green  = "#00FF00";
    FString yellow = "#FFFF00";
    FString red    = "#FF0000";
    FString bg     = white;
    for (i=0;i<CLOCKInit.nOfSequences;i++) {
        end = start + CLOCKInit.nOfSamples[i] * abs(CLOCKInit.samplingPeriodUsec[i]);
        bg = white;
        if(acquisitionTimesBufferIndex < acquisitionTimesBufferSize){
            if(acquisitionTimesBuffer[acquisitionTimesBufferIndex] > (start + CLOCKInit.samplingPeriodUsec[i]) && acquisitionTimesBuffer[acquisitionTimesBufferIndex] <= (end + abs(CLOCKInit.samplingPeriodUsec[i]))){
                bg = green;
            }
        }
        else{
            bg = green;
        }
        if(CLOCKInit.samplingPeriodUsec[i] > 0){
            hStream.Printf("<tr bgcolor=\"%s\"><td>%d</td><td>%d</td><td>%d</td><td>%d</td><td>%d</td></tr>\n",bg.Buffer(),i,CLOCKInit.nOfSamples[i],CLOCKInit.samplingPeriodUsec[i], start, end);
            totalSamples     += CLOCKInit.nOfSamples[i];
        }
        else{
            hStream.Printf("<tr bgcolor=\"%s\"><td>%d</td><td>(0) %d</td><td>%d</td><td>%d</td><td>%d</td></tr>\n",bg.Buffer(),i,CLOCKInit.nOfSamples[i],CLOCKInit.samplingPeriodUsec[i], start, end);
        }
        start = end;
    }
    hStream.Printf("</table>");
    hStream.Printf("<h2>Total Samples = %d</h2>\n", totalSamples);
    hStream.Printf("<h2>Total already read Samples = %d</h2>\n", CLOCKInit.nOfReadSamples);
}

OBJECTREGISTER(RTDataStorageSystem,"$Id: RTDataStorageSystem.cpp,v 1.14 2011/02/11 07:59:31 astephen Exp $")
