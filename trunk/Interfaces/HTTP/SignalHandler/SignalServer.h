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

#if !defined(SIGNAL_SERVER_H)
#define SIGNAL_SERVER_H

#include "ConfigurationDataBase.h"
#include "CDBExtended.h"
#include "MessageHandler.h"
#include "CDBBrowserMenu.h"
#include "SignalInterface.h"
#include "SignalMessageInterface.h"

OBJECT_DLL(SignalServer)

/** a generic handler for GAP messages */
class SignalServer:public  SignalMessageInterface,
                   public  HttpInterface
                   {
    OBJECT_DLL_STUFF(SignalServer)

    /**if true SignalInterface instead of  is used in GCRTemplate and data retrived as binary format**/
    int32                            binaryDownloadMode;

public:
    /** */
    virtual ~SignalServer()
    {
    };

    /** */
    SignalServer(){
        binaryDownloadMode=0;
    }

    /** The HTTP entry point */
    virtual     bool                ProcessHttpMessage(HttpStream &hStream);

    /**
        This object behaves like as GenericSignalServer
     */
    virtual     bool                ObjectLoadSetup(
            ConfigurationDataBase &     info,
            StreamInterface *           err){

        bool ret = True;
        ret = ret && SignalMessageInterface::ObjectLoadSetup(info,err);

        CDBExtended cdbx(info);

        if (!cdbx.ReadInt32((int32 &)binaryDownloadMode,"BinaryDownloadMode",0)){
            AssertErrorCondition(Warning,"binaryDownloadMode default False");
        }

        return ret;
    }


};

#endif
