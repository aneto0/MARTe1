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
 * An object container with the additional ability to
 * display its content on the web
 */

#if !defined (HTTP_GROUP_RESOURCE)
#define HTTP_GROUP_RESOURCE

#include "System.h"
#include "HttpStream.h"
#include "GCReferenceContainer.h"
#include "HttpInterface.h"

class HttpGroupResource;


OBJECT_DLL(HttpGroupResource)

/** It is a GCReferenceContainer that also displays its content as HTML */
class HttpGroupResource:public GCReferenceContainer,public HttpInterface {

private:
OBJECT_DLL_STUFF(HttpGroupResource)
public:
    /** */
                            HttpGroupResource(){
    };

    /** */
                            HttpGroupResource(
                const char *        comment){
        this->comment = comment;
    }

    /** the main entry point for HttpInterface */
    virtual     bool                ProcessHttpMessage(HttpStream &hStream);

    /** Parameters as specified in GCReferenceContainer and HttpInterface
    */
    virtual     bool        ObjectLoadSetup(
            ConfigurationDataBase & info,
            StreamInterface *       err){

        bool ret = GCReferenceContainer::ObjectLoadSetup(info,err);
        ret = ret && HttpInterface::ObjectLoadSetup(info,err);
        return ret;
    }

    /** Parameters as specified in GCReferenceContainer and HttpInterface
    */
    virtual     bool        ObjectSaveSetup(
            ConfigurationDataBase & info,
            StreamInterface *       err){

        bool ret = GCReferenceContainer::ObjectSaveSetup(info,err);
        ret = ret && HttpInterface::ObjectSaveSetup(info,err);
        return ret;
    }

};

#endif

