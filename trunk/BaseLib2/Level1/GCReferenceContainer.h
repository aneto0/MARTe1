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
 * A container of references to objects of type GarbageCollectable + Object.
 * The input/output operations are performed using GCReferences  
 */
#if !defined (GC_REFERENCE_CONTAINER_H)
#define GC_REFERENCE_CONTAINER_H

#include "LinkedListable.h"
#include "GCRTemplate.h"
#include "GCNamedObject.h"
#include "Object.h"
#include "MutexSem.h"
#include "GCRCItem.h"
#include "GCNOExtender.h"


/** Simply list the content of a GCReferenceContainer*/
class GCRCLister:public IteratorT<GCReference> {
    /** where to write to */
    StreamInterface *   stream;

    /** number of recursion levels */
    int                 level;
public:
    /** @param s = NULL the code will use printf to write to the console */
    GCRCLister(StreamInterface *s = NULL){
        stream = s;
        level = 0;
    }

    /** actual function */
    virtual void Do(GCReference data){
        GCRTemplate< GCNOExtender<BString> > gcnobs;
        gcnobs = data;
        if (gcnobs.IsValid()){
            if (stream == NULL) printf("*%s \n",gcnobs->Buffer());
            else stream->Printf("*%s \n",gcnobs->Buffer());
            return;
        }

        GCRTemplate<GCNamedObject> ref;
        ref = data;
        if (ref.IsValid()){
            if (stream == NULL) printf("%s %s \n",ref->ClassName(),ref->Name());
            else stream->Printf("%s %s \n",ref->ClassName(),ref->Name());
        }
    }

    /** actual function */
    virtual void Do2(GCReference data,SFTestType mode){
        if (mode == SFTTBack) level--;
        else
        if (mode == SFTTRecurse) level++;
        else {
            for (int i = 0;i < level;i++){
                if (stream == NULL) printf("    ");
                else stream->Printf("    ");
            }
            GCRTemplate< GCNOExtender<BString> > gcnobs;
            gcnobs = data;
            if (gcnobs.IsValid()){
                if (stream == NULL) printf("*%s \n",gcnobs->Buffer());
                else stream->Printf("*%s \n",gcnobs->Buffer());
                return;
            }
            GCRTemplate<GCNamedObject> ref;
            ref = data;
            if (ref.IsValid()){
                if (stream == NULL) printf("%s %s \n",ref->ClassName(),ref->Name());
                else stream->Printf("%s %s \n",ref->ClassName(),ref->Name());
            }
        }
    }
};

class GCReferenceContainer;

extern "C" {
    /**
        This is the main function operating to GCReferenceContainer (GCRC)
        @param gcrc is the container being sarched
        @param gc is the ref to the object found
        @param name if not NULL then the functionj searches for the object with this name
            name can be a composite xxx.yyy.zzz in wich case a container named xxx is searched for
            and one named yyy within xxx and finally the object zzz in yyy is returned.
        @param remove if True the find becomes an erase
        @param recurse if True the pattern xxx.yyy.zzz is searched also in subtrees
            so it will match any uuu... ...www.xxx.yyy.zzz
        @param index if not -1 then the index-th element of the container is returned
        @param selector if not NULL then the filter selector is applied as search criterium
            Uses the @param selector Test2 function (not Test) thus distinguishes
            between not found and not the right path and notifies of returning from
            recursion level using SFTTBack. One can match the partial finding on the
            way back out of recursion by returning SFTTFound in this last case
        @param partialMatch if not NULL then a partial match will be accepted i.e. xxx.yyy
            a partial match xxx.yyy will take priority over a recursive one if @param recursive is True
        */

    bool GCRCFind(              GCReferenceContainer &          gcrc,
                                GCReference &                   gc,
                                const char *                    name        =   NULL,
                                SearchFilterT<GCReference> *    selector    =   NULL,
                                int                             index       =   -1,
                                bool                            remove      =   False,
                                bool                            recurse     =   False,
                                const char **                   unMatched   =   NULL);

    /** */
    bool GCRCObjectLoadSetup(GCReferenceContainer &gcrc,ConfigurationDataBase &info,StreamInterface *err);

    /** */
    bool GCRCObjectSaveSetup(GCReferenceContainer &gcrc,ConfigurationDataBase &info,StreamInterface *err);

    /** if objectPath is not NULL then the function adds a link */
    bool GCRCInsert(GCReferenceContainer &gcrc,const char *objectPath,GCReference gc,int  position = -1);

    /** */
    bool GCRCIterate(GCReferenceContainer &gcrc,IteratorT<GCReference> * iterator,bool recurse);

}


