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
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "CommandHistory.h"
#include "Shell.h"
#include "FileLogger.h"
#include "Common.h"
#include "ReaderThread.h"
#include "Autocomplete.h"
#include "Wildcards.h"
#include "VarManager.h"
#include "AliasManager.h"

// Kernel and api include
#include "api/RTAIConsoleAPI.h"
#include "module/RTAIConsole.h"


extern FileLogger Log;

bool Shell::inKernelMode = False;
void __thread_decl CallRemoteFunctionThread(void *KernelCallArray){
    CallRemoteFunction((int *)KernelCallArray);
    Shell::inKernelMode = False;
    printf("Hit any key\n");
}

Shell::Shell(const char *startCommand):con(PerformCharacterInput) {
   
    if(startCommand != NULL && strlen(startCommand) != 0){
        CmdLine = startCommand;
    }
// brute hack to stop console echo
    struct termio shellNewSettings;
    if(ioctl(fileno(stdin), TCGETA, &shellOriginalSettings) >= 0){
        shellNewSettings = shellOriginalSettings;
        shellNewSettings.c_lflag &= ~ECHO;
        shellNewSettings.c_cc[VMIN] = 1;
        shellNewSettings.c_cc[VTIME] = 0;
        ioctl(fileno(stdin), TCSETAW, &shellNewSettings);
    }
    // LexicalAnalyzer configuration
    la.AddSeparators(" :\n=");
    la.AddTerminals(" ()\":\n=");

    // Set initial path (for motd location)
    char path[256];
    MotdPath=getcwd(path,256);
    MotdPath+="/";
    MotdPath+=MOTD_LOC;


    // Shell init
    SwitchCol(Grey,Black);


    con.Printf(" ");
	
    con.Clear();
    PrintMOTD();
    killConsole=false;

    // Start fifo reader thread
    bool res=CheckFifo();
    if (!res) {
            con.Printf("FIFO files not present in %s and %s. Aborting.",RTAICIN,RTAICOUT);
            killConsole=true;
            return;
    }
    
    if(startCommand != NULL && strlen(startCommand) != 0){        
        InterpretCommandLine();
    }
    
    Threads::BeginThread(ReaderThread,(void*)&con,THREADS_DEFAULT_STACKSIZE,"ReaderThread", XH_NotHandled, 0x1);
}


Shell::~Shell() {
    con.Printf("\n\n");
    ioctl(fileno(stdin), TCSETAW, &shellOriginalSettings);
}


void Shell::PrintMOTD() {
	File motd;
	FString Line;
	bool res;
		
	res=motd.OpenRead(MotdPath.Buffer());
	
	if(res==false) return; //motd non-existent
	
	while (motd.GetLine(Line)) {
		con.Printf("%s\n",Line.Buffer());
		Line.SetSize(0);
	}
}


void Shell::DeleteChars(int NChars){
	for (int i=0;i<NChars;i++) {
		con.Printf("\b \b");
	}
}


void Shell::WritePrompt() {
	char path[256];
	
	SwitchCol(Red,Black);
    con.Printf("%s > ",getcwd(path,256));
    SwitchCol(Grey,Black);
}


