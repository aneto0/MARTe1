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

#include "GAM.h"
#include "GlobalObjectDataBase.h"
#include "GCNString.h"
#include "BasicTypes.h"
#include "SignalInterface.h"
#include "HtmlStream.h"
#include "Signal.h"
#include "MATLABHandler.h"

OBJECTLOADREGISTER(MATLABHandler,"$Id$")

bool MATLABHandler::SaveSingleSignal(const char* signalName, MatlabConverter &mc, const char* matlabVarName) {
    GCRTemplate<GCNamedObject> signalGCO = GetSignal(signalName);

    if (!signalGCO.IsValid()){
        AssertErrorCondition(FatalError,"ProcessHttpMessage: SaveSingleSignal(%s) returns an object not comaptible with GCNamedObject",signalName);
        return False;
    }

    SignalInterface* s;

    if ( (s = dynamic_cast<SignalInterface*>(signalGCO.operator->())) == NULL ){
        AssertErrorCondition(FatalError,"ProcessHttpMessage: SaveSingleSignal(%s) signalGCO is not a Signal",signalName);
        return False;
    }

    // Find signal type
    int numberOfSamples = s->NumberOfSamples();

    if (s->Type() == BTDInt32 || s->Type() == BTDUint32) {
        MatrixI m;
        m.Allocate(1, numberOfSamples);
        int* data = m.Data();
        memcpy((void*)data, (void*)s->Buffer(), numberOfSamples*sizeof(int));
        mc.Save(m,matlabVarName);
    } else if (s->Type() == BTDFloat) {
        MatrixF m;
        m.Allocate(1, numberOfSamples);
        float* data = m.Data();
        memcpy((void*)data, (void*)s->Buffer(), numberOfSamples*sizeof(float));
        mc.Save(m,matlabVarName);

    } else {
        AssertErrorCondition(FatalError,"ProcessHttpMessage: SaveSingleSignal(%s) unhandled signal type",signalName);
        return False;
    }

    return True;
}

void ConvertStringToHTML(FString &toBuild, const char* signalName) {
    toBuild.SetSize(0);
    char tmp[50];
    int i;
    int counter = 0;
    int sLen = strlen(signalName);
    for (i=0; i<sLen; i++) {
        char point = signalName[i];
        if (point == '<') {
            tmp[counter]='&';
            tmp[counter+1]='l';
            tmp[counter+2]='t';
            tmp[counter+3]=';';
            counter += 4;
        } else if (point == '>') {
            tmp[counter]='&';
            tmp[counter+1]='g';
            tmp[counter+2]='t';
            tmp[counter+3]=';';
            counter += 4;
        } else {
            tmp[counter] = point;
            counter++;
        }
    }
    tmp[counter] = '\0';
    toBuild = tmp;
}

bool MATLABHandler::SaveAllSignals(MatlabConverter &mc) {

    CDBExtended cdbx(signalDataBase);

    if (cdbx->Move("Signals")){
        for (int i = 0;i < cdbx->NumberOfChildren();i++){
            if (cdbx->MoveToChildren(i)){
                FString nodeName;
                if (cdbx->NodeName(nodeName)){
                    FString tmp;
                    RemoveIllegalCharacters(tmp, nodeName, "/<:-");
                    if (!SaveSingleSignal(nodeName.Buffer(), mc, tmp.Buffer())) {
                        AssertErrorCondition(FatalError,"ProcessHttpMessage: SaveAllSignals(): could not find signal %s",nodeName.Buffer());
                        return False;
                    }
                }
                cdbx->MoveToFather();
            }
        }
        cdbx->MoveToFather();
    }

    return True;
}

void MATLABHandler::PrintForm(HttpStream &hStream) {
    hStream.Printf("<html><head><title>MATLAB Handler</title></head><body>\n");
    hStream.Printf("<form enctype=\"multipart/form-data\" method=\"post\">\n");
    hStream.Printf("<input type=\"hidden\" name=\"FORMSENT\">\n");
    hStream.Printf("<input type=\"checkbox\" name=\"ALLSIGNALS\">Dump all signals to file<br />\n");
    CDBExtended cdbx(signalDataBase);
    if (cdbx->Move("Signals")){
        FString tmp;
        for (int i = 0;i < cdbx->NumberOfChildren();i++){
            if (cdbx->MoveToChildren(i)){
                FString nodeName;
                if (cdbx->NodeName(nodeName)){
                    ConvertStringToHTML(tmp, nodeName.Buffer());
                    hStream.Printf("<input type=\"checkbox\" name=\"%s\">%s<br />\n", nodeName.Buffer(), tmp.Buffer());
                }
                cdbx->MoveToFather();
            }
        }
    }
    hStream.Printf("<input type=\"submit\" value=\"Send\"></form>");
    hStream.Printf("</body></html>");
}

/** The HTTP entry point */
bool MATLABHandler::ProcessHttpMessage(HttpStream &hStream) {

    // The form has been sent: download data
    if (hStream.Switch("InputCommands.FORMSENT")) {
        hStream.Switch((uint32)0);
        MatlabConverter mc(&hStream);
        bool ret = True;

        if (!hStream.Switch("InputCommands.ALLSIGNALS")) {
            CDBExtended cdbx(signalDataBase);
            if (cdbx->Move("Signals")){
                FString tmp;
                for (int i = 0;i < cdbx->NumberOfChildren();i++){
                    if (cdbx->MoveToChildren(i)){
                        FString nodeName;
                        if (cdbx->NodeName(nodeName)){
                            tmp.SetSize(0);
                            tmp.Printf("InputCommands.%s", nodeName.Buffer());
                            if (hStream.Switch(tmp.Buffer())) { // The signal was selected
                                hStream.Switch((uint32)0);
                                RemoveIllegalCharacters(tmp, nodeName, "/<:-");
                                if (!SaveSingleSignal(nodeName.Buffer(), mc, tmp.Buffer())) {
                                    AssertErrorCondition(FatalError,"ProcessHttpMessage could not find signal %s",nodeName.Buffer());
                                    ret = False;
                                }
                            }
                        }
                        cdbx->MoveToFather();
                    }
                }
            }
            hStream.Switch((uint32)0);
            hStream.SSPrintf("OutputHttpOtions.Content-Type","application/octet-stream");
            hStream.SSPrintf("OutputHttpOtions.Content-Disposition:", "attachment; filename=\"signals.mat\"");
        } else { // If ALLSIGNALS has been chosen
            hStream.Switch((uint32)0);
            hStream.SSPrintf("OutputHttpOtions.Content-Type","application/octet-stream");
            hStream.SSPrintf("OutputHttpOtions.Content-Disposition:", "attachment; filename=\"allsignals.mat\"");
            ret = SaveAllSignals(mc);
        }
        //copy to the client
        hStream.WriteReplyHeader(True);
        return True;
    } else { // The form hasn't yet been sent
        PrintForm(hStream);
        hStream.SSPrintf("OutputHttpOtions.Content-Type","text/html");
        //copy to the client
        hStream.WriteReplyHeader(True);
        return True;
    }
}


