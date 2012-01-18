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
#include "ConfigurableProperty.h"


OBJECTLOADREGISTER(ConfigurableProperty, "$Id: $")


// ******************************************************************
// Constructors/Destructor
// ******************************************************************
ConfigurableProperty::ConfigurableProperty()
{
	this->name = "";
	this->property = NULL;
	this->type = CL_None;
	this->permissions = CL_PERMISSIONS_NONE;
	this->dimensions = 0;
	this->size = NULL;
}

ConfigurableProperty::~ConfigurableProperty()
{
	if(this->size != NULL)
		delete[] size;
}
// ******************************************************************


// ******************************************************************
// Object load setup
// ******************************************************************
bool ConfigurableProperty::ObjectLoadSetup(ConfigurationDataBase &info, StreamInterface *err)
{
	CDBExtended cdbx(info);

	return True;
}
// ******************************************************************


// ******************************************************************
// Name handling methods
// ******************************************************************
bool ConfigurableProperty::SetName(const char *newName)
{
	if(newName != NULL)
		this->name = newName;
	else
		return False;

	return True;
}


const char *ConfigurableProperty::GetName()
{
	return this->name.Buffer();
}


bool ConfigurableProperty::GetName(FString &name)
{
	name = this->name;

	return True;
}
// ******************************************************************


// ******************************************************************
// Pointer handling methods
// ******************************************************************
bool ConfigurableProperty::SetPropertyPointer(void *newPointer, int dims, int *size)
{
	if(this->size != NULL)
	{
		delete[] size;
		size = NULL;
	}

	this->dimensions = dims;
	if(dims != 0)
	{
		if(size == NULL)
			return False;

		this->size = new int[dims];
		if(this->size != NULL)
		{
			for(int i = 0 ; i < dims ; i++)
				this->size[i] = size[i];
		}
		else
			return False;
	}

	this->property = newPointer;

	return True;
}
// ******************************************************************


// ******************************************************************
// Type handling methods
// ******************************************************************
bool ConfigurableProperty::SetType(CL_PropertyType newType)
{
	this->type = newType;

	return True;
}


CL_PropertyType ConfigurableProperty::GetType()
{
	return this->type;
}


bool ConfigurableProperty::GetType(FString &type)
{
	switch(this->type)
	{
		case CL_Int8:
			type = "Int8";
			break;

		case CL_UInt8:
			type = "UInt8";
			break;

		case CL_Int16:
			type = "Int16";
			break;

		case CL_UInt16:
			type = "UInt16";
			break;

		case CL_Int32:
			type = "Int32";
			break;

		case CL_UInt32:
			type = "UInt32";
			break;

		case CL_Int64:
			type = "Int64";
			break;

		case CL_UInt64:
			type = "UInt64";
			break;

		case CL_Float32:
			type = "Float32";
			break;

		case CL_Float64:
			type = "Float64";
			break;

		case CL_Float32Array:
			type = "Float32Array";
			break;

		case CL_Object:
			type = "Object";
			break;

		default:
			type = "Unknown";
			break;
	}

	return True;
}
// ******************************************************************


// ******************************************************************
// TODO: Permission handling methods
// ******************************************************************
bool ConfigurableProperty::SetPermissions(CL_PropertyPermissions newPermissions)
{
	this->permissions = newPermissions;

	return True;
}


CL_PropertyPermissions ConfigurableProperty::GetPermissions()
{
	return this->permissions;
}


bool ConfigurableProperty::GetPermissions(FString &permissionString)
{
	switch(this->permissions)
	{
		case CL_PERMISSIONS_NONE:
			permissionString = "PERMISSIONS_NONE";
			break;

		case CL_PERMISSIONS_GET:
			permissionString = "PERMISSIONS_GET";
			break;

		case CL_PERMISSIONS_SET:
			permissionString = "PERMISSIONS_SET";
			break;

		case CL_PERMISSIONS_BOTH:
			permissionString = "PERMISSIONS_BOTH";
			break;

		default:
			permissionString = "PERMISSIONS_UNKNOWN";
			break;
	}

	return True;
}
// ******************************************************************


