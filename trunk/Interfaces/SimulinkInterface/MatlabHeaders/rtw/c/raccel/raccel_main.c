/*
 * Abstract:
 *   Main program for the Rapid Acceleration target.
 *
 * $Revision: 1.1.6.26 $
 * Copyright 2007-2010 The MathWorks, Inc.
 */


#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include "tmwtypes.h"
#include "simstruc.h"
#include "rt_logging.h"
#include "rtmodel.h"
#include "rt_nonfinite.h"
#include "raccel.h"
#include "raccel_sup.h"

#include "ext_work.h"

#include "sl_solver_rtw.h"
#include "slexec_cgparallel.h"
/*=========*
 * Defines *
 *=========*/

#ifndef TRUE
#define FALSE (0)
#define TRUE  (1)
#endif

#ifndef EXIT_FAILURE
#define EXIT_FAILURE  1
#endif
#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS  0
#endif

#define GOTO_EXIT_IF_ERROR(msg, cond)         \
        if (cond) {                           \
            errorPrefix = msg;                \
            goto EXIT_POINT;                  \
        }

#define ERROR_EXIT(msg, cond)                 \
        if (cond) {                           \
            (void)fprintf(stderr, msg, cond); \
            return(EXIT_FAILURE);             \
        }

/*====================*
 * External functions *
 *====================*/
extern SimStruct *raccel_register_model(void);
extern void raccel_setup_MMIStateLog(SimStruct*);

extern void MdlInitializeSizes(void);
extern void MdlInitializeSampleTimes(void);
extern void MdlStart(void);
extern void MdlOutputs(int_T tid);
extern void MdlUpdate(int_T tid);
extern void MdlTerminate(void);

#define SL_MAX(A, B)   (((A) > (B)) ? (A) : (B))


/*=============*
 * Global data *
 *=============*/
boolean_T          gblInstalledSigHandlers;
int         gblVerboseFlag = 0;
extern int         gblTimeLimit;
extern const char* gblInportFileName;
extern const char* gblSlvrJacPatternFileName;
extern boolean_T   gbl_raccel_isMultitasking;
extern boolean_T   gbl_raccel_tid01eq;
extern int_T       gbl_raccel_NumST;
extern boolean_T   gblExtModePortCheckOnly;
extern boolean_T   gblExtModeEnabled;
extern const char* gblErrorStatus;
extern ParallelExecutionOptions parallelExecutionOptions;


#  define rtExtModeSingleTaskUpload(S)                          \
   if (gblExtModeEnabled) {                                     \
        int stIdx;                                              \
        rtExtModeUploadCheckTrigger(ssGetNumSampleTimes(S));    \
        for (stIdx=0; stIdx<ssGetNumSampleTimes(S); stIdx++) {  \
            if (ssIsSampleHit(S, stIdx, 0 /*unused*/)) {        \
                rtExtModeUpload(stIdx,ssGetTaskTime(S,stIdx));  \
            }                                                   \
        }                                                       \
   }

/* Function: rsimOutputLogUpdate ===============================================
 *
 */
static void rsimOutputLogUpdate(SimStruct* S)
{
    double currTime  = ssGetT(S);
    bool   logOutput = !ssGetOutputTimesOnly(S);

#ifdef DEBUG_TIMING
    rsimDisplayTimingData(S,
                          sizeof(struct SimStruct_tag),
                          sizeof(struct _ssMdlInfo));
#endif

    if (gblExtModeEnabled) {
        rtExtModeOneStep(ssGetRTWExtModeInfo(S),
                         ssGetNumSampleTimes(S),
                         (boolean_T*)&ssGetStopRequested(S));
        if (ssGetStopRequested(S)) return;
    }

    /* Setup the task times and sample hit flags for the discrete rates */
    rsimUpdateDiscreteTaskTimesAndSampleHits(S);
    if (ssGetErrorStatus(S) != NULL) return;

    /*
     * See if we are at an outputTime, and if so set logOutput to true and
     * increment the next output time index to point to the next entry in
     * the outputTimes array.
     */
    if ( ssGetNumOutputTimes(S) > 0 &&
         ssGetOutputTimesIndex(S) < ssGetNumOutputTimes(S) ) {
        time_T nextOutputTime = ssGetNextOutputTime(S);
        /* utAssert(currTime <= nextOutputTime); */
        if (currTime == nextOutputTime) {
            uint_T idx = ssGetOutputTimesIndex(S);
            ssSetOutputTimesIndex(S, idx+1);
            logOutput = 1; /* this is one of the specified output times */
        }
    }
    ssSetLogOutput(S, logOutput);

    MdlOutputs(0);

    if (gblExtModeEnabled) {
        rtExtModeSingleTaskUpload(S);
    }

    if (ssGetLogOutput(S)) {
        (void)rt_UpdateTXYLogVars(ssGetRTWLogInfo(S), ssGetTPtr(S));
        if (ssGetErrorStatus(S) != NULL) return;
    }

    MdlUpdate(0);
    if (ssGetErrorStatus(S) != NULL) return;

    ssSetLogOutput(S, FALSE);
    ssSetTimeOfLastOutput(S, currTime);

    /* Update the timing engine and determine the solver stop time */
    rsimUpdateTimingEngineAndSolverStopTime(S);
    if (ssGetErrorStatus(S) != NULL) return;

    if (gblExtModeEnabled) {
        rtExtModeCheckEndTrigger();
    }
    return;

} /* rsimOutputLogUpdate */


