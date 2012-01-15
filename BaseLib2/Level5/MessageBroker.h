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

#if !defined(_MESSAGE_BROKER_)
#define _MESSAGE_BROKER_

/** @file
    A special object that allows to deliver message to a remote system
    */

#include "GCReferenceContainer.h"
#include "MessageDeliveryRequest.h"
#include "GCNamedObject.h"
#include "MessageEnvelope.h"
#include "MessageHandler.h"
#include "BasicUDPSocket.h"


class MessageBroker;

extern "C"{

    /** To handle replies mainly */
    bool                            MBProcessMessage2(MessageBroker &mb,GCRTemplate<MessageEnvelope> envelope,const char *subAddress);

    /** initialise object */
    bool                            MBObjectLoadSetup(MessageBroker &mb,ConfigurationDataBase &info,StreamInterface *err);

    /** download settings */
    bool                            MBObjectSaveSetup(MessageBroker &mb,ConfigurationDataBase &info,StreamInterface *err);
}


OBJECT_DLL(MessageBroker)

/** Allows to send messages to a  remote program where a MessageServer is installed and active */
class MessageBroker: public GCNamedObject, public MessageHandler{

OBJECT_DLL_STUFF(MessageBroker)

    friend  bool        MBProcessMessage2(MessageBroker &mb,GCRTemplate<MessageEnvelope> envelope,const char *subAddress);

    friend  bool        MBObjectLoadSetup(MessageBroker &mb,ConfigurationDataBase &info,StreamInterface *err);

    friend  bool        MBObjectSaveSetup(MessageBroker &mb,ConfigurationDataBase &info,StreamInterface *err);

protected:

    /** attach this address to the subAddress! */
            BString                                     peerAddressOffset;

    /** the machine to send the message to*/
            BString                                     peerIpAddress;

    /** the UDP port to send to */
            int32                                       peerPort;

//    /** the socket used for the communication */
//            BasicUDPSocket                              socket;

public:
    /** constructor */
                                    MessageBroker()
    {
        peerPort = 0;
    }

    /** Destructor */
    virtual                         ~MessageBroker()
    {
//        socket.Close();
    }

    /** the virtual function needed by MessageHandlerInterface
        this function should never be called*/
    virtual bool                    ProcessMessage(
                GCRTemplate<MessageEnvelope>    envelope)
    {
        AssertErrorCondition(FatalError,"ProcessMessage should never have been called");
        return False;
    }

    /** the virtual function needed by MessageHandlerInterface */
    virtual bool                    ProcessMessage2(
                GCRTemplate<MessageEnvelope>    envelope,
                const char *                    subAddress)
    {
        return MBProcessMessage2(*this,envelope,subAddress);
    }

    /** initialise object
        @param PeerAddressOffset can be omitted. if declared changes the address used as
            destination by replacing the part matched so far with this string
        @param PeerIpAddress ip address of peer
        @param PeerPort  peer port number */
    virtual     bool                ObjectLoadSetup(
                ConfigurationDataBase &         info,
                StreamInterface *               err){

        return MBObjectLoadSetup(*this,info,err);
    }

    /** save settings */
    virtual     bool                ObjectSaveSetup(
                ConfigurationDataBase &         info,
                StreamInterface *               err){

        return MBObjectSaveSetup(*this,info,err);
    }


};



#endif
