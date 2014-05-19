
/******************************************************************
 *
 *  File: raccel_mat.c
 *
 *  $Revision: 1.1.6.17 $
 *
 *  Abstract:
 *      - provide matfile handling for reading and writing matfiles
 *        for use with rsim stand-alone, non-real-time simulation
 *      - provide functions for swapping rtP vector for parameter tuning
 *
 * Copyright 2007-2010 The MathWorks, Inc.
 ******************************************************************/

/*
 * This file is still using the old 32-bit mxArray API
 */
#define MX_COMPAT_32

/* INCLUDES */
#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  <math.h>
#include  <float.h>

/*
 * We want access to the real mx* routines in this file and not their RTW
 * variants in rt_matrx.h, the defines below prior to including simstruc.h
 * accomplish this.
 */
#include "mat.h"
#define TYPEDEF_MX_ARRAY
#define rt_matrx_h
#include "simstruc.h"
#undef rt_matrx_h
#undef TYPEDEF_MX_ARRAY

#include  "raccel.h"
#include "slexec_cgparallel.h"

extern const char *gblToFileSuffix;
extern const char* gblSolverOptsFilename;
extern int         gblSolverOptsArrIndex;
extern int         gblNumFrFiles;
extern int         gblNumToFiles;

FileInfo *gblFromFileInfo = NULL;
FileInfo *gblToFileInfo = NULL;
ParallelExecutionOptions parallelExecutionOptions;
/* ParallelExecutionOptions parallelExecutionOptions =  */
/* {0, PARALLEL_EXECUTION_ON, 0, false, NULL}; */


/* Function: rt_RAccelAddToFileSuffix ==========================================
 * Abstract:
 * This adds the ToFileSuffix
 */
void rt_RAccelAddToFileSuffix(char *fileName)
{
    /* We assume that the char* pointer passed in has sufficent length
     * to hold the suffix also */
    static char origFileName[mxMAXNAM];
    const char *dot = strchr(fileName, '.');
    size_t actFNameLen = (size_t)(dot-(fileName));
    (void)strcpy(origFileName, fileName);
    if (gblToFileSuffix == NULL) return;
    (void)strncpy(fileName, origFileName, actFNameLen);
    fileName[actFNameLen] = '\0';
    (void)strcat(fileName, gblToFileSuffix);
    (void)strcat(fileName, origFileName+actFNameLen);
} /* end rt_RAccelAddToFileSuffix */


/* Function rt_RAccelReplaceFromFilename =========================================
 * Abstract:
 *  This function replaces the Name for From File blocks
 */

void  rt_RAccelReplaceFromFilename(const char *blockpath, char *fileName)
{
   int_T fileIndex;
   for (fileIndex=0; fileIndex<gblNumFrFiles; fileIndex++) {
	if (strcmp(gblFromFileInfo[fileIndex].blockpath, blockpath)==0) {
            (void)strcpy(fileName,gblFromFileInfo[fileIndex].filename);
            break;
        }
    }
}

/* Function rt_RAccelReplaceToFilename =========================================
 * Abstract:
 *  This function replaces the Name for To File blocks
 */

void  rt_RAccelReplaceToFilename(const char *blockpath, char *fileName)
{
   int_T fileIndex;
    for (fileIndex=0; fileIndex<gblNumToFiles; fileIndex++) {
	if (strcmp(gblToFileInfo[fileIndex].blockpath, blockpath)==0) {
            (void)strcpy(fileName,gblToFileInfo[fileIndex].filename);
            break;
        }
    }
}



/*******************************************************************************
 * strcmpi
 *
 */

int strcmpi(const char* s1, const char* s2) {
    
    const unsigned char *p1 = (const unsigned char *) s1;
    const unsigned char *p2 = (const unsigned char *) s2;
    unsigned char c1, c2;
    
    if (p1 == p2)
        return 0;
    
    do
    {
        c1 = tolower (*p1++);
        c2 = tolower (*p2++);
        if (c1 == '\0')
            break;
    }
    while (c1 == c2);
    
    return c1 - c2;
}



