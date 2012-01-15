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
 * DDBInterface class definition.
 */

#if !defined (DDBINTERFACE_H)
#define DDBINTERFACE_H

#include "Object.h"
#include "FString.h"
#include "DDBInterfaceDescriptor.h"
#include "DDBSignalDescriptor.h"
#include "LinkedListable.h"
#include "BasicTypes.h"

class ConfigurationDataBase;
class LinkedListHolder;
class DDBInterface;

extern "C"{
    /** Used by AddSignal to add a structured type. */
    bool DDBIAddSignal(
                 DDBInterface&         ddbi,
                 const char*           signalName,
                 const char*           signalType,
                 DDBStoringProperties  properties);

    bool DDBIObjectLoadSetup(DDBInterface&         ddbi,
                       ConfigurationDataBase&      info,
                       StreamInterface *err);

    bool DDBIObjectSaveSetup(DDBInterface&         ddbi,
                       ConfigurationDataBase&      info,
                       StreamInterface *err);

    bool DDBIFinalise(DDBInterface&         ddbi);

    const DDBSignalDescriptor* DDBIFind(const DDBInterface& ddbi,
                                        const char* signalName);

    const DDBSignalDescriptor* DDBISignalsList(const DDBInterface& ddbi);

    bool  DDBISignalRename(DDBInterface& ddbi,
                           const char* signalName,
                           const char* newSignalName);

    int32 DDBIBufferWordSize(const DDBInterface& ddbi);

    /* Implement the Menu Interface */
    bool  DDBIBrowseMenu(StreamInterface &in,StreamInterface &out,void *userData);

    bool  DDBIDumpMenu(StreamInterface &in,StreamInterface &out,void *userData);
}

/** DataBufferPointer containes the int32 pointer to the memory where
    the signal is stored. It contains also the signal size, where the size
    is the number of int32 elements that are to be read.
 */

class  DataBufferPointer{

private:
    // Pointer to the Data Buffer
    int32     *dataBufferPointer;

    // Number of consecutive signals to read
    int32     signalSize;
public:

    DataBufferPointer(){
        dataBufferPointer = NULL;
        signalSize        = 0;
    }

    DataBufferPointer(int32 *dataBufferPointer, int32 signalSize){
        SetSignalParameters(dataBufferPointer,signalSize);
    }

    void SetSignalParameters(int32 *dataBufferPointer, int32 signalSize){
        this->dataBufferPointer = dataBufferPointer;
        this->signalSize        = signalSize;
    }

    int32* GetPointer()const{
        return dataBufferPointer;
    }

    int32 GetSize()const{
        return signalSize;
    }
};

/** The ddbInterface used by GAM. The ddbInterface is build around the access
    method is being used to collect data. Signals with the same access mode
    should be grouped and accessed in a unique operation for efficiency.
 */

OBJECT_DLL(DDBInterface)
class DDBInterface: public Object, public LinkedListable{

    friend class DDB;
    friend class GAM;

    friend bool DDBIAddSignal(DDBInterface&         ddbi,
                              const char*           signalName,
                              const char*           signalType,
                              DDBStoringProperties  properties);

    friend bool DDBIObjectLoadSetup(DDBInterface&         ddbi,
                              ConfigurationDataBase&      info,
                              StreamInterface *err);

    friend bool DDBIObjectSaveSetup(DDBInterface&         ddbi,
                       ConfigurationDataBase&      info,
                       StreamInterface *err);

    friend bool DDBIFinalise(DDBInterface&         ddbi);

    friend const DDBSignalDescriptor* DDBIFind(const DDBInterface&  ddbi,
                                               const char* signalName);

    friend const DDBSignalDescriptor* DDBISignalsList(const DDBInterface& ddbi);

    friend bool  DDBISignalRename(DDBInterface& ddbi,
                                  const char* signalName,
                                  const char* newSignalName);

    friend int32 DDBIBufferWordSize(const DDBInterface& ddbi);


    friend bool  DDBIBrowseMenu(StreamInterface &in,StreamInterface &out,void *userData);
    friend bool  DDBIDumpMenu  (StreamInterface &in,StreamInterface &out,void *userData);

    OBJECT_DLL_STUFF(DDBInterface)

private:

    /** Describes the properties of the interface, i.e. The interface Name, the GAM owner,
        and the access mode to the signals in the DDB */
    DDBInterfaceDescriptor        ddbInterfaceDescriptor;

