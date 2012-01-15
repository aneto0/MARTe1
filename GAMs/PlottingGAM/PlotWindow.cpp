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

#include "PlotWindow.h"
#include "CDBExtended.h"

/// This method runs at initialisation
bool PlotWindow::Initialise(ConfigurationDataBase &cdbData) {

    CDBExtended cdb(cdbData);

    FString onlyPositiveY;
    cdb.ReadFString(onlyPositiveY, "OnlyPositiveY", "No");
    if(onlyPositiveY == "Yes" || onlyPositiveY == "yes" || onlyPositiveY == "YES") {
        onlyPositiveYBool = True;
    } else {
        onlyPositiveYBool = False;
    }

    cdb.ReadInt32(plotPixelWidth , "PlotPixelWidth" , 1250);
    cdb.ReadInt32(plotPixelHeight, "PlotPixelHeight", 350);

    if(cdb.ReadFloat(xLimits[0], "XAxisStartValue", 0.0)) {
        if(!cdb.ReadFloat(xLimits[1], "XAxisEndValue", 0.0)) {
            AssertErrorCondition(Warning, "PlotWindow::Initialise: %s XaxisStartValue found but XAxisEndValue not found", Name());
            xLimits[0] = 0.0;
            xLimits[1] = 0.0;
        }
    } else {
        xLimits[0] = 0.0;
        xLimits[1] = 0.0;
    }

    if(cdb.ReadFString(xAxisSignalName, "XAxisSignalName", "") && (xLimits[0] == xLimits[1])) {
        if(!cdb.ReadFString(xAxisSignalType, "XAxisSignalType", "")) {
            AssertErrorCondition(InitialisationError, "PlotWindow::Initialise: %s XAxisSignalName found but XAxisSignalType not found", Name());
            return False;
        }
        if(!AddInputInterface(xAxisInput, "XAxisInputInterface")) {
            AssertErrorCondition(InitialisationError, "PlotWindow::Initialise: %s failed to add XAxisInputInterface", Name());
            return False;
        }
        if(!(xAxisInput->AddSignal(xAxisSignalName.Buffer(), xAxisSignalType.Buffer()))) {
            AssertErrorCondition(InitialisationError, "PlotWindow::Initialise: %s failed to add XAxisInputInterface", Name());
            return False;
        }
        cdb.ReadFloat(xAxisScaleFactor, "XAxisScaleFactor", 1.0);
    }

    FString yAxisScaleString;
    if(cdb.ReadFString(yAxisScaleString, "YAxisScale", "Linear")) {
        if(yAxisScaleString == "Log10") {
            yAxisScaleMask = YSCALE_LOG10;
        } else {
            AssertErrorCondition(Warning, "PlotWindow::Initialise: %s YAxisScale unknown, assuming %s", Name(), yAxisScaleString.Buffer());
        }
    } else {
        yAxisScaleMask = YSCALE_LINEAR;
    }

    // Read number of buffers to plot
    if(!cdb.ReadInt32(numberOfBuffers2Plot, "PlotLastNBuffers", 50)) {
        AssertErrorCondition(Warning, "PlotWindow::Initialise: %s cannot read PlotLastNBuffers, assuming %d", Name(), numberOfBuffers2Plot);
    } else {
        if(numberOfBuffers2Plot <= 0) {
            AssertErrorCondition(InitialisationError, "PlotWindow::Initialise: %s PlotLastNBuffers <= 0", Name());
            return False;
        }
    }

    // Read axis labels
    cdb.ReadFString(xLabel, "XLabel", "");
    cdb.ReadFString(yLabel, "YLabel", "");

    if(!cdb->Move("InputSignals")) {
        AssertErrorCondition(InitialisationError, "PlotWindow::Initialise: %s cannot move to InputSignals", Name());
        return False;
    }

    // Check how many input signals
    if((totalNumberOfSignals = cdb->NumberOfChildren()) == 0) {
        AssertErrorCondition(InitialisationError, "PlotWindow::Initialise: %s totalNumberOfSignals = 0", Name());
        return False;
    }

    // Allocate memory for individual signal information
    if((signals = new SignalInfo[totalNumberOfSignals]) == NULL) {
        AssertErrorCondition(InitialisationError, "PlotWindow::Initialise: %s cannot allocate memory for signals", Name());
        return False;
    }

    // Add one input interface per signal and read SignalName and SignalType
    title.SetSize(0);
    title.Printf("[");
    for(int i = 0 ; i < totalNumberOfSignals ; i++) {
        FString inputInterfaceName;
        inputInterfaceName.SetSize(0);
        inputInterfaceName.Printf("%sInputInterface%d", Name(), i);
        if(!AddInputInterface(signals[i].input, inputInterfaceName.Buffer())) {
            AssertErrorCondition(InitialisationError,"PlotWindow::Initialise: %s failed to add input interface", Name());
            return False;
        }

        cdb->MoveToChildren(i);
        if(!cdb.ReadFString(signals[i].signalName, "SignalName")) {
            AssertErrorCondition(InitialisationError,"PlotWindow::Initialise: %s failed to read SignalName for signal %d", Name(), i);
            return False;
        }
        if(!cdb.ReadFString(signals[i].signalType, "SignalType")) {
            AssertErrorCondition(InitialisationError,"PlotWindow::Initialise: %s failed to read SignalType for signal %d", Name(), i);
            return False;
        }
        signals[i].input->AddSignal(signals[i].signalName.Buffer(), signals[i].signalType.Buffer());
        signals[i].numberOfSamplesPerCycle = signals[i].input->BufferWordSize(); // This works only because we have one input interface per signal

        if(i < totalNumberOfSignals-1) {
            title.Printf("'%s',", signals[i].signalName.Buffer());
        } else {
            title.Printf("'%s']", signals[i].signalName.Buffer());
        }

        signals[i].numberOfBuffers = numberOfBuffers2Plot;

        signals[i].buffer2PlotTotalNumberOfSamples = signals[i].numberOfSamplesPerCycle*signals[i].numberOfBuffers;

        // Allocate memory for buffer to be plotted
        if(signals[i].signalType == "float") {
            signals[i].numberOfBytesPerBuffer = signals[i].numberOfSamplesPerCycle*sizeof(float);
            signals[i].signalTypeMask = TYPE_FLOAT;
        } else if(signals[i].signalType == "int32") {
            signals[i].numberOfBytesPerBuffer = signals[i].numberOfSamplesPerCycle*sizeof(int32);
            signals[i].signalTypeMask = TYPE_INT32;
        } else if(signals[i].signalType == "uint32") {
            signals[i].numberOfBytesPerBuffer = signals[i].numberOfSamplesPerCycle*sizeof(uint32);
            signals[i].signalTypeMask = TYPE_UINT32;
        } else {
            AssertErrorCondition(InitialisationError, "PlotWindow::Initialise: %s unsupported type for %s", Name(), signals[i].signalName.Buffer());
            return False;
        }
        signals[i].buffer2PlotTotalNumberOfBytes = signals[i].numberOfBytesPerBuffer*signals[i].numberOfBuffers;
        signals[i].buffer2plot = (char *)malloc(signals[i].buffer2PlotTotalNumberOfBytes);

        cdb->MoveToFather();
    }

    if(xAxisInput) {
        if(xAxisSignalType == "float") {
            xAxisSignalTypeMask = TYPE_FLOAT;
        } else if(xAxisSignalType == "int32") {
            xAxisSignalTypeMask = TYPE_INT32;
        } else if(xAxisSignalType == "uint32") {
            xAxisSignalTypeMask = TYPE_UINT32;
        } else {
            AssertErrorCondition(InitialisationError, "PlotWindow::Initialise: %s unsupported type for %s", Name(), xAxisSignalName.Buffer());
            return False;
        }
    	if((xAxisArray = (char *)malloc(signals[0].buffer2PlotTotalNumberOfBytes)) == NULL) {
    	    AssertErrorCondition(InitialisationError, "PlotWindow::Initialise: %s Unable to allocate memory for xAxisArray", Name());
    	    return False;
    	}
    }

    return True;
}

