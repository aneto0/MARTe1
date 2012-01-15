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

#include "System.h"
#include "Threads.h"
#include "ErrorManagement.h"
#include "Processes.h"
#include "BString.h"
#include "MutexSem.h"
#include "ProcessorType.h"

#ifdef _RTAI
// Code for the RTAI RT_TASK struct array
// It is used to keep track of allocated RT_TASKs and avoid memory leaks.
// Please note that maximum MAX_SIM_TASKS can be active at a time
#define MAX_SIM_TASKS 200
// Safety margin to keep some free rt_task_pool_element and avoid racing conditions
#define MARGIN 3

// Struct which describes a single element of the task pool
// first element is a pointer to the allocated RT_TASK, second one is TRUE
//
typedef struct _rt_task_pool_element{
    RT_TASK* task;
    volatile bool free;
} rt_task_pool_element;


class TaskPoolManager{
private:
    MutexSem sem;
    rt_task_pool_element TaskPool[MAX_SIM_TASKS];
    int current;
    int freeCounter;

public:
    TaskPoolManager(){
        current = 0;
        freeCounter = MAX_SIM_TASKS - MARGIN;

        sem.Create();
        for (int i=0; i<MAX_SIM_TASKS; i++){
            TaskPool[i].task = (RT_TASK*)malloc(RT_TASK_STRUCT_SIZE);
            TaskPool[i].free = True;
        }
    }
    ~TaskPoolManager(){
        sem.Close();
        for (int i=0; i<MAX_SIM_TASKS; i++){
            free((void*&)TaskPool[i].task);
        }
    }
    bool DeleteTask(RT_TASK* task){
        int j;

        sem.Lock();
        for (j=0; j<MAX_SIM_TASKS; j++){
            if (TaskPool[j].task == task) {
                TaskPool[j].free = True;
                freeCounter++;
                sem.UnLock();
                return True;
            }
        }
        sem.UnLock();
        return False;
    }
    RT_TASK* CreateTask(){
        int j;

        sem.Lock();
        if (freeCounter <= 0) {
            sem.UnLock();
            CStaticAssertErrorCondition(FatalError,"TaskPoolManager::CreateTask no available RT_TASK in pool");
            return NULL;
        }
        int index;
        for (j=0; j<MAX_SIM_TASKS; j++){
            index = j+current;
            if (index>=MAX_SIM_TASKS) index -= MAX_SIM_TASKS;
            if (TaskPool[index].free == True) {
                TaskPool[index].free = False;
                current = index + 1;
                freeCounter--;
                sem.UnLock();
                return TaskPool[index].task;
            }
        }
        sem.UnLock();
        CStaticAssertErrorCondition(FatalError,"TaskPoolManager::CreateTask error on counter!");
        return NULL;
    }
};

TaskPoolManager RTAITaskPoolManager;

#endif


#if defined(_OS2)

// used only in os2 to remember the priority class used
uint32 priorityClass = NORMAL_PRIORITY_CLASS;

#endif

//################################################################################################
//
//      Pluggable ThreadInitialisation
//
//################################################################################################

//void GlobalThreadInitialisation(){
//    ThreadIPTLS = TlsAlloc();
//}

void __thread_decl SystemThreadFunction(void *threadData){
    ThreadInitialisationInterface *tii = (ThreadInitialisationInterface *)threadData;
    if (tii ==NULL){
        return;
    }

    TDB_Lock();
    bool tdbOk = TDB_NewEntry(tii);
    TDB_UnLock();

    //Guarantee that the OS finishes the housekeeping before releasing the thread to the user
    tii->ThreadWait();
    //Start the user thread
    tii->UserThreadFunction();

    TDB_Lock();
    ThreadInitialisationInterface *tii2 = TDB_RemoveEntry();
    TDB_UnLock();

    if (tii != tii2){
        CStaticAssertErrorCondition(FatalError,"SystemThreadFunction TDB_RemoveEntry returns wrong tii \n");
    }

#if defined(_RTAI)
	if(strcmp(tii->GetThreadName(),"RTAIMain")==0){
	}else{

	    RT_TASK* tmp = rt_whoami();

	    bool found = RTAITaskPoolManager.DeleteTask(tmp);

	    if (found == False) {
            CStaticAssertErrorCondition(FatalError, "Threads::SystemThreadFunction (%p) Thread dying not found in thread pool, possible memory leak! ", tmp);
            return;
	    }
	} 
#endif
    delete tii;
}

