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
#ifndef __CONFIGURABLEOBJECT_H__
#define	__CONFIGURABLEOBJECT_H__


//#include <stdio.h>
//#include <stdlib.h>

//#include <GCNamedObject.h>
//#include <GarbageCollectable.h>
#include <GlobalObjectDataBase.h>
#include <SXMemory.h>
#include <CDBExtended.h>
#include <LinkedListHolder.h>
#include <MessageHandler.h>

#include "ConfigurableProperty.h"
#include "ConfigurationRequest.h"
#include "ConfigurationReply.h"


//OBJECT_DLL(ConfigurableObject)


class ConfigurableObject : /*public Object,*/ /*public GarbageCollectable,*/ public MessageHandler//, public GCNamedObject
{
private:
	GCNamedObject *messageSender;
	LinkedListHolder listOfProperties;


public:

	// Constructors and destructor
	ConfigurableObject();
	virtual ~ConfigurableObject();

	// Message processing
	virtual bool ProcessMessage(GCRTemplate<MessageEnvelope> envelope);

	// Public methods to use this class
	bool SetMessageSender(GCNamedObject *newSender);
	bool CreateProperty(const char *name, CL_PropertyType type, void *pProperty, CL_PropertyPermissions permissions = CL_PERMISSIONS_BOTH, int dim = 0, int *size = NULL);


private:

	// Configuration handling methods
	bool GetPropertyList(ConfigurationRequest &configReq, ConfigurationReply &reply);
	bool GetPropertyInformation(ConfigurationRequest &configReq, ConfigurationReply &reply);
	bool GetProperty(ConfigurationRequest &configReq, ConfigurationReply &reply);
	bool SetProperty(ConfigurationRequest &configReq, ConfigurationReply &reply);

	// Find a property
	ConfigurableProperty *FindProperty(FString pName);

//	OBJECT_DLL_STUFF(ConfigurableObject)
};


#endif	/* __CONFIGURABLEOBJECT_H__ */
