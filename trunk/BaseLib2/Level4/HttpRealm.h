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
 * A container of security options
 */

#if !defined (HTTP_REALM)
#define HTTP_REALM

#include "GCNamedObject.h"
#include "HttpDefinitions.h"

class FString;

class HttpRealm{

public:
    /** True if the key is valid in the context
        @param command is the HTTP command used
        @param ipNumber is the ip of the client */
    virtual bool        Validate(
                            const char *            key,
                            HSHttpCommand           command,
                            uint32                  ipNumber) = 0;

    /** True if digest authorisation needed */
    virtual bool        DigestSecurityNeeded() = 0;

    /** Return the value to associate to WWW-Authenticate */
    virtual bool        GetAuthenticationRequest(FString &message)= 0;

    /** requires GCNamedObject or derivatives in the parenthood */
    const char *        RealmName()
    {
        GCNamedObject *gcno = dynamic_cast<GCNamedObject *>(this);
        if (gcno) return gcno->Name();
        return "";
    }

};

#endif
