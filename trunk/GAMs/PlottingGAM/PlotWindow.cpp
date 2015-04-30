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
    cdb.ReadInt32(plotPixelWidth, "PlotPixelWidth", 1250);
    cdb.ReadInt32(plotPixelHeight, "PlotPixelHeight", 350);

    //Read the statistic flag: 0 = Statistic disable, 1 = Statistic enable
    cdb.ReadInt32(statistic, "Statistic", 0);
    //Read the plot type: "Line" or "Bar".
    cdb.ReadFString(plotType, "PlotType", "Line");
    //Read the zoom: 0 = max zoom, increase to zoom less.
    cdb.ReadFloat(zoom, "Zoom", 0);

    //Read the x-axis bounds: if the values are equal (or if there aren't in cfg) the program try to get x-values from ddb 
    if (cdb.ReadFloat(xLimits[0], "XAxisStartValue", 0.0)) {
        if (!cdb.ReadFloat(xLimits[1], "XAxisEndValue", 0.0)) {
            AssertErrorCondition(
                    Warning,
                    "PlotWindow::Initialise: %s XaxisStartValue found but XAxisEndValue not found",
                    Name());
            xLimits[0] = 0.0;
            xLimits[1] = 0.0;
        }
    }
    else {
        xLimits[0] = 0.0;
        xLimits[1] = 0.0;
    }

    //If x bounds are equal try to get the x-axis-input-interface.
    if (cdb.ReadFString(xAxisSignalName, "XAxisSignalName", "")
            && (xLimits[0] == xLimits[1])) {
        if (!cdb.ReadFString(xAxisSignalType, "XAxisSignalType", "")) {
            AssertErrorCondition(
                    InitialisationError,
                    "PlotWindow::Initialise: %s XAxisSignalName found but XAxisSignalType not found",
                    Name());
            return False;
        }
        if (!AddInputInterface(xAxisInput, "XAxisInputInterface")) {
            AssertErrorCondition(
                    InitialisationError,
                    "PlotWindow::Initialise: %s failed to add XAxisInputInterface",
                    Name());
            return False;
        }
        if (!(xAxisInput->AddSignal(xAxisSignalName.Buffer(),
                                    xAxisSignalType.Buffer()))) {
            AssertErrorCondition(
                    InitialisationError,
                    "PlotWindow::Initialise: %s failed to add XAxisInputInterface",
                    Name());
            return False;
        }
        cdb.ReadFloat(xAxisScaleFactor, "XAxisScaleFactor", 1.0);
    }

    //Get the yAxis scale: "Linear" or "Log10" 
    FString yAxisScaleString;
    if (cdb.ReadFString(yAxisScaleString, "YAxisScale", "Linear")) {
        if (yAxisScaleString == "Log10") {
            yAxisScaleMask = YSCALE_LOG10;
        }
        else {
            AssertErrorCondition(
                    Warning,
                    "PlotWindow::Initialise: %s YAxisScale unknown, assuming %s",
                    Name(), yAxisScaleString.Buffer());
        }
    }
    else {
        yAxisScaleMask = YSCALE_LINEAR;
    }

    //Get the yAxis scale factor: y values are multiplied for this value in output.
    cdb.ReadFloat(yAxisScaleFactor, "YAxisScaleFactor", 1.0);

    // Read number of buffers to plot
    if (!cdb.ReadInt32(numberOfBuffers2Plot, "PlotLastNBuffers", 50)) {
        AssertErrorCondition(
                Warning,
                "PlotWindow::Initialise: %s cannot read PlotLastNBuffers, assuming %d",
                Name(), numberOfBuffers2Plot);
    }
    else {
        if (numberOfBuffers2Plot <= 0) {
            AssertErrorCondition(
                    InitialisationError,
                    "PlotWindow::Initialise: %s PlotLastNBuffers <= 0", Name());
            return False;
        }
    }

    // Read axis labels
    cdb.ReadFString(xLabel, "XLabel", "");
    cdb.ReadFString(yLabel, "YLabel", "");

    if (!cdb->Move("InputSignals")) {
        AssertErrorCondition(
                InitialisationError,
                "PlotWindow::Initialise: %s cannot move to InputSignals",
                Name());
        return False;
    }

    // Check how many input signals
    if ((totalNumberOfSignals = cdb->NumberOfChildren()) == 0) {
        AssertErrorCondition(
                InitialisationError,
                "PlotWindow::Initialise: %s totalNumberOfSignals = 0", Name());
        return False;
    }

    // Allocate memory for individual signal information
    if ((signals = new SignalInfo[totalNumberOfSignals]) == NULL) {
        AssertErrorCondition(
                InitialisationError,
                "PlotWindow::Initialise: %s cannot allocate memory for signals",
                Name());
        return False;
    }

    // Add one input interface per signal and read SignalName and SignalType
    title.SetSize(0);
    title.Printf("[");
    for (int i = 0; i < totalNumberOfSignals; i++) {
        FString inputInterfaceName;
        inputInterfaceName.SetSize(0);
        inputInterfaceName.Printf("%sInputInterface%d", Name(), i);
        if (!AddInputInterface(signals[i].input, inputInterfaceName.Buffer())) {
            AssertErrorCondition(
                    InitialisationError,
                    "PlotWindow::Initialise: %s failed to add input interface",
                    Name());
            return False;
        }

        cdb->MoveToChildren(i);
        if (!cdb.ReadFString(signals[i].signalName, "SignalName")) {
            AssertErrorCondition(
                    InitialisationError,
                    "PlotWindow::Initialise: %s failed to read SignalName for signal %d",
                    Name(), i);
            return False;
        }
        if (!cdb.ReadFString(signals[i].signalType, "SignalType")) {
            AssertErrorCondition(
                    InitialisationError,
                    "PlotWindow::Initialise: %s failed to read SignalType for signal %d",
                    Name(), i);
            return False;
        }
        signals[i].input->AddSignal(signals[i].signalName.Buffer(),
                                    signals[i].signalType.Buffer());
        signals[i].numberOfSamplesPerCycle = signals[i].input->BufferWordSize(); // This works only because we have one input interface per signal
		//Double is two words...
		if (signals[i].signalType == "double") {
			signals[i].numberOfSamplesPerCycle = signals[i].numberOfSamplesPerCycle / 2;		
		}


        if (i < totalNumberOfSignals - 1) {
            title.Printf("'%s',", signals[i].signalName.Buffer());
        }
        else {
            title.Printf("'%s']", signals[i].signalName.Buffer());
        }

        signals[i].numberOfBuffers = numberOfBuffers2Plot;

        signals[i].buffer2PlotTotalNumberOfSamples =
                signals[i].numberOfSamplesPerCycle * signals[i].numberOfBuffers;

        // Allocate memory for buffer to be plotted
        if (signals[i].signalType == "float") {
            signals[i].numberOfBytesPerBuffer =
                    signals[i].numberOfSamplesPerCycle * sizeof(float);
            signals[i].signalTypeMask = TYPE_FLOAT;
        }
        else if (signals[i].signalType == "double") {
            signals[i].numberOfBytesPerBuffer =
                    signals[i].numberOfSamplesPerCycle * sizeof(double);
            signals[i].signalTypeMask = TYPE_DOUBLE;
        }
        else if (signals[i].signalType == "int32") {
            signals[i].numberOfBytesPerBuffer =
                    signals[i].numberOfSamplesPerCycle * sizeof(int32);
            signals[i].signalTypeMask = TYPE_INT32;
        }
        else if (signals[i].signalType == "uint32") {
            signals[i].numberOfBytesPerBuffer =
                    signals[i].numberOfSamplesPerCycle * sizeof(uint32);
            signals[i].signalTypeMask = TYPE_UINT32;
        }
        else {
            AssertErrorCondition(
                    InitialisationError,
                    "PlotWindow::Initialise: %s unsupported type for %s",
                    Name(), signals[i].signalName.Buffer());
            return False;
        }
        signals[i].buffer2PlotTotalNumberOfBytes =
                signals[i].numberOfBytesPerBuffer * signals[i].numberOfBuffers;
        signals[i].buffer2plot = (char *) malloc(
                signals[i].buffer2PlotTotalNumberOfBytes);

        cdb->MoveToFather();
    }

    if (xAxisInput) {
        if (xAxisSignalType == "float") {
            xAxisSignalTypeMask = TYPE_FLOAT;
        }
        else if (xAxisSignalType == "int32") {
            xAxisSignalTypeMask = TYPE_INT32;
        }
        else if (xAxisSignalType == "uint32") {
            xAxisSignalTypeMask = TYPE_UINT32;
        }
        else {
            AssertErrorCondition(
                    InitialisationError,
                    "PlotWindow::Initialise: %s unsupported type for %s",
                    Name(), xAxisSignalName.Buffer());
            return False;
        }
        if ((xAxisArray = (char *) malloc(
                signals[0].buffer2PlotTotalNumberOfBytes)) == NULL) {
            AssertErrorCondition(
                    InitialisationError,
                    "PlotWindow::Initialise: %s Unable to allocate memory for xAxisArray",
                    Name());
            return False;
        }
    }

    //Reset to zero all signals at the beginning.
    if (statistic == 1) {
        for (int32 i = 0; i < totalNumberOfSignals; i++) {
            for (int32 n = 0; n < signals[i].numberOfBuffers; n++) {

                if (signals[i].signalTypeMask == TYPE_INT32) {
                    int32* incrementDataPtr = (int32*) (signals[i].buffer2plot
                            + n * signals[i].numberOfBytesPerBuffer);
                    (*incrementDataPtr)=0;
                }
                else if (signals[i].signalTypeMask == TYPE_DOUBLE) {
                    double* incrementDataPtr = (double *) (signals[i].buffer2plot
                            + n * signals[i].numberOfBytesPerBuffer);
                    (*incrementDataPtr)=0;
                }
                else if (signals[i].signalTypeMask == TYPE_FLOAT) {
                    float* incrementDataPtr = (float*) (signals[i].buffer2plot
                            + n * signals[i].numberOfBytesPerBuffer);
                    (*incrementDataPtr)=0;
                }
                else if (signals[i].signalTypeMask == TYPE_UINT32) {
                    uint32* incrementDataPtr = (uint32*) (signals[i].buffer2plot
                            + n * signals[i].numberOfBytesPerBuffer);
                    (*incrementDataPtr)=0;
                }

            }
        }
    }

    return True;
}

