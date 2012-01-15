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

#include "HttpThreadListResource.h"
#include "HttpUtilities.h"
#include "HtmlStream.h"

OBJECTLOADREGISTER(HttpThreadListResource,"$Id: HttpThreadListResource.cpp,v 1.5 2011/02/07 15:35:04 dalves Exp $")

void HttpThreadListResource::PrintThreadStateAsString(HtmlStream &hmStream, uint32 threadState){
    BString str = "Unknown";
    if(threadState == THREAD_STATE_UNKNOWN){
        hmStream.Printf("%s", str.Buffer());
        return;
    }

    str = "";
    if((threadState & THREAD_STATE_READY) == THREAD_STATE_READY){
        str += "Ready | ";
    }
    if((threadState & THREAD_STATE_PEND) == THREAD_STATE_PEND){
        str += "Pending | ";
    }
    if((threadState & THREAD_STATE_BLOCKED) == THREAD_STATE_BLOCKED){
        str += "Blocked  | ";
    }
    if((threadState & THREAD_STATE_SEM) == THREAD_STATE_SEM){
        str += "Semaphore | ";
    }
    if((threadState & THREAD_STATE_SUSP) == THREAD_STATE_SUSP){
        str += "Suspended | ";
    }
    if((threadState & THREAD_STATE_DELAY) == THREAD_STATE_DELAY){
        str += "Delayed | ";
    }
    if((threadState & THREAD_STATE_RUN) == THREAD_STATE_RUN){
        str += "Running | ";
    }
    if((threadState & THREAD_STATE_DEAD) == THREAD_STATE_DEAD){
        str += "Dead | ";
    }
    if((threadState & THREAD_STATE_TOUT) == THREAD_STATE_TOUT){
        str += "Timeout | ";
    }

    if(str.Size() > 3){
        str.SetSize(str.Size() - 3);
    }
    else{
        str = "Unknown";
    }

    hmStream.Printf("%s", str.Buffer());
}


bool HttpThreadListResource::ProcessHttpMessage(HttpStream &hStream) {

    hStream.Printf( "<html><head>" );
    hStream.Printf( "<style type=\"text/css\">\n" );
    hStream.Printf("%s\n", css);
    hStream.Printf( "</style>\n" );
    hStream.Printf("<TITLE>Thread list</TITLE>\n");
    hStream.Printf("</head>");
    hStream.Printf("<BODY BGCOLOR=\"#ffffff\">""<H1>Thread list</H1>\n");
    hStream.SSPrintf("OutputHttpOtions.Content-Type","text/html");
    HtmlStream hmStream(hStream);
    hmStream.SSPrintf(HtmlTagStreamMode,"P");
    hmStream.Printf("Number of online CPUs: ");
    if (ProcessorsAvailable() != -1){
        hmStream.Printf("%d", ProcessorsAvailable());
    } else {
        hmStream.Printf("?");
    }
    hmStream.SSPrintf(HtmlTagStreamMode,"/P");
    hmStream.SSPrintf(HtmlTagStreamMode,"TABLE CLASS=\"bltable\"");
    hmStream.SSPrintf(HtmlTagStreamMode,"TR");
    hmStream.SSPrintf(HtmlTagStreamMode,"TH");
    hmStream.Printf("Name");
    hmStream.SSPrintf(HtmlTagStreamMode,"/TH");
    hmStream.SSPrintf(HtmlTagStreamMode,"TH");
    hmStream.Printf("Thread ID");
    hmStream.SSPrintf(HtmlTagStreamMode,"/TH");
    hmStream.SSPrintf(HtmlTagStreamMode,"TH");
    hmStream.Printf("OS Thread ID");
    hmStream.SSPrintf(HtmlTagStreamMode,"/TH");
    hmStream.SSPrintf(HtmlTagStreamMode,"TH");
    hmStream.Printf("State");
    hmStream.SSPrintf(HtmlTagStreamMode,"/TH");
    hmStream.SSPrintf(HtmlTagStreamMode,"TH");
    hmStream.Printf("Priority");
    hmStream.SSPrintf(HtmlTagStreamMode,"/TH");
    hmStream.SSPrintf(HtmlTagStreamMode,"TH");
    hmStream.Printf("Allowed on CPUs");
    hmStream.SSPrintf(HtmlTagStreamMode,"/TH");
    hmStream.SSPrintf(HtmlTagStreamMode,"/TR");
    int i =0;
    for (i = 0;i < TDB_NumberOfThreads() ; i ++) {

        ThreadInitialisationInterface tii;
        bool ok = False;
        TDB_Lock();
        ok = TDB_GetInfo(tii,i);
        TDB_UnLock();

        if (ok) {
            hmStream.SSPrintf(HtmlTagStreamMode,"TR");
            hmStream.SSPrintf(HtmlTagStreamMode,"TD");
            hmStream.Printf("%s", tii.GetThreadName());
            hmStream.SSPrintf(HtmlTagStreamMode,"/TD");
            hmStream.SSPrintf(HtmlTagStreamMode,"TD");
            hmStream.Printf("0x%x", tii.tid);
            hmStream.SSPrintf(HtmlTagStreamMode,"/TD");
            hmStream.SSPrintf(HtmlTagStreamMode,"TD");
            hmStream.Printf("%d", tii.osTid);
            hmStream.SSPrintf(HtmlTagStreamMode,"/TD");
            hmStream.SSPrintf(HtmlTagStreamMode,"TD");
            PrintThreadStateAsString(hmStream, ThreadsGetState(tii.tid));
            hmStream.SSPrintf(HtmlTagStreamMode,"/TD");
            hmStream.SSPrintf(HtmlTagStreamMode,"TD");
            hmStream.Printf("%d", ThreadsGetPriority(tii.tid));
            hmStream.SSPrintf(HtmlTagStreamMode,"/TD");
            hmStream.SSPrintf(HtmlTagStreamMode,"TD");
            int cpus = ThreadsGetCPUs(tii.tid);
            if(cpus == -1){
                hmStream.Printf("?");
            }
            else{
                if (ProcessorsAvailable() != -1){
                    FString cpuStr = "";
                    for(int i=0; i<ProcessorsAvailable(); i++){
                        if(i == 0){
                            if((cpus & 1) == 1){
                                cpuStr.Printf("0,");
                            }
                        }
                        else if((cpus & (1 << i)) == (1 << i)){
                            cpuStr.Printf("%d,", i);
                        }
                    }
                    if(cpuStr.Size() > 0){
                        cpuStr.SetSize(cpuStr.Size() - 1);
                        hmStream.Printf("%s", cpuStr.Buffer());
                    }
                    else{
                        hmStream.Printf("?");
                    }
                }
                else{
                    hmStream.Printf("%d (masked)", cpus);
                }
            }
            hmStream.SSPrintf(HtmlTagStreamMode,"/TD");
            hmStream.SSPrintf(HtmlTagStreamMode,"/TR");
        }
    }
    //copy to the client
    hStream.WriteReplyHeader(True);
    return True;
}




