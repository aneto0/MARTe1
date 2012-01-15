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

#include "WaveformClassPoints.h"
#include "LoadCDBObjectClass.h"

///
WaveformClassPoints::WaveformClassPoints(){
    timeBase       = NULL;
    values         = NULL;
    numberOfValues = 0;
    closestEnable  = False;
}

///
WaveformClassPoints::~WaveformClassPoints(){
    if( timeBase != NULL ) free((void*&)timeBase);
    if( values   != NULL ) free((void*&)values);
    timeBase = NULL;
    values   = NULL;
}

///
float WaveformClassPoints::GetValue(int32 usecTime){

    ////////////////////////////
    /// Linear interpolation ///
    ////////////////////////////

    float output = 0.0;

    if( WaveformPeriodicClass::Execute(usecTime) ){

        int32 timeRef;

        if( actFrequency == 0 ){
            // Not periodic or error in the input
            //timeRef = usecTime;
            timeRef = usecTime-tStartUsecLocal; // Added after removing the tStart in the initialisation
        }else{
            // Periodic
            // Normalisation between 0 and 1
            timeRef = (int32)(localPhase*actFrequency*1000000);
        }

        int k = 0;
        while( timeBase[k] <= timeRef && k < numberOfValues ) k++;

        if( k == 0 ) return 0.0;

        if( k == numberOfValues ) return 0.0;

        if( closestEnable ){
            if( fabs(timeRef-timeBase[k]) > fabs(timeRef-timeBase[k-1]) ) output = values[k-1];
            else                                                          output = values[k];
        }else output = values[k] + (values[k-1]-values[k])*((float)(timeRef-timeBase[k])) / ((float)(timeBase[k-1]-timeBase[k]));

        output = output * gain + offsetValue;
    }

    return output;
}

