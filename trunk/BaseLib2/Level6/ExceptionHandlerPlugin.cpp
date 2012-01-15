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

#include "ExceptionHandlerPlugin.h" 
#include "ThreadInitialisationInterface.h"
#include "LinkedListHolder.h"
#include "ErrorManagement.h"
#include "ExceptionHandlerInterface.h"
#include "ExceptionInformation.h"


// if EHCount is set to 99 before the increment then the whole task will be terminated
#define EHCOUNT_TERMINATE_PROCESS 100
// if EHCount is set to 99 before the increment then the whole task will be terminated
#define EHCOUNT_TERMINATE_THREAD    2
//
#define EHCOUNT_HANDLER_INSTALLED   1
//
#define EHCOUNT_START_VALUE         0

#if defined (_WIN32)

// the standard eh function
EXCEPTION_DISPOSITION __cdecl StdEhFunction (
    struct _EXCEPTION_RECORD *ExceptionRecord,
    void                     *EstablisherFrame,
    struct _CONTEXT          *ContextRecord,
    void                     *DispatcherContext );


// install standard basic handler
#define EH_INSTALL_BASIC_HANDLER(function,tip)                                 \
uint32 exceptionHandlerFun = (uint32)function;                                 \
__asm {push    exceptionHandlerFun }/* to install the exception handler */     \
__asm {push    FS:[0]           }/* push new address and old address */        \
__asm {mov     FS:[0],ESP       }/* move pointer to new location */            \
tip->EHCount = EHCOUNT_START_VALUE; /* installation of return address */       \
__asm {mov     eax,0            }/* Zero out EAX */                            \
__asm {add     [eax], 1         }/* create controlled exception */


#define EH_UNINSTALL_HANDLER                                                   \
__asm {mov     eax,[ESP]        }                                              \
__asm {mov     FS:[0], EAX      }                                              \
__asm {add     esp, 8           }



/// information passed to the ThreadInitialisationFunction
class ExceptionHandler_TII : public ThreadInitialisationInterface{
protected:

    ///
    uint32                  exceptionAction;

    ///  takes one of the EHCOUNT_ values
    int32                   EHCount;

    /// contains a list of exception handlers
    LinkedListHolder        exceptionHandlers;

public:

    //
    ExceptionHandler_TII(
        ThreadFunctionType      userThreadFunction,
        void                    *userData,
        const char              *name                   = NULL,
        uint32                  exceptionAction         = XH_NotHandled
        ){
        this->userThreadFunction = userThreadFunction;
        this->userData           = userData;
        this->exceptionAction    = exceptionAction;
        if (name != NULL){
            this->name = strdup(name);
        } else {
            this->name = strdup("Unknown");
        }

        EHCount            = EHCOUNT_START_VALUE;

    }

    /** Normal class destructor. It just frees the memory allocated for the name string. */
    virtual ~ExceptionHandler_TII(){
        free((void *&)name);
    };

    /** The function representing the thread. */
    virtual void UserThreadFunction(){
        if (userThreadFunction == NULL) {
            CStaticAssertErrorCondition(ParametersError,"ExceptionHandlerPlugin::UserThreadFunction = NULL");
            return;
        }

        // create basic handler class
        ExceptionHandlerInterface *eh = new ExceptionHandlerInterface(exceptionAction);

        // adds it to the list of handlers
        exceptionHandlers.ListInsert(eh);

        // install std exception handler function for the current thread
        // and create exception so that to record return location
        EH_INSTALL_BASIC_HANDLER(StdEhFunction,this)

        // the eh fixes the problem by pointing eax to EHCount which then becomes 1
        // next time an exception is called somewhere else the context will be reinstated and
        // the execution will continue here but EHCount will not be 1 anymore
        if (EHCount == EHCOUNT_HANDLER_INSTALLED) {
            // the user function
                userThreadFunction(userData);
        }

        if (EHCount == EHCOUNT_TERMINATE_PROCESS) {
            exit(-1);
        } else
        if ((EHCount != EHCOUNT_HANDLER_INSTALLED) &&
            (EHCount != EHCOUNT_TERMINATE_THREAD )){
            CStaticAssertErrorCondition(Information,"expecting EHCount = %i or %i not %i",EHCOUNT_HANDLER_INSTALLED,EHCOUNT_TERMINATE_THREAD,EHCount);
        }

        // remove exception handler function
        EH_UNINSTALL_HANDLER

        // remove any exception handler
        while ((eh = (ExceptionHandlerInterface *)exceptionHandlers.ListExtract())!=NULL){
            // and destroys it!
            delete eh;
        }

        // the user function
        userThreadFunction(userData);

    }

    /** */
    virtual bool ExceptionProtectedExecute(ThreadFunctionType userFunction,void *userData, ExceptionHandlerInterface *eh){
        exceptionHandlers.ListInsert(eh);
        EHCount = EHCOUNT_START_VALUE; /* installation of return address */
        __asm {mov     eax,0            }/* Zero out EAX */
        __asm {add     [eax], 1         }/* create controlled exception */

        bool firstAttempt = (EHCount == EHCOUNT_HANDLER_INSTALLED);
            userFunction(userData);

        exceptionHandlers.ListExtract(eh);

        return firstAttempt;
    }

