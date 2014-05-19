/* $Revision: 1.1.6.14 $
 *
 * Copyright 1994-2008 The MathWorks, Inc.
 *
 * File: rt_logging_mmi.c
 *
 * Abstract:
 */

#ifndef rt_logging_c
#define rt_logging_c

#include <stdlib.h>
#include <stddef.h>
#include "rtwtypes.h"
#include "rtw_matlogging.h"
#include "rtw_modelmap.h"

static const char_T rtMemAllocError[] = "Memory allocation error";
#define FREE(m) if (m != NULL) free(m)

#define ACCESS_C_API_FOR_RTW_LOGGING true

/* Function: rt_FillStateSigInfoFromMMI =======================================
 * Abstract:
 *
 * Returns:
 *	== NULL  => success.
 *	~= NULL  => failure, the return value is a pointer to the error
 *                           message, which is also set in the simstruct.
 */
const char_T * rt_FillStateSigInfoFromMMI(RTWLogInfo   *li,
                                                 const char_T **errStatus)
{
    int_T                  i;
    int_T                  nSignals     = 0;
    int_T                  *dims        = NULL;
    BuiltInDTypeId         *dTypes      = NULL;
    int_T                  *cSgnls      = NULL;
    char_T                 **labels     = NULL;
    char_T                 **blockNames = NULL;
    char_T                 **stateNames = NULL;
    boolean_T              *crossMdlRef = NULL;
    void                   **sigDataAddr = NULL;
    int_T                  *logDataType = NULL;
    boolean_T              *isVarDims   = NULL;


    const rtwCAPI_ModelMappingInfo *mmi = (const rtwCAPI_ModelMappingInfo *)rtliGetMMI(li);

    int_T                  sigIdx       = 0;

    RTWLogSignalInfo *     sigInfo;
    /* reset error status */
    *errStatus = NULL;

    sigInfo = (RTWLogSignalInfo *)calloc(1,sizeof(RTWLogSignalInfo));
    if (sigInfo == NULL) goto ERROR_EXIT;

    nSignals = rtwCAPI_GetNumStateRecordsForRTWLogging(mmi);

    if (nSignals >0) {
        /* These are all freed before exiting this function */
        dims        = (int_T *)calloc(nSignals,sizeof(int_T));
        if (dims == NULL) goto ERROR_EXIT;
        dTypes      = (BuiltInDTypeId *)calloc(nSignals,sizeof(BuiltInDTypeId));
        if (dTypes == NULL) goto ERROR_EXIT;
        cSgnls      = (int_T *)calloc(nSignals,sizeof(int_T));
        if (cSgnls == NULL) goto ERROR_EXIT;
        labels      = (char_T **)calloc(nSignals, sizeof(char_T*));
        if (labels == NULL) goto ERROR_EXIT;
        blockNames  = (char_T**)calloc(nSignals, sizeof(char_T*));
        if (blockNames == NULL) goto ERROR_EXIT;
        stateNames  = (char_T**)calloc(nSignals, sizeof(char_T*));
        if (stateNames == NULL) goto ERROR_EXIT;
        crossMdlRef  = (boolean_T*)calloc(nSignals, sizeof(boolean_T));
        if (crossMdlRef == NULL) goto ERROR_EXIT;
        logDataType = (int_T *)calloc(nSignals,sizeof(int_T));
        if (logDataType == NULL) goto ERROR_EXIT;
        /* Allocate memeory for isVarDims pointer and set all elements to 0's */
        isVarDims = (boolean_T *)calloc(nSignals,sizeof(boolean_T));
        if (isVarDims == NULL) goto ERROR_EXIT;

        /* This is freed in stopDataLogging (it's needed in the meantime) */
        sigDataAddr = (void **)calloc(nSignals,sizeof(void *));
        if (sigDataAddr == NULL) goto ERROR_EXIT;

        *errStatus = rtwCAPI_GetStateRecordInfo(mmi,
                                                (const char_T**) blockNames,
                                                (const char_T**) labels,
                                                (const char_T**) stateNames,
                                                dims,
                                                (int_T*)dTypes,
                                                logDataType,
                                                cSgnls,
                                                sigDataAddr,
                                                crossMdlRef,
                                                NULL,
                                                NULL, /* sigSampleTime */
                                                &sigIdx,
                                                false, /* crossingModel */
                                                NULL,  /* stateDerivVector */
                                                ACCESS_C_API_FOR_RTW_LOGGING);

        if (*errStatus != NULL) goto ERROR_EXIT;

        rtliSetLogXSignalPtrs(li,(LogSignalPtrsType)sigDataAddr);
    }

    sigInfo->numSignals = nSignals;
    sigInfo->numCols = dims;
    sigInfo->numDims = NULL;
    sigInfo->dims = dims;
    sigInfo->dataTypes = dTypes;
    sigInfo->complexSignals = cSgnls;
    sigInfo->frameData = NULL;
    sigInfo->labels.ptr = labels;
    sigInfo->titles = NULL;
    sigInfo->titleLengths = NULL;
    sigInfo->plotStyles = NULL;
    sigInfo->blockNames.ptr = blockNames;
    sigInfo->stateNames.ptr = stateNames;
    sigInfo->crossMdlRef = crossMdlRef;
    sigInfo->dataTypeConvert = NULL;

    sigInfo->isVarDims = isVarDims;
    sigInfo->currSigDims = NULL;

    rtliSetLogXSignalInfo(li,sigInfo);

    /* Free logDataType it's not needed any more,
     * the rest of them will be freed later */
    FREE(logDataType);
    return(NULL); /* NORMAL_EXIT */

  ERROR_EXIT:
    if (*errStatus == NULL) {
        *errStatus = rtMemAllocError;
    }
    /* Free local stuff that was allocated. It is no longer needed */
    for (i = 0; i < nSignals; ++i) utFree(blockNames[i]);
    FREE(blockNames);
    for (i = 0; i < nSignals; ++i) utFree(stateNames[i]);
    FREE(stateNames);
    FREE(labels);
    FREE(dims);
    FREE(dTypes);
    FREE(logDataType);
    FREE(cSgnls);
    FREE(isVarDims);
    return(*errStatus);

} /* end rt_InitSignalsStruct */