///
bool WaveformClassPoints::ObjectLoadSetup(ConfigurationDataBase &cdbData, StreamInterface *err){

    if( !WaveformPeriodicClass::ObjectLoadSetup(cdbData,NULL) ){
        CStaticAssertErrorCondition(InitialisationError,"WaveformClassPoints::Init error ObjectLoadSetup");
        return False;
    }

    CDBExtended cdb(cdbData);

    FString error;

    // Load time base points in seconds
    float *timeBaseSec = NULL;
    if( !LoadVectorObject(cdb,"TimeVector",(void*&)timeBaseSec,numberOfValues,CDBTYPE_float,error) ){
        CStaticAssertErrorCondition(InitialisationError,"WaveformClassPoints::Init error loading %s.TimeVector",Name());
        return False;
    }

    // Order and negative values check
    if( !CheckTimeBase(timeBaseSec,numberOfValues) ){
        CStaticAssertErrorCondition(InitialisationError,"WaveformClassPoints::Init %s.TimeVector not ordered",Name());
        if( timeBaseSec != NULL ) free((void*&)timeBaseSec);
        return False;
    }

    // Load values
    int dim;
    if( !LoadVectorObject(cdb,"ValueVector",(void*&)values,dim,CDBTYPE_float,error,numberOfValues) ){
        CStaticAssertErrorCondition(InitialisationError,"WaveformClassPoints::Init error loading %s.ValueVector",Name());
        if( timeBaseSec != NULL ) free((void*&)timeBaseSec);
        return False;
    }


    // TimeRelative = On (Frequency has to be set to 0 otherwise error)
    FString timeRelative;
    cdb.ReadFString(timeRelative,"TimeRelative","Off");

    if( strcmp(timeRelative.Buffer(),"On") == 0 && frequency != 0 ){
        CStaticAssertErrorCondition(InitialisationError,"WaveformClassPoints::Init Frequency != 0 and TimeRelative = On");
        if( timeBaseSec != NULL ) free((void*&)timeBaseSec);
        return False;
    }

    if( strcmp(timeRelative.Buffer(),"On") == 0 ){
        if( timeBaseSec[dim-1] > 1.0 ){
            CStaticAssertErrorCondition(InitialisationError,"WaveformClassPoints::Init error on TimeVector [0:1] (1)");
            if( timeBaseSec != NULL ) free((void*&)timeBaseSec);
            return False;
        }
        float deltaTime = tEnd-tStart;
        int j;
        for( j = 0; j < numberOfValues; j++ ){
            timeBaseSec[j] *= deltaTime;
        }
    }else
    if( frequency != 0 ){
        if( timeBaseSec[dim-1] > 1.0 ){
            CStaticAssertErrorCondition(InitialisationError,"WaveformClassPoints::Init error on TimeVector [0:1] (2)");
            if( timeBaseSec != NULL ) free((void*&)timeBaseSec);
            return False;
        }
    }else{
        int j;
        for( j = 0; j < numberOfValues; j++ ){
            timeBaseSec[j] += tStart;
        }
        if( tEnd > tStart+timeBaseSec[dim-1] ) tEnd = tStart+timeBaseSec[dim-1];
    }

    // Covert from second to microseconds
    timeBase = (int32*)malloc(sizeof(int32)*numberOfValues);
    if( timeBase == NULL ){
        CStaticAssertErrorCondition(InitialisationError,"WaveformClassPoints::Init error allocating memory");
        if( timeBaseSec != NULL ) free((void*&)timeBaseSec);
        return False;
    }

    int j;
    for( j = 0; j < numberOfValues; j++ ){
        //timeBase[j] = (int32)(timeBaseSec[j]*1000000);
        timeBase[j] = (int32)((timeBaseSec[j]-tStart)*1000000);  // removed normalisation to tStart
    }

    if( timeBaseSec != NULL ) free((void*&)timeBaseSec);

    FString closest;
    cdb.ReadFString(closest,"Closest","Off");
    if( strcmp(closest.Buffer(),"On") == 0 ) closestEnable = True;
    else                                     closestEnable = False;

    if( phase < 0 || phase > 100 ){
        CStaticAssertErrorCondition(InitialisationError,"WaveformClassPoints::Init error phase is not in [-100,100]");
        return False;
    }
    phase /= 100.0;

    GenerateDataPlot(tStart,tEnd);

    return True;
}


///
bool WaveformClassPoints::CheckTimeBase(float *tBase, int nValues){
    int i;
    for( i = 0; i < nValues-1; i++ )
        if( tBase[i] < 0.0 || tBase[i] >= tBase[i+1] ){
            CStaticAssertErrorCondition(InitialisationError,"WaveformObject::CheckTimeBase timeBase incorrect");
            return False;
        }

    return True;
}

///
bool WaveformClassPoints::ProcessHttpMessage(HttpStream &hStream) {

    hStream.SSPrintf("OutputHttpOtions.Content-Type","text/html");
    hStream.keepAlive = False;
    hStream.WriteReplyHeader(False);
    hStream.Printf("<html><head><title>WaveformClassPoints %s</title></head>\n", Name());

    hStream.Printf("<h1>WaveformClassPoints %s</h1>", Name());

    WaveformPeriodicClass::ProcessHttpMessage(hStream);

    hStream.Printf("<table border=\"1\">");
    hStream.Printf("<tr><th>UsecTime</th><th>Value</th></tr>");
    int i;
    for( i = 0; i < numberOfValues; i++ )
        hStream.Printf("<tr><td>%f</td><td>%f</td></tr>",(float)timeBase[i]/1000000,values[i]);
    hStream.Printf("</table> ");

    ProcessGraphHttpMessage(hStream);

    hStream.Printf("</body></html>");

    hStream.WriteReplyHeader(True);
    return True;
}



OBJECTLOADREGISTER(WaveformClassPoints,"$Id: WaveformClassPoints.cpp,v 1.4 2009/10/28 16:10:56 lzabeo Exp $")






