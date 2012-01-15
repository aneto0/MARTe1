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
 * @brief HTML drawing with flot http://code.google.com/p/flot/
 */
#if !defined(FLOT_PLOT_INTERFACE_H)
#define FLOT_PLOT_INTERFACE_H

#include "ConfigurationDataBase.h"
#include "CDBExtended.h"
#include "MessageHandler.h"
#include "CDBBrowserMenu.h"
#include "SignalInterface.h"
#include "SignalMessageInterface.h"

/**
 * @brief Provides web plotting with zoom to BaseLib2 classes
 *
 * This class is capable of plotting data using
 * using flot: http://code.google.com/p/flot/
 * It has to be extended by a signal provider implementing the GetSignalData method
 *
 * The url arguments are:\n
 * SignalList=mandatory comma separated signal names\n
 * xxSignal=optional signal name for the xx axis\n
 * xxMultiplier=a multiplier for the xx values\n
 * yyMultiplier=a multiplier for the yy values\n
 * drawLines=true to draw the lines between the points (fast)\n
 * drawPoints=true to draw a circle around each point (slow)\n
 * Width=optional width of the chart (default 600)\n
 * Height=optional height of the chart (default 300)\n
 * example:http://localhost:8084/BROWSE/FlotPlot/?SignalList=CYCLETIME,TIMERRELATIVEUSECTIME&xxSignal=TIME&xxMultiplier=1e-6&yyMultiplier=1e6\n
 *
 * It also expects to find the flot scripts in the URL http://.../FLOT_DIR/
 */
class FlotPlotInterface :public  HttpInterface{
private:
    /**
     * Printfs a value in the HttpStream, based on the signal type and on the multiplier value
     * @param buffer the signal buffer
     * @param btd the signal type
     * @param hStream the http stream where data is written to
     * @param multiplier value to be applied to the value to be printed
     * @param downsample the downsample value (only samples multiple of this value are considered)
     */
    void PrintNextValue(char *&buffer, BasicTypeDescriptor &btd, Streamable &hStream, float multiplier = 1.0, int32 downsample = 1);
    /**
     * Utility method to convert a BaseLib2 signal into a javascript flot representation
     * @param xxSignal the signal for the xx axis (can be not valid)
     * @param signalGCO the signal to be displayed
     * @param hStream the http stream where data is written to
     * @param xxMultiplier value which will multiply all the xx values
     * @param yyMultiplier value which will multiply all the yy values
     * @param downsample the downsample value (only samples multiple of this value are considered)
     */
    void PopulateSignalData(GCRTemplate<SignalInterface> xxSignal, GCRTemplate<SignalInterface> signalGCO, Streamable &hStream, float xxMultiplier = 1.0, float yyMultiplier = 1.0, int32 downsample = 1);
public:
    /** */
    virtual ~FlotPlotInterface(){
    };

    /** */
    FlotPlotInterface(){
    }

    /** The HTTP entry point */
    virtual     bool                ProcessHttpMessage(HttpStream &hStream);

    /** 
     * Retrieve a signal based on the name
     * @param xxSignalName the signal name
     * @param yySignalName the signal yy data encoded in a SignalInterface
     * @param xxSignal the signal xx data (usually time) encoded in a SignalInterface
     * @param yySignal the signal yy data encoded in a SignalInterface
     * @return True if at least the yySignal data is valid
     */
    virtual bool GetSignalData(FString xxSignalName, FString yySignalName, GCRTemplate<SignalInterface> &xxSignal, GCRTemplate<SignalInterface> &yySignal) = 0;
};

#endif