/*******************************************************************************
 *
 * Rountine to load solver options.
 * The options listed below can be changed as
 * the start of execution using the -S flag
 *
 *            Solver
 *            RelTol
 *            MinStep
 *            MaxStep
 *            InitialStep
 *            Refine
 *            MaxOrder
 *              ExtrapolationOrder        --  used by ODE14x
 *              NumberNewtonIterations    --  used by ODE14x
 *              SolverResetMethod -- used by ODE15s, ODE23t, ODE23tb
 */
void rsimLoadSolverOpts(SimStruct* S)
{
    MATFile*    matf;
    mxArray*    pa;
    mxArray*    pFromFile;
    mxArray*    pToFile;
    mxArray*    sa;
    size_t      idx    = 0;
    int_T       fileIndex;
    const char* result = NULL;
    RTWLogInfo *li = ssGetRTWLogInfo(S);

    if (gblSolverOptsFilename == NULL) return;

    if ((matf=matOpen(gblSolverOptsFilename,"rb")) == NULL) {
        result = "could not find MAT-file containing new parameter data";
        goto EXIT_POINT;
    }


    /* Load From File blocks information*/
    {
        if ((pFromFile=matGetVariable(matf,"fromFile")) == NULL){
            result = "error reading From File blocks options from MAT-file";
            goto EXIT_POINT;
        }

        if (gblNumFrFiles>0) {
            gblFromFileInfo = (FileInfo*)calloc(gblNumFrFiles,sizeof(FileInfo));
            if (gblFromFileInfo==NULL) {
                result = "memory allocation error (gblFromFileInfo)";
                goto EXIT_POINT;
            }
     
            for (fileIndex=0; fileIndex < gblNumFrFiles; fileIndex++){
                char storedBlockpath[MAXSTRLEN] = "\0";
                char storedFilename[MAXSTRLEN] = "\0";
                mxGetString(mxGetField(pFromFile,fileIndex,"blockPath"),storedBlockpath,MAXSTRLEN);
                (void)strcpy(gblFromFileInfo[fileIndex].blockpath,storedBlockpath);
                mxGetString(mxGetField(pFromFile,fileIndex,"filename"),storedFilename,MAXSTRLEN);
                (void)strcpy(gblFromFileInfo[fileIndex].filename,storedFilename);
            }
        }
    }

    /* Load To File blocks information*/
    {
        if ((pToFile=matGetVariable(matf,"toFile")) == NULL){
            result = "error reading From File blocks options from MAT-file";
            goto EXIT_POINT;
        }

        if (gblNumToFiles>0) {
            gblToFileInfo = (FileInfo*)calloc(gblNumToFiles,sizeof(FileInfo));
            if (gblToFileInfo==NULL) {
                result = "memory allocation error (gblFromFileInfo)";
                goto EXIT_POINT;
            }
        

            for (fileIndex=0; fileIndex < gblNumToFiles; fileIndex++){
                char storedBlockpath[MAXSTRLEN] = "\0";
                char storedFilename[MAXSTRLEN] = "\0";
                mxGetString(mxGetField(pToFile,fileIndex,"blockPath"),storedBlockpath,MAXSTRLEN);
                (void)strcpy(gblToFileInfo[fileIndex].blockpath,storedBlockpath);
                mxGetString(mxGetField(pToFile,fileIndex,"filename"),storedFilename,MAXSTRLEN);
                (void)strcpy(gblToFileInfo[fileIndex].filename,storedFilename);
            }
        }
    }

     /* Load the structure of solver options */
    if ((pa=matGetVariable(matf,"slvrOpts")) == NULL ) {
        result = "error reading new solver options from MAT-file";
        goto EXIT_POINT;
    }
        /* Should be structure */
    if ( !mxIsStruct(pa) || (mxGetN(pa) > 1 && mxGetM(pa) > 1) ) {
        result = "solver options should be a vector of structures";
        goto EXIT_POINT;
    }

    if (gblSolverOptsArrIndex > 0) idx = gblSolverOptsArrIndex;

    if (idx > 0 && mxGetNumberOfElements(pa) <= idx) {
        result = "options array size is less than the specified array index";
        goto EXIT_POINT;
    }


    /* Solver     */
    {
        const char* opt = "Solver";
        static char solver[256] = "\0";
        if ( (sa=mxGetField(pa,idx,opt)) != NULL ) {
            if (mxGetString(sa, solver, 256) != 0) {
                result = "error reading solver option Solver Name";
                goto EXIT_POINT;
            }
            if (ssIsVariableStepSolver(S)) {
                if (isVariableStepSolver(solver)) {
                    if (strcmpi(ssGetSolverName(S), "variablestepdiscrete") 
                        != 0) {
                        ssSetSolverName(S, solver);
                    }
                } else {
                    result = "solver type does not match";
                    goto EXIT_POINT;
                }
            } else {
                bool isVariable = isVariableStepSolver(solver);
                if (!isVariable) {
                    if (strcmpi(ssGetSolverName(S), "fixedstepdiscrete") 
                        != 0) {
                        ssSetSolverName(S, solver);
                        if (strcmpi(ssGetSolverName(S), "ode14x") == 0) {
                            ssSetSolverExtrapolationOrder(S, 4);
                            ssSetSolverNumberNewtonIterations(S, 1);
                        }
                    }
                } else {
                    result = "solver type does not match";
                    goto EXIT_POINT;
                }
            }
        }
    }

    /* RelTol */
    {
        const char* opt = "RelTol";
        if ( (ssIsVariableStepSolver(S)) &&
             ((sa=mxGetField(pa,idx,opt)) != NULL) ) {
            if ( !mxIsDouble(sa) || mxGetNumberOfElements(sa) != 1 ) {
                result = "error reading solver option RelTol";
                goto EXIT_POINT;
            }
            ssSetSolverRelTol(S, mxGetPr(sa)[0]);
        }
    }

    /* AbsTol */
    {
        const char* opt = "AbsTol";
        int     nx   = ssGetNumContStates(S);

        /* Load absolute tolerance if :
         * Is variable step solver AND 
         * there are continuous states in the model */

        if ( (ssIsVariableStepSolver(S)) && (nx > 0) &&
             ((sa=mxGetField(pa,idx,opt)) != NULL) ) {
            double* aTol = ssGetAbsTolVector(S);
            const uint8_T* aTolControl = ssGetAbsTolControlVector(S);
            
            int     n    = mxGetNumberOfElements(sa);

            if ( !mxIsDouble(sa) || (n != 1 && n != nx) ) {
                result = "error reading solver option AbsTol";
                goto EXIT_POINT;
            }
            if (n == 1) {
                int i;
                for (i = 0; i < nx; i++) {
                    if (aTolControl[i] == SL_SOLVER_TOLERANCE_GLOBAL){
                        aTol[i] = mxGetPr(sa)[0];
                    }
                }
            } else {
                int i;
                for (i = 0; i < nx; i++) {
                    if (aTolControl[i] == SL_SOLVER_TOLERANCE_GLOBAL){
                        aTol[i] = mxGetPr(sa)[i];
                    }
                }
            }
        }
    }

    /* Refine */
    {
        const char* opt = "Refine";
        int         refine;
        if ( (ssIsVariableStepSolver(S)) &&
             ((sa=mxGetField(pa,idx,opt)) != NULL) ) {
            if ( !mxIsDouble(sa) || mxGetNumberOfElements(sa) != 1 ) {
                result = "error reading solver option Refine";
                goto EXIT_POINT;
            }
            refine = (int) (mxGetPr(sa)[0]);
            ssSetSolverRefineFactor(S, refine);
        }
    }

    /* MaxStep */
    {
        const char* opt = "MaxStep";
        if ( (ssIsVariableStepSolver(S)) &&
             ((sa=mxGetField(pa,idx,opt)) != NULL) ) {
            if ( !mxIsDouble(sa) || mxGetNumberOfElements(sa) != 1 ) {
                result = "error reading solver option MaxStep";
                goto EXIT_POINT;
            }
            ssSetMaxStepSize(S, mxGetPr(sa)[0]);
        }
    }

    /* MinStep */
    {
        const char* opt = "MinStep";
        if ( (ssIsVariableStepSolver(S)) &&
             ((sa=mxGetField(pa,idx,opt)) != NULL) ) {
            if ( !mxIsDouble(sa) || mxGetNumberOfElements(sa) != 1 ) {
                result = "error reading solver option MinStep";
                goto EXIT_POINT;
            }
            ssSetMinStepSize(S, mxGetPr(sa)[0]);
        }
    }
    
    /* MaxConsecutiveMinStep */
    {
        const char* opt = "MaxConsecutiveMinStep";
        int         maxConsecutiveMinStep;
        if ( (ssIsVariableStepSolver(S)) &&
             ((sa=mxGetField(pa,idx,opt)) != NULL) ) {
            if ( !mxIsDouble(sa) || mxGetNumberOfElements(sa) != 1 ) {
                result = "error reading solver option MaxConsecutiveMinStep";
                goto EXIT_POINT;
            }
            maxConsecutiveMinStep = (int) (mxGetPr(sa)[0]);
            ssSetSolverMaxConsecutiveMinStep(S, maxConsecutiveMinStep);
        }
    }

    /* InitialStep */
    {
        const char* opt = "InitialStep";
        if ( (ssIsVariableStepSolver(S)) &&
             ((sa=mxGetField(pa,idx,opt)) != NULL) ) {
            if ( !mxIsDouble(sa) || mxGetNumberOfElements(sa) != 1 ) {
                result = "error reading solver option InitialStep";
                goto EXIT_POINT;
            }
            ssSetStepSize(S, mxGetPr(sa)[0]);
        }
    }

    /* MaxOrder */ /*Used only by ode15s*/
    {
        const char* opt = "MaxOrder";
        int         maxOrder;
        if ( (strcmpi(ssGetSolverName(S), "ode15s") == 0) &&
             ((sa=mxGetField(pa,idx,opt)) != NULL) ) {
            if ( !mxIsDouble(sa) || mxGetNumberOfElements(sa) != 1 ) {
                result = "error reading solver option MaxOrder";
                goto EXIT_POINT;
            }
            maxOrder = (int) (mxGetPr(sa)[0]);
            ssSetSolverMaxOrder(S, maxOrder);
        }
    }

     /* ConsecutiveZCsStepRelTol */
    {
        const char* opt = "ConsecutiveZCsStepRelTol";
        double      consecutiveZCsStepRelTol;
        if ( (ssIsVariableStepSolver(S)) &&
             ((sa=mxGetField(pa,idx,opt)) != NULL) ) {
            if ( !mxIsDouble(sa) || mxGetNumberOfElements(sa) != 1 ) {
                result = "error reading solver option ConsecutiveZCsStepRelTol";
                goto EXIT_POINT;
            }
            consecutiveZCsStepRelTol = (mxGetPr(sa)[0]);
            ssSetSolverConsecutiveZCsStepRelTol(S, consecutiveZCsStepRelTol);    
        }
    }
 
     /* MaxConsecutiveZCs */
    {
        const char* opt = "MaxConsecutiveZCs";
        int         maxConsecutiveZCs;
        if ( (ssIsVariableStepSolver(S)) &&
             ((sa=mxGetField(pa,idx,opt)) != NULL) ) {
            if ( !mxIsDouble(sa) || mxGetNumberOfElements(sa) != 1 ) {
                result = "error reading solver option MaxConsecutiveZCs";
                goto EXIT_POINT;
            }
            maxConsecutiveZCs = (int) (mxGetPr(sa)[0]);
            ssSetSolverMaxConsecutiveZCs(S, maxConsecutiveZCs);
        }
    }


    /* ExtrapolationOrder -- used by ODE14x, only */
    {
        const char* opt = "ExtrapolationOrder";
        int         extrapolationOrder;
        if ( (strcmpi(ssGetSolverName(S), "ode14x") == 0) &&
             ((sa=mxGetField(pa,idx,opt)) != NULL) ) {
            if ( !mxIsDouble(sa) || mxGetNumberOfElements(sa) != 1 ) {
                result = "error reading solver option ExtrapolationOrder";
                goto EXIT_POINT;
            }
            extrapolationOrder = (int) (mxGetPr(sa)[0]);
            ssSetSolverExtrapolationOrder(S, extrapolationOrder);
        }
    }

    /* NumberNewtonIterations -- used by ODE14x, only */
    {
        const char* opt = "NumberNewtonIterations";
        int         numberIterations;
        if ( (strcmpi(ssGetSolverName(S), "ode14x") == 0) &&
             ((sa=mxGetField(pa,idx,opt)) != NULL) ) {
            if ( !mxIsDouble(sa) || mxGetNumberOfElements(sa) != 1 ) {
                result = "error reading solver option NumberNewtonIterations";
                goto EXIT_POINT;
            }
            numberIterations = (int) (mxGetPr(sa)[0]);
            ssSetSolverNumberNewtonIterations(S, numberIterations);
        }
    }

    /* SolverResetMethod: {'Fast'} | 'Robust' - used by ODE15s, ODE23t, ODE23tb */
    {
        const char* opt = "SolverResetMethod";
	const char* robustMethod = "Robust";
        static char resetMethod[256] = "\0";
        bool        isRobustResetMethod = false;
        if ( ((strcmpi(ssGetSolverName(S), "ode15s") == 0) ||
              (strcmpi(ssGetSolverName(S), "ode23t") == 0) ||
              (strcmpi(ssGetSolverName(S), "ode23tb") == 0)) &&
             ((sa=mxGetField(pa,idx,opt)) != NULL) ) {
            if (mxGetString(sa, resetMethod, 256) != 0) {
                result = "error reading solver option SolverResetMethod";
                goto EXIT_POINT;
            }
	    isRobustResetMethod = (strcmp(resetMethod,robustMethod) == 0);
	    ssSetSolverRobustResetMethod(S,isRobustResetMethod);
        }
    }


    /* MaxNumMinSteps */
    {
        const char* opt = "MaxNumMinSteps";
        int         maxNumMinSteps;
        if ( (ssIsVariableStepSolver(S)) &&
             ((sa=mxGetField(pa,idx,opt)) != NULL) ) {
            if ( !mxIsDouble(sa) || mxGetNumberOfElements(sa) != 1 ) {
                result = "error reading solver option MaxNumMinSteps";
                goto EXIT_POINT;
            }
            maxNumMinSteps = (int) (mxGetPr(sa)[0]);
            ssSetMaxNumMinSteps(S, maxNumMinSteps);
        }
    }
    
    /* ZcDetectionTol */
    /*
    {
        const char* opt = "ZcDetectionTol";
        if ( (ssIsVariableStepSolver(S)) &&
             ((sa=mxGetField(pa,idx,opt)) != NULL) ) {
            if ( !mxIsDouble(sa) || mxGetNumberOfElements(sa) != 1 ) {
                result = "error reading solver option ZcDetectionTol";
                goto EXIT_POINT;
            }
            ssSetSolverZcDetectionTol(S, mxGetPr(sa)[0]);
        }
    }
    */
    
    /* OutputTimes */
    {
        const char* opt = "OutputTimes";
        double* outputTimes = NULL;
        if ( (ssIsVariableStepSolver(S)) &&
             ((sa=mxGetField(pa,idx,opt)) != NULL) ) {
            if ( !mxIsDouble(sa)) {
                result = "error reading solver option OutputTimes";
                goto EXIT_POINT;
            }
            ssSetNumOutputTimes(S, mxGetNumberOfElements(sa));
            outputTimes = 
                  (double*) calloc(mxGetNumberOfElements(sa), sizeof(double));
            (void) memcpy(outputTimes, mxGetPr(sa),
                          mxGetNumberOfElements(sa)*sizeof(double));
            ssSetOutputTimes(S, outputTimes);

        }
    }
    
    /* OutputTimesOnly */
    {
        const char* opt = "OutputTimesOnly";
        if ( (ssIsVariableStepSolver(S)) &&
             ((sa=mxGetField(pa,idx,opt)) != NULL) ) {
            if ( !mxIsLogicalScalar(sa)) {
                result = "error reading solver option OutputTimesOnly";
                goto EXIT_POINT;
            }
            ssSetOutputTimesOnly(S, (int_T)mxIsLogicalScalarTrue(sa));
        }
    }
    
    /* StartTime */
    {
        const char* opt = "StartTime";
        if ( (sa=mxGetField(pa,idx,opt)) != NULL ) {
            if ( !mxIsDouble(sa) || mxGetNumberOfElements(sa) != 1 ) {
                result = "error reading solver option StartTime";
                goto EXIT_POINT;
            }
            ssSetTStart(S, mxGetPr(sa)[0]);
        }
    }

    /* StopTime */
    {
        const char* opt = "StopTime";
        if ( (sa=mxGetField(pa,idx,opt)) != NULL ) {
            if ( !mxIsDouble(sa) || mxGetNumberOfElements(sa) != 1 ) {
                result = "error reading solver option StopTime";
                goto EXIT_POINT;
            }
            ssSetTFinal(S, mxGetPr(sa)[0]);
        }
    }

    /* StateTimeName */
    {
        const char* opt = "TimeSaveName";
        static char timeSaveName[mxMAXNAM] = "\0";
        if ( (sa=mxGetField(pa,idx,opt)) != NULL ) {
            if (mxGetString(sa, timeSaveName, mxMAXNAM) != 0) {
                result = "error reading logging option TimeSaveName";
                goto EXIT_POINT;
            }
            rtliSetLogT(ssGetRTWLogInfo(S), timeSaveName);
        }
    }

    /* StateSaveName */
    if (rtliGetLogX(li)[0] != '\0')
    {
        const char* opt2 = "StateSaveName";
        static char stateSaveName[mxMAXNAM] = "\0";
        
        if ( (sa=mxGetField(pa,idx,opt2)) != NULL ) {
            if (mxGetString(sa, stateSaveName, mxMAXNAM) != 0) {
                result = "error reading logging option StateSaveName";
                goto EXIT_POINT;
            }
            rtliSetLogX(ssGetRTWLogInfo(S), stateSaveName);
        }
    }

    /* OutputSaveName */
    if ( rtliGetLogY(li)[0] != '\0' )
    {
        const char* opt2 = "OutputSaveName";
        static char outputSaveName[mxMAXNAM] = "\0";

        if ( (sa=mxGetField(pa,idx,opt2)) != NULL ) {
            if (mxGetString(sa, outputSaveName, mxMAXNAM) != 0) {
                result = "error reading logging option OutputSaveName";
                goto EXIT_POINT;
            }
            rtliSetLogY(ssGetRTWLogInfo(S), outputSaveName);
        }
    }

    /* FinalStateName */
    if (rtliGetLogXFinal(li)[0] != '\0' )
    {
        const char* opt2 = "FinalStateName";
        static char finalStateName[mxMAXNAM] = "\0";
        
        if ( (sa=mxGetField(pa,idx,opt2)) != NULL ) {
            if (mxGetString(sa, finalStateName, mxMAXNAM) != 0) {
                result = "error reading logging option FinalStateName";
                goto EXIT_POINT;
            }
            rtliSetLogXFinal(ssGetRTWLogInfo(S), finalStateName);
        }
    }
    
    /* SaveFormat */
    {
        const char* opt = "SaveFormat";
        static char saveFormat[256] = "\0";
        int_T saveFormatOpt = 0;
        if ( (sa=mxGetField(pa,idx,opt)) != NULL ) {
            if (mxGetString(sa, saveFormat, 256) != 0) {
                result = "error reading logging option SaveFormat";
                goto EXIT_POINT;
            }
            if (strcmp(saveFormat,"StructureWithTime")==0) {
                saveFormatOpt = 2;
            } else if (strcmp(saveFormat,"Structure")==0) {
                saveFormatOpt = 1;
            } else if (strcmp(saveFormat,"Matrix")==0 ||
                       strcmp(saveFormat,"Array")==0) {
                saveFormatOpt = 0;
            }
            rtliSetLogFormat(ssGetRTWLogInfo(S), saveFormatOpt);
        }
    }

    /* Decimation */
    {
        const char* opt = "Decimation";
        if ( (sa=mxGetField(pa,idx,opt)) != NULL ) {
            if ( !mxIsIntVector(sa) || mxGetNumberOfElements(sa) != 1 ) {
                result = "error reading logging option Decimation";
                goto EXIT_POINT;
            }
            rtliSetLogDecimation(ssGetRTWLogInfo(S), (int_T)mxGetPr(sa)[0]);
        }
    }

    /* MaxDataPoints */
    {
        const char* opt = "MaxDataPoints";
        if ( (sa=mxGetField(pa,idx,opt)) != NULL ) {
            if ( !mxIsIntVector(sa) || mxGetNumberOfElements(sa) != 1 ) {
                result = "error reading logging option MaxDataPoints";
                goto EXIT_POINT;
            }
            rtliSetLogMaxRows(ssGetRTWLogInfo(S), (int_T)mxGetPr(sa)[0]);
        }
    }

#ifdef RACCEL_ENABLE_PARALLEL_EXECUTION
    /* Parallel Execution enabled flag */
    {
        const char* opt = "ParallelExecutionInRapidAccelerator";
        boolean_T val = true;
        if ( (sa=mxGetField(pa,idx,opt)) != NULL ) {
            if ( !mxIsIntVector(sa) || mxGetNumberOfElements(sa) != 1 ) {
                result = "error reading flag ParallelExecutionInRapidAccelerator";
                goto EXIT_POINT;
            }
            val = (boolean_T)mxGetPr(sa)[0];
            if (val) {
                parallelExecutionOptions.parallelExecutionMode =  
                    PARALLEL_EXECUTION_ON;
            } else {
                parallelExecutionOptions.parallelExecutionMode =  
                    PARALLEL_EXECUTION_OFF;
            }
        }
        
    }
    
    /* Parallel Execution profiling flag */
    {
        const char* opt = "ParallelExecutionProfiling";
        if ( (sa=mxGetField(pa,idx,opt)) != NULL ) {
            if ( !mxIsIntVector(sa) || mxGetNumberOfElements(sa) != 1 ) {
                result = "error reading flag ParallelExecutionProfiling";
                goto EXIT_POINT;
            }

            parallelExecutionOptions.enableTiming = (boolean_T)mxGetPr(sa)[0];
        }
        
    }

    /* Parallel Execution  profiling result file name */
    {
        const char* opt = "ParallelExecutionProfilingOutputFilename";
        static char pExecProfOutFilename[1024] = "\0";
        if ( (sa=mxGetField(pa,idx,opt)) != NULL ) {
            if (mxGetString(sa,pExecProfOutFilename,1024) != 0) {
                result = "error reading option ParallelExecutionProfilingOutputFilename";
                goto EXIT_POINT;
            }            
            parallelExecutionOptions.timingOutputFilename = 
                pExecProfOutFilename;
        } 
    }

    /* Parallel Execution node execution modes file name */
    {
        const char* opt = "ParallelExecutionNodeExecutionModesFilename";
        static char pExecNodeExecModesFilename[1024] = "\0";
        if ( (sa=mxGetField(pa,idx,opt)) != NULL ) {
            if (mxGetString(sa,pExecNodeExecModesFilename,1024) != 0) {
                result = "error reading option ParallelExecutionProfilingOutputFilename";
                goto EXIT_POINT;
            }            
            parallelExecutionOptions.nodeExecutionModesFilename = 
                pExecNodeExecModesFilename;
        } 
    }

    /* Parallel Execution Number Of Steps To Analysis */
    {
        const char* opt = "ProfilingBasedParallelExecution";
        if ( (sa=mxGetField(pa,idx,opt)) != NULL ) {
            if ( !mxIsDouble(sa) || mxGetNumberOfElements(sa) != 1 ) {
                result = "error reading solver option ProfilingBasedParallelExecution";
                goto EXIT_POINT;
            }
            parallelExecutionOptions.numberOfStepsToAnalyze = 
                (int)mxGetPr(sa)[0];
        }
    }

#endif


  EXIT_POINT:
    if(pa!=NULL) { 
        mxDestroyArray(pa); 
        pa = NULL; 
    } 
         
    if(pToFile !=NULL){
        mxDestroyArray(pToFile); 
        pToFile = NULL; 
    }
    
    if(pFromFile !=NULL){
        mxDestroyArray(pFromFile); 
        pFromFile = NULL;
    }
    
    if (matf != NULL) {
        matClose(matf); matf = NULL;
    }  

    ssSetErrorStatus(S, result);
    return;

} /* rsimLoadSolverOpts */


/* EOF raccel_mat.c */
