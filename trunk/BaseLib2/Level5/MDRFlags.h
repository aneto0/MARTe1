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

/** 
 * @file
 * Flags to control MessageEnvelope and MessageDeliveryRequest
 */
#ifndef _MESSAGE_DELIVERY_REQUEST_FLAGS
#define _MESSAGE_DELIVERY_REQUEST_FLAGS

class MDRFlags {
public:
    /** */
    int flags;

#if !defined(_CINT)

    /** */
    MDRFlags(int flags = 0){
        this->flags = flags;
    }

    /** */
#endif

};

#if !defined(_CINT)


/** */
static inline MDRFlags operator&(MDRFlags a,MDRFlags b){
    return MDRFlags(a.flags & b.flags);
}

/** */
static inline MDRFlags operator|(MDRFlags a,MDRFlags b){
    return MDRFlags(a.flags | b.flags);
}

/** */
static inline bool operator==(MDRFlags a,MDRFlags b){
    return (a.flags == b.flags);
}

/** */
static inline bool operator!=(MDRFlags a,MDRFlags b){
    return (a.flags != b.flags);
}

/** */
static inline bool operator==(MDRFlags a,int x){
    return (a.flags == x);
}

/** */
static inline bool operator!=(MDRFlags a,int x){
    return (a.flags != x);
}


class MessageDeliveryRequest;
class MessageDispatcher;

/** no flags */
static const MDRFlags MDRF_None(0x0);

/** no flags */
static const MDRFlags MDRF_ReplyMask(0x3);

/** no flags */
static const MDRFlags MDRF_ReplyNMask(0xFFFFFFFC);

/** Create a reply soon before or after user overridden MessageHandler::ProcessMessage */
static const MDRFlags MDRF_AutomaticReply(0x1);

/** Create a reply after or as part of user overridden MessageHandler::ProcessMessage */
static const MDRFlags MDRF_LateReply(0x2);

/** Create a reply soon before user overridden MessageHandler::ProcessMessage */
static const MDRFlags MDRF_EarlyAutomaticReply(0x1);

/** Create a reply soon after user overridden MessageHandler::ProcessMessage */
static const MDRFlags MDRF_LateAutomaticReply(0x3);

/** No reply generated or expected */
static const MDRFlags MDRF_NoReply(0x0);

/** Manual reply expected */
static const MDRFlags MDRF_ManualReply(0x2);

/** Mask to check for any form of replya expected */
static const MDRFlags MDRF_ReplyExpected(0x3);

/** allows using partialName in address */
static const MDRFlags MDRF_MatchPartialName(0x4);

/** This is a reply with no new message */
static const MDRFlags MDRF_Reply(0x10000);

/** */
static MDRFlags MDRFFromString(const char *s){
    if (s == NULL) return MDRF_NoReply;
    if (s[0] == 0) return MDRF_NoReply;
    
    if (strcmp(s,"NoReply")==0) return MDRF_NoReply; 
    if (strcmp(s,"EarlyAutomaticReply")==0) return MDRF_EarlyAutomaticReply; 
    if (strcmp(s,"LateAutomaticReply")==0) return MDRF_LateAutomaticReply; 
    if (strcmp(s,"ManualReply")==0) return MDRF_ManualReply; 

    return MDRF_NoReply;
}

/** */
static const char * MDRFToString(MDRFlags m){

    MDRFlags mdrf = (m & MDRF_ReplyMask) ;
    if (mdrf == MDRF_NoReply) return "NoReply"; 
    if (mdrf == MDRF_EarlyAutomaticReply) return "EarlyAutomaticReply"; 
    if (mdrf == MDRF_LateAutomaticReply) return "LateAutomaticReply"; 
    if (mdrf == MDRF_ManualReply) return "ManualReply"; 

    return "NoReply";
}


#endif  //CINT

#endif

