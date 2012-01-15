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
 * Definition of the class DDBSignalDescriptor.
 */
#if !defined (DDBSIGNALDESCRIPTOR_H)
#define DDBSIGNALDESCRIPTOR_H

#include "Object.h"
#include "FString.h"
#include "LinkedListable.h"
#include "DDBDefinitions.h"
#include "BasicTypes.h"

class DDBSignalDescriptor;
extern "C"{
    void DDBSD(DDBSignalDescriptor *ddbsd, 
                const char*                 signalName,
                const int                   signalSize,
                const int                   from,
                const BasicTypeDescriptor   typeCode,
                const DDBStoringProperties& ddbss);
    
    void DDBSD2(DDBSignalDescriptor *ddbsd, const DDBSignalDescriptor& descriptor);
    bool DDBSDBrowse(StreamInterface &in,StreamInterface &out,void *userData);
};

/** 
 * This class is used to represent a signal. 
 */
OBJECT_DLL(DDBSignalDescriptor)
class DDBSignalDescriptor:  public Object, public LinkedListable{

    OBJECT_DLL_STUFF(DDBSignalDescriptor)
friend void DDBSD(DDBSignalDescriptor *ddbsd, 
                const char*                 signalName,
                const int                   signalSize,
                const int                   from,
                const BasicTypeDescriptor   typeCode,
                const DDBStoringProperties& ddbss);
friend void DDBSD2(DDBSignalDescriptor *ddbsd, const DDBSignalDescriptor& descriptor);    
            
friend bool DDBSDBrowse(StreamInterface &in,StreamInterface &out,void *userData);

private:

    /** The name of the signal. */
    FString               signalName;

    /** A field coding the type and size of the elementary type used to build this signal. */
    BasicTypeDescriptor   typeCode;

    /** Signal Size which is the actual signal size if the 'from' field is -1.
        If the 'from' field is different from -1, signal size specifies the number
        of consecutive elements to be read. */
    int32                 signalSize;

    /** If -1 it means that the whole signal is to be read. Otherwise it specifies the starting
        element from which to start reading. */
    int32                 from;

    /** Specifies the storing properties of the signal. I.E. DDB_FlatNamed, DDB_Consecutive, DDB_Unsized.*/
    DDBStoringProperties  storingProperties;

    /** The standard contructor. It has been overridden to avoid silent use. */
    DDBSignalDescriptor():typeCode(BTDUint32){}

public:

    /** Constructor for partially sized signals. Note that this constructor
        set the DDB_Unsized bit overriding the setting made by the user. */
    inline DDBSignalDescriptor(const char*                 signalName,
                        const int                   signalSize,
                        const int                   from,
                        const BasicTypeDescriptor   typeCode,
                        const DDBStoringProperties& ddbss){
        DDBSD(this, signalName, signalSize, from, typeCode, ddbss);
    }

    /** The copy contructor. */
    inline DDBSignalDescriptor(const DDBSignalDescriptor& descriptor){
        DDBSD2(this, descriptor);
    }

    /** Destructor. It just frees the memory allocated during construction. */
    virtual ~DDBSignalDescriptor(){};

    /** DDBSignalDescriptor save function. Uses a CDB to pass the initialisation parameters.
        The CDB information is read from the subtree that is currently addressed.
        For binary save/loads put the binaryy content in root.binary    */
    virtual bool ObjectSaveSetup(ConfigurationDataBase &info,StreamInterface *err) const;

    /** DDBSignalDescriptor load function. Uses a CDB to pass the initialisation parameters.
        The CDB information is read from the subtree that is currently addressed.
        For binary save/loads put the binaryy content in root.binary    */
//    virtual bool ObjectLoadSetup(ConfigurationDataBase &info,StreamInterface *err);

public:

    /////////////////////
    // GET SIGNAL INFO //
    /////////////////////

    /** Gets the name of the signal.
        @returns A pointer to signalName. */
    const char* SignalName() const {
        return signalName.Buffer();
    }

    /** Gets the number of elements in the signal. To Get the size in byte of the signal
        the user has to specify --> SignalSize()*TypeSize(SignalTypeCode());
        @returns maxDimensions. */
    int32 SignalSize() const{
        return signalSize;
    }

    /** Gets the starting index of the signal.
        @returns from. */
    int32 SignalFromIndex() const{
        if(from == -1)return 0;
        else          return from;
    }

    /** Gets the type code of the signal.
        @returns The typeCode. */
    BasicTypeDescriptor SignalTypeCode() const {
        return typeCode;
    }

    /** Gets the storing properties of the signal.
        @returns storingProperties. */
    DDBStoringProperties SignalStoringProperties() const {
        return storingProperties;
    }

    /////////////////////
    // SET SIGNAL INFO //
    /////////////////////

    /** Gets the name of the signal.
        @returns A pointer to signalName. */
    void SignalRename(const char* newSignalName){
        signalName=newSignalName;
    }

    /** Sets the number of maxDimensions of the signal. */
    void SetSignalDimensions(const int signalSize){
        this->signalSize = signalSize;
    }

    ///////////////////////
    // UTILITY FUNCTIONS //
    ///////////////////////

    /** Returns the signal name with added informations.
        If the signal is unsized it returns the element/s to be used withing round brackets.
        If the signal is sized it returns the signal name with the dimension within square brackets.
    */
    bool GetCompoundName(FString &compoundName){
        if(storingProperties.CheckMask(DDB_Unsized)){
            if(signalSize == 1)compoundName.Printf("%s(%d)",signalName.Buffer(),from);
            else               compoundName.Printf("%s(%d:%d)",signalName.Buffer(),from,from+signalSize);
        }else{
            compoundName.Printf("%s[%d]",signalName.Buffer(),signalSize);
        }
        return True;
    }

    DDBSignalDescriptor *Next() const {
        return (DDBSignalDescriptor *)LinkedListable::Next();
    }

    /** Print method. */
    void Print(StreamInterface& s)const;

};

/** This is the iterator for a list of DDBSignalDescriptor. */
class ShowSignalDescriptorIterator: public Iterator{
private:
    /** The stream uset to perform the print operation. */
    StreamInterface& s;
public:

    /** The constructor initialises the stream as specified. */
    ShowSignalDescriptorIterator(StreamInterface& si): s(si){}

    virtual ~ShowSignalDescriptorIterator(){};

    /** The print operation. */
    void Do(LinkedListable* ll){
        DDBSignalDescriptor* ddbsd=dynamic_cast<DDBSignalDescriptor*>(ll);
        if (ddbsd){
            ddbsd->Print(s);
            s.Printf("\n");
        }
    }
};


/** The search filter used to find the signalDescriptor by name in the listOfSignalDescriptors. */
class DDBSignalSearchFilter : public SearchFilter{
private:

    /** The signalName to be searched. */
    const char* name;

public:

    /** Constructor. */
    DDBSignalSearchFilter(const char* name){
        this->name=strdup(name);
    }

    /** Destructor. */
    virtual ~DDBSignalSearchFilter(){
        if (name)
            free((void*&)name);
    }

    /** The function implementing the test. */
    bool Test(LinkedListable* ll){
        DDBSignalDescriptor* ddbsd=dynamic_cast<DDBSignalDescriptor*>(ll);
        if (ddbsd == NULL){
            return False;
        }
        return (strcmp(name,ddbsd->SignalName())==0);
    }
};

#endif
