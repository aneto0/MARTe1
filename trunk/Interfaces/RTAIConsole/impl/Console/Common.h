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
#ifndef COMMON_H_
#define COMMON_H_

// Location of kallsyms file (default:/proc/kallsyms). 
#define KALL_SYMS_LOC "/proc/kallsyms"

// Log file filename. 
#define LOG_LOC "log"

// MotD filename. 
#define MOTD_LOC "motd"

// History log filename. 
#define HIST_LOC ".history"
// Max number of commands in history
#define HIST_MAX_LENGTH 100

// Alias file filename
#define ALIAS_LOC ".RCaliases"


// Maximum length of a command line. 
#define MAX_CMDLINE_DIM 200

// Maximum length of a kallsyms row. 
#define MAX_KALL_ROW_DIM 100

// Location of FIFOs. 
#define RTAICIN "/tmp/RTAICIn"
#define RTAICOUT "/tmp/RTAICOut"

// Location of FCOMM killfile. 
#define KILLFCOMMFILE "/tmp/exit"

#endif /*COMMON_H_*/
