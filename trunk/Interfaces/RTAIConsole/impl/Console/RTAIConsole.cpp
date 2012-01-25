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
#include <signal.h>

#include "Common.h"
#include "Shell.h"
#include "FString.h"
#include "FileLogger.h"

FileLogger Log(LOG_LOC);

void ctrlc_catcher(int sig){
	printf("Please use quit or exit command to end RTAIConsole.\nRemember first to unload every module which needs delete_all_deconstructor!\n");
}

int main(int argc,char **argv) {

	(void) signal(SIGINT, ctrlc_catcher);

        FString command;
        command.SetSize(0);
        command="";
        if(argc > 1){
        	int i=0;
        	for (i=1;i<argc;i++){
        		command+=argv[i];
        		command+=" ";
        	}
        	command+="\n";
        }
	Shell RTAIShell(command.Buffer());
	
	// Main shell cycle
	RTAIShell.Start();
}


