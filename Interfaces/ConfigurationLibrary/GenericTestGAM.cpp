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
#include "GenericTestGAM.h"


OBJECTLOADREGISTER(GenericTestGAM, "$Id: $")


// ******************************************************************
// Constructors/Destructor
// ******************************************************************
GenericTestGAM::GenericTestGAM()
{
	this->signalsOutputInterface = NULL;
}


GenericTestGAM::~GenericTestGAM()
{
}
// ******************************************************************


// ******************************************************************
// Initialise the module
// ******************************************************************
bool GenericTestGAM::Initialise(ConfigurationDataBase& cdbData)
{
	CDBExtended cdb(cdbData);

	//
	// Call the parent Initialise method
	//
	//GAM::Initialise(cdb);

	//
	// Create the output signals interface
	//
	if(!AddOutputInterface(this->signalsOutputInterface, "SignalsOutputInterface"))
	{
		AssertErrorCondition(InitialisationError,
							 "GenericTestGAM::Initialise: %s failed to add the output interface SignalsOutputInterface",
							 this->Name());

		return False;
	}

	//
	// Add the int32 signal
	//
	FString signalName;
	if(!cdb.ReadFString(signalName, "SignalInt32"))
	{
		AssertErrorCondition(InitialisationError,
							 "GenericTestGAM::Initialise: %s failed get the SignalInt32 property for signal SignalInt32",
							 this->Name());

		return False;
	}
	if(!this->signalsOutputInterface->AddSignal(signalName.Buffer(), "int32"))
	{
		AssertErrorCondition(InitialisationError,
							 "GenericTestGAM::Initialise: %s failed add the SignalInt32 signal to the interface",
							 this->Name());

		return False;
	}

	//
	// Initialize the module
	//
	//this->module.ObjectLoadSetup(cdb, NULL);

	//
	// Setup the property related stuff
	//
	this->SetMessageSender(this);
	this->CreateProperty("value8",    CL_Int8,         &this->value8);
	this->CreateProperty("value8u",   CL_UInt8,        &this->value8u);
	this->CreateProperty("value16",   CL_Int16,        &this->value16);
	this->CreateProperty("value16u",  CL_UInt16,       &this->value16u);
	this->CreateProperty("value32",   CL_Int32,        &this->value32);
	this->CreateProperty("value32u",  CL_UInt32,       &this->value32u);
	this->CreateProperty("value32_f", CL_Float32,      &this->value32_f);
	this->CreateProperty("value64_f", CL_Float64,      &this->value64_f);
	this->CreateProperty("module",    CL_Object,       &this->module);
	int size[1] = { FLOAT_ARRAY_DIM };
	this->CreateProperty("array32_f", CL_Float32Array,  this->array32_f, CL_PERMISSIONS_BOTH, 1, size);

	return True;
}
// ******************************************************************