ThreadInitialisationInterface * DefaultThreadInitialisationInterfaceConstructor(
    ThreadFunctionType          userThreadFunction,
    void                        *userData,
    const char                  *threadName,
    ExceptionHandlerBehaviour   exceptionHandlerBehaviour){

    return new ThreadInitialisationInterface(userThreadFunction,userData,threadName);
}


/// Allows to set the thread initialisation method
void ThreadsSetInitialisationInterfaceConstructor(ThreadInitialisationInterfaceConstructorType tiic){
    if (tiic != NULL){
        ::threadInitialisationInterfaceConstructor = tiic;
    } else {
        ::threadInitialisationInterfaceConstructor = DefaultThreadInitialisationInterfaceConstructor;
    }
}

/** Assign default initialisation. */
ThreadInitialisationInterfaceConstructorType threadInitialisationInterfaceConstructor = DefaultThreadInitialisationInterfaceConstructor;

/** This function allows to call a subroutine within an exception handler protection */
bool ThreadProtectedExecute(ThreadFunctionType userFunction,void *userData, ExceptionHandlerInterface *ehi){
    ThreadInitialisationInterface *tii = ThreadsGetInitialisationInterface();
    if (tii == NULL){
        CStaticAssertErrorCondition(FatalError,"ExceptionProtectedExecute:no ThreadInitialisationInterface in TLS!");
        return False;
    }

    return tii->ExceptionProtectedExecute(userFunction,userData, ehi);

}

//####################################################
//
// main wrapper
//
//####################################################

//
struct PrivateMainHandlerData{
    //
    int  argc;
    //
    char **argv;
    //
    int  returnValue;
    //
    int (*userMainFunction)(int argc,char **argv);
    //
    PrivateMainHandlerData(int (*userMainFunction)(int argc,char **argv),int argc,char **argv){
        this->argc              = argc;
        this->argv              = argv;
        this->userMainFunction  = userMainFunction;
    }
};


//
static void MainHandlerFunction(void *p){
    PrivateMainHandlerData *pmhd = (PrivateMainHandlerData *)p;
    pmhd->returnValue = pmhd->userMainFunction(pmhd->argc,pmhd->argv);
}


/* obtained from argv[0] */
static BString processName("Unknown");
/* parse -loggerServer XXXXX */
static BString loggerServer("http://localhost:32767");
/* parse -httpRelay XXXXX */
static BString httpRelay("http://localhost:8080");

const char *GetProcessName(){
    return processName.Buffer();
}
const char *GetLoggerServerURL(){
    return loggerServer.Buffer();
}
const char *GetHttpRelayURL(){
    return httpRelay.Buffer();
}
void SetLoggerServerURL(const char *url){
    loggerServer = url;
}



// this is the real main disable define
int MainHandler(int (*userMainFunction)(int argc,char **argv),int argc,char **argv){
    PrivateMainHandlerData pmhd(userMainFunction,argc,argv);

    if ((argv != NULL) && (argv[0] != NULL)){
        // strip path from application name
        char *name = argv[0];
        char *nameE= name+strlen(name);
        while ((nameE>name) && (strchr("\\/:",nameE[-1])==NULL))nameE--;
        processName = nameE;
    }

    int argN=1;
    while(argN < (argc-1)){
        if (strncasecmp(argv[argN],"-loggerServer",20)==0){
            loggerServer = argv[++argN];
        }
        if (strncasecmp(argv[argN],"-httpRelay",20)==0){
            httpRelay = argv[++argN];
        }
        argN++;
    }

    if (threadInitialisationInterfaceConstructor == NULL){
        CStaticAssertErrorCondition(ParametersError,"MainHandler (%s) threadInitFunctionConstructor is NULL",GetProcessName());
        return -1;
    }
    ThreadInitialisationInterface *tii = threadInitialisationInterfaceConstructor(MainHandlerFunction,&pmhd,GetProcessName(),XH_KillTask);
    if (tii == NULL){
        CStaticAssertErrorCondition(InitialisationError,"MainHandler (%s) threadInitFunctionConstructor returns NULL",GetProcessName());
        return -1;
    }

    tii->ThreadPost();
    SystemThreadFunction(tii);

    return pmhd.returnValue;
}

