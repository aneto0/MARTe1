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

#ifndef FILELOGGER_H_
#define FILELOGGER_H_

#include "File.h"
#include "FString.h"


/*! @class FileLogger
 *  A basic filelogger.
 */
class FileLogger {
	private:
		/**
 		* The log file file descriptor.
 		*/
		File LogFile;
		/**
 		* The FString containing the name of the log file.
 		*/
		FString Filename;
		
		/**
 		* Writes on the log file the current date.
 		* @return nothing.
 		*/
		void WriteDate();
		
		/**
 		* Writes on the log file the current time.
 		* @return nothing.
 		*/
		void WriteTime();
		
	public:
		/**
 		* Object constructor. Initializes the logger to log on file Name.
 		* @param Name the name of the file to log data to.
 		* @return nothing.
 		*/
		FileLogger(char* Name);
		
		/**
 		* Basic deconstructor. Just closes the file descriptor.
 		* @return nothing.
 		*/
		~FileLogger();
		
		/**
 		* Writes a line in the logfile, adding the time at which it happened.
 		* @param row the FString containing the data to log. Note that it must not end
 		*            with a new line.
 		* @return true.
 		*/
		bool  WriteLog(FString* row);
};

#endif /*FILELOGGER_H_*/
