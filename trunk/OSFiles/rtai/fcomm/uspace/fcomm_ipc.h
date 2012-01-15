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
#ifndef FCOMM_IPC_H
#define	FCOMM_IPC_H

/**
 * Error initiating the main task
 */
#define ERROR_MAIN_TASK -2

int     init_shm(void);
void    clean_close_shm(void);
int     get_rt_free_uid(void);
int     initDebugSocket(const char *remoteLoggerAddr, int remoteLoggerPort, int level);
void    closeDebugSocket(void);
void    logUDPStandardMessage(int errorLevel, char *msg);
int     loggerVerboseLevel;
enum    verboseLevels{
    VERBOSE_ALL,
    VERBOSE_ERRORS,
    VERBOSE_WARNINGS,
    VERBOSE_INFORMATIONS    
};
#endif