    /** A list of SignalDescriptor. This list should be made by all the signals accessed
        by the user with the specified access mode. */
    LinkedListHolder              listOfSignalDescriptors;

private:

    /** Array of pointers to specific DDB signals. */
    DataBufferPointer            *ddbSignalPointers;

    /** A pointer to the working buffer of this interface. */
    int32*                        buffer;

    /** Buffer Word Size */
    int32                         bufferWordSize;

    /** This method reset the pointer list. It should only be called by
        a DDB class to allow reinitialization. */
    bool ResetPointerList();

    /** This function should be called by the DDB after having added the interface.
        It looks into the ddbSignalPointers array for consecutive elements. If consecutive
        elements are found, only one pointer with an increased number of elements is used.
        @return True if everything goes well. False if the interface has not been finalized.
    */
    bool CheckConsecutyAndOptimizeInterface();

private:

    /** A status from where the interface can be linked to the DDB. */
    bool                          finalised;

private:

    /** It checks if the signal name has the interval extension.
        If the signals name is specified with square brackets the
        function assumes that the number specified within brackets is
        the signals size (i.e. A[5] is a signal of name A of size 5).
        If the signal is specified with round brackets the function assumes
        that the numbers specify the elements that are to be used. Valid
        sintax is A(4), A(1,3,4,6), A(4:6), A(2:4,7:9). If the user specifies
        the signal with round brackets the storing properties of the signal are
        set to DDB_Unsized. Only interface with read access can use the
        range specifications.
    */
    bool ParseRawNameAndAdd(const char* rawName, const char *signalType, DDBStoringProperties& properties);

   /** Used to initialise the pointers to the DDB buffer. This function has
        to be called from the DDB who is the only one entity that can assign
        meaningful pointers and sizes.
        @param  signalName the name of the signal to link.
        @param  ddbSignalPointer The pointer of the signal in the DDB.
        @return True if Link has been successfull. False otherwise.
    */
    bool Link(const char* signalName, int32* ddbSignalPointer);


    /** Finds by name a signal in the listOfSignalDescriptors starting from index element in the list.
        @param signalName The name of the signal.
        @param index      The list position from which to start the search.
        @return           The descriptor which matches the signalName starting from element index in the list
     */
    DDBSignalDescriptor* Find(const char* signalName, uint32 &index);

    /** The method adds structured signals to this ddbInterface.
        @param signalName Raw name of the signal.
        @param signalType Specifies what is the name of the structure/class or element type.
        @param signalSize Number of consecutive elements to be read.
        @param from       Signal offset. The routine will read/write 'signalSize' elements from the position 'from'
        @param properties Specify the storing properties of the signal.
        @return           True if the signal has been added to the list. False otherwise.
    */
    bool AddSignalRecursive(const char*           signalName,
                            const char*           signalType,
                            int                   signalSize,
                            int                   from,
                            DDBStoringProperties& properties);


    /** The method adds basic type signals to this ddbInterface.
        @param signalName Raw name of the signal.
        @param signalSize Number of consecutive elements to be read.
        @param from       Signal offset. The routine will read/write 'signalSize' elements from the position 'from'
        @param typeCode   A variable specifying the conventions used to store the variable.
        @param properties Specifies the storing properties of the signal.
        @return           True if the signal has been added to the list. False otherwise.
    */
    bool AddUnstructuredSignal(const char*                   signalName,
                               const int                     signalSize,
                               const int                     from,
                               const BasicTypeDescriptor     typeCode,
                               DDBStoringProperties&         properties);
public:

    /** Constructor.
        @param ownerName            Name of the GAM that owns the Interface
        @param interfaceName        Name of the Interface
        @param requestedAccessMode  Specifies the access mode of the interface to the DDB

    */
    DDBInterface(const char* ownerName, const char* interfaceName,
                 DDBInterfaceAccessMode requestedAccessMode) :ddbInterfaceDescriptor(ownerName, interfaceName, requestedAccessMode){
        ddbSignalPointers      = NULL;
        buffer                 = NULL;
        bufferWordSize         = 0;
        finalised              = False;
   }

    /** Destructor. */
    virtual ~DDBInterface(){
        if (ddbSignalPointers!=NULL){
            delete[] ddbSignalPointers;
        }
        if(buffer != NULL){
            free((void*&)buffer);
        }
    };


