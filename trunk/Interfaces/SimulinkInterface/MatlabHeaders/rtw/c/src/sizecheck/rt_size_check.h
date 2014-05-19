/* Copyright 2009-2011 The MathWorks, Inc. */
#ifndef rt_size_check_h
#define rt_size_check_h

#include "simstruc.h"

#ifdef __cplusplus
extern "C" {
#endif

void
rtErrorMsgID(SimStruct*  S, const char* msgID);

void
rtSizeEqCheck1D(SimStruct*  S, int  dims1, int dims2);

void 
rtSizeEqCheckND(SimStruct*  S, int* dims1, int* dims2, int nDims) ;

#ifdef __cplusplus
}
#endif

#endif
