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
#include "ConfigurationReply.h"
#include "ConfigurableProperty.h"


// ******************************************************************
// Constructors/Destructor
// ******************************************************************
ConfigurationReply::ConfigurationReply()
{
}


ConfigurationReply::~ConfigurationReply()
{
}
// ******************************************************************


// ******************************************************************
// Get the reply as an FString
// ******************************************************************
bool ConfigurationReply::GetString(FString &str)
{
	replyCdb->WriteToStream(str);

	return True;
}
// ******************************************************************


// ******************************************************************
// Create CDB from FString method
// ******************************************************************
bool ConfigurationReply::CreateCDB(FString &data, ConfigurationDataBase &cdb)
{
	SXMemory config((char *) data.Buffer(), strlen(data.Buffer()));
	if(!cdb->ReadFromStream(config, NULL))
	{
		CStaticAssertErrorCondition(RecoverableError,
							 "ConfigurableObject::CreateCDB: error generating CDB.");

		return False;
	}

	return True;
}
// ******************************************************************


// ******************************************************************
// Reply generation methods
// ******************************************************************
void ConfigurationReply::CreateInvalidConfigurationReply(ConfigurationRequest &command)
{
	FString commandUidString = command.GetCommandUID();

	//
	// Generate the reply
	//
	FString replyString = "";
	replyString += "Result = InvalidConfiguration\n";
	replyString += "CommandUID = ";
	replyString += commandUidString;
	replyString += "\nSuccess = False\n";

	this->CreateCDB(replyString, this->replyCdb);
}


void ConfigurationReply::CreatePropertyDoesNotExistReply(ConfigurationRequest &command)
{
	FString commandUidString = command.GetCommandUID();

	//
	// Generate the reply
	//
	FString replyString = "";
	replyString += "Result = PropertyDoesNotExist\n";
	replyString += "CommandUID = ";
	replyString += commandUidString;
	replyString += "\nSuccess = False\n";

	this->CreateCDB(replyString, this->replyCdb);
}


void ConfigurationReply::CreatePropertyNotReadableReply(ConfigurationRequest &command)
{
	FString commandUidString = command.GetCommandUID();

	//
	// Generate the reply
	//
	FString replyString = "";
	replyString += "Result = PropertyNotReadable\n";
	replyString += "CommandUID = ";
	replyString += commandUidString;
	replyString += "\nSuccess = False\n";

	this->CreateCDB(replyString, this->replyCdb);
}


void ConfigurationReply::CreatePropertyNotWritableReply(ConfigurationRequest &command)
{
	FString commandUidString = command.GetCommandUID();

	//
	// Generate the reply
	//
	FString replyString = "";
	replyString += "Result = PropertyNotWritable\n";
	replyString += "CommandUID = ";
	replyString += commandUidString;
	replyString += "\nSuccess = False\n";

	this->CreateCDB(replyString, this->replyCdb);
}


void ConfigurationReply::CreateCannotModifyPropertyReply(ConfigurationRequest &command)
{
	FString commandUidString = command.GetCommandUID();

	//
	// Generate the reply
	//
	FString replyString = "";
	replyString += "Result = CannotModifyProperty\n";
	replyString += "CommandUID = ";
	replyString += commandUidString;
	replyString += "\nSuccess = False\n";

	this->CreateCDB(replyString, this->replyCdb);
}


void ConfigurationReply::CreatePropertySetReply(ConfigurationRequest &command)
{
	FString commandUidString = command.GetCommandUID();

	//
	// Generate the reply
	//
	FString replyString = "";
	replyString += "Result = PropertySet\n";
	replyString += "CommandUID = ";
	replyString += commandUidString;
	replyString += "\nSuccess = True\n";

	this->CreateCDB(replyString, this->replyCdb);
}


void ConfigurationReply::CreatePropertyConfigurationReply(ConfigurationRequest &command, ConfigurationDataBase &configCdb)
{
	//
	// Get the property configuration
	//
	FString configString;
	if(!configCdb->WriteToStream(configString))
		return;

	//
	// Get the received command UID
	//
	FString commandUidString = command.GetCommandUID();

	//
	// Generate the reply
	//
	FString replyString = "";
	replyString += "Result = PropertyConfiguration\n";
	replyString += "CommandUID = ";
	replyString += commandUidString;
	replyString += "\nSuccess = True\n";
	replyString += "PropertyConfiguration =\n";
	replyString += "{\n";
	replyString += configString;
	replyString += "}\n";

	this->CreateCDB(replyString, this->replyCdb);
}


void ConfigurationReply::CreatePropertyListReply(ConfigurationRequest &command, FString &pList)
{
	//
	// Get the received command UID
	//
	FString commandUidString = command.GetCommandUID();

	//
	// Generate the reply
	//
	FString replyString = "";
	replyString += "Result = PropertyList\n";
	replyString += "CommandUID = ";
	replyString += commandUidString;
	replyString += "\nSuccess = True\n";
	replyString += "PropertyList =\n";
	replyString += "{\n";
	replyString += pList;
	replyString += "}\n";

	this->CreateCDB(replyString, this->replyCdb);
}


void ConfigurationReply::CreatePropertyInformationReply(ConfigurationRequest &command, ConfigurableProperty &property)
{
	//
	// Get the received command UID
	//
	FString commandUidString = command.GetCommandUID();

	//
	// Generate the reply
	//
	FString replyString = "";
	replyString += "Result = PropertyInformation\n";
	replyString += "CommandUID = ";
	replyString += commandUidString;
	replyString += "\nSuccess = True\n";
	replyString += "Information =\n";
	replyString += "{\n";

	FString pName, pType;
	property.GetName(pName);
	property.GetType(pType);

	replyString += "\tName = ";
	replyString += pName;
	replyString += "\n";

	replyString += "\tType = ";
	replyString += pType;
	replyString += "\n";

	replyString += "}\n";

	this->CreateCDB(replyString, this->replyCdb);
}
// ******************************************************************
