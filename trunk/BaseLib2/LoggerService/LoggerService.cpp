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

#include "LoggerService.h"
#include "FastResourceContainer.h"
#include "CStream.h"
#include "Processes.h"
#include "Threads.h"
#include "ObjectRegistryItem.h"
#include "QueueHolder.h"
#include "ErrorSystemInfo.h"
#include "MutexSem.h"
#include "UDPSocket.h"
#include "Object.h"
#include "Console.h"
#include "Sleep.h"
#include "URL.h"

const int32 LSPageSize = 128;
const int32 LSNOfPages = 512;
const int32 LSNOfErrors = 64;

//
class LoggerPage:public Queueable{
public:
    //
    char page[LSPageSize];

    //
    int  usedSize;

    //
    LoggerPage(){
        usedSize = 0;
    }

    //
    ~LoggerPage(){
    }
};


//
void LoggerServiceBufferFN(CStream *cs);

//
void LoggerServiceThreadFN(void *arg);


// ex static
class LoggerService{

    // unused pages
    FastResourceContainer   pagesIndex;

    //
    FastResourceContainer   logsIndex;

    //
    LoggerPage              *pages;

    // Tid is != 0 if the thread is running
    TID                     logProcessingThread;

    // if True the Thread has to continue processing errors
    bool                    hasToContinue;

    // the thread will be left to continue until this number is growing!
    int                     processed;

    // a circular buffer of errors
    // on one side add the new ones, on the other recycle the old ones
    QueueHolder             storedErrors;

// ERROR PROCESSING DATA

    // the elements contained in storedErrors
    ErrorSystemInfo         *LSErrors;

    /** protects access to storedErrors */
    MutexSem                mux;

    /** used to relay messages */
    UDPSocket               sock;

//    /** server url */
//    URL                     loggerServerUrl;
    FString                 loggerServerUrl;

public:

    bool ThreadHasToContinue()
    {
        return hasToContinue;
    }

    /** */
    LoggerService();

    /** */
    TID StartThread();

    /** */
    void StopThread();

    /** */
    ~LoggerService();

    /** */
    LoggerPage *GetPage();

    /** */
    void ReturnPage(LoggerPage *page);

    /** */
    void AddLogEntry(LoggerPage *page);

    /** */
    LoggerPage *GetLogEntry();

    /** */
    bool HasPendingErrorEntries()
    {
        return (logsIndex.Size() > 0);
    }

// ERROR PROCESSING

    /** */
    void Lock()
    {
        mux.Lock();
    }

    /** */
    void UnLock()
    {
        mux.UnLock();
    }

    /** */
    void ProcessError(Streamable &in);

    /** */
    bool SetRemoteLogger();

    /** */
    bool LastError(ErrorSystemInfo &info,int32 index);

    /** */
    void ThreadFinished();

};

/* Global instance of logger service
    on initialisation it will self install
    Since this is a global object it will initialise before Main() is called
*/
LoggerService LoggerServiceInstance;



/****************************************************/
/****************************************************/

    //
LoggerService::LoggerService():
    logsIndex(LSNOfPages,True),
    pagesIndex(LSNOfPages,False)
{
    hasToContinue       = False;
    pages               = new LoggerPage[LSNOfPages];
    if (pages == NULL){
        printf("FAILED ALLOCATING PAGES FOR LOGGERSERVICE !!!! \n");
        return;
    }
    LSErrors            = new ErrorSystemInfo[LSNOfErrors];
    if (LSErrors == NULL){
        printf("FAILED ALLOCATING ERROR FOR LOGGERSERVICE !!!! \n");
        delete[] pages;
        pages = NULL;
        return;
    }

    logProcessingThread = 0;

    SleepMsec(200);

    sock.Open();

#if defined LOGGER_AUTOSTART
    StartThread();

    LSSetUserAssembleErrorMessageFunction(LSAssembleErrorMessage);

    LSSetUserAssembleISRErrorMessageFunction(LSAssembleErrorMessage);
#endif
}