bool Shell::PerformInput() {
	char c;
	char buffer[MAX_CMDLINE_DIM];
	uint32 CmdLineLength=MAX_CMDLINE_DIM;
	int i=0;
	uint32 one=1;
	FString tmp;

	i=0;
	con.Read(&c,one);

	while (c!='\n') {
		switch (c) {
			case BSPACE: //backspace
				if (i==0) {
					break;
				}
				DeleteChars(1);
				i--;
				break;

			
			case ESC: //escape sequence!
				con.Read((void*)&c,one);
				//arrow up/down
				if (c=='[') {
					con.Read((void*)&c,one);
					if (c=='A') { //up
						DeleteChars(i);
						tmp.SetSize(0);
						tmp=CmdHistory.GetPrev();
						con.Printf("%s",tmp.Buffer());
						
						//fill buffer
						tmp.Seek(0);
						for (int j=0;j<=tmp.Size();j++){
							const char* p;
							p=tmp.Buffer();
							buffer[j]=p[j];
							i=j;
						}
					}
					if (c=='B') { //down
						DeleteChars(i);
						tmp.SetSize(0);
						tmp=CmdHistory.GetNext();
						con.Printf("%s",tmp.Buffer());
						
						//fill buffer
						tmp.Seek(0);
						for (int j=0;j<=tmp.Size();j++){
							const char* p;
							p=tmp.Buffer();
							buffer[j]=p[j];
							i=j;
						}
					}
				}
				break;
			
			case '\t':
				//autocomplete
				//check if cd command
				if (buffer[0]=='c' && buffer[1]=='d' && buffer[2]==' ') {
					tmp.SetSize(0);
					const char *p;
					p=&buffer[3];
					tmp=Autocomplete(p,i-3);
					if (tmp=="**ERROR**") break;
					if (strncmp(tmp.Buffer(),"**MULTI**",9)==0) {// multiple completion
						const char *t;
						t=tmp.Buffer();
						con.Printf("\nMultiple possible completion. Choices:\n");
						con.Printf("%s",&t[10]);
						WritePrompt();
						for (int j=0;j<i;j++) {
							con.Printf("%c",buffer[j]);
						}
						break;
					}
					//Single completion
					tmp.SetSize(tmp.Size()-1);
					
					//clean cmdline
					DeleteChars(i);
					// write command and fill buffer
					p=tmp.Buffer();
					con.Printf("cd %s",p);
					buffer[0]='c';
					buffer[1]='d';
					buffer[2]=' ';
					for (int j=0;j<=tmp.Size();j++){
						buffer[j+3]=p[j];
					}
					i=tmp.Size()+3;			
				}
				
				break;
			
			default:
				buffer[i]=c;
				printf("%c",c);
				i++;
				break;
		}
		con.Read((void*)&c,one);
	}
	buffer[i]='\n';
	buffer[i+1]='\0';
	con.Printf("\n");
	CmdHistory.ResetCurrent();
	char *p=buffer;
	
	if (*p=='\n'){
		return false; //noop
	}
	
	//strip whitespaces and \ns at the beginning of the command
	while (*p==' ') {
		if (*p=='\0') return false; //noop
		p++;
	}
	
	if (*p=='#'){
		return false; // row starting with #s are comments
	}
	
	CmdLine=p;
	//con.Printf("\nCmdLine={%s}\n",CmdLine.Buffer());
	return true;
}


void Shell::KernelInput() {
	char c;
	int flag=0;
	uint32 one=1;
	
    int fd = open(RTAICIN, O_RDWR);
	
	while (inKernelMode) {	
		con.Read((void*)&c,one);
		
		//check :c combination
		if (c==':' && flag==0) {
			flag++;
		} else if (c=='c' && flag==1) {
			flag++;
		} else if (c=='\n' && flag==2) {
			return; //:c ok, exit kernel input mode
		} else {
			flag=0;
		}
		// ----
		
		if (flag==0) {		
			write(fd,&c,1);
			c='\n';
			write(fd,&c,1);
		}
	}
	close(fd);
		
}


