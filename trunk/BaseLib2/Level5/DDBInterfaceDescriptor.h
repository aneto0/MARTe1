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
 * Definition of the class DDBInterfaceDescriptor.
 */
#if !defined (DDBINTERFACEDESCRIPTOR_H)
#define DDBINTERFACEDESCRIPTOR_H

#include "Object.h"
#include "LinkedListable.h"
#include "Streamable.h"
#include "FString.h"
#include "DDBDefinitions.h"

class DDBInterfaceDescriptor;

extern "C" {
    bool DDBIDObjectLoadSetup(DDBInterfaceDescriptor&     ddbid,
                              ConfigurationDataBase&      info,
                              StreamInterface *err);
}

/** 
 * Implements a user of the DDB.
 */
OBJECT_DLL(DDBInterfaceDescriptor)
class DDBInterfaceDescriptor: public LinkedListable, public Object{
OBJECT_DLL_STUFF(DDBInterfaceDescriptor)


    friend bool DDBIDObjectLoadSetup(DDBInterfaceDescriptor&     ddbid,
                                     ConfigurationDataBase&      info,
                                     StreamInterface *err);

private:

    /** Name of the signal user (GAM). In this context user means both gams for which the signal
        is an input and those for which the signal is an output. */
    FString                    ownerName;

    FString                    interfaceName;

    /** Specifies the Interface access mode to the signals*/
    DDBInterfaceAccessMode     accessMode;

    /** Standard constructor is disabled. */
    DDBInterfaceDescriptor(){}

public:

    /** Constructor. This constructor is the only one that can be safely called
     in the constructor of a derived class to initialise the parent. */
    DDBInterfaceDescriptor(
        const char*                  ownerName,
        const char*                  interfaceName,
        const DDBInterfaceAccessMode accessMode):accessMode(accessMode){

        if ((ownerName==NULL) || (strlen(ownerName)==0)){
            this->ownerName="Unknown";
            AssertErrorCondition(ParametersError,"DDBInterfaceDescriptor::DDBInterfaceDescriptor(): The name of the owner of this interface has not been specified.");
        }else{
            this->ownerName=ownerName;
        }

        if ((interfaceName==NULL) || (strlen(interfaceName)==0)){
            this->interfaceName="Unspecified";
            AssertErrorCondition(ParametersError,"DDBInterfaceDescriptor::DDBInterfaceDescriptor(): The name of the interface has not been specified.");
        }else{
            this->interfaceName=interfaceName;
        }
    }


    /** Copy constructor. It is worth noticing that the copy constructor is the only
        made public. With this trick it is possible to create a new DDBInterfaceDescriptor
        only copying it from a derived class. */
    DDBInterfaceDescriptor(const DDBInterfaceDescriptor& descriptor);

    /** Destructor. */
    virtual ~DDBInterfaceDescriptor(){};

public:

    /** The name of the owner of the interface. */
    const char* OwnerName() const{
        return ownerName.Buffer();
    }

    /** The name of the interface itself. */
    const char* InterfaceName() const{
        return interfaceName.Buffer();
    }

    /** */
    const DDBInterfaceAccessMode& AccessMode() const{
        return accessMode;
    }

    /** Print the class as a whole. */
    void Print(StreamInterface& s) const;

    /** */
    bool ObjectSaveSetup(ConfigurationDataBase &info, StreamInterface *err){
        return DDBIDObjectLoadSetup(*this, info, err);
    };

};

/** Implements the Object Save Setup for all members of the list */
class InterfaceDescriptorShowIterator : public Iterator{
private:
    ConfigurationDataBase& cdb;
public:

    InterfaceDescriptorShowIterator(ConfigurationDataBase& cdbi):cdb(cdbi){}

    void Do(LinkedListable* ll){
        DDBInterfaceDescriptor* ddbid =
            dynamic_cast<DDBInterfaceDescriptor*>(ll);
        if (ddbid){
            ddbid->ObjectSaveSetup(cdb,NULL);
        }
    }
};

/** Ancillary class used to serch a DDBInterfaceDescriptor in a List. */
class DDBInterfaceDescriptorSearchFilter : public SearchFilter{
private:

    /** Id to look for. */
    FString combinedName;

public:

    /** Constructor. */
    DDBInterfaceDescriptorSearchFilter(const char *gamName, const char *interfaceName){
        combinedName.Printf("%s::%s",gamName,interfaceName);
    }
    
    virtual ~DDBInterfaceDescriptorSearchFilter(){};

    /** Implementation of the virtual method (see SearchFilter). */
    bool Test(LinkedListable* ll){
        DDBInterfaceDescriptor* ddbid=dynamic_cast<DDBInterfaceDescriptor*>(ll);
        if (!ddbid){
            return False;
        }
        FString temp;
        temp.Printf("%s::%s",ddbid->OwnerName(),ddbid->InterfaceName());

        return (combinedName == temp);
    }
};

#endif
