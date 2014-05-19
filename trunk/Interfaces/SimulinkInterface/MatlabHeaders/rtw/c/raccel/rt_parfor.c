/* $Revision: 1.1.6.4 $
 *
 * Copyright 2010 The MathWorks, Inc.
 *
 */
#include "rt_parfor.h"
#include "mwutil.h"

typedef void  (*RTParallelForExecuterFcn)(int32_T numIters, RTParallelForBodyFcn fcn);
typedef void *(*RTParallelForInitSchedulerFcn)(int32_T numThreads);
typedef void  (*RTParallelForClearSchedulerFcn)(void *shceduler);
typedef double (*RTParallelForTickCountFcn)();

RTParallelForExecuterFcn       rt__parallel__for__execute__fcn = NULL;
RTParallelForInitSchedulerFcn  rt__parallel__for__init__scheduler__fcn = NULL;
RTParallelForClearSchedulerFcn rt__parallel__for__clear__scheduler__fcn = NULL;
RTParallelForTickCountFcn rt__parallel__for__tick__count = NULL;

boolean_T rt__parallel__for__enabled = TRUE;
boolean_T rt__parallel__for__profile = FALSE;
int32_T rt__num__parallel__for__ss = 0;
double    *rt__parallel__for__prof__recs = NULL;
const char *rt__prof__file__name = NULL;

void *rt_ParallelForEachInitScheduler(SimStruct *S, int32_T numThreads, int32_T numParSSs)
{
    int error = 0;
    void *init = NULL;
#ifdef _WIN32
    const char *rtparfor_sharelib = "rtparfor.dll";
#endif
#ifdef __linux__
    const char *rtparfor_sharelib = "libmwrtparfor.so";
#endif
#ifdef __APPLE__
    const char *rtparfor_sharelib = "libmwrtparfor.dylib";
#endif
    
    void *handle = utLoadLibrary(rtparfor_sharelib, &error);
    
    if (handle != NULL && error == 0) {
        *(void **)&rt__parallel__for__execute__fcn = utFindSymbolInLibrary(handle, "rt_ParForExecute");
        if (rt__parallel__for__execute__fcn == NULL) {
            ssSetErrorStatus(S, "Error no rt_ParForExecute symbol found in libmwrtparfor.\n");
            (void) fprintf(stderr, "%s\n", ssGetErrorStatus(S));
            return NULL;
        }
        
        *(void **)&rt__parallel__for__init__scheduler__fcn = utFindSymbolInLibrary(handle, "rt_ParForInitScheduler");
        if (rt__parallel__for__init__scheduler__fcn == NULL) {
            ssSetErrorStatus(S, "Error no rt_ParForInitScheduler symbol found in libmwrtparfor.\n");
            (void) fprintf(stderr, "%s\n", ssGetErrorStatus(S));
            return NULL;
        } 
        
        *(void **)&rt__parallel__for__clear__scheduler__fcn = utFindSymbolInLibrary(handle, "rt_ParForClearScheduler");
        if (rt__parallel__for__clear__scheduler__fcn == NULL) {
            ssSetErrorStatus(S, "Error no rt_ParForcClearScheduler symbol found in libmwrtparfor.\n");
            (void) fprintf(stderr, "%s\n", ssGetErrorStatus(S));
            return NULL;
        } 
        *(void **)&rt__parallel__for__tick__count = utFindSymbolInLibrary(handle, "rt_ParForGetTickCount");
        if (rt__parallel__for__tick__count == NULL) {
            ssSetErrorStatus(S, "Error no rt_ParForcGetTickCount symbol found in libmwrtparfor.\n");
            (void) fprintf(stderr, "%s\n", ssGetErrorStatus(S));
            return NULL;
        } 
        if (rt__parallel__for__profile) {
            rt__num__parallel__for__ss = numParSSs;
            rt__parallel__for__prof__recs = (double *)calloc(rt__num__parallel__for__ss, sizeof(double));
        }
    } else {
        ssSetErrorStatus(S, "Error loading shared object file libmwrtparfor.\n");
        (void) fprintf(stderr, "%s\n", ssGetErrorStatus(S));
        return NULL;
    }
    init = (void *)(rt__parallel__for__init__scheduler__fcn)(numThreads);
    return init;
}

void rt_ParallelForEachClearScheduler(void *scheduler)
{
    if (rt__parallel__for__clear__scheduler__fcn != NULL && scheduler != NULL) { 
        (rt__parallel__for__clear__scheduler__fcn)(scheduler);
    }
    if (rt__parallel__for__profile && rt__parallel__for__prof__recs != NULL) {
        FILE *fptr;
        int32_T i;
        if ((fptr = fopen(rt__prof__file__name, "w+b")) == NULL) {
            (void)fprintf(stderr,"*** Error opening %s",rt__prof__file__name);
            return;
        }
        for (i=0; i<rt__num__parallel__for__ss; i++) {
            (void)fprintf(fptr, "%12.8f\n", rt__parallel__for__prof__recs[i]);
        }
        (void)fclose(fptr);
        free(rt__parallel__for__prof__recs);
    }
}

#define rt_ParallelForEachExecuteHelper(_numIters, _fcn)  \
{                                                         \
    if (rt__parallel__for__enabled) {                     \
        (rt__parallel__for__execute__fcn)(_numIters,_fcn);\
    } else {                                              \
        int iter;                                         \
        for (iter = 0; iter < _numIters; iter++) {        \
            (_fcn)(iter);                                 \
        }                                                 \
    }                                                     \
}

void rt_ParallelForEachExecute(int32_T numIters, RTParallelForBodyFcn fcn, int32_T parforSysIdx)
{
    if (!rt__parallel__for__profile) {
        rt_ParallelForEachExecuteHelper(numIters, fcn);
    } else {
        double t0 = (rt__parallel__for__tick__count)();
        double t1 = 0;

        rt_ParallelForEachExecuteHelper(numIters, fcn);
        
        t1 = (rt__parallel__for__tick__count)();
        rt__parallel__for__prof__recs[parforSysIdx] += t1-t0;
    }
}

void rt_ParallelForEachSetEnableFlag(boolean_T s)
{
    rt__parallel__for__enabled = s;
}

void rt_ParallelForEachSetProfilingFlag(boolean_T s)
{
     rt__parallel__for__profile = s;
}

void rt_ParallelForEachSetProfResultFileName(const char *fname)
{
    rt__prof__file__name = fname;  
}
