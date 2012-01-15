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
 * Definition of the GCRTemplate class.
 * This template generates a specialised GCReference that is able to refer to
 * objects of class T derivates. GCTemplate is also a derivate of GCReference
 */

#if !defined (_GCRTEMPLATE_H)
#define _GCRTEMPLATE_H

#include "GCReference.h"

#define nullTReference NULL


template<typename T>
class GCRTemplate : public GCReference{
private:

    /** This definition prevents users to make a copy of a reference
        by taking its address. */
    GCRTemplate<T>* operator&(){
        return this;
    }

protected:

    /** The pointer to the referenced object seen with the T interface.
        Note that objectPointer is also available from the mother class */
    T* typeTObjectPointer;

    /** standard initialisation */
    void Init(){
        objectPointer       = nullReference;
        typeTObjectPointer  = nullTReference;
    }

    /** use when subclassing */
    void Load(T *p){
        Init();
        if (p != NULL){
            GarbageCollectable *gc = dynamic_cast<GarbageCollectable *>(p);
            if (gc != NULL){
                GCReference::operator=(gc);
                typeTObjectPointer = p;
            }
        }
    }

public:

    /** Default Constructor */
    GCRTemplate(){
        Init();
    }

    /** Creates an empty reference or a reference to base type T.
        if @param create is GCFT_CreateInstance then it will create the object */
    GCRTemplate(GCFlagType create){
        Init();
        if (create == GCFT_Create){
            T *p = new T;
            if (p != NULL){
                GarbageCollectable *gc;
                gc = dynamic_cast<GarbageCollectable *>(p);
                if (gc != NULL){
                    GCReference::operator=(gc);
                    typeTObjectPointer = p;
                }
            }
        }
    }

    /** Creates a reference to an object descendent from base type T.
        Recommended usage is GCRTemplate<T>(new MYClass());   */
    GCRTemplate(T *p){
        Load(p);
    }

    /** Creates a new reference copting from a geneeric one
        The operation might fail, in which case the reference
        produced is invalid. */
    GCRTemplate(const GCReference& object){
        Init();

        // use operator =
        (*this) = object;
    }

    /** Creates a new reference from an existing one. */
    GCRTemplate(const GCRTemplate<T>& object){
        Init();

        // use operator =
        (*this) = object;
    }

    /** Creates a new reference to an object spcified by name. */
    GCRTemplate(const char* typeName): GCReference(typeName){

        if (!GCReference::IsValid()){
            typeTObjectPointer = nullTReference;
            return;
        }

        typeTObjectPointer = dynamic_cast<T*>(objectPointer);
        if (typeTObjectPointer == NULL){
            GCReference::RemoveReference();
            typeTObjectPointer = nullTReference;
            return;
        }
    }

    /** Correctly removes the reference. */
    virtual void RemoveReference(){
        typeTObjectPointer = nullTReference;

        GCReference::RemoveReference();
    }

    /** at the end destroy the object if necessary */
    virtual ~GCRTemplate(){
        RemoveReference();
    }

    /** A function to stabilish if the reference is pointing to a valid object. */
    virtual bool IsValid() const {
        if (!GCReference::IsValid()) return False;
        return (typeTObjectPointer != nullTReference);
    }

    /** Assignament operator for references. */
    GCRTemplate<T>& operator=(const GCRTemplate<T>& reference){
        RemoveReference();
        if (reference.IsValid()){
            GCReference::operator=(reference);
            typeTObjectPointer = reference.typeTObjectPointer;
        }

        return *this;
    }

    /** Assignement operator from the type of the parent class. */
    GCRTemplate<T>& operator=(const GCReference& reference){
        RemoveReference();

        if (reference.IsValid()){
            // do this first to allow access to objectPointer
            GCReference::operator=(reference);
            typeTObjectPointer = dynamic_cast<T*>(objectPointer);
            if (typeTObjectPointer == NULL){
                RemoveReference();
            }
        }

        return *this;
    }

    /** Creates a reference to a duplicate object */
    inline bool Clone(const GCReference& reference) {
        RemoveReference();

        if (!GCReference::Clone(reference)) return False ;

        typeTObjectPointer = dynamic_cast<T*>(objectPointer);
        if (typeTObjectPointer == NULL){
            RemoveReference();
        }

        return IsValid();
    }

    /** Creates a reference to a duplicate object */
    inline bool Clone(const GCRTemplate<T>& reference) {
        RemoveReference();

        if (!GCReference::Clone(reference)) return False ;

        typeTObjectPointer = dynamic_cast<T*>(objectPointer);
        if (typeTObjectPointer == NULL){
            RemoveReference();
        }

        return IsValid();
    }

    /** Equivalence operator. */
    bool operator== (const GCRTemplate<T>& reference){
        return ((GCReference::operator==(reference)) &&
            (typeTObjectPointer == reference.typeTObjectPointer));
    }

    /** Operator -> used to access the referenced object methods and field. */
    T* operator->()const {
        return typeTObjectPointer;
    }

    /**  Create an object from a set of configs
        The syntax is
        Name = {
            Class = < class Name >
            <Class specific>
        }
        Note that info should address the subtree not the node "Name"

        if createOnly == True than the object ObjectLoadSetup is not called
    */
    virtual     bool                ObjectLoadSetup(
                        ConfigurationDataBase &         info,
                        StreamInterface *               err,
                        bool                            createOnly=False){
        GCReference gc;
        if (!gc.ObjectLoadSetup(info,err,createOnly)) return False;
        *this = gc;
        return IsValid();
    }

};

#endif

