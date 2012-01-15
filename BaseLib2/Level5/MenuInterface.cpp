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

/** @file */


#include "MenuInterface.h"
#include "MessageHandler.h"
#include "GCRTemplate.h"
#include "GCReferenceContainer.h"


bool MenuSystemTextMenu(
    MenuInterface &     ms,
    StreamInterface &   in,
    StreamInterface &   out,
    bool                exitImmediately)

{
    if (ms.action != NULL){
        return ms.action(in,out,ms.userData);
    }

    // check for inheritance from a GCReferenceContainer
    // if so implement the menu browsing .
    GCRTemplate<MenuInterface> menuIf(&ms);
    if (!menuIf.IsValid()){
        return False;
    }
    GCRTemplate<GCReferenceContainer> container(menuIf);
    if (!container.IsValid()){
        return False;
    }

    {
        // lock the container during this next pghase */
        container->Lock();

        // copy references to valid MenuSystems to this container
//        GCRContainerTemplate<MenuInterface> menuContainer;
        GCReferenceContainer menuContainer;

        int i;
        for (i=0;i<container->Size();i++){
            GCRTemplate<MenuInterface> menuItem =  container->Find(i);
            if (menuItem.IsValid()) {
                menuContainer.Insert(menuItem);
            }
        }
        // now I operate on the private container so no worries
        container->UnLock();

        if (menuContainer.Size() > 0){
            if (ms.action != NULL){
                return ms.action(in,out,ms.userData);
            }

            if (ms.entryAction != NULL){
                ms.entryAction(in,out,ms.userData);
            }

            // empty input
            char buffer[128];
            uint32 size = sizeof(buffer);
            int bias    = 0;
            while(1){
                out.Printf(
                "###############################################################################\n"
                "##     % 36s                                ##\n"
                "###############################################################################\n"
                ,ms.Title());
                out.Printf("% 37s#\n","");
                int max = 8;
                for (int i = bias; i<max+bias;i++){
                    GCRTemplate<MenuInterface> m1;
                    if (i < menuContainer.Size())       m1 = menuContainer.Find(i);
                    GCRTemplate<MenuInterface> m2;
                    if ((i+max) < menuContainer.Size()) m2 = menuContainer.Find(i+max);
                    char label[32];
                    sprintf(label,"%c",i+'A');
                    if (m1.IsValid()) {
                        if (out.Switch("colour")){
                            out.Printf("%i %i",Red,Black);
                            out.Switch((uint32)0);
                        }
                        out.Printf("%s",label);
                        if (out.Switch("colour")){
                            out.Printf("%i %i",Grey,Black);
                            out.Switch((uint32)0);
                        }
                        if (m1->action){
                            out.Printf(":  % 32s #",m1->Title());
                        } else {
                            out.Printf(":->% 32s #",m1->Title());
                        }
                    }  else out.Printf("% 37s#","");
                    label[0] += max;
                    if (m2.IsValid()) {
                        if (out.Switch("colour")){
                            out.Printf("%i %i",Red,Black);
                            out.Switch((uint32)0);
                        }
                        out.Printf("%s",label);
                        if (out.Switch("colour")){
                            out.Printf("%i %i",Grey,Black);
                            out.Switch((uint32)0);
                        }
                        if (m2->action){
                            out.Printf(":  % 32s     \n",m2->Title());
                        } else {
                            out.Printf(":->% 32s     \n",m2->Title());
                        }
                    } else out.Printf("   % 32s \n","");
                    out.Printf("% 37s#\n","");
                }
                if(menuContainer.Size() > 2*max){
                    out.Printf("###############################################################################\n");
                    if(     bias == 0)               out.Printf("0: EXIT                         >: MOVE DOWN                                   \n");
                    else if(bias > menuContainer.Size()-2*max) out.Printf("0: EXIT         <: MOVE UP                                                     \n");
                    else                             out.Printf("0: EXIT         <: MOVE UP      >: MOVE DOWN                                   \n");
                    out.Printf("###############################################################################\n");
                }else{
                    out.Printf("###############################################################################\n");
                    out.Printf("0: EXIT                                                                        \n");
                    out.Printf("###############################################################################\n");
                }
                if (exitImmediately) return True;
                size = sizeof(buffer);
                if (!in.Read(buffer,size)) {
                    SleepMsec(100);
                } else {
                    if (strlen(buffer)>0){

                        char command = toupper(buffer[0]);
                        switch(command){
                        case '0':{
                            if (ms.exitAction == NULL) return True;
                            return ms.exitAction(in,out,ms.userData);
                        }break;
                        case '<':{
                            if(bias > 0 ) bias -= (2*max);
                        }break;
                        case '>':{
                            if(bias < menuContainer.Size() - max) bias += (2*max);
                        }break;
                        default:{
                            int index = command - 'A';
                            if ((index >= 0) && (index < menuContainer.Size())){

                                GCRTemplate<MenuInterface> m;
                                m = menuContainer.Find(index);
                                m->TextMenu(in,out);
                                if (m->action != NULL){
                                    out.Printf("\nPRESS ENTER TO RETURN TO MENU \n");
                                    in.Read(buffer,size);
                                }
                            }
                        }break;
                        }
                    }
                }
            }
            return True;
        }
    }

    return False;
}

/** The menu handler */
bool MenuInterface::ProcessMenuMessage(
        GCRTemplate<MessageEnvelope>    envelope)
{
//    if (action == NULL) return False;

    if (!envelope.IsValid()){
        CStaticAssertErrorCondition(ParametersError,"MenuMessageHandler: envelope is NULL ");
        return False;
    }

    GCRTemplate<Message> message = envelope->GetMessage();
    if (!message.IsValid()){
        CStaticAssertErrorCondition(ParametersError,"MenuMessageHandler: message is NULL ");
        return False;
    }

    GCRTemplate<StreamInterface> outputStream= message->Find("OUTPUTSTREAM");;
    if (!outputStream.IsValid()){
        CStaticAssertErrorCondition(ParametersError,"MenuMessageHandler: outputStream is missing in message or not a Stream ");
        return False;
    }

    GCRTemplate<StreamInterface> inputStream = message->Find("INPUTSTREAM");
    if (!inputStream.IsValid()){
        inputStream = outputStream;
    }

    MenuSystemTextMenu(*this,*(inputStream.operator->()) ,*(outputStream.operator->()),False);

    GCRTemplate<MessageEnvelope> replyEnvelope(GCFT_Create);
    GCRTemplate<Message> reply(GCFT_Create);
    message->Init(FinishedMessage,"");
    replyEnvelope->PrepareReply(envelope,reply);
    MessageHandler::SendMessage(replyEnvelope);

    return True;
}



