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
#ifndef PROCESSES_H
#define PROCESSES_H

/**
 * @file
 * A class to handle processes and retrieve process information
 */
#include "System.h"
#include "Threads.h"


#if defined(_OS2)
#include <DosQuerySysState.h>
#endif

class Processes {
public:

    /** Returns the current's process id. */
    static uint32 GetThisPid(){
#if defined(_OS2)
        PTIB ptib;
        PPIB ppib;
        DosGetInfoBlocks(&ptib,&ppib);
        return ppib->pib_ulpid;
#elif defined(_WIN32)
        return GetCurrentProcessId();
#elif defined(_VXWORKS)
        return 0;
#elif defined(_RTAI)
        return (uint32) rt_whoami();
#elif (defined(_SOLARIS) || defined(_LINUX) || defined(_MACOSX)) && (defined USE_PTHREAD)
    return (uint32) getpid();
#else
    return 0;
#endif
    }

    /** Retrieves the name of this application
        It is obtained by parsing argv[0]
        it does not work if REAL_MAIN is defined
    */
    static const char *ProcessName(){
        return GetProcessName();
    }

    /** Launch a specified command as a newly detached process.
        @param command A pointer to a string cpntaining the command to execute.
        @param args A pointer to a string containing the arguments of the command.
        @return The task id of the new process or 0 if the process
        has not been created.
     */
    static uint32 Detach(const char *command,const char *args,const char *path = NULL){
#if defined(_OS2)
        char buffer[512];
        sprintf(buffer,"%s%c%s%c",command,0,args,0);
        RESULTCODES rtmmmdrc;
        APIRET ret = DosExecPgm(NULL,0,EXEC_BACKGROUND,(PSZ)buffer,NULL,&rtmmmdrc,(PSZ)command);
        if (ret==0){
            return rtmmmdrc.codeTerminate;
        } else return 0;
#elif defined(_WIN32)
        char buffer[512];
        sprintf(buffer,"%s %s",command,args);
        STARTUPINFO info;
        PROCESS_INFORMATION ps;
        GetStartupInfo(&info);
        info.lpReserved = NULL;
        if (CreateProcess((LPCTSTR)NULL,(LPTSTR)buffer,NULL,NULL,FALSE,
                          DETACHED_PROCESS,NULL,path,&info,&ps)==FALSE) return 0;
        return ps.dwProcessId;
#elif defined(_VXWORKS)
        return 0;
#elif (defined(_RTAI) || defined(_LINUX) || defined(_MACOSX))
        printf("Processes.h: Detach: Warning Not Implemented\n");
        return 0;
#elif (defined(_SOLARIS)) && (defined USE_PTHREAD)
    return execv(command,(char *const *)args);
#else
    return 0;
#endif
    }

    /** Launch a specified command as a newly detached process ??? As above. ???.
        @param command A pointer to a string cpntaining the command to execute.
        @param args A pointer to a string containing the arguments of the command.
        @param path A pointer to a string containing the path to the command.
        @return The task id of the new process or 0 if the process
        has not been created.
     */
    static uint32 Start(const char *command,const char *args,const char *path=NULL){
#if defined(_OS2)
#error does not handle path
        uint32 task_id;
        STARTDATA SData       = {0};
        SData.Length  = sizeof(STARTDATA);
        SData.Related = SSF_RELATED_INDEPENDENT; /* start an independent session */
        SData.FgBg    = SSF_FGBG_FORE;           /* start session in foreground  */
        SData.TraceOpt = SSF_TRACEOPT_NONE;      /* No trace                     */
        SData.PgmTitle = (PSZ)command;
        SData.PgmName = (PSZ)command;
        SData.PgmInputs = (PSZ)args;                     /* Keep session up           */
        SData.TermQ = 0;                            /* No termination queue      */
        SData.Environment = 0;                      /* No environment string     */
        SData.InheritOpt = SSF_INHERTOPT_SHELL;     /* Inherit shell's environ.  */
        SData.SessionType = SSF_TYPE_WINDOWABLEVIO; /* Windowed VIO session      */
        SData.IconFile = 0;                         /* No icon association       */
        SData.PgmHandle = 0;
        /* Open the session VISIBLE and MAXIMIZED */
        SData.PgmControl = SSF_CONTROL_VISIBLE | SSF_CONTROL_MAXIMIZE;
        SData.InitXPos  = 30;     /* Initial window coordinates              */
        SData.InitYPos  = 40;
        SData.InitXSize = 200;    /* Initial window size */
        SData.InitYSize = 140;
        SData.Reserved = 0;
        UCHAR     achObjBuf[256] = {0};     /* Error data if DosStart fails */
        SData.ObjectBuffer  = achObjBuf; /* Contains info if DosExecPgm fails */
        SData.ObjectBuffLen = (ULONG) sizeof(achObjBuf);

        ULONG     ulSessID    = 0;          /* Session ID returned          */
        APIRET rc = DosStartSession(&SData, &ulSessID, &task_id);  /* Start the session */

        if (rc == 0) return task_id;
        return 0;

#elif defined(_WIN32)
        char buffer[512];
        uint32 task_id;
        if (path != NULL)
            if (path[0] == 0)
                path = NULL;

        sprintf(buffer,"%s %s",command,args);
        STARTUPINFO info;
        PROCESS_INFORMATION ps;
        GetStartupInfo(&info);
        info.lpReserved = NULL;
        if (CreateProcess((LPCTSTR)NULL,buffer,NULL,NULL,FALSE,
                          CREATE_NEW_CONSOLE,NULL,path,&info,&ps)==FALSE) return 0;
        return  ps.dwProcessId;
#elif defined(_VXWORKS)
        return 0;
#elif (defined(_RTAI) || defined(_LINUX) || defined(_MACOSX))
        printf("Processes.h: Detach: Warning Not Implemented\n");
        return 0;
#elif (defined(_SOLARIS)) && (defined USE_PTHREAD)
    return execv(command,(char *const *)args);
#else
    return 0;
#endif
    }