// ******************************************************************
// Set/Get Property
// TODO: Check if err is being written in all cases
// ******************************************************************
bool ConfigurableProperty::Set(ConfigurationDataBase &configCdb, CL_PropertyErrors &err)
{
	// TODO: Int64, UInt64, ...

	CDBExtended cdb(configCdb);

	// DEBUG
	CStaticAssertErrorCondition(InitialisationError, "ConfigurableProperty::Set");

	//
	// Int8
	//
	if(this->type == CL_Int8)
	{
		int32 value;

		if(!cdb.ReadInt32(value, "Value"))
		{
			err = CL_ERROR_CDBREAD;
			return False;
		}

		//
		// Change the value
		//
		*((int8 *) this->property) = (int8) value;
	}

	//
	// UInt8
	//
	if(this->type == CL_UInt8)
	{
		int32 value;

		if(!cdb.ReadInt32(value, "Value"))
		{
			err = CL_ERROR_CDBREAD;
			return False;
		}

		//
		// Change the value
		//
		*((uint8 *) this->property) = (uint8) value;
	}

	//
	// Int16
	//
	if(this->type == CL_Int16)
	{
		int32 value;

		if(!cdb.ReadInt32(value, "Value"))
		{
			err = CL_ERROR_CDBREAD;
			return False;
		}

		//
		// Change the value
		//
		*((int16 *) this->property) = (int16) value;
	}

	//
	// UInt16
	//
	if(this->type == CL_UInt16)
	{
		int32 value;

		if(!cdb.ReadInt32(value, "Value"))
		{
			err = CL_ERROR_CDBREAD;
			return False;
		}

		//
		// Change the value
		//
		*((uint16 *) this->property) = (uint16) value;
	}

	//
	// Int32
	//
	if(this->type == CL_Int32)
	{
		int32 value;

		if(!cdb.ReadInt32(value, "Value"))
		{
			err = CL_ERROR_CDBREAD;
			return False;
		}

		//
		// Change the value
		//
		*((int32 *) this->property) = value;
	}

	//
	// UInt32
	//
	if(this->type == CL_UInt32)
	{
		int32 value;

		if(!cdb.ReadInt32(value, "Value"))
		{
			err = CL_ERROR_CDBREAD;
			return False;
		}

		//
		// Change the value
		//
		*((uint32 *) this->property) = (uint32) value;
	}

	//
	// Float32
	//
	if(this->type == CL_Float32)
	{
		float value;

		if(!cdb.ReadFloat(value, "Value"))
		{
			err = CL_ERROR_CDBREAD;
			return False;
		}

		//
		// Change the value
		//
		*((float *) this->property) = value;
	}

	//
	// Float32Array
	//
	if(this->type == CL_Float32Array)
	{
		int dimension;
		int *size = NULL;
		FString error;

		// Read the dimension of the received array
		if(!LoadDimension(cdb, "Value", dimension, (int *&) size, error))
		{
			err = CL_ERROR_COULDNOTREADDIM;
			return False;
		}

		// Check the dimension of the received array
		if(dimension == 0)
		{
			err = CL_ERROR_WRONGARRAYDIM;
			return False;
		}
		if(dimension != this->dimensions)
		{
			err = CL_ERROR_WRONGARRAYDIM;
			return False;
		}
		for(int i = 0 ; i < this->dimensions ; i++)
		{
			if(size[i] != this->size[i])
			{
				err = CL_ERROR_WRONGARRAYDIM;
				return False;
			}
		}
		delete[] size;

		// Read the array
		if(!cdb.ReadFloatArray((float *) this->property, this->size, this->dimensions, "Value"))
		{
			err = CL_ERROR_CDBREAD;
			return False;
		}
	}

	//
	// Float64
	//
	if(this->type == CL_Float64)
	{
		double value;

		if(!cdb.ReadDouble(value, "Value"))
		{
			err = CL_ERROR_CDBREAD;
			return False;
		}

		//
		// Change the value
		//
		*((double *) this->property) = value;
	}

	//
	// Object
	//
	if(this->type == CL_Object)
		((Object *) this->property)->ObjectLoadSetup(cdb, NULL);

	err = CL_ERROR_NONE;
	return True;
}


