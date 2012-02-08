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

#include "Threads.h"
#include "MutexSem.h"
#include "Memory.h"
#include "LinkedListHolder.h"
#include "StreamInterface.h"
#include "ErrorManagement.h"


#undef malloc
#undef free
#undef realloc
#undef strdup


// if defined each alloc and malloc will be monitored .
//#define MEMORYStatistics
// if defined each alloc and malloc will be producing an output on stderr

//#define MEMORYMonitoring
//
//#define MEMORYDirtying

// number of words
//#define MEMORYWatchdogArea 32

#if defined (MEMORYStatistics)

struct MEMORYInfo{

    //
    TID tid;

    //
    int size;

    //
    void Init(int size){
        this->tid   = Threads::ThreadId();
        this->size  = size;
    }

    //
    void Init(MEMORYInfo &info,int size){
        this->size  = size;
        this->tid   = info.tid;

    }
};

struct ThreadAllocationStatistics{

    // the id of the thread
    TID threadId;

    //
    int totalMemorySize;

    //
    int nOfMemoryChunks;

};

#define MAX_NO_OF_MEMORY_MONITORS 32

//
MutexSem MEMORYmux;

// this is init before any static object initialisation
bool AllocStatisticsActive = False;

// where the information is stored
ThreadAllocationStatistics threadAllocationStatistics[MAX_NO_OF_MEMORY_MONITORS]={
    {0,0,0}
};

static inline ThreadAllocationStatistics *FindAndAddTas(TID tid){
    ThreadAllocationStatistics *tas = NULL;
    int i = 0;
    for (i = 0; (i < MAX_NO_OF_MEMORY_MONITORS) && (threadAllocationStatistics[i].threadId != 0);i++){
        if (threadAllocationStatistics[i].threadId == tid)
            tas = &threadAllocationStatistics[i];
    }
    if (tas != NULL) return tas;
    if ((i < MAX_NO_OF_MEMORY_MONITORS) && (threadAllocationStatistics[i].threadId == 0)){
        threadAllocationStatistics[i].threadId = tid;
        threadAllocationStatistics[i].totalMemorySize = 0;
        threadAllocationStatistics[i].nOfMemoryChunks = 0;
        if ((i+1) < MAX_NO_OF_MEMORY_MONITORS)
            threadAllocationStatistics[i+1].threadId = 0;
        return &threadAllocationStatistics[i];
    }
    return NULL;
}

static inline ThreadAllocationStatistics *FindTas(TID tid){
    ThreadAllocationStatistics *tas = NULL;
    int i = 0;
    for (i = 0; (i < MAX_NO_OF_MEMORY_MONITORS) && (threadAllocationStatistics[i].threadId != 0);i++){
        if (threadAllocationStatistics[i].threadId == tid)
            tas = &threadAllocationStatistics[i];
    }
    if (tas != NULL) return tas;
    return NULL;
}

static inline void AddMemoryChunk(MEMORYInfo *info){
    if (info->size <= 0) return;

    ThreadAllocationStatistics *tas = FindAndAddTas(info->tid);
    if (tas == NULL) return;

    tas->totalMemorySize += info->size;
    tas->nOfMemoryChunks ++;

#if defined MEMORYMonitoring
    fprintf(stderr,"[%i] %i  -> %i @%p\n",info->tid,info->size,tas->totalMemorySize,info);
#endif
}

//
static inline void FreeMemoryChunk(MEMORYInfo *info){
    if (info->size <= 0) return;

    ThreadAllocationStatistics *tas = FindAndAddTas(info->tid);
    if (tas == NULL) return;

    tas->totalMemorySize -= info->size;
    tas->nOfMemoryChunks --;

#if defined MEMORYMonitoring
    fprintf(stderr,"[%i] %i  -> %i @%p\n",info->tid,info->size,tas->totalMemorySize,info);
#endif
}

//
static inline void ReallocMemoryChunk(MEMORYInfo *info,MEMORYInfo *saveInfo){
    ThreadAllocationStatistics *tas = FindAndAddTas(info->tid);
    if (tas == NULL) return;

    tas->totalMemorySize -= saveInfo->size;
    tas->totalMemorySize += info->size;

#if defined MEMORYMonitoring
    fprintf(stderr,"[%i] %i %i -> %i @%p\n",info->tid,-saveInfo->size,info->size,tas->totalMemorySize,info);
#endif
}