bool Shell::CheckInternalCommand() {
	FString LogString;
	FString IntCmd;
	LA_TokenData *tk_data;
	int DummyArray[RTAIFunctionNumberOfParameters];
	
	for (int i=0;i<RTAIFunctionNumberOfParameters;i++)
		DummyArray[i]=0;

	// Look if it is an internal command	
	IntCmd.SetSize(0);
	
	// 07-01-08 => Support for "kernel input mode"
	const char *p=CmdLine.Buffer();
	if (p[0]==':' && p[1]=='c' && p[2]=='\n') {
		con.Printf("Entering kernel input mode...\n");
		KernelInput();
		return true;
	}

	tk_data=la.GetToken(CmdLine);
	// Consume the first spare \n
	if (tk_data->Token()==LF) {
		delete tk_data;
		tk_data=la.GetToken(CmdLine);
	}
	
	if (tk_data->Token()!=LATV_Ident) {
		return false; //It isn't an ident, can't be an internal command
	}

	IntCmd=tk_data->Data();
	delete tk_data;
	tk_data=la.GetToken(CmdLine);
	if ((tk_data->Token()!=SPACE) && (tk_data->Token()!=LF) && (tk_data->Token()!=CR)) {
		//Next char isn't a space nor an enter => not an internal command!
		return false;
	}
	delete tk_data;
	
	// At this point we have an ident and a space -> we can check for internal command
	
	/*********************************************************
	* QUIT COMMAND
	**********************************************************/
	if ((IntCmd=="quit") || (IntCmd=="exit")) {
		killConsole=true;
		return true;
	}
	
	
	/*********************************************************
	* REBOOT COMMAND
	**********************************************************/
	if (IntCmd=="reboot") {
		CmdLine.SetSize(0);
		CmdLine = "::emergency_restart";
		InterpretCommandLine();
		return true;
	}
	
	/*********************************************************
	* MOTD COMMAND
	**********************************************************/
	if (IntCmd=="motd") {
		PrintMOTD();
		LogString.Printf("Executed motd internal command.");
		Log.WriteLog(&LogString);
		return true;
	}
	
	/*********************************************************
	* LS COMMAND
	**********************************************************/
	if (IntCmd=="ls") {
		system("ls");
		LogString.Printf("Executed ls internal command.");
		Log.WriteLog(&LogString);
		LogString.SetSize(0);
		return true;
	}
	
	/*********************************************************
	* PWD COMMAND
	**********************************************************/
	if (IntCmd=="pwd") {
		char path[256];
		con.Printf("%s\n",getcwd(path,256));
		LogString.Printf("Executed pwd internal command. Current directory: %s.",path);
		Log.WriteLog(&LogString);
		LogString.SetSize(0);
		return true;
	}
	
	/*********************************************************
	* CD COMMAND
	**********************************************************/
	if (IntCmd=="cd") {
		int res;
		char path[256];
		
		tk_data=la.GetToken(CmdLine);
		if ( (tk_data->Token()!=LATV_Ident) && (tk_data->Token()!=LATV_Number)) { //It must ba an identifier!
			return false;
		}
		
		res=chdir(tk_data->Data());
		if (res==-1) {
			con.Printf("%s\n",strerror(errno));
			return true;
		}
		
		LogString.Printf("Executed cd internal command. New directory: %s.",getcwd(path,256));
		Log.WriteLog(&LogString);
		LogString.SetSize(0);
		return true;
	}
	
	/*********************************************************
	* LOAD/INSMOD COMMAND
	**********************************************************/
	if ((IntCmd=="load") || (IntCmd=="insmod")) {
		FString modload;
		int res;
		long long int address;
		FString Module;
		FString Function;
		Function="init_all_constructors";
		FString modname;
		
		tk_data=la.GetToken(CmdLine);
		if (tk_data->Token()!=LATV_Ident) { //It must ba an identifier!
			return false;
		}
		//inserting module
		modname.Printf("%s",tk_data->Data());
		LogString.Printf("Executed insmod/load internal command, loading module %s.",tk_data->Data());
		Log.WriteLog(&LogString);
		LogString.SetSize(0);
		modload.Printf("insmod %s",modname.Buffer());
		res=system(modload.Buffer());
		if (res!=0) {
			con.Printf("Error inserting module, check dmesg.\n");
			return true;
		}
		//init_all_cons
		LogString.Printf("Executed insmod internal command, starting init_all_constructors.");
		Log.WriteLog(&LogString);
		// Delete .ext
		modname.Seek(0);
		modname.GetToken(Module,".");
		address=FindFunctionAddress(Function,Module);
		if (address<(long long int)1) {
			con.Printf("Could not find module's init_all_constructors. Is it an RTAI module?");
		} else {
			DummyArray[0]=(int)address;
			CallRemoteFunction(DummyArray);
		}
		return true;
	}
	
	/*********************************************************
	* UNLOAD/RMMOD COMMAND
	**********************************************************/
	if ((IntCmd=="unload") || (IntCmd=="rmmod")) {
		FString modload;
		int res;
		long long int address;
		FString Module;
		FString Function;
		Function="delete_all_deconstructors";
		FString modname;
		
		tk_data=la.GetToken(CmdLine);
		if (tk_data->Token()!=LATV_Ident) { //It must ba an identifier!
			return false;
		}
		modname.Printf("%s",tk_data->Data());
		
		// delete_all_cons
		LogString.Printf("Executed rmmod/unload internal command, starting delete_all_deconstructors.");
		Log.WriteLog(&LogString);
		// Delete .ext
		modname.Seek(0);
		modname.GetToken(Module,".");
		address=FindFunctionAddress(Function,Module);
		if (address<(long long int)1) {
			con.Printf("Could not find module's delete_all_deconstructors. Is it an RTAI module?\n");
		} else {
			DummyArray[0]=(int)address;
			CallRemoteFunction(DummyArray);
		}
		
		//removing module
		LogString.Printf("Executed rmmod/unload internal command, unloading module %s.",tk_data->Data());
		Log.WriteLog(&LogString);
		LogString.SetSize(0);
		modload.Printf("rmmod %s",modname.Buffer());
		res=system(modload.Buffer());
		if (res!=0) {
			con.Printf("Error removing module, check dmesg.\n");
		}
		return true;
	}
	
	/*********************************************************
	* LIST COMMAND
	**********************************************************/
	if (IntCmd=="list") {
		bool res;
		FString ModuleName;

		CmdLine.GetToken(ModuleName,"",NULL," \n");
		res=FindAllModulesFunction(ModuleName);
		if (res==false) { //if we don't have results...
			con.Printf("No function exported by %s. Have you loaded the module before issuing list?\n",ModuleName.Buffer());
		}
		return true;
	}
	
	/*********************************************************
	* EXEC COMMAND
	**********************************************************/
	if (IntCmd=="exec") {
		int res;
		FString ExeCmd;
		
		CmdLine.GetToken(ExeCmd,"",NULL,"\n");
		
		LogString.Printf("Executed external shell command \"%s\".",ExeCmd.Buffer());
		Log.WriteLog(&LogString);
		LogString.SetSize(0);
		
		res=system(ExeCmd.Buffer());

		return true;
	}
	
	/*********************************************************
	* DMESG COMMAND
	**********************************************************/
	if (IntCmd=="dmesg") {
		system("dmesg | tail -n 50");
		return true;	
	}
	
	/*********************************************************
	* LSMOD COMMAND
	**********************************************************/
	if (IntCmd=="lsmod") {
		system("lsmod");
		return true;	
	}
	
	/*********************************************************
	* CLS COMMAND
	**********************************************************/
	if (IntCmd=="cls") {
		con.Clear();
		return true;	
	}
	
	/*********************************************************
	* ALIAS COMMAND
	**********************************************************/
	if (IntCmd=="alias") {
		FString tmp = AM.DumpAliases();
		con.Printf("%s\n",tmp.Buffer());
		return true;	
	}
	
	/*********************************************************
	* RUN COMMAND
	**********************************************************/
	if (IntCmd=="run") {
		File Script;
		FString ScriptBuff;
		bool res;
		
		tk_data=la.GetToken(CmdLine);
		if (tk_data->Token()!=LATV_Ident) { //It must ba an identifier!
			return false;
		}
		res=Script.OpenRead(tk_data->Data());
		
		if (res==false) {
			con.Printf("Error loading script, script file not found.\n");
			return true;
		}
		
		LogString.Printf("Running script %s",tk_data->Data());
		Log.WriteLog(&LogString);
		LogString.SetSize(0);
		
		Script.Seek(0);
		Script.GetToken(ScriptBuff,"");
		ScriptBuff.Seek(0);
		
		int i=0;
		res=true;
		CmdLine.SetSize(0);
		while (ScriptBuff.GetLine(CmdLine)) {
			// 29-01-08 => "GetLine" doesn't put a \n at the end of the FString
			//			   causing problems to the lexical analyzer! (FIXED)
			CmdLine+='\n';
			
			const char* p;
			p=CmdLine.Buffer();
			CmdLine.Seek(0);	
			i++;
			if ((*p!='#')&&(*p!='\n')) {
				bool row=true;
				row=InterpretCommandLine();
				if (row==false) {
					con.Printf("Error in script at line %d.\n",i);
					LogString.Printf("Error in script %s at line %d, stopping.",tk_data->Data(),i);
					Log.WriteLog(&LogString);
					LogString.SetSize(0);
					return true;
				}
			}
			CmdLine.SetSize(0);
		}
		
		LogString.Printf("Finished running script %s",tk_data->Data());
		Log.WriteLog(&LogString);
		
		
		return true;	
	}
	
	/*********************************************************
	* KILLRTAI COMMAND
	**********************************************************/
	if (IntCmd=="killrtai") {
		FString KillCmd;
		
		KillCmd.Printf("touch %s",KILLFCOMMFILE);
		system(KillCmd.Buffer());
		
		LogString.Printf("Executed killrtai internal command.");
		Log.WriteLog(&LogString);
		return true;
	}
	
	/*********************************************************
	* SET (VARIABLE) COMMAND
	**********************************************************/
	if (IntCmd=="set") {
		FString VarName;
		int VarValue;
		
		tk_data=la.GetToken(CmdLine);
		if (tk_data->Token()!=LATV_Ident) {
			con.Printf("Wrong variable name!\n");
			return true;
		}
		VarName=tk_data->Data();
		delete tk_data;
		
		tk_data=la.GetToken(CmdLine);
		if (*(tk_data->Data())!='=') {
			con.Printf("Syntax error!\n");
			return true;
		}
		delete tk_data;
		
		tk_data=la.GetToken(CmdLine);
		if (tk_data->Token()==LATV_Number) {
			VarValue=atoi(tk_data->Data());
		} else if (tk_data->Token()==LATV_Ident) {
			// Look if ident is an existing variable. If so copy value.
			Variable* copy=VM.SeekVariable(tk_data->Data());
			if (copy!=NULL) {
				bool res=VM.AddVariable(VarName,copy->Data);
				if (res==false) {
					con.Printf("Variable already existent!\n");
				}
				delete tk_data;
				return true;
			}
			con.Printf("%s doesn't exists!\n",tk_data->Data());
			delete tk_data;
			return true;
		} else {
			con.Printf("Syntax error, set accepts only integers or other variables!\n");
			delete tk_data;
			return true;
		}
		delete tk_data;
		
		bool res=VM.AddVariable(VarName,VarValue);
		if (res==false) {
			con.Printf("Variable already existent!\n");
			return true;
		}
		
		return true;
	}
	
	
	/*********************************************************
	* UNSET (VARIABLE) COMMAND
	**********************************************************/
	if (IntCmd=="unset") {
		tk_data=la.GetToken(CmdLine);
		if (tk_data->Token()!=LATV_Ident) {
			con.Printf("Unset expects a variable name!\n");
			return true;
		}
		
		bool res=VM.DelVariable(tk_data->Data());
		
		if (!res) {
			con.Printf("%s does not exist!\n",tk_data->Data());
		}
		delete tk_data;
		return true;
	}
	
	
	/*********************************************************
	* PRINT (VARIABLE) COMMAND
	**********************************************************/
	if (IntCmd=="print") {
		tk_data=la.GetToken(CmdLine);
		if (tk_data->Token()!=LATV_Ident) {
			con.Printf("Print expects a variable name!\n");
			return true;
		}
		
		//Check if * used
		if (*(tk_data->Data())=='*'){
			Variable* pippo=VM.ReturnFirst();
			while (pippo!=NULL){
				con.Printf("Name=%s \t Value=%d\n",pippo->Name.Buffer(),pippo->Data);
				pippo=pippo->next;
			}
			return true;
		}
		
		
		Variable* res=VM.SeekVariable(tk_data->Data());
		delete tk_data;
		if (res!=NULL){
			con.Printf("%d\n",res->Data);
			return true;
		} else {
			con.Printf("non-existant variable!\n");
			return true;
		}
	}
	
	
	/*********************************************************
	* HELP COMMAND
	**********************************************************/
	if ((IntCmd=="help") || (IntCmd=="?")) {
		con.Printf("** BUILTIN COMMAND LIST **\n");
		con.Printf("? or help                  ==> this guide.\n");
		con.Printf("motd                       ==> prints on screen the content of the motd file.\n");
		con.Printf("load or insmod MODULENAME  ==> load the module specified in the kernel.\n");
		con.Printf("unload or rmmod MODULENAME ==> unloads the module specified from the kernel.\n");
		con.Printf("list MODNAME::FNAME        ==> lists function FNAME of module MODNAME.\n");
		con.Printf("                               It supports * and ? wildchars.\n");
		con.Printf("                               e.g.: list rt*::*rintk.\n");
		con.Printf("exec COMMANDLINE           ==> executes COMMANDLINE in sh.\n");
		con.Printf("run SCRIPTFILE             ==> executes script SCRIPTFILE.\n");
		con.Printf("pwd                        ==> shows current path.\n");
		con.Printf("cd PATH                    ==> changes current directory to PATH.\n");
		con.Printf("ls                         ==> shows files/dirs in current directory.\n");
		con.Printf("dmesg                      ==> shows last 50 entries of dmesg file.\n");
		con.Printf("lsmod                      ==> shows loaded kernel modules.\n");
		con.Printf("killrtai                   ==> kills FCOMM process.\n");
		con.Printf("reboot                     ==> reboots the machine using emergency_restart.\n");
		con.Printf("cls                        ==> clears screen.\n\n");
		con.Printf("set VARNAME=VALUE          ==> assigns to VARNAME the value VALUE (integer).\n");
		con.Printf("unset VARNAME              ==> deletes VARNAME.\n");
		con.Printf("print VARNAME              ==> prints contents of VARNAME.\n");
		con.Printf("alias                      ==> echoes on screen defined aliases.\n");
		con.Printf("                               Use * to list all variables defined.\n\n");
		con.Printf(":c                         ==> enters and exits from kernel mode input\n");
		con.Printf("                               (sends keyboard input to FCOMM FIFO)\n\n");
		return true;
	}
	
	// At last check if it is an alias
	FString alias_name=IntCmd;
	FString alias_fnc=AM.CheckAlias(IntCmd);
	if (alias_fnc.Size()!=0) {
		CmdLine=alias_fnc;
		if(InterpretCommandLine()==false){
			con.Printf("Alias %s returned error: check alias file!\n",alias_name.Buffer());
		}
		return true;
	}
	
	// Not a valid command, return error
	con.Printf("Internal Command Syntax Error!\n");
	return true;
}


