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
#include "ConfigurableObject.h"

//OBJECTLOADREGISTER(ConfigurableObject,"$Id: $")


// ******************************************************************
// Constructors/Destructor
// ******************************************************************
ConfigurableObject::ConfigurableObject()
{
	this->messageSender = NULL;
}

ConfigurableObject::~ConfigurableObject()
{
}
// ******************************************************************


// ******************************************************************
// Message handler method
// ******************************************************************
bool ConfigurableObject::ProcessMessage(GCRTemplate<MessageEnvelope> envelope)
{
	ConfigurationDataBase receivedCdb;
	ConfigurationReply reply;
	GCRTemplate<MessageEnvelope> replyEnvelope(GCFT_Create);
	GCRTemplate<Message> message = envelope->GetMessage();
	GCRTemplate<Message> replyMessage(GCFT_Create);

	if(message.IsValid())
	{
		//
		// Create CDB from the received message
		//
		SXMemory config((char *) message->Content(), strlen(message->Content()));
		if(!receivedCdb->ReadFromStream(config, NULL))
		{
			CStaticAssertErrorCondition(RecoverableError,
								 "ConfigurableObject::ProcessMessage: Could not create a CDB from the received message.");

			return False;
		}

		//
		// Create the message request object
		//
		ConfigurationRequest command;

		//
		// Parse the command from the CDB
		//
		if(!command.CreateFromCDB(receivedCdb))
		{
			CStaticAssertErrorCondition(RecoverableError,
								 "ConfigurableObject::ProcessMessage: Invalid configuration message.");

			// TODO: Criar uma nova mensagem para este tipo de erro
			//this->CreateInvalidConfigurationReply(commandCdb, replyCdb);
		}
		else
		{
			//
			// SetProperty command
			//
			if(command.GetCommand() == "SetProperty")
				this->SetProperty(command, reply);

			//
			// GetProperty command
			//
			if(command.GetCommand() == "GetProperty")
				this->GetProperty(command, reply);

			//
			// GetPropertyInformation command
			//
			if(command.GetCommand() == "GetPropertyInformation")
				this->GetPropertyInformation(command, reply);

			//
			// GetPropertyList command
			//
			if(command.GetCommand() == "GetPropertyList")
				this->GetPropertyList(command, reply);
		}

		//
		// Reply
		//
		FString replyString;
		if(!reply.GetString(replyString))
		{
			CStaticAssertErrorCondition(InitialisationError,
									 "ConfigurableObject::ProcessMessage: Could not get the reply string.");

			return False;
		}
		replyMessage->Init(0x100000, replyString.Buffer());
		replyEnvelope->PrepareMessageEnvelope(replyMessage, envelope->Sender(), MDRF_Reply, this->messageSender);
		MHSendMessage(replyEnvelope);
	}

	return True;
}
// ******************************************************************


// ******************************************************************
// Message Sender
// ******************************************************************
bool ConfigurableObject::SetMessageSender(GCNamedObject *newSender)
{
	this->messageSender = newSender;

	return True;
}
// ******************************************************************


// ******************************************************************
// Property handling methods
// TODO: Exchange the second argument with the third
// ******************************************************************
bool ConfigurableObject::CreateProperty(const char *name,
										CL_PropertyType type,
										void *pProperty,
										CL_PropertyPermissions permissions,
										int dim,
										int *size)
{
	//
	// Create the property
	//
	GCRTemplate<ConfigurableProperty> property(GCFT_Create);

	//
	// Initialize the property
	//
	property->SetName(name);
	property->SetType(type);
	if(type == CL_Float32Array)
		property->SetPropertyPointer(pProperty, dim, size);
	else
		property->SetPropertyPointer(pProperty);

	//
	// Add the property to the list
	//
	this->listOfProperties.ListAdd(property.operator->());

	//
	// Add the property to the GlobalObjectDataBase
	//
	GCRTemplate<GCReferenceContainer> godb = GetGlobalObjectDataBase();
	godb->Insert(property);

	return True;
}
// ******************************************************************


// ******************************************************************
// Configuration handling methods
// ******************************************************************
bool ConfigurableObject::GetPropertyList(ConfigurationRequest &configReq, ConfigurationReply &reply)
{
	//
	// Generate the reply
	//
	FString commandUidString = configReq.GetCommandUID();
	FString pList = "";

	//
	// Add the property names
	//
	for(int i = 0 ; i < this->listOfProperties.ListSize() ; i++)
	{
		char buffer[10];
		sprintf(buffer, "%d", i);
		ConfigurableProperty *property = (ConfigurableProperty *) this->listOfProperties.ListPeek(i);
		pList += "\tProperty[";
		pList += buffer;
		pList += "] = ";
		pList += property->GetName();
		pList += "\n";
	}

	//
	// Create the reply message
	//
	reply.CreatePropertyListReply(configReq, pList);

	return True;
}


bool ConfigurableObject::GetPropertyInformation(ConfigurationRequest &configReq, ConfigurationReply &reply)
{
	// DEBUG
	CStaticAssertErrorCondition(InitialisationError, "ConfigurableObject::SetProperty");

	//
	// Find property
	//
	ConfigurableProperty *prop = this->FindProperty(configReq.GetPropertyName());

	//
	// Property not found
	//
	if(prop != NULL)
		reply.CreatePropertyInformationReply(configReq, *prop);
	else
		reply.CreatePropertyDoesNotExistReply(configReq);

	return True;
}


bool ConfigurableObject::SetProperty(ConfigurationRequest &configReq, ConfigurationReply &reply)
{
	CDBExtended cdb(configReq.GetConfiguration());

	// DEBUG
	CStaticAssertErrorCondition(InitialisationError, "ConfigurableObject::SetProperty");

	//
	// Check if property exists
	//
	ConfigurableProperty *prop = this->FindProperty(configReq.GetPropertyName());

	//
	// Set property
	//
	if(prop != NULL)
	{
		CL_PropertyErrors err;
		if(prop->Set(cdb, err))
			reply.CreatePropertySetReply(configReq);
		else
			reply.CreateInvalidConfigurationReply(configReq);
	}
	else
		reply.CreatePropertyDoesNotExistReply(configReq);

	return True;
}


bool ConfigurableObject::GetProperty(ConfigurationRequest &configReq, ConfigurationReply &reply)
{
	ConfigurationDataBase cdb;

	//
	// Check if property exists
	//
	ConfigurableProperty *prop = this->FindProperty(configReq.GetPropertyName());

	//
	// Property not found
	//
	if(prop != NULL)
	{
		CL_PropertyErrors err;
		if(!prop->Get(cdb, err))
			reply.CreateInvalidConfigurationReply(configReq);
		else
			reply.CreatePropertyConfigurationReply(configReq, cdb);
	}
	else
		reply.CreatePropertyDoesNotExistReply(configReq);

	return True;
}
// ******************************************************************


// ******************************************************************
// Get a property from the list
// ******************************************************************
ConfigurableProperty *ConfigurableObject::FindProperty(FString pName)
{
	ConfigurableProperty *prop = NULL;
	for(int i = 0 ; i < this->listOfProperties.ListSize() ; i++)
	{
		prop = (ConfigurableProperty *) this->listOfProperties.ListPeek(i);
		if(pName == prop->GetName())
			return prop;
	}

	return NULL;
}
// ******************************************************************
