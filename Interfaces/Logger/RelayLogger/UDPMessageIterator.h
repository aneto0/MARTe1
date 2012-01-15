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
#if !defined (UDP_MESSAGE_ITERATOR)
#define UDP_MESSAGE_ITERATOR

#include "ObjectMacros.h"
#include "GCReferenceContainer.h"
#include "StreamInterface.h"
#include "MessageHandler.h"
#include "MessageQueue.h"

class UDPMessageIterator:public IteratorT<GCReference> {

private:
    MessageQueue *messageQueue;
    GCRTemplate<MessageEnvelope> messageEnv;
            
public:
    
    UDPMessageIterator(MessageQueue *messageQueue){
        this->messageQueue = messageQueue;        
    }

    /** actual function */
    virtual void Do(GCReference node){        
        GCRTemplate<GCReferenceContainer> root;
        GCReferenceContainer ref;
        //GCRTemplate<MessageEnvelope> messageEnv = messageQueue->Peek();
                
        if(messageEnv.IsValid() == False || messageEnv->Size() == 0)
            return;
                
        root = node;
        if (!root.IsValid()){            
            return;
        }
                
        if(root->Size() == 0){
            GCRTemplate<MessageHandler> mh = root;
            if(mh.IsValid()){                
                mh->SendMessage(messageEnv);
            }
        }
        else{
            UDPMessageIterator iter(messageQueue);
            root->Iterate(&iter, GCFT_Recurse);
        }
    }
};

#endif
