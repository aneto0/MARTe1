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
 * @brief Stopping/Starting of services in Windows
 */
#if !defined (WINDOWS_SERVICE_APPLICATION)
#define WINDOWS_SERVICE_APPLICATION

#include "System.h"
#include "FString.h"


class WindowsServiceApplication;

extern "C" {
    /** */
    bool WSAMain(WindowsServiceApplication *wsa,int argc,char **argv);
}

/** this class allow simple building of window services
    this only works on windows!!
    Only one instance of this class derivative can be used since the mechanism
    uses a global variable    */
class WindowsServiceApplication{

friend bool WSAMain(WindowsServiceApplication *wsa,int argc,char **argv);

#if defined (_MSC_VER)
private:

    HANDLE                  hDoneEvent;

    HANDLE                  hThread;

    DWORD                   dwCurrentState;

    SERVICE_STATUS_HANDLE   hService;

public:
    BOOL                    NotifySCM(DWORD dwState, DWORD dwWin32ExitCode,DWORD dwProgress);

    void                    ServiceHandler(DWORD fdwControl);

    void                    ServiceMain(DWORD dwArgc, LPTSTR *lpszArgv);


#endif
    /** */
    FString serviceName;

    /** */
    FString title;

private:
    /** */
    int Install();

    /** */
    int Start();

private:
    /** replace this with the main of the application */
    virtual bool ServiceInit()=0;

    /** replace this with the start service */
    virtual bool ServiceStart()=0;

    /** replace this with the stop service */
    virtual bool ServiceStop()=0;

    /** */
    bool NormalMain();

public:
    /** */
    bool Main(int argc,char **argv){
        return WSAMain(this,argc,argv);
    }

    /** */
    WindowsServiceApplication(const char *serviceName,const char *title){
#if defined (_MSC_VER)
        hDoneEvent      = NULL;
        hThread         = NULL;
#endif
        this->serviceName = serviceName;
        this->title       = title;
    }

    virtual ~WindowsServiceApplication(){};
};

#endif
