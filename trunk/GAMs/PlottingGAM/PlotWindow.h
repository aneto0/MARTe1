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

const uint32 TYPE_NONE = 0x0;
const uint32 TYPE_FLOAT = 0x1;
const uint32 TYPE_INT32 = 0x2;
const uint32 TYPE_UINT32 = 0x4;
const uint32 TYPE_DOUBLE = 0x8;

const uint32 YSCALE_LINEAR = 0x0;
const uint32 YSCALE_LOG10 = 0x1;

class SignalInfo {
public:
    /** Input from database. **/
    DDBInputInterface *input;
 
    /** Name of the signal. **/
    FString signalName;

    /** Type of the signal (string from cfg). **/
    FString signalType;

    /** Type of the signal (uint32 flag). **/ 
    uint32 signalTypeMask;

    /** Number of samples for each cycle. **/
    uint32 numberOfSamplesPerCycle;

    /** Number of buffers to plot for each cycle (from cfg). **/
    uint32 numberOfBuffers;
 
    /** Number of bytes for each sample (depends from signal type: NSamplesPerCycle*sizeof(type)). **/  
    uint32 numberOfBytesPerBuffer;

    /** Numbers of samples of the plot window (NSamplesPerCycle*NOfBuffers). **/
    uint32 buffer2PlotTotalNumberOfSamples;

    /** Numbers of bytes of the plot window (NBytesPerBuffer*NOfBuffers). **/
    uint32 buffer2PlotTotalNumberOfBytes;

    /** Pointer at the beginning of the plot window. **/
    char *buffer2plot;
public:
    SignalInfo() {
        input = NULL;
        buffer2plot = NULL;
        signalTypeMask = TYPE_NONE;
    }
    ~SignalInfo() {
        if (buffer2plot != NULL) {
            free((void*&) buffer2plot);
            buffer2plot = NULL;
        }
    }
};

OBJECT_DLL(PlotWindow)
class PlotWindow: public GAM {
OBJECT_DLL_STUFF(PlotWindow)

private:

    /** x-axis input from ddb. **/
    DDBInputInterface *xAxisInput;

    /** Pointer at the beginning of the x-axis input for the plot window. **/
    char *xAxisArray;

    /** x-axis signal name. **/
    FString xAxisSignalName;

    /** x-axis signal type (string). **/
    FString xAxisSignalType;

    /** x-axis signal type (uint32 flag). **/
    uint32 xAxisSignalTypeMask;

    /** x-axis scale factor: the x-axis output is multiplied for this value. **/
    float xAxisScaleFactor;

    /** y-axis scale factor: the y-axis output is multiplied for this value. **/
    float yAxisScaleFactor;

    /** Could be LINEAR or LOG10, depends by cfg. **/
    uint32 yAxisScaleMask;

    /** Signals to be plotted. **/
    uint32 totalNumberOfSignals;

    /** Pointer to a signals array. **/
    SignalInfo *signals;

    /** Number of buffers to plot for each signal each cycle time. **/
    int32 numberOfBuffers2Plot;

private:

    FastPollingMutexSem sem;

public:
    /** Define dimensions of the window. **/
    int32 plotPixelWidth;

    int32 plotPixelHeight;

    /** Define x-axis bounds. **/
    float xLimits[2];

    /** Title and labels of the window. **/
    FString title;

    FString xLabel;

    FString yLabel;


    /** Statistic flag (0 = time, 1 = statistic). **/
    int32 statistic;

    /** Plot type ("Line" or "Bar"). **/
    FString plotType;

    float zoom;

public:

    /// Constructor
    PlotWindow() {
        sem.Create();

        xAxisInput = NULL;
        xAxisArray = NULL;
        xAxisSignalTypeMask = TYPE_NONE;
        xAxisScaleFactor = 1.0;
        yAxisScaleFactor = 1.0;
        yAxisScaleMask = YSCALE_LINEAR;
        totalNumberOfSignals = 0;
        numberOfBuffers2Plot = 1;
        xLimits[0] = 0.0;
        xLimits[1] = 0.0;
        statistic = 0;
	plotType = "Line";
	zoom = 0.0;
    }
    ;

    /// Destructor
    ~PlotWindow() {
        if (signals != NULL) {
            delete[] signals;
            signals = NULL;
        }
        if (xAxisArray != NULL) {
            free((void*&) xAxisArray);
            xAxisArray = NULL;
        }
    }
    ;

    /// GAM configuration
    bool Initialise(ConfigurationDataBase &cdb);

    /// GAM execution method
    bool Execute(GAM_FunctionNumbers execFlag);

    /// Print data on an FString to plot on a browser.
    bool AppendJsDataString(FString &jsDataString);
};
#endif
