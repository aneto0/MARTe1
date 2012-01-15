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

#include "URL.h"
#include "FString.h"

bool URLLoad(URL &url, Streamable &stream){
    FString ptoken;
    stream.GetToken(ptoken,"/",NULL,"");

    stream.GetToken(url.server,"/",NULL,"");

    // evaluate protocol
    url.protocol = URLP_NONE;
    if (strncasecmp(ptoken.Buffer(),"http:",6)==0){
        url.protocol = URLP_HTTP;
        url.port = 80;
    } else
    if (strncasecmp(ptoken.Buffer(),"ftp:",5)==0){
        url.protocol = URLP_FTP;
        url.port = 25;
    } else
    if (strncasecmp(ptoken.Buffer(),"file:",6)==0){
        url.protocol = URLP_FILE;
    }

    url.server.SetSize(0);
    char term;
    stream.GetToken(url.server,"/:",&term,"");

    FString potoken;
    if (term == ':'){
        stream.GetToken(potoken,"/",NULL,"");
        url.port = atoi(potoken.Buffer());
    }

    url.uri.PutC('/');
    stream.GetLine(url.uri);

    return True;
}

bool URL::Save(Streamable &stream){
    switch(protocol){
        case URLP_HTTP:{
            stream.Printf("http://%s:%i/%s",server.Buffer(),port,uri.Buffer());
        } break;
        case URLP_FTP:{
            stream.Printf("ftp://%s/%s",server.Buffer(),uri.Buffer());
        } break;
        case URLP_FILE:{
            stream.Printf("file:///%s",uri.Buffer());
        } break;
        default: return False;
    }
    return True;
}

