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
#ifndef __CONFIGURATIONREQUEST_H__
#define	__CONFIGURATIONREQUEST_H__


#include <ConfigurationDataBase.h>
#include <CDBExtended.h>
#include <FString.h>


class ConfigurationRequest
{
private:
	FString command;
	FString commandUID;
	FString propertyName;
	ConfigurationDataBase configuration;
	bool validMessage;

public:

	// Constructors and destructor
	ConfigurationRequest();
	~ConfigurationRequest();

	// Create requests
	bool CreateFromCDB(ConfigurationDataBase &requestData);
	bool CreateFromString(FString &requestData);

	// Get information
	FString GetCommand();
	FString GetCommandUID();
	FString GetPropertyName();
	ConfigurationDataBase &GetConfiguration();
};


#endif	/* __CONFIGURATIONREQUEST_H__ */

