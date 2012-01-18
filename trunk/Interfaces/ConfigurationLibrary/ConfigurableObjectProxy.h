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
#ifndef __CONFIGURABLEOBJECTPROXY_H__
#define	__CONFIGURABLEOBJECTPROXY_H__


// TODO: Apagar tudo o que tem a ver com isto
#include <iostream>
using namespace std;

#include <SXMemory.h>
#include <GCNamedObject.h>
#include <FString.h>
#include <MessageHandler.h>
#include <Message.h>
#include <MessageEnvelope.h>
#include <ConfigurationDataBase.h>

#include "ConfigurationRequest.h"
#include "ConfigurationReply.h"


OBJECT_DLL(ConfigurableObjectProxy)


class ConfigurableObjectProxy : public GCNamedObject, public MessageHandler
{
private:
	GCNamedObject *messageSender;
	FString destinationObject;
	//FString propertyName;
	FString messageServerIP;
	uint32 messageServerPort;

	// TODO: Variables to improve
	EventSem communicationSem;
	ConfigurationDataBase *myCdb;

	ConfigurationRequest request;
	ConfigurationReply reply;

	// Send message remotely method
	void SendMessage(const char *messageString);

	// Create CDB from FString method
	bool CreateCDB(FString &data, ConfigurationDataBase &cdb);

public:

	// Constructors and destructor
	ConfigurableObjectProxy();
	virtual ~ConfigurableObjectProxy();

	// Message processing
	virtual bool ProcessMessage(GCRTemplate<MessageEnvelope> envelope);

	// Remote object setting
	bool SetMessageSender(GCNamedObject *newSender);
	void SetDestinationObject(const char *objectName);
	void SetServer(const char *ip, uint32 port);

	// Configuration handling methods
	bool GetPropertyList(ConfigurationDataBase &config);
	bool GetPropertyInformation(const char *pName, ConfigurationDataBase &config);
	bool GetProperty(const char *pName, ConfigurationDataBase &config);
	bool SetProperty(const char *pName, ConfigurationDataBase &config);
	bool SetProperty(const char *pName, FString &config);
	bool SetProperty(const char *pName, const char *config);

	OBJECT_DLL_STUFF(ConfigurableObjectProxy)
};


#endif	/* __CONFIGURABLEOBJECTPROXY_H__ */