bool Shell::GetModuleFunc(FString* Module, FString* Function) {
	LA_TokenData *tk_data;
	
	tk_data=la.GetToken(CmdLine);
	//Possibility => if "::" we aren't using a module
	if (tk_data->Token()==COLON) {
		//check if next char is another colon, otherwise error
		delete tk_data;
		tk_data=la.GetToken(CmdLine);
		if (tk_data->Token()!=COLON) return false;
		//Ok, we have a "null" module
		Module->Printf("NULL");
		//If next token isn't an ident, error
		delete tk_data;
		tk_data=la.GetToken(CmdLine);
		if(tk_data->Token()!=LATV_Ident) return false;
		*Function=tk_data->Data();
		return true;
	}
	//Possibility => if ident we ARE using a module
	if (tk_data->Token()==LATV_Ident) {
		*Module=tk_data->Data();
		//Now we need 2 ":", otherwise error
		delete tk_data;
		tk_data=la.GetToken(CmdLine);
		if (tk_data->Token()!=COLON) return false;
		delete tk_data;
		tk_data=la.GetToken(CmdLine);
		if (tk_data->Token()!=COLON) return false;
		//If next token isn't an ident, error
		delete tk_data;
		tk_data=la.GetToken(CmdLine);
		if(tk_data->Token()!=LATV_Ident) return false;
		*Function=tk_data->Data();
		return true;
	}
	// Possibility => none of the above=error
	return false;
}


