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
#include "UDPLoggerFile.h"
#include "Sleep.h"

OBJECTLOADREGISTER(UDPLoggerFile, "$Id: UDPLoggerFile.cpp,v 1.4 2008/05/29 10:28:55 aneto Exp $")


/** This function will be called whenever there is a new message to process available*/
void UDPLoggerFile::ProcessMessage(GCRTemplate<LoggerMessage> loggerMsg){
    
    UDPLogFileFilter filter(loggerMsg->GetSourceAddress(), loggerMsg->GetPID());
    GCRTemplate<UDPLogFile> logFile = Find(&filter);
    if(logFile.IsValid() == True){        
        loggerMsg->FormatMessage(logFile->GetHtmlStream());
        logFile->Ping();
    }
    else{
        GCRTemplate<UDPLogFile> logFile(GCFT_Create);
        logFile->Init(baseDir, loggerMsg->GetSourceAddress(), loggerMsg->GetPID());
        /*if(!Lock()){
            AssertErrorCondition(Warning, "ProcessMessage Lock failed!");
        }*/
        Insert(logFile);
        /*if(!UnLock()){
            AssertErrorCondition(Warning, "ProcessMessage Lock failed!");
        }*/
        loggerMsg->FormatMessage(logFile->GetHtmlStream());
        logFile->Ping();
    }
}

/** Thread which is responsible for managing the files*/
void UDPLoggerFileManager(void *args){
    UDPLoggerFile *udpLoggerFile = (UDPLoggerFile *)args;
    while(udpLoggerFile->IsAlive()){
        SleepSec((float)udpLoggerFile->maxNotUsedTimeSecs);    
        uint32 cTime = time(NULL);
        for(int i=0; i<udpLoggerFile->Size(); i++){
            GCRTemplate<UDPLogFile> logFile = udpLoggerFile->Find(i);
            if(logFile.IsValid()){
                if((cTime - logFile->GetLastAccessTime()) > udpLoggerFile->maxNotUsedTimeSecs){
                    /*if(!udpLoggerFile->Lock()){
                        udpLoggerFile->AssertErrorCondition(Warning, "UDPLoggerFileManager Lock failed!");
                    }*/
                    udpLoggerFile->AssertErrorCondition(Warning, "Going to close file: %s because no messages were received on the last %d seconds", logFile->GetLogFile().FileName(), udpLoggerFile->maxNotUsedTimeSecs);
                    udpLoggerFile->toDeleteFileList->ListAdd(new DirectoryEntry(udpLoggerFile->toDeleteFileList, logFile->GetSimpleName().Buffer()));
                    //Deconstructor should close file
                    udpLoggerFile->Remove(i);                
                    /*if(!udpLoggerFile->UnLock()){
                        udpLoggerFile->AssertErrorCondition(Warning, "UDPLoggerFileManager UnLock failed!");
                    }*/
                    udpLoggerFile->CheckNumberOfLogFiles();
                }
            }
        }
    }
}
