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
 * @file allows to upload configuration file to MARTe
 */
#ifndef CFG_UPLOADER_H
#define CFG_UPLOADER_H

#include "System.h"
#include "HttpStream.h"
#include "HttpInterface.h"
#include "HtmlStream.h"
#include "FString.h"
#include "GCReferenceContainer.h"
#include "CDBExtended.h"
#include "GlobalObjectDataBase.h"
#include "MessageHandler.h"

OBJECT_DLL(CFGUploader)
class CFGUploader: public GCReferenceContainer, public HttpInterface, public MessageHandler{
OBJECT_DLL_STUFF(CFGUploader)

private:
/** The id of the configuration file entry as received from the http request*/
    FString configFileID;

/** The location of MARTe*/
    FString marteLocation;

/** Timeout for the upload */
    int32 uploadTimeoutMSec;

public:

    /** the main entry point for HttpInterface */
    virtual     bool                ProcessHttpMessage(HttpStream &hStream);

    /** the default constructor */
    CFGUploader(){
        uploadTimeoutMSec = 30000;            
    }

    ~CFGUploader(){
    }

    /** save an object content into a set of configs */
    virtual     bool                ObjectSaveSetup(
            ConfigurationDataBase &     info,
            StreamInterface *           err){

        GCReferenceContainer::ObjectSaveSetup(info,err);
        return HttpInterface::ObjectSaveSetup(info,err);
    }

    /**  initialise an object from a set of configs */
    virtual     bool                ObjectLoadSetup(
            ConfigurationDataBase &     info,
            StreamInterface *           err){

        GCReferenceContainer::ObjectLoadSetup(info,err);

        CDBExtended &cdbx = (CDBExtended &)info;

        cdbx.ReadFString(configFileID, "configFileID", "cfgFile");

        if(!cdbx.ReadFString(marteLocation, "MARTeLocation", "MARTe")){
            AssertErrorCondition(Warning, "ObjectLoadSetup::MARTe location wasn not specified. Using default: %s", marteLocation.Buffer());
        }
        //Try to read the upload timeout specified in secs or usecs
        if(!cdbx.ReadInt32(uploadTimeoutMSec, "UploadTimeoutMSec")){
            cdbx.ReadInt32(uploadTimeoutMSec, "UploadTimeoutSec", 10);
            uploadTimeoutMSec *= 1000;
        }
        return HttpInterface::ObjectLoadSetup(info,err);
    }
    
private:
    /**Utility method to print the form*/
    bool PrintHTTPForm(HtmlStream &hmStream);
};
#endif
