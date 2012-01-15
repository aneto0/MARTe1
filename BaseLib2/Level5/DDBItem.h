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
 * Definition of the class DDBItem.
 */
#if !defined (DDBITEM_H)
#define DDBITEM_H

#include "Object.h"
#include "DDBDefinitions.h"
#include "DDBInterface.h"
#include "DDBInterfaceDescriptor.h"
#include "LinkedListable.h"
#include "DDBSignalDescriptor.h"
#include "BasicTypes.h"

class LinkedListHolder;
class DDBInterface;

extern "C"{
    /** Implements DDBItem Browse functionality for Menu*/
    bool DDBItemBrowseMenu(StreamInterface &in,StreamInterface &out,void *userData);
}


/** This class imlements the abstraction of the signal descriptor from the DDB point of view.
    The name of the DDBItem is the same of the name of the signal. */
OBJECT_DLL(DDBItem)
class DDBItem: public LinkedListable, public Object{

    friend class CheckDDBItemIterator;

    friend bool DDBItemBrowseMenu(StreamInterface &in,StreamInterface &out,void *userData);

    OBJECT_DLL_STUFF(DDBItem)

private:

    /** The name of the signal. */
    FString                     signalName;

    /** Size of the signal */
    int                         size;

    /** A field coding the type and size of the elementary type used to build this signal. */
    BasicTypeDescriptor         typeCode;

    /** Offset in bytes of the signal structure in the DBB. It will be set to 0 at construction time
        and updated by the proper function of the DDB class. */
    int32                       signalOffset;

private:

    /** Contains the status of the signal (i.e. if there is access Error...)*/
    DDBItemStatus               status;

    /** Contains the access Mode of the signal (should be the or of the access modes of the ddb interfaces)*/
    DDBInterfaceAccessMode      accessMode;

    /** List of users. It must be a list of type DDBUser. */
    LinkedListHolder            listOfInterfaceDescriptors;

private:

    /** Updates the status of the DDBItem after insertion of a new interface.
        The function checks whether there are some access violation and set
        the status accordingly.
    */
    void UpdateStatus(const DDBInterface& ddbInterface);

    /** To find by name a user in the listOfInterfaceDescriptors.
        @param userName Name of the user.
        @return A pointer to the user if it has been found, NULL otherwise.
     */
    DDBInterfaceDescriptor* Find(const DDBInterface& ddbInterface){
        DDBInterfaceDescriptorSearchFilter ddbidsf(ddbInterface.GetInterfaceDescriptor().OwnerName(),ddbInterface.GetInterfaceDescriptor().InterfaceName());
        return dynamic_cast<DDBInterfaceDescriptor*>(listOfInterfaceDescriptors.ListSearch(&ddbidsf)); // Get the first element in the list.
    }

public:

    /** The contructor. */
    DDBItem(const char*          signalName,
            int                  size,
            BasicTypeDescriptor  typeCode){

        this->signalName       = signalName;
        this->size             = size;
        this->typeCode         = typeCode;
        this->signalOffset     = 0;

    }

    /** A constructor of a DDBItem from a SignalDescriptor. */
    DDBItem(const DDBSignalDescriptor& descriptor){
        this->signalName = descriptor.SignalName();
        this->size       = descriptor.SignalSize();
        this->typeCode   = descriptor.SignalTypeCode();
        // If the signal descriptor does not specify the real signal size..
        if(descriptor.SignalStoringProperties().CheckMask(DDB_Unsized)){
            status      |= DDB_UndefinedSize;
        }
        signalOffset     = 0;
    }

    /** The virtual destructs allows to correctly free memory allocated for polimorphic objects. */
    virtual ~DDBItem(){ }

    /** If it is the first user, it sets the variables name and type otherwise it checks for compatibility.
        It may set the WriteConflictError bit.
        @param ddbInterface DDB Interface that uses this item.
        @return True if the user has been added. */
    bool AddInterfaceDescriptor(const DDBInterface& ddbInterface);

    /** Used by the AddInterface method in DDB. */
    void SetSignalOffset(int32 offset){
        signalOffset=offset;
    }

public:
    /** Set the signal status to the desired value. Previous values
        stored in the status are lost.
    */
    void SetSignalStatus(DDBItemStatus status){
        this->status = status;
    }

    /** Method to read the signalOffset. */
    int32 SignalOffset() const {
        return signalOffset;
    }

    /** Returns the signal Name. */
    const char *SignalName()const {
        return  signalName.Buffer();
    }

    /** Returns the signal size.*/
    int32 SignalSize(){
        return size;
    }

    /** Returns the signal type code. */
    BasicTypeDescriptor SignalTypeCode() const{
        return typeCode;
    }

    /** Set the signal size to the desired value. */
    void SetSignalSize(int32 size){
        this->size = size;
    }