#endif

// top of normal memory
#ifdef __LP64__
char *NormalMemTop = (char *)0xFFFFFFFFFFFFFFFF;
#else
char *NormalMemTop = (char *)0xFFFFFFFF;
#endif


#if defined(_V6X5100)|| defined(_V6X5500)
/**
 * CODAS Board Support for VxWorks 6.8 provides routines 
 * 
 * sysUserMemoryStart() : base address of user memory, typically 40M
 * sysUserMemoryEnd()   : top address of user memory, typically end of memory
 * 
 * CAUTION : rtsLib provides RtMalloc which uses this area for a custom memory partition
 *           MARTe must be made compatible with CODAS tasks if they are using this.
 * 
 */
extern "C"{
  extern void *sysUserMemoryStart(void);
  extern void *sysUserMemoryEnd(void);
}
#endif


#if defined(_VX5100) || defined(_VX5500)|| defined(_V6X5100)|| defined(_V6X5500)

// Data Partition Identifier
PART_ID dataPartId = NULL;

static class InitExtraMem{
public:
    InitExtraMem(){


#if defined(_V6X5100)|| defined(_V6X5500)

#define ASTEPHEN_VXWORKS6_PATCH

#ifdef ASTEPHEN_VXWORKS6_PATCH
      // Create the new partition of maximal size in the user memory area.

      // To use all of upper memory (40M to 512M)

      /*
      NormalMemTop = (char *) sysUserMemoryStart();
      char * UserMemStart = NormalMemTop;
      char * UserMemTop = (char * ) sysUserMemoryEnd();
      
      size_t NormalMemSize = (size_t) (UserMemTop - UserMemStart);

      dataPartId = memPartCreate(NormalMemTop, NormalMemSize);
      */
      
      NormalMemTop = (char *) megs_256;
      char * UserMemStart = NormalMemTop;
      char * UserMemTop = (char * )  megs_512;
      
      size_t NormalMemSize = (size_t) (UserMemTop - UserMemStart);

      dataPartId = memPartCreate(NormalMemTop, NormalMemSize);

#else
        // Gets the address of the top of the logical memory
        NormalMemTop = sysMemTop();
        // Create the new partition of 32 mega
        dataPartId = memPartCreate(NormalMemTop,megs_32);
#endif

#else

        // Gets the address of the top of the logical memory
        NormalMemTop = sysMemTop();
        // Create the new partition of 32 mega
        dataPartId = memPartCreate(NormalMemTop,megs_32);


#endif

    }

}_InitExtraMem;

static void *ExtraMalloc(unsigned nByte){
    if (dataPartId == NULL) return NULL;
    // Try to allocate memory in the Data Partition (the bigger one)
    return memPartAlloc(dataPartId,nByte);
}

static bool ExtraFree(void *p){
    if (dataPartId==NULL){
        free(p);
        return True;
    }
    return (memPartFree(dataPartId,(char *)p) == OK);
}

#else

static void *ExtraMalloc(unsigned nByte){
    return malloc(nByte);
}

static bool ExtraFree(void *p){
    free(p);
    return True;
}

#endif

//void *MEMORYMalloc(int size,int allocFlags, int line, const char *fileName){
void *MEMORYMalloc(int size,MemoryAllocationFlags allocFlags){
    if (size <= 0) return NULL;
    
    void *data;

#if defined(MEMORYStatistics)

    int actualSize = size + (sizeof(MEMORYInfo)) + MEMORYWatchdogArea * sizeof(intptr); // threadID + memory Pool

    MEMORYInfo *mem;
    if (allocFlags & MEMORYExtraMemory) mem = (MEMORYInfo *) ExtraMalloc(actualSize);
    else                                mem = (MEMORYInfo *) malloc(actualSize);

    if (mem == NULL) return NULL;

    mem->Init(size);

    if (!AllocStatisticsActive){
        AllocStatisticsActive = True;
        MEMORYmux.Create();
    }

    MEMORYmux.Lock();
    AddMemoryChunk(mem);
    MEMORYmux.UnLock();


#if (MEMORYWatchdogArea >0)
    {
        char *p = (char *)mem;
        p += size;
        p += sizeof(MEMORYInfo);
        int *pw = (int *)p;
        for (int i = 0;i < MEMORYWatchdogArea;i++){
            *pw = (int)pw;
            pw++;
        }
    }
#endif

    data = (void *) (mem + 1);

#else
    if (allocFlags & MEMORYExtraMemory) data = ExtraMalloc(size);
    else                                data =  malloc(size);
#endif

#if defined(MEMORYDirtying)
    char *p = (char *)data;
    char *pE = p + size;
    int64 *p32 = (int64 *)p;
    int64 *p32E = (int64 *)pE;
    p32E--;
    while (p32<p32E) *p32++ = 0xDEADBABE;
    p = (char *)p32;
    while (p<pE) *p++ = 0XFF;
#endif
    return data;
}

