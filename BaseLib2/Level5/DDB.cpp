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

#include "DDB.h"
#include "GAM.h"
#include "MenuContainer.h"
#include "MenuEntry.h"
#include "File.h"

bool DDBAddInterface(DDB &ddb,const DDBInterface& gamInterface){
    return ddb.__AddInterface(gamInterface);
}

bool DDB::__AddInterface(const DDBInterface& gamInterface)
{
    if (finalised){
        AssertErrorCondition(IllegalOperation,"DDB::AddInterface(): Trying to add an interface to a finalised DDB. No action taken.");
        return False;
    }

    if(!gamInterface.IsFinalised()){
        AssertErrorCondition(IllegalOperation,"DDB::AddInterface(): Interface %s is not finalised. Operation Skipped",gamInterface.GetInterfaceDescriptor().InterfaceName());
        return False;
    }
    const DDBSignalDescriptor* currentDescriptor = gamInterface.SignalsList();

    bool ret = True;

    while (currentDescriptor!=NULL){
        DDBItem *matchingItem = Find(currentDescriptor->SignalName());
        if (matchingItem != NULL){
            // The signal with that name has already been added to the DDB.
            DDBItemStatus         status     = matchingItem->Status();

            ////////////////////////
            // Check the TypeCode //
            ////////////////////////

            if (matchingItem->SignalTypeCode() != currentDescriptor->SignalTypeCode()){
                //*matchingItem|=DDB_TypeMismatch;
                matchingItem->SetSignalStatus(status | DDB_TypeMismatch);
                BString s1,s2;
                AssertErrorCondition(ParametersError,
                    "DDB::AddInterface(): The signal %s type redefinition. Being %s, asked %s.",
                     currentDescriptor->SignalName(),
                     matchingItem->SignalTypeCode().ConvertToString(s1),
                     currentDescriptor->SignalTypeCode().ConvertToString(s2));
                ret = False;
            }

            /////////////////////////////////////////////
            // If the item in DDB is DDB_UndefinedSize //
            /////////////////////////////////////////////

            if(status.CheckMask(DDB_UndefinedSize)){
                // the signal to be added is as well DDB_UndefinedSize update the minimum expected size
                if(currentDescriptor->SignalStoringProperties().CheckMask(DDB_Unsized)){
                    int currentDescriptorMinSize = currentDescriptor->SignalSize() +  currentDescriptor->SignalFromIndex();
                    if(matchingItem->SignalSize() < currentDescriptorMinSize){
                        matchingItem->SetSignalSize(currentDescriptorMinSize);
                    }
                }else{
                // the signal to be added has specified the signal size
                    if(matchingItem->SignalSize() > currentDescriptor->SignalSize()){
                        matchingItem->SetSignalStatus(status | DDB_SizeMismatch);
                        AssertErrorCondition(FatalError,"DDB::AddInterface(): Signal %s has specified size %d. Interface %s is trying to access element %d",currentDescriptor->SignalName(),currentDescriptor->SignalSize(),gamInterface.GetInterfaceDescriptor().InterfaceName() ,matchingItem->SignalSize()-1);
                        ret = False;
                    }else{
                        matchingItem->SetSignalSize(currentDescriptor->SignalSize());
                        // Clear the undefined size bit
                        matchingItem->SetSignalStatus(status & ~DDB_UndefinedSize);
                    }
                }
            }else{

            ///////////////////////////////////////////
            // If the item in DDB has a defined size //
            ///////////////////////////////////////////

                if(currentDescriptor->SignalStoringProperties().CheckMask(DDB_Unsized)){
                    int currentDescriptorMinSize = currentDescriptor->SignalSize() +  currentDescriptor->SignalFromIndex();
                    if(matchingItem->SignalSize() < currentDescriptorMinSize){
                        matchingItem->SetSignalStatus(status | DDB_SizeMismatch);
                        AssertErrorCondition(FatalError,"DDB::AddInterface(): Signal %s has specified size %d. Interface %s is trying to access element %d",currentDescriptor->SignalName(),matchingItem->SignalSize(),gamInterface.GetInterfaceDescriptor().InterfaceName() ,currentDescriptorMinSize);
                        ret = False;
                    }
                }else{
                // the signal to be added has specified the signal size
                    if(matchingItem->SignalSize() != currentDescriptor->SignalSize()){
                        matchingItem->SetSignalStatus(status | DDB_SizeMismatch);
                        AssertErrorCondition(FatalError,"DDB::AddInterface(): Signal %s has specified size %d. Interface %s is specifying size %d",currentDescriptor->SignalName(),matchingItem->SignalSize(),gamInterface.GetInterfaceDescriptor().InterfaceName() ,currentDescriptor->SignalSize());
                        ret = False;
                    }
                }
            }
            matchingItem->AddInterfaceDescriptor(gamInterface);

        }else{
            // The signal with that name doesn't exist in the DDB.
            DDBItem* newItem = new DDBItem(*currentDescriptor);
            if(newItem == NULL){
                AssertErrorCondition(FatalError,"AddInterface(): Failed allocating memory for new Item %s.",currentDescriptor->SignalName());
                return False;
            }

            newItem->AddInterfaceDescriptor(gamInterface);
            listOfDDBItems.ListAdd(newItem);
        }

        currentDescriptor = dynamic_cast<DDBSignalDescriptor*>(currentDescriptor->Next());
    }

    return ret;
}

