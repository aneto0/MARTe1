/*
 * Copyright 2011 EFDA | European Fusion Development Agreement
 *
 * Licensed under the EUPL, Version 1.1 or - as soon they 
   will be approved by the European Commission - subsequent  
   versions of the EUPL (the "Licence"); 
 * You may not use this work except in compliance with the 
   Licence. 
 * You may obtain a copy of the Licence at: 
 *  
 * http://ec.europa.eu/idabc/eupl
 *
 * Unless required by applicable law or agreed to in 
   writing, software distributed under the Licence is 
   distributed on an "AS IS" basis, 
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either 
   express or implied. 
 * See the Licence for the specific language governing 
   permissions and limitations under the Licence. 
 *
 * $Id$
 *
**/

/** 
 * @file
 * @brief Encode/decode system logging
 */
#if !defined (ERROR_SYSTEM_INFO)
#define ERROR_SYSTEM_INFO

#include "System.h"
#include "ObjectRegistryItem.h"
#include "FString.h"

class ObjectRegistryItem;
class ErrorSystemInfo;

extern "C" {

    /** */
    void ESIInit(ErrorSystemInfo &esi);

    /** */
    void ESILoad(ErrorSystemInfo &esi,StreamInterface &in);

    /** */
    void ESISend(ErrorSystemInfo &esi,StreamInterface &out);

    /** */
    void ESIFromBuffer(ErrorSystemInfo &esi,char *buffer,uint32 size);

    /** */
    void ESIComposeToText(ErrorSystemInfo &esi,StreamInterface &out);

}


/** The header of the message sent to a remote logger */
class ErrorSystemInfo:public Queueable {
    friend void ESIInit(ErrorSystemInfo &esi);
    friend void ESILoad(ErrorSystemInfo &esi,StreamInterface &in);
    friend void ESISend(ErrorSystemInfo &esi,StreamInterface &out);
    friend void ESIFromBuffer(ErrorSystemInfo &esi,char *buffer,uint32 size);
    friend void ESIComposeToText(ErrorSystemInfo &esi,StreamInterface &out);

//public:
    /** what to do with this error */
    uint32                  errorAction;

    /** */
    int32                   errorCode;

    /** */
    uint32                  errorTime;

    /** the process the error originated from 0 means NONE */
    uint32                  pid;

    /** the thread the error originated from */
    uint32                  tid;

    /** the object the error originated from 0 means NONE */
    void                   *object;

    /** the class the error originated from 0 means NONE */
    FString                 className;

    /** the original error message */
    FString                 errorMessage;

    /** */
    FString                 threadName;

public:
    /** */
    ErrorSystemInfo(){
        ESIInit(*this);
    }


    /** load a error informations structure from a Stream. 0 is the terminator */
    void Load(StreamInterface &in){
        ESILoad(*this,in);
    }

    /** compose an error message as text */
    void ComposeToText(StreamInterface &out){
        ESIComposeToText(*this,out);
    }

    /** compose an error message as text */
    void Send(StreamInterface &out){
        ESISend(*this,out);
    }

    /** reverse of Send but still in old style */
    void FromBuffer(char *buffer,uint32 size){
        ESIFromBuffer(*this,buffer,size);
    }

    /** */
    bool IsEmpty()      {
        return (errorMessage.Size()==0);
    }

    /** */
    void Clear()        {
        errorMessage = "";
    }

    /** */
    uint32 ErrorAction(){
        return errorAction;
    }

    /** */
    void   *Object(){
        return object;
    }

    /** */
    const char  *ClassName() {
        return className.Buffer();
    }

    /** */
    const char  *ThreadName() {
        return threadName.Buffer();
    }

    /** */
    uint32 Tid(){
        return tid;
    }

    /** */
    uint32 Pid(){
        return pid;
    }

    /** */
    int32  ErrorCode() {
        return errorCode;
    }

    /** */
    uint32 ErrorTime() {
        return errorTime;
    }

    /** */
    const char  *ErrorMessage() {
        return errorMessage.Buffer();
    }

};

#endif

