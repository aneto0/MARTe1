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
#if (defined(_LINUX) || defined(_SOLARIS))
#include <sys/mman.h>
#endif
#include "MARTeService.h"
#include "File.h"
#include "ConfigurationDataBase.h"
#include "GCReferenceContainer.h"
#include "MenuContainer.h"
#include "GlobalObjectDataBase.h"
#include "LoggerService.h"

const char *cfgName = "MARTe-Startup.cfg";

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
bool MARTeService::ServiceInit(){

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
bool MARTeService::ServiceStart(){
    File config;

    if(!config.OpenRead((char *)cfgName)){
        CStaticAssertErrorCondition(FatalError, "InitGlobalContainer:: Failed opening file %s\n", cfgName);
        return False;
    }

    // Bufferize cfg file - should solve slowness in RTAI
    FString cfg_buffer;
    cfg_buffer.SetSize(80000);
    config.Seek(0);
    config.GetToken(cfg_buffer,"");
    config.Close();
    cfg_buffer.Seek(0);

    ConfigurationDataBase cdb;
    cdb->ReadFromStream(cfg_buffer);

    CDBExtended info(cdb);
   
    int32 defaultCPUs;
    if(!info.ReadInt32(defaultCPUs, "DefaultCPUs", 0)){
        CStaticAssertErrorCondition(Warning, "DefaultCPUs were not specified. This can be used to help BaseLib2 decide where to put new threads");    	
    }
    else{
        ProcessorType::SetDefaultCPUs(defaultCPUs);
    }
 
    FString logAddress;
    if(!info.ReadFString(logAddress,"LoggerAddress","localhost")){
        CStaticAssertErrorCondition(Warning, "No LoggerAddress specified, using localhost as default");	
    }
    
    int32 logPort;
    if(!info.ReadInt32(logPort,"LoggerPort",32767)){
        CStaticAssertErrorCondition(Warning, "No LoggerPort specified, using 32767 as default");    	
    }

    LSSetUserAssembleErrorMessageFunction(LSAssembleErrorMessage);
    LSSetRemoteLogger(logAddress.Buffer(),logPort);
    LSStartService();

    CStaticAssertErrorCondition(Information, "InitGlobalContainer:: Loading MARTe with file %s \n", cfgName);

    if(!GetGlobalObjectDataBase()->ObjectLoadSetup(cdb,NULL)){
        CStaticAssertErrorCondition(FatalError, "InitGlobalContainer::GetGlobalObjectDataBase().ObjectLoadSetup(cdb,NULL) Failed\n");
        return False;
    }

    CStaticAssertErrorCondition(Information, "InitGlobalContainer:: Successfully initialized MARTe\n");

    GCRTemplate<Message> gcrtm(GCFT_Create);
    gcrtm->Init(0 ,"START");
    GCRTemplate<MessageEnvelope> gcrtme(GCFT_Create);

    bool ret = True;
    ret &= gcrtme->PrepareMessageEnvelope(gcrtm,"StateMachine");
    ret &= MessageHandler::SendMessage(gcrtme);
    if(!ret){
        printf("StartMARTeActivities:  Failed to send START message to StateMachine\n");
        CStaticAssertErrorCondition(FatalError, "StartMARTeActivities:  Failed to send START message to StateMachine\n");
    }

    return ret;
}

/** replace this with the stop service */
bool MARTeService::ServiceStop(){
    GCRTemplate<Message> gcrtm(GCFT_Create);
    gcrtm->Init(0 ,"STOP");
    GCRTemplate<MessageEnvelope> gcrtme(GCFT_Create);

    bool ret = True;
    ret &= gcrtme->PrepareMessageEnvelope(gcrtm,"StateMachine");
    ret &= MessageHandler::SendMessage(gcrtme);
    if(!ret){
        printf("StartMARTeActivities:  Failed to send START message to StateMachine\n");
        CStaticAssertErrorCondition(FatalError, "StartMARTeActivities:  Failed to send START message to StateMachine\n");
    }

    SleepSec(2.0);

    return ret;
}

int main(int argc,char **argv){
#if (defined(_LINUX) || defined(_SOLARIS))
    if(mlockall(MCL_CURRENT|MCL_FUTURE) != 0){
        printf("Failed to do mlockall\n\n");
        return -1;
    }
#endif
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

    MARTeService ms(p+pos,p+pos);

#ifdef _LINUX
    if(argc > 1){
        cfgName = argv[1];
    }   
#endif
    if (ms.Main(argc,argv)){
#if (defined(_LINUX) || defined(_SOLARIS))
        munlockall();
#endif
        return 0;
    }

    return -1;
}

