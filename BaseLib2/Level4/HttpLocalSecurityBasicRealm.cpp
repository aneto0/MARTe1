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

#include "HttpLocalSecurityBasicRealm.h"
#include "Base64Codec.h"

OBJECTLOADREGISTER(HttpLocalSecurityBasicRealm,"$Id$")


/** This tool only supports basic security */
bool HttpLocalSecurityBasicRealm::DigestSecurityNeeded(){
    return False;
}

bool HttpLocalSecurityBasicRealm::GetAuthenticationRequest(FString &message){
    return message.Printf("Basic realm=\"%s\"",RealmName());
}

bool HttpLocalSecurityBasicRealm::Validate(const char *key,HSHttpCommand command,uint32 ipNumber){
    if (strncmp(key,"Basic",5)==0){
        key += 5;
        FString b64CodedKey;
        b64CodedKey = key;
        FString deCodedKey;
        B64Decode(b64CodedKey,deCodedKey);


        FString userName;
        deCodedKey.Seek(0);
        deCodedKey.GetToken(userName,":\n\t");
        FString passwd;
        deCodedKey.GetToken(passwd,":\n\t");

        bool ret;
#if defined (_WIN32)

        HANDLE hToken;
        ret = LogonUser(
          (char *)userName.Buffer(),
          ".",
          (char *)passwd.Buffer(),
          LOGON32_LOGON_NETWORK,
          LOGON32_PROVIDER_DEFAULT,
          &hToken);

        if (!ret){
            printf("error= 0x%x\n",GetLastError());
        }
#else
        ret = False;
#endif
        return ret;
    }

    return True;
}