//####################################################
//
// Exported Functions
//
//####################################################


void ThreadsSetPriorityLevel(uint32 level){
#if defined(_OS2)
    DosSetPriority(PRTYS_THREAD,priorityClass,level,0);
#elif defined(_VXWORKS)
    if (level > 31) level = 31;
    int prio;
    taskPriorityGet(taskIdSelf(),&prio);
    prio = 255 - prio;
    int priorityClass = prio /64;
    level = level * 2;
    taskPrioritySet(taskIdSelf(),255 - level - 64 * priorityClass);
#elif (defined(_LINUX) || (defined _SOLARIS) || defined(_MACOSX)) && (defined USE_PTHREAD)
    level = level * 99 / 31;
    int policy = 0;
    sched_param param;
    pthread_getschedparam(ThreadsThreadId(), &policy, &param);
    policy = SCHED_RR;
    param.sched_priority = level;
    pthread_setschedparam(ThreadsThreadId(), policy, &param);
#elif (defined(_WIN32) || defined(_RSXNT))
    level /= 5;
    switch (GetPriorityClass(GetCurrentProcess())){
    case IDLE_PRIORITY_CLASS     : {
        if (level > 5) level = 5;
    }break;
    case REALTIME_PRIORITY_CLASS : {
        if (level > 6) level = 6;
    }break;
    case HIGH_PRIORITY_CLASS     :
    case NORMAL_PRIORITY_CLASS   :
    default:{
        if (level > 5) level = 5;
        if (level == 0) level = 1;
    }
    }
    switch (level){
    case 0: SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_IDLE);         break;
    case 1: SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_LOWEST);       break;
    case 2: SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_BELOW_NORMAL); break;
    case 3: SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_NORMAL);       break;
    case 4: SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_ABOVE_NORMAL); break;
    case 5: SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_HIGHEST);      break;
    case 6: SetThreadPriority(GetCurrentThread(),THREAD_PRIORITY_TIME_CRITICAL);break;
    }

#elif defined(_RTAI)
    int currentPriority = rt_get_prio(rt_whoami());
    if (level>31) level=31;
    int newPriority = (currentPriority & 0xFFFF) + (31 - level) << 16;
    rt_change_prio((RT_TASK *)rt_whoami(), newPriority);
#endif
}

#if (defined(_SOLARIS) || defined (_LINUX) || defined(_MACOSX)) && (defined USE_PTHREAD)
typedef void *(__thread_decl *StandardThreadFunction)(void *args);
#elif defined(_RTAI)
typedef void (__thread_decl *StandardThreadFunction)(long int args);
#endif

