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
 * @brief Generic Application Module (GAM) interface class definition.
 * 
 * GAMs are the core of real-time systems built with BaseLib2 (e.g. MARTe).
 * GAMs interchange information using the DDB.
 */
#if !defined (GAM_H)
#define GAM_H

#include "GCReferenceContainer.h"
#include "LinkedListHolder.h"
#include "CDB.h"
#include "DDBDefinitions.h"
#include "MenuContainer.h"

class DDBIOInterface;
class DDBInputInterface;
class DDBOutputInterface;

/** Parameter specifying how the Execute method has to behave. */
enum GAM_FunctionNumbers{

/** Called once before going online. */
GAMPrepulse  =  0x00000001,

/** Called continuously while offline. */
GAMOffline   =  0x00000002,

/** Called continuously after a fault has been detected. */
GAMSafety    =  0x00000003,

/** Called once to verify if the parameter configuration is acceptable ???. */
GAMCheck     =  0x00000004,

/** Called once before going online. */
GAMPostpulse =  0x00000005,

/** Called once before going online. */
GAMStartUp   =  0x00000006,

/** Called continuously when online after data has been read. */
GAMOnline    =  0x00010000,

/** Called continuously when online after data has been written (but before writing the JPF).
    User defined functions (GAMOnline3, ... etc. ) starts from this offset. */
GAMOnline2   =  0x00010001,

};

/** class GAM; */
class GAM;

extern "C"{
    /** Exported method to insert an input interface to a gam. */
    bool GAMAddInputInterface(GAM& gam, const char* interfaceName,
                        DDBInputInterface *&ddbi);

    /** Exported method to insert an output interface to a gam. */
    bool GAMAddOutputInterface(GAM& gam, const char* interfaceName,
                        DDBOutputInterface *&ddbo, DDBInterfaceAccessMode accessMode);

    /** Exported method to insert an input/output interface to a gam. */
    bool GAMAddIOInterface(GAM& gam, const char* interfaceName,
                        DDBIOInterface *&ddbio,    DDBInterfaceAccessMode accessMode);

    /** Exported method to rename some or all the signal contained in the gam interfaces . */
    bool GAMRemapInterfaces(GAM& gam, ConfigurationDataBase& cdbData);

    /** Exported method to finalise the gam interface. */
    bool GAMFinaliseInterfaces(GAM& gam);

    /** Implements the Menu Interface for the GAM */
    bool GAMMenuInterface(StreamInterface &in,StreamInterface &out,void *userData);

    /** Export the GetPersistentGAMCDB function*/
    bool GAMGetPersistentCDB(GAM& gam, ConfigurationDataBase &cdb);

    /*Update the GAMPersistentCDB with the requested values*/
    bool GAMUpdatePersistentCDB(GAM &gam, ConfigurationDataBase &cdb);
}


OBJECT_DLL(GAM)

/** 
 * The interface of a GAM.
 */
class GAM: public GCReferenceContainer{

    friend bool GAMAddInputInterface(GAM& gam, const char* interfaceName, DDBInputInterface *&ddbi);

    friend bool GAMAddOutputInterface(GAM& gam, const char* interfaceName, DDBOutputInterface *&ddbo, DDBInterfaceAccessMode accessMode);

    friend bool GAMAddIOInterface(GAM& gam, const char* interfaceName, DDBIOInterface *&ddbio,   DDBInterfaceAccessMode accessMode);

    friend bool GAMFinaliseInterfaces(GAM& gam);

    friend bool GAMRemapInterfaces(GAM& gam, ConfigurationDataBase& cdbData);

    friend bool GAMMenuInterface(StreamInterface &in,StreamInterface &out,void *userData);

    friend bool GAMGetPersistentGAMCDB(GAM& gam, ConfigurationDataBase &cdb);

    friend bool GAMUpdatePersistentGAMCDB(GAM &gam, ConfigurationDataBase &cdb);

protected:

    /** Menu Interface for the GAM. It is initialized after
        the FinaliseInterface function is called.
     */
    GCRTemplate<MenuContainer>        menu;


private:

    /** The container of interfaces with the ddb. */
    LinkedListHolder      listOfDDBInterfaces;

    /** */
    bool                  finalisedInterfaces;

    /** It finalise all DDBInterfaces in listOfDDBInterfaces.
        It parses all interfaces, sets each interface in a state that
        does not allow adding or modifying signals, and allocates the
        memory for storing the signals.
        @return True if everything was fine. False otherwise.
    */
    bool FinaliseInterfaces(){
        return  GAMFinaliseInterfaces(*this);
    };

    /** Renames some or all signals in the ddb interfaces to the value
        specified in the data base.
        The code checks for the presence of a field "Remappings". If this field is
        found, the code checks in this subtree for an entry with the same name of one of the
        interfaces. If the interface name is found, the code parses all the fields
        in the subtree. The name of the entry has to be the name of a signal contained
        in the interface, the value associated to the entry name is the new name that
        is used for the signal.
        Example:
        Remappings = {
            InputInterface = {
                OldSignalName = NewSignalName
                ...
                ...
            }
            OutputInterface = {
                OldSignalName = NewSignalName
                ...
                ...
            }
        }

        @param  cdbData The data base containing the optional field Remappings.
        @return True if everything was fine. False otherwise.
     */
    bool RemapInterfaces(ConfigurationDataBase& cdbData){
        return  GAMRemapInterfaces(*this,cdbData);
    }


protected:

    /** This function is called by the executor during the ObjectLoadSetup process.
        It initialises the gam parameters and add interfaces to the list.
        @param cdbData The data base containing the needed informations
        @return True if the initialisation was successful. False otherwise.
    */
    virtual bool Initialise(ConfigurationDataBase& cdbData)=0;

public:

