#ifndef SIMULINK_EXECUTION_SSBRIDGE_API_H
#define SIMULINK_EXECUTION_SSBRIDGE_API_H

/**
 * @file slexec_ssbridge.h
 *
 * This header is the exported interface to the simulink execution engine. It is
 * used by the RTW-RSim and Rapid Accelerator targets.
 *
 * 
 * Copyright 2011-2012 The MathWorks, Inc.
 */


/********************************************************************************
 * Section 1. Determine Linkage based on where this file is being included
 *******************************************************************************/
#if defined(RSIM_WITH_SL_SOLVER) && defined(__cplusplus)
#define LINKAGE_SPEC extern "C"  
#elif defined(RSIM_WITH_SL_SOLVER) && !defined(__cplusplus)
#define LINKAGE_SPEC extern    
#elif defined(BUILDING_SLEXEC_SIMBRIDGE)
# include "package.h"
#define LINKAGE_SPEC extern "C" DLL_EXPORT_SYM
#elif defined(SLEXEC_SIMBRIDGE_SIMSTRUCT_UNITTEST)
#define LINKAGE_SPEC extern "C"  
#else
# error Unrecognized use.
#endif

/********************************************************************************
 * Section 2. SimStruct Accessors
 *******************************************************************************/

LINKAGE_SPEC time_T ssGetTaskTime(const SimStruct* S, int sti);

LINKAGE_SPEC time_T ssGetT(const SimStruct* S);
LINKAGE_SPEC void _ssSetT(SimStruct* S, time_T t);

LINKAGE_SPEC int ssGetTNextWasAdjusted(const SimStruct* S, int sti);

LINKAGE_SPEC time_T _ssGetVarNextHitTime(const SimStruct* S, int sti);
LINKAGE_SPEC void _ssSetVarNextHitTime(SimStruct* S, int sti, time_T v);

LINKAGE_SPEC int ssGetSolverNeedsReset(const SimStruct* S);
LINKAGE_SPEC void ssSetSolverNeedsReset(SimStruct* S);
LINKAGE_SPEC void _ssClearSolverNeedsReset(SimStruct* S);

LINKAGE_SPEC int ssIsSampleHit(const SimStruct* S, int sti, int tid);
LINKAGE_SPEC int ssIsSpecialSampleHit(const SimStruct* S, int my_sti, 
                                      int promoted_sti, int tid);

LINKAGE_SPEC int ssIsContinuousTask(const SimStruct* S, int tid);

LINKAGE_SPEC uint32_T ssGetClockTick(const SimStruct *S, int sti);
LINKAGE_SPEC uint32_T ssGetClockTickH(const SimStruct *S, int sti);

LINKAGE_SPEC int ssGetStopRequested(const SimStruct* S);
LINKAGE_SPEC void ssSetStopRequested(SimStruct* S, int value);

LINKAGE_SPEC int ssGetSolverIsAtLeftPostOfContZcEvent(SimStruct* S);
LINKAGE_SPEC int ssGetSolverIsAtRightPostOfContZcEvent(SimStruct* S);

#define ssGetTPtr(S) \
        (S)->mdlInfo->t                                 /*   (time_T *)    */
#define _ssSetTPtr(S,t_ptr) \
          (S)->mdlInfo->t = (t_ptr)


/********************************************************************************
 * Section 2. External mode hooks
 *******************************************************************************/

typedef struct ssBridgeExtModeCallbacks_tag {

    void (*oneStepFcn)(RTWExtModeInfo*, int, boolean_T*);
    void (*uploadCheckTriggerFcn)(int_T);
    void (*uploadFcn)(int_T, time_T);
    void (*checkEndTriggerFcn)();
      
} ssBridgeExtModeCallbacks_T;

typedef struct ssBridgeSetupAndTerminationCallbacks_tag{
    void (*setup_model)(SimStruct *);
    const char* (*jacobianPatternFileName)();
} ssBridgeSetupAndTerminationCallbacks_T;

LINKAGE_SPEC time_T rteiGetT(RTWExtModeInfo* ei);
# define rteiSetTPtr(E,p) /* no-op */

/********************************************************************************
 * Section 3. API to the new SimulinkExecution module.
 *******************************************************************************/

LINKAGE_SPEC void ssInitializeExecutionEngine(
    SimStruct* S,
    ssBridgeExtModeCallbacks_T* extModeCallbacks,
    ssBridgeSetupAndTerminationCallbacks_T* setupCallbacks);

LINKAGE_SPEC void ssSetSolverParams(SimStruct* S);

LINKAGE_SPEC int ssOneStep(SimStruct* S);

LINKAGE_SPEC void ssTerminateExecutionEngine(SimStruct* S);

/********************************************************************************
 * Section 4. Stub out these macros for now, once everything is functional we can
 * remove them completely
 *******************************************************************************/

#define _ssSetSampleHit(S,sti,val)  /* no-op */
#define _ssSetSampleHitPtr(S,ptr)  /* no-op */
#define _ssSetTaskTime(S,sti,time) /* no-op */
#define ssSetTNextWasAdjustedPtr(S, ptr)  /* no-op */
#define _ssSetPerTaskSampleHitsPtr(S, hitsMatrix)  /* no-op */
#define ssSetTimeOfNextSampleHitPtr(S, ptr)  /* no-op */

/*

#define ssGetSampleHitPtr(S)
#define ssGetPerTaskSampleHitsPtr(S)

#define ssIsAsyncTID(S, sti)
#define ssIsContinuousTask(S, tid)
#define _ssSetTaskTime(S, sti, time)
#define ssIsSampleHitInTask(S, my_sti, tid)
#define _ssSetSampleHitInTask(S, task_j, task_i, hit)

#define ssGetTimeOfNextSampleHitPtr(S)

#define ssGetVarNextHitTimesListPtr(S)
#define _ssSetVarNextHitTimesListPtr(S,listPtr)


*/

#endif
