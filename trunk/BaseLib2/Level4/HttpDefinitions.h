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
 * HTTP definitions
 */

#if !defined (HTTP_DEFINITIONS_H)
#define HTTP_DEFINITIONS_H

#include "System.h"

/** read while available (no content length specified)*/
enum HSReadMode {
    HTTPNoContentLengthSpecified = -2
};

/** the status of HttpStream */
enum HSOperatingMode {
/** operates on the string both read and write */
    HSOMWriteToString  = 0x0,

/** final mode: only write, and use the client direct stream */
    HSOMWriteToClient  = 0x1,

/** operates read and write with CDB  */
    HSOMWriteToCDB     = 0x2,

/** no more reading or writing   */
    HSOMCompleted      = 0x4
};

/** the command requested via HTTP */
enum HSHttpCommand {
/** none */
    HSHCNone = 0,

/** HTTP GET */
    HSHCGet  = 1,

/** HTTP PUT */
    HSHCPut  = 2,

/** HTTP POST */
    HSHCPost = 3,

/** HTTP HEAD */
    HSHCHead = 4,

/** HTTP REPLY */
    HSHCReply = 0x10000000,

/** HTTP REPLY OK*/
    HSHCReplyOK = HSHCReply + 200,

/** HTTP REPLY AUTH REQUIURED*/
    HSHCReplyAUTH = HSHCReply + 401

};

/** create a HSHttpCommand relative toa reply with a specfic ErrorCode */
static inline HSHttpCommand GenerateReplyCode(uint32 httpErrorCode){
    return  (HSHttpCommand) (httpErrorCode + (uint32)HSHCReply);
}

/** create a HSHttpCommand relative toa reply with a specfic ErrorCode */
static inline bool IsReplyCode(HSHttpCommand hshc,uint32 &httpErrorCode){
    if (hshc < HSHCReply) return False;
    httpErrorCode = (uint32)hshc - (uint32)HSHCReply;
    return True;
}

/**
 * @return the HTTP string corresponding to the error code
 */
static inline const char *GetErrorCodeString(int32 httpErrorCode){
    switch(httpErrorCode){
        case 200:
            return "OK";
        case 201:
            return "CREATED";
        case 202:
            return "Accepted";
        case 203:
            return "Partial Information";
        case 204:
            return "No Response";
        case 400:
            return "Bad request";
        case 401:
            return "Unauthorized";
        case 402:
            return "PaymentRequired";
        case 403:
            return "Forbidden";
        case 404:
            return "Not found";
        case 500:
            return "Internal Error";
        case 501:
            return "Not implemented";
        case 301:
            return "Moved";
        case 302:
            return "Found";
        case 303:
            return "Method";
    }
    return "Unknown code";
}

#endif

