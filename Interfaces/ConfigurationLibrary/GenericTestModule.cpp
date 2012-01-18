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
#include <stdio.h>

#include "GenericTestModule.h"


// ******************************************************************
// Constructors/Destructor
// ******************************************************************
GenericTestModule::GenericTestModule()
{
	this->nelements = 0;
	this->vector = NULL;
}


GenericTestModule::~GenericTestModule()
{
}
// ******************************************************************


// ******************************************************************
// Object load setup
// ******************************************************************
bool GenericTestModule::ObjectLoadSetup(ConfigurationDataBase &cdb, StreamInterface *err)
{
	CDBExtended cdbData(cdb);

	// DEBUG
	this->AssertErrorCondition(InitialisationError,
						 "GenericTestModule::ObjectLoadSetup: I am being configured!");

	//
	// Example: Read a vector
	//
	if(cdbData->Exists("NumberOfElements"))
	{
		cdbData.ReadInt32(this->nelements, "NumberOfElements", (int32) 0);
		this->AssertErrorCondition(InitialisationError,
								"GenericTestModule::ObjectLoadSetup: NumberOfElements = %d",
								(long int) this->nelements);
	}
	else
		return False;

	if(cdbData->Exists("Vector"))
	{
		if(this->vector != NULL)
			delete[] this->vector;

		this->vector = new double[this->nelements];
		if(this->vector == NULL)
		{
			this->nelements = 0;

			return False;
		}
		if(!cdbData.ReadDoubleArray(this->vector, (int32 *) &this->nelements, 1, "Vector"))
		{
			delete[] this->vector;
			this->vector = NULL;
			this->nelements = 0;

			return False;
		}
	}
	else
	{
		this->vector = NULL;
		this->nelements = 0;

		return False;
	}

	//
	// DEBUG: Print the vector
	//
	FString vecStr = "";
	for(int i = 0 ; i < this->nelements ; i++)
	{
		char vecElemStr[20];
		sprintf(vecElemStr, "%f", this->vector[i]);
		vecStr += vecElemStr;
		vecStr += " ";
	}
	this->AssertErrorCondition(InitialisationError,
							"GenericTestModule::ObjectLoadSetup: Vector = { %s}",
							vecStr.Buffer());

	return True;
}


bool GenericTestModule::ObjectSaveSetup(ConfigurationDataBase &cdb, StreamInterface *err)
{
	// DEBUG
	this->AssertErrorCondition(InitialisationError,
						 "GenericTestModule::ObjectSaveSetup: I am being introspected!");

	//
	// Create the CDB string
	//
	FString vecCdb = "";
	char buffer[20];
	vecCdb += "NumberOfElements = ";
	sprintf(buffer, "%d", this->nelements);
	vecCdb += buffer;
	vecCdb += "\n";
	if(this->vector != NULL)
	{
		vecCdb += "Vector = { ";

		FString vecStr = "";
		for(int i = 0 ; i < this->nelements ; i++)
		{
			char vecElemStr[20];
			sprintf(vecElemStr, "%f", this->vector[i]);
			vecStr += vecElemStr;
			vecStr += " ";
		}
		vecCdb += vecStr;
	}
	vecCdb += "}\n";

	//
	// Create the return CDB
	//
	SXMemory vecConfig((char *) vecCdb.Buffer(), strlen(vecCdb.Buffer()));
	if(!cdb->ReadFromStream(vecConfig, NULL))
		this->AssertErrorCondition(InitialisationError,
						 "GenericTestModule::ObjectSaveSetup: ReadFromStream False");

	return True;
}
// ******************************************************************
