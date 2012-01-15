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

#if !defined (PLOT_WINDOW_H)
#define PLOT_WINDOW_H

#include "GAM.h"
#include "DDBInputInterface.h"
#include "FastPollingMutexSem.h"

const uint32 TYPE_NONE   = 0x0;
const uint32 TYPE_FLOAT  = 0x1;
const uint32 TYPE_INT32  = 0x2;
const uint32 TYPE_UINT32 = 0x4;

const uint32 YSCALE_LINEAR = 0x0;
const uint32 YSCALE_LOG10  = 0x1;

class SignalInfo {
public:
    DDBInputInterface *input;

    FString            signalName;

    FString            signalType;

    uint32             signalTypeMask;

    uint32             numberOfSamplesPerCycle;

    uint32             numberOfBuffers;

    uint32             numberOfBytesPerBuffer;

    uint32             buffer2PlotTotalNumberOfSamples;

    uint32             buffer2PlotTotalNumberOfBytes;

    char              *buffer2plot;
public:
    SignalInfo() {
	input = NULL;
	buffer2plot = NULL;
	signalTypeMask = TYPE_NONE;
    }
    ~SignalInfo() {
	if(buffer2plot != NULL) {
	    free((void*&)buffer2plot);
	    buffer2plot = NULL;
	}
    }
};

OBJECT_DLL(PlotWindow)
class PlotWindow : public GAM {
OBJECT_DLL_STUFF(PlotWindow)

private:

    DDBInputInterface  *xAxisInput;

    char               *xAxisArray;

    FString             xAxisSignalName;

    FString             xAxisSignalType;

    uint32              xAxisSignalTypeMask;

    float               xAxisScaleFactor;

    uint32              yAxisScaleMask;

    uint32              totalNumberOfSignals;

    SignalInfo         *signals;

    int32               numberOfBuffers2Plot;

private:

    FastPollingMutexSem sem;

public:

    int32               plotPixelWidth;

    int32               plotPixelHeight;

    float               xLimits[2];

    FString             title;

    FString             xLabel;

    FString             yLabel;

    bool                onlyPositiveYBool;

public:

    /// Constructor
    PlotWindow() {
	sem.Create();

	xAxisInput = NULL;
	xAxisArray = NULL;
	xAxisSignalTypeMask = TYPE_NONE;
	xAxisScaleFactor = 1.0;
	yAxisScaleMask = YSCALE_LINEAR;
	totalNumberOfSignals = 0;
	numberOfBuffers2Plot = 1;
	xLimits[0] = 0.0;
	xLimits[1] = 0.0;
    };

    /// Destructor
    ~PlotWindow() {
	if(signals != NULL) {
	    delete [] signals;
	    signals = NULL;
	}
	if(xAxisArray != NULL) {
	    free((void*&)xAxisArray);
	    xAxisArray = NULL;
	}
    };
    
    /// GAM configuration
    bool  Initialise(ConfigurationDataBase &cdb);

    /// GAM execution method
    bool  Execute(GAM_FunctionNumbers execFlag);

    bool  AppendJsDataString(FString &jsDataString);
};
#endif
