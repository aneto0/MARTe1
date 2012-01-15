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


#include "StateMachine.h"
#include "CDBExtended.h"
#include "HtmlStream.h"

OBJECTLOADREGISTER(StateMachine,"$Id$")

bool SMObjectLoadSetup(StateMachine &sm,ConfigurationDataBase &info,StreamInterface *err){

    CDBExtended cdbx(info);

    bool ret = sm.GCReferenceContainer::ObjectLoadSetup(info,err);
    ret = ret && sm.HttpInterface::ObjectLoadSetup(info,err);

    cdbx.ReadBString(sm.errorStateName,"ErrorStateName","ERROR");

    cdbx.ReadInt32(sm.verboseLevel,"VerboseLevel",0);

    sm.currentState = sm.Find(0);
    sm.errorState = sm.Find(sm.errorStateName.Buffer());
    sm.defaultState = sm.Find("DEFAULT");

    if (sm.currentState.IsValid()){
        if (sm.verboseLevel >= 1) {
            sm.AssertErrorCondition(Information,"ObjectLoadSetup:Current State is %s",sm.currentState->Name());
        }
    } else {
        sm.AssertErrorCondition(FatalError,"ObjectLoadSetup:Current State is not defined");
        return False;
    }

    if (!sm.errorState.IsValid()){
        sm.errorState = sm.currentState;
        sm.AssertErrorCondition(Warning,"ObjectLoadSetup:Undefined Error State: using current state");
    }
    return ret;

}

bool SMObjectSaveSetup(StateMachine &sm,ConfigurationDataBase &info,StreamInterface *err){

    bool ret = sm.GCReferenceContainer::ObjectSaveSetup(info,err);
    ret = ret && sm.HttpInterface::ObjectSaveSetup(info,err);
    return ret;

}

bool SMProcessMessage(StateMachine &sm,GCRTemplate<MessageEnvelope> envelope){

    if (sm.verboseLevel >= 10) {
        sm.AssertErrorCondition(Information,"ProcessMessage:%s Received Message from %s: ",sm.Name(),envelope->Sender());
    }

    GCRTemplate<Message> message = envelope->GetMessage();
    if (message.IsValid()){

        if (sm.verboseLevel >= 10) {
            sm.AssertErrorCondition(Information,"ProcessMessage:Message[(code)0x%x,(id)%i]  is %s\n",message->GetMessageCode().Code(),message->Id(),message->Content());
        }

        MessageCode messageCode     = message->GetMessageCode();
        if (messageCode == NullMessage){
            sm.Trigger(0,message->Content());
        } else {
            sm.Trigger(messageCode.Code(),"");
        }

    }
    else {
        sm.AssertErrorCondition(CommunicationError,"ProcessMessage:Empty Message Envelope");
    }

    if (envelope->ManualReplyExpected()){
        if (sm.verboseLevel >= 10) {
            sm.AssertErrorCondition(Information,"ProcessMessage:%s Sending Manual reply to %s\n",sm.Name(),envelope->Sender());
        }
        GCRTemplate<MessageEnvelope> gcrtme(GCFT_Create);
        GCRTemplate<Message> gcrtm(GCFT_Create);
        if (sm.currentState.IsValid()){
            gcrtm->Init(sm.currentState->StateCode(),sm.currentState->Name());
        } else {
            gcrtm->Init(FinishedMessage,"REPLY MESSAGE");
        }
        gcrtme->PrepareReply(envelope,gcrtm);
        MessageHandler::SendMessage(gcrtme);
    }

    return True;
}

bool StateMachine::ChangeState(const char *newStateName){
    if(strcmp(newStateName,"SAMESTATE") == 0) return False;

    bool stateChanged = False;

    GCRTemplate<StateMachineState> newState = Find(newStateName);
    if (newState.IsValid()){
        if ( currentState != newState ){
            SMS_ActOnResults ret;
            // exit current state
            ret = currentState->Exit(*this);

            if (ret == SMS_NotFound){
                // perform exit on the default state
                if (defaultState.IsValid()){
                    ret = defaultState->Exit(*this);
                }
            }

            if (ret == SMS_Error){
                currentState = errorState;
                stateChanged = True;
                AssertErrorCondition(Warning,"Trigger:Failed existing from state %s",currentState->Name());
            } else {
                // try changing state
                currentState = newState;
                stateChanged = True;

                SMS_ActOnResults ret;

                ret  = newState->Enter(*this);

                if (ret == SMS_NotFound){
                    // perform enter on the default state
                    if (defaultState.IsValid()){
                        ret = defaultState->Enter(*this);
                    }
                }

                if (ret == SMS_Error){
                    currentState = errorState;
                    stateChanged = True;

                    ret  = errorState->Enter(*this);
                    if (ret == SMS_NotFound){
                        // perform enter on the default state
                        if (defaultState.IsValid()){
                            ret = defaultState->Enter(*this);
                        }
                    }

                    AssertErrorCondition(Warning,"Trigger:Failed entering into state %s",newState->Name());
                }
            }
        }
    }
    return stateChanged;
}

