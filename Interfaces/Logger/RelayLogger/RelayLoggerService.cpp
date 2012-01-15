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
#include "RelayLoggerService.h"
#include "File.h"
#include "ConfigurationDataBase.h"
#include "GCReferenceContainer.h"
#include "MenuContainer.h"
#include "GlobalObjectDataBase.h"

#ifdef _LINUX
const char *cfgName = "/etc/RelayLoggerLinux.cdb";
#else
const char *cfgName = "RelayLoggerWindows.cfg";
#endif

void SSStart(const char *name)
{
    GCRTemplate<Message> gcrtm(GCFT_Create);
    gcrtm->Init(0,"START");

    GCRTemplate<MessageEnvelope> gcrtme(GCFT_Create);

    gcrtme->PrepareMessageEnvelope(gcrtm,name,MDRF_None,NULL);

    MessageHandler::SendMessage(gcrtme);

}

void SSStop(const char *name)
{
    GCRTemplate<Message> gcrtm(GCFT_Create);
    gcrtm->Init(0,"STOP");

    GCRTemplate<MessageEnvelope> gcrtme(GCFT_Create);

    gcrtme->PrepareMessageEnvelope(gcrtm,name,MDRF_None,NULL);

    MessageHandler::SendMessage(gcrtme);

}

/** replace this with the main of the application */
bool RelayLoggerService::ServiceInit(){

    File config;
    if(!config.OpenRead(cfgName)){
        printf("Failed opening file %s\n",cfgName);
        return False;
    }

    FString err;
    if (!cdb->ReadFromStream(config,&err)){
        printf("Init: cdb.ReadFromStream failed: %s",err.Buffer());
        return False;
    }


    return True;
}

/** replace this with the start service */
bool RelayLoggerService::ServiceStart(){
    GCRTemplate<GCReferenceContainer> godb = GetGlobalObjectDataBase();
    godb->ObjectLoadSetup(cdb,NULL);

    GCRTemplate<MenuContainer> menuContainer;
    menuContainer = godb->Find("UDP_RELAY_SERVER");
    if (!menuContainer.IsValid()){
        printf("cannot find MENUS MenuContainer object\n");
        printf("Going to die...\n");
        return False;
    }

    SSStart("HTTPSERVER");

    /** show objects */
    GCRCLister lister;
    godb->Iterate(&lister, GCFT_Recurse);

    return True;
}

/** replace this with the stop service */
bool RelayLoggerService::ServiceStop(){
    SSStop("HTTPSERVER");

    return True;
}

int main(int argc,char **argv){
    FString serviceName;
    serviceName = argv[0];
    char *p = serviceName.BufferReference();

    // find the end
    int pos = serviceName.Size();
    while (pos > 0){
        if (p[pos] == '.') p[pos] = 0;
        if ((p[pos] == '\\') || (p[pos] == '/')){
            pos++;
            break;
        }
        pos--;
    }

    RelayLoggerService rls(p+pos,p+pos);

    if (rls.Main(argc,argv)) return 0;

    return -1;
}