    /** Kill the process specified by the task id*/
    static bool Kill(uint32 task_id){
#if defined(_OS2)
        if (DosKillProcess(DKP_PROCESS, task_id) != 0 ) return False;
#elif defined(_WIN32)
        if (TerminateProcess(OpenProcess(PROCESS_TERMINATE,FALSE,task_id),0)==FALSE) return False;
#elif defined(_VXWORKS)
        return False;
#elif (defined(_RTAI) || defined(_LINUX) || defined(_MACOSX))
        printf("Processes.h: Kill: Warning Not Implemented\n");
        return 0;
#elif (defined _SOLARIS) && (defined USE_PTHREAD)
    return (kill((pid_t)task_id,0)==0);
#endif
        return True;
    }

    /** Check whether a process is alive */
    static bool IsAlive(uint32 task_id){
#if defined(_OS2)
        char buffer[1024];
        APIRET rc;
        rc = DosQuerySysState(0x1,0,task_id,0,buffer,sizeof(buffer));
        return (rc == 0);
#elif defined(_WIN32)
        uint32 exitCode;
        HANDLE h = OpenProcess(PROCESS_QUERY_INFORMATION,FALSE,task_id);
        if (h==NULL) return False;
        if (GetExitCodeProcess(h,(LPDWORD)&exitCode)==TRUE){
            CloseHandle(h);
            return (exitCode==STILL_ACTIVE);
        }
        CloseHandle(h);
        return False;
#elif defined(_VXWORKS)
        return False;
#elif (defined(_RTAI) || defined(_LINUX) || defined(_MACOSX))
        printf("Processes.h: IsAlive: Warning Not Implemented\n");
        return False;
#elif (defined(_SOLARIS)    ) && (defined USE_PTHREAD)
    return (sigsend(P_PID,task_id,0)==0);
#else
    return 0;
#endif
    }


    /** Causes the machine to reboot. */
    static void Reboot(){
#if defined(_OS2)
#elif defined(_WIN32)
        HANDLE pHandle = GetCurrentProcess();
        HANDLE tHandle;
        OpenProcessToken(pHandle,TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &tHandle);
        LUID luid;
        LookupPrivilegeValue("","SeShutdownPrivilege",&luid);
        TOKEN_PRIVILEGES tkp;
        tkp.PrivilegeCount = 1;
        tkp.Privileges[0].Luid = luid;
        tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
        AdjustTokenPrivileges(tHandle,FALSE,&tkp,0,0,0);
        ExitWindowsEx(EWX_FORCE | EWX_REBOOT,0);
#elif defined(_VXWORKS)
        vplsCrateRst();
#elif defined(_LINUX) || defined(_MACOSX)
        reboot(RB_AUTOBOOT);
#elif defined(_RTAI)
//Use emergency_restart() for reboot, it can be called from interrupt context, unlike machine_restart().
        emergency_restart();
#elif ((defined _SOLARIS)) && (defined USE_PTHREAD)
        reboot(RB_AUTOBOOT,NULL);
#else
#endif
    }
};

#endif

