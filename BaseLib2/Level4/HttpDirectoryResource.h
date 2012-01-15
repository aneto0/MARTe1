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
 * An HTTP node serving a file directory
 */

#if !defined (HTTP_DIRECTORY_RESOURCE)
#define HTTP_DIRECTORY_RESOURCE

#include "System.h"
#include "HttpStream.h"
#include "HttpInterface.h"


OBJECT_DLL(HttpDirectoryResource)
class HttpDirectoryResource: public GCNamedObject,public HttpInterface{
    OBJECT_DLL_STUFF(HttpDirectoryResource)

    /** the directory that is broadcasted */
    BString baseDir;

    /** the name of a start html file */
    BString startHtml;
    
    /** the filter to use on the directory */
    BString fileFilter;

    /** sends a file */
    bool FileAction(FString &fileName,HttpStream &hstream);

public:

    /** the main entry point for HttpInterface */
    virtual     bool                ProcessHttpMessage(HttpStream &hStream);

    /** the default constructor */
    HttpDirectoryResource(){
        baseDir = "c:/";
        startHtml = "index";
    }

    /** save an object content into a set of configs */
    virtual     bool                ObjectSaveSetup(
            ConfigurationDataBase &     info,
            StreamInterface *           err){

        GCNamedObject::ObjectSaveSetup(info,err);
        CDBExtended &cdbx = (CDBExtended &)info;

        cdbx.WriteString(baseDir.Buffer(),"BaseDir");
        cdbx.WriteString(startHtml.Buffer(),"StartHtml");
        cdbx.WriteString(fileFilter.Buffer(),"FileFilter");
        return HttpInterface::ObjectSaveSetup(info,err);
    }

    /**  initialise an object from a set of configs */
    virtual     bool                ObjectLoadSetup(
            ConfigurationDataBase &     info,
            StreamInterface *           err){

        GCNamedObject::ObjectLoadSetup(info,err);
        CDBExtended &cdbx = (CDBExtended &)info;

        cdbx.ReadBString(baseDir,"BaseDir","");
        cdbx.ReadBString(startHtml,"StartHtml","index");
        cdbx.ReadBString(fileFilter,"FileFilter","*");
        return HttpInterface::ObjectLoadSetup(info,err);
    }

};

#endif
