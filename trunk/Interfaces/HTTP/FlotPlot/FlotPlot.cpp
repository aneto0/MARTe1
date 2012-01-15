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

#include "FlotPlot.h"
#include "GAM.h"
#include "GlobalObjectDataBase.h"
#include "GCNString.h"
#include "BasicTypes.h"
#include "SignalInterface.h"
#include "HtmlStream.h"
#include "Signal.h"

OBJECTLOADREGISTER(FlotPlot, "$Id: FlotPlot.cpp,v 1.4 2011/07/22 09:55:01 aneto Exp $")
/** The HTTP entry point */
bool
FlotPlot::ProcessHttpMessage(HttpStream &hStream) {
    //Check if it is to plot a signal
    if(FlotPlotInterface::ProcessHttpMessage(hStream)){
        return True;
    }
    //Apparently not... list all signals for the selected pulse number
    hStream.Seek(0);
    hStream.Switch((uint32) 0);
    //Render a simple table with all the signals
    {
        hStream.Printf(
                "<html>\n"
                "<head>\n"
                "<title>%s</title>\n"
                "<script type=\"text/javascript\">\n"
                "   function Plot(signalName){\n"
                "       window.location.href = \"./?SignalList=\" + signalName;\n"
                "       return true;\n"
                "   }\n"
                "   function PlotSelected()\n{"
                "       var xxSignal = \"\";\n"
                "       for (i=0; i<document.form01.xxSignal.length; i++){\n"
                "           if (document.form01.xxSignal[i].checked==true){\n"
                "               xxSignal = document.form01.xxSignal[i].value\n"
                "               break;\n"
                "           }\n"
                "       }\n"
                "       var signalList = \"\";\n"
                "       for (i=0; i<document.form01.SignalList.length; i++){\n"
                "           if (document.form01.SignalList[i].checked==true){\n"
                "               signalList += document.form01.SignalList[i].value + \",\"\n"
                "           }\n"
                "       }\n"
                "       signalList = signalList.substring(0, signalList.length - 1);\n"
                "       var xxMultiplier = document.form01.xxMultiplier.value;\n"
                "       var yyMultiplier = document.form01.yyMultiplier.value;\n"
                "       var width        = document.form01.Width.value;\n"
                "       var height       = document.form01.Height.value;\n"
                "       var drawLines    = \"false\";\n"
                "       if (document.form01.drawLines.checked==true){\n"
                "           drawLines = \"true\";\n"
                "       }\n"
                "       var drawPoints = \"false\";\n"
                "       if (document.form01.drawPoints.checked==true){\n"
                "           drawPoints = \"true\";\n"
                "       }\n"
                "       window.location.href = \"./?SignalList=\" + signalList + \"&Width=\" + width + \"&Height=\" + height + \"&xxSignal=\" + xxSignal + \"&xxMultiplier=\" + xxMultiplier + \"&yyMultiplier=\" + yyMultiplier + \"&DrawLines=\" + drawLines + \"&DrawPoints=\" + drawPoints;\n"
                "       return true;\n"
                "   }\n"
                "</script>\n"
                "</head>\n"
                "<body bgcolor=\"#ffffff\"><h1>%s</h1><ul>\n"
                "<form name=\"form01\" method=\"get\">\n\n"
                , Name()
                , Name()
                );

        CDBExtended cdbx(signalDataBase);

        if (cdbx->Move("Signals")) {
            unsigned int childNo=cdbx->NumberOfChildren();
            if (childNo > 0) {
                hStream.Printf("<table>\n");
                HtmlStream hmStream(hStream);
                hmStream.SSPrintf(HtmlTagStreamMode, "tr");
                hmStream.SSPrintf(HtmlTagStreamMode, "th");
                hStream.Printf("Signal Name");
                hmStream.SSPrintf(HtmlTagStreamMode, "/th");
                hmStream.SSPrintf(HtmlTagStreamMode, "th");
                hStream.Printf("");
                hmStream.SSPrintf(HtmlTagStreamMode, "/th");
                hmStream.SSPrintf(HtmlTagStreamMode, "th");
                hStream.Printf("XX?");
                hmStream.SSPrintf(HtmlTagStreamMode, "/th");
                hmStream.SSPrintf(HtmlTagStreamMode, "th");
                hStream.Printf("YY?");
                hmStream.SSPrintf(HtmlTagStreamMode, "/th");
                hmStream.SSPrintf(HtmlTagStreamMode, "/tr");
                for (int i = 0; i < childNo; i++) {
                    if (cdbx->MoveToChildren(i)) {
                        BString nodeName;
                        if (cdbx->NodeName(nodeName)) {

                            hmStream.SSPrintf(HtmlTagStreamMode, "tr");
                            hmStream.SSPrintf(HtmlTagStreamMode, "td");
                            hmStream.Printf("%s", nodeName.Buffer());
                            hmStream.SSPrintf(HtmlTagStreamMode, "/td");
                            hmStream.SSPrintf(HtmlTagStreamMode, "td");
                            hmStream.SSPrintf(HtmlTagStreamMode,
                                    "input "
                                    "type=\"button\" name=\"%s\" title=\"plot\" "
                                    "onclick=\"Plot('%s')\"",
                                    nodeName.Buffer(),
                                    nodeName.Buffer()
                                    );
                            hmStream.SSPrintf(HtmlTagStreamMode, "/td");
                            hmStream.SSPrintf(HtmlTagStreamMode, "td");
                            hStream.Printf("<input type=\"radio\" name=\"xxSignal\" value=\"%s\"/>", nodeName.Buffer());
                            hmStream.SSPrintf(HtmlTagStreamMode, "/td");
                            hmStream.SSPrintf(HtmlTagStreamMode, "td");
                            hStream.Printf("<input type=\"checkbox\" name=\"SignalList\" value=\"%s\"/>", nodeName.Buffer());
                            hmStream.SSPrintf(HtmlTagStreamMode, "/td");
                            hmStream.SSPrintf(HtmlTagStreamMode, "/tr");
                        }
                        cdbx->MoveToFather();
                    }
                }
                hStream.Printf("</table>\n");
                hStream.Printf("xx multiplier: <input type=\"text\" name=\"xxMultiplier\" value=\"1.0\"/><br/>");
                hStream.Printf("yy multiplier: <input type=\"text\" name=\"yyMultiplier\" value=\"1.0\"/><br/>");
                hStream.Printf("Width: <input type=\"text\" name=\"Width\" value=\"600\"/><br/>");
                hStream.Printf("Height: <input type=\"text\" name=\"Height\" value=\"300\"/><br/>");
                hStream.Printf("Draw lines:    <input type=\"checkbox\" name=\"drawLines\" checked=\"yes\" /><br/>");
                hStream.Printf("Draw points:   <input type=\"checkbox\" name=\"drawPoints\" /><br/>");
                hStream.Printf("<input type=\"button\" value=\"Plot selected\" onclick=\"PlotSelected()\"/>");

                cdbx->MoveToFather();

            }

        }

    }
    hStream.Printf(
            "</form>\n"
            "</ul></body>\n"
            "</html>\n");

    hStream.SSPrintf("OutputHttpOtions.Content-Type", "text/html");
    //copy to the client
    hStream.WriteReplyHeader(True);


    return True;
}