/* Function: rsimOneStepST =====================================================
 *
 *      Perform one step of the model.
 *      Errors are set in the SimStruct's ErrorStatus, NULL means no errors.
 */
static void rsimOneStepST(SimStruct *S)
{
    ssSetSimTimeStep(S, MAJOR_TIME_STEP);

    /* Check if the solver found ZC event.
     *  if yes, this is a major step at right post */
    if ( ssGetSolverFoundContZcEvents(S) ){
        ssSetSolverIsAtRightPostOfContZcEvent(S, true);
    }

    /* Clear the flag that blocks set in their major time step methods (output,
       update, etc.) to flag that they have changed their state. If this flag is
       true then we need to run model outputs again at the first minor step
       (i.e., at the same time as the last major step).*/
    
    ssClearBlkStateChange(S);
    rsimOutputLogUpdate(S);

    /* Done with the step. Set the right post flag to false
     * It is immaterial whether this step was really a right post step */
    ssSetSolverIsAtRightPostOfContZcEvent(S, false);

    if (ssGetErrorStatus(S) != NULL) return;

    /* Call rsimAdvanceSolver only if no stop was requested (540586) */
    if (!ssGetStopRequested(S)) rsimAdvanceSolver(S);

} /* rsimOneStepST */


/* Function: rsimOneStepMT =====================================================
 *
 *      Perform one step of the model.
 *      Errors are set in the SimStruct's ErrorStatus, NULL means no errors.
 */
static void rsimOneStepMT(SimStruct *S)
{
    int_T  i;
    const int_T FIRST_TID = gbl_raccel_tid01eq ? 1 : 0;
    int_T* sampleHit = ssGetSampleHitPtr(S);

    ssSetSimTimeStep(S, MAJOR_TIME_STEP);
    
    /* Clear the flag that blocks set in their major time step methods (output,
       update, etc.) to flag that they have changed their state. If this flag is
       true then we need to run model outputs again at the first minor step
       (i.e., at the same time as the last major step).*/
    
    ssClearBlkStateChange(S);

#ifdef DEBUG_TIMING
    rsimDisplayTimingData(S,
                          sizeof(struct SimStruct_tag),
                          sizeof(struct _ssMdlInfo));
#endif
    if (gblExtModeEnabled) {
        rtExtModeOneStep(ssGetRTWExtModeInfo(S),
                         ssGetNumSampleTimes(S),
                         (boolean_T*)&ssGetStopRequested(S));
        if (ssGetStopRequested(S)) return;
    }
    /* Setup the task times and sample hit flags for the discrete rates */
    rsimUpdateDiscreteTaskTimesAndSampleHits(S);
    if (ssGetErrorStatus(S) != NULL) return;

    ssSetLogOutput(S,TRUE);

    /* Do output, log, update for the base rate */
    MdlOutputs(FIRST_TID);

    if (gblExtModeEnabled) {
        rtExtModeUploadCheckTrigger(ssGetNumSampleTimes(S));
        rtExtModeUpload(FIRST_TID, ssGetTaskTime(S, FIRST_TID));
    }

    if (ssGetLogOutput(S)) {
        (void)rt_UpdateTXYLogVars(ssGetRTWLogInfo(S), ssGetTPtr(S));
        if (ssGetErrorStatus(S) != NULL) return;
    }

    MdlUpdate(FIRST_TID);
    if (ssGetErrorStatus(S) != NULL) return;

    /* Do not log outputs during minor time steps */
    ssSetLogOutput(S, FALSE);
    ssSetTimeOfLastOutput(S,ssGetT(S));

    /* Call the solver push time forward based on the continuous dynamics */
    if (ssGetSampleTime(S,0) == CONTINUOUS_SAMPLE_TIME &&
        !ssGetStopRequested(S) ) {
        rsimUpdateSolverStopTimeForFixedStepMultiTaskingSim(S);
        if (!ssGetStopRequested(S)) rsimAdvanceSolver(S);
    }

    /* Do output and update for the remaining rates */
    ssSetLogOutput(S, TRUE);
    for (i = FIRST_TID+1; i < gbl_raccel_NumST; i++) {
        if ( !sampleHit[i] ) continue;
        MdlOutputs(i);
        rtExtModeUpload(i, ssGetTaskTime(S,i));
        MdlUpdate(i);
    }
    if (ssGetErrorStatus(S) != NULL) return;
    ssSetLogOutput(S, FALSE);

    if (gblExtModeEnabled) {
        rtExtModeCheckEndTrigger();
    }

    /* Update the timing engine and determine the solver stop time */
    rsimUpdateTimingEngineAndSolverStopTime(S);
    if (ssGetErrorStatus(S) != NULL) return;

} /* rsimOneStepMT */

