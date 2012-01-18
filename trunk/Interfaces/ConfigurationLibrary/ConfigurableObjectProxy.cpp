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
#include "ConfigurableObjectProxy.h"

OBJECTLOADREGISTER(ConfigurableObjectProxy,"$Id: $")


// ******************************************************************
// Constructors/Destructor
// ******************************************************************
ConfigurableObjectProxy::ConfigurableObjectProxy()
{
	this->messageSender = this;
	this->destinationObject = "";
	this->messageServerIP = "localhost";
	this->messageServerPort = 0;

	this->myCdb = NULL;

	this->communicationSem.Create();
}


ConfigurableObjectProxy::~ConfigurableObjectProxy()
{
}
// ******************************************************************


// ******************************************************************
// Create CDB from FString method
// ******************************************************************
bool ConfigurableObjectProxy::CreateCDB(FString &data, ConfigurationDataBase &cdb)
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
// Remote message sending
// ******************************************************************
void ConfigurableObjectProxy::SetDestinationObject(const char *objectName)
{
	this->destinationObject = objectName;
}

void ConfigurableObjectProxy::SetServer(const char *ip, uint32 port)
{
	this->messageServerPort = port;
	this->messageServerIP = ip;
}

void ConfigurableObjectProxy::SendMessage(const char *messageString)
{
	this->communicationSem.Reset();

	GCRTemplate<MessageEnvelope> envelope(GCFT_Create);
	GCRTemplate<Message>         message(GCFT_Create);
	message->Init(0x100000, messageString);

	envelope->PrepareMessageEnvelope(message, this->destinationObject.Buffer(), MDRF_ManualReply, this->messageSender);
	MHSendMessageRemotely(envelope, this->messageServerIP.Buffer(), this->messageServerPort);

	cout << "Waiting for an event..." << endl;
	this->communicationSem.Wait();
	cout << "Event posted..." << endl;
}
// ******************************************************************


// ******************************************************************
// Message handler method
// ******************************************************************
bool ConfigurableObjectProxy::ProcessMessage(GCRTemplate<MessageEnvelope> envelope)
{
	if(!envelope.IsValid())
	{
		cout << "Received an invalid envelope!" << endl;
		return False;
	}

	cout << "Received message from " << envelope->Sender() << endl;
	GCRTemplate<Message>  message = envelope->GetMessage();

	// We can do something much smarter here....
	//cout << "Content is: " << message->Content() << endl;

	//
	// TODO: Parse the reply
	// TODO: Do it properly
	//
	FString messageContent = message->Content();
	ConfigurationDataBase cdb;
	this->CreateCDB(messageContent, cdb);
	CDBExtended ecdb(cdb);
	FString responseType;
	ecdb.ReadFString(responseType, "Result");

	//
	// TODO: Do it the proper way
	// CAVADOR: Create the CDB with the reply
	//
	if(this->myCdb != NULL)
	{
		delete this->myCdb;
		this->myCdb = NULL;
	}
	if(responseType == "PropertyConfiguration")
	{
		this->myCdb = new ConfigurationDataBase();
		ecdb->Move("PropertyConfiguration");
		(*this->myCdb)->CopyFrom(ecdb.operator ->());
		//this->CreateCDB(messageContent, *this->myCdb);
		//FString tmp;
		//(*this->myCdb)->WriteToStream(tmp);
		//cout << "myCdb is: " << tmp.Buffer() << endl;
	}

	//
	// Unlock the wait for the answer semaphore
	//
	this->communicationSem.Post();

	return True;
}
// ******************************************************************


// ******************************************************************
// Configuration handling methods
// ******************************************************************
bool ConfigurableObjectProxy::GetPropertyList(ConfigurationDataBase &config)
{
	FString message = "";
	message += "Command = GetPropertyList\n";
	message += "CommandUID = XYZ\n";
	//message += "CommandUID = GetPropertyList\n";

	this->SendMessage(message.Buffer());

	return True;
}


bool ConfigurableObjectProxy::GetPropertyInformation(const char *pName, ConfigurationDataBase &config)
{
	FString message = "";
	message += "Command = GetPropertyInformation\n";
	//message += "CommandUID = XYZ\n";
	message += "CommandUID = GetPropertyInformation_";
	message += pName;
	message += "\n";
	message += "PropertyName = ";
	message += pName;
	message += "\n";

	this->SendMessage(message.Buffer());

	return True;
}


bool ConfigurableObjectProxy::GetProperty(const char *pName, ConfigurationDataBase &config)
{
	//
	// Generate the message string
	//
	FString message = "";
	message += "Command = GetProperty\n";
	//message += "CommandUID = XYZ\n";
	message += "CommandUID = GetProperty_";
	message += pName;
	message += "\n";
	message += "PropertyName = ";
	message += pName;
	message += "\n";

	//
	// Send the message
	//
	this->SendMessage(message.Buffer());

	//
	// Get the reply CDB with the configuration
	//
	config->CopyFrom((*this->myCdb).operator ->());

	//
	//DEBUG
	//
	//FString tmp;
	//config->WriteToStream(tmp);
	//cout << "config is: " << tmp.Buffer() << endl;

	return True;
}


bool ConfigurableObjectProxy::SetProperty(const char *pName, const char *config)
{
	FString configStr = config;

	return this->SetProperty(pName, configStr);
}


bool ConfigurableObjectProxy::SetProperty(const char *pName, FString &config)
{
	ConfigurationDataBase cdb;

	if(!this->CreateCDB(config, cdb))
		return False;

	return this->SetProperty(pName, cdb);
}


bool ConfigurableObjectProxy::SetProperty(const char *pName, ConfigurationDataBase &config)
{
	FString configurationString;
	if(!config->WriteToStream(configurationString))
		return False;

	FString message = "";
	message += "Command = SetProperty\n";
	//message += "CommandUID = XYZ\n";
	message += "CommandUID = SetProperty_";
	message += pName;
	message += "\n";
	message += "PropertyName = ";
	message += pName;
	message += "\n";
	message += "PropertyConfiguration = {\n";
	message += configurationString;
	message += "\n}\n";

	this->SendMessage(message.Buffer());

	return True;
}
// ******************************************************************


// ******************************************************************
// Message Sender
// ******************************************************************
bool ConfigurableObjectProxy::SetMessageSender(GCNamedObject *newSender)
{
	this->messageSender = newSender;

	return True;
}
// ******************************************************************