bool Shell::BuildCommandArray (int *KernelCallArray) {
	FString StringData;
	char *CStringData;
	int NoOfParams=0;
	LA_TokenData *tk_data;
	StringData.SetSize(0);
	FString Type;
	long long int param;
	
	tk_data=la.GetToken(CmdLine);
	
	while(tk_data->Token()!=LATV_EOF && NoOfParams<RTAIMaxNumberParameters) {
		
		Type.SetSize(0);
		switch (tk_data->Token()) {
			case SPACE:
				//Whitespace, just jump over
				break;
			
			/*********************************************************
			* (TYPE_NAME)##...# --> Data Conversion
			**********************************************************/				
			case OPENPAR: //(=data conversion
				//Next data should be an identifier, otherwise error!!
				delete tk_data;
				tk_data=la.GetToken(CmdLine);
				if (tk_data->Token()!=LATV_Ident){
					return false;
				}
				//Save the type in which we want to convert the number
				Type=tk_data->Data();			
				//Now a ) otherwise error 
				delete tk_data;
				tk_data=la.GetToken(CmdLine);
				if (tk_data->Token()!=CLOSEPAR){
					return false;
				}
				//Now a number otherwise error
				delete tk_data;
				tk_data=la.GetToken(CmdLine);
				if (tk_data->Token()!=LATV_Number && tk_data->Token()!=LATV_Float){
					return false;
				}
				param=strtoll(tk_data->Data(),NULL,10);
				//Do the conversion and save it to the array
				if (Type=="int64") { //we pass it as 2 ints
					KernelCallArray[2+NoOfParams]=(int)(param & 0xFFFFFFFFLL);
					NoOfParams++; 
					KernelCallArray[2+NoOfParams]=(int)(param >> 32);
				} else if (Type=="int") {
					KernelCallArray[2+NoOfParams]=(int)(param);
				}
				NoOfParams++;
				break;
			
			/*********************************************************	
			* ##...# --> Simple Number
			**********************************************************/
			case LATV_Number: //we consider it a simple int
				param=strtoll(tk_data->Data(),NULL,10);
				KernelCallArray[2+NoOfParams]=(int)(param);
				delete tk_data;
				NoOfParams++;
				break;
			
			/*********************************************************	
			* ##.## or ##E## --> Float Number
			**********************************************************/
			// Temporarly commented out
			/*case LATV_Float:
				temp2=atof(tk_data->Data());
				break;
			*/
			
			/*********************************************************		
			* "BLA BLA BLA" --> String 
			**********************************************************/
			case QUOTES: //"s
				//We have a string, so we populate an FString
				//till we find another "
				delete tk_data;
				tk_data=la.GetToken(CmdLine);
				while(tk_data->Token()!=QUOTES) {
					StringData+=tk_data->Data();
					StringData+=" ";
					delete tk_data;
					tk_data=la.GetToken(CmdLine);
				}
				delete tk_data;
				//Delete last whitespace
				StringData.SetSize(StringData.Size()-1);
				//Copy string to kernelspace (RTAI Console module API)
				CStringData = (char *)malloc(strlen(StringData.Buffer()) + 1);
				strcpy(CStringData, StringData.Buffer());
				KernelCallArray[2+NoOfParams]=CopyToKernel((int)CStringData, strlen(StringData.Buffer()) + 1);
				NoOfParams++;
				free((void *&)CStringData);
				StringData.SetSize(0);
				break;
				
				
			/*********************************************************	
			* IDENTIFIER --> i.e. string without "s
			**********************************************************/
			case LATV_Ident: //It is an identifier (i.e. a string w/o "s)
                //CStringData = (char *)malloc(strlen(StringData.Buffer()) + 1);
                //strcpy(CStringData, StringData.Buffer());
				//KernelCallArray[2+NoOfParams]=CopyToKernel((int)CStringData, strlen(StringData.Buffer()) + 1);
                //free((void *&)CStringData);
				Variable* tmp=VM.SeekVariable(tk_data->Data());
				if (tmp==NULL){ // error, var non-existant
					con.Printf("Error, variable %s does not exist.\n",tk_data->Data());
					return false;
				}
				delete tk_data;
				KernelCallArray[2+NoOfParams]=tmp->Data;
				NoOfParams++;
				break;
		} //end switch
		
		// Increment number of params
		tk_data=la.GetToken(CmdLine);
	}//end while
	
	KernelCallArray[1] = NoOfParams;
	return true;
}


