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
#if !defined(UDP_PROXY_PLUGIN)
#define UDP_PROXY_PLUGIN

#include "LoggerMessageQueue.h"
#include "LoggerMessage.h"
#include "GCReferenceContainer.h"
#include "Threads.h"
#include "UDPLoggerPlugin.h"
#include "File.h"
#include "Directory.h"
#include "CDBExtended.h"
#include "HtmlStream.h"

/**Sorts a directory by access time*/
static int32 DirectoryTimeSortFilter(LinkedListable *data1,LinkedListable *data2){
    DirectoryEntry *de1 = (DirectoryEntry *)data1;
    DirectoryEntry *de2 = (DirectoryEntry *)data2;
    if(de1->Time() == de2->Time()){
        return 0;
    }

    return (de1->Time() > de2->Time()) ? 1 : -1;
}

OBJECT_DLL(UDPLoggerFile)
/** @file
    Saves all the received messages in specific files*/

/**
 * Describes a log file
 */
class UDPLogFile : public GCNamedObject {
private:
    File        logFile;

    HtmlStream  htmlStream;

    uint32      lastAccessTime;

    FString     simpleName;

    void PrintCurrentTime(char *stime){
        uint32 errorTime = time(NULL);
        sprintf(stime,"%s",ctime((time_t *)&errorTime));
        stime[strlen(stime)-1]=0;
        char *c = stime;
        while (*c != 0){
            if (*c == ':') *c = '.';
            if (*c == ' ') *c = '_';
            c++;
        }
    }

public:

    UDPLogFile() : htmlStream(logFile){
    }

    void Init(FString baseDir, FString sourceAddress, uint32 taskID){
        char stime[64];
        PrintCurrentTime(stime);

        FString logFileName;
        simpleName.Printf("Proc%i@%s[%s]_log.html", taskID, sourceAddress.Buffer(), stime);
        logFileName.Printf("%s%s", baseDir.Buffer(), simpleName.Buffer());
        logFile.SetOpeningModes(createOverwrite | localitySequential | shareModeNoW | accessModeW) ;
        logFile.Open(logFileName.Buffer());
        FString objName;
        objName.Printf("%i@%s", taskID, sourceAddress.Buffer());
        SetObjectName(objName.Buffer());
    }

    virtual ~UDPLogFile(){
        AssertErrorCondition(Information, "Closing file: %s\n", logFile.FileName());
        //htmlStream will automatically flush at deconstruction
        logFile.Close();
    }

    /**
     * @return The last time it was used
     */
    uint32  GetLastAccessTime(){
        return lastAccessTime;
    }

    File& GetLogFile(){
        return logFile;
    }

    HtmlStream& GetHtmlStream(){
        return htmlStream;
    }

    const FString& GetSimpleName(){
        return simpleName;
    }

    void Ping(){
        lastAccessTime = time(NULL);
    }
};

/** Thread which is responsible for managing the files*/
void UDPLoggerFileManager(void *args);

// to allow deleting of dead clients
class UDPLogFileFilter: public SearchFilterT<GCReference>{
    FString name;

public:
    UDPLogFileFilter(FString sourceAddress, int taskID){
        name.Printf("%i@%s", taskID, sourceAddress.Buffer());
    }

    virtual ~UDPLogFileFilter(){};

    bool Test(GCReference data){
        GCRTemplate<GCNamedObject> gcno;
        gcno = data;

        if (!gcno.IsValid())
            return False;

        return name == gcno->Name();
    }
};


class UDPLoggerFile : public UDPLoggerPlugin{
private:
OBJECT_DLL_STUFF(UDPLoggerFile)

    /**The maximum time for which a file is allowed not to be used*/
    int32 maxNotUsedTimeSecs;

    /**The maximum number of log files allowed*/
    int32 maxNumLogFiles;

    /**Holds the file list to delete*/
    Directory *toDeleteFileList;

