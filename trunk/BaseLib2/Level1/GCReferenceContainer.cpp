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

#include "GCReferenceContainer.h"
#include "GCNamedObject.h"
#include "ConfigurationDataBase.h"
#include "GlobalObjectDataBase.h"
#include "BString.h"
#include "MuxLock.h"

const uint32 GCRC_MAX_NUMBER_OF_OBJECTS = 10000;

OBJECTLOADREGISTER(GCReferenceContainer,"$Id$")

// set to 1 to use HARD REFERENCES WHEN POSSIBLE
// the downside is that one cannot save the object
#define GCRC_USE_HARD_REFERENCES 0

/** finds an object by @param name and returns a reference to it into @param reference  */
bool GCRCFind(
            GCReferenceContainer &          gcrc,
            GCReference &                   gc,
            const char *                    name,
            SearchFilterT<GCReference> *    selector,
            int                             index,
            bool                            remove,
            bool                            recurse,
            const char **                   unMatched){

    LinkedListable *ll = NULL;

    // if the request is to get a specific element then just do it
    if ((name == NULL) && (selector == NULL)) {

        // unlocks automatically on exit from function
        // just lock for this code block
        MuxLock muxLock;
        if (!muxLock.Lock(gcrc.mux, gcrc.msecTimeout)){
            gcrc.AssertErrorCondition(Timeout,"GCRCFind: timeout on resource sharing ");
            return False;
        }

        // get the LinkedListable;
        if (remove){
            ll = gcrc.list.ListExtract(index);
        } else {
            ll = gcrc.list.ListPeek(index);
        }
        if (ll == NULL) {
            gcrc.AssertErrorCondition(FatalError,"GCRCFind(%i): no element in the list fits criteria",index);
            return False;
        }

        // check type
        GCRCItem *gcri = dynamic_cast<GCRCItem *>(ll);
        if (gcri == NULL){
            gcrc.AssertErrorCondition(FatalError,"GCRCFind(): non GCRCItem derived element in the list");

            if (remove) delete ll;
            return False;
        }

        if (recurse && (gcri->Link()!=NULL)){
            GCRTemplate< GCNOExtender<BString> > gcrtgcnobs(GCFT_Create);
            BString *bs = gcrtgcnobs.operator->();
            (*bs)=gcri->Link();
            gc = gcrtgcnobs;
        } else {
            // retrieve reference
            gc = gcri->Reference();
        }
        if (remove) delete ll;
        return True;
    }

    if (name != NULL){
        // the name before the .
        BString baseName;
        // the name after the .
        //BString extraName;
        // use a pointer to the original nane so that I can pass it out as unmatched!
        const char *extraName = "";

        baseName = name;

        bool isUniqueName = (name[0] == '(');

//        if (!isUniqueName){
            // separate name
            int index = 0;
            while ((name[index] != 0) && (strchr(".",name[index])==NULL)) index++;
            if (name[index] != 0){
                extraName = name + index + 1;
                baseName.SetSize(index);
            }
//        }

        // loop through each element
        // no locking is necessary since the Find(int) function is already safe
        for (int ix = 0; ix < gcrc.Size(); ix++){

            // get ith element
            GCReference ref;
            // if recurse then get references as BString!!
            ref = gcrc.Find(ix,recurse);

            if (!ref.IsValid()){
                gcrc.AssertErrorCondition(Warning,"GCRCFind(%s): element %i not in list",name,index);
                return False;
            }

            // check name
            GCRTemplate<GCNamedObject> gcrtgcno = ref;
            if (gcrtgcno.IsValid()){

                // treat unique name as any other name and allow search of a subtree : (nnn).subname.subname2
                bool found = False;
                if (isUniqueName) {
                    BString uniqueName;
                    gcrtgcno->GetUniqueName(uniqueName);
                    found =(uniqueName == baseName);
                } else {
                    found = (baseName == gcrtgcno->Name());
                }

/*
                if (isUniqueName){
                    BString uniqueName;
                    gcrtgcno->GetUniqueName(uniqueName);
                    if (uniqueName == baseName){
                        gc = ref;
                        if (remove){
                            gcrc.Remove(ix);
                        }
                        return True;
                    }
                } else

                if (baseName == gcrtgcno->Name()){
*/
                if (found) {
                    // found only if extra name is NULL
                    //if (extraName.Size() == 0){
                    if (extraName[0] == 0){
                        gc = ref;
                        if (remove) {
                            gcrc.Remove(ix);
                        }
                        return True;
                    } else { // match all the name
                        // check for a container
                        GCRTemplate<GCReferenceContainer> gcrtgcrc = ref;
                        if (gcrtgcrc.IsValid()){
                            if (remove) {
                                /* recurse is set to False because we already have matched part of the string and the rest must follow */
                                gc = gcrtgcrc->_Remove(extraName/*.Buffer()*/, NULL, -1 , /*recurse*/False);
                            } else {
                                gc = gcrtgcrc->_Find(extraName/*.Buffer()*/, NULL, -1 , /*recurse*/ False,unMatched);
                            }
                            // exit if found
                            if (gc.IsValid()) return True;
                        }
                        // not found...

                        // partial match!
                        if ((unMatched) && (!remove)){
                            gc = ref;
                            *unMatched = extraName;
                            return True;
                        }

                        // it was not a folder but we expected so
                        return False;
                    }
                }

                // not match the name but recurse on
                if (recurse){

                    // check for a container
                    GCRTemplate<GCReferenceContainer> gcrtgcrc = ref;
                    if (gcrtgcrc.IsValid()){

                        GCReference ref;
                        if (remove) {
                            ref = gcrtgcrc->_Remove(name, NULL, -1 , recurse);
                        } else {
                            ref = gcrtgcrc->_Find(name, NULL, -1 , recurse);
                        }

                        // exit if found
                        if (ref.IsValid()){
                            gc = ref;
                            return True;
                        }
                    }
                }

            } // end for
        }

        return False;
    } // name != NULL

    if (selector != NULL){

        // loop through each element
        // no locking is necessary since the Find(int) function is already safe
        int ix;
        for (ix = 0; ix < gcrc.Size(); ix++){

            // get ith element
            GCReference ref = gcrc.Find(ix/*,recurse*/);
            if (!ref.IsValid()){
                gcrc.AssertErrorCondition(Warning,"GCRCFind(char *): element %i not in list",ix);
                return False;
            }

            // check node or mark start of recurse
            SFTestType sftt;
            sftt = selector->Test2(ref);
            if (sftt == SFTTFound){
                gc = ref;
                if (remove){
                    gcrc.Remove(ix);
                }
                return True;
            } else

            // if recursion check and not the wrong path
            // if a group node and then visit it
            if (recurse && (sftt != SFTTWrongPath)) {

                // check type
                GCRTemplate<GCReferenceContainer> gcrtgcrc = ref;
                if (gcrtgcrc.IsValid()){

                    if (remove){
                        gc = gcrtgcrc->_Remove(NULL, selector, -1 , recurse);
                    } else {
                        gc = gcrtgcrc->_Find(NULL  , selector, -1 , recurse);
                    }

                    // exit if found
                    if (gc.IsValid())  return True;

                    // mark end of recurse
                    // allow matching on the way back
                    //
                    if (selector->Test2(ref,SFTTBack) == SFTTFound){
                        gc = ref;
                        return True;
                    }
                }

            } // if recurse

        } // for loop
    } // name != NULL

    return False;
}


