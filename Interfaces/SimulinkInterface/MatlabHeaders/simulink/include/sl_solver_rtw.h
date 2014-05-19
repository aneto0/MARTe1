#ifdef SUPPORTS_PRAGMA_ONCE
# pragma once
#endif
#ifndef sl_solver_rtw_h
#define sl_solver_rtw_h
/*
 *
 * Copyright 2008-2010 The MathWorks, Inc.
 *
 * This header is the intrace to the sl_solver_rtw module. It is used by the
 * RTW-RSim and Rapid Accelerator targets.
 *
 * $Revision: 1.1.6.7 $
 *
 */

#include "simstruc.h"

#if defined(RSIM_WITH_SL_SOLVER)
 /*
  * This header is being included by either the RSim or RapidAccelerator runtime
  * interface (both require C linkage)
  */
# define LINKAGE_SPEC  extern
#else
# include "package.h"
# if defined(SL_SOLVER_RTW_INTERNAL)
  /*
   * This header is being included by files inside src/sl_solver_rtw
   */
#  define LINKAGE_SPEC  PUBLISHED_EXTERN_C
# else
  /*
   * This file is being include by other internal modules.
   */
#  define LINKAGE_SPEC  IMPORT_EXTERN_C
# endif
#endif

LINKAGE_SPEC void rsimEnableDebugOutput(size_t simStructSize,
                                    size_t ssMdlInfoSize);

LINKAGE_SPEC void rsimCreateSolverData(SimStruct* S, const char* slvrJpatternFileName);

LINKAGE_SPEC void rsimInitializeEngine(SimStruct* S);

LINKAGE_SPEC void rsimAdvanceSolver(SimStruct* S);

LINKAGE_SPEC void rsimUpdateDiscreteTaskTimesAndSampleHits(SimStruct* S);

LINKAGE_SPEC void rsimUpdateSolverStopTimeForFixedStepMultiTaskingSim(SimStruct* S);

LINKAGE_SPEC void rsimUpdateTimingEngineAndSolverStopTime(SimStruct* S);

LINKAGE_SPEC void rsimTerminateEngine(SimStruct* S, int verbose);

LINKAGE_SPEC void rsimDisplayTimingData(SimStruct* S,
                                    size_t simStructSize,
                                    size_t ssMdlInfoSize);

LINKAGE_SPEC void rsimInstallAllHandlers(
    SimStruct* S,
    void (*logDataFcn)(SimStruct* s),
    int        timeLimit );
    
LINKAGE_SPEC void rsimUninstallNonfatalHandlers(void);

LINKAGE_SPEC int isVariableStepSolver(const char* slvrName);

LINKAGE_SPEC int isFixedStepSolver(const char* slvrName);

#endif /* sl_solver_rtw_h */
