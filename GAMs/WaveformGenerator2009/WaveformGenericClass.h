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

#if !defined (WAVEFORMGENERICCLASS_H)
#define WAVEFORMGENERICCLASS_H

#include "System.h"
#include "WaveformInterface.h"
#include "GCReferenceContainer.h"
#include "TriggerObj.h"
#include "HttpInterface.h"
#include "HttpJScopeInterface.h"
#include "GAM.h"

//#define PI2 6.28318530717959

const float PI2 = 8.0*atan(1);

class WaveformGenericClass: public WaveformInterface, public GCReferenceContainer, public HttpInterface{

protected:

/******************/
/*   Time window  */
/******************/

    /** Start waveform time */
    float                               tStart;

    /** End waveform time */
    float                               tEnd;

    /** Start time in microseconds */
    int32                               tStartUsec;

    /** End time in microseconds */
    int32                               tEndUsec;

    /** Local start time different from tStartUsec in case of triggered waveform */
    int32                               tStartUsecLocal;

/*************/
/*   Offset  */
/*************/

    /** Fixed offset */
    float                               offsetValue;

    /** Offset waveform */
    GCRTemplate<WaveformInterface>      offsetWaveform;

    /** Fixed/Variable offset flag */
    bool                                variableOffset;

/******************/
/*  AmpliduteGain */
/******************/

    /** Fixed Amplitude gain */
    float                               gain;

    /** Amplitude gain waveform */
    GCRTemplate<WaveformInterface>      gainWaveform;

    /** Fixed/Variable amplitude gain flag */
    bool                                variableGain;


/*******************************************/

    /** Signal output type */
    FString                             signalType;

    /** Integer output flag */
    bool                                isIntSignal;

/******* Graphic ***************************/

    /** */
    FString                             xGraphData;

    /** */
    FString                             yGraphData;

    /** */
    HttpJScopeInterface                 httpJScope;

/*******************************************/

    /** */
    bool                                triggerON;

    /** */
    GCRTemplate<TriggerObj>             triggerObj;

/*******************************************/

    /** */
    bool                                simMode;

/*******************************************/

    /** */
    GAM_FunctionNumbers                 currentState;

public:

    /** */
    WaveformGenericClass(){
        tStart              = 0.0;
        tStartUsec          = 0;
        tEnd                = 100.0;
        tEndUsec            = 100000000;
        offsetValue         = 0.0;
        variableOffset      = False;
        gain                = 1.0;
        variableGain        = False;
        signalType          = "float";
        isIntSignal         = False;
        triggerON           = False;
        tStartUsecLocal     = 0;
        xGraphData          = "";
        yGraphData          = "";
        simMode             = False;
        currentState        = GAMOffline;
    }

    /** Copy constructor */
    WaveformGenericClass(const WaveformGenericClass &wave){
        this->tStart              = wave.tStart;
        this->tStartUsec          = wave.tStartUsec;
        this->tEnd                = wave.tEnd;
        this->tEndUsec            = wave.tEndUsec;
        this->offsetValue         = wave.offsetValue;
        this->variableOffset      = wave.variableOffset;
        this->gain                = wave.gain;
        this->variableGain        = wave.variableGain;
        this->signalType          = wave.signalType;
        this->isIntSignal         = wave.isIntSignal;
        this->triggerON           = wave.triggerON;
        this->tStartUsecLocal     = wave.tStartUsecLocal;
        this->xGraphData          = wave.xGraphData;
        this->yGraphData          = wave.yGraphData;
        this->simMode             = wave.simMode;
        this->currentState        = wave.currentState;

        if( this->variableOffset ) this->offsetWaveform = wave.offsetWaveform;
        if( this->variableGain   ) this->gainWaveform   = wave.gainWaveform;
        if( this->triggerON      ) this->triggerObj     = wave.triggerObj;
    }


    /** */
    ~WaveformGenericClass(){};

    /** */
    virtual bool ObjectLoadSetup(ConfigurationDataBase &cdbData, StreamInterface *err);

    /** */
    bool Execute(int32 actTimeUsec);

    /** */
    void SetSimulationMode(bool mode){ simMode = mode; }

    /** */
    bool IsInt(){ return isIntSignal; }

    /** */
    char* GetSignalType(){ return signalType.BufferReference(); }

    /** */
    void SetTStart(int32 tStart){
        this->tStartUsec = tStart;
        this->tStart     = (float)(tStart)/1000000;
    }

    /** */
    void SetTEnd(int32 tEnd){
        this->tEndUsec = tEndUsec;
        this->tEnd     = (float)(tEndUsec)/1000000;
    }

    /** */
    virtual void SetState(GAM_FunctionNumbers currentState) {
        this->currentState = currentState;
    }

    /** */
    virtual bool ProcessHttpMessage(HttpStream &hStream);

    /** */
    char *GetXData(){ return xGraphData.BufferReference(); }

    /** */
    char *GetYData(){ return yGraphData.BufferReference(); }


    /** */
    void GenerateDataPlot(float startTime, float endTime){
        SetSimulationMode(True);

        int i;
        float delta = (endTime-startTime)/999.0;

        xGraphData.Printf("[ ");
        yGraphData.Printf("[ ");

        for( i = 0; i < 999; i++){
            float timeS = startTime+i*delta;
            xGraphData.Printf(" %f",timeS);
            yGraphData.Printf(" %f",GetValue((int32)(timeS*1000000)));
        }
        xGraphData.Printf(" ]");
        yGraphData.Printf(" ]");

        Reset();

        SetSimulationMode(False);
    }

protected:

    /** Default graph message */
    bool ProcessGraphHttpMessage(HttpStream &hStream, FString waveName = "Default", FString color = "Red"){
        httpJScope.SetTitle(Name());
        httpJScope.ProcessGraphHttpMessageCreate(hStream);
        httpJScope.ProcessGraphHttpMessageAddSignal(hStream,xGraphData,yGraphData,waveName,color);
        httpJScope.ProcessGraphHttpMessageShow(hStream);
        return True;
    }


    /** */
    bool IsWaveformType(FString entry, CDBExtended &cdb, GCRTemplate<WaveformInterface> &waveform, float &value, bool &isWaveformType);

    /** */
    bool IsNumber(FString entry, float &value){
        char * pEnd;
        value = (float) strtod((char*)entry.Buffer(),&pEnd);
        if( *pEnd == '\0' ) return True;
        else return False;
    }

};

#endif
