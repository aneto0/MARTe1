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
 * An Http Server that can be inserted in object trees and
 * started and stopped by means of messages
 */
#if !defined (HTTP_SERVICE)
#define HTTP_SERVICE

#include "HttpBasicService.h"
#include "MessageHandler.h"


OBJECT_DLL(HttpService)

/** a mini web server with plugins and realm based security
see MultiClientClass for the activation method
and HttpGroupResource for the resource installation */
class HttpService: public HttpBasicService, public MessageHandler{

#if !defined(_CINT)

private:
OBJECT_DLL_STUFF(HttpService)

public:


    /** See HttpBasicService */
    virtual     bool                ObjectLoadSetup(
            ConfigurationDataBase & info,
            StreamInterface *       err){

        return HttpBasicService::ObjectLoadSetup(info,err);
    }

    /** See HttpBasicService */
    virtual     bool                ObjectSaveSetup(
            ConfigurationDataBase & info,
            StreamInterface *       err){

        return HttpBasicService::ObjectSaveSetup(info,err);
    }

    /** the function to be implemented by the user application
        Useful messages are those with a content START and STOP which will respectively
        start the server and stop the server. If explicity reply is requested it will
        be created with content "ACK" */
    virtual     bool                ProcessMessage(
            GCRTemplate<MessageEnvelope> envelope);

#endif
};


#endif

