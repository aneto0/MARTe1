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

#if !defined (HTTPJSCOPEINTERFACE_H)
#define HTTPJSCOPEINTERFACE_H

#include "System.h"
#include "HttpInterface.h"

class HttpJScopeInterface{

protected:

    /** */
    FString                 backGroundColor;

    /** */
    int                     plotSequence;

    /** */
    FString                 codeBase;

    /** */
    int                     width;

    /** */
    int                     height;

    /** */
    FString                 title;

public:

    /** */
    HttpJScopeInterface(){
        backGroundColor = "#CCCCCC"; // gray
        plotSequence    = 1;
        codeBase        = "JAVA";
        width           = 600;
        height          = 400;
        title           = "None";
    }


    /** */
    HttpJScopeInterface(const char *title){
        backGroundColor = "#CCCCCC"; // gray
        plotSequence    = 1;
        codeBase        = "JAVA";
        width           = 600;
        height          = 400;
        this->title           = title;
    }

    /** */
    HttpJScopeInterface(char *title, int width, int height){
        backGroundColor = "#CCCCCC"; // gray
        plotSequence    = 1;
        codeBase        = "JAVA";
        this->width     = width;
        this->height    = height;
        this->title     = title;
    }

    /** */
    void SetCodeBase(char *codeBase){
        this->codeBase = codeBase;
    }

    /** */
    void SetBackGroundColor(char *color){
        this->backGroundColor = color;
    }

    /** */
    void SetTitle(const char *title){
        this->title = title;
    }

    /** */
    void SetSize(int width, int height){
        this->width  = width;
        this->height = height;
    }

    /** */
    ~HttpJScopeInterface(){};

    /** */
    bool ProcessGraphHttpMessageCreate(HttpStream &hStream){
        hStream.Printf("<HEAD><TITLE>%s</TITLE></HEAD>",title.Buffer());
        hStream.Printf("<BODY bgcolor=\"%s\" topmargin=\"0\" leftmargin=\"0\" marginwidth=\"0\" marginheight=\"0\">",backGroundColor.Buffer());
        hStream.Printf("<FONT color=\"#0\" size=2><TABLE><TR><TD>");
        hStream.Printf("</TD></TR><TR><TD>");
        hStream.Printf("<APPLET codebase = \"/%s\" archive = \"SignalViewer.jar\" code=\"CompositeWaveDisplay.class\" align=\"baseline\" width=\"%i\" height=\"%i\" name=\"CompositeWaveDisplay\">",codeBase.Buffer(),width,height);
        return True;
    }

    /** */
    bool ProcessGraphHttpMessageShow(HttpStream &hStream){
        hStream.Printf("</APPLET>");
        hStream.Printf("</TD></TR>");
        hStream.Printf("</TABLE>");
        hStream.Printf("</PRE></FONT></BODY></HTML>");
        plotSequence = 1;
        return True;
    }

    /** */
    bool ProcessGraphHttpMessageAddSignal(HttpStream &hStream, FString xData, FString yData, FString waveName = "Default", FString color = "Red"){
        hStream.Printf("<PARAM name = \"SIGNAL_%i\" value = \"xData = %s yData = %s row = 1 col = 1 name = %s \">",plotSequence,xData.Buffer(),yData.Buffer(),waveName.Buffer());
        plotSequence++; // In case of multiple plot
        return True;
    }
};

#endif