OBJECT_DLL(GCReferenceContainer)

/** A container of objects that are GarbageCollectable.
    If the object are also descendent from GcNamedObject then
    they can be accessed by name */
class GCReferenceContainer: public GCNamedObject{

    friend bool GCRCFind(GCReferenceContainer &gcrc,GCReference &gc,const char *name,SearchFilterT<GCReference> * selector,int index,bool remove,bool recurse,const char **unMatched);

    friend bool GCRCObjectLoadSetup(GCReferenceContainer &gcrc,ConfigurationDataBase &info,StreamInterface *err);

    friend bool GCRCObjectSaveSetup(GCReferenceContainer &gcrc,ConfigurationDataBase &info,StreamInterface *err);

    friend bool GCRCInsert(GCReferenceContainer &gcrc,const char *objectPath,GCReference gc,int position);

    friend bool GCRCIterate(GCReferenceContainer &gcrc,IteratorT<GCReference> * iterator,bool recurse);

OBJECT_DLL_STUFF(GCReferenceContainer)

    /** to ease the use of GCRCFind*/
    inline GCReference _Find(   const char *                    name,
                                SearchFilterT<GCReference> *    selector    =   NULL,
                                int                             index       =   -1,
                                bool                            recurse     =   False,
                                const char **                   unMatched   =   NULL){
        GCReference gc;
        GCRCFind(*this,gc,name,selector,index,False,recurse,unMatched);
        return gc;
    }

    /** to ease the use of GCRCFind*/
    inline GCReference _Remove( const char *                    name,
                                SearchFilterT<GCReference> *    selector    =   NULL,
                                int                             index       =   -1,
                                bool                            recurse     =   False){
        GCReference gc;
        GCRCFind(*this,gc,name,selector,index,True,recurse);
        return gc;
    }

protected:

    /** The list of references */
    LinkedListHolder                list;

    /** protects multiple access to resources */
    MutexSem                        mux;

    /** timeout  */
    TimeoutType                     msecTimeout;

    /**  */
    LinkedListable *  CreateListElement(
                        const char *                objectPath,
                        GCReference                 gc);

public:

    /** the default constructor */
                                    GCReferenceContainer()
    {
        mux.Create();
        msecTimeout = TTInfiniteWait;
    }

    /** */
    virtual     ~GCReferenceContainer(){
        LinkedListable *p = list.List();
        list.Reset();
        while(p != NULL){
            LinkedListable *q = p;
            p = p->Next();
            delete q;
        }
        msecTimeout = TTInfiniteWait;
    }

    /** set internal semafore timeout in msec */
                void                SetTimeout(
                        TimeoutType                 msecTimeout)
    {
        this->msecTimeout = msecTimeout;
    }

    /**  See ObjectLoadSetup Syntax    */
    virtual     bool                ObjectSaveSetup(
                        ConfigurationDataBase &     info,
                        StreamInterface *           err)
    {
        return GCRCObjectSaveSetup(*this,info,err);
    };

    /**  initialise an object from a set of configs
        The syntax is
        <COMMAND> = {
            <command parameters>
        }

        Valid commands are Add, Remove, and AddReference

        example :

        Add = {
            name1 = {
                Class = <name of class>   -> Create object of given class
                ... class specific parameters
            }
        }
        alternative
        +name1 = {
            Class = <name of class>   -> Create object of given class
            ... class specific parameters
        }

        AddReference = {
            name2 name3
        } -> copy reference from globalObjectDataBase

        Remove = {
            name1 name2 name3  or ALL
        }
    */
    virtual     bool                ObjectLoadSetup(
                        ConfigurationDataBase &         info,
                        StreamInterface *               err)
    {

        return GCRCObjectLoadSetup(*this,info,err);
    }

