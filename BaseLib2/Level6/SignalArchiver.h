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

#if !defined(SIGNAL_ARCHIVER_H)
#define SIGNAL_ARCHIVER_H

#include "ConfigurationDataBase.h"
#include "CDBExtended.h"
#include "MessageHandler.h"
#include "HttpDirectoryResource.h"
#include "SignalInterface.h"
#include "SignalMessageInterface.h"
#include "File.h"

/**
 * @file 
 * @brief Automatically store signals in disk
 *
 * This class automatically stores signals retrieved with SignalMessageInterface
 * in a disk directory
 */

/**
 * Thread which stores the signals, to avoid blocking the messaging mechanism
 */
extern "C"{
    void SignalArchivingFn(void *args);
}

enum StorageMode{
    TEXT,
    BINARY,
    MATLAB
};

OBJECT_DLL(SignalArchiver)

class SignalArchiver:public SignalMessageInterface,
                     public HttpInterface{
OBJECT_DLL_STUFF(SignalArchiver)

    friend void SignalArchivingFn(void *args);
    /**
     * Full path for the directory where the data should be stored
     */
    FString archiveDirectoryPath;

    /**
     * If specified, use this as the relative directory for a given run, otherwise it will use the current time
     * It can also be set with a message, whose content is the relative path
     */
    FString relativeDirectoryPath;

    /**
     * The message id for which the content is the relative path
     */
    int32   relativeDirectoryPathMessageCode;

    /**
     * Renders the signals
     */
    HttpDirectoryResource httpDirResource;

    /**
     * Stores all the retrieved signals in the directory specified by archiveDirectoryPath
     * @return True if the saving is successful
     */
    bool StoreSignals();

    /**
     * The storage mode
     */
    StorageMode storageMode;

    /**
     * Stores a signal in matlab format
     * @param signal the signal to be stored
     * @param originalSignalName how the signal is named in the message
     * @param matlabVarName how the variable is going to be called in matlab
     * @param file the file where data is to be stored
     * @return True if no error were detected
     */
    bool SaveInMatlab(GCRTemplate<SignalInterface> signal, FString &originalSignalName, FString &matlabVarName, File &file);

    /**
     * Returns the current date and time in a string format
     * @param timeStr the string where the date is written to
     */
    void GetDateTimeString(FString &timeStr);

    /**
     * Archiving thread tid
     */
    TID archiveTID;    

    /**
     * To synchronise with the archiving thread
     */
    EventSem archiveSem;
    bool     running;

public:
    /** */
    virtual ~SignalArchiver(){
        running       = False;
        archiveSem.Post();
        int32 counter = 0;
        while(!running){
            SleepSec(100e-3);
            if(counter > 10){
                break;
            } 
        }
        if(counter > 10){
            AssertErrorCondition(FatalError, "%s::Thread did not terminate on its own... killing thread", Name());
        }
    };

    /** */
    SignalArchiver(){
        archiveDirectoryPath.SetSize(0);
        relativeDirectoryPath.SetSize(0);
        storageMode                      = MATLAB;
        archiveTID                       = 0;
        relativeDirectoryPathMessageCode = -1;
        archiveSem.Create();
    }

    /**
     * The HTTP entry point (it is delegated to an HttpDirectoryResource
     */
    virtual bool ProcessHttpMessage(HttpStream &hStream){
        return httpDirResource.ProcessHttpMessage(hStream);
    }

    virtual     bool                ObjectLoadSetup(
            ConfigurationDataBase &     info,
            StreamInterface *           err){

        bool ret = True;
        ret = ret && SignalMessageInterface::ObjectLoadSetup(info,err);

        CDBExtended cdbx(info);
        FString tmp;
        if (!cdbx.ReadFString(tmp, "StorageMode")){
            SignalMessageInterface::AssertErrorCondition(InitialisationError, "%s::ObjectLoadSetup StorageMode not specified", SignalMessageInterface::Name());
        }
        if(tmp == "TEXT"){
            storageMode = TEXT;
        }
        else if(tmp == "BINARY"){
            storageMode = BINARY;
        }
        else if(tmp == "MATLAB"){
            storageMode = MATLAB;
        }
        else{
            SignalMessageInterface::AssertErrorCondition(InitialisationError, "%s::ObjectLoadSetup Invalid StorageMode", SignalMessageInterface::Name());
            return False;
        }
        if(!cdbx.ReadFString(archiveDirectoryPath, "ArchiveDirectoryPath")){
            SignalMessageInterface::AssertErrorCondition(InitialisationError, "%s::ObjectLoadSetup ArchiveDirectoryPath must be specified", SignalMessageInterface::Name());
        }
    
        int32 cpuMask;
        if(!cdbx.ReadInt32(cpuMask, "CPUMask")){
            archiveTID = Threads::BeginThread((void (__thread_decl *)(void *))&SignalArchivingFn,this);
        }
        else{
            archiveTID = Threads::BeginThread((void (__thread_decl *)(void *))&SignalArchivingFn,this, THREADS_DEFAULT_STACKSIZE, Name(), XH_NotHandled, cpuMask);
        }
        cdbx.WriteFString(archiveDirectoryPath, "BaseDir");

        cdbx.ReadFString(relativeDirectoryPath, "RelativeDirectoryPath");
        cdbx.ReadInt32(relativeDirectoryPathMessageCode, "RelativeDirectoryPathMessageCode");
        return httpDirResource.ObjectLoadSetup(cdbx, err);
    }

    /**
     * Processes the incoming message and checks for the content.
     * If the code is relativeDirectoryPathMessageCode then it will set the relativeDirectoryPath = to the message content
     * If the content is STORESIGNALS, data storage is triggered, otherwise
     * SignalMessageInterface::ProcessMessage is called
     */
    virtual bool ProcessMessage(GCRTemplate<MessageEnvelope> envelope){
        if (!envelope.IsValid()) {
            SignalMessageInterface::AssertErrorCondition(FatalError, "%s::ProcessMessage: envelope is not valid!!", SignalMessageInterface::Name());
            return False;
        }

        GCRTemplate<Message> msg = envelope->GetMessage();
        if (!msg.IsValid()) {
            SignalMessageInterface::AssertErrorCondition(FatalError, "%s::ProcessMessage: (Message)msg is not valid!!", SignalMessageInterface::Name());
            return False;
        }
        FString content = msg->Content();
    
        if(msg->GetMessageCode().Code() == relativeDirectoryPathMessageCode){
            relativeDirectoryPath = content;
        } 
        if(content == "STORESIGNALS"){
            return StoreSignals();
        }
        else{
            return SignalMessageInterface::ProcessMessage(envelope);
        }
    }
};

#endif

