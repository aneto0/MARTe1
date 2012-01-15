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
 * A tool to encode and de-encode urls
 */
#if !defined (URL_H)
#define URL_H

#include "FString.h"

class URL;

/** the protocols that can be specified in a url */
enum URLProtocols{
    /** NONE  */
    URLP_NONE = 0,

    /** HTTP  */
    URLP_HTTP = 1,

    /** FTP */
    URLP_FTP  = 2,

    /** FILE */
    URLP_FILE = 3
};

extern "C" {
    bool URLLoad(URL &url, Streamable &s);
}

/** the URL */
class URL{

    /** valid for HTTP,FTP */
    FString         server;

    /** valid for HTTP,FTP*/
    uint32          port;

    /** HTTP FTP FILE */
    URLProtocols    protocol;

    /** The resource identification */
    FString         uri;

public:
    /** constructor */
    URL(){
        Init();
    }

    /** Initialise from discrete information */
    bool            Init(
                        const char *        server      = "",
                        const char *        uri         = "",
                        uint32              port        = 80,
                        URLProtocols        protocol    = URLP_HTTP)
    {
        this->server    = server;
        this->uri       = uri;
        this->port      = port;
        this->protocol  = protocol;
        return True;
    }

    /** Initialise from information on a stream*/
    bool            Load(Streamable &       stream){
        return URLLoad(*this, stream);
    }

    /** Initialise from information on a stream*/
    bool            Load(const char *       buffer)
    {
        FString s;
        s = buffer;
        s.Seek(0);
        return Load(s);
    }

    /** encode to a stream */
    bool            Save(Streamable &       stream);

    /** the decoded server */
    const char *    Server()
    {
        return server.Buffer();
    }

    /** The resource locator */
    const char *    URI()
    {
        return uri.Buffer();
    }

    /** The comm port */
    uint32          Port()
    {
        return port;
    }

    /** The protocol */
    URLProtocols    Protocol()
    {
        return protocol;
    }

private:
    friend bool URLLoad(URL &url, Streamable &s);
};

#endif
