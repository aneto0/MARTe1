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
 * @brief Dynamic Data Buffer (DDB) class definition.
 *
 * The DDB enables the live interchange of information
 * between different modules.
 */
#if !defined (DDB_H)
#define DDB_H

#include "GCNamedObject.h"
#include "DDBDefinitions.h"
#include "DDBItem.h"
#include "DDBInterface.h"
#include "DDBSignalDescriptor.h"
#include "LinkedListable.h"
#include "LinkedListHolder.h"
#include "Streamable.h"
#include "GCRTemplate.h"

/** Predefinition of class GAM */
class GAM;

class DDB;

extern "C"{

    /** Exported function to creat a link between the ddb and the gam interfaces */
    bool DDBCreateLinkToGAM(DDB &ddb, GCRTemplate<GAM> gam);

    /** Exported function to add the signals contained in the gam interfaces to the ddb */
    bool DDBAddGAMInterface(DDB &ddb, GCRTemplate<GAM> gam);

    /** Exported function to creat a link between the ddb and one interfaces */
    bool DDBCreateLink(DDB &ddb, DDBInterface &ddbInterface);

    /** Exported function to add the signals contained in the interfaces to the ddb */
    bool DDBAddInterface(DDB &ddb, const DDBInterface &ddbInterface);

    /** Exported function to check correctness of the ddb signals and allocate the space to store the data */
    bool DDBCheckAndAllocate(DDB &ddb);

    /** Exported function to print informations about the ddb structure */
    void DDBPrint(DDB &ddb, Streamable& s);

    /** Exported function to implement the object load setup of the ddb */
    bool DDBObjectLoadSetup(DDB &ddb, ConfigurationDataBase &cdb, StreamInterface *err);

    /** Exported function to save the ddb informations into a data base */
    bool DDBObjectSaveSetup(DDB &ddb, ConfigurationDataBase &cdb, StreamInterface *err);

    /** Implements DDB Browse functionality for Menu*/
    bool DDBBrowseMenu(StreamInterface &in,StreamInterface &out,void *userData);

    /** Implements DDB Dump functionality for Menu*/
    bool DDBDumpMenu  (StreamInterface &in,StreamInterface &out,void *userData);

}

/** 
 * DDB class
 * The DDB provides a way to store the signals in memory and to share these amongst GAMs.
 */
OBJECT_DLL(DDB)

class DDB: public GCNamedObject{

    friend bool DDBCreateLinkToGAM(DDB &ddb, GCRTemplate<GAM> gam);

    friend bool DDBAddGAMInterface(DDB &ddb, GCRTemplate<GAM> gam);

    friend bool DDBCreateLink(DDB &ddb, DDBInterface &ddbInterface);

    friend bool DDBAddInterface(DDB &ddb, const DDBInterface &ddbInterface);

    friend bool DDBCheckAndAllocate(DDB &ddb);

    friend void DDBPrint(DDB &ddb, Streamable& s);

    friend bool DDBObjectLoadSetup(DDB &ddb, ConfigurationDataBase &cdb, StreamInterface *err);

    friend bool DDBObjectSaveSetup(DDB &ddb, ConfigurationDataBase &cdb, StreamInterface *err);

    friend bool DDBBrowseMenu(StreamInterface &in,StreamInterface &out,void *userData);

    friend bool DDBDumpMenu  (StreamInterface &in,StreamInterface &out,void *userData);

    OBJECT_DLL_STUFF(DDB)

private:

    /** List of signal descriptions (DDBItem). */
    LinkedListHolder    listOfDDBItems;

    /** The pointer to the buffer memory. */
    char*               buffer;

    /** The size in byte of the DDB buffer. */
    int32               totalDDBSize;

    /** Flag specifying if the DDB has been finalised.
        When the DDB is finalised it cannot accept new DDBItems.
        The CheckAndAllocate member sets this flag if the DDB
        is coherent.
    */
    bool                finalised;

    /** Finds a DDBItem by name in the listOfDDBItems.
        The name of the DDBItem is the name of the signal has passed to
        the GAM DDBInterface.

        @param The name of the signal to be found.
        @return A pointer to the DDBItem if it has been found, NULL otherwise.
     */
    DDBItem* Find(const char* name){
        DDBItemSearchFilter ddbisf(name);
        return dynamic_cast<DDBItem*>(listOfDDBItems.ListSearch(&ddbisf));
    }


    /** Private member that implements the AddInterface
        The reason of this member is to allow access to private members of
        the DDBInterface from the exported functions.
        @param ddbInterface The interface to be added.
        @return True If the inteface was added to the DDB. False otherwise.
    */
    bool __AddInterface(const DDBInterface& gamInterface);


    /** Private member that implements the CreateLink
        The reason of this member is to allow access to private members of
        the DDBInterface from the exported functions.
        @param ddbInterface The interface to be linked.
        @return True If the inteface was linked to the DDB. False otherwise.
    */
    bool __CreateLink(DDBInterface& gamInterface);

public:

    /** Constructor. */
    DDB(){
        buffer        = NULL;
        totalDDBSize  = 0;
        finalised     = False;
    }

    /** Destructor. */
    ~DDB(){
        if (buffer!=NULL) free((void*&)buffer);
    }

    /** Reset DDB. */
    void Reset(){
        if (buffer!=NULL) free((void*&)buffer);
        listOfDDBItems.CleanUp();
        totalDDBSize      = 0;
        finalised         = False;
    }

    /** This method is used to add an interface to the DDB.
        @param gamInterface A DDBInterface to be added to the DDB.
        @return True if the gamInterface has been added successfully. */
    bool AddInterface(const DDBInterface& gamInterface){return DDBAddInterface(*this, gamInterface);}

    /** If the DDB ddb is finalised a call to this method will initialise pointers in
        the specified interface.
        @param gamInterface A DDBInterface to which creat the link.
        @return True if the gamInterface has been successully linked. */
    bool CreateLink(DDBInterface& gamInterface){return DDBCreateLink(*this, gamInterface);};

    /** This method is used to add all the interfaces that are used by a GAM to the DDB.
        @param gam The gam to add.
        @return True if the gamInterface has been added. */
    bool AddInterface(GCRTemplate<GAM> gam){return DDBAddGAMInterface(*this, gam);};

    /** If the DDB ddb is finalised a call to this method will initialise pointers to all interfaces in
        the specified gam.
        @param gam The gam whose DDBInterface is to creat the link.
        @return True if the gamInterface has been added. */
    bool CreateLink(GCRTemplate<GAM> gam){return DDBCreateLinkToGAM(*this, gam);};

    /** Check the list of DDBItem for consistency and allocates the memory.
        @return True if the DDB is consistent, False otherwise.
    */
    bool CheckAndAllocate(){return DDBCheckAndAllocate(*this);}

    /** Prints the DDB information on the specified stream.
        @param s The stream used in the operation. */
    void Print(Streamable& s){DDBPrint(*this,s);}

    /** Loads the DDB Parameters from the CDB.
        @param  cdb The data base containing the initialisation parameters
        @param  err The error stream
        @return True if the Load was successful. False otherwise.
    */
    bool ObjectLoadSetup(ConfigurationDataBase &cdb, StreamInterface *err){
        return DDBObjectLoadSetup(*this,cdb,err);
    };

    /** Saves the DDB Parameters from the CDB.
        @param  cdb The data base where to save the initialisation parameters
        @param  err The error stream
        @return True if the Save was successful. False otherwise.
    */
    bool ObjectSaveSetup(ConfigurationDataBase &cdb, StreamInterface *err){
        return DDBObjectSaveSetup(*this,cdb,err);
    };


};

#endif
