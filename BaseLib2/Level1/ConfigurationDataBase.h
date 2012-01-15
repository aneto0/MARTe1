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

/** 
 * @file
 * A reference to a CDB
 */
#include "CDBVirtual.h"
#include "GCRTemplate.h"

#if !defined(_CONFIGURATION_DATABASE_)
#define _CONFIGURATION_DATABASE_

#include "CDBVirtual.h"
#include "CDBNull.h"
#include "ObjectRegistryDataBase.h"
#include "GCRTemplate.h"
#include "GCReference.h"


/** contains a reference to CDB of type CDBVirtual.
Forces call to CDB to be made via the virtual table */
class ConfigurationDataBase: public GCRTemplate<CDBVirtual>{

#if !defined(_CINT)

protected:

    /**  */
    void Load(CDBVirtual *p){
        GCRTemplate<CDBVirtual>::Load(p);
        if (!IsValid()) GCRTemplate<CDBVirtual>::Load(new CDBNull);
    }

public:

//##################################################################/
//##                                                              ##
//##            Primary   functions:   all inline                 ##
//##                                                              ##
//##################################################################/

    /** Copy Operator of a CDB. It creates a new reference to an existing database */
    inline void operator=(ConfigurationDataBase &base)
    {
        RemoveReference();
        if (base.IsValid()) {
            Load(base->Clone(CDBCM_CopyAddress));
        }
    }

    /** Constructor of a CDB. If base is specified it creates a new reference to an existing database
        @param base        If NULL creates a new database. If not NULL it creates a reference to an existing one
        @param cdbcm       Use CDBCM_CopyAddress to copy the address from the reference.
                           In case of two or more flags, after oring one must cast back to the enum type     */
    inline ConfigurationDataBase(ConfigurationDataBase &base,CDBCreationMode cdbcm=CDBCM_CopyAddress){
        if (base.IsValid()) {
            Load(base->Clone(cdbcm));
        }
    }

    /** Constructor of a CDB from a generic base type    */
    inline ConfigurationDataBase(const char *coreClassName="CDB"):
        GCRTemplate<CDBVirtual>(coreClassName)
    {
        if (!IsValid()) GCRTemplate<CDBVirtual>::Load(new CDBNull);
    }

    inline ConfigurationDataBase(GCReference cdb,CDBCreationMode cdbcm=CDBCM_CopyAddress)
    {
        GCRTemplate<CDBVirtual> test(cdb);
        if(test.IsValid()){
            Load(test->Clone(CDBCM_CopyAddress));
        }else {
            GCRTemplate<CDBVirtual>::Load(new CDBNull);
        }
    }

    /** will destroy database if instance count goes to 0 */
    virtual  ~ConfigurationDataBase(){}

    /** Allow access to the methods of CDB */
    CDBVirtual *operator->(){
        return GCRTemplate<CDBVirtual>::operator->();
    }

#endif //CINT
};

#endif

