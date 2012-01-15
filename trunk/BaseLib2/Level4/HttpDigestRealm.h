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
 * Password protection support
 */

#if !defined (HTTP_DIGEST_REALM)
#define HTTP_DIGEST_REALM

#include "System.h"
#include "HttpRealm.h"

extern "C" {

    /** to generate entries in the configuration file */
    void GeneratePasswordDigest(const char *user,const char *password,const char *realm,FString &HA1);
}

OBJECT_DLL(HttpDigestRealm)

/** A class providing access to local security system */
class HttpDigestRealm: public GCNamedObject, public HttpRealm {

    OBJECT_DLL_STUFF(HttpDigestRealm)

    /** calculates a code that is unique to connection instance
        the call is multi-thread safe since it uses thread-specific information */
    void CalculateNonce(FString &nonce);

    /** a NULL terminated list of strings */
    const char **   users;

    /** memory deallocation */
    void                CleanUp()
    {
        if (users != NULL){
            int index = 0;
            while (users[index] != NULL){
                free((void *&)users[index]);
                index++;
            }
            free((void *&)users);
        }
    }

    /** retrieves the cooked up user and password and validate
        against @param command and @param ipNumber*/
    bool                FindUser(
                            const char *            userName,
                            FString &               HA1,
                            HSHttpCommand           command,
                            uint32                  ipNumber);
public:

    /** This tool only supports digest security */
    virtual bool        DigestSecurityNeeded();

    /** True if the key is valid in the context
        @param command is the HTTP command used
        @param ipNumber is the ip of the client */
    virtual bool        Validate(
                            const char *            key,
                            HSHttpCommand           command,
                            uint32                  ipNumber);

    /** authentication request */
    virtual bool        GetAuthenticationRequest(
                            FString &               message);

    /** constructor */
                        HttpDigestRealm()
    {
        users = NULL;
    }

    /** destructor */
                        ~HttpDigestRealm()
    {
        CleanUp();
    }

    /**
        Users = {
            UserName = [code]:[commands{Get Put Head pOst}]:[ip]:[mask(FFFFFFFF)]:[ip]:[mask(FFFFFFFF)]...
            code is either [DIGEST]<32 character md5(128) digest of user:realm:password (lower case exadecimal characters)>
            or             B64<BASE64 of Password>
            or             CLEAR<Password>
            ....
            empty code means do not check code
            empty commands means do not check commands
            etc..

        }

        + those of GCNamedObject */
    virtual     bool    ObjectLoadSetup(
                            ConfigurationDataBase & info,
                            StreamInterface *       err);


};

#endif