    /** Removes all references   */
    inline      bool                CleanUp()
    {
        list.CleanUp();
        return True;
    }


    /** Adds an element at position @param position.
        The object added is a reference to the global object specified by
        @param objectPath.
        position = 0 is add before any other element.
        position = -1 is add at the end    */
    inline      bool                Insert(
                        const char *                    objectPath,
                        int                             position = -1)
    {
        GCReference dummy;
        return GCRCInsert(*this,objectPath,dummy,position);
    }

    /** Adds an element at position @param position.
        The object added is that referenced to by @param gc.
        position = -1 is add at the end    */
    inline      bool                Insert(
                        GCReference                     gc,
                        int                             position = -1)
    {
        return GCRCInsert(*this,NULL,gc,position);
    }

    /** finds an object by @param name and returns a reference to it
        into @param reference. if name contains separators like .
        than the first segment is matched and the remainder is passed
        to the matching object if it is a container.
        If recurse is true and the pattern cannot be matched from the root
        then it will try searching also from the subnodes.
        The tree is visited side to side.
        Note that if a name starts with ( it is a unique name in the form
        (%08x)name. In this case the name is not fragmented and is matched
        against the object unique name
        if unMatched is provided then a partial match is accepted and the
        unmatched part of the name is returned in unMatched as a pointer to
        name unmatched substring*/
    inline      GCReference         Find(
                        const char *                    name,
                        GCFlagType                      recurse =   GCFT_None,
                        const char **                   unMatched = NULL)
    {
        return _Find(name,NULL,-1,(recurse==GCFT_Recurse),unMatched);
    }

    /** finds an object and returns a reference to it into @param reference
        Uses the @param selector Test2 function (not Test) thus distinguishes
        between not found and not the right path and notifies of returning from
        recursion level using SFTTBack. One can match the partial finding on the
        way back out of recursion by returning SFTTFound in this last case  */
    inline      GCReference         Find(
                        SearchFilterT<GCReference> *    selector,
                        GCFlagType                      recurse =   GCFT_None)
    {
        return _Find(NULL,selector,-1,(recurse==GCFT_Recurse));
    }

    /** get a reference to the @param index -th element
        if noReference is True then a reference will be returned as a BString*/
    inline      GCReference         Find(
                        int                             index,
                        bool                            referenceAsBString = False)
    {
        return _Find(NULL,NULL,index,referenceAsBString);
    }

    /** removes an object by @param name and returns a reference to it
        into @param reference. if name conatins separators like . or :
        than the first segment is matched and the remainder is passed
        to the matching object if it is a container
        If recurse is true then it will try searching also the subnodes.
        The tree is visited side to side  */
    inline      GCReference         Remove(
                        const char *                    name,
                        GCFlagType                      recurse =   GCFT_None)
    {
        return _Remove(name,NULL,-1,(recurse==GCFT_Recurse));
    }

    /** removes any element from the list that fit the criteria specified by selector  */
    inline      GCReference         Remove(
                        SearchFilterT<GCReference> *    selector,
                        GCFlagType                      recurse =   GCFT_None)
    {
        return _Remove(NULL,selector,-1,(recurse==GCFT_Recurse));
    }

    /** removes the index-th element of the list  */
    inline      GCReference         Remove(
                        int                             index)
    {
        return _Remove(NULL,NULL,index);
    }

    /** acts on each element of the list using the provided iterator
        if recurse is enabled it applies the iterator also on the sub containers
        after and before applying it on the container itself
        Note that this function does not keep the container locked during the call
        to the user function. When recursing, only the current container is locked.
     */
                bool                Iterate(
                        IteratorT<GCReference> *        iterator,
                        GCFlagType                      recurse =   GCFT_None)
    {
        return GCRCIterate(*this,iterator,(recurse==GCFT_Recurse));

    }

    /** How many elements in the list */
                int                 Size()
    {
        return list.ListSize();
    }


    /** Lock the resource */
    bool Lock(){
        return mux.Lock(msecTimeout);
    }

    bool Lock2(TimeoutType tt){
        return mux.Lock(tt);
    }

    /** Unlock the resource */
    bool UnLock(){
        return mux.UnLock();
    }

};



#endif


