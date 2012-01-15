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


#include "WindowsServiceApplication.h"

#if defined(_LINUX)
bool run = True;
#include <signal.h>
void LXServiceStop(int signal){
    run = False;
}
#endif


#if defined (_MSC_VER)
#define INFINITE -1

WindowsServiceApplication *wsa = NULL;

int WindowsServiceApplication::Install(){

    SC_HANDLE  hSCManager = NULL;
    SC_HANDLE  hService = NULL;
    CHAR       szWinDir[MAX_PATH];
    CHAR       szImagePath[MAX_PATH];
    if((hSCManager = OpenSCManager(NULL, NULL,SC_MANAGER_CREATE_SERVICE)) == NULL) {
        printf("OpenSCManager Failed\n");
        return -1;
    }

    GetWindowsDirectory(szWinDir, MAX_PATH);
    sprintf(szImagePath, "%s\\system32\\%s.exe", szWinDir, serviceName.Buffer());

    if ((hService = CreateService(hSCManager,serviceName.Buffer(),title.Buffer(),SERVICE_ALL_ACCESS,
        SERVICE_WIN32_OWN_PROCESS, SERVICE_DEMAND_START,SERVICE_ERROR_IGNORE, szImagePath,
        NULL, NULL, NULL, NULL, NULL)) == NULL) {
            printf("CreateService Failed, %d\n",GetLastError());
            return -1;
    }
    CloseServiceHandle(hService);
    CloseServiceHandle(hSCManager);
    printf("%s installed successfully\n", szImagePath);
    return 0;
}

BOOL WindowsServiceApplication::NotifySCM(DWORD dwState, DWORD dwWin32ExitCode,DWORD dwProgress){
   SERVICE_STATUS serviceStatus;
   // fill in the SERVICE_STATUS structure
   serviceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
   serviceStatus.dwCurrentState = dwCurrentState = dwState;
   serviceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP |
      SERVICE_ACCEPT_PAUSE_CONTINUE | SERVICE_ACCEPT_SHUTDOWN;
   serviceStatus.dwWin32ExitCode = dwWin32ExitCode;
   serviceStatus.dwServiceSpecificExitCode = 0;
   serviceStatus.dwCheckPoint = dwProgress;
   serviceStatus.dwWaitHint = 3000;
   // send status to SCM
   return SetServiceStatus(hService, &serviceStatus);
}

void ServiceHandlerFN(DWORD fdwControl){
    wsa->ServiceHandler(fdwControl);
}


void WindowsServiceApplication::ServiceHandler(DWORD fdwControl){
   switch(fdwControl) {
      case SERVICE_CONTROL_STOP:
         OutputDebugString("Stop\n");
         wsa->NotifySCM(SERVICE_STOP_PENDING, 0, 1);
         SetEvent(hDoneEvent);
         NotifySCM(SERVICE_STOPPED, 0, 0);
         break;
       case SERVICE_CONTROL_PAUSE:
         OutputDebugString("Pause\n");
         NotifySCM(SERVICE_PAUSE_PENDING, 0, 1);
         wsa->ServiceStop();
         NotifySCM(SERVICE_PAUSED, 0, 0);
         break;
      case SERVICE_CONTROL_CONTINUE:
         OutputDebugString("Continue\n");
         NotifySCM(SERVICE_CONTINUE_PENDING, 0, 1);
         ServiceStart();
         NotifySCM(SERVICE_RUNNING, 0, 0);
         break;
      case SERVICE_CONTROL_INTERROGATE:
         OutputDebugString("Interrogate\n");
         NotifySCM(dwCurrentState, 0, 0);
         break;
      case SERVICE_CONTROL_SHUTDOWN:
         OutputDebugString("Shutdown\n");
         break;
   }
}

static void ServiceMainFN(DWORD dwArgc, LPTSTR *lpszArgv){
    wsa->ServiceMain(dwArgc, lpszArgv);
}

void WindowsServiceApplication::ServiceMain(DWORD dwArgc, LPTSTR *lpszArgv){
    if (!(hService = RegisterServiceCtrlHandler(serviceName.Buffer(),(LPHANDLER_FUNCTION)ServiceHandlerFN))) return;

    NotifySCM(SERVICE_START_PENDING, 0, 1);

    ServiceInit();

    NotifySCM(SERVICE_START_PENDING, 0, 2);

    if ((hDoneEvent=CreateEvent(NULL,FALSE,FALSE,NULL)) == 0)   return;

    NotifySCM(SERVICE_START_PENDING, 0, 3);

    ServiceStart();

    NotifySCM(SERVICE_RUNNING, 0, 0);

    WaitForSingleObject(hDoneEvent, INFINITE);

    ServiceStop();

    CloseHandle(hDoneEvent);
    return;
}



bool WindowsServiceApplication::NormalMain(){
    if (!ServiceInit())         return False;

    printf("\n INITIALISATION  COMPLETED \n");

    if (!ServiceStart())         return False;

    printf("\nPRESS ENTER TO STOP SERVICE\n");
    char buffer[512];
    gets(buffer);

    if (!ServiceStop())          return False;
    return True;
}


bool WSAMain(WindowsServiceApplication *wsa,int argc,char **argv){
    ::wsa = wsa;
    if (argc >= 2){
        if (strncmp(argv[1],"INSTALL",7)==0){
            return wsa->Install();
        }
        if (strncmp(argv[1],"APP",3)==0){
            argv[1][0] = 0;
            return wsa->NormalMain();
        }
    }

    SERVICE_TABLE_ENTRY ServiceTable[] = {
      {wsa->serviceName.BufferReference(), (LPSERVICE_MAIN_FUNCTION)ServiceMainFN},
      {NULL, NULL}
    };
    // connect to  the service control manager
    StartServiceCtrlDispatcher(ServiceTable);
    return True;
}

#else

bool WSAMain(WindowsServiceApplication *wsa,int argc,char **argv){
#if defined _WIN32
    return wsa->NormalMain();
#elif defined _LINUX

    if (!wsa->ServiceInit())         return False;

    printf("\n INITIALISATION  COMPLETED \n");

    if (!wsa->ServiceStart())        return False;

    signal(SIGTERM, LXServiceStop);
    while(run){
        SleepSec(1.0);
    }

    if (!wsa->ServiceStop())         return False;

#else
    return True;
#endif
}



#endif
