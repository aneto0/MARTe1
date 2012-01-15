//******************************************************************************
//
//      MARTe
//      $Log: MARTe.cpp,v $
//      Revision 1.14  2011/02/18 13:28:16  ppcc_dev
//      Hard termination on win32. Win32 expert to review
//
//      Revision 1.13  2010/02/09 15:07:06  aneto
//      Allow to specify in what CPUs should the default threads be set
//
//      Revision 1.12  2009/02/05 15:57:11  aneto
//      Added 2 seconds sleep in order to avoid the exit of MARTe when the config file loading fails
//
//      Revision 1.11  2009/01/07 14:30:09  aneto
//      External configuration file loading in VxWorks
//
//      Revision 1.10  2009/01/07 13:48:11  aneto
//      VxWorks support
//
//      Revision 1.9  2009/01/06 11:05:36  fpiccolo
//      Minor modifications to allow starting the menu on RTAI as it is done for VxWorks
//
//      Revision 1.8  2008/10/07 12:40:52  rvitelli
//      Corrected init problem with DisplayGAM and Java
//
//      Revision 1.7  2008/07/24 12:44:03  rvitelli
//      Added support for Logger settings in cfg file.
//
//      Revision 1.6  2008/07/23 11:06:18  rvitelli
//      added Seek(0)
//
//      Revision 1.5  2008/07/18 08:18:48  rvitelli
//      Solved problem with RTAI slowness
//      while loading cfg file (added buffering).
//
//      Revision 1.4  2008/07/16 15:12:35  rvitelli
//      Added support for argc and argv
//
//      Revision 1.3  2008/07/03 08:36:35  aneto
//      Added #ifdefs for RTAI.
//
//      Revision 1.2  2008/05/20 16:26:28  fisa
//      a few more features
//      works on a PC
//
//      Revision 1.1  2008/05/09 13:07:44  fpiccolo
//      Work in progress.
//
//******************************************************************************

#include "System.h"
#include "File.h"
#include "FString.h"
#include "Console.h"
#include "ConfigurationDataBase.h"
#include "LoggerService.h"
#include "GlobalObjectDataBase.h"
#include "Sleep.h"
#include "EventSem.h"

#include "Message.h"
#include "MessageEnvelope.h"
#include "MessageHandler.h"
#include "Console.h"
#include "MenuContainer.h"

#include "MARTeMenu.h"

extern "C"{
    int InitGlobalContainer(const void *fileName);
    int MARTeMenu();
    int StartMARTeActivities();
    int StopMARTeActivities();
}

static bool initialized;
static EventSem    sem;

#ifdef _VXWORKS
extern "C"{
    int MARTe(char *argv);
}

    int MARTe(char *argv){
        const char          *marteCfg = "MARTe.cfg";
        if(argv != NULL){
            marteCfg = argv;
        }
#else
    int main(int argc, char *argv[]){

        if (argc>2) return 0;
        const char          *marteCfg = "MARTe.cfg";
        if(argc > 1)         marteCfg = argv[1];
#endif //#ifdef _VXWORKS
        sem.Create();

        if(initialized == True){
            CStaticAssertErrorCondition(FatalError, "MARTe:: MARTe has already been initialized.\n");
            printf("MARTe:: MARTe has already been initialized.\n");
            return 0;
        }
        // VxWorks needs to start another thread for initialisation, but this causes problems with DisplayGAM under linux
#ifdef _VXWORKS
        Threads::BeginThread((ThreadFunctionType)InitGlobalContainer, (void *)marteCfg,THREADS_DEFAULT_STACKSIZE,"InitGlobalContainer");
        sem.Wait();
#else
        InitGlobalContainer(marteCfg);
#endif

            SleepSec(2.0);
        if(initialized == False){
            GCRCLister lister;
            GetGlobalObjectDataBase()->Iterate(&lister,GCFT_Recurse);
            printf("MARTe:: MARTe Initialization has failed.\n");
            CStaticAssertErrorCondition(FatalError, "MARTe:: MARTe Initialization has failed.\n");
            SleepSec(2.0);
        }else{
            
#if defined(_VXWORKS) || defined(_RTAI) 

            StartMARTeActivities();

#else
            StartMARTeActivities();			
            MARTeMenu();					
            StopMARTeActivities();
            sem.Close();
#if defined(_WIN32)			
			SleepSec(2.0);
			//Very bad way of terminating the application but this keep hanging... to be reviewed by windows expert...
			HANDLE hHandle;
			DWORD dwExitCode = 0;
			hHandle = ::OpenProcess(PROCESS_ALL_ACCESS,0,GetCurrentProcessId());
			::GetExitCodeProcess(hHandle,&dwExitCode);
			::TerminateProcess(hHandle,dwExitCode);
#endif
#endif
        }		
        return 1;
    }


    int InitGlobalContainer(const void *fileName){

        const char *fName = (const char *)fileName;
        File config;

        if(!config.OpenRead((char *)fName)){
            CStaticAssertErrorCondition(FatalError, "InitGlobalContainer:: Failed opening file %s\n", fName);
            sem.Post();
            return 0;
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

        CStaticAssertErrorCondition(Information, "InitGlobalContainer:: Loading MARTe with file %s \n", fName);

        if(!GetGlobalObjectDataBase()->ObjectLoadSetup(cdb,NULL)){
            CStaticAssertErrorCondition(FatalError, "InitGlobalContainer::GetGlobalObjectDataBase().ObjectLoadSetup(cdb,NULL) Failed\n");
            sem.Post();
            return 0;
        }

        CStaticAssertErrorCondition(Information, "InitGlobalContainer:: Successfully initialized MARTe\n");
        initialized = True;
        sem.Post();
        return 0;
    }


    int StartMARTeActivities(){

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
    };


    int StopMARTeActivities(){

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

#if defined(_VXWORKS) || defined(_RTAI) 

        // Cleanup added for RTAI and VxWorks
        GetGlobalObjectDataBase()->CleanUp();
        initialized = False;
        LSStopService();
#endif
        
        return ret;
    }


    int MARTeMenu(){

        //Get_private_MARTeMenuInfo();
        Console con;
        GCReference menu = GetGlobalObjectDataBase()->Find("MARTeMenu");
        if(menu.IsValid()){
            GCRTemplate<MenuContainer> marteMenu(menu);
            if(!marteMenu.IsValid()){
                printf("MARTeMenu: The object MARTeMenu is not of type MenuContainer. \n");
                CStaticAssertErrorCondition(FatalError, "MARTeMenu: The object MARTeMenu is not of type MenuContainer. \n");
                return 0;
            }

            marteMenu->TextMenu(con,con);

        }else{

            con.Printf("Press ENTER to exit\n");
            char buffer[128];
            con.GetToken(buffer, "\n \t \r", 127 );
        }

        return 1;
    }

