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

#include "Console.h"
#include "SXMemory.h"
#include "File.h"
#include "ConfigurationDataBase.h"
#include "GlobalObjectDataBase.h"
#include "MenuContainer.h"
#include "LoggerService.h"

bool run = True;
#if defined(_LINUX) || defined (_SOLARIS)
#include <signal.h>
void ServiceStop(int signal){
    run = False;
}
#endif

void Start(const char *name)
{
    GCRTemplate<Message> gcrtm(GCFT_Create);
    gcrtm->Init(0,"START");

    GCRTemplate<MessageEnvelope> gcrtme(GCFT_Create);

    gcrtme->PrepareMessageEnvelope(gcrtm,name,MDRF_None,NULL);

    MessageHandler::SendMessage(gcrtme);

}

void Stop(const char *name)
{
    GCRTemplate<Message> gcrtm(GCFT_Create);
    gcrtm->Init(0,"STOP");

    GCRTemplate<MessageEnvelope> gcrtme(GCFT_Create);

    gcrtme->PrepareMessageEnvelope(gcrtm,name,MDRF_None,NULL);

    MessageHandler::SendMessage(gcrtme);

}

const char *cdbMap=
"+UDP_RELAY_SERVER={\n"
"    Class = UDPLoggerReceiver\n"
"    Title = \"UDP RELAY LOGGER\" \n"
"    Port=32767\n"
"    +UDP_LOGGER_FILE={\n"
"        Title = \"UDP FILE LOGGER\" \n"
"        Class = UDPLoggerFile\n"
"        MaxNumLogFiles=100\n"
"        MaxNotUsedTimeSecs=3600\n"
#if defined (_WIN32)
"         BaseDir = .\n"
#else
"        BaseDir = /tmp"
#endif
"    }\n"
"    +UDP_LOGGER_RELAY={\n"
"        Title = \"UDP LOGGER RELAY\" \n"
"        Class = UDPLoggerRelay\n"
"        MaxNotPingTimeSecs=30\n"
"        RelayServerPort=44444\n"
"        MaxHistoryQueueMessages=10000\n"
"    }\n"
"}"
"+WEBROOT={\n"
"    Class = HttpGroupResource\n"
"    +LISTFILES = {\n"
"         Class = HttpDirectoryResource \n"
#if defined (_WIN32)
"         BaseDir = .\n"
#else
"         BaseDir = /tmp/\n"
#endif
"         FileFilter = *.html\n"
"    }\n"
"    +APPLET = {\n"
"         Class = HttpDirectoryResource \n"
#if defined (_WIN32)
"         BaseDir = .\n"
#else
"         BaseDir = /tmp\n"
#endif
"         StartHtml = applet\n"
"    }\n"
"    AddReference = {\n"
"         UDP_RELAY_SERVER\n"
"    }\n"
"}\n"
"+HTTPSERVER = {\n"
"    Class = HttpService\n"
"    Port=8083\n"
"    VerboseLevel=10\n"
"    Root=WEBROOT\n"
"}\n";

int main(int argc, char **argv){
//    LSSetUserAssembleErrorMessageFunction(NULL);
    LSSetUserAssembleErrorMessageFunction(LSAssembleErrorMessage);
    LSSetRemoteLogger("localhost",32767);

    bool useConsole = (argc > 1 && argv[1][0] == '-' && argv[1][1] == 'c');

    //Init the containers
//    SXMemory config((char *)cdbMap,strlen(cdbMap));
    File config;
    if(argc > 2){
        if(!config.OpenRead(argv[2])){
            printf("Failed opening file %s\n", argv[2]);
            return 0;
        }
    }
    else{
#ifdef _LINUX
        if(!config.OpenRead("/etc/RelayLoggerLinux.cdb")){
#else
        if(!config.OpenRead("RelayLoggerWindows.cfg")){
#endif
            printf("Failed opening file CDB.cfg\n");
            return 0;
        }
    }

    ConfigurationDataBase cdb;
    FString err;
    if (!cdb->ReadFromStream(config,&err)){
        printf("Init: cdb.ReadFromStream failed: %s",err.Buffer());
        return -1;
    }

    GCRTemplate<GCReferenceContainer> godb = GetGlobalObjectDataBase();
    godb->ObjectLoadSetup(cdb,NULL);

    /** show objects */
    GCRCLister lister;
    godb->Iterate(&lister, GCFT_Recurse);

    GCRTemplate<MenuContainer> menuContainer;
    menuContainer = godb->Find("UDP_RELAY_SERVER");
    if (!menuContainer.IsValid()){
        printf("cannot find MENUS MenuContainer object\n");
        printf("Going to die...\n");
        return -1;
    }

    Start("HTTPSERVER");

    //Console configuration
    if (useConsole){
        Console con(PerformCharacterInput);
        con.SetPaging(True);

        menuContainer->TextMenu(con, con);
    }
    else{
#if defined (_LINUX) || defined (_SOLARIS)
        signal(SIGTERM, ServiceStop);
        while(run){
            SleepSec(1.0);
        }
#endif
    }
    Stop("HTTPSERVER");
    SleepSec(1.0);
    return 0;
}
