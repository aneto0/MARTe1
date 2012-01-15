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
 * A class managing a pointer to an Object of type GarbageCollectable.
 * The class implements a garbage collection mechanism that will destroy
 * a class only when all references are destroyed.
 * Access to the methods of the referred object is obtained via the ()
 * operator.
 * Shared use of the same reference on multiple threads (static) needs semaphore protection.
 * Use of different references to same object is safe.
 */
#if !defined (_GCREFERENCE_H)
#define _GCREFERENCE_H

#include "Atomic.h"
#include "ObjectRegistryDataBase.h"
#include "GarbageCollectable.h"

#define nullReference NULL

/** Flags for the Garbage Collection classes */
enum GCFlagType {

    /** Used to request recursion on operations on a GCReferenceContainer */
    GCFT_Recurse        = 2,

    /** Used when constructing GCRTemplate to force a creation of an instance of the object */
    GCFT_Create         = 1,

    /**  */
    GCFT_None           = 0
};

extern "C"{

    void GCRConstructor(GCReference &gc, const char *typeName);

    bool GCRObjectLoadSetup(GCReference &gc,ConfigurationDataBase &info,StreamInterface *err,bool createOnly);

    bool GCRObjectSaveSetup(GCReference &gc,ConfigurationDataBase &info,StreamInterface *err);

}

class GCReference{

    friend void GCRConstructor(GCReference &gc, const char *typeName);

    friend bool GCRObjectLoadSetup(GCReference &gc,ConfigurationDataBase &info,StreamInterface *err,bool createOnly);

    friend bool GCRObjectSaveSetup(GCReference &gc,ConfigurationDataBase &info,StreamInterface *err);

private:

    /** This definition prevents users to make a copy of a reference
        by taking its address. */
    GCReference* operator&(){
        return this;
    }

    /** Toi allow friend functions to access private memebers*/
    void IncrementReference(){
        objectPointer->Increment();
    }

protected:

    /** The pointer to the referenced object. */
    GarbageCollectable*         objectPointer;

#if defined (GCRLOADPOINTER)
public:
#else
protected:
#endif

    /** refer to an Object pointer
       accessible only if macro GCRLOADPOINTER is set */
    GCReference& operator=(GarbageCollectable * pointer)
    {
        RemoveReference();
        objectPointer = pointer;
        if (objectPointer == nullReference){
            CStaticAssertErrorCondition(FatalError,"GCReference::operator=(GCReference *p): p is NULL");
            return *this;
        }
        objectPointer->Increment();
        return *this;
    };

    /** Creates a new reference to an object pointer.
       accessible only if macro GCRLOADPOINTER is set */
    GCReference(GarbageCollectable * pointer)
    {
        objectPointer = nullReference;
        *this = pointer;
    }

public:

    /** Creates an empty reference. */
    GCReference()
    {
        objectPointer = nullReference;
    };

    /** Creates a new reference from an existing one. */
    GCReference(const GCReference& object)
    {
        objectPointer = nullReference;
        (*this)=object;
    };

    /** Creates a new object of tyepe @param typeName and builds a reference to it */
    GCReference(const char* typeName)
    {
        GCRConstructor(*this,typeName);
    };

    /**  Create an object from a set of configs
        The syntax is
        Name = {
            Class = < class Name >
            <Class specific>
        }
        Or
        Name = "LinK to object in the GlobalObjectDataBase"
        Note that info should point to the subtree "Name"
        if createOnly == True than the object ObjectLoadSetup is not called     */
    virtual     bool                ObjectLoadSetup(
                        ConfigurationDataBase &         info,
                        StreamInterface *               err,
                        bool                            createOnly=False)
    {
        return GCRObjectLoadSetup(*this,info,err,createOnly);
    }

    /**  save an object reference to a set of configs
        The syntax is
        Class = < class Name >
        <Class specific>      */
    virtual     bool                ObjectSaveSetup(
                        ConfigurationDataBase &         info,
                        StreamInterface *               err)
    {
        return GCRObjectSaveSetup(*this,info,err);
    }



    /** Correctly removes the reference. */
    virtual void RemoveReference()
    {
        if (objectPointer != nullReference){
            int32 numberOfReferences = objectPointer->Decrement();

            if (numberOfReferences == 0){
                delete objectPointer;  // Polimorphism used here it need to destroy derived object.
            }
            else if (numberOfReferences < 0){
                CStaticAssertErrorCondition(FatalError, "GCReference::RemoveReference: The number of references is negative: %d", numberOfReferences);
            }
        }
        objectPointer = nullReference;
    };

    /** Assignament operator for references. */
    GCReference& operator=(const GCReference& reference)
    {
        RemoveReference();
        if (reference.IsValid()){
            reference.objectPointer->Increment();
            objectPointer = reference.objectPointer;
        }
        return *this;
    };

    /** Needed to enable virtual deleting */
    virtual ~GCReference()
    {
        RemoveReference();
    };

    /** A function to stabilish if the reference is pointing to a valid object. */
    virtual bool IsValid() const
    {
        return (objectPointer != nullReference);
    };

    /** Gets the number of references. */
    inline int32 NumberOfReferences() const {
        if (!IsValid()){
            return 0;
        }
        return objectPointer->NumberOfReferences();
    }

    /** Equivalence operator. */
    inline bool operator==(const GCReference& reference) const {
        return (objectPointer == reference.objectPointer);
    }

    /** Not Equivalence operator. */
    inline bool operator!=(const GCReference& reference) const {
        return (objectPointer != reference.objectPointer);
    }

    /** It allows the access to the pointed object.
        It can be used also if the object is invalid */
    inline GarbageCollectable* operator->() const
    {
        return objectPointer;
    }

    /** Creates a reference to a duplicate object */
    inline bool Clone(const GCReference &reference){
        if (!reference.IsValid()) return False;
        GarbageCollectable* tmp = reference->Clone();
        if (tmp == NULL) return False;

        RemoveReference();
        objectPointer = tmp;
        objectPointer->Increment();
        
        // This is necessary, otherwise when
        // GCReference::Clone is called by
        // GCRTemplate, at this point the IsValid
        // function of GCRTemplate would be called,
        // returning false as the setup of GCRTemplate
        // Tobject is not yet done.
        return GCReference::IsValid();
    }
};

/** Usage GCReference gc = GCNEW(myClass); */
#define GCNew(className)            \
    GCReference (new className)




#endif
