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

#if !defined(MATLAB_HANDLER)
#define MATLAB_HANDLER


#include "ConfigurationDataBase.h"
#include "CDBExtended.h"
#include "MessageHandler.h"
#include "CDBBrowserMenu.h"
#include "SignalInterface.h"
#include "MatlabConverter.h"
#include "Matrix.h"
#include "SignalMessageInterface.h"

OBJECT_DLL(MATLABHandler)

/** a generic handler for GAP messages */
class MATLABHandler:        public  SignalMessageInterface,
                            public  HttpInterface
                            {
    OBJECT_DLL_STUFF(MATLABHandler)

protected:

    bool SaveSingleSignal(const char* signalName, MatlabConverter &mc, const char* matlabVarName);
    bool SaveAllSignals(MatlabConverter &mc);

    void PrintForm(HttpStream &hStream);

public:
    /** */
    virtual ~MATLABHandler() {
    };

    /** */
    MATLABHandler() {
    }

    /** The HTTP entry point */
    virtual     bool                ProcessHttpMessage(HttpStream &hStream);

    /**
        This object behaves like as GenericGAPMessageHandler
     */
    virtual     bool                ObjectLoadSetup(
            ConfigurationDataBase &     info,
            StreamInterface *           err){

        return SignalMessageInterface::ObjectLoadSetup(info,err);
    }
};

#endif

