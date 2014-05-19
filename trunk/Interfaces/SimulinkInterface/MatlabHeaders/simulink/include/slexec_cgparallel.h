#ifndef SIMULINK_EXECUTION_CGPARALLEL_H
#define SIMULINK_EXECUTION_CGPARALLEL_H

/**
 * @file slexec_cgparallel.h
 *
 * This header is the exported interface to the simulink/execution/cgparallel. 
 * It is used by the Rapid Accelerator targets.
 *
 * 
 * Copyright 2008-2011 The MathWorks, Inc.
 */


/********************************************************************************
 * Section 1. Determine Linkage based on where this file is being included
 *******************************************************************************/
#if defined(RSIM_WITH_SL_SOLVER) && defined(__cplusplus)
#define LINKAGE_SPEC  extern "C"  
#elif defined(RSIM_WITH_SL_SOLVER) && !defined(__cplusplus)
#define LINKAGE_SPEC extern    
#elif defined(BUILDING_SLEXEC_CGPARALLEL)
# include "package.h"
# define LINKAGE_SPEC     extern "C" DLL_EXPORT_SYM
#elif defined(SLEXEC_CGPARALLEL_SIMSTRUCT_UNITTEST)
#define LINKAGE_SPEC  extern "C"  
#else
# error Unrecognized use.
#endif

#include "tmwtypes.h"
#include "simstruc.h"

typedef void (*ParallelForTaskFunction)(int);

typedef enum {
    PARALLEL_EXECUTION_AUTO = -1,
    PARALLEL_EXECUTION_OFF = 0,
    PARALLEL_EXECUTION_ON = 1
} ParallelExecutionMode;

typedef struct ParallelExecutionOptions_tag {
    int numberOfNodes;
    ParallelExecutionMode parallelExecutionMode;
    int numberOfThreads;
    int numberOfStepsToAnalyze;
    boolean_T enableTiming;
    const char* timingOutputFilename;
    const char* nodeExecutionModesFilename;
} ParallelExecutionOptions;


LINKAGE_SPEC void cg_initialize_parallel_execution(
    ParallelExecutionOptions options);

LINKAGE_SPEC void cg_analyze_parallel_execution();

LINKAGE_SPEC void cg_parallel_for(int loopSize, 
                                  ParallelForTaskFunction taskFunction, 
                                  int nodeIndex);

#endif
