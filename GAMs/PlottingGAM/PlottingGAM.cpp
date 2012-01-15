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

#include "PlottingGAM.h"
#include "CDBExtended.h"

/// This method runs at initialisation
bool PlottingGAM::Initialise(ConfigurationDataBase &cdbData) {

    CDBExtended cdb(cdbData);

    cdb.ReadInt32(httpRefreshMsecTime, "HttpRefreshMsecTime", 500);

    numberOfPlotWindows = Size();

    if((gcrtpw = new GCRTemplate<PlotWindow>[numberOfPlotWindows]) == NULL) {
	AssertErrorCondition(InitialisationError, "PlottingGAM::Initialise: %s unable to allocate memory for reference storage", Name());
	return False;
    }

    for(int i = 0 ; i < numberOfPlotWindows ; i++) {
    	GCRTemplate<PlotWindow> gcr = Find(i);
    	if(gcr.IsValid()) {
    	    gcrtpw[i] = gcr;
    	} else {
	    AssertErrorCondition(InitialisationError, "PlottingGAM::Initialise: %s only PlotWindow objects allowed", Name());
	    return False;
	}
    }

    return True;
}

/// Called in every control loop
bool PlottingGAM::Execute(GAM_FunctionNumbers execFlag) {

    for(int i = 0 ; i < numberOfPlotWindows ; i++) {
    	gcrtpw[i]->Execute(execFlag);
    }

    return True;
}

bool PlottingGAM::ProcessHttpMessage(HttpStream &hStream) {

    FString ajaxString;
    ajaxString.SetSize(0);
    ajaxString.Printf("var data = new Array(%d);\n", numberOfPlotWindows);
    ajaxString.Printf("var titles = new Array(%d);\n", numberOfPlotWindows);
    ajaxString.Printf("var xLabels = new Array(%d);\n", numberOfPlotWindows);
    ajaxString.Printf("var yLabels = new Array(%d);\n", numberOfPlotWindows);
    ajaxString.Printf("var xAxisLimits = new Array(%d);\n", numberOfPlotWindows);
	int i;
    for(i = 0 ; i < numberOfPlotWindows ; i++) {
    	gcrtpw[i]->AppendJsDataString(ajaxString);
    	ajaxString.Printf("data[%d] = pwData;\n", i);
    	ajaxString.Printf("titles[%d] = title;\n", i);
    	ajaxString.Printf("xLabels[%d] = xLabel;\n", i);
    	ajaxString.Printf("yLabels[%d] = yLabel;\n", i);
    	ajaxString.Printf("xAxisLimits[%d] = xLimits;\n", i);
    }    
    
    FString refreshData;
    refreshData.SetSize(0);
    hStream.Seek(0);
    if(hStream.Switch("InputCommands.PlottingGAMRefreshData")){
        hStream.Seek(0);
        hStream.GetToken(refreshData, "");
        hStream.Switch((uint32)0);
    	hStream.Printf("%s\n", ajaxString.Buffer());
    	hStream.WriteReplyHeader(True);
    	return True;
    }

    hStream.SSPrintf("OutputHttpOtions.Content-Type","text/html");
    hStream.keepAlive = False;

    //copy to the client
    hStream.WriteReplyHeader(False);

    /* Start html document */
    hStream.Printf("<html>\n");

    /* Page head */
    hStream.Printf("<head>\n");

    /* Page title */
    hStream.Printf("<title>Plotting GAM</title>\n");

    /* Include relevant RGraph libraries */
    hStream.Printf("<script src=\"/RGRAPH_LIB_DIR/RGraph.common.core.js\"></script>\n");
    hStream.Printf("<script src=\"/RGRAPH_LIB_DIR/RGraph.common.zoom.js\"></script>\n");
    hStream.Printf("<script src=\"/RGRAPH_LIB_DIR/RGraph.line.js\"></script>\n");
    hStream.Printf("<script src=\"/PLOTTING_GAM_DIR/PlottingGAM.js\"></script>\n");

    /* End page head */
    hStream.Printf("</head>\n");

    hStream.Printf("<body onload=\"timedUpdate();\">\n");
    hStream.Printf("<script>\n");

    hStream.Printf("var request = new XMLHttpRequest();\n");

    hStream.Printf("function timedUpdate() {\n");
    hStream.Printf("var url = \"?PlottingGAMRefreshData\";\n");
    hStream.Printf("request.open('GET', url, true);\n");
    hStream.Printf("request.onreadystatechange = plotManager;\n");
    hStream.Printf("request.send(null);\n");
    hStream.Printf("window.setTimeout(timedUpdate, %d);\n", httpRefreshMsecTime);
    hStream.Printf("}\n");

    hStream.Printf("function plotManager() {\n");
    hStream.Printf("if(this.readyState != 4 || this.status != 200) {\n");
    hStream.Printf("return;\n");
    hStream.Printf("}\n");
    hStream.Printf("eval(request.responseText);\n");
    for(i = 0 ; i < numberOfPlotWindows ; i++) {
	hStream.Printf("plot('canvas%d', data[%d], xAxisLimits[%d], titles[%d], xLabels[%d], yLabels[%d], %d);\n", i, i, i, i, i, i, gcrtpw[i]->onlyPositiveYBool);
    }
    hStream.Printf("}\n");
    
    hStream.Printf("</script>\n");
    
    int32 heightCumulative = 0;
    for(i = 0 ; i < numberOfPlotWindows ; i++) {
    	hStream.Printf("<canvas id='canvas%dtitle' width='%d' height='50' style='position:absolute; left:0px; top:%dpx'></canvas>\n", i, gcrtpw[i]->plotPixelWidth, heightCumulative);
	heightCumulative += 50;
    	hStream.Printf("<canvas id='canvas%d' width='%d' height='%d' style='position:absolute; left:0px; top:%dpx'></canvas>\n", i, gcrtpw[i]->plotPixelWidth, gcrtpw[i]->plotPixelHeight, heightCumulative);
	heightCumulative += gcrtpw[i]->plotPixelHeight;
    }
    hStream.Printf("<canvas id='canvasAck' width='400' height='400' style='position:absolute; left:0px; top:%dpx'></canvas>\n", heightCumulative+50);
    hStream.Printf("<script>\n");
    hStream.Printf("putRGraphLabel(\"canvasAck\");\n", numberOfPlotWindows-1);
    hStream.Printf("</script>\n");
    hStream.Printf("</body>\n");

    hStream.Printf("</html>\n");

    return True;
}
OBJECTLOADREGISTER(PlottingGAM, "$Id: PlottingGAM.cpp,v 1.11 2011/02/17 14:06:37 ppcc_dev Exp $")