TID  ThreadsBeginThread(ThreadFunctionType function,
                        void *parameters,
                        uint32 stacksize,
                        const char *name,
                        ExceptionHandlerBehaviour exceptionHandlerBehaviour,
                        ProcessorType runOnCPUs){

    if(runOnCPUs == PTUndefinedCPUs){
        if(ProcessorType::GetDefaultCPUs() != 0){
            runOnCPUs = ProcessorType::GetDefaultCPUs();
        }
        else{
            runOnCPUs = 0xff;
        }
    }

    if (threadInitialisationInterfaceConstructor == NULL){
        CStaticAssertErrorCondition(ParametersError,"Threads::ThreadsBeginThread (%s) threadInitialisationInterfaceConstructor is NULL",name);
        return (TID)-1;
    }
    ThreadInitialisationInterface *tii = threadInitialisationInterfaceConstructor(function,parameters,name,exceptionHandlerBehaviour);
    if (tii == NULL){
        CStaticAssertErrorCondition(InitialisationError,"Threads::ThreadsBeginThread (%s) threadInitialisationInterfaceConstructor returns NULL",name);
        return (TID)-1;
    }

    TID ID = 0;
#if (defined(_OS2) || defined(_RSXNT))
    ID = (TID)_beginthread(SystemThreadFunction,NULL,stacksize,tii);
#elif defined(_WIN32)
    ID = (HANDLE)_beginthread(SystemThreadFunction,stacksize,tii);
#elif defined(_VXWORKS)
    ID = taskSpawn((char *)name,100,VX_FP_TASK,stacksize,(FUNCPTR)SystemThreadFunction,(int)tii,0,0,0,0,0,0,0,0,0);
#elif ((defined(_LINUX)) && (defined (USE_PTHREAD)))
    pthread_attr_t   stackSizeAttribute;
    pthread_attr_init(&stackSizeAttribute);
    pthread_attr_setstacksize(&stackSizeAttribute, stacksize);
    StandardThreadFunction Function = (StandardThreadFunction)SystemThreadFunction;
    pthread_create(&ID, &stackSizeAttribute, Function, tii);
    pthread_detach(ID);
    pthread_setaffinity_np(ID, sizeof(runOnCPUs.processorMask), (cpu_set_t *)&runOnCPUs.processorMask);
#elif (defined _SOLARIS || defined(_MACOSX)) && (defined USE_PTHREAD)
    StandardThreadFunction Function = (StandardThreadFunction)SystemThreadFunction;
    pthread_create(&ID, NULL, Function, tii);
#elif defined(_RTAI)
    int ret = 0;
    TaskID task = (TaskID)(RTAITaskPoolManager.CreateTask());
    if (task == NULL) {
        CStaticAssertErrorCondition(FatalError,"Threads::BeginThread (%s) Too many threads running, no space available in the thread pool! ",name);
        return 0;
    }
    StandardThreadFunction  Function = (StandardThreadFunction)SystemThreadFunction;
    ret = rt_task_init(task,Function, (long int)tii, stacksize, RTAI_PRIORITY_NORMAL, 1, NULL);
    rt_set_runnable_on_cpus(task, runOnCPUs.processorMask);
    ret = rt_task_resume(task);
    ID  = (TID)task; 
#endif
    //Enable the user thread to run...
    tii->ThreadPost();
    return ID;
}

TID  ThreadsThreadId(){

#if (defined(_WIN32)||defined(_RSXNT))
    return (TID)GetCurrentThreadId();
#elif defined(_OS2)
    PTIB ptib;
    PPIB ppib;
    DosGetInfoBlocks(&ptib,&ppib);
    return (TID)ptib->tib_ordinal;
#elif defined(_VXWORKS)
    return (TID)taskIdSelf();
#elif defined(_RTAI)
    return (TID)rt_whoami();
#elif (defined(_LINUX) || (defined _SOLARIS) || defined(_MACOSX)) && (defined USE_PTHREAD)
    return pthread_self();
#endif
    return 0;
}

TID  ThreadsThreadOsId() {
#if defined(_LINUX)
    return (syscall(SYS_gettid));
#else
    return (TID)-1;
#endif
}

bool ThreadsKill(TID tid){
#if (defined(_RTAI))
    int ret;
        ret = rt_task_delete((TaskID)tid);
    if (ret == 0) {
        bool found = RTAITaskPoolManager.DeleteTask((RT_TASK*)tid);
            if (found == False) {
            CStaticAssertErrorCondition(FatalError,"Threads::KillThread (%p) Thread to be killed not found in thread pool, possible memory leak! ",tid);
            return -1;
        }
    }
#elif (defined(_OS2))
        APIRET rc;
        if((rc = DosKillThread(tid))!=0)return False;
        return True;
#elif (defined(_WIN32)||defined(_RSXNT))
        if(TerminateThread((HANDLE)tid,0)==FALSE)return False;
        return True;
#elif (defined(_VXWORKS))
        if (taskDelete(tid) != OK)return False;
        return True;
#elif (defined(_LINUX) || (defined _SOLARIS) || defined(_MACOSX)) && (defined USE_PTHREAD)
        if(tid == 0) return True;
        int ret = pthread_cancel(tid);
        if (ret == 0) {
            pthread_join(tid, NULL);
            return True;
        }
        return False;
#endif
    return True;
}