    /** Returns a copy of the ddbitem status. */
    DDBItemStatus Status() const {
        return status;
    }

    /** Returns a copy of the ddbitem access mode. */
    DDBInterfaceAccessMode Access() const{
        return accessMode;
    }
    /** Print the object data. */
    void Print(StreamInterface& s, bool showUsers);

    /** DDBSignalDescriptor save function. Uses a CDB to pass the initialisation parameters.
        The CDB information is read from the subtree that is currently addressed.
        For binary save/loads put the binaryy content in root.binary    */
    virtual bool ObjectSaveSetup(ConfigurationDataBase &cdb,StreamInterface *err);

    /** DDBSignalDescriptor save function. Uses a CDB to pass the initialisation parameters.
        The CDB information is read from the subtree that is currently addressed.
        For binary save/loads put the binaryy content in root.binary    */
    virtual bool ObjectLoadSetup(ConfigurationDataBase &cdb,StreamInterface *err);

};

/** This is the iterator for a list of DDBSignalDescriptor. N.B.!
    After the iterator has been used, a call to the Reset method need
    to be made to use it again. */
class CheckDDBItemIterator : public Iterator{
private:

    /** The result of the check on the DDBItem list. */
    bool& checkResult;
    int32& currentSize;

public:

    /** The constructor initialises the stream as specified. */
    CheckDDBItemIterator(bool& checkResult, int32& currentSize): checkResult(checkResult),
        currentSize(currentSize){
        this->checkResult=True;
        this->currentSize=0;
    }

    /** The print operation. */
    void Do(LinkedListable* ll){
        DDBItem* ddbi=dynamic_cast<DDBItem*>(ll);
        if(ddbi == NULL)return;

        DDBItemStatus status = ddbi->Status();

        ddbi->SetSignalOffset(currentSize);
	currentSize += ddbi->SignalTypeCode().ByteSize() * ddbi->SignalSize();

        if (status==DDB_NoError){
            checkResult=checkResult && True;
        }
        else{
            FString errorDescription;
            errorDescription.Printf("Signal %s",ddbi->SignalName());
            LinkedListable *item = ddbi->listOfInterfaceDescriptors.List();
            for(uint32 i = 0; i < ddbi->listOfInterfaceDescriptors.ListSize(); i++){
                DDBInterfaceDescriptor *desc = (DDBInterfaceDescriptor *)item;
                errorDescription += ", used by ";
                errorDescription += desc->OwnerName();
                errorDescription += " with interface ";
                errorDescription += desc->InterfaceName();
                item = item->Next();
            }

            errorDescription += " reports error: ";
            if (status.CheckMask(DDB_WriteConflictError))
                errorDescription += " writeConflictError.";
            if (status.CheckMask(DDB_WriteExclusiveViolation))
                errorDescription += " writeExclusiveViolation.";
            if (status.CheckMask(DDB_MissingSourceError))
                errorDescription += " missingSourceError.";
            if (status.CheckMask(DDB_ConsecutivityViolation))
                errorDescription += " consecutivityViolation.";
            if (status.CheckMask(DDB_SizeMismatch))
                errorDescription += " sizeMismatch.";
            if (status.CheckMask(DDB_UndefinedSize))
                errorDescription += " undefinedSize.";
            if (status.CheckMask(DDB_TypeMismatch))
                errorDescription += " typeMismatch.";
            if (status.CheckMask(DDB_UndefinedType))
                errorDescription += " undefinedType.";

            CStaticAssertErrorCondition(FatalError,errorDescription.Buffer());
            checkResult=checkResult && False;
        }
    }

    /** Reset of the check iterator. */
    void Reset(){
        checkResult=True;
    }

};

/** Class implementing the search of a DDBItem by name. */
class DDBItemSearchFilter : public SearchFilter{
private:

    /** The string to search. */
    const char* name;

public:

    /** Constructor. */
    DDBItemSearchFilter(const char* name){
        this->name=strdup(name);
    }

    /** Destructor. */
    virtual ~DDBItemSearchFilter(){
        if (name)
            free((void*&)name);
    }

    /** Function performing the test. */
    bool Test(LinkedListable* ll){
        DDBItem* ddbi=dynamic_cast<DDBItem*>(ll);
        if (!ddbi){
            return False;
        }
        const char *signalName = ddbi->SignalName();
        int fullSize  = strlen(signalName);
        int shortSize = strlen(name);

        // Test first with the full name
        int sizeDifference = fullSize - shortSize;

        //  Cannot be longer
        if (sizeDifference >= 0) {
            // If there is fullName length == shortName length....
            if (sizeDifference == 0){
                if (strcmp(signalName,name)==0) return True;
            }
            signalName                += sizeDifference;
            const char  *signalNameDot = signalName - 1;
            if ((strcmp(signalName,name)==0) && (signalNameDot[0] == '.')) return True;
        }

        return False;
    }
};

#endif
