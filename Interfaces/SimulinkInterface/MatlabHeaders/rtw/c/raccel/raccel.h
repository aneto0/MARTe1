/*
 * Copyright 2007-2010 The MathWorks, Inc.
 *
 * File: raccel.h
 *
 *  $Revision: 1.1.6.3 $
 *  $Date: 2010/09/24 12:38:33 $
 *
 * Abstract:
 *	Data structures used with the RSIM from file and from workspace block
 *      handling.
 *
 * Requires include files
 *	tmwtypes.h
 *	simstruc_type.h
 * Note including simstruc.h before rsim.h is sufficient because simstruc.h
 * includes both tmwtypes.h and simstruc_types.h.
 */

#ifndef __RACCEL_H__
#define __RACCEL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "rapid_utils.h"

extern void rsimLoadSolverOpts(SimStruct* S);
extern void rt_RAccelReplaceFromFilename(const char *blockpath, char *fileNam);
extern void rt_RAccelReplaceToFilename(const char *blockpath, char *fileNam);

#ifdef __cplusplus
}
#endif

#endif /* __RACCEL_H__ */