    /* The actual exception handler */
    virtual EXCEPTION_DISPOSITION __cdecl HandleException (
        struct _EXCEPTION_RECORD *exceptionRecord,
        void                     *establisherFrame,
        struct _CONTEXT          *contextRecord,
        void                     *dispatcherContext ){


        ExceptionHandlerInterface *eh = (ExceptionHandlerInterface *)exceptionHandlers.List();
        if (eh == NULL) {
            CStaticAssertErrorCondition(Warning,"Exception:no handler object in list!");
            return ExceptionContinueSearch;
        }

        // check whether this is the first call
        if (EHCount == 0){
            if (exceptionRecord->ExceptionFlags & CONTEXT_EXTENDED_REGISTERS){
                // SAVE RETURN POINT
               eh->context = *contextRecord;
            } else {
                char *src = (char *)contextRecord;
                char *dst = (char *)&eh->context;
                int size = sizeof(_CONTEXT)-MAXIMUM_SUPPORTED_EXTENSION;
                int i;
                for (i=0;i<size;i++) dst[i] = src[i];
            }
        } else {
            // prepare ExceptionInformation
            ExceptionInformation ei(exceptionRecord,contextRecord);

            uint32 answer = eh->Catch(ei);
            // check if other installed handlers here!
            while ((eh != NULL)&&((answer & 0xFFFF)==XH_TryOther)){
                eh = (ExceptionHandlerInterface *)eh->Next();
                answer = eh->Catch(ei);
            }
            if ((answer & XH_NoReport)==0){
                // depending on return flags report ExceptionInformation
                CStaticAssertErrorCondition(Exception,"Exception: \n%s",ei.infoStr.Buffer());
            }
            // depending on results return to OS with different flags
            switch(answer &0xFFFF){
                case XH_TryOther:
                case XH_NotHandled:
                    return ExceptionContinueSearch;
                break;
                case XH_ContinueExec:
                    return ExceptionContinueExecution;
                break;
                case XH_KillThread:
                    // -1 because will be incremented by the code at the return point!
                    EHCount = EHCOUNT_TERMINATE_THREAD-1;
                break;
                case XH_KillTask:
                    // -1 because will be incremented by the code at the return point!
                    EHCount = EHCOUNT_TERMINATE_PROCESS-1;
                break;
                case XH_TryAgain:
                    // -1 because will be incremented by the code at the return point!
                    EHCount = EHCOUNT_HANDLER_INSTALLED-1;
                break;
                default:
                    EHCount = EHCOUNT_TERMINATE_PROCESS-1;
                break;
            }

            // RESTORE RETURN POINT
            if (exceptionRecord->ExceptionFlags & CONTEXT_EXTENDED_REGISTERS){
                 *contextRecord = eh->context;
            } else {
                char *dst = (char *)contextRecord;
                char *src = (char *)&eh->context;
                int size = sizeof(_CONTEXT)-MAXIMUM_SUPPORTED_EXTENSION;
                int i;
                for (i=0;i<size;i++) dst[i] = src[i];
            }
        }

        // Change EAX in the context record so that it points to someplace
        // where we can successfully write
        contextRecord->Eax = (DWORD)&EHCount;

        return ExceptionContinueExecution;
    }
};

EXCEPTION_DISPOSITION __cdecl StdEhFunction (
    struct _EXCEPTION_RECORD *exceptionRecord,
    void                     *establisherFrame,
    struct _CONTEXT          *contextRecord,
    void                     *dispatcherContext ){

    if (exceptionRecord->ExceptionFlags != 0) {
        CStaticAssertErrorCondition(Information,"Exception Flags = %x",exceptionRecord->ExceptionFlags);
        return ExceptionContinueSearch;
    }

    ExceptionHandler_TII *tii = (ExceptionHandler_TII *)ThreadsGetInitialisationInterface();
    if (tii == NULL){
        CStaticAssertErrorCondition(FatalError,"Exception:no ExceptionHandler_TII in TLS!");
        return ExceptionContinueSearch;
    }

    return tii->HandleException (
                    exceptionRecord,
                    establisherFrame,
                    contextRecord,
                    dispatcherContext
                );
}


// Forward declaration.
/** This is the default TII object instantiator eventually used in the BeginThread method.
    @param userThreadFunction The thread entry point.
    @param userData A pointer to data that can be passed to the thread.
    @param threadName The thread name.
    @param exceptionHandlerBehaviour Describes the behaviour of threads when an exception occurr.
*/
ThreadInitialisationInterface * ExceptionHandler_TIIC(
    ThreadFunctionType          userThreadFunction,
    void                        *userData,
    const char                  *threadName,
    ExceptionHandlerBehaviour   exceptionHandlerBehaviour){

    return new ExceptionHandler_TII(
                   userThreadFunction,
                   userData,
                   threadName,
                   exceptionHandlerBehaviour
                );
}

#endif

void ExceptionHandlerInstallTIIC(){
#if defined(_WIN32)
    ThreadsSetInitialisationInterfaceConstructor( ExceptionHandler_TIIC);
#else
    CStaticAssertErrorCondition(IllegalOperation,"ExceptionHandler not supported in this platform");
#endif
}