/// Called in every control loop
bool PlotWindow::Execute(GAM_FunctionNumbers execFlag) {

    if(sem.FastTryLock()) {
	for(int i = 0 ; i < totalNumberOfSignals ; i++) {
	    signals[i].input->Read();
	    memmove(signals[i].buffer2plot, signals[i].buffer2plot + signals[i].numberOfBytesPerBuffer, signals[i].numberOfBytesPerBuffer*(signals[i].numberOfBuffers-1));
	    memcpy(signals[i].buffer2plot + signals[i].buffer2PlotTotalNumberOfBytes - signals[i].numberOfBytesPerBuffer, signals[i].input->Buffer(), signals[i].numberOfBytesPerBuffer);
	}
	
	if(xAxisInput) {
	    xAxisInput->Read();
	    memmove(xAxisArray, xAxisArray + signals[0].numberOfBytesPerBuffer, signals[0].numberOfBytesPerBuffer*(signals[0].numberOfBuffers-1));
	    memcpy(xAxisArray + signals[0].buffer2PlotTotalNumberOfBytes - signals[0].numberOfBytesPerBuffer, xAxisInput->Buffer(), signals[0].numberOfBytesPerBuffer);
	}
	sem.FastUnLock();
    }

    return True;
}

bool PlotWindow::AppendJsDataString(FString &jsDataString) {

    jsDataString.Printf("var pwData = new Array(%d);\n", totalNumberOfSignals);

    jsDataString.Printf("var title = %s;\n", title.Buffer());
    jsDataString.Printf("var xLabel = '%s';\n", xLabel.Buffer());
    jsDataString.Printf("var yLabel = '%s';\n", yLabel.Buffer());

    sem.FastLock();
    for(int i = 0 ; i < totalNumberOfSignals ; i++) {
	jsDataString.Printf("pwData[%d] = new Array(%d);\npwData[%d] = [", i, signals[i].buffer2PlotTotalNumberOfSamples, i);
	for(int j = 0 ; j < signals[i].buffer2PlotTotalNumberOfSamples-1 ; j++) {
	    if(signals[i].signalTypeMask == TYPE_FLOAT) {
		if(yAxisScaleMask == YSCALE_LINEAR) {
		    jsDataString.Printf("%f,", (float)(*((float *)(signals[i].buffer2plot)+j)));
		} else if(yAxisScaleMask == YSCALE_LOG10) {
#if (defined (_VXWORKS))
		    jsDataString.Printf("%f,", log10((double)(*((float *)(signals[i].buffer2plot)+j))));
#else
		    jsDataString.Printf("%f,", log10f((float)(*((float *)(signals[i].buffer2plot)+j))));
#endif
		}
	    } else if(signals[i].signalTypeMask == TYPE_INT32) {
		if(yAxisScaleMask == YSCALE_LINEAR) {
		    jsDataString.Printf("%d,", (int32)(*((int32 *)(signals[i].buffer2plot)+j)));
		} else if(yAxisScaleMask == YSCALE_LOG10) {
#if (defined (_VXWORKS))
		    jsDataString.Printf("%f,", log10((double)(*((int32 *)(signals[i].buffer2plot)+j))));
#else
		    jsDataString.Printf("%f,", log10f((float)(*((int32 *)(signals[i].buffer2plot)+j))));
#endif
		}
	    } else if(signals[i].signalTypeMask == TYPE_UINT32) {
		if(yAxisScaleMask == YSCALE_LINEAR) {
		    jsDataString.Printf("%d,", (uint32)(*((uint32 *)(signals[i].buffer2plot)+j)));
		} else if(yAxisScaleMask == YSCALE_LOG10) {
#if (defined (_VXWORKS))
		    jsDataString.Printf("%f,", log10((double)(*((uint32 *)(signals[i].buffer2plot)+j))));
#else
		    jsDataString.Printf("%f,", log10f((float)(*((uint32 *)(signals[i].buffer2plot)+j))));
#endif
		}
	    } else {
		return False;
	    }
	}
	if(signals[i].signalTypeMask == TYPE_FLOAT) {
	    if(yAxisScaleMask == YSCALE_LINEAR) {
		jsDataString.Printf("%f];\n", (float)(*((float *)(signals[i].buffer2plot)+signals[i].buffer2PlotTotalNumberOfSamples-1)));
	    } else if(yAxisScaleMask == YSCALE_LOG10) {
#if (defined (_VXWORKS))
		jsDataString.Printf("%f];\n", log10((double)(*((float *)(signals[i].buffer2plot)+signals[i].buffer2PlotTotalNumberOfSamples-1))));
#else
		jsDataString.Printf("%f];\n", log10f((float)(*((float *)(signals[i].buffer2plot)+signals[i].buffer2PlotTotalNumberOfSamples-1))));
#endif
	    }
	} else if(signals[i].signalTypeMask == TYPE_INT32) {
	    if(yAxisScaleMask == YSCALE_LINEAR) {
		jsDataString.Printf("%d];\n", (int32)(*((int32 *)(signals[i].buffer2plot)+signals[i].buffer2PlotTotalNumberOfSamples-1)));
	    } else if(yAxisScaleMask == YSCALE_LOG10) {
#if (defined (_VXWORKS))
		jsDataString.Printf("%f];\n", log10((double)(*((int32 *)(signals[i].buffer2plot)+signals[i].buffer2PlotTotalNumberOfSamples-1))));
#else
		jsDataString.Printf("%f];\n", log10f((float)(*((int32 *)(signals[i].buffer2plot)+signals[i].buffer2PlotTotalNumberOfSamples-1))));
#endif
	    }
	} else if(signals[i].signalTypeMask == TYPE_UINT32) {
	    if(yAxisScaleMask == YSCALE_LINEAR) {
		jsDataString.Printf("%d];\n", (uint32)(*((uint32 *)(signals[i].buffer2plot)+signals[i].buffer2PlotTotalNumberOfSamples-1)));
	    } else if(yAxisScaleMask == YSCALE_LOG10) {
#if (defined (_VXWORKS))
		jsDataString.Printf("%f];\n", log10((double)(*((uint32 *)(signals[i].buffer2plot)+signals[i].buffer2PlotTotalNumberOfSamples-1))));
#else
		jsDataString.Printf("%f];\n", log10f((float)(*((uint32 *)(signals[i].buffer2plot)+signals[i].buffer2PlotTotalNumberOfSamples-1))));
#endif
	    }
	} else {
	    return False;
	}
    }

    if(xAxisInput) {
    	if(xAxisSignalTypeMask == TYPE_FLOAT) {
	    xLimits[0] = xAxisScaleFactor*((float)*((float *)xAxisArray));
	    xLimits[1] = xAxisScaleFactor*((float)*((float *)xAxisArray+signals[0].buffer2PlotTotalNumberOfSamples-1));
    	} else if(xAxisSignalTypeMask == TYPE_INT32) {
	    xLimits[0] = xAxisScaleFactor*((int32)*((int32 *)xAxisArray));
	    xLimits[1] = xAxisScaleFactor*(int32)*((int32 *)xAxisArray+signals[0].buffer2PlotTotalNumberOfSamples-1);
    	} else if(xAxisSignalTypeMask == TYPE_UINT32) {
	    xLimits[0] = xAxisScaleFactor*((uint32)*((uint32 *)xAxisArray));
	    xLimits[1] = xAxisScaleFactor*(uint32)*((uint32 *)xAxisArray+signals[0].buffer2PlotTotalNumberOfSamples-1);
    	} else {
    	    return False;
    	}
    }
    sem.FastUnLock();

    if(xLimits[0] != xLimits[1]) {
	jsDataString.Printf("var xLimits = [%.2f,%.2f];\n", xLimits[0], xLimits[1]);
    } else {
	jsDataString.Printf("var xLimits = [];\n");
    }

    return True;
}
OBJECTLOADREGISTER(PlotWindow, "$Id: PlotWindow.cpp,v 1.13 2011/06/29 14:52:25 dalves Exp $")
