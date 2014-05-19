/* $Revision: 1.1.6.2 $
 *
 * Copyright 2010 The MathWorks, Inc.
 *
 */
#if defined(_MSC_VER)
# pragma once
#endif
#if defined(__GNUC__) && (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ > 3))
# pragma once
#endif

#ifndef rt_parfor_h
#define rt_parfor_h

#include "tmwtypes.h"
#include "simstruc.h"

typedef void  (*RTParallelForBodyFcn)(int32_T itr);

#ifdef __cplusplus
extern "C" {
#endif

extern void rt_ParallelForEachExecute(int32_T numIters, RTParallelForBodyFcn fcn, int32_T parforSysIdx);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern void *rt_ParallelForEachInitScheduler(SimStruct *S, int32_T numThreads, int32_T numParSSs);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern void rt_ParallelForEachClearScheduler(void *scheduler);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern void rt_ParallelForEachSetEnableFlag(boolean_T s);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C" {
#endif

void rt_ParallelForEachSetProfilingFlag(boolean_T s);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C" {
#endif

void rt_ParallelForEachSetProfResultFileName(const char *fname);

#ifdef __cplusplus
}
#endif

#endif /* rt_parfor_h */
