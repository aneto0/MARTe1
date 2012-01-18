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
#include "ConfigurationRequest.h"


// ******************************************************************
// Constructors/Destructor
// ******************************************************************
ConfigurationRequest::ConfigurationRequest()
{
	this->validMessage = False;
}


ConfigurationRequest::~ConfigurationRequest()
{
}
// ******************************************************************


// ******************************************************************
// Create requests
// ******************************************************************
bool ConfigurationRequest::CreateFromCDB(ConfigurationDataBase &requestData)
{
	CDBExtended commandCdb(requestData);

	//
	// Initialisations
	//
	this->validMessage = False;

	//
	// Read command
	//
	if(commandCdb.ReadFString(this->command, "Command"))
		this->validMessage = True;
	else
	{
		this->validMessage = False;

		return False;
	}

	//
	// Read command UID
	//
	if(commandCdb.ReadFString(this->commandUID, "CommandUID"))
		this->validMessage = True;
	else
	{
		this->validMessage = False;

		return False;
	}

	//
	// Check if the command is GetPropertyList
	// Nothing more to be read
	//
	if(this->command == "GetPropertyList")
		return True;

	//
	// Read property name
	//
	if(commandCdb.ReadFString(this->propertyName, "PropertyName"))
		this->validMessage = True;
	else
	{
		this->validMessage = False;

		return False;
	}

	//
	// Check if the command is GetPropertyInformation or GetProperty
	// Nothing more to be read
	//
	if((this->command == "GetPropertyInformation") ||
			(this->command == "GetProperty"))
		return True;

	//
	// Leave the CDB in the PropertyConfiguration node
	//
	if(commandCdb->Move("PropertyConfiguration"))
	{
		this->configuration = commandCdb;
		this->validMessage = True;
	}
	else
	{
		this->validMessage = False;

		return False;
	}

	return True;
}


bool ConfigurationRequest::CreateFromString(FString &requestData)
{
	return True;
}
// ******************************************************************


// ******************************************************************
// Get information
// ******************************************************************
FString ConfigurationRequest::GetCommand()
{
	return this->command;
}


FString ConfigurationRequest::GetCommandUID()
{
	return this->commandUID;
}


FString ConfigurationRequest::GetPropertyName()
{
	return this->propertyName;
}


ConfigurationDataBase &ConfigurationRequest::GetConfiguration()
{
	return this->configuration;
}
// ******************************************************************
