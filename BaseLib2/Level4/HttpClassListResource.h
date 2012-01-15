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
 * Shows the list of registered classes
 */


#if !defined (HTML_CLASSLIST_RESOURCE)
#define HTML_CLASSLIST_RESOURCE

#include "HttpInterface.h"
#include "GCNamedObject.h"
#include "HttpStream.h"

OBJECT_DLL(HttpClassListResource)

/** let you display registered classes
    This class is to be used only internally to BASELIB2.
    Can be used as component of a web page by spacifying this class
    in a HttpGroupResource.
    Other parametyers are Basedir = the directory mapped
    and StartHtml = the start html file (.html is added ) */
class HttpClassListResource:public GCNamedObject,public HttpInterface {
OBJECT_DLL_STUFF(HttpClassListResource)


public:

    /** */
    HttpClassListResource(){}

    /** the main entry point for HttpInterface */
    virtual     bool                ProcessHttpMessage(HttpStream &hStream);

    /** save an object content into a set of configs */
    virtual     bool                ObjectSaveSetup(
            ConfigurationDataBase &     info,
            StreamInterface *           err){
        bool ret = GCNamedObject::ObjectSaveSetup(info,err);
        ret = ret && HttpInterface::ObjectSaveSetup(info,err);
        return ret;
    }

    /**  initialise an object from a set of configs */
    virtual     bool                ObjectLoadSetup(
            ConfigurationDataBase &     info,
            StreamInterface *           err){
        bool ret = GCNamedObject::ObjectLoadSetup(info,err);
        ret = ret && HttpInterface::ObjectLoadSetup(info,err);
        return ret;
    }


};



#endif
