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
 * A web component to manage multiple Http based applications
 * This service provides a link to the HTTP applications and
 * also assign a different port to each
 */
#if !defined (HTTP_SERVICE_RELAY_RESOURCE)
#define HTTP_SERVICE_RELAY_RESOURCE

#include "HttpGroupResource.h"

class HttpServiceRelayResource;


OBJECT_DLL(HttpServiceRelayResource)

/** It is a HttpGroupResource that uses puts each resource in a frame */
class HttpServiceRelayResource:public HttpGroupResource {

private:
OBJECT_DLL_STUFF(HttpServiceRelayResource)

    /** the next port to be used */
    int32                   nextHttpPort;

    /** remove server:ports that have been shut-down */
    void CleanUnusedPorts();

public:
    /** default constructor */
                            HttpServiceRelayResource(){
        nextHttpPort = 8081;
    };

    /** with comment */
                            HttpServiceRelayResource(
                const char *        comment){
        this->comment = comment;
        nextHttpPort = 8081;
    }

    virtual                ~HttpServiceRelayResource(){
    }

    /** the main entry point for HttpInterface */
    virtual     bool        ProcessHttpMessage(HttpStream &hStream);

    /**
        Parameters as specified in GCReferenceContainer and HttpInterface

    */
    virtual     bool        ObjectLoadSetup(
            ConfigurationDataBase & info,
            StreamInterface *       err){

        bool ret = HttpGroupResource::ObjectLoadSetup(info,err);

        CDBExtended cdbx(info);

        cdbx.ReadInt32(nextHttpPort,"StartHttpPort",8081);

        return ret;
    }

    /** Parameters as specified in GCReferenceContainer and HttpInterface
    */
    virtual     bool        ObjectSaveSetup(
            ConfigurationDataBase & info,
            StreamInterface *       err){

        bool ret = HttpGroupResource::ObjectSaveSetup(info,err);

        return ret;
    }

};



#endif