/* Process the @param code or @param value to determine the next state */
bool StateMachine::Trigger(int code,const char *value){
    if (!currentState.IsValid()){
        AssertErrorCondition(FatalError,"Trigger:Undefined State");
        return False;
    }

    if (!errorState.IsValid()){
        errorState = currentState;
        AssertErrorCondition(Warning,"Trigger:Undefined Error State: using current state");
    }

    bool stateChanged = False;
    FString newStateName;
    bool matched = False;
    matched = currentState->Trigger(*this,code,value,newStateName,verboseLevel);
    if (!matched && defaultState.IsValid()){
        matched = defaultState->Trigger(*this,code,value,newStateName,verboseLevel);
    }
    if (matched){
        stateChanged = ChangeState(newStateName.Buffer());
    } else{
        if (currentState != errorState){
            currentState = errorState;
            stateChanged = True;
            currentState->Exit(*this);

            // perform exit on the default state
            if (defaultState.IsValid()){
                defaultState->Exit(*this);
            }
        }
    }

    if ((verboseLevel >= 1) && (stateChanged)){
        if (currentState.IsValid()){
            AssertErrorCondition(Information,"Current State is %s",currentState->Name());
        }
    }

    return False;
}

/** Simply list the content of a GCReferenceContainer*/
class SMEPageBuilder:public IteratorT<GCReference> {
    /** where to write to */
    StreamInterface *   stream;

public:
    /** @param s = NULL the code will use printf to write to the console */
    SMEPageBuilder(StreamInterface *s = NULL){
        stream = s;
    }

    /** actual function */
    virtual void Do(GCReference data){
        if (!stream) return;
        const char *name;

        GCRTemplate<MessageEnvelope> me;
        me = data;
        if (me.IsValid()){
            GCRTemplate<Message> message = me->GetMessage();
            name = me->Name();
            if (stream){
                stream->Printf(
                    "<TR>\n"
                    "<TD>.act</TD>\n"
                    "<TD></TD>"
                    "<TD>%s</TD>\n",
                    name);
                if (message.IsValid()){
                    int32 code = message->GetMessageCode().Code();
                    if (code > UserMessageCode.Code()){
                        stream->Printf(
                            "<TD>U+%i</TD>\n",code-UserMessageCode.Code()
                        );
                    } else {
                        stream->Printf(
                            "<TD>%i</TD>\n",code
                        );
                    }
                    stream->Printf(
                        "<TD>%s</TD>\n",
                        message->Content()
                    );
                } else {
                    stream->Printf(
                        "<TD>NO CONTENT</TD>\n"
                        "<TD>NO CONTENT</TD>\n"
                    );
                }
                stream->Printf(
                    "<TD>%s</TD>\n"
                    "</TR>\n",me->Destination()
                );
            }
            return;
        }

        GCRTemplate<MessageDeliveryRequest> mdr;
        mdr = data;
        if (mdr.IsValid()){
            GCRTemplate<Message> message = mdr->GetMessage();
            name = mdr->Name();
            if (stream){
                stream->Printf(
                    "<TR>\n"
                    "<TD>. act</TD>\n"
                    "<TD></TD>"
                    "<TD>%s</TD>\n",
                    name);
                if (message.IsValid()){
                    int32 code = message->GetMessageCode().Code();
                    if (code > UserMessageCode.Code()){
                        stream->Printf(
                            "<TD>U+%i</TD>\n",code-UserMessageCode.Code()
                        );
                    } else {
                        stream->Printf(
                            "<TD>%i</TD>\n",code
                        );
                    }
                    stream->Printf(
                        "<TD>%s</TD>\n",
                        message->Content()
                    );
                } else {
                    stream->Printf(
                        "<TD>NO CONTENT</TD>\n"
                        "<TD>NO CONTENT</TD>\n"
                    );
                }
                stream->Printf(
                    "<TD>%s</TD>\n"
                    "</TR>\n",mdr->Destinations()
                );
            }
            return;
        }
    }

};