bool ConfigurableProperty::Get(ConfigurationDataBase &configCdb, CL_PropertyErrors &err)
{
	// TODO: Int64, UInt64, ...

	// DEBUG
	CStaticAssertErrorCondition(InitialisationError, "ConfigurableProperty::Get");

	//
	// Int8
	//
	if(this->type == CL_Int8)
	{
		int32 value = (int32) *((int8 *) this->property);

		FString valString;
		valString.SSPrintf((uint32) 0, "%d", value);

		FString configString = "\tValue = ";
		configString += valString;
		configString += "\n";

		if(!this->CreateCDB(configString, configCdb))
			return False;
	}

	//
	// UInt8
	//
	if(this->type == CL_UInt8)
	{
		uint32 value = (uint32) *((uint8 *) this->property);

		FString valString;
		valString.SSPrintf((uint32) 0, "%u", value);

		FString configString = "\tValue = ";
		configString += valString;
		configString += "\n";

		if(!this->CreateCDB(configString, configCdb))
			return False;
	}

	//
	// Int16
	//
	if(this->type == CL_Int16)
	{
		int32 value = (int32) *((int16 *) this->property);

		FString valString;
		valString.SSPrintf((uint32) 0, "%d", value);

		FString configString = "\tValue = ";
		configString += valString;
		configString += "\n";

		if(!this->CreateCDB(configString, configCdb))
			return False;
	}

	//
	// UInt16
	//
	if(this->type == CL_UInt16)
	{
		uint32 value = (uint32) *((uint16 *) this->property);

		FString valString;
		valString.SSPrintf((uint32) 0, "%u", value);

		FString configString = "\tValue = ";
		configString += valString;
		configString += "\n";

		if(!this->CreateCDB(configString, configCdb))
			return False;
	}

	//
	// Int32
	//
	if(this->type == CL_Int32)
	{
		int32 value = *((int32 *) this->property);

		FString valString;
		valString.SSPrintf((uint32) 0, "%d", value);

		FString configString = "\tValue = ";
		configString += valString;
		configString += "\n";

		if(!this->CreateCDB(configString, configCdb))
			return False;
	}

	//
	// UInt32
	//
	if(this->type == CL_UInt32)
	{
		uint32 value = *((uint32 *) this->property);

		FString valString;
		valString.SSPrintf((uint32) 0, "%u", value);

		FString configString = "\tValue = ";
		configString += valString;
		configString += "\n";

		if(!this->CreateCDB(configString, configCdb))
			return False;
	}

	//
	// Float32
	//
	if(this->type == CL_Float32)
	{
		float value = *((float *) this->property);

		FString valString;
		valString.SSPrintf((uint32) 0, "%f", value);

		FString configString = "\tValue = ";
		configString += valString;
		configString += "\n";

		if(!this->CreateCDB(configString, configCdb))
			return False;
	}

	//
	// Float32Array
	//
	if(this->type == CL_Float32Array)
	{
		CDBExtended cdb(configCdb);

		// Check if the array exists
		if((this->dimensions == 0) || (this->size == NULL))
		{
			err = CL_ERROR_ARRAYDOESNOTEXIST;
			return False;
		}

		// Write the array
		if(!cdb.WriteFloatArray((float *) this->property, this->size, this->dimensions, "Value"))
		{
			err = CL_ERROR_CDBWRITE;
			return False;
		}
	}

	//
	// Float64
	//
	if(this->type == CL_Float64)
	{
		double value = *((double *) this->property);

		FString valString;
		valString.SSPrintf((uint32) 0, "%f", value);

		FString configString = "\tValue = ";
		configString += valString;
		configString += "\n";

		if(!this->CreateCDB(configString, configCdb))
			return False;
	}

	//
	// Object
	//
	if(this->type == CL_Object)
		((Object *) this->property)->ObjectSaveSetup(configCdb, NULL);

	return True;
}
// ******************************************************************


// ******************************************************************
// Create CDB from FString method
// ******************************************************************
bool ConfigurableProperty::CreateCDB(FString &data, ConfigurationDataBase &cdb)
{
	SXMemory config((char *) data.Buffer(), strlen(data.Buffer()));

	return cdb->ReadFromStream(config, NULL);
}
// ******************************************************************
