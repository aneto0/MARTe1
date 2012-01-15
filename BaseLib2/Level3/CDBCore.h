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
 * Implements the share database core
 */
#if !defined(CONFIGURATION_DATABASE_CORE)
#define CONFIGURATION_DATABASE_CORE

#include "ConfigurationDataBase.h"
#include "CDBGroupNode.h"
#include "MutexSem.h"

class CDBCore: public CDBGroupNode {

private:

    /** to synchronize the access to the database */
    MutexSem            mux;

public:
    /** */
    CDBCore(){
        mux.Create();
    }

    /** */
    virtual ~CDBCore(){
    }

    /** */
    virtual bool Lock()  {
        if (!mux.Lock()){
            CStaticAssertErrorCondition(FatalError,"CDBCore Lock failed !!");
            return False;
        }
        return True;
    }

    /** */
    virtual void UnLock(){
        mux.UnLock();
    }

};

#endif

