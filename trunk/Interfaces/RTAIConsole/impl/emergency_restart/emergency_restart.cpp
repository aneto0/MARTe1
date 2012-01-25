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
#include "System.h"
#include "FString.h"
#include "File.h"
#include "api/RTAIConsoleAPI.h"
#include "module/RTAIConsole.h"

#define KALL_SYMS_LOC "/proc/kallsyms"

int main(int argc, char** argv) {
    File KAllSyms;
	
    FString KAllSymsBuffer;
    FString KAllSymsRow;
    FString Token;
	
    KAllSyms.OpenRead(KALL_SYMS_LOC);
    KAllSymsBuffer.SetSize(80000);
    KAllSyms.Seek(0);
    KAllSyms.GetToken(KAllSymsBuffer,"");
    KAllSymsBuffer.Seek(0);
		
    while (KAllSymsBuffer.GetLine(KAllSymsRow)) {
        KAllSymsRow.Seek(0);
	if (strncmp(KAllSymsRow.Buffer()+11,"emergency_restart",17)==0){
	    // Row Tokenizing
	    KAllSymsRow.GetToken(Token," \t\n\r");
	    int res[2];
            res[0] = strtoll(Token.Buffer(),NULL,16);
            res[1] = 0;
            CallRemoteFunction(res);
	}
	KAllSymsRow.SetSize(0);
    }
    return 0;
}
