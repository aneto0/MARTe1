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

#include "GAM.h"
#include "DDBIOInterface.h"
#include "DDBInputInterface.h"
#include "DDBOutputInterface.h"
#include "MenuEntry.h"
#include "GlobalObjectDataBase.h"

bool GAMAddInputInterface(GAM& gam, const char* interfaceName, DDBInputInterface *&ddbi){

    if( gam.finalisedInterfaces ){
        CStaticAssertErrorCondition(FatalError,"%s: Cannot add interfaces to finalised gam", gam.GamName());
        return False;
    }

    /*
        This check has been inserted to avoid the adding of interfaces already
        created by the user.
        WARNING: The interface has to be deleted by the GAM and not by the user
    */
    if( ddbi == NULL )
        ddbi = new DDBInputInterface(gam.GamName(),interfaceName,DDB_ReadMode);

    if(ddbi == NULL){
        CStaticAssertErrorCondition(OSError,"%s: Failed to allocate memory for object DDBInputInterface", gam.GamName());
        return False;
    }

    gam.listOfDDBInterfaces.ListAdd(ddbi);

    return True;
}

bool GAMAddOutputInterface(GAM& gam,       const char* interfaceName,
                           DDBOutputInterface *&ddbo, DDBInterfaceAccessMode accessMode=DDB_WriteMode){

    if( gam.finalisedInterfaces ){
        CStaticAssertErrorCondition(FatalError,"%s: Cannot add interfaces to finalised gam", gam.GamName());
        return False;
    }

    accessMode&=~DDB_ReadMode;

    /*
        This check has been inserted to avoid the adding of interfaces already
        created by the user.
        WARNING: The interface has to be deleted by the GAM and not by the user
    */

    if( ddbo == NULL )
        ddbo = new DDBOutputInterface(gam.GamName(),interfaceName,accessMode);

    if(ddbo == NULL){
        CStaticAssertErrorCondition(OSError,"%s: Failed to allocate memory for object DDBOutputInterface",gam.GamName());
        return False;
    }

    gam.listOfDDBInterfaces.ListAdd(ddbo);

    return True;
}

bool GAMAddIOInterface(GAM& gam, const char* interfaceName,
                       DDBIOInterface *&ddbio, DDBInterfaceAccessMode accessMode=DDB_ReadMode|DDB_WriteMode){

    ddbio = new DDBIOInterface(gam.GamName(),interfaceName,accessMode);
    if(ddbio == NULL){
        CStaticAssertErrorCondition(OSError,"%s: Failed to allocate memory for object DDBIOInterface", gam.GamName());
        return False;
    }
    gam.listOfDDBInterfaces.ListAdd(ddbio);

    return True;
}

bool GAMFinaliseInterfaces(GAM& gam){

    LinkedListable* currentInterface = gam.listOfDDBInterfaces.List();
    while (currentInterface!=NULL){

        DDBInterface* cInterface=dynamic_cast<DDBInterface*>(currentInterface);
        if(cInterface == NULL){
            CStaticAssertErrorCondition(InitialisationError,"FinaliseInterfaces: dynamic_cast to DDBInterface* has failed for %s",gam.GamName());
            return False;
        }

        if(!cInterface->Finalise()){
            CStaticAssertErrorCondition(InitialisationError,"FinaliseInterfaces: Finalise failed for %s",gam.GamName());
            return False;
        }

        currentInterface = currentInterface->Next();
    }

    gam.finalisedInterfaces = True;

    GCRTemplate <MenuContainer>       interfaces(GCFT_Create);
    interfaces->SetTitle("DDB Interfaces");
    currentInterface = gam.listOfDDBInterfaces.List();
    while (currentInterface!=NULL){

        DDBInterface* cInterface=dynamic_cast<DDBInterface*>(currentInterface);
        if(cInterface == NULL){
            CStaticAssertErrorCondition(InitialisationError,"FinaliseInterfaces: dynamic_cast to DDBInterface* has failed for %s",gam.GamName());
            return False;
        }
#if 0
        GCRTemplate <MenuContainer>       ddbI(GCFT_Create);
        ddbI->SetTitle(cInterface->InterfaceName());

        GCRTemplate <MenuEntry>          dumpDDBInterface(GCFT_Create);
        dumpDDBInterface->SetTitle("Dump Interface");
        dumpDDBInterface->SetUp(DDBIDumpMenu, NULL, NULL, cInterface);

        GCRTemplate <MenuEntry>          browseDDBInterface(GCFT_Create);
        browseDDBInterface->SetTitle("Browse Interface");
        browseDDBInterface->SetUp(DDBIBrowseMenu, NULL, NULL, cInterface);

        ddbI->Insert(dumpDDBInterface);
        ddbI->Insert(browseDDBInterface);

        interfaces->Insert(ddbI);
#endif
        currentInterface = currentInterface->Next();

    }

    gam.menu->Insert(interfaces);

    /* User Menu */
    GCRTemplate <MenuEntry>           userMenu(GCFT_Create);
    userMenu->SetTitle("User Menu");
    userMenu->SetUp(GAMMenuInterface, NULL, NULL, &gam);
    gam.menu->Insert(userMenu);

    return True;
}