/* Function: WriteResultsToMatFile =============================================
 *
 *     This function is called from main for normal exit or from the
 *     signal handler in case of abnormal exit (^C, time out etc).
 */
void WriteResultsToMatFile(SimStruct* S)
{
   rt_StopDataLoggingForRaccel(gblMatLoggingFilename,ssGetRTWLogInfo(S),0);
  
}

/* Function: main ==============================================================
 *
 *      Execute model on a generic target such as a workstation.
 */
int_T main(int_T argc, char_T *argv[])
{
    SimStruct  *S                 = NULL;
    boolean_T  calledMdlStart     = FALSE;
    boolean_T  dataLoggingActive  = FALSE;
    boolean_T  initializedExtMode = FALSE;
    const char *result;
    const char *program;
    time_t     now;
    int matFileFormat;
    const char *errorPrefix       = NULL;

    program = argv[0];
    gblInstalledSigHandlers = FALSE;
    /* No re-defining of data types allowed. */
    if ((sizeof(real_T)   != 8) ||
        (sizeof(real32_T) != 4) ||
        (sizeof(int8_T)   != 1) ||
        (sizeof(uint8_T)  != 1) ||
        (sizeof(int16_T)  != 2) ||
        (sizeof(uint16_T) != 2) ||
        (sizeof(int32_T)  != 4) ||
        (sizeof(uint32_T) != 4) ||
        (sizeof(boolean_T)!= 1)) {
        ERROR_EXIT("Error: %s\n", "Re-defining data types such as REAL_T is not supported by RSIM");
    }

    rt_InitInfAndNaN(sizeof(real_T));


    /* Parse arguments */
    gblErrorStatus = ParseArgs(argc, argv);
    ERROR_EXIT("Error parsing input arguments: %s\n", gblErrorStatus);

    /* Initialize the model */
    S = raccel_register_model();
    ERROR_EXIT("Error during model registration: %s\n", ssGetErrorStatus(S));

    ssClearFirstInitCondCalled(S);
    /* Override StopTime */
    if (gblFinalTimeChanged) ssSetTFinal(S,gblFinalTime);

    MdlInitializeSizes();
    MdlInitializeSampleTimes();

    /* We don't have GOTO_EXIT_IF_ERROR here as engine is not initialized 
       via rsimInitializeEngine */
    rt_RapidReadMatFileAndUpdateParams(S);
    ERROR_EXIT("Error reading parameter data from mat-file: %s\n",
              ssGetErrorStatus(S));

    /* load solver options */
    rsimLoadSolverOpts(S);
    ERROR_EXIT("Error loading solver options: %s\n", ssGetErrorStatus(S));

# if defined(DEBUG_SOLVER)
    rsimEnableDebugOutput(sizeof(struct SimStruct_tag),
                          sizeof(struct _ssMdlInfo));
# endif

#ifdef RACCEL_ENABLE_PARALLEL_EXECUTION
    parallelExecutionOptions.numberOfNodes = RACCEL_NUM_PARALLEL_NODES;
    parallelExecutionOptions.numberOfThreads = 
        RACCEL_PARALLEL_EXECUTION_NUM_THREADS;
    cg_initialize_parallel_execution(parallelExecutionOptions);
#endif

    rsimInitializeEngine(S);
    ERROR_EXIT("Error initializing RSIM engine: %s\n", ssGetErrorStatus(S));

    /* initialize external model */
    if (gblExtModeEnabled) {
        rtExtModeCheckInit(ssGetNumSampleTimes(S));
        initializedExtMode = TRUE;       
    }

    raccel_setup_MMIStateLog(S);

    if (ssIsVariableStepSolver(S)) {
        (void)rt_StartDataLoggingWithStartTime(ssGetRTWLogInfo(S),
                                               ssGetTStart(S),
                                               ssGetTFinal(S),
                                               0.0,
                                               &ssGetErrorStatus(S));
    } else {
        (void)rt_StartDataLoggingWithStartTime(ssGetRTWLogInfo(S),
                                               ssGetTStart(S),
                                               ssGetTFinal(S),
                                               ssGetStepSize(S),
                                               &ssGetErrorStatus(S));
    }
    GOTO_EXIT_IF_ERROR("Error starting data logging: %s\n", 
                       ssGetErrorStatus(S));
    dataLoggingActive = TRUE;

    if (gblExtModeEnabled) {
        /* If -w flag is specified wait here for connect signal from host */
        rtExtModeWaitForStartPkt(ssGetRTWExtModeInfo(S),
                                 ssGetNumSampleTimes(S),
                                 (boolean_T *)&ssGetStopRequested(S));
        if (ssGetStopRequested(S)) goto EXIT_POINT;
    }

    /* Start the model */
    now = time(NULL);
    if(gblVerboseFlag) {
        (void)printf("\n** Starting model @ %s", ctime(&now));
        }

    /* Enable logging in the MdlStart method */
    ssSetLogOutput(S,TRUE);

    /* Enable -i option to load inport data file */
    result = rt_RapidReadInportsMatFile(gblInportFileName, &matFileFormat);
    if (result!= NULL) {
        ssSetErrorStatus(S,result);
        GOTO_EXIT_IF_ERROR("Error starting model: %s\n",  ssGetErrorStatus(S));
    }

    MdlStart();
    ssSetLogOutput(S,FALSE);

    calledMdlStart = TRUE;
    GOTO_EXIT_IF_ERROR("Error starting model: %s\n", ssGetErrorStatus(S));

    result = rt_RapidCheckRemappings();
    ssSetErrorStatus(S,result);
    GOTO_EXIT_IF_ERROR("Error: %s\n", ssGetErrorStatus(S));

    /* Create solver data */
    rsimCreateSolverData(S, gblSlvrJacPatternFileName);
    GOTO_EXIT_IF_ERROR("Error creating solver data: %s\n", ssGetErrorStatus(S));

    ssSetFirstInitCondCalled(S);

    /*********************
     * Execute the model *
     *********************/

    /* Install Signal and Run time limit handlers */
    
    rsimInstallAllHandlers(S,WriteResultsToMatFile,gblTimeLimit);
    

    gblInstalledSigHandlers = TRUE;
    GOTO_EXIT_IF_ERROR("Error: %s\n", ssGetErrorStatus(S));

    while ( ((ssGetTFinal(S)-ssGetT(S)) > (fabs(ssGetT(S))*DBL_EPSILON)) ) {

#ifdef RACCEL_ENABLE_PARALLEL_EXECUTION
        cg_analyze_parallel_execution();
#endif
        if (gblExtModeEnabled) {
            rtExtModePauseIfNeeded(ssGetRTWExtModeInfo(S),
                                   ssGetNumSampleTimes(S),
                                   (boolean_T *)&ssGetStopRequested(S));
        }
        if (ssGetStopRequested(S)) break;

        if (gbl_raccel_isMultitasking) {
            rsimOneStepMT(S);
        } else {
            rsimOneStepST(S);
        }
        if (ssGetErrorStatus(S)) break;
    }
    if (ssGetErrorStatus(S) == NULL && !ssGetStopRequested(S)) {
        /* Do a major step at the final time */
        if (gbl_raccel_isMultitasking) {
            rsimOneStepMT(S);
        } else { 
            rsimOutputLogUpdate(S);
        }
    }

  EXIT_POINT:
    /********************
     * Cleanup and exit *
     ********************/
    if (ssGetErrorStatus(S) != NULL) {
        if (errorPrefix) {
            (void)fprintf(stderr, errorPrefix, ssGetErrorStatus(S));
        } else {
            (void)fprintf(stderr, "Error: %s\n", ssGetErrorStatus(S));
        }
    }
    if (gblInstalledSigHandlers) {
        rsimUninstallNonfatalHandlers();
        gblInstalledSigHandlers = FALSE;
    }
    if (dataLoggingActive){
        WriteResultsToMatFile(S);
    }

    
    rsimTerminateEngine(S,0);
    
        
    if (initializedExtMode) {
        rtExtModeShutdown(ssGetNumSampleTimes(S));
    }
    if (calledMdlStart) {
        MdlTerminate();
    }

    rt_RapidFreeGbls(matFileFormat);
    return ssGetErrorStatus(S) ? EXIT_FAILURE : EXIT_SUCCESS;

} /* end main */

/* EOF */
