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

#include "FlotPlotInterface.h"
#include "GAM.h"
#include "GlobalObjectDataBase.h"
#include "GCNString.h"
#include "BasicTypes.h"
#include "SignalInterface.h"
#include "HtmlStream.h"
#include "Signal.h"

/** @file
    A generic signal
 */
void FlotPlotInterface::PrintNextValue(char *&buffer, BasicTypeDescriptor &btd, Streamable &hStream, float multiplier, int32 downsample){
    if(btd == BTDFloat){
        hStream.Printf("%f", *((float *)buffer) * multiplier);
    }
    else if(btd == BTDInt32){
        hStream.Printf("%f", (float)(*((int32 *)buffer) * multiplier));
    }
    else if(btd == BTDUint32){
        hStream.Printf("%f", (float)(*((uint32 *)buffer) * multiplier));
    }
    else if(btd == BTDInt64){
        hStream.Printf("%lf", (double)(*((int64 *)buffer) * multiplier));
    }
    else if(btd == BTDUint64){
        hStream.Printf("%lf", (double)(*((uint64 *)buffer) * multiplier));
    }
    else if(btd == BTDDouble){
        hStream.Printf("%lf", *((double *)buffer) * multiplier);
    }
    buffer += btd.ByteSize() * downsample;
}

void FlotPlotInterface::PopulateSignalData(GCRTemplate<SignalInterface> xxSignal, GCRTemplate<SignalInterface> signal, Streamable &hStream, float xxMultiplier, float yyMultiplier, int32 downsample){
    
    //Try to enforce a valid downsample...
    while(downsample > (int32)signal->NumberOfSamples()){
        downsample /= 10;
    }


    hStream.Printf("\"data\":[");
    int32 i=0;
    char                *buffer     = (char *)signal->Buffer();
    BasicTypeDescriptor  signalType = signal->Type();
    if(buffer == NULL){
        return;
    }

    if(!xxSignal.IsValid()){
        //This cast is really needed!
        for(i=0; i<(int32)(signal->NumberOfSamples()-downsample); i+=downsample){
            hStream.Printf("[%d,",(int32)(i*xxMultiplier));
            PrintNextValue(buffer, signalType, hStream, yyMultiplier, downsample);
            hStream.Printf("],");
        }
        hStream.Printf("[%d,",(int32)(i*xxMultiplier));
        PrintNextValue(buffer, signalType, hStream, yyMultiplier, downsample);
        hStream.Printf("]]\n");
    }
    else{
        char                *xxBuffer = (char *)xxSignal->Buffer();
        BasicTypeDescriptor  xxType   = xxSignal->Type();
        //This cast is really needed!
        for(i=0; i<(int32)(signal->NumberOfSamples()-downsample); i+=downsample){
            hStream.Printf("[");
            PrintNextValue(xxBuffer, xxType, hStream, xxMultiplier, downsample);
            hStream.Printf(",");
            PrintNextValue(buffer, signalType, hStream, yyMultiplier, downsample);
            hStream.Printf("],");
        }
        hStream.Printf("[");
        PrintNextValue(xxBuffer, xxType, hStream, xxMultiplier, downsample);
        hStream.Printf(",");
        PrintNextValue(buffer, signalType, hStream, yyMultiplier, downsample);
        hStream.Printf("]]\n");
    }
}

