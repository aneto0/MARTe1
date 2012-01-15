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
#if !defined (LOGGER_MESSSAGE)
#define LOGGER_MESSSAGE

#include "System.h"
#include "GCNString.h"

enum LoggerMessageWriteFields{
    LM_ERROR_CODE  = 0x0001,
    LM_ERROR_TIME  = 0x0002,
    LM_PID         = 0x0004,
    LM_TID         = 0x0008,
    LM_OBJECT      = 0x0010,
    LM_OSS_ERR     = 0x0020,
    LM_CLASS_NAME  = 0x0040,
    LM_ERROR_MSG   = 0x0100,
    LM_THREAD_NAME = 0x0200,
    LM_SOURCE_ADDR = 0x0400,
    LM_OS_ERR_MSG  = 0x0800,
    LM_ERROR_NAME  = 0x1000
};

OBJECT_DLL(LoggerMessage)
class LoggerMessage : public GCNamedObject{
OBJECT_DLL_STUFF(LoggerMessage)
private:
    /** The error code*/
    int32                   errorCode;

    /** The error time*/
    uint32                  errorTime;

    /** the process the error originated from 0 means NONE */
    uint32                  pid;

    /** the thread the error originated from */
    uint32                  tid;

    /** the object the error originated from 0 means NONE */
    uint32                  object;

    /** operating system error code*/
    uint32                  osErrorCode;
    
    /** the class the error originated from 0 means NONE */
    FString                 className;

    /** the error message with extra info provided by the user*/
    FString                 errorMessage;

    /** The name of the thread to which this error belongs*/
    FString                 threadName;
    
    /** The address of the source which generated this LoggerMessage*/
    FString                 sourceAddress;
    
    /** A string with the error which was returned by the operating system*/
    FString                 osErrorMessage;

    /**
     * Loads a LoggerMessage encoded with binary format.
     * This method of encoding is deprecated.
     */
    void LoadFromBinary(char *buffer, uint32 size);
       
    /**
     * Loads a LoggerMessage encoded using the text format, which is the preferred
     * way of encoding.     
     */
    bool LoadFromText(char *buffer, uint32 size);
    
        
public:
    /** Bit field containing the fields to output in the formatted message*/
    int32                   formattedMsgFields;
 
    /**
     * Constructs a new LoggerMessage from an encoded format, which is stored in buffer.
     * @param sourceAddress the address of the source which generated this packet
     * @param buffer the encoded message as it is received from another source 
     * (typically an udp port)
     * @param size the size of the buffer
     */
    LoggerMessage(){        
        formattedMsgFields = 0xFFFF;
    }
    
    bool LoadFromMessage(const char *sourceAddress, char *buffer, uint32 size);
    
    virtual ~LoggerMessage(){        
    }
            
    /**
     * Formats a message into the standard BaseLib2 format, using features like
     * color.
     * @param out The stream to write to.
     */
    virtual void FormatMessage(StreamInterface &out);
    
    /**
     * Encodes the message into text format.
     * @param out The stream to write to.
     */
    void EncodeToText(StreamInterface &out);
    
    /**
     * Resets all the values in this Logger Message.
     * @param buffer the encoded message as it is received from another source 
     * (typically an udp port)
     * @param size the size of the buffer
     */
    void Reset(){
        errorCode             = 0;
        errorTime             = 0;
        tid                   = 0;
        pid                   = 0;
        osErrorCode           = 0;
        object                = 0;
        errorMessage.SetSize(0);
        className.SetSize(0);
        threadName.SetSize(0);
        sourceAddress.SetSize(0);
        osErrorMessage.SetSize(0);
    }
    
    /**
     * @return the hostname
     */
    const FString &GetSourceAddress(){
        return sourceAddress;
    }
    
    /**
     * @return the PID
     */
    uint32 GetPID(){
        return pid;
    }

    /**
     * @return the error code
     */
    uint32 GetErrorCode(){
        return errorCode;
    }
};

#endif
