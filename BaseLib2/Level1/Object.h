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
 * The base BaseLib2 object. Provides the infrastructure to create an object
 * by name and to automatically retrieve information regarding the class.
 * It needs that the final class provides a className buffer and overrides
 * both methods Name() and Type()
 */
#if !defined (_OBJECT_H)
#define _OBJECT_H

#include "System.h"
#include "ObjectRegistryItem.h"
#include "StreamInterface.h"
#include "ErrorManagement.h"
#include "ObjectMacros.h"


#if defined(_OS2)
#include "OS2Errors.h"
#endif

class ErrorSystemInstructions;
class Object;
class ConfigurationDataBase;

extern "C" {

    /** used to deallocate memory when calling delete for an Object descendant */
    void OBJDeleteFun(void *p,ObjectRegistryItem  *info);

    /** used to allocate memory when calling new for an Object descendant */
    void *OBJNewFun(unsigned int len,ObjectRegistryItem  *info);


//###########################################################################################
//
//                                        ERROR MANAGEMENT FUNCTIONS
//
//###########################################################################################
    /**
    @name ErrorManagementFunctions
    @{ */

    /** What to do in case of error. Sets the general rule */
    void OBJSetGlobalOnErrorBehaviour(EMFErrorBehaviour behaviour);

    /** Get general rule on error behaviour */
    uint32 OBJGetGlobalErrorAction();

    /** What to do in case of error. Sets the rule for this thread */
    void OBJSetThreadOnErrorBehaviour(EMFErrorBehaviour behaviour,int32 code);

    /** What to do in case of error. Sets the rule for this class */
    void OBJSetClassOnErrorBehaviour(Object &ob,EMFErrorBehaviour behaviour,int32 code);

    /** What to do in case of error. Sets the rule for this object */
    void OBJSetObjectOnErrorBehaviour(Object &ob,EMFErrorBehaviour behaviour,int32 code);

    /** Retrieve the global error-action association table.
    The table maps between error actions and circumstances (object,thread and class)  */
    ErrorSystemInstructions *OBJGetGlobalInstructions();

    /**
    @} */

//###########################################################################################
//
//                                        OTHER FUNCTIONS
//
//###########################################################################################


    /** Enables logging of new and delete of descendents of Object */
    void OBJEnableAllocationMonitoring();

}

OBJECT_DLL(Object)
/** Standard base class providing type information system
 */
class Object {
private:

OBJECT_DLL_STUFF(Object)


public:

    /** needed to enable virtual deleting */
    virtual ~Object(){
    }

    /** Returns the name of the class */
    const char *ClassName() const{
        if (Info() == NULL) return NULL;
        return Info()->ClassName();
    }

    /** Returns the type of the class.
        This identification code is unique within an application */
    uint32 ClassType(){
        if (Info() == NULL) return 0;
        return Info()->ClassType();
    }

    /** an user definable class Id the default is a checksum of the name based on SUM(x) sum(x^2) sum(x^3) sum(x^4)
        This identification code is with an high probability unique among different applications */
    uint32 ClassId(){
        if (Info() == NULL) return 0;
        return Info()->ClassId();
    }

    /** What file extensions are asociated to this class */
    const char *DefaultExtension(){
        if (Info() == NULL) return NULL;
        return Info()->DefaultExtension();
    }

    /** What flags are set for this class */
    const uint32 DefaultFlags(){
        if (Info() == NULL) return 0;
        return Info()->DefaultFlags();
    }

    /** Returns the version of the class */
    const char *Version(){
        if (Info() == NULL) return NULL;
        return Info()->Version();
    }

    /** Standard Object Creation function. Uses a CDB to pass the initialisation parameters.
        The CDB information is read from the subtree that is currently addressed.
        For binary save/loads put the binaryy content in root.binary    */
    virtual bool ObjectLoadSetup(ConfigurationDataBase &cdb,StreamInterface *err){
        AssertErrorCondition(IllegalOperation,"ObjectLoadSetup is not implemented");
        return False;
    }

    /** Standard Object Save function. Uses a CDB to pass the initialisation parameters.
        The CDB information is read from the subtree that is currently addressed.
        For binary save/loads put the binaryy content in root.binary    */
    virtual bool ObjectSaveSetup(ConfigurationDataBase &cdb,StreamInterface *err){
        AssertErrorCondition(IllegalOperation,"ObjectSaveSetup is not implemented");
        return False;
    }

//###########################################################################################
//
//                                        ERROR MANAGEMENT
//
//###########################################################################################

#if defined(_VXWORKS)
    void VAssertErrorCondition(EMFErrorType errorCode,const char *errorDescription,va_list argList);

#else
    /**
        Sets the error status and depending on setup does appropriate action
        This call is to be called from static members
    */
    void VAssertErrorCondition(EMFErrorType errorCode,const char *errorDescription,va_list argList){
        VCAssertErrorCondition(errorCode,this,ClassName(),errorDescription,argList);
    }
#endif

#if defined(_VXWORKS)
    void AssertErrorCondition(EMFErrorType errorCode,const char *errorDescription=NULL,...) const;
#else
    /**
        Sets the error status and depending on setup does appropriate action
        This call is to be called from static members
    */
    void AssertErrorCondition(EMFErrorType errorCode,const char *errorDescription=NULL,...) const{
        va_list argList;
        va_start(argList,errorDescription);
        VCAssertErrorCondition(errorCode,this,ClassName(),errorDescription,argList);
        va_end(argList);
    }
#endif

#if defined(_VXWORKS)
    void ISRAssertErrorCondition(EMFErrorType errorCode,const char *errorDescription=NULL,...);
#else
    /**
        Sets the error status and depending on setup does appropriate action
        This call is to be called from interrupts
    */
    void ISRAssertErrorCondition(EMFErrorType errorCode,const char *errorDescription=NULL,...){
        va_list argList;
        va_start(argList,errorDescription);
        VCISRAssertErrorCondition(errorCode,this,ClassName(),errorDescription=NULL,argList);
        va_end(argList);
    }
#endif

#if defined(_OS2)
    /** Sets the error status and depending on setup does appropriate action */
    void AssertPlatformErrorCondition(EMFErrorType errorCode,int32 os2ErrorCode,const char *errorDescription=NULL,...){
        va_list argList;
        va_start(argList,errorDescription);
        VCAssertPlatformErrorCondition(errorCode,this,ClassName(),os2ErrorCode,errorDescription,argList);
        va_end(argList);
    }
#elif defined(_VXWORKS)
    void AssertPlatformErrorCondition(EMFErrorType errorCode,const char *errorDescription=NULL,...);
#else
    /** Sets the error status and depending on setup does appropriate action */
    void AssertPlatformErrorCondition(EMFErrorType errorCode,const char *errorDescription=NULL,...){
        va_list argList;
        va_start(argList,errorDescription);
        VCAssertPlatformErrorCondition(errorCode,this,ClassName(),errorDescription,argList);
        va_end(argList);
    }
#endif


};

#endif

