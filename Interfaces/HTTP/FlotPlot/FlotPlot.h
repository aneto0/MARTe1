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

#if !defined(FLOT_PLOT_H)
#define FLOT_PLOT_H

/**
 * @file Using the SignalMessageInterface this class is capable of retrieving acquired data
 * Plotting delegated to the FlotPlotInterface
 */

#include "ConfigurationDataBase.h"
#include "CDBExtended.h"
#include "MessageHandler.h"
#include "CDBBrowserMenu.h"
#include "SignalInterface.h"
#include "SignalMessageInterface.h"
#include "FlotPlotInterface.h"

OBJECT_DLL(FlotPlot)

class FlotPlot:public  SignalMessageInterface,
                   public FlotPlotInterface 
                   {
    OBJECT_DLL_STUFF(FlotPlot)
    /** */
    virtual ~FlotPlot()
    {
    };

    /** */
    FlotPlot(){
    }

    /** @sa FlotPlotInterface::GetSignalData  */
    virtual bool GetSignalData(FString xxSignalName, FString signalName, GCRTemplate<SignalInterface> &xxSignal, GCRTemplate<SignalInterface> &yySignal){
        if(xxSignalName.Size() > 0){
            xxSignal = GetSignal(xxSignalName.Buffer());
        }
        yySignal = GetSignal(signalName.Buffer());
        return True;
    }

    /** The HTTP entry point */
    virtual     bool                ProcessHttpMessage(HttpStream &hStream);

    /**
     */
    virtual     bool                ObjectLoadSetup(
            ConfigurationDataBase &     info,
            StreamInterface *           err){

        bool ret = True;
        ret = ret && SignalMessageInterface::ObjectLoadSetup(info,err);

        return ret;
    }


};

#endif