    /** This method allows the user to add a DDBInputInterface in the list.
        The interface created will have only the DDB_ReadMode access and
        this cannot be changed.
        @param  ddbi the pointer to the new interface that has been added to the list
        @param  interfaceName the name of the interface. The name will be used to search
                the list for the interface.
        @return True if the interface was successfully added. False otherwise.
    */
    inline  bool AddInputInterface(DDBInputInterface *&ddbi, const char* interfaceName){
        return GAMAddInputInterface(*this,interfaceName, ddbi);
    }

    /** This method allows the user to add a DDBOutputInterface in the list.
        By default the interface has only the DDB_WriteMode access. Anyway
        other access modes can be used with the exception of the read mode.
        The DDB_ReadMode bit is always cleared before construction.
        @param  ddbo the pointer to the new interface that has been added to the list
        @param  interfaceName the name of the interface. The name will be used to search
                the list for the interface.
        @param  accessMode the flag specifying the writing access mode for the signals
                contained in the interface.
        @return True if the interface was successfully added. False otherwise.

    */
    inline bool AddOutputInterface(DDBOutputInterface *&ddbo, const char* interfaceName,
                                   DDBInterfaceAccessMode accessMode=DDB_WriteMode){
        return GAMAddOutputInterface(*this, interfaceName, ddbo, accessMode);
    }

    /** This method allows the user to add a DDBIOInterface in the list.
        By default the interface has the DDB_ReadMode and DDB_WriteMode
        access. All the other access modes can freely be used.
        @param  ddbio the pointer to the new interface that has been added to the list
        @param  interfaceName the name of the interface. The name will be used to search
                the list for the interface.
        @param  accessMode the flag specifying the writing and reading access mode for
                the signals contained in the interface.
        @return True if the interface was successfully added. False otherwise.
    */
    inline bool AddIOInterface(DDBIOInterface *&ddbio,        const char* interfaceName,
                        DDBInterfaceAccessMode accessMode=DDB_ReadMode|DDB_WriteMode){
        return GAMAddIOInterface(*this, interfaceName, ddbio, accessMode);
    }

public:

    /** */
    GAM(){
        finalisedInterfaces = False;
    }

    /** Get GAM Name.
        @return The name of the gam.
    */
    const char* GamName() const{return  Name();}

    /** Get the list of interfaces used by the GAM.
        @return the pointer to the list of interfaces associated with the gam.
    */
    LinkedListable *InterfacesList()const {return listOfDDBInterfaces.List();}

    /** Destructor. */
    virtual ~GAM(){}

    /** This function is called by the executor when appropriate. The user can (has to)
        take different actions depending on the value of the parameter passed.
        @param  functionNumber The execution flag (GAMOnline, GAMOffline ...).
        @return True if everything went ok. False otherwise. Returning False during GAMOnline
                will cause the real time system to switch to a safety state.
    */
    virtual bool Execute(GAM_FunctionNumbers functionNumber)=0;

    /** This function is the one actually called by the executor during
        the initialisation phase.
        @param   info the data base containing the initialisation parameters for the gam
        @param   err  the stream to output error messages, warning and information
        @return  True if everything went fine. False otherwise.
    */
    bool ObjectLoadSetup(ConfigurationDataBase &info,StreamInterface *err){

        CDBExtended cdb(info);

        finalisedInterfaces = False;

        // Set the Gam Name from the CDB Node
        if (!GCReferenceContainer::ObjectLoadSetup(info,err)){
            AssertErrorCondition(FatalError,"ObjectLoadSetup:Cannot retrieve name of node");
            return False;
        }

        // Create The Menu Container
        GCRTemplate<MenuContainer> xMenu(GCFT_Create);
        menu = xMenu;

        /* Contruct the Menu Interface */
        menu->CleanUp();
        menu->SetTitle(Name());

        // Initialize interfaces
        if (!Initialise(info)) {
            AssertErrorCondition(InitialisationError,"ObjectLoadSetup::Initialise Failed for GAM %s", Name());
            return False;
        }
        // Rename signals, if needed
        if (!RemapInterfaces(info)) {
            AssertErrorCondition(Warning,"ObjectLoadSetup::RemapInterfaces Failed for GAM %s", Name());
        }
        // Finalize interfaces.
        if (!FinaliseInterfaces()) {
            AssertErrorCondition(InitialisationError,"ObjectLoadSetup::FinaliseInterfaces Failed for GAM %s", Name());
            return False;
        }

        AssertErrorCondition(Information,"ObjectLoadSetup::Successfully loaded GAM %s", Name());
        return True;
    }

    /** This function saves the GAM parameters into the specified CDB.
        @param   ConfigurationDataBase where to save the GAM parameters
        @param   Error Stream Interface
        @return  True if everything went well. False otherwise.
    */
    bool ObjectSaveSetup(ConfigurationDataBase &info,StreamInterface *err){
        return True;
    };


    /** Implements the User Menu Interface*/
    virtual bool MenuInterface(StreamInterface &in,StreamInterface &out,void *userData){
        return True;
    }

    /** Returns the Menu Container Class */
    GCRTemplate<MenuContainer> Menu(){return menu;}

    /**
    * Gets a configuration database which can be used to store any values that
    * will be maintained also when the GAM is reconfigured. If the CDB doesn't exist it
    * is created. The CDB will be named GAMPersistentCDB
    *
    */
    bool GetGAMPersistentCDB(ConfigurationDataBase &cdb){
        return GAMGetPersistentCDB(*this, cdb);
    }

    /*Update the GAMPersistentCDB with the requested values*/
    bool UpdateGAMPersistentCDB(ConfigurationDataBase &cdb){
        return GAMUpdatePersistentCDB(*this, cdb);
    }

    OBJECT_DLL_STUFF(GAM)

};


#endif