/** Simply list the content of a GCReferenceContainer*/
class SMSPageBuilder:public IteratorT<GCReference> {
    /** where to write to */
    StreamInterface *   stream;

public:
    /** @param s = NULL the code will use printf to write to the console */
    SMSPageBuilder(StreamInterface *s = NULL){
        stream = s;
    }

    /** actual function */
    virtual void Do(GCReference data){
        if (!stream) return;
        GCRTemplate<StateMachineEvent> sme;
        const char * name;
        sme = data;
        if (sme.IsValid()){
            FString codeString;
            if (sme->Code() < UserMessageCode.Code()){
                codeString.Printf("%i",sme->Code());
            } else {
                codeString.Printf("U+%i",sme->Code()-UserMessageCode.Code());
            }

            name = sme->Name();
            stream->Printf(
                "<TR>\n"
                "<TD>.    event</TD>\n"
                "<TD><form><button type=\"submit\" name=\"StatusChangeRequest\" value=\"%s\">%s</button></form></TD>\n", name, name);
            stream->Printf(
                "<TD></TD>\n"
                "<TD>%s</TD>\n"
                "<TD>%s</TD>\n"
                "<TD>%s</TD>\n"
                "<TD>%s</TD></TR>\n",
                codeString.Buffer(),
                sme->Value(),
                sme->NextState(),
                sme->ErrorState()
            );
            SMEPageBuilder smepb(stream);
            sme->Iterate(&smepb);
            return;
        }

        GCRTemplate<MessageEnvelope> me;
        me = data;
        if (me.IsValid()){
            GCRTemplate<Message> message = me->GetMessage();
            name = me->Name();
            if (stream){
                stream->Printf(
                    "<TR>\n"
                    "<TD>.  entry</TD>"
                    "<TD></TD>"
                    "<TD>%s</TD>\n",
                    name);
                if (message.IsValid()){
                    int32 code = message->GetMessageCode().Code();
                    if (code > UserMessageCode.Code()){
                        stream->Printf(
                            "<TD>U+%i</TD>\n",code-UserMessageCode.Code()
                        );
                    } else {
                        stream->Printf(
                            "<TD>%i</TD>\n",code
                        );
                    }
                    stream->Printf(
                        "<TD>%s</TD>\n",
                        message->Content()
                    );
                } else {
                    stream->Printf(
                        "<TD>NO CONTENT</TD>\n"
                        "<TD>NO CONTENT</TD>\n"
                    );
                }
                stream->Printf(
                    "<TD>%s</TD>\n" "</TR>\n",me->Destination()
                );
            }
            return;
        }

        GCRTemplate<MessageDeliveryRequest> mdr;
        mdr = data;
        if (mdr.IsValid()){
            GCRTemplate<Message> message = mdr->GetMessage();
            name = mdr->Name();
            if (stream){
                stream->Printf(
                    "<TR>\n"
                    "<TD>. entry</TD>"
                    "<TD></TD>\n"
                    "<TD>%s</TD>\n",
                    name);
                if (message.IsValid()){
                    int32 code = message->GetMessageCode().Code();
                    if (code > UserMessageCode.Code()){
                        stream->Printf(
                            "<TD>U+%i</TD>\n",code-UserMessageCode.Code()
                        );
                    } else {
                        stream->Printf(
                            "<TD>%i</TD>\n",code
                        );
                    }
                    stream->Printf(
                        "<TD>%s</TD>\n",
                        message->Content()
                    );
                } else {
                    stream->Printf(
                        "<TD>NO CONTENT</TD>\n"
                        "<TD>NO CONTENT</TD>\n"
                    );
                }
                stream->Printf(
                    "<TD>%s</TD>\n"
                    "</TR>\n",mdr->Destinations()
                );
            }
            return;
        }
    }

};


/** Simply list the content of a GCReferenceContainer*/
class SMPageBuilder:public IteratorT<GCReference> {
    /** where to write to */
    StreamInterface *   stream;

    GCRTemplate<StateMachineState>  currentState;


public:
    /** @param s = NULL the code will use printf to write to the console */
    SMPageBuilder(StreamInterface *s,GCRTemplate<StateMachineState> currentState){
        stream = s;
        this->currentState = currentState;
    }