bool Shell::InterpretCommandLine() {
	FString LogString;
	FString Module;
	FString Function;
	int KernelCallArray[RTAIFunctionNumberOfParameters];
	bool res;
	
	la.Reset();
	
	// Initialize KernelCallArray
	for (int i=0;i<RTAIFunctionNumberOfParameters;i++) 
		KernelCallArray[i]=0; 
	
	CmdLine.Seek(0);
	// Check if internal command
	res=CheckInternalCommand();
	
	if (res==false) { // Could be a kernel call
		CmdLine.Seek(0);
		// Get module & function
		res=GetModuleFunc(&Module,&Function);
                if(Function == "main"){
                    con.Printf("Please use %s::RTAIMain instead of %s::main\n", Module.Buffer(), Module.Buffer());
                    return False;
                }
                else if(res==false) {
			con.Printf("Kernel Command Syntax Error!\n");
			CmdLine.Seek(0);
			return false;
		}
		else {
			KernelCallArray[0]=(int)FindFunctionAddress(Function,Module);
			if (KernelCallArray[0] == 0) { //If we have an invalid address...
				con.Printf("Kernel Command not found. Did you load the appropriate module?\n");
				return false;
			} else {
				// Now we have te to-be-called function's address, let's check the command line parameters
				res=BuildCommandArray(KernelCallArray);
				if (res==false) { //There is an error in the command line syntax
					con.Printf("Kernel Call Command Line Parameter(s) Syntax Error!\n");
					return false;
				} else { //all OK, we can make the kernel call
					LogString.Printf("Executed kernel call at address %x.",KernelCallArray[0]);
					Log.WriteLog(&LogString);
					LogString.SetSize(0);						
                                        inKernelMode = True;
					Threads::BeginThread(CallRemoteFunctionThread,(void*)KernelCallArray,THREADS_DEFAULT_STACKSIZE,"CallRemoteFunctionThread", XH_NotHandled, 0x1);
//					CallRemoteFunction(KernelCallArray);
                                        Shell::KernelInput();
				}
			}
		}
	} //end kernel call's if
	
	//Solves strange behaviour of the lexical analyzer
	la.Reset();
	return true;
}


