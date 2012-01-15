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

#include "SignalServer.h"
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

OBJECTLOADREGISTER(SignalServer, "$Id$")


/** The HTTP entry point */
bool
SignalServer::ProcessHttpMessage(HttpStream &hStream) {


    FString binaryDownload;
    binaryDownload = "";

    if (hStream.Switch("InputCommands.binaryDownloadMode")) {

        hStream.Seek(0);
        hStream.GetToken(binaryDownload, "");
        if (binaryDownload == "true")
            binaryDownloadMode = 1;
        else binaryDownloadMode = 0;

    }
    // what subtree to open
    FString signalRequest;

    signalRequest = "";

    if (hStream.Switch("InputCommands.SignalRequest")) {
        hStream.Seek(0);
        hStream.GetToken(signalRequest, "");

        // Switch To Normal
        hStream.Switch((uint32) 0);

        if (binaryDownloadMode) {

            GCRTemplate<SignalInterface> signalGCO = GetSignal(signalRequest.Buffer());
            if (!signalGCO.IsValid()) {
                AssertErrorCondition(FatalError, "ProcessHttpMessage: GetSignal(%s) returns an object not comaptible with GCNamedObject", signalRequest.Buffer());
                return False;
            } else {
                uint32 size = signalGCO->NumberOfSamples() * signalGCO->Type().ByteSize();
                hStream.Write(signalGCO->Buffer(), size);
            }

        } else {

            GCRTemplate<GCNamedObject> signalGCO = GetSignal(signalRequest.Buffer());
            if (!signalGCO.IsValid()) {
                AssertErrorCondition(FatalError, "ProcessHttpMessage: GetSignal(%s) returns an object not comaptible with GCNamedObject", signalRequest.Buffer());
                return False;
            } else {
                // Using new CDBOS: saves a CDB to a stream without buffering to memory
                ConfigurationDataBase content("CDBOS");
                // Assigns the stream to write to!
                content->WriteToStream(hStream);
                signalGCO->ObjectSaveSetup(content, NULL);
            }

        }

        if (binaryDownloadMode)
            hStream.SSPrintf("OutputHttpOtions.Content-Type", "binary/octet-stream");
        else hStream.SSPrintf("OutputHttpOtions.Content-Type", "text/html");

        hStream.SSPrintf("OutputHttpOtions.Content-Disposition:", "attachment; filename=\"%s\"", signalRequest.Buffer());
        //copy to the client
        hStream.WriteReplyHeader(True);

        return True;

    }

    {

        hStream.Printf(
                "<HTML>\n"
                "<HEAD>\n"
                "<TITLE>%s</TITLE>\n"
                "<SCRIPT TYPE=\"text/javascript\">\n"
                "   function Download(signalName){\n"
                "   window.location.replace(\"./?SignalRequest=\" + signalName +\"&binaryDownloadMode=\"+document.form01.binaryDownloadMode.checked);\n"
                "   return true;\n"
                "   }\n"
                "</SCRIPT>\n"
                "</HEAD>\n"
                "<BODY BGCOLOR=\"#ffffff\"><H1>%s</H1><UL>\n"
                "<FORM NAME=\"form01\" METHOD=\"GET\">\n\n"
                , Name()
                , Name()
                );

        CDBExtended cdbx(signalDataBase);

        if (cdbx->Move("Signals")) {
            unsigned int childNo=cdbx->NumberOfChildren();
            if (childNo > 0) {
                hStream.Printf("<INPUT TYPE=\"CHECKBOX\" %s NAME=\"binaryDownloadMode\" value=\"%d\"><B>Download As Binary File</B></INPUT>"
                        "<TABLE>\n"
                        "\n"
                        ,(!binaryDownloadMode ? "" : "checked=\"yes\"")
                        , binaryDownloadMode
                        );

                HtmlStream hmStream(hStream);
                for (int i = 0; i < childNo; i++) {
                    if (cdbx->MoveToChildren(i)) {
                        BString nodeName;
                        if (cdbx->NodeName(nodeName)) {

                            hmStream.SSPrintf(HtmlTagStreamMode, "TR");
                            hmStream.SSPrintf(HtmlTagStreamMode, "TD");
                            hmStream.Printf("%s", nodeName.Buffer());
                            hmStream.SSPrintf(HtmlTagStreamMode, "/TD");
                            hmStream.SSPrintf(HtmlTagStreamMode, "TD");
                            hmStream.SSPrintf(HtmlTagStreamMode,
                                    "INPUT "
                                    "type=\"button\" name=\"%s\" title=\"DOWNLOAD\" "
                                    "ONCLICK=\"Download('%s')\"",
                                    nodeName.Buffer(),
                                    nodeName.Buffer()
                                    );
                            hmStream.SSPrintf(HtmlTagStreamMode, "/TD");
                            hmStream.SSPrintf(HtmlTagStreamMode, "/TR");
                        }
                        cdbx->MoveToFather();
                    }
                }
                hStream.Printf("</TABLE>\n");

                cdbx->MoveToFather();

            }

        }

    }
    hStream.Printf(
            "</FORM>\n"
            "</UL></BODY>\n"
            "</HTML>\n");

    hStream.SSPrintf("OutputHttpOtions.Content-Type", "text/html");
    //copy to the client
    hStream.WriteReplyHeader(True);


    return True;
}