    /** actual function */
    virtual void Do(GCReference data){
        if (!stream) return;
        GCRTemplate<StateMachineState> sms;
        const char * name;
        sms = data;
        if (sms.IsValid()){
            name = sms->Name();
            if (sms == currentState){
                {
                    HtmlStream hmStream(*stream);
                    hmStream.SSPrintf(HtmlTagStreamMode,"TR");
                    hmStream.SSPrintf(HtmlTagStreamMode,"TD");
                    hmStream.SSPrintf(ColourStreamMode,"%i %i",Red,White);
                    hmStream.Printf("%s",name);
                    hmStream.SSPrintf(HtmlTagStreamMode,"/TD");
                    hmStream.SSPrintf(HtmlTagStreamMode,"/TR");
                }
                SMSPageBuilder smspb(stream);
                sms->Iterate(&smspb);
            } else {
                stream->Printf(
                    "<TR>\n"
                    "<TD>%s</TD></TR>\n",name);
                SMSPageBuilder smspb(stream);
                sms->Iterate(&smspb);
            }
            return;
        }
    }

};

/*

<head><TITLE></TITLE>
<SCRIPT LANGUAGE="JavaScript" FOR="window" EVENTS="onload()">
<!--
       window.setTimeout("window.location.reload()", 10000 )
//-->
</SCRIPT></HEAD>

<FORM NAME="MainForm">
<SCRIPT LANGUAGE="JavaScript" FOR="window" EVENTS="onload()">
<!--
    if (parent.frames['BarFrame'] != null){
        parent.frames['BarFrame'].document.BarForm.MAINSTATUS.value = 'LIST';
        parent.frames['BarFrame'].document.BarForm.submit();
    }
//-->
</SCRIPT>
<INPUT TYPE="hidden" NAME= "OID" VALUE=322>
<INPUT TYPE="hidden" NAME= "UID" VALUE=10001>
<INPUT TYPE="hidden" NAME= "ACTION" VALUE=NONE>
<INPUT TYPE="hidden" NAME= "STATUS" VALUE=LIST>
<INPUT TYPE="hidden" NAME= "OID2ADD" VALUE=0>
<INPUT TYPE="hidden" NAME= "UID2ADD" VALUE=0>
<INPUT TYPE="hidden" NAME= "SIG2ADD" VALUE=>
<INPUT TYPE="hidden" NAME= "TT" VALUE=0>
<SCRIPT>
    function DisplaySignalInfo(signal) {
       window.open(signal, 'DISPLAY_SIGNAL','status,resizable,scrollbars')
    }
</SCRIPT>
<FONT SIZE=1><IMG BORDER=0 src="/RES/USERSEL.gif" ><BR></FONT>

<TABLE BORDER=3 CELLSPACING=0 CELLPADDING=0>
<TR bgcolor = "#C0C0C0">
<TD><FONT color="#608020" >Signal Name</FONT> <TD><FONT color="#608020" >size</FONT> <TD><FONT color="#608020" >cal0</FONT> <TD><FONT color="#608020" >cal1</FONT> <TD><FONT color="#608020" >row</FONT> <TD><FONT color="#608020" >col</FONT> <TD><FONT color="#608020" >T</FONT><TD><FONT color="#608020" >D</FONT><TD><B><FONT color="#30FF30" >SEL</FONT><TD><B><FONT color="#FF3030" >DEL</FONT></FONT></TR>

</TABLE>
</FORM>
----CDBStringDataNode Class = <INPUT type="text" name="+SECURITY.Class"  value="GCReferenceContainer" ONCHANGE=ChangeValue("+SECURITY.Class",this.value);><BR>

   function ChangeValue(parameter,value){
       window.location.replace("./?CDBBMSelector=|+SECURITY|&CDBBMParameter=" + parameter + "&CDBBMValue=" + value);
       return true;
   }
<script type="text/javascript">
   function ChangeValue(parameter,value){
       window.location.replace("./?CDBBMSelector=|+SECURITY|&CDBBMParameter=" + parameter + "&CDBBMValue=" + value);
       return true;
   }
   function Expand(expand){
       window.location.replace("./?CDBBMSelector=" + expand);
       return true;
   }
</script>

*/