bool GAMRemapInterfaces(GAM& gam, ConfigurationDataBase& cdb){

    CDBExtended  cdbData(cdb);

    if(!cdbData->Move("Remappings")){
        CStaticAssertErrorCondition(Warning,"GAM::RemapInterfaces: No Remappings specified for %s GAM",gam.GamName());
        return True;
    }

    LinkedListable* currentInterface = gam.listOfDDBInterfaces.List();
    while (currentInterface!=NULL){

        // Cast to DDBInterface
        DDBInterface* cInterface=dynamic_cast<DDBInterface*>(currentInterface);
        if(cInterface == NULL){
            CStaticAssertErrorCondition(InitialisationError,"RemapInterfaces: dynamic_cast to DDBInterface* has failed for %s",gam.GamName());
            return False;
        }

        if(cdbData->Move(cInterface->GetInterfaceDescriptor().InterfaceName())){

            for (int signalIndex=0; signalIndex < cdbData->NumberOfChildren(); signalIndex++){

                cdbData->MoveToChildren(signalIndex);

                FString oldName, newName;
                cdbData->NodeName(oldName);
                cdbData.ReadFString(newName,"");
                newName.Seek(0);
                cInterface->SignalRename(oldName.Buffer(),newName.Buffer());

                cdbData->MoveToFather(1);
            }

            cdbData->MoveToFather(1);

        }else{
            CStaticAssertErrorCondition(InitialisationError,"RemapInterfaces: No Remapping specified for interface %s in %s",cInterface->GetInterfaceDescriptor().InterfaceName(),gam.GamName());
        }

        currentInterface = currentInterface->Next();
    }

    cdbData->MoveToFather(1);

    return True;
}

bool GAMMenuInterface(StreamInterface &in,StreamInterface &out,void *userData){
    GAM *gam = (GAM *) userData;
    if(gam == NULL) return False;
    return gam->MenuInterface(in, out, NULL);
}


bool GAMGetPersistentCDB(GAM &gam, ConfigurationDataBase &cdb){
    GCRTemplate<GCReferenceContainer>  godb   = GetGlobalObjectDataBase();
    GCRTemplate<CDB>                   cdbGAM = godb->Find("GAMConfigurationDatabase");

    if(!cdbGAM.IsValid()){
        CStaticAssertErrorCondition(Information, "GAM: %s no GAMConfigurationDatabase seems to be available. Trying to create DB", gam.Name());
        if(!godb->Lock()){
            CStaticAssertErrorCondition(InitialisationError, "GAM: %s Could not lock the DB", gam.Name());
            return False;
        }

        GCRTemplate<CDB> gamCDB(GCFT_Create);
        gamCDB->SetObjectName("GAMConfigurationDatabase");
        godb->Insert(gamCDB);
        godb->UnLock();
    }

    cdbGAM = godb->Find("GAMConfigurationDatabase");
    if(!cdbGAM.IsValid()){
        CStaticAssertErrorCondition(InitialisationError, "GAM: %s could not find the GAMConfigurationDatabase. It should have been created already", gam.Name());
        return False;
    }

    cdbGAM->MoveToRoot();
    if(!cdbGAM->Move(gam.Name())){
        cdbGAM->AddChildAndMove(gam.Name());
        cdbGAM->MoveToFather();
        return True;
    }

    return cdb->CopyFrom(cdbGAM.operator->());
}

bool GAMUpdatePersistentCDB(GAM &gam, ConfigurationDataBase &cdb){

    GCRTemplate<GCReferenceContainer>  godb   = GetGlobalObjectDataBase();
    GCRTemplate<CDB>                   cdbGAM = godb->Find("GAMConfigurationDatabase");
    if(!cdbGAM.IsValid()){
        CStaticAssertErrorCondition(Information, "GAMUpdatePersistentCDB: %s no GAMConfigurationDatabase seems to be available. Trying to create DB", gam.Name());
        if(!godb->Lock()){
            CStaticAssertErrorCondition(InitialisationError, "GAMUpdatePersistentCDB: %s Could not lock the DB", gam.Name());
            return False;
        }

        GCRTemplate<CDB> gamCDB(GCFT_Create);
        gamCDB->SetObjectName("GAMConfigurationDatabase");
        godb->Insert(gamCDB);
        godb->UnLock();
    }

    cdbGAM = godb->Find("GAMConfigurationDatabase");
    if(!cdbGAM.IsValid()){
        CStaticAssertErrorCondition(InitialisationError, "GAMUpdatePersistentCDB: %s could not find the GAMConfigurationDatabase. It should have been created already", gam.Name());
        return False;
    }

    CDBExtended cdbe(cdbGAM);
    cdbe->MoveToRoot();
    cdbe->Delete(gam.Name());
    cdbe->AddChildAndMove(gam.Name());

    cdb->MoveToRoot();
    if(!cdbe->CopyFrom(cdb.operator->())){
        CStaticAssertErrorCondition(InitialisationError, "GAMUpdatePersistentCDB: %s could not create the cdb to the database", gam.Name());
        return False;
    }

    return cdbe->MoveToFather();
}

OBJECTREGISTER(GAM,"$Id: GAM.cpp,v 1.16 2011/01/05 11:02:00 aneto Exp $")