TID LoggerService::StartThread()
{
    if (logProcessingThread != 0) return  logProcessingThread;
	    for (int i=0;i<LSNOfErrors;i++){
        storedErrors.QueueAdd(&LSErrors[i]);
    }
    mux.Create();
    hasToContinue       = True;
    logProcessingThread = Threads::BeginThread(LoggerServiceThreadFN,this,THREADS_DEFAULT_STACKSIZE,"LOGGERSERVICE");
    if (logProcessingThread == 0) hasToContinue = False;
    return logProcessingThread;
}

void LoggerService::StopThread()
{
    int oldprocessed = processed;
    while(processed != oldprocessed) {
        oldprocessed = processed;
        SleepMsec(20);
    }

    hasToContinue = False;
    int i = 0;
    // In RTAI killing the thread creates problem with FCOMM. Added #ifdef to let the thread die by itself
    for(i = 1; (i < 20) && (logProcessingThread != 0); i++)SleepMsec(1);
#ifndef _RTAI
    if(logProcessingThread != 0){
        Threads::Kill(logProcessingThread);
    }
#endif

    for (i=0;i<LSNOfErrors;i++){
        storedErrors.QueueExtract();
    }
    mux.Close();
}

//
LoggerService::~LoggerService()
{
#if !defined (_RTAI)
    StopThread();
#endif
if (pages != NULL)delete []pages;
if (LSErrors != NULL)delete []LSErrors;
    pages = NULL;
    LSErrors = NULL;
}


//
LoggerPage *LoggerService::GetPage()
{
    if(pages == NULL)return NULL;
    int pageNo = pagesIndex.Take();
    if ((pageNo >=0 )&&(pageNo < LSNOfPages)){
//fprintf(stderr,"getting page %i\n",pageNo);
        return &pages[pageNo];
    }
    return NULL;
}

//
void LoggerService::ReturnPage(LoggerPage *page)
{
    if(pages == NULL)return;
    if (page == NULL) return;
    page->SetNext(NULL);
    int pageNo = page - pages;
    if ((pageNo >=0 )&&(pageNo < LSNOfPages)){
//fprintf(stderr,"returning page %i\n",pageNo);
        pagesIndex.Return(pageNo);
    }
}

//
void LoggerService::AddLogEntry(LoggerPage *page)
{
    if(pages == NULL)return;
    int pageNo = page - pages;
    if ((pageNo >=0 )&&(pageNo < LSNOfPages)){
        logsIndex.Return(pageNo);
    }
}

//
LoggerPage *LoggerService::GetLogEntry()
{
    if(pages == NULL)return NULL;
    int pageNo = logsIndex.Take();
    if ((pageNo >=0 )&&(pageNo < LSNOfPages)){
        return &pages[pageNo];
    }
    return NULL;
}

//
void LoggerService::ProcessError(Streamable &in)
{
    Lock();
    ErrorSystemInfo *error = (ErrorSystemInfo *)storedErrors.QueueExtract();
    UnLock();

    in.Seek(0);
    error->Load(in);

    uint32 errorAction = error->ErrorAction();

    if (errorAction & onErrorReportConsoleES){
        Console con;
        error->ComposeToText(con);
    }
    if (errorAction & onErrorRemoteLogES){
        // check if changed
        SetRemoteLogger();

        // send information
        error->Send(sock);
    }
    if (errorAction & onErrorQuitES){
#ifndef _RTAI
        exit(-1);
#else
        printf("RTAI SERIOUS ERROR @ LOGGERSERVICE\n");
#endif
    }
    if (errorAction & onErrorRememberES){
        Lock();
        storedErrors.QueueAdd(error);
        UnLock();
    } else {
        Lock();
        storedErrors.QueueInsert(error);
        UnLock();
    }
}

//
bool LoggerService::SetRemoteLogger()
{
    const char *newServer = GetLoggerServerURL();
    // To avoid during destruction that the static BString loggerSertver is
    // destroyed before the thread is killed
    if(newServer == NULL)             return True;
    if (loggerServerUrl == newServer) return True;

    loggerServerUrl = newServer;
    loggerServerUrl.Seek(0);
    URL url;
    url.Load(loggerServerUrl);
    CStaticAssertErrorCondition(Information,"Switching log server to %s:%i",url.Server(),url.Port());
//printf("Switching log server to %s:%i",url.Server(),url.Port());
    if (!sock.Connect(url.Server(),url.Port()))return False;
    return True;
}