void Shell::Start() {
	bool res;
	
	while (!killConsole) {
		// Get shell command
		CmdLine.SetSize(0);
		WritePrompt();
		res=PerformInput(); //false if noop
		
		if (res==true) {
			res=InterpretCommandLine();
			CmdLine.SetSize(CmdLine.Size()-1);
			CmdHistory.InsertCommand(CmdLine);
		}
	} //end while
}


long long int Shell::FindFunctionAddress(FString& FunctionName, FString& ModuleName) {
	File KAllSyms;
	
	FString KAllSymsBuffer;
	FString KAllSymsRow;
	RowToken Token;
	
	KAllSyms.OpenRead(KALL_SYMS_LOC);
	KAllSymsBuffer.SetSize(80000);
	KAllSyms.Seek(0);
	KAllSyms.GetToken(KAllSymsBuffer,"");
	KAllSymsBuffer.Seek(0);
		
	while (KAllSymsBuffer.GetLine(KAllSymsRow)) {
		KAllSymsRow.Seek(0);
		if (strncmp(KAllSymsRow.Buffer()+11,FunctionName.Buffer(),(size_t)FunctionName.Size())==0){
			// Row Tokenizing
			KAllSymsRow.GetToken(Token.Address," \t\n\r");
			KAllSymsRow.GetToken(Token.Type," \t\n\r");
			KAllSymsRow.GetToken(Token.Name," \t\n\r");
			KAllSymsRow.GetToken(Token.Module,"\n\t",NULL," \n\t[]");
			// If Module==NULL then just check Function
			if (ModuleName=="NULL") {
				return strtoll(Token.Address.Buffer(),NULL,16);
			}
			// Check Module/Function
			if (Token.Module==ModuleName) {
				return strtoll(Token.Address.Buffer(),NULL,16);
			}
			Token.Address.SetSize(0);
			Token.Type.SetSize(0);
			Token.Name.SetSize(0);
			Token.Module.SetSize(0);
		}
		KAllSymsRow.SetSize(0);
	}
	return 0;
}