// ******************************************************************
// Execute the module functionalities
// ******************************************************************
bool GenericTestGAM::Execute(GAM_FunctionNumbers functionNumber)
{
	//
	// Call the parent Execute method
	//
	//GAM::Execute(functionNumber);

	//
	// Update the global GAM state
	//
	this->state = functionNumber;

	//
	// Check if values changed
	//
	if(this->value8 != this->previous_value8)
	{
		AssertErrorCondition(Information,
							 "GenericTestGAM::Execute: value8 changed on %s. %d --> %d",
							 this->Name(),
							 this->previous_value8,
							 this->value8);

		this->previous_value8 = this->value8;
	}
	if(this->value8u != this->previous_value8u)
	{
		AssertErrorCondition(Information,
							 "GenericTestGAM::Execute: value8u changed on %s. %u --> %u",
							 this->Name(),
							 this->previous_value8u,
							 this->value8u);

		this->previous_value8u = this->value8u;
	}
	if(this->value16 != this->previous_value16)
	{
		AssertErrorCondition(Information,
							 "GenericTestGAM::Execute: value16 changed on %s. %d --> %d",
							 this->Name(),
							 this->previous_value16,
							 this->value16);

		this->previous_value16 = this->value16;
	}
	if(this->value16u != this->previous_value16u)
	{
		AssertErrorCondition(Information,
							 "GenericTestGAM::Execute: value16u changed on %s. %u --> %u",
							 this->Name(),
							 this->previous_value16u,
							 this->value16u);

		this->previous_value16u = this->value16u;
	}
	if(this->value32 != this->previous_value32)
	{
		AssertErrorCondition(Information,
							 "GenericTestGAM::Execute: value32 changed on %s. %d --> %d",
							 this->Name(),
							 this->previous_value32,
							 this->value32);

		this->previous_value32 = this->value32;
	}
	if(this->value32u != this->previous_value32u)
	{
		AssertErrorCondition(Information,
							 "GenericTestGAM::Execute: value32u changed on %s. %u --> %u",
							 this->Name(),
							 this->previous_value32u,
							 this->value32u);

		this->previous_value32u = this->value32u;
	}
	if(this->value64 != this->previous_value64)
	{
		AssertErrorCondition(Information,
							 "GenericTestGAM::Execute: value64 changed on %s. %d --> %d",
							 this->Name(),
							 this->previous_value64,
							 this->value64);

		this->previous_value64 = this->value64;
	}
	if(this->value64u != this->previous_value64u)
	{
		AssertErrorCondition(Information,
							 "GenericTestGAM::Execute: value64u changed on %s. %u --> %u",
							 this->Name(),
							 this->previous_value64u,
							 this->value64u);

		this->previous_value64u = this->value64u;
	}
	if(this->value32_f != this->previous_value32_f)
	{
		AssertErrorCondition(Information,
							 "GenericTestGAM::Execute: value32_f changed on %s. %f --> %f",
							 this->Name(),
							 this->previous_value32_f,
							 this->value32_f);

		this->previous_value32_f = this->value32_f;
	}
	if(this->value64_f != this->previous_value64_f)
	{
		AssertErrorCondition(Information,
							 "GenericTestGAM::Execute: value64_f changed on %s. %f --> %f",
							 this->Name(),
							 this->previous_value64_f,
							 this->value64_f);

		this->previous_value64_f = this->value64_f;
	}
	if(this->value32r != this->previous_value32r)
	{
		AssertErrorCondition(Information,
							 "GenericTestGAM::Execute: value32r changed on %s. %d --> %d",
							 this->Name(),
							 this->previous_value32r,
							 this->value32r);

		this->previous_value32r = this->value32r;
	}
	if(this->value32w != this->previous_value32w)
	{
		AssertErrorCondition(Information,
							 "GenericTestGAM::Execute: value32w changed on %s. %d --> %d",
							 this->Name(),
							 this->previous_value32w,
							 this->value32w);

		this->previous_value32w = this->value32w;
	}

	for(int i = 0 ; i < FLOAT_ARRAY_DIM ; i++)
	{
		if(this->array32_f[i] != this->previous_array32_f[i])
		{
			FString oldStr = "";
			FString newStr = "";

			for(int j = 0 ; j < FLOAT_ARRAY_DIM ; j++)
			{
				FString valString;
				valString.SSPrintf((uint32) 0, "%f", this->previous_array32_f[j]);
				oldStr += valString;
				if(j != FLOAT_ARRAY_DIM - 1)
					oldStr += " ; ";
			}
			for(int j = 0 ; j < FLOAT_ARRAY_DIM ; j++)
			{
				FString valString;
				valString.SSPrintf((uint32) 0, "%f", this->array32_f[j]);
				newStr += valString;
				if(j != FLOAT_ARRAY_DIM - 1)
					newStr += " ; ";
			}
			for(int j = 0 ; j < FLOAT_ARRAY_DIM ; j++)
				this->previous_array32_f[j] = this->array32_f[j];

			AssertErrorCondition(Information,
								 "GenericTestGAM::Execute: array32_f changed on %s. %s --> %s",
								 this->Name(),
								 oldStr.Buffer(),
								 newStr.Buffer());
			break;
		}
	}

	// TODO: Implement the output signals

	return True;
}
// ******************************************************************


// ******************************************************************
// Message handler method
// ******************************************************************
bool GenericTestGAM::ProcessMessage(GCRTemplate<MessageEnvelope> envelope)
{
	//
	// Give the oportunity to the ConfigurableObject to respond to the message
	//
	if(!ConfigurableObject::ProcessMessage(envelope))
	{
		AssertErrorCondition(RecoverableError,
							 "ConfigurableObject::ProcessMessage: Failed.");

		return False;
	}

	return True;
}
// ******************************************************************