bool ThreadsIsAlive(TID tid){
    bool alive = True;
#if defined(_OS2)
#error IsAlive not implemented
#elif defined(_RTAI)
//!CHECK!
    return (((TaskID)tid) != NULL);
#elif (defined(_WIN32)||defined(_RSXNT))
    alive = (GetThreadPriority((HANDLE)tid) != THREAD_PRIORITY_ERROR_RETURN);
#elif (defined(_VXWORKS))
    alive = (taskIdVerify(tid)==OK);
#elif (defined(_LINUX) || (defined _SOLARIS) || defined(_MACOSX)) && (defined USE_PTHREAD)
    if(tid == 0) return False;
    alive = (pthread_kill(tid, 0) == 0);
#endif
    return alive;
}


void ThreadsSetNormalClass(){
#if (defined(_OS2))

    priorityClass = NORMAL_PRIORITY_CLASS;

#elif (defined(_WIN32)||defined(_RSXNT))
    SetPriorityClass(GetCurrentProcess(),NORMAL_PRIORITY_CLASS);
#elif (defined(_LINUX)) && (defined USE_PTHREAD)

#elif defined(_VXWORKS)
    int prio;
    taskPriorityGet(taskIdSelf(),&prio);
    prio = 255 - prio;
    int priorityLevel = prio &63;
    taskPrioritySet(taskIdSelf(),255 - priorityLevel - 64);
#elif (defined(_SOLARIS) || defined(_MACOSX)) && (defined USE_PTHREAD)
    setpriority(PRIO_PROCESS,0,0);
#elif defined(_RTAI)
    ThreadsSetPriorityLevel(RTAI_PRIORITY_NORMAL);
#endif
}

void ThreadsSetRealTimeClass(){
#if (defined(_OS2))
    priorityClass = REALTIME_PRIORITY_CLASS;
#elif (defined(_WIN32)||defined(_RSXNT))
    SetPriorityClass(GetCurrentProcess(),REALTIME_PRIORITY_CLASS);
#elif( defined(_LINUX)) && (defined USE_PTHREAD)
    ThreadsSetPriorityLevel(31);
#elif (defined _SOLARIS || defined(_MACOSX)) && (defined USE_PTHREAD)
    setpriority(PRIO_PROCESS,0,-20);
#elif defined(_VXWORKS)
    int prio;
    taskPriorityGet(taskIdSelf(),&prio);
    prio = 255 - prio;
    int priorityLevel = prio &63;
    taskPrioritySet(taskIdSelf(),255 - priorityLevel - 64*3);
#elif defined(_RTAI)
    rt_change_prio((RT_TASK *)rt_whoami(), RTAI_PRIORITY_MAX);
#endif
}

void ThreadsSetIdleClass(){
#if (defined(_OS2))
    priorityClass = IDLE_PRIORITY_CLASS;
#elif (defined(_WIN32)||defined(_RSXNT))
    SetPriorityClass(GetCurrentProcess(),IDLE_PRIORITY_CLASS);
#elif (defined(_LINUX)) && (defined USE_PTHREAD)
    ThreadsSetPriorityLevel(0);
#elif (defined _SOLARIS || defined(_MACOSX)) && (defined USE_PTHREAD)
    setpriority(PRIO_PROCESS,0,20);
#elif defined(_VXWORKS)
    int prio;
    taskPriorityGet(taskIdSelf(),&prio);
    prio = 255 - prio;
    int priorityLevel = prio &63;
    taskPrioritySet(taskIdSelf(),255 - priorityLevel );
#elif defined(_RTAI)
    rt_change_prio((RT_TASK *)rt_whoami(), RTAI_PRIORITY_IDLE);
#endif
}