bool Shell::FindAllModulesFunction(FString ModuleName) {
	File KAllSyms;
	
	FString KAllSymsBuffer;
	FString KAllSymsRow;
	RowToken Token;
	int addfnc=0;
	
	FString FList;
	FList.SetSize(0);
	
	const char* c;
	c=ModuleName.Buffer();
	int results=0;
	
	KAllSyms.OpenRead(KALL_SYMS_LOC);
	KAllSymsBuffer.SetSize(80000);
	KAllSyms.Seek(0);
	KAllSyms.GetToken(KAllSymsBuffer,"");
	KAllSymsBuffer.Seek(0);
	
	while (KAllSymsBuffer.GetLine(KAllSymsRow)) {
		const char* p;
		const char* q;
		
		KAllSymsRow.Seek(0);
		KAllSymsRow.GetToken(Token.Address," \t\n\r");
		KAllSymsRow.GetToken(Token.Type," \t\n\r");
		KAllSymsRow.GetToken(Token.Name," \t\n\r");
		KAllSymsRow.GetToken(Token.Module,"\n\t",NULL," \n\t[]");
		
		FString tmp;
		tmp=Token.Module;
		tmp+="::";
		tmp+=Token.Name;
		p=tmp.Buffer();
		q=ModuleName.Buffer();
		addfnc=wildcmp(p,q);
		tmp.SetSize(0);
		
		if(addfnc) {
			SwitchCol(Red,Black);
			con.Printf("%s::",Token.Module.Buffer());
			SwitchCol(Green,Black);
			con.Printf("%s\n",Token.Name.Buffer());
			results++;
		}
		addfnc=0;
		Token.Address.SetSize(0);
		Token.Type.SetSize(0);
		Token.Name.SetSize(0);
		Token.Module.SetSize(0);
		KAllSymsRow.SetSize(0);
	}
	SwitchCol(Grey,Black);
	if (results==0) return false;
	con.Printf("Found %d results.\n",results);
	return true;
}