bool
GCRCIterate(
        GCReferenceContainer &          gcrc,
        IteratorT<GCReference> *        iterator,
        bool                            recurse){

    // do all elements first
    int i;
    for (i=0;i<gcrc.Size();i++){


        GCReference ref = gcrc.Find(i,recurse);

        // does the element or notifies start of recursion
        if (ref.IsValid()) iterator->Do2(ref);

        if (recurse){
            GCRTemplate<GCReferenceContainer> gcrtgcrc = ref;
            if (gcrtgcrc.IsValid()) {

                // notify end of recursion
                iterator->Do2(ref,SFTTRecurse);

                if (recurse)
                    gcrtgcrc->Iterate(iterator,GCFT_Recurse);
                else
                    gcrtgcrc->Iterate(iterator,GCFT_None);

                // notify end of recursion
                iterator->Do2(ref,SFTTBack);
            }
        }
    }

    return True;

}

bool GCRCObjectSaveSetup(GCReferenceContainer &gcrc,ConfigurationDataBase &info,StreamInterface *err){

    // load name of object
    gcrc.GCNamedObject::ObjectSaveSetup(info,err);

    // unlocks automatically on exit from function
    MuxLock muxLock;
    if (!muxLock.Lock(gcrc.mux, gcrc.msecTimeout)){
        gcrc.AssertErrorCondition(Timeout,"ObjectSaveSetup: timeout on resource sharing ");
        return False;
    }

    ConfigurationDataBase cdb(info);

//    BString msg = "ALL";
    int size[1] ={ 1};

//    if (!cdb->WriteArray(&msg,CDBTYPE_BString,&size[0],1,"Remove")){
//        gcrc.AssertErrorCondition(FatalError,"ObjectSaveSetup:cannot assign \"Remove = ALL \" subtree");
//        return False;
//    }

    cdb->MoveToFather();

    if (gcrc.Size() > 0){

//        if (!cdb->AddChildAndMove("Add")){
//            gcrc.AssertErrorCondition(FatalError,"ObjectSaveSetup:cannot AddChildAndMove(\"Add\")");
//            return False;
//        }

        BString *referenceObjects = new BString[gcrc.Size()];
        int noOfReferenceObjects = 0;

        int i;
        for (i = 0; i < gcrc.Size();i++){

            GCReference reference = gcrc.Find(i,True);
            if (reference.IsValid()){

                // is it a reference ?
                GCRTemplate< GCNOExtender<BString> > gcrtgcnobs;
                gcrtgcnobs =  reference;
                if (gcrtgcnobs.IsValid()){
                    referenceObjects[noOfReferenceObjects++] = gcrtgcnobs->Buffer();
                } else {

                    // is it an object with name
                    GCRTemplate<GCNamedObject> namedObjectRef;
                    namedObjectRef = reference;
                    if (namedObjectRef.IsValid()){
                        int mallocSize = strlen(namedObjectRef->Name())+3;
                        char *name = (char *)malloc (mallocSize);
                        if (name != NULL){
                            name[0] = 0;
                            strcat(name,"+");
                            strcat(name,namedObjectRef->Name());

                            if (!cdb->AddChildAndMove(name)){
                                gcrc.AssertErrorCondition(FatalError,"cannot AddChildAndMove(\"%s\")",name);
                                delete[] referenceObjects;
                                free((void *&)name);
                                return False;
                            }

                            free((void *&)name);
                            namedObjectRef.ObjectSaveSetup(cdb,err);

//                        namedObjectRef->ObjectSaveSetup(cdb,err);

                            cdb->MoveToFather();
                        } else {
                            gcrc.AssertErrorCondition(FatalError,"ObjectSaveSetup: malloc(%i) failed ",mallocSize);
                        }
                    // any other object
                    } else {
                        char name[32];
                        sprintf(name,"+OBJECT%i",i);
                        GCRTemplate<Object> objectRef;
                        objectRef = reference;
                        if (objectRef.IsValid()){
                            if (cdb->AddChildAndMove(name)){
                                gcrc.AssertErrorCondition(FatalError,"cannot AddChildAndMove(\"%s\")",name);
                                delete[] referenceObjects;
                                return False;
                            }

                            objectRef->ObjectSaveSetup(cdb,err);

                            cdb->MoveToFather();

                        } else {
                            gcrc.AssertErrorCondition(FatalError,"Cannot save objects whose class do not descend from Object");
                        }
                    }
                }
            }
        }

//        cdb->MoveToFather();

        if (noOfReferenceObjects > 0){
            int size[1] = { noOfReferenceObjects };
            if (!cdb->WriteArray(referenceObjects,CDBTYPE_BString,&size[0],1,"AddReference")){
                gcrc.AssertErrorCondition(FatalError,"cannot write \"AddReference = {} \"");
                delete[] referenceObjects;
                return False;
            }
        }
        delete[] referenceObjects;

    }

    return True;
}