void ThreadsSetHighClass(){
#if (defined(_OS2))
    priorityClass = HIGH_PRIORITY_CLASS;
#elif (defined(_WIN32)||defined(_RSXNT))
    SetPriorityClass(GetCurrentProcess(),HIGH_PRIORITY_CLASS);
#elif (defined(_LINUX)) && (defined USE_PTHREAD)
    ThreadsSetPriorityLevel(49);
#elif (defined (_SOLARIS) || defined(_MACOSX)) && (defined USE_PTHREAD)
    setpriority(PRIO_PROCESS,0,10);
#elif defined(_VXWORKS)
    int prio;
    taskPriorityGet(taskIdSelf(),&prio);
    prio = 255 - prio;
    int priorityLevel = prio &63;
    taskPrioritySet(taskIdSelf(),255 - priorityLevel - 64*2);
#elif defined(_RTAI)
    rt_change_prio((RT_TASK *)rt_whoami(), RTAI_PRIORITY_HIGH);
#endif
}

int32 ThreadsGetCPUs(TID tid){
    int32 cpus = -1;
#if defined(_RTAI)
    cpus = fcomm_get_thread_cpu((RT_TASK *)tid);
#elif defined(_LINUX)
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    if(pthread_getaffinity_np(tid, sizeof(cpuset), &cpuset) != 0){
        return cpus;
    }
    cpus    = 0;
    int32 j = 0;
    for(j = 0; j < CPU_SETSIZE; j++){
        if (CPU_ISSET(j, &cpuset)){
            cpus |= (1 << j);
        }
    }
#endif

    return cpus;
}

int ThreadsGetPriority(TID tid){
    int priority = -1;
#if defined(_RTAI)
    priority = fcomm_get_prio((RT_TASK*)tid);
#elif defined(_LINUX)
    sched_param param;
    int policy;
    pthread_getschedparam (tid, &policy, &param);
    priority = param.sched_priority;
#elif defined(_VXWORKS)
    taskPriorityGet(tid, &priority);
#endif
    return priority;
}

uint32 ThreadsGetState(TID tid){
    uint32 threadState = THREAD_STATE_UNKNOWN;
#if defined(_RTAI)
    threadState = 0;
    uint32 osState = (uint32)rt_get_task_state((RT_TASK*)tid);
    if((osState & RT_SCHED_READY) == RT_SCHED_READY)
        threadState |= THREAD_STATE_READY;
    if((osState & RT_SCHED_SEMAPHORE) == RT_SCHED_SEMAPHORE){
        threadState |= THREAD_STATE_SEM;
        threadState |= THREAD_STATE_PEND;
    }
    if((osState & RT_SCHED_SUSPENDED) == RT_SCHED_SUSPENDED){
        threadState |= THREAD_STATE_SUSP;
    }
    if((osState & RT_SCHED_DELAYED)== RT_SCHED_DELAYED){
        threadState |= THREAD_STATE_DELAY;
        threadState |= THREAD_STATE_PEND;
    }
#elif defined(_VXWORKS)    
    threadState = 0;
    TASK_DESC pTaskDesc;
    taskInfoGet(tid, &pTaskDesc);
    uint32 osState = pTaskDesc.td_status;

    if(osState == WIND_READY){
        threadState = THREAD_STATE_READY;
    }
    else{
        if((osState & WIND_PEND) == WIND_PEND)
            threadState |= THREAD_STATE_PEND;
        if((osState & WIND_SUSPEND) == WIND_SUSPEND)
            threadState |= THREAD_STATE_SUSP;
        if((osState & WIND_DELAY) == WIND_DELAY){
            if((osState & WIND_PEND) == WIND_PEND){
                threadState |= THREAD_STATE_TOUT;
            }
            else{
                threadState |= THREAD_STATE_DELAY;
                threadState |= THREAD_STATE_PEND;
            }
        }
        if((osState & WIND_DEAD) == WIND_DEAD){
            threadState |= THREAD_STATE_DEAD;
            threadState |= THREAD_STATE_SUSP;
        }
    }
#endif
    return threadState;
}

ThreadInitialisationInterface *ThreadsGetInitialisationInterface()
{
    return TDB_GetTII();
}

