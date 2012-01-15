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

#include "MessageBroker.h"
#include "CDBExtended.h"

OBJECTLOADREGISTER(MessageBroker,"$Id$")

bool MBProcessMessage2(
            MessageBroker&                  mb,
            GCRTemplate<MessageEnvelope>    envelope,
            const char *                    subAddress){

    // fancy that, I am getting personal mail!
/*    if (subAddress == NULL){
        mb.AssertErrorCondition(FatalError, "While I appreciate receiving mails, you really should not address me!..");
        return False;
    }*/

    if (!envelope.IsValid()){
        mb.AssertErrorCondition(FatalError, "not valid Envelope!!!");
        return False;
    }

    // use mirrored address or build relative address
    if (mb.peerAddressOffset.Size() > 0){
        FString remappedAddress;
        if(subAddress != NULL){
            remappedAddress.Printf("%s.%s",mb.peerAddressOffset.Buffer(),subAddress);
        }
        else{
            remappedAddress = mb.peerAddressOffset.Buffer();
        }
        envelope->SetDestination(remappedAddress.Buffer());
    }

    return MHSendMessageRemotely(envelope,mb.peerIpAddress.Buffer(),mb.peerPort);


}

/** initialise object */
bool
MBObjectLoadSetup(MessageBroker &mb,ConfigurationDataBase &info,StreamInterface *err){

    bool ret = mb.GCNamedObject::ObjectLoadSetup(info,err);

    CDBExtended cdbx(info);

    bool ret1 = cdbx.ReadBString(mb.peerIpAddress,"PeerIpAddress","");
    if (!ret1){
        mb.AssertErrorCondition(ParametersError,"ObjectLoadSetup: PeerIpAddress must be specified");
        ret = False;
    }


    ret1 = cdbx.ReadBString(mb.peerAddressOffset,"PeerAddressOffset","");
    if (!ret1){
        mb.AssertErrorCondition(ParametersError,"ObjectLoadSetup: PeerAddressOffset is assumed to empty");
    }


    ret1 = cdbx.ReadInt32 (mb.peerPort,"PeerPort",0);
    if (!ret1){
        mb.AssertErrorCondition(ParametersError,"ObjectLoadSetup: PeerPort must be specified");
        ret = False;
    }

    return ret;

}

/** download settings */
bool
MBObjectSaveSetup(MessageBroker &mb,ConfigurationDataBase &info,StreamInterface *err){

    mb.GCNamedObject::ObjectSaveSetup(info,err);

    CDBExtended cdbx(info);

    cdbx.WriteString(mb.peerAddressOffset.Buffer(),"PeerAddressOffset");

    cdbx.WriteString(mb.peerIpAddress.Buffer(),"PeerIpAddress");

    cdbx.WriteInt32 (mb.peerPort,"PeerPort");

    return True;

}