/// Called in every control loop
bool PlotWindow::Execute(GAM_FunctionNumbers execFlag) {

    //If statistic reset all signals when the state machine is prepulse.
    if (statistic == 1 && execFlag == GAMPrepulse) {
        for (int32 i = 0; i < totalNumberOfSignals; i++) {
            for (int32 n = 0; n < signals[i].numberOfBuffers; n++) {

                if (signals[i].signalTypeMask == TYPE_INT32) {
                    int32* incrementDataPtr = (int32*) (signals[i].buffer2plot
                            + n * signals[i].numberOfBytesPerBuffer);
                    (*incrementDataPtr)=0;
                }
                else if (signals[i].signalTypeMask == TYPE_FLOAT) {
                    float* incrementDataPtr = (float*) (signals[i].buffer2plot
                            + n * signals[i].numberOfBytesPerBuffer);
                    (*incrementDataPtr)=0;
                }
                else if (signals[i].signalTypeMask == TYPE_DOUBLE) {
                    double *incrementDataPtr = (double *) (signals[i].buffer2plot
                            + n * signals[i].numberOfBytesPerBuffer);
                    (*incrementDataPtr)=0;
                }
                else if (signals[i].signalTypeMask == TYPE_UINT32) {
                    uint32* incrementDataPtr = (uint32*) (signals[i].buffer2plot
                            + n * signals[i].numberOfBytesPerBuffer);
                    (*incrementDataPtr)=0;
                }

            }
        }
    }

    if (sem.FastTryLock()) {
        //If time mode, signals are shifted when arrive a new input.
        if (statistic == 0) {
            for (int i = 0; i < totalNumberOfSignals; i++) {
                signals[i].input->Read();
                memmove(signals[i].buffer2plot,
                        signals[i].buffer2plot
                                + signals[i].numberOfBytesPerBuffer,
                        signals[i].numberOfBytesPerBuffer
                                * (signals[i].numberOfBuffers - 1));
                memcpy(signals[i].buffer2plot
                        + signals[i].buffer2PlotTotalNumberOfBytes
                               - signals[i].numberOfBytesPerBuffer,
                       signals[i].input->Buffer(),
                       signals[i].numberOfBytesPerBuffer);
            }
            //The same for the x-axis.
            if (xAxisInput) {
                xAxisInput->Read();
                memmove(xAxisArray,
                        xAxisArray + signals[0].numberOfBytesPerBuffer,
                        signals[0].numberOfBytesPerBuffer
                                * (signals[0].numberOfBuffers - 1));
                memcpy(xAxisArray + signals[0].buffer2PlotTotalNumberOfBytes
                               - signals[0].numberOfBytesPerBuffer,
                       xAxisInput->Buffer(), signals[0].numberOfBytesPerBuffer);
            }
        }
        if (statistic == 1) {
            //If statistic we must calculate in which interval falls the new input data.
            for (int32 i = 0; i < totalNumberOfSignals; i++) {

                signals[i].input->Read();

                //Calculate range and granularity.
                float range = xLimits[1] - xLimits[0];
                float granularity = range / numberOfBuffers2Plot;

                int32 n = 0;

                //Check the type of data and calculate the index, namely the number of the interval where the data falls.
                if (signals[i].signalTypeMask == TYPE_INT32)
                    n = (int32) ((*(((int32*) (signals[i].input->Buffer())))
                            - xLimits[0]) / granularity);
                else if (signals[i].signalTypeMask == TYPE_UINT32)
                    n = (int32) ((*(((uint32*) (signals[i].input->Buffer())))
                            - xLimits[0]) / granularity);
                else if (signals[i].signalTypeMask == TYPE_FLOAT)
                    n = (int32) ((*(((float*) (signals[i].input->Buffer())))
                            - xLimits[0]) / granularity);
                else if (signals[i].signalTypeMask == TYPE_DOUBLE)
                    n = (int32) ((*(((double*) (signals[i].input->Buffer())))
                            - xLimits[0]) / granularity);


                //If data falls out of range, intervals are the bounds.
                if (n < 0) {
                    n = 0;
                }
                if (n > (signals[i].numberOfBuffers - 1)) {
                    n = signals[i].numberOfBuffers - 1;
                }

                //Increment the related bar.
                if (signals[i].signalTypeMask == TYPE_INT32) {
                    int32* incrementDataPtr = (int32*) (signals[i].buffer2plot
                            + n * signals[i].numberOfBytesPerBuffer);
                    (*incrementDataPtr)++;
                }
                else if (signals[i].signalTypeMask == TYPE_FLOAT) {
                    float* incrementDataPtr = (float*) (signals[i].buffer2plot
                            + n * signals[i].numberOfBytesPerBuffer);
                    (*incrementDataPtr)++;
                }
                else if (signals[i].signalTypeMask == TYPE_DOUBLE) {
                    double *incrementDataPtr = (double *) (signals[i].buffer2plot
                            + n * signals[i].numberOfBytesPerBuffer);
                    (*incrementDataPtr)++;
                }
                else if (signals[i].signalTypeMask == TYPE_UINT32) {
                    uint32* incrementDataPtr = (uint32*) (signals[i].buffer2plot
                            + n * signals[i].numberOfBytesPerBuffer);
                    (*incrementDataPtr)++;
                }

            }
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

    for (int i = 0; i < totalNumberOfSignals; i++) {

        uint32 signalTypeState = signals[i].signalTypeMask;

        //Print the header.
        jsDataString.Printf("pwData[%d] = new Array(%d);\npwData[%d] = [", i,
                            signals[i].buffer2PlotTotalNumberOfSamples, i);

        int32 nOfSamples = signals[i].buffer2PlotTotalNumberOfSamples;
        for (int j = 0; j < nOfSamples; j++) {

            //Case float.
            if (signalTypeState == TYPE_FLOAT) {
                if (yAxisScaleMask == YSCALE_LINEAR) {
                    jsDataString.Printf(
                            "%f",
                            (float) (yAxisScaleFactor
                                * *((float *) (signals[i].buffer2plot) + j)));
                }
                else if (yAxisScaleMask == YSCALE_LOG10) {

#if (defined (_VXWORKS))
                    jsDataString.Printf("%f", log10((double)(yAxisScaleFactor * *((float *)(signals[i].buffer2plot)+j))));
#else
                    float value = (float) (yAxisScaleFactor
                            * *((float *) (signals[i].buffer2plot) + j));

                    if(value < -1) {
                        value=-log10f(-value);
                        jsDataString.Printf("%f", value);
                    }
                    else {
                        if (value > 1) {
                            value=log10f(value);
                            jsDataString.Printf("%f", value);
                        }	    
                        //In case of log10 y-axis, if y=0 the y-value remains to zero (also if it should be -inf)
                        else {
                            value = 0.0;
                            jsDataString.Printf("%f", value);
                        }
                    }
#endif
                }
            }
            else if (signalTypeState == TYPE_DOUBLE) {
                if (yAxisScaleMask == YSCALE_LINEAR) {
                    jsDataString.Printf(
                            "%lf",
                            (double) (yAxisScaleFactor
                                * *((double *) (signals[i].buffer2plot) + j)));
                }
                else if (yAxisScaleMask == YSCALE_LOG10) {

#if (defined (_VXWORKS))
                    jsDataString.Printf("%f", log10((double)(yAxisScaleFactor * *((double *)(signals[i].buffer2plot)+j))));
#else
                    double value = (double) (yAxisScaleFactor
                            * *((double *) (signals[i].buffer2plot) + j));

                    if(value < -1) {
                        value=-log10f(-value);
                        jsDataString.Printf("%f", value);
                    }
                    else {
                        if (value > 1) {
                            value=log10f(value);
                            jsDataString.Printf("%f", value);
                        }	    
                        //In case of log10 y-axis, if y=0 the y-value remains to zero (also if it should be -inf)
                        else {
                            value = 0.0;
                            jsDataString.Printf("%f", value);
                        }
                    }
#endif
                }
            }
            //Case int32
            else if (signalTypeState == TYPE_INT32) {
                if (yAxisScaleMask == YSCALE_LINEAR) {
                    jsDataString.Printf(
                            "%d",
                            (int32) (yAxisScaleFactor
                                    * *((int32 *) (signals[i].buffer2plot) + j)));
                }
                else if (yAxisScaleMask == YSCALE_LOG10) {
#if (defined (_VXWORKS))
                    jsDataString.Printf("%f", log10(yAxisScaleFactor * (double)(*((int32 *)(signals[i].buffer2plot)+j))));
#else

                    int32 value = (int32) (yAxisScaleFactor
                            * *((int32 *) (signals[i].buffer2plot) + j));
			
		    float retValue;

                    if(value < -1) {
                        retValue=-log10f(-value);
                        jsDataString.Printf("%f", retValue);
		    }
		    else {
                        if (value > 1) {
			    retValue=log10f(value);
                            jsDataString.Printf("%f", retValue);
                        }	    
                    //In case of log10 y-axis, if y=0 the y-value remains to zero (also if it should be -inf)
                        else {
                            retValue = 0.0;
                            jsDataString.Printf("%f", retValue);
                        }
                    }

#endif
                }
            }
            //Case uint32
            else if (signalTypeState == TYPE_UINT32) {
                if (yAxisScaleMask == YSCALE_LINEAR) {
                    jsDataString.Printf(
                            "%d",
                            (uint32) (yAxisScaleFactor
                                    * *((uint32 *) (signals[i].buffer2plot) + j)));
                }
                else if (yAxisScaleMask == YSCALE_LOG10) {
#if (defined (_VXWORKS))
                    jsDataString.Printf("%f", log10(yAxisScaleFactor * (double)(*((uint32 *)(signals[i].buffer2plot)+j))));
#else
                    uint32 value = (uint32) (yAxisScaleFactor
                            * *((uint32 *) (signals[i].buffer2plot) + j));
                    
                    float retValue;
                    if(value < -1) {
                        retValue=-log10f(-value);
                        jsDataString.Printf("%f", retValue);
		    }
		    else {
                        if (value > 1) {
			    retValue=log10f(value);
                            jsDataString.Printf("%f", retValue);
                        }	    
                    //In case of log10 y-axis, if y=0 the y-value remains to zero (also if it should be -inf)
                        else {
                            retValue = 0.0;
                            jsDataString.Printf("%f", retValue);
                        }
                    }
#endif
                }
            }
            else {
                return False;
            }

            if (j < (nOfSamples - 1))
                jsDataString.Printf(",");
            else
                jsDataString.Printf("];\n");

        }

    }

    //If the x-axis is get by ddb, print limits as the bound of current x-axis.
    if (xAxisInput) {
        if (xAxisSignalTypeMask == TYPE_FLOAT) {
            xLimits[0] = xAxisScaleFactor * ((float) *((float *) xAxisArray));
            xLimits[1] = xAxisScaleFactor
                    * ((float) *((float *) xAxisArray
                            + signals[0].buffer2PlotTotalNumberOfSamples - 1));
        }
        else if (xAxisSignalTypeMask == TYPE_INT32) {
            xLimits[0] = xAxisScaleFactor * ((int32) *((int32 *) xAxisArray));
            xLimits[1] = xAxisScaleFactor
                    * (int32) *((int32 *) xAxisArray
                            + signals[0].buffer2PlotTotalNumberOfSamples - 1);
        }
        else if (xAxisSignalTypeMask == TYPE_UINT32) {
            xLimits[0] = xAxisScaleFactor * ((uint32) *((uint32 *) xAxisArray));
            xLimits[1] = xAxisScaleFactor
                    * (uint32) *((uint32 *) xAxisArray
                            + signals[0].buffer2PlotTotalNumberOfSamples - 1);
        }
        else {
            return False;
        }
    }
    sem.FastUnLock();

    //Print limits.
    if (xLimits[0] != xLimits[1]) {
        jsDataString.Printf("var xLimits = [%.6f,%.6f];\n", xLimits[0],
                            xLimits[1]);
    }
    else {
        jsDataString.Printf("var xLimits = [];\n");
    }

    return True;
}
OBJECTLOADREGISTER(PlotWindow,
                   "$Id$")
