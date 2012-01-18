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
#ifndef __GENERICTESTGAM_H__
#define	__GENERICTESTGAM_H__


#include <stdio.h>
#include <string.h>

//#include <SXMemory.h>
#include <ConfigurationDataBase.h>
#include <GAM.h>
#include <DDBOutputInterface.h>
#include <CDBExtended.h>
#include <MessageHandler.h>

#include <ConfigurableObject.h>
#include <ConfigurableProperty.h>


#include "GenericTestModule.h"


#define FLOAT_ARRAY_DIM 5


OBJECT_DLL(GenericTestGAM)


class GenericTestGAM : public GAM, public ConfigurableObject
{
private:

	// Interfaces (for signals)
	DDBOutputInterface *signalsOutputInterface;


	// Values (for parameters)
	int8   value8;
	uint8  value8u;
	int16  value16;
	uint16 value16u;
	int32  value32;
	uint32 value32u;
	int64  value64;
	uint64 value64u;
	float  value32_f;
	double value64_f;
	int32  value32r;
	int32  value32w;
	float  array32_f[FLOAT_ARRAY_DIM];

	GenericTestModule module;

	// Values (previous values)
	int8   previous_value8;
	uint8  previous_value8u;
	int16  previous_value16;
	uint16 previous_value16u;
	int32  previous_value32;
	uint32 previous_value32u;
	int64  previous_value64;
	uint64 previous_value64u;
	float  previous_value32_f;
	double previous_value64_f;
	int32  previous_value32r;
	int32  previous_value32w;
	float  previous_array32_f[FLOAT_ARRAY_DIM];

	GAM_FunctionNumbers state;


public:

	// Constructors and destructor
	GenericTestGAM();
	virtual ~GenericTestGAM();

	// Virtual methods
	virtual bool Initialise(ConfigurationDataBase& cdbData);
	virtual bool Execute(GAM_FunctionNumbers functionNumber);
	virtual bool ProcessMessage(GCRTemplate<MessageEnvelope> envelope);
/*
	// Configuration handling methods
	bool GetPropertyList(ConfigurationDataBase &commandCdb, ConfigurationDataBase &replyCdb);
	bool GetPropertyInformation(ConfigurationDataBase &commandCdb, ConfigurationDataBase &replyCdb);
	bool GetProperty(ConfigurationDataBase &commandCdb, ConfigurationDataBase &replyCdb);
	bool SetProperty(ConfigurationDataBase &commandCdb, ConfigurationDataBase &replyCdb);

	// Response generator methods
	bool PropertyExists(FString &pName);
	bool CreateCDB(FString &data, ConfigurationDataBase &config);
	void CreateInvalidConfigurationReply(CDBExtended &commandCdb, ConfigurationDataBase &replyCdb);
	void CreatePropertyDoesNotExistReply(CDBExtended &commandCdb, ConfigurationDataBase &replyCdb);
	void CreatePropertyNotReadableReply(CDBExtended &commandCdb, ConfigurationDataBase &replyCdb);
	void CreatePropertyNotWritableReply(CDBExtended &commandCdb, ConfigurationDataBase &replyCdb);
	void CreateCannotModifyPropertyReply(CDBExtended &commandCdb, ConfigurationDataBase &replyCdb);
	void CreatePropertySetReply(CDBExtended &commandCdb, ConfigurationDataBase &replyCdb);
	void CreatePropertyConfigurationReply(CDBExtended &commandCdb, ConfigurationDataBase &configCdb, ConfigurationDataBase &replyCdb);
	void CreatePropertyInformationReply(CDBExtended &commandCdb, ConfigurationDataBase &informationCdb, ConfigurationDataBase &replyCdb);
*/
	OBJECT_DLL_STUFF(GenericTestGAM)
};


#endif	/* __GenericTestGAM_H__ */
