/* Copyright 2010 The MathWorks, Inc.
 *
 * File    : version.h
 * Abstract:
 *      Required include file for custom external mode
 */

#ifndef __VERSION_H__
#define __VERSION_H__


#ifdef SL_EXT_SO
#ifndef SL_EXT
#define SL_EXT ".so"
#endif
#endif

#ifdef SL_EXT_DLL
#ifndef SL_EXT
#define SL_EXT ".dll"
#endif
#endif

#ifdef SL_EXT_DYLIB
#ifndef SL_EXT
#define SL_EXT ".dylib"
#endif
#endif

#ifndef MIN
#define MIN(A, B)   (((A) < (B)) ? (A) : (B))
#endif

#endif /* __VERSION_H__ */