bool GCRCObjectLoadSetup(GCReferenceContainer &gcrc,ConfigurationDataBase &info,StreamInterface *err){

    // we will proceed on error in many cases but still report a False
    int errorCount = 0;

    // load name of object
    gcrc.GCNamedObject::ObjectLoadSetup(info,err);

    ConfigurationDataBase cdb(info);

    // loop among the options Add AddReference and Remove
    // basically execute first the one that is specified first
    int childNo = 0;
    while (cdb->MoveToChildren(childNo)){
        BString name;
        if (!cdb->NodeName(name)){
            gcrc.AssertErrorCondition(FatalError,"ObjectLoadSetup: cdb->NodeName(name) failed for node %i",childNo);
            return False;
        }
	
        if ((name == "AddReference") ||
            (name == "Remove")){

            bool isRemove = (name == "Remove");

            // check for leaf
            if (cdb->NumberOfChildren() > 0){
                gcrc.AssertErrorCondition(ParametersError,"%s must be a vector of string node not a tree node",name.Buffer());
                return False;
            }

            // how many elements to add
            int size[2];
            int nDim = 2;
            if (!cdb->GetArrayDims(size,nDim,"")){
                gcrc.AssertErrorCondition(FatalError,"cannot get dimension of %s",name.Buffer());
                return False;
            }

            if (nDim > 1){
                gcrc.AssertErrorCondition(ParametersError,"%s should be a one dimensional array",name.Buffer());
                return False;
            }

            // Only allow up to GCRC_MAX_NUMBER_OF_OBJECTS
            if (size[0] > GCRC_MAX_NUMBER_OF_OBJECTS){
                gcrc.AssertErrorCondition(ParametersError,"%s specifies too many objects : %i",name.Buffer(),size[0]);
                return False;
            }

            BString *nameArray = new BString[size[0]];
            if (!cdb->ReadArray(nameArray,CDBTYPE_BString,size,nDim,"")){
                gcrc.AssertErrorCondition(FatalError,"cannot read the value of node %s",name.Buffer());
                delete[] nameArray;
                return False;
            }

            int ix;
            for (ix = 0; ix < size[0]; ix++){

                if (isRemove){
                    if (nameArray[ix] == "ALL"){
                        gcrc.CleanUp();
                    } else {
                        GCReference temp = gcrc.Remove(nameArray[ix].Buffer());
                        if (!temp.IsValid()){
                            gcrc.AssertErrorCondition(ParametersError,"failed removing %s ",nameArray[ix].Buffer());
                            errorCount++;
                        }
                    }

                } else { // AddReference
#if GCRC_USE_HARD_REFERENCES
                    GCReference ref;
                    ref = GODBFindByName(nameArray[ix].Buffer());
                    if (ref.IsValid()){
                        if (!gcrc.Insert(ref)){
                            gcrc.AssertErrorCondition(FatalError,"cannot insert node %s",nameArray[ix].Buffer());
                        }
                    } else
#endif
                    if (!gcrc.Insert(nameArray[ix].Buffer())){
                        gcrc.AssertErrorCondition(FatalError,"cannot insert node %s",nameArray[ix].Buffer());
                        errorCount++;
                    } else {
                        gcrc.AssertErrorCondition(Information,"added link to node %s",nameArray[ix].Buffer());
                    }
                }
           }

           delete[] nameArray;

        } else
        if (name == "Add"){
            int objectNo = 0;

            // check for leaf
            if (cdb->NumberOfChildren() < 0){
                gcrc.AssertErrorCondition(ParametersError,"%s must be a tree node h",name.Buffer());
                errorCount++;
            } else {

                // loop among the name = blocks
                while (cdb->MoveToChildren(objectNo)){

                    GCReference ref;
                    if (!ref.ObjectLoadSetup(cdb,err,True)){
                        errorCount++;
                    } else {
                        gcrc.Insert(ref);

                        GCRTemplate<Object> oref;
                        oref = ref;
                        if (oref.IsValid()){
                            if (!oref->ObjectLoadSetup(cdb,err)){
                                gcrc.AssertErrorCondition(InitialisationError,"%s initialisation error",name.Buffer());
                                errorCount++;
                            }
                        }
                    }

                    objectNo++;
                    cdb->MoveToFather();
                }   // end while
            }
        } else
        if (name.Buffer()[0] == '+'){

            GCReference ref;
            if (!ref.ObjectLoadSetup(cdb,err,True)){
                errorCount++;
            } else {
                gcrc.Insert(ref);

                GCRTemplate<Object> oref;
                oref = ref;
                if (oref.IsValid()){
                    if (!oref->ObjectLoadSetup(cdb,err)){
                        gcrc.AssertErrorCondition(InitialisationError,"%s initialisation error",name.Buffer());
                        errorCount++;
                    }
/* Correct the name by removing the '+'
                      else {
                        GCRTemplate<GCNamedObject> gcnoref;
                        gcnoref = ref;
                        if (gcnoref.IsValid()){
                            if ((gcnoref->Name()[0] == '+') ||
                                (gcnoref->Name()[0] == 0)){
                                gcnoref->SetObjectName(name.Buffer()+1);
                            }
                        }
                    }
*/
                }
            }
        }
// reporting this might be over the top
//        else  {
//            gcrc.AssertErrorCondition(Warning,"ObjectLoadSetup: node name %s is unknown command (Add/Remove)",name.Buffer());
//        }

        childNo++;
        cdb->MoveToFather();
    }

    return (errorCount == 0);
}

