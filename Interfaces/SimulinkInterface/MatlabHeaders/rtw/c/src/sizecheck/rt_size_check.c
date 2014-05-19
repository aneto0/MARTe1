/* Copyright 2009-2010 The MathWorks, Inc. */
#include "rt_size_check.h"
#include <stdio.h>

const char sizeErrorMsg[]="Invalid runtime dimensions found. Run the model in normal mode simulation for specific information about the invalid input dimensions.";
const char genericErrorMsg[]="A run-time error occurred. Run the model in normal mode simulation for specific information about the error.";

void
rtErrorMsgID(SimStruct*  S, const char* msgID)
{
    /* Ignoring the msgID. */
    ssSetErrorStatus(S, genericErrorMsg);
}

void
rtSizeEqCheck1D(SimStruct*  S, int  dims1, int dims2)
{
 if (dims1 != dims2) {
        ssSetErrorStatus(S, sizeErrorMsg);
        return;  /* FAIL */
    }
    return; /* SUCCESS */
}

void 
rtSizeEqCheckND(SimStruct*  S, int* dims1, int* dims2, int  nDims) 
{
    int i;
    bool hasError = false;
    /* Check signal dimensions match */
    if (nDims == 2){
        /* Case nx1 to n is no mismatch */
        if( (dims1[0]*dims1[1]) != (dims2[0]*dims2[1]) ){
            hasError = true;
        }else{
            if( (dims1[0]==dims2[0]) || (dims1[0]==dims2[1])){
                hasError = false; 
            }
            else{
                hasError = true;
            }
        }

    }
    else{
        for(i = 0; i < nDims; i++) {
            if(dims1[i] != dims2[i]) {
                hasError = true;
            }
        }
    }

    if(hasError){
        ssSetErrorStatus(S, sizeErrorMsg);
    }
    return;
}
