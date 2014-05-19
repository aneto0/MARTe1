/* $Revision: 1.1.6.5 $
 *
 * Copyright 1994-2004 The MathWorks, Inc.
 *
 * File: rt_logging_mmi.h
 *
 * Abstract:
 */

#ifndef rt_logging_mmi_h
#define rt_logging__mmi_h

#include <stdlib.h>
#include "rtwtypes.h"
#include "rtw_matlogging.h"
#include "rtw_modelmap.h"

#ifdef __cplusplus
extern "C" {
#endif

const char_T * rt_FillStateSigInfoFromMMI(RTWLogInfo   *li,
                                          const char_T **errStatus);

void rt_CleanUpForStateLogWithMMI(RTWLogInfo *li);

const char_T * rt_FillSigLogInfoFromMMI(RTWLogInfo    *li,
                                        const char_T  **errStatus);

void rt_CleanUpForSigLogWithMMI(RTWLogInfo *li);

#ifdef __cplusplus
}
#endif

#endif /*  rt_logging_h */
