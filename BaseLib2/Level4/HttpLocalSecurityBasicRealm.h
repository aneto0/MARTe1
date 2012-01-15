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
 * A class providing access to local security system
 */
#if !defined (HTTP_LOCAL_SECURITY_BASIC_REALM)
#define HTTP_LOCAL_SECURITY_BASIC_REALM

#include "System.h"
#include "HttpRealm.h"

OBJECT_DLL(HttpLocalSecurityBasicRealm)

class HttpLocalSecurityBasicRealm: public GCNamedObject, public HttpRealm {

    OBJECT_DLL_STUFF(HttpLocalSecurityBasicRealm)

public:

    /** This tool only supports basic security */
    virtual bool DigestSecurityNeeded();

    /** True if the key is valid in the context
        @param command is the HTTP command used
        @param ipNumber is the ip of the client */
    virtual bool        Validate(
                            const char *            key,
                            HSHttpCommand           command,
                            uint32                  ipNumber);

    /** authentication request */
    virtual bool GetAuthenticationRequest(FString &message);

    /** No parameters
        only those of GCNamedObject */
    virtual     bool        ObjectLoadSetup(
            ConfigurationDataBase & info,
            StreamInterface *       err){

        return GCNamedObject::ObjectLoadSetup(info,err);
    }

};

#endif