bool DDBAddGAMInterface(DDB &ddb,GCRTemplate<GAM> gam){

    if(!gam.IsValid()){
        ddb.AssertErrorCondition(FatalError,"AddInterface: GamReference not valid");
        return False;
    }

    LinkedListable *interfaces = gam->InterfacesList();
    while(interfaces != NULL){
        DDBInterface *ddbInterface = dynamic_cast<DDBInterface *>(interfaces);
        if(ddbInterface == NULL){
            ddb.AssertErrorCondition(FatalError,"AddInterface: failed to dynamic cast to DDBInterface for GAM %s", gam->GamName());
            return False;
        }

        if(!ddb.AddInterface(*ddbInterface)){
            ddb.AssertErrorCondition(FatalError,"AddInterface: Failed adding Interface %s for GAM %s to DDB",ddbInterface->GetInterfaceDescriptor().InterfaceName() ,gam->GamName());
            return False;
        }

        ddb.AssertErrorCondition(Information,"AddInterface: Adding Interface %s to GAM %s",ddbInterface->InterfaceName() ,gam->GamName());
        interfaces = interfaces->Next();
    }

    return True;
}

bool DDBCheckAndAllocate(DDB &ddb){

    bool checkResult=False;
    // Check for errors
    CheckDDBItemIterator checkIterator(checkResult,ddb.totalDDBSize);
    ddb.listOfDDBItems.ListIterate(&checkIterator);

    if (checkResult){
        //allocate memory
        ddb.buffer=(char*)malloc(ddb.totalDDBSize);
        if (ddb.buffer==NULL){
            ddb.AssertErrorCondition(FatalError,"DDB::CheckAndAllocate(): Failed to allocate memory for the DDB.");
            return False;
        }
        ddb.finalised = True;
        ddb.AssertErrorCondition(Information,"DDB::CheckAndAllocate(): Successfully Checked and Allocated %d signals",ddb.listOfDDBItems.ListSize());
        return True;
    }else{
        ddb.AssertErrorCondition(FatalError,"DDB::CheckAndAllocate(): One or more errors prevent DDB allocation.");
        ddb.finalised = False;
        return False;
    }

}

bool DDBCreateLink(DDB &ddb, DDBInterface& gamInterface){
    return ddb.__CreateLink(gamInterface);
}

bool DDB::__CreateLink(DDBInterface& gamInterface){

    if (buffer==NULL) {
        AssertErrorCondition(FatalError,"CreateLink: buffer has not been allocated");
        return False;
    }

    if (!finalised) {
        AssertErrorCondition(FatalError,"CreateLink: Cannot create link when DDB is not finalised");
        return False;
    }

    // Reset the list of pointers to a previous ddb
    if(!gamInterface.ResetPointerList()){
        AssertErrorCondition(FatalError,"CreateLink: Failed to reset the list of pointers for interface %s",gamInterface.GetInterfaceDescriptor().InterfaceName());
        return False;
    }

    const LinkedListable *signal = gamInterface.SignalsList();
    while (signal != NULL){
        const DDBSignalDescriptor* descriptor = dynamic_cast<const DDBSignalDescriptor*>(signal);
        if(descriptor == NULL){
            AssertErrorCondition(FatalError,"CreateLink: Failed dynamic cast to DDBSignalDescriptor*");
            return False;
        }

        DDBItem *item = Find(descriptor->SignalName());
        if(item == NULL){
            AssertErrorCondition(FatalError,"CreateLink: Cannot find item %s in DDB List.",descriptor->SignalName());
            return False;
        }

        // The DDB send back the address of the first element of a vector.
        int32* signalPointer = reinterpret_cast<int32*>(buffer + item->SignalOffset());
        if (!gamInterface.Link(descriptor->SignalName(),signalPointer)){
            AssertErrorCondition(FatalError,"CreateLink: Link to signal %s for interface %s failed",descriptor->SignalName(),gamInterface.GetInterfaceDescriptor().InterfaceName());
            return False;
        }
        signal = signal->Next();
    }

    return gamInterface.CheckConsecutyAndOptimizeInterface();
}

