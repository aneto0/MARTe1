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
 * Renders information regarding all the threads which are alive
 */

#if !defined (HTTP_THREAD_LIST_RESOURCE)
#define HTTP_THREAD_LIST_RESOURCE

#include "System.h"
#include "HttpStream.h"
#include "HttpInterface.h"
#include "HtmlStream.h"


OBJECT_DLL(HttpThreadListResource)
/** a HTTP node listing the active threads*/
class HttpThreadListResource: public GCNamedObject,public HttpInterface{
    OBJECT_DLL_STUFF(HttpThreadListResource)
private:
    const char *css;

    /**
     * Helper function which outputs the thread state to the a html stream
     * @param hmStream the stream to write to
     * @param threadState the thread state as returned by ThreadsGetState
     */
    void PrintThreadStateAsString(HtmlStream &hmStream, uint32 threadState);

public:

    /** the main entry point for HttpInterface */
    virtual     bool                ProcessHttpMessage(HttpStream &hStream);

    /** the default constructor */
    HttpThreadListResource(){
         css = "table.bltable {"
                 "margin: 1em 1em 1em 2em;"
                 "background: whitesmoke;"
                 "border-collapse: collapse;"
               "}"
               "table.bltable th, table.bltable td {"
                 "border: 1px silver solid;"
                 "padding: 0.2em;"
               "}"
               "table.bltable th {"
                  "background: gainsboro;"
                  "text-align: left;"
               "}"
               "table.bltable caption {"
                  "margin-left: inherit;"
                  "margin-right: inherit;"
               "}";
    }

    /** save an object content into a set of configs */
    virtual     bool                ObjectSaveSetup(
            ConfigurationDataBase &     info,
            StreamInterface *           err){

        GCNamedObject::ObjectSaveSetup(info,err);
        return HttpInterface::ObjectSaveSetup(info,err);
    }

    /**  initialise an object from a set of configs */
    virtual     bool                ObjectLoadSetup(
            ConfigurationDataBase &     info,
            StreamInterface *           err){

        GCNamedObject::ObjectLoadSetup(info,err);
        return HttpInterface::ObjectLoadSetup(info,err);
    }

};

#endif