LinkedListable *
GCReferenceContainer::CreateListElement(
              const char *                objectPath,
              GCReference                 gc)
{

#if GCRC_USE_HARD_REFERENCES
    gc = GODBFindByName(objectPath);
    if (gc.IsValid()){
        objectPath = NULL;
    }
#endif

    if (objectPath != NULL){

        GCRCILink *gcrci = new GCRCILink;
        if (gcrci->Load(objectPath)){
            return gcrci;
        }
        delete gcrci;

        return NULL;
    }

    GCRCIBasic *gcrci = new GCRCIBasic;
    if (gcrci->Load(gc)){
        return gcrci;
    }
    delete gcrci;

    return NULL;
}

bool
GCRCInsert(
        GCReferenceContainer &          gcrc,
        const char *                    objectPath,
        GCReference                     gc,
        int                             position){

    // unlocks automatically on exit from function
    MuxLock muxLock;
    if (!muxLock.Lock(gcrc.mux, gcrc.msecTimeout)){
        gcrc.AssertErrorCondition(Timeout,"Insert: timeout on resource sharing ");
        return False;
    }

    LinkedListable *ll = gcrc.CreateListElement(objectPath,gc);

    if (ll == NULL){
        BString s;
        GCRTemplate<GCNamedObject> ref;
        ref = gc;
        if (ref.IsValid()) s = ref->Name();

        gcrc.AssertErrorCondition(FatalError,"Insert: failed adding object %s. Possibly non compatible class",s.Buffer());
        return False;
    }
    if (position == -1) gcrc.list.ListAdd(ll);
    else                gcrc.list.ListInsert(ll,position);
    return True;
}






