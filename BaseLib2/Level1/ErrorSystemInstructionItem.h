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
 * Holder for the information regarding the caller (TID, ...)
 */ 
#if !defined (ERROR_SYSTEM_INSTR_ITEM)
#define ERROR_SYSTEM_INSTR_ITEM

#include "System.h"
#include "LinkedListable.h"

#if defined (_LINUX) || (_RTAI) || (_VXWORKS) || (_MACOSX)
#define ANY_VALUE              (intptr) NULL
#else
#define ANY_VALUE              NULL
#endif
#define NOT_FOUND              0xffffffff

class ObjectRegistryItem;


/** what to do in case of error */
class ErrorSystemInstructionItem: public LinkedListable {

    /** what to do with this error */
    uint32                  errorAction;

    /** the object the action is restricted to */
    void                   *object;

    /** the class the action is restricted to */
    const char             *className;

    /** the thread the action is restricted to */
    TID                     tid;

    /** the error code the action is restricted to */
    int32                   errorCode;
public:

    /** */
    ErrorSystemInstructionItem(uint32 errorAction,void *object,const char *className,TID tid,uint32 errorCode){
        this->errorAction = errorAction;
        this->errorCode   = errorCode;
        this->object      = object;
        this->tid         = tid;
        this->className   = className;
        printf("className = %s\n", className);
    }

    /** what to do with this error */
    uint32              ErrorAction(){ return errorAction; }

    /** the object the action is restricted to */
    void                *Object(){ return object; }

    /** the class the action is restricted to */
    const char          *ClassName() {
         if (className==NULL) return "";
         return className;
    }

    /** the thread the action is restricted to */
    TID                 Tid(){ return tid; };

    /** the error code the action is restricted to */
    int32               ErrorCode() { return errorCode; }
};

/** */
#define OBJECT_ERRORINSTRUCTION(errorAction,object,code) new ErrorSystemInstructionItem(errorAction,object,ANY_VALUE,ANY_VALUE,code)

/** */
#define ERRCODE_ERRORINSTRUCTION(errorAction,code) new ErrorSystemInstructionItem(errorAction,ANY_VALUE,ANY_VALUE,ANY_VALUE,code)

/** */
#define CLASS_ERRORINSTRUCTION(errorAction,classInfo,code) new ErrorSystemInstructionItem(errorAction,ANY_VALUE,classInfo,ANY_VALUE,code)

/** */
#define TID_ERRORINSTRUCTION(errorAction,tid,code) new ErrorSystemInstructionItem(errorAction,ANY_VALUE,ANY_VALUE,tid,code)

#endif