void rt_CleanUpForStateLogWithMMI(RTWLogInfo *li)
{
    int_T i;
    RTWLogSignalInfo *sigInfo = _rtliGetLogXSignalInfo(li); /* get the non-const ptr */
    int_T nSignals = sigInfo->numSignals;

    if ( nSignals > 0 ) {

        for (i = 0; i < nSignals; ++i) utFree(sigInfo->blockNames.ptr[i]);
        FREE(sigInfo->blockNames.ptr);
        FREE(sigInfo->labels.ptr);
        FREE(sigInfo->crossMdlRef);
        FREE(sigInfo->dims);
        FREE(sigInfo->dataTypes);
        FREE(sigInfo->complexSignals);
        FREE(sigInfo->isVarDims);

        FREE(sigInfo);
        rtliSetLogXSignalInfo(li, NULL);

        FREE(_rtliGetLogXSignalPtrs(li)); /* get the non-const ptr */
        rtliSetLogXSignalPtrs(li,NULL);
    }
}


/* Signal Logging Routines */

/* Function: rt_FillSigLogInfoFromMMI =======================================
 * Abstract:
 *
 * Returns:
 *	== NULL  => success.
 *	~= NULL  => failure, the return value is a pointer to the error
 *                           message, which is also set in the simstruct.
 */