//
bool LoggerService::LastError(ErrorSystemInfo &info,int32 index)
{
    Lock();
    ErrorSystemInfo *last = (ErrorSystemInfo *)storedErrors.QueuePeek(index);
    if (last != NULL){
        if (!last->IsEmpty())  {
            info = *last;
            UnLock();
            return True;
        }
    }
    UnLock();
    return False;

}

//
void LoggerService::ThreadFinished()
{
    logProcessingThread = 0;
    hasToContinue       = False;
}



void  LoggerServiceThreadFN(void *arg){
    Threads::SetPriorityLevel(0);

//    LoggerServiceInstance.EnableRemoteLogger();

    FString errorMsg;
    while(LoggerServiceInstance.ThreadHasToContinue()){
        LoggerPage *pages = LoggerServiceInstance.GetLogEntry();

        if (pages == NULL) {
            SleepMsec(1);
        }
        else {
            errorMsg = "";
            while(pages != NULL) {
                LoggerPage *p = pages;
                LoggerPage *q = pages;
                if (q->Next() != NULL){
                    while (q->Next()->Next() != NULL) q = (LoggerPage *)q->Next();
                    p = (LoggerPage *)q->Next();
                }
                uint32 size = p->usedSize;
                errorMsg.Write(p->page,size);
                LoggerServiceInstance.ReturnPage(p);
                if (q->Next() == p) q->SetNext(NULL);
                else pages = NULL;
            }
            LoggerServiceInstance.ProcessError(errorMsg);
        }
    }
    LoggerServiceInstance.ThreadFinished();
}


void LoggerServiceBufferFN(CStream *cs){
    if (cs==NULL) return;

    LoggerPage *last = (LoggerPage *)cs->context;
    if (last!=NULL) last->usedSize = LSPageSize;

    LoggerPage *p =LoggerServiceInstance.GetPage();
    if (p==NULL) return;

    p->SetNext(last);
    cs->context = p;
    cs->bufferPtr = p->page;
    cs->sizeLeft = LSPageSize;
}


bool GetLogStream(CStream *cs){
    if (cs == NULL) return False;
    cs->sizeLeft = 0;
    cs->bufferPtr = NULL;
    cs->context = NULL;
    cs->NewBuffer = LoggerServiceBufferFN;
    LoggerServiceBufferFN(cs);
    if (cs->bufferPtr==NULL){
        cs->sizeLeft  = 0;
        cs->NewBuffer = NULL;
        cs->context   = NULL;
        return False;
    }
    return True;
}

bool EndLogStream(CStream *cs){
    if (cs==NULL) return False;

    LoggerPage *p = (LoggerPage *)cs->context;
    if (p==NULL){
    return False;
    }
    p->usedSize = LSPageSize-cs->sizeLeft;

    LoggerServiceInstance.AddLogEntry(p);

    cs->bufferPtr = NULL;
    cs->sizeLeft  = 0;
    cs->context   = NULL;
    cs->NewBuffer = NULL;
    return True;
}



// Cannot simply write to a stream because of the need of allocating memory
// which is incompatible with being called by interrupts
void LSAssembleErrorMessage(const char *errorDescription,va_list argList,const char *errorHeader,...){

    CStream cs;

    bool ret = GetLogStream(&cs);
    if (!ret) return;

    if (errorHeader!=NULL){
        va_list argList2;
        va_start(argList2,errorHeader);
        VCPrintf(&cs,errorHeader,argList2);
        va_end(argList2);
    }

    VCPrintf(&cs,errorDescription,argList);
    EndLogStream(&cs);

}


void LSProcessPendingErrors(){
    int maxWait = 100;
    SleepMsec(10);
    while (LoggerServiceInstance.HasPendingErrorEntries() && (maxWait>0)){
        SleepMsec(10);
        maxWait--;
    }
}


void LSStartService(){
     LoggerServiceInstance.StartThread();
}

void LSStopService(){
    LoggerServiceInstance.StopThread();
}

bool LSLastError(ErrorSystemInfo &info,int32 index){
    return LoggerServiceInstance.LastError(info,index);
}

