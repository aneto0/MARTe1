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
#ifndef __GENERICTESTMODULE_H__
#define	__GENERICTESTMODULE_H__


#include <ConfigurationDataBase.h>
#include <CDBExtended.h>
#include <StreamInterface.h>
#include <SXMemory.h>


class GenericTestModule : public Object
{
private:

	int32 nelements;
	double *vector;


public:

	// Constructors and destructor
	GenericTestModule();
	virtual ~GenericTestModule();

	// Object setup
	virtual bool ObjectLoadSetup(ConfigurationDataBase &cdb, StreamInterface *err);
	virtual bool ObjectSaveSetup(ConfigurationDataBase &cdb, StreamInterface *err);
};


#endif	/* __GENERICTESTMODULE_H__ */