const char_T * rt_FillSigLogInfoFromMMI(RTWLogInfo    *li,
                                        const char_T  **errStatus)
{
    int_T                  i;
    int_T                  nSignals      = 0;
    int_T                  *dims         = NULL;
    BuiltInDTypeId         *dTypes       = NULL;
    int_T                  *cSgnls       = NULL;
    char_T                 **labels      = NULL;
    char_T                 **blockNames  = NULL;
    boolean_T              *crossMdlRef  = NULL;
    void                   **sigDataAddr = NULL;
    int_T                  *logDataType  = NULL;
    boolean_T              *isVarDims    = NULL;

    const rtwCAPI_ModelMappingInfo *mmi
        = (const rtwCAPI_ModelMappingInfo *)rtliGetMMI(li);

    int_T                  sigIdx       = 0;

    RTWLogSignalInfo *     sigInfo;
    /* reset error status */
    *errStatus = NULL;

    sigInfo = (RTWLogSignalInfo *)calloc(1,sizeof(RTWLogSignalInfo));
    if (sigInfo == NULL) goto ERROR_EXIT;

    nSignals = rtwCAPI_GetNumSigLogRecordsForRTWLogging(mmi);

    if (nSignals >0) {
        /* These are all freed before exiting this function */
        dims        = (int_T *)calloc(nSignals,sizeof(int_T));
        if (dims == NULL) goto ERROR_EXIT;
        dTypes      = (BuiltInDTypeId *)calloc(nSignals,sizeof(BuiltInDTypeId));
        if (dTypes == NULL) goto ERROR_EXIT;
        cSgnls      = (int_T *)calloc(nSignals,sizeof(int_T));
        if (cSgnls == NULL) goto ERROR_EXIT;
        labels      = (char_T **)calloc(nSignals, sizeof(char_T*));
        if (labels == NULL) goto ERROR_EXIT;
        blockNames  = (char_T**)calloc(nSignals, sizeof(char_T*));
        if (blockNames == NULL) goto ERROR_EXIT;
        crossMdlRef  = (boolean_T*)calloc(nSignals, sizeof(boolean_T));
        if (crossMdlRef == NULL) goto ERROR_EXIT;
        logDataType = (int_T *)calloc(nSignals,sizeof(int_T));
        if (logDataType == NULL) goto ERROR_EXIT;
        /* Allocate memeory for isVarDims pointer and set all elements to 0's */
        isVarDims = (boolean_T *)calloc(nSignals,sizeof(boolean_T));
        if (isVarDims == NULL) goto ERROR_EXIT;

        /* This is freed in stopDataLogging (it's needed in the meantime) */
        sigDataAddr = (void **)calloc(nSignals,sizeof(void *));
        if (sigDataAddr == NULL) goto ERROR_EXIT;

        *errStatus = rtwCAPI_GetSigLogRecordInfo(mmi,
                                                 (const char_T**)blockNames,
                                                 (const char_T**)labels,
                                                 dims,
                                                 (int_T*)dTypes,
                                                 logDataType,
                                                 cSgnls,
                                                 sigDataAddr,
                                                 crossMdlRef,
                                                 &sigIdx,
                                                 false,
                                                 ACCESS_C_API_FOR_RTW_LOGGING);

        if (*errStatus != NULL) goto ERROR_EXIT;

        rtliSetSigLogSignalPtrs(li,(LogSignalPtrsType)sigDataAddr);
    }

    sigInfo->numSignals = nSignals;
    sigInfo->numCols = dims;
    sigInfo->numDims = NULL;
    sigInfo->dims = dims;
    sigInfo->dataTypes = dTypes;
    sigInfo->complexSignals = cSgnls;
    sigInfo->frameData = NULL;
    sigInfo->labels.ptr = labels;
    sigInfo->titles = NULL;
    sigInfo->titleLengths = NULL;
    sigInfo->plotStyles = NULL;
    sigInfo->blockNames.ptr = blockNames;
    sigInfo->crossMdlRef = crossMdlRef;
    sigInfo->dataTypeConvert = NULL;
    sigInfo->isVarDims = isVarDims;
    sigInfo->currSigDims = NULL;
    rtliSetSigLogSignalInfo(li,sigInfo);

    /* Free logDataType it's not needed any more,
     * the rest of them will be freed later */
    FREE(logDataType);
    return(NULL); /* NORMAL_EXIT */

  ERROR_EXIT:
    if (*errStatus == NULL) {
        *errStatus = rtMemAllocError;
    }
    /* Free local stuff that was allocated. It is no longer needed */
    for (i = 0; i < nSignals; ++i) utFree(blockNames[i]);
    FREE(blockNames);
    FREE(labels);

    FREE(dims);
    FREE(dTypes);
    FREE(logDataType);
    FREE(cSgnls);
    FREE(isVarDims);
    return(*errStatus);

} /* end rt_FillSigLogInfoFromMMI */


void rt_CleanUpForSigLogWithMMI(RTWLogInfo *li)
{
    int_T i;
    RTWLogSignalInfo *sigInfo = _rtliGetSigLogSignalInfo(li);
    int_T nSignals = sigInfo->numSignals;

    if ( nSignals > 0 ) {

        for (i = 0; i < nSignals; ++i) utFree(sigInfo->blockNames.ptr[i]);
        FREE(sigInfo->blockNames.ptr);
        FREE(sigInfo->labels.ptr);

        FREE(sigInfo->crossMdlRef);
        FREE(sigInfo->dims);
        FREE(sigInfo->dataTypes);
        FREE(sigInfo->complexSignals);
        FREE(sigInfo->isVarDims);
        FREE(sigInfo);
        rtliSetSigLogSignalInfo(li, NULL);

        FREE(_rtliGetSigLogSignalPtrs(li));
        rtliSetSigLogSignalPtrs(li,NULL);
    }
}



#endif /*  rt_logging_mmi_c */