    /** the directory where the files are saved */
    FString baseDir;

    /**Checks the directory log size*/
    void CheckNumberOfLogFiles(){
        DirectoryEntry *de =  NULL;
        int i = toDeleteFileList->ListSize();
        while(i > maxNumLogFiles){
            de = (DirectoryEntry *)toDeleteFileList->ListExtract((uint32)0);
            AssertErrorCondition(Information, "Deleting %s = %d\n", de->Name(), de->Time());
            FileEraseFile("%s%s", baseDir.Buffer(), de->Name());
            i--;
        }
    }

public:

    UDPLoggerFile(){
    }

    virtual ~UDPLoggerFile(){
        //Clean up will remove all entries which will call the correct
        //deconstructors and close all the files
        CleanUp();
        CheckNumberOfLogFiles();
        delete toDeleteFileList;
    }

    virtual     bool                ObjectLoadSetup(
            ConfigurationDataBase & info,
            StreamInterface *       err){

        bool ret = UDPLoggerPlugin::ObjectLoadSetup(info, err);
        CDBExtended &cdbx = (CDBExtended &)info;

        if (!cdbx.ReadInt32(maxNotUsedTimeSecs, "MaxNotUsedTimeSecs", 30)){
            AssertErrorCondition(Information, "ObjectLoadSetup:using default MaxNotUsedTimeSecs: %d", maxNotUsedTimeSecs);
        }

        if (!cdbx.ReadInt32(maxNumLogFiles, "MaxNumLogFiles", 10)){
            AssertErrorCondition(Information, "ObjectLoadSetup:using default MaxNumLogFiles: %d", maxNumLogFiles);
        }

        if (!cdbx.ReadBString(baseDir,"BaseDir", "./")){
            AssertErrorCondition(Information, "ObjectLoadSetup:using default BaseDir: %s", baseDir.Buffer());
        }

        if(baseDir.Buffer()[baseDir.Size() - 1] != '/'){
            if(baseDir.Buffer()[baseDir.Size() - 1] != '\\'){
                baseDir += '/';
                AssertErrorCondition(Information, "ObjectLoadSetup:Going to add a / to the BaseDir: %s", baseDir.Buffer());
            }
        }

        toDeleteFileList = new Directory(baseDir.Buffer(), "Proc*.html");
        toDeleteFileList->ListBSort(&DirectoryTimeSortFilter);

        ++maxNumLogFiles;
        CheckNumberOfLogFiles();
        Threads::BeginThread(UDPLoggerFileManager, this);
        return True;
    }

    virtual bool TextMenu(
                    StreamInterface &               in,
                    StreamInterface &               out)
    {
        out.Printf("\n\n\n\nThe size of the internal logger queue is: %d\n", loggerMessageQ.Size());
        out.Printf("\n\n\n\nNumber of open files is: %d\n", Size());
        int currentTime = 0;
        out.Printf("N  %-75s %s\n", "FileName", "Number of seconds since last access");
        for(int i=0; i<Size(); i++){
            GCRTemplate<UDPLogFile> logFile = Find(i);
            currentTime = time(NULL) - logFile->GetLastAccessTime();
            out.Printf("%d  %-75s %d\n", (i + 1), logFile->GetSimpleName().Buffer(), currentTime);
        }

        if(toDeleteFileList->ListSize() > 0){
            out.Printf("\n\nNext top ten files to be permanently deleted are:\n");
            for(int i=0; i<10 && i<toDeleteFileList->ListSize(); i++){
                out.Printf("% 2d: %s\n", (i+1), ((DirectoryEntry *)toDeleteFileList->ListPeek(i))->Name());
            }
        }

        //SXNull passed by the http server has always size 0
        if(in.Size() != 0){
            WaitRead(in, out);
        }
        return True;
    }

    virtual void ProcessMessage(GCRTemplate<LoggerMessage> msg);
};

#endif