void *MEMORYRealloc(void *&data,int newSize){

    if (newSize == 0){
    if (data == NULL)return NULL;
        MEMORYFree(data);
        return NULL;
    }
    if (data == NULL){
        data = MEMORYMalloc(newSize);
        return data;
    }

#if defined(MEMORYStatistics)
    MEMORYInfo *mem = (MEMORYInfo *)data;
    mem--;

#if (MEMORYWatchdogArea >0)
    {
        char *p = (char *)mem;
        p += mem->size;
        p += sizeof(MEMORYInfo);
        int *pw = (int *)p;
        for (int i = 0;i < MEMORYWatchdogArea;i++){
            if (*pw != (int)pw){
                CStaticAssertErrorCondition(FatalError,"Realloc:Corruption on WatchDog Area at location %i\n",i);
            }
            pw++;
        }
    }
#endif

    MEMORYInfo saveInfo = *mem;

    int actualSize = newSize + (sizeof(MEMORYInfo)) + MEMORYWatchdogArea * sizeof(intptr); // threadID + memory Pool

    if (data > NormalMemTop) {
        CStaticAssertErrorCondition(FatalError,"cannot reallocate on Extra memory\n");
        MEMORYFree(data);
        return data;
    } else {
        mem = (MEMORYInfo *) realloc(mem,actualSize);
    }

    if (mem == NULL) return mem;

    mem->Init(saveInfo,newSize);

    if (!AllocStatisticsActive){
        AllocStatisticsActive = True;
        MEMORYmux.Create();
    }

    if (AllocStatisticsActive){
        MEMORYmux.Lock();
        ReallocMemoryChunk(mem,&saveInfo);
        MEMORYmux.UnLock();
    }

#if (MEMORYWatchdogArea >0)
    {
        char *p = (char *)mem;
        p += newSize;
        p += sizeof(MEMORYInfo);
        int *pw = (int *)p;
        for (int i = 0;i < MEMORYWatchdogArea;i++){
            *pw = (int)pw;
            pw++;
        }
    }
#endif

    data = (void *) (mem + 1);

#else
    if (data > NormalMemTop) {
        CStaticAssertErrorCondition(FatalError,"cannot reallocate on Extra memory\n");
        MEMORYFree(data);
        return data;
    }

    data = realloc(data,newSize);
#endif

    return data;
}


char *MEMORYStrDup(const char *s){
    if (s== NULL) return NULL;
    int sz = strlen(s);
    char *c= (char *)MEMORYMalloc(sz+1);
    char *cE = c+sz;
    char *cS = c;
    while (cS<=cE) *cS++ = *s++;
    return c;
}


//void MEMORYFree(void *&data,int line, const char *fileName){
void MEMORYFree(void *&data){
    if (data == NULL) return;

#if defined(MEMORYStatistics)

    MEMORYInfo *mem = (MEMORYInfo *)data;
    data = NULL;
    mem--;

#if (MEMORYWatchdogArea >0)
    {
        char *p = (char *)mem;
        p += mem->size;
        p += sizeof(MEMORYInfo);
        int *pw = (int *)p;
        for (int i = 0;i < MEMORYWatchdogArea;i++){
            if (*pw != (int)pw){
                CStaticAssertErrorCondition(FatalError, "Free:Corruption on WatchDog Area at location %i\n",i);
            }
            pw++;
        }
    }
#endif


    if (!AllocStatisticsActive){
        AllocStatisticsActive = True;
        MEMORYmux.Create();
    }

    if (AllocStatisticsActive){
        MEMORYmux.Lock();
        FreeMemoryChunk(mem);
        MEMORYmux.UnLock();
    }

    if ((char *)mem > NormalMemTop) {
        ExtraFree(mem);
    } else {
        free(mem);
    }
#else
    if (data > NormalMemTop) {
        ExtraFree(data);
    } else {
        free(data);
    }
    data= NULL;
#endif
}


