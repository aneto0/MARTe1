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
#include "SyslogRelay.h"
#include "Sleep.h"

OBJECTLOADREGISTER(SyslogRelay, "$Id$")

/** This function will be called whenever there is a new message to process available*/
void SyslogRelay::ProcessMessage(GCRTemplate<LoggerMessage> loggerMsg){
    //I hope it is not a problem to call openlog. I believe it only triggers an update of the ident...
    FString ident = loggerMsg->GetSourceAddress();
    openlog(ident.Buffer(), LOG_NDELAY, LOG_USER);
    FString msg;
    loggerMsg->formattedMsgFields = 0xffff;
    loggerMsg->formattedMsgFields &= ~LM_SOURCE_ADDR;
    loggerMsg->formattedMsgFields &= ~LM_ERROR_TIME;
    loggerMsg->FormatMessage(msg);

    int32 syslogErrorCode = LOG_EMERG;
    switch(loggerMsg->GetErrorCode()){
        case Information          :  syslogErrorCode = LOG_INFO    ; break;
        case Warning              :  syslogErrorCode = LOG_WARNING ; break;
        case FatalError           :  syslogErrorCode = LOG_CRIT    ; break;
        case RecoverableError     :  syslogErrorCode = LOG_ERR     ; break;
        //case InitialisationError  :  syslogErrorCode = LOG_EMERG   ; break;
        case InitialisationError  :  syslogErrorCode = LOG_CRIT    ; break;
        case OSError              :  syslogErrorCode = LOG_CRIT    ; break;
        case ParametersError      :  syslogErrorCode = LOG_ERR     ; break;
        case IllegalOperation     :  syslogErrorCode = LOG_ERR     ; break;
        case ErrorSharing         :  syslogErrorCode = LOG_ERR     ; break;
        case ErrorAccessDenied    :  syslogErrorCode = LOG_ERR     ; break;
        case Exception            :  syslogErrorCode = LOG_CRIT    ; break;
        case Timeout              :  syslogErrorCode = LOG_ERR     ; break;
        case CommunicationError   :  syslogErrorCode = LOG_ERR     ; break;
        //case SyntaxError          :  syslogErrorCode = LOG_EMERG   ; break;
        case SyntaxError          :  syslogErrorCode = LOG_CRIT    ; break;
        case Debug                :  syslogErrorCode = LOG_DEBUG   ; break;
    };
    syslog(syslogErrorCode, "%s", msg.Buffer());
}