    /** Used by AddSignal to add a structured type.
        @param signalName    Name of the signal.
        @param signalType    Type of the signal.
        @param properties    Specifies the storing properties of the signal
    */
    bool AddSignal( const char*           signalName,
                    const char*           signalType,
                    DDBStoringProperties  properties=DDB_Default){
        return DDBIAddSignal(*this,signalName,signalType,properties);
    }

    /** Allocates memory for the DDB read and/or write operations and put the
        interface in a status which stops accepting new signals.
    */
    bool Finalise(){return DDBIFinalise(*this);};

    /** Renames all occurences of signal 'signalName' in the SignalDescriptor List with the
        name 'newSignalName.
        @param signalName       The name of the signal in the ddb interface
        @param newSignalName    The new name for the signal
        @return                 True if at least one occurrence of signal 'signalName' has been found. False otherwise.
    */
    bool SignalRename(const char* signalName, const char* newSignalName){
        return DDBISignalRename(*this,signalName,newSignalName);
    }
public:


    /** Saves the DDInterface to CDB */
    bool ObjectSaveSetup(ConfigurationDataBase &info, StreamInterface *err){
        return DDBIObjectSaveSetup(*this, info, err);
    }

    /** Initialises the Interface */
    bool ObjectLoadSetup(ConfigurationDataBase &info, StreamInterface *err){
        return DDBIObjectLoadSetup(*this, info,err);
    }

public:

    /** Returns the Interface Descriptor */
    const DDBInterfaceDescriptor& GetInterfaceDescriptor() const{return ddbInterfaceDescriptor;}

    /** Return the Interface Name */
    const char *InterfaceName() const{
        return ddbInterfaceDescriptor.InterfaceName();
    }

    /** Returns the first element in the listOfSignalDescriptors. */
    //const DDBSignalDescriptor* SignalsList() const{return dynamic_cast<DDBSignalDescriptor*>(listOfSignalDescriptors.List());}
    const DDBSignalDescriptor* SignalsList() const{
        return DDBISignalsList(*this);
    }


public:

    /** Print method. */
    void Print(StreamInterface& s) const;

    /** Check if the DDB is finalised.
        @return True if finalised. False otherwise.
    */
    bool IsFinalised() const{ return finalised;}

    /** Returns the Interface Buffer */
    int32 *Buffer(){return buffer;}

    /** Returns the number of float/int32 signals that are stored in the buffer.
        If this method is called before the interface has been finalised it
        returns the buffer size of the added signal list. Otherwise it returns
        the buffer size.
        The returned value is >= NumberOfEntries().
        @return the size in word of the signal that are present in the list.
    */
    int32 BufferWordSize() const{
        return DDBIBufferWordSize(*this);
    }

    /** Returns a pointer the DDBSignalDescriptor of a signal identified
        by name in the DDBInterface.
        @return DDBSignalDescriptor*
    */
    const DDBSignalDescriptor* Find(const char* signalName) const{
        return DDBIFind(*this, signalName);
    };


    /** Returns the number of entries in the DDBInterface list.
        This can be different from the BufferWordSize if any of
        the signal in the List has a size larger than one.
        @return listOfSignalDescriptors.ListSize()
    */
    int32 NumberOfEntries() const{
        return listOfSignalDescriptors.ListSize();
    };

protected:

    /** Fast function to read the databuffer. */
    inline void Read(){
        if (buffer == NULL) return;

        int32* inputBufferIterator       = buffer;

        for(uint32 i = 0; (i < listOfSignalDescriptors.ListSize()) && (ddbSignalPointers[i].GetPointer() != NULL); i++){
            const int32* innerPointersListIterator = ddbSignalPointers[i].GetPointer();
            const int32* innerPointersListEnd      = innerPointersListIterator + ddbSignalPointers[i].GetSize();
            while (innerPointersListIterator < innerPointersListEnd){
                *inputBufferIterator++ = *innerPointersListIterator++;
            }
        }
    }

    /** Fast function to write the databuffer. */
    inline void  Write(){
        if (buffer == NULL) return;

        int32* inputBufferIterator   = buffer;

        for(uint32 i = 0; (i < listOfSignalDescriptors.ListSize()) && (ddbSignalPointers[i].GetPointer() != NULL); i++){
            int32* innerPointersListIterator = ddbSignalPointers[i].GetPointer();
            int32* innerPointersListEnd      = innerPointersListIterator + ddbSignalPointers[i].GetSize();
            while (innerPointersListIterator < innerPointersListEnd){
                *innerPointersListIterator++ = *inputBufferIterator++;
            }
        }
    }

};

#endif
