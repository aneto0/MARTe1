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
#ifndef __CONFIGURABLEPROPERTY_H__
#define	__CONFIGURABLEPROPERTY_H__


#include <GarbageCollectable.h>
#include <LinkedListable.h>
#include <ConfigurationDataBase.h>
#include <CDBExtended.h>
#include <SXMemory.h>
#include <LoadCDBObjectClass.h>


OBJECT_DLL(ConfigurableProperty)


enum CL_PropertyType
{
	CL_None         = 0,
	CL_Int8         = 1,
	CL_Int16        = 2,
	CL_Int32        = 3,
	CL_Int64        = 4,
	CL_UInt8        = 5,
	CL_UInt16       = 6,
	CL_UInt32       = 7,
	CL_UInt64       = 8,
	CL_Float32      = 9,
	CL_Float64      = 10,
	CL_Float32Array = 11,
	CL_Object       = 12
	// TODO: Faltam ainda os arrays e strings
};


enum CL_PropertyPermissions
{
	CL_PERMISSIONS_NONE = 1,
	CL_PERMISSIONS_GET  = 2,
	CL_PERMISSIONS_SET  = 3,
	CL_PERMISSIONS_BOTH = 4
};


enum CL_PropertyErrors
{
	CL_ERROR_NONE              = 0,
	CL_ERROR_CDBREAD           = 1,
	CL_ERROR_CDBWRITE          = 2,
	CL_ERROR_COULDNOTREADDIM   = 3,
	CL_ERROR_WRONGARRAYDIM     = 4,
	CL_ERROR_ARRAYDOESNOTEXIST = 5
};


class ConfigurableProperty : public GCNamedObject, public LinkedListable
{
private:
	void *property;
	FString name;
	CL_PropertyType type;
	CL_PropertyPermissions permissions;
	// TODO: Permissions
	int dimensions;
	int *size;


public:

	// Constructors and destructor
	ConfigurableProperty();
	virtual ~ConfigurableProperty();

	// Object load setup
	virtual bool ObjectLoadSetup(ConfigurationDataBase &info, StreamInterface *err);

	// Name
	bool SetName(const char *newName);
	const char *GetName();
	bool GetName(FString &name);

	// Type
	bool SetType(CL_PropertyType newType);
	CL_PropertyType GetType();
	bool GetType(FString &type);

	// Permissions
	bool SetPermissions(CL_PropertyPermissions newPermissions);
	CL_PropertyPermissions GetPermissions();
	bool GetPermissions(FString &permissionString);

	// Property pointer
	bool SetPropertyPointer(void *newPointer, int dims = 0, int *size = NULL);

	// Get/Set
	bool Set(ConfigurationDataBase &configCdb, CL_PropertyErrors &err);
	bool Get(ConfigurationDataBase &configCdb, CL_PropertyErrors &err);

	bool CreateCDB(FString &data, ConfigurationDataBase &cdb);

	OBJECT_DLL_STUFF(ConfigurableProperty)
};



#endif	/* __CONFIGURABLEPROPERTY_H__ */
