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

#include "GCReference.h"
#include "BString.h"
#include "GCRTemplate.h"
#include "GCNamedObject.h"
#include "GlobalObjectDataBase.h"
#include "ConfigurationDataBase.h"

#define nullReference NULL

/** Creates a new reference to an object spcified by name. */
void GCRConstructor(GCReference &gc,const char* typeName){

    gc.objectPointer = nullReference;
    if ((typeName == NULL) || (typeName[0] == 0)){
        CStaticAssertErrorCondition(FatalError,"GCReference::GCReference(): typeName is NULL or empty.");
        return;
    }

    Object* objPtr = OBJObjectCreateByName(typeName);
    if (objPtr==NULL){
        CStaticAssertErrorCondition(FatalError,"GCReference::GCReference(): \"%s\" Unknown type name.",typeName);
        return;
    } else {
        gc.objectPointer = dynamic_cast<GarbageCollectable*>(objPtr);
    }

    if (gc.objectPointer == NULL){
        CStaticAssertErrorCondition(FatalError,"GCReference::GCReference(): type %s does not support Garbage Collection.",typeName);
        delete objPtr;
        gc.objectPointer = nullReference;
        return;
    }

    gc.IncrementReference();
}


bool
GCRObjectLoadSetup(
            GCReference &               gc,
            ConfigurationDataBase &     info,
            StreamInterface *           err,
            bool                        createOnly){

    BString objectName;
    if (!info->NodeName(objectName)){
        CStaticAssertErrorCondition(FatalError,"GCReference::ObjectLoadSetup:Cannot retrieve name of node");
        return False;
    }

    // check for leaf: assume link in that case
    if (info->NumberOfChildren() < 0){
        BString linkName;
        int size[1] = { 1 };
        int nDim = 1;
        if (!info->ReadArray(&linkName,CDBTYPE_BString,size,nDim,"")){
            CStaticAssertErrorCondition(FatalError,"GCReference::ObjectLoadSetup:cannot read the valu eof node %s",objectName.Buffer());
            return False;
        }
        gc = GODBFindByName(linkName.Buffer());
        if (gc.IsValid()){
        } else {
            CStaticAssertErrorCondition(ParametersError,"GCReference::ObjectLoadSetup:%s=%s limk si not found ",objectName.Buffer(),linkName.Buffer());
            return False;
        }
        CStaticAssertErrorCondition(Information,"GCReference::ObjectLoadSetup:%s=%s is ok",objectName.Buffer(),linkName.Buffer());
        return True;
    }

    BString className;
    int size[1] = { 1 };
    int nDim = 1;
    if (!info->ReadArray(&className,CDBTYPE_BString,size,nDim,"Class")){
        CStaticAssertErrorCondition(FatalError,"GCReference::ObjectLoadSetup:cannot find the Class entry for node %s",objectName.Buffer());
        return False;
    } else {
        // this actually creates the object if the class is registered
        GCReference ref(className.Buffer());

        if (!ref.IsValid()){
            CStaticAssertErrorCondition(FatalError,"GCReference::ObjectLoadSetup:cannot build object for node %s of class %s",objectName.Buffer(),className.Buffer());
            return False;
        } else {
            gc = ref;

            if (createOnly){
                return True;
            } else {
                // see if it is a Object derivative
                // so that the object can be initialised
                GCRTemplate<Object> objectRef;
                objectRef = ref;
                if (objectRef.IsValid()){

                    // check for initialisation errors
                    if (!objectRef->ObjectLoadSetup(info,err)){
                        CStaticAssertErrorCondition(FatalError,"GCReference::ObjectLoadSetup:failed initing object %s of class %s ",objectName.Buffer(),className.Buffer());
                        gc.RemoveReference();
                        return False;
                    } else {
                        CStaticAssertErrorCondition(Information,"GCReference::ObjectLoadSetup:successfully added object %s of class %s ",objectName.Buffer(),className.Buffer());
                    }

                } else {
                    CStaticAssertErrorCondition(Information,"GCReference::ObjectLoadSetup:cannot call ObjectLoadSetup for object %s of class %s because not derived from Object",objectName.Buffer(),className.Buffer());
                }
            }
        }
    }

    return True;
}

bool
GCRObjectSaveSetup(
            GCReference &               gc,
            ConfigurationDataBase &     info,
            StreamInterface *           err){

    GCRTemplate<Object> objectRef;
    objectRef = gc;
    if (!gc.IsValid()){
        CStaticAssertErrorCondition(Information,"GCReference::ObjectSaveSetup:cannot be called for object non discendent from Object class\n");
        return False;
    }

    BString className;
    className = objectRef->ClassName();
    int size[1] = { 1 };
    int nDim = 1;
    if (!info->WriteArray(&className,CDBTYPE_BString,size,nDim,"Class")){
        CStaticAssertErrorCondition(FatalError,"GCReference::ObjectSaveSetup: cannot write the Class entry");
        return False;
    }

    return objectRef->ObjectSaveSetup(info,err);

}