/** The HTTP entry point */
bool
FlotPlotInterface::ProcessHttpMessage(HttpStream &hStream) {
    //A multiplier for the xx axis
    hStream.Seek(0);
    FString multiplier;
    float xxMultiplier = 1;
    if(hStream.Switch("InputCommands.xxMultiplier")){
        hStream.GetToken(multiplier, "");
        if(multiplier.Size() != 0){
            xxMultiplier = atof(multiplier.Buffer());
        }
    }

    //A multiplier for the yy axis
    hStream.Seek(0);
    float yyMultiplier = 1;
    if(hStream.Switch("InputCommands.yyMultiplier")){
        multiplier.SetSize(0);
        hStream.GetToken(multiplier, "");
        if(multiplier.Size() != 0){
            yyMultiplier = atof(multiplier.Buffer());
        }
    }
    //A signal for the xx can be specified
    hStream.Seek(0);
    FString xxSignalName;
    if(hStream.Switch("InputCommands.xxSignal")){
        hStream.GetToken(xxSignalName, "");
    }
    //Option downsampling factor
    int32 downsample = 1;
    if(hStream.Switch("InputCommands.Downsample")){
        FString downsampleStr;
        hStream.GetToken(downsampleStr, "");
        if(downsampleStr.Size() != 0){
            downsample = atoi(downsampleStr.Buffer());
        }
        if(downsample < 1){
            downsample = 1;
        }
    }
    //Width of the chart
    int32 width = 600;
    if(hStream.Switch("InputCommands.Width")){
        FString widthStr;
        hStream.GetToken(widthStr, "");
        if(widthStr.Size() != 0){
            width = atoi(widthStr.Buffer());
        }
    }
    int32 height = 300;
    if(hStream.Switch("InputCommands.Height")){
        FString heightStr;
        hStream.GetToken(heightStr, "");
        if(heightStr.Size() != 0){
            height = atoi(heightStr.Buffer());
        }
    }

    //Is the request to retrieve a signal in json format?
    if(hStream.Switch("InputCommands.DownloadSignal")){
        //Look for the signal name
        FString signalName;
        hStream.Seek(0);
        hStream.GetToken(signalName, "");
        GCRTemplate<SignalInterface> signal;
        GCRTemplate<SignalInterface> xxSignal;
        //Try to fetch the data
        GetSignalData(xxSignalName, signalName, xxSignal, signal);
        if (!signal.IsValid()) {
            CStaticAssertErrorCondition(FatalError, "ProcessHttpMessage: GetSignalData(%s) returns an object not compatible with SignalInterface", signalName.Buffer());
            return False;
        } 
        //Reset the stream
        hStream.Switch((uint32) 0);
        //Write the json header
        hStream.Printf("{\n");
        if(signalName.Size() > 0){
            hStream.Printf("\"label\":");
            signalName.Seek(0);
            FString tokenizedName;
            //Beware of signals between ""
            while(signalName.GetToken(tokenizedName, "\""));
            signalName = tokenizedName;
            tokenizedName.SetSize(0);
            signalName.Seek(0);
            //In order to write strange characters as <
            while(signalName.GetToken(tokenizedName, "<")){
                tokenizedName += "_";
            }
            //Remove the last _
            tokenizedName.SetSize(tokenizedName.Size() - 1);
            hStream.Printf("\"%s\",\n", tokenizedName.Buffer());
        }
        PopulateSignalData(xxSignal, signal, hStream, xxMultiplier, yyMultiplier, downsample);
        hStream.Printf("}\n");
        hStream.SSPrintf("OutputHttpOtions.Content-Type", "application/json");
        hStream.WriteReplyHeader(True);
        return True;
    }
    //The list of signals is comma separated (e.g. SignalList=COLLECTIONABSOLUTEUSECTIME,CYCLETIME)
    else if (hStream.Switch("InputCommands.SignalList")) {
        FString signalList;
        hStream.Seek(0);
        hStream.GetToken(signalList, "");
        signalList.Seek(0);
        //Draw the single points?
        FString drawPoints;
        hStream.Seek(0);
        hStream.Switch("InputCommands.DrawPoints");
        multiplier.SetSize(0);
        hStream.GetToken(drawPoints, "");
        if(drawPoints.Size() == 0){
            drawPoints = "false";
        }

        //Draw the lines? Default is yes...
        FString drawLines;
        hStream.Seek(0);
        hStream.Switch("InputCommands.DrawLines");
        multiplier.SetSize(0);
        hStream.GetToken(drawLines, "");
        if(drawLines.Size() == 0){
            drawLines = "true";
        }

        // Switch To Normal
        hStream.Seek(0);
        hStream.Switch((uint32) 0);
        hStream.Printf("<html>\n");
        hStream.Printf("<head>\n");
        //Location of the flot javascripts
        hStream.Printf("<script language=\"javascript\" type=\"text/javascript\" src=\"/FLOT_DIR/jquery.js\"></script>\n");
        hStream.Printf("<script language=\"javascript\" type=\"text/javascript\" src=\"/FLOT_DIR/jquery.flot.js\"></script>\n");
        hStream.Printf("<script language=\"javascript\" type=\"text/javascript\" src=\"/FLOT_DIR/jquery.flot.selection.js\"></script> \n");
        hStream.Printf("</head>\n");
        hStream.Printf("<body>\n");
        hStream.Printf("<div id=\"plotholder\" style=\"width:%dpx;height:%dpx;\"></div>\n", width, height);

        //jQuery/flot code
        //Holder for the signal name to be iterated from the signal list
        FString signalName;
        signalList.GetToken(signalName, ",");
        signalList.Seek(0);
        int32 signalCounter = 0;
        hStream.Printf("<script type=\"text/javascript\">\n");
        FString privateSignalName;
        privateSignalName.Printf("signal%d", signalCounter++);
        hStream.Printf("    var %s;\n", privateSignalName.Buffer());
        hStream.Printf("$(function () {\n");

        //Flot options and zooming
        hStream.Printf("    var signals = []\n");
        hStream.Printf("    var options = {\n");
        hStream.Printf("        legend: { show: true },\n");
        hStream.Printf("        series: {\n");
        hStream.Printf("            lines: { show: %s },\n", drawLines.Buffer());
        hStream.Printf("            points: { show: %s },\n", drawPoints.Buffer());
        hStream.Printf("        },\n");
        hStream.Printf("        selection: { mode: \"xy\" },\n");
        hStream.Printf("        yaxis: { ticks: 10 }\n");
        hStream.Printf("    };\n");
        hStream.Printf("    var plotholder = $(\"#plotholder\");\n");
        hStream.Printf("    var plot = $.plot(plotholder, signals, options);\n");
        hStream.Printf("    plotholder.bind(\"plotselected\", function (event, ranges) {\n");
        hStream.Printf("        //clamp the zooming to prevent eternal zoom\n");
        hStream.Printf("        if (ranges.xaxis.to - ranges.xaxis.from < 0.00001)\n");
        hStream.Printf("            ranges.xaxis.to = ranges.xaxis.from + 0.00001;\n");
        hStream.Printf("        if (ranges.yaxis.to - ranges.yaxis.from < 0.00001)\n");
        hStream.Printf("            ranges.yaxis.to = ranges.yaxis.from + 0.00001;\n");
        hStream.Printf("        //do the zooming\n");
        hStream.Printf("        plot = $.plot(plotholder, signals,\n");
        hStream.Printf("                  $.extend(true, {}, options, {\n");
        hStream.Printf("                      xaxis: { min: ranges.xaxis.from, max: ranges.xaxis.to },\n");
        hStream.Printf("                      yaxis: { min: ranges.yaxis.from, max: ranges.yaxis.to }\n");
        hStream.Printf("                  }));\n");
        hStream.Printf("        });\n");
        hStream.Printf("    //zoom out with double click\n");
        hStream.Printf("    plotholder.bind(\"dblclick\", function (event) {\n");
        hStream.Printf("        $.plot(plotholder, signals, options);\n");
        hStream.Printf("    });\n");

        hStream.Printf("    function onDataReceived(series) {\n");
        hStream.Printf("        var firstcoordinate = '(' + series.data[0][0] + ', ' + series.data[0][1] + ')';\n");
        hStream.Printf("        signals.push(series); \n");
        hStream.Printf("        $.plot(plotholder, signals, options);\n");
        hStream.Printf("    }\n");

        //Variable for flot which contains the list of signals and the labels to be applied
        signalName.SetSize(0);
        //Iterate through all the signals and populate
        while(signalList.GetToken(signalName, ",")){
            hStream.Printf("    $.ajax({\n");
            //Removed any < characters...
            signalName.Seek(0);
            FString tokenizedName;
            while(signalName.GetToken(tokenizedName, "<")){
                tokenizedName += "%3C";
            }
            signalName = tokenizedName;
            if(signalName.Size() > 3){
                signalName.SetSize(signalName.Size() - 3);
            }
            //Removed any > characters...
            signalName.Seek(0);
            tokenizedName.SetSize(0);
            while(signalName.GetToken(tokenizedName, ">")){
                tokenizedName += "%3E";
            }
            signalName = tokenizedName;
            if(signalName.Size() > 3){
                signalName.SetSize(signalName.Size() - 3);
            }
            //Add all the original GET parameters to the URL
            FString paramList;
            paramList.Printf("       url:'?DownloadSignal=%s", signalName.Buffer());
            int32 i=0;
            for(i=0; i<hStream.NumberOfInputCommands(); i++){
                FString name;
                FString value;
                if(hStream.InputCommandName(name, i) && hStream.InputCommandValue(value, i)){
                    paramList.Printf("&%s=%s", name.Buffer(), value.Buffer());
                }
            }
            paramList += "',\n";
            hStream.Printf("        %s", paramList.Buffer());
            hStream.Printf("        method: 'GET',\n");
            hStream.Printf("        dataType:'json',\n");
            hStream.Printf("        success:onDataReceived\n");
            hStream.Printf("    });\n");
            signalName.SetSize(0);
        }
        hStream.Printf("});\n");
        hStream.Printf("</script>\n");
        hStream.Printf("</body>\n");
        hStream.Printf("</html>\n");
        hStream.SSPrintf("OutputHttpOtions.Content-Type","text/html");
        hStream.WriteReplyHeader(True);
        return True;
    }
    return False;
}