void MEMORYDisplayAllocationStatistics(StreamInterface *out){

#if defined(MEMORYStatistics)

    if (out!=NULL)
        out->Printf(" THREADID  TOTALMEMORYSIZE  CHUNKS\n");
    else
        printf(" THREADID  TOTALMEMORYSIZE  CHUNKS\n");

    int i;
    for (i = 0; (i < MAX_NO_OF_MEMORY_MONITORS) && (threadAllocationStatistics[i].threadId != 0);i++){
        if (out != NULL){
            out->Printf(" % 8x       % 10i  % 6i\n",threadAllocationStatistics[i].threadId,threadAllocationStatistics[i].totalMemorySize,threadAllocationStatistics[i].nOfMemoryChunks);
        } else {
            printf(" % 8x       % 10i  % 6i\n",threadAllocationStatistics[i].threadId,threadAllocationStatistics[i].totalMemorySize,threadAllocationStatistics[i].nOfMemoryChunks);
        }
    }

#else
    if (out != NULL)
        out->Printf("Statistics not available. Need to recompile BaseLib2 with -DMEMORYStatistics\n");
    else
        printf("Statistics not available. Need to recompile BaseLib2 with -DMEMORYStatistics\n");
#endif

}

bool MEMORYAllocationStatistics(int &size, int &chunks, TID tid){
#if defined(MEMORYStatistics)
    if (tid == (TID)0xFFFFFFFF) tid = Threads::ThreadId();

    ThreadAllocationStatistics *tas = FindTas(tid);

    if (tas){
        size = tas->totalMemorySize;
        chunks = tas->nOfMemoryChunks;
        return True;
    }
#endif

    return False;
}



bool MEMORYCheck(void *address, MemoryTestAccessMode accessMode,int size){
#if (defined (_WIN32) || defined(_RSXNT))

    if (accessMode & MTAM_Execute){
        if (IsBadCodePtr((FARPROC)address)) return False;
    }
    if (accessMode & MTAM_Read){
        if (IsBadReadPtr(address,size)) return False;
    }
    if (accessMode & MTAM_Write){
        if (IsBadWritePtr(address,size)) return False;
    }

#endif
    return True;
}

void *SharedMemoryAlloc(uint32 key, uint32 size, uint32 permMask){
#if (defined(_RTAI))
    return rt_named_malloc(key, size);
#elif (defined(_LINUX) || defined(_SOLARIS) || defined(_MACOSX))
    key_t keyid = (key_t)key;
    int32 shmid = shmget(key, size, IPC_CREAT | permMask);
    if(shmid == -1){
        int32 err = errno;
        CStaticAssertErrorCondition(FatalError, "SharedMemoryAlloc:smhid: %s\n", strerror(err));
        return NULL;
    }
    void *shm = shmat(shmid, NULL, 0);
    if(shm == (char *) -1){
        int32 err = errno;
        CStaticAssertErrorCondition(FatalError, "SharedMemoryAlloc:shmat: %s\n", strerror(err));
        return NULL;
    }
    return shm;
#else
    return NULL;
#endif
}

void SharedMemoryFree(void *address){
#if (defined(_RTAI))
    rt_named_free(address);
#elif (defined(_LINUX) || defined(_MACOSX))
    int32 ret = shmdt(address);
    if(ret == -1){
        int32 err = errno;
        CStaticAssertErrorCondition(FatalError, "SharedMemoryFree: %s\n", strerror(err));
    }
#elif (defined(_SOLARIS) )
    int32 ret = shmdt((char *)address);
    if(ret == -1){
        int32 err = errno;
        CStaticAssertErrorCondition(FatalError, "SharedMemoryFree: %s\n", strerror(err));
    }

#else

#endif
}