bool SMProcessHttpMessage(StateMachine &sm,HttpStream &hStream){

    FString displayMode;
    if (hStream.Switch("InputCommands.DisplayMode")){
        hStream.Seek(0);
        hStream.GetToken(displayMode,"");
        hStream.Switch((uint32)0);
    }
    int refreshDelay = 100000;
    if (hStream.Switch("InputCommands.RefreshDelay")){
        hStream.Seek(0);
        FString token;
        hStream.GetToken(token,"");
        refreshDelay = atoi (token.Buffer());
        if (refreshDelay < 2000) refreshDelay = 2000;
        hStream.Switch((uint32)0);
    }
    FString statusChangeRequest;
    if (hStream.Switch("InputCommands.StatusChangeRequest")){
        hStream.Seek(0);
        hStream.GetToken(statusChangeRequest,"");
        hStream.Switch((uint32)0);
    }    

    // the sub-components are addressed by the server directly
//    const char *address = hStream.unMatchedUrl.Buffer();
//    if (strcmp(address,"SIMPLE")==0){
      if (displayMode == "SIMPLE"){
        hStream.Printf("<html><HEAD>\n");
        hStream.Printf("<TITLE>%s</TITLE>\n",sm.Comment());
        hStream.Printf("<SCRIPT type=\"text/javascript\">\n");
        hStream.Printf("   function PageChange(mode,delay){ \n");
        hStream.Printf("       window.location.replace(\"./?DisplayMode=\" + mode + \"&RefreshDelay=\" + delay); \n");
        hStream.Printf("       return true; \n");
        hStream.Printf("   } \n");
        hStream.Printf("</SCRIPT>\n");
        hStream.Printf("<SCRIPT LANGUAGE=\"JavaScript\" FOR=\"window\" EVENTS=\"onload()\">\n");
        hStream.Printf("<!--\n");
        hStream.Printf("       window.setTimeout(\"window.location.reload()\", %i ) \n",refreshDelay);
        hStream.Printf("//-->\n");
        hStream.Printf("</SCRIPT>\n");

        hStream.Printf("</HEAD><BODY BGCOLOR=\"#ffffff\"><H1>%s : ",sm.Comment());
        { // start a frame so that the hmStream is flush at end of frame
            HtmlStream hmStream(hStream);
            hmStream.SSPrintf(ColourStreamMode,"%i %i",Red,White);
            hmStream.Printf("%s",sm.currentState->Name());
        }
        hStream.Printf("</BODY>\n");
        hStream.Printf("<FORM NAME=\"%s.MainForm\" method=\"GET\">\n",sm.Name());
        hStream.Printf("<INPUT type=\"button\" name=\"DisplayMode\" value=\"FULL\" ONCLICK=PageChange(\"FULL\",%i)></BUTTON>\n",refreshDelay);
        hStream.Printf("<INPUT type=\"text\" name=\"RefreshDelay\"  value=\"%i\" ONCHANGE=PageChange(\"%s\",this.value);></BUTTON>\n",refreshDelay,displayMode.Buffer());
        hStream.Printf("</FORM>\n");
//    } else
//    if ((address != NULL) && (address[0] !=0)) {
//        return False;
    } else {

        if(statusChangeRequest.Size() > 0){
            //sm.ChangeState(statusChangeRequest.Buffer());
            FString eventLocation;
            eventLocation.Printf("%s.%s", sm.currentState->Name(), statusChangeRequest.Buffer());
            GCRTemplate<StateMachineEvent> sme = sm.Find(eventLocation.Buffer());
            if(sme.IsValid()){
                GCRTemplate<MessageEnvelope> envelope(GCFT_Create);
                GCRTemplate<Message>         msg(GCFT_Create);
                msg->Init(sme->Code(),sme->NextState());
                envelope->PrepareMessageEnvelope(msg, "StateMachine");
                sm.ProcessMessage(envelope);
            }
        }

        SMPageBuilder smpb(&hStream,sm.currentState);

        hStream.Printf("<html><head><TITLE>%s</TITLE>"
                      "</head><BODY BGCOLOR=\"#ffffff\"><H1>%s</H1><UL>",sm.Comment(),sm.Comment());
        hStream.Printf("<FORM NAME=\"%s.MainForm\">\n",sm.Name());
        hStream.Printf("<BUTTON type=\"submit\" name=\"DisplayMode\" value=\"SIMPLE\">Compact View</BUTTON>\n");
        hStream.Printf("</FORM>\n");
        hStream.Printf("<TABLE border=2>\n");
        hStream.Printf("<TR><TD>STATE</TD><TD>EVENT</TD><TD>ACTION</TD>"
                           "<TD>CODE</TD><TD>VALUE</TD><TD>NEXT/DEST</TD><TD>ERROR</TD></TR>\n");
        sm.Iterate(&smpb);
        hStream.Printf("</TABLE>\n");
        hStream.Printf("</UL></BODY>\n");
    }

    hStream.SSPrintf("OutputHttpOtions.Content-Type","text/html");
    //copy to the client
    hStream.WriteReplyHeader(True);

    return True;
}

