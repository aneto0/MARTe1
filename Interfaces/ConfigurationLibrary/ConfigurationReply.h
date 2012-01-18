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
#ifndef __CONFIGURATIONREPLY_H__
#define	__CONFIGURATIONREPLY_H__


#include <FString.h>
#include <SXMemory.h>
#include <ConfigurationDataBase.h>
#include <CDBExtended.h>


#include "ConfigurationRequest.h"
#include "ConfigurableProperty.h"


class ConfigurationReply
{
private:

	ConfigurationDataBase replyCdb;


public:

	// Constructors and destructor
	ConfigurationReply();
	~ConfigurationReply();

	bool GetString(FString &str);

	bool CreateCDB(FString &data, ConfigurationDataBase &config);

	void CreateInvalidConfigurationReply(ConfigurationRequest &configReq);
	void CreatePropertyDoesNotExistReply(ConfigurationRequest &configReq);
	void CreatePropertyNotReadableReply(ConfigurationRequest &configReq);
	void CreatePropertyNotWritableReply(ConfigurationRequest &configReq);
	void CreateCannotModifyPropertyReply(ConfigurationRequest &configReq);
	void CreatePropertySetReply(ConfigurationRequest &configReq);
	void CreatePropertyConfigurationReply(ConfigurationRequest &configReq, ConfigurationDataBase &configCdb);
	void CreatePropertyListReply(ConfigurationRequest &command, FString &pList);
	void CreatePropertyInformationReply(ConfigurationRequest &command, ConfigurableProperty &property);
};


#endif	/* __CONFIGURATIONREPLY_H__ */