bool DDBCreateLinkToGAM(DDB &ddb, GCRTemplate<GAM> gam){

    if(!gam.IsValid()){
        ddb.AssertErrorCondition(FatalError,"CreateLink: GamReference not valid");
        return False;
    }

    if (ddb.buffer==NULL) {
        ddb.AssertErrorCondition(FatalError,"CreateLink: buffer has not been allocated");
        return False;
    }

    if (!ddb.finalised) {
        ddb.AssertErrorCondition(FatalError,"CreateLink: Cannot create link when DDB is not finalised");
        return False;
    }

    LinkedListable *interfaces = gam->InterfacesList();
    while(interfaces != NULL){
        DDBInterface *ddbInterface = dynamic_cast<DDBInterface *>(interfaces);
        if(ddbInterface == NULL){
            ddb.AssertErrorCondition(FatalError,"CreateLink: failed to dynamic cast to DDBInterface for GAM %s", gam->GamName());
            return False;
        }

        if(!ddb.CreateLink(*ddbInterface)){
            ddb.AssertErrorCondition(FatalError,"CreateLink: Failed creating Link to Interface %s for GAM %s",ddbInterface->GetInterfaceDescriptor().InterfaceName() ,gam->GamName());
            return False;
        }

        ddb.AssertErrorCondition(Information,"CreateLink: CreateLink to Interface %s to GAM %s",ddbInterface->InterfaceName() ,gam->GamName());
        interfaces = interfaces->Next();
    }

    return True;
}

void DDBPrint(DDB &ddb, Streamable& s){
    ConfigurationDataBase cdb;
    ddb.ObjectSaveSetup(cdb,NULL);
    cdb->WriteToStream(s);
}

bool DDBObjectLoadSetup(DDB &ddb, ConfigurationDataBase &info, StreamInterface *err){

    if (!ddb.GCNamedObject::ObjectLoadSetup(info,err)){
        ddb.AssertErrorCondition(FatalError,"ObjectLoadSetup:Cannot retrieve name of node");
        return False;
    }

    return True;
}

bool DDBObjectSaveSetup(DDB &ddb, ConfigurationDataBase &info, StreamInterface *err){

    CDBExtended cdb(info);
    cdb.WritePointer(ddb.buffer,"BaseAddress");
    cdb.WriteInt32(ddb.listOfDDBItems.ListSize(),"NumberOfSignals");


    LinkedListable *item = ddb.listOfDDBItems.List();
    int32 counter = 0;
    while(item != NULL){
        DDBItem *ddbItem = dynamic_cast<DDBItem *>(item);
        if(ddbItem == NULL){
            ddb.AssertErrorCondition(FatalError,"DDBObjectSaveSetup: failed to dynamic cast to DDBItem");
            return False;
        }

        FString nodeName;
        nodeName.Printf("DDBItem%d",counter++);
        cdb->AddChildAndMove(nodeName.Buffer());

        ddbItem->ObjectSaveSetup(cdb,err);

        cdb.WritePointer((ddb.buffer + ddbItem->SignalOffset()),"PositionInMemory");
        cdb->MoveToFather();
        item = item->Next();
    }

    return True;
}

bool DDBBrowseMenu(StreamInterface &in,StreamInterface &out,void *userData){
    DDB *ddb = (DDB *)userData;
    if  (ddb == NULL) return False;

    GCRTemplate<MenuContainer> menu(GCFT_Create);
    menu->SetTitle(ddb->Name());
    

    LinkedListable *item = ddb->listOfDDBItems.List();
    int counter = 0;
    while(item != NULL){
        DDBItem *ddbItem = dynamic_cast<DDBItem *>(item);
        if(ddbItem == NULL){
            ddb->AssertErrorCondition(FatalError,"DDBBrowseMenu: failed to dynamic cast to DDBItem");
            return False;
        }

        GCRTemplate<MenuEntry> entry(GCFT_Create);
        entry->SetTitle(ddbItem->SignalName());
        entry->SetUp(DDBItemBrowseMenu, NULL, NULL, (void *)ddbItem);
        menu->Insert(entry);

        item = item->Next();
    }

    return menu->TextMenu(in, out);
}

bool DDBDumpMenu  (StreamInterface &in,StreamInterface &out,void *userData){
    DDB *ddb = (DDB *)userData;
    if  (ddb == NULL) return False;
    
ConfigurationDataBase cdb;
    if(!ddb->ObjectSaveSetup(cdb, &out)){
        ddb->AssertErrorCondition(InitialisationError, "DDBDumpMenu: DDB::ObjectSaveSetup Failed for object %s", ddb->Name() );
        return False;
    }

    FString fileName = ddb->Name();
    fileName+=".cfg";
    File ddbDump;
    if(!ddbDump.OpenNew(fileName.Buffer())){
        ddb->AssertErrorCondition(InitialisationError, "DDBDumpMenu: Failed opening File %s", fileName.Buffer() );
        return False;
    }

    cdb->WriteToStream(ddbDump);

    return True;
}





OBJECTLOADREGISTER(DDB,"$Id: DDB.cpp,v 1.18 2010/01/22 16:54:07 aneto Exp $")

