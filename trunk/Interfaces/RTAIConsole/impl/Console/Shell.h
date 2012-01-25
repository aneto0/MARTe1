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
/** @file */
#include "File.h"
#include "FString.h"
#include "BasicConsole.h"
#include "Console.h"
#include "LexicalAnalyzer.h"
#include "CommandHistory.h"
#include "VarManager.h"
#include "AliasManager.h"

#ifndef SHELL_H_
#define SHELL_H_

#define BSPACE 127
#define SPACE 32
#define CR 13
#define LF 10
#define COLON 58
#define QUOTES 34
#define OPENPAR 40
#define CLOSEPAR 41
#define ESC 27


/*! @struct RowToken
 *  The structure of a line of the kallsyms file.
 */
struct RowToken{
	/**
 	* Address of the function.
 	*/
	FString Address;
	/**
 	* Type of function.
 	*/
	FString	Type;
	/**
 	* Function name.
 	*/
	FString Name;
	/**
 	* Kernel module name.
 	*/
	FString Module;
};

/*! @class Shell
 *  Provides the shell environment: understands commands and performs I/O operations.
 */
class Shell {
	private:
		struct termio shellOriginalSettings;
		/**
 		* The console object to write to.
 		*/
		Console con;
		/**
 		* The lexical analyzer object to tokenize the command line.
 		*/
		LexicalAnalyzer la;
		
		AliasManager AM;
		
		VarManager VM;
		/**
 		* The FString containing the command line.
 		*/
		FString CmdLine;
		/**
 		* Set to true to end the main shell cycle.
 		*/
		bool killConsole;
		/**
 		* Path to the MotD file.
 		*/
		FString MotdPath;
		/**
 		* The command history manager
 		*/
		CommandHistory CmdHistory;
		
		
		/**
 		* Writes RTAIConsole standard command prompt.
 		* @return nothing.
 		*/
		void WritePrompt();
		
		/**
 		* Performs command input using BaseLib2's Console.Read(), and puts it in CmdLine variable.
 		* @return false if the command line is empty, true otherwise.
 		*/
		bool PerformInput();
		
		/**
 		* Redirects command input to the kernel communication file.
 		*/
		void KernelInput();
		
		/**
 		* Prints on screen the MotD (Message of the Day) file from RTAIConsole starting directory, if present.
 		* @return nothing.
 		*/
		void PrintMOTD();
		
		void DeleteChars(int NChars);
		
		/**
 		* Prints on screen the MotD (Message of the Day) file from RTAIConsole starting directory, if present.
 		* @return nothing.
 		*/
		bool LogHistory();
		
		/**
 		* Checks if CmdLine contains an internal command, and performs it.
 		* @return true if CmdLine contains an internal command (even if
 		*           the command itself failed), false otherwise.
 		*/
		bool CheckInternalCommand();
		
		/**
 		* Populates Module and Function with what is present in CmdLine.
 		* The syntax is modulename::functionname.
 		* @param Module a pointer to the FString in which the module name should be written.
 		* @param Function a pointer to the FString in which the function name should be written.
 		* @return false if there is a syntax error, true otherwise.
 		*/
		bool GetModuleFunc(FString* Module, FString* Function);
		
		/**
 		* Populates KernelCallArray with data from CmdLine in order to perform
 		* the kernel call.
 		* @param KernelCallArray a pointer to the int array.
 		* @return false if there is a syntax error, true otherwise.
 		*/
		bool BuildCommandArray (int *KernelCallArray);
		
		/**
 		* Interprets the command line, distinguishing between internal command and
 		* kernel calls. 
 		* @return false if there is a syntax error, true otherwise.
 		*/
		bool InterpretCommandLine();
		
		/**
 		* Looks at the kallsyms file to find the given function's address
 		* @param FunctionName name of the function to look for.
 		* @param ModuleName name of the kernel module to which FunctionName belongs.
 		*        If ModuleName=="NULL" it just looks for FunctionName exported directly
 		*        by the kernel.
 		* @return the function address.
 		*/
		long long int FindFunctionAddress(FString& FunctionName, FString& ModuleName);
		
		/**
 		* Lists all functions belonging to the supplied ModuleName. It has a basic support
 		* for the "*" wildchar.
 		* @param ModuleName name of the module (can begin or end with * wildchar).
 		* @return false if ModuleName doesn't export any function (or doesn't exist). True otherwise.
 		*/
		bool FindAllModulesFunction(FString ModuleName);
	
	
		/**
 		* Changes the color of the console.
 		* @param Fore foreground color.
 		* @param Bg background color.
 		* @return nothing.
 		*/
		inline void SwitchCol(int Fore,int Bg){
			if (con.Switch("colour")){
        		con.Printf("%i %i",Fore,Bg);
        		con.Switch((uint32)0);
    		}
		}
    		
	public:
                /**
                * Stay in kernel mode?
                */
                static bool inKernelMode;
		/**
 		* Class constructor, initializes the shell.
 		* @return nothing.
 		*/
		Shell(const char *startCommand=NULL);
                                
		/**
 		* Class deconstructor. Does nothing.
 		* @return nothing.
 		*/
		~Shell();
		
		/**
 		* Starts the main shell cycle. The only method to be invoked after the creation
 		* of the class.
 		* @return nothing.
 		*/
		void Start();
};

#endif /*SHELL_H_*/
