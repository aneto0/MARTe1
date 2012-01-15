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
 * A web resource that allows sending a set of pre-programmed messages
 * It is a container of of type MessageDeliveryRequest or
 * MessageEnvelope
 */
#if !defined (HTTP_MESSAGE_SEND_RESOURCE)
#define HTTP_MESSAGE_SEND_RESOURCE

#include "System.h"
#include "HttpStream.h"
#include "GCReferenceContainer.h"
#include "HttpInterface.h"

class HttpMessageSendResource;


OBJECT_DLL(HttpMessageSendResource)

/** It is a GCReferenceContainer that also displays its content as HTML */
class HttpMessageSendResource:public GCReferenceContainer,public HttpInterface {

private:
OBJECT_DLL_STUFF(HttpMessageSendResource)
public:
    /** */
                            HttpMessageSendResource(){
    };

    /** */
                            HttpMessageSendResource(
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


