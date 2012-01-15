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
 * The base class for a generic HTTP resource object
 */
#if !defined (HTTP_INTERFACE_)
#define HTTP_INTERFACE_

#include "HttpRealm.h"
#include "CDBExtended.h"
#include "HttpStream.h"

class HttpInterface{

public:

    /** a reference to the realm database */
            GCRTemplate<HttpRealm>      realm;

    /** the page name to be presented to the viewer */
            FString                     comment;

public:

    const char *Comment()
    {
        if (comment.Size() > 0) return comment.Buffer();
        GCNamedObject *gcno = dynamic_cast<GCNamedObject *>(this);
        if (gcno != NULL)       return gcno->Name();
        return "NO NAME";
    }

    virtual ~HttpInterface(){};

    /** a reference to the realm database */
    GCRTemplate<HttpRealm>          Realm()
    {
        return realm;
    }

    /** The message is actually a multi stream (HttpStream)
        with a convention described in HttpStream*/
    virtual     bool                ProcessHttpMessage(HttpStream &hStream) = 0;

    /** parameters are Realm and Comment
        Realm is a GCReference (see the class for parameters)
        Comment is a string used as description of page    */
    bool ObjectLoadSetup(
            ConfigurationDataBase &     info,
            StreamInterface *           err){
        CDBExtended cdb(info);
        cdb.ReadFString(comment,"Comment","");
        if (cdb->Move("Realm")){
            if (!realm.ObjectLoadSetup(cdb,err)) return False;
        }
        return True;
    }

    /** parameters are same as ObjectLoadSetup */
    bool ObjectSaveSetup(
            ConfigurationDataBase &     info,
            StreamInterface *           err){
        CDBExtended cdb(info);
        if (comment.Size()>0) cdb.WriteFString(comment,"Comment");
        if (cdb->Move("Realm")){
            if (!realm.ObjectLoadSetup(cdb,err)) return False;
        }
        return True;
    }

};


#endif
