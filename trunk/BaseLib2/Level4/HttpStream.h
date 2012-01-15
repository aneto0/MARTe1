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
 * A stream to handle HTTP communication
 */

#if !defined (HTTP_STREAM_H)
#define HTTP_STREAM_H

#include "System.h"
#include "GCNamedObject.h"
#include "StreamConfigurationDataBase.h"
#include "EventSem.h"
#include "GCRTemplate.h"
#include "HRT.h"
#include "HttpDefinitions.h"
#include "HttpRealm.h"

class HttpStream;

extern "C" {

    /** begins a Http message: sends out the HTTPHeader as currently formed
        and the outstanding body
        any further write will operate directly on the stream
        if @param complete is True then the whole body has been cached
        and therefore we can announce the body size
        If command is a reply we are generating a reply message!*/
    bool                HSWriteHeader(
                            HttpStream          &hs,
                            bool                bodyCompleted,
                            HSHttpCommand       command,
                            const char *        url);

}


/** implements a multi stream
    the main stream read and write channels operate with the client
       (or on a cache until CompleteReadOperation() is not called)
    the other streams allow read/write access to special information like
    HttpOptions or HTML form commands
    The http Options are accessed using the streams whose name begins with InputHttpOtions. or OutputHttpOptions.
    InputCommands.* contain the informations produced by submitting a form
    The stream called Peer contains the name of the connecting peer
*/
OBJECT_DLL(HttpStream)
class HttpStream: public GCNamedObject, virtual public StreamConfigurationDataBase {
    // stuff need for all the classes derived from ErrorSystem
    OBJECT_DLL_STUFF(HttpStream)

friend bool             HSWriteHeader(
                            HttpStream          &hs,
                            bool                bodyCompleted,
                            HSHttpCommand       command,
                            const char *        url);

    /** private */
    bool               _WriteHeader(
                            bool                bodyCompleted,
                            HSHttpCommand       command,
                            const char *        url);

private:
    /**
     * Handles a post request
     */
    bool handlePost(const FString &contentType, FString &content);

protected:

    /** the part of the reply that is yet to be sent*/
    FString                 unsentReplyBody;

    /** the stream connected to the remote client
        Should implement a timeout for read operation and
        translation for specific HTTP % hex char groups */
    Streamable *            clientStream;

    /** to wait for body completed */
    EventSem                bodyCompletedEvent;


public:
    /** possible values are writeToString writeToClient writeToCDB  */
    HSOperatingMode         operationMode;

    /** what command was requested
        This affects the working of Read and Write
        For instance Write on the main channel does not work for Http HEAD
        It can also be the reply, in which case it contains the reply code */
    HSHttpCommand           httpCommand;

    /** 1000 means v1.0 2100 means v2.1*/
    uint32                  httpVersion;

    /** True if communication should continue after transaction */
    bool                    keepAlive;

    /** The requested page URL  */
    FString                 url;

    /** The URL with . instead of \/  */
    FString                 path;

    /** The remainder of url not matched in the search */
    FString                 unMatchedUrl;

    /** The Http return code */
    uint32                  httpErrorCode;

    /** The last time the body has been updated */
    int64                   lastUpdateTime;

    /** How much data is still waiting in the input stream from the client */
    int32                   unreadInput;
public:
    /** initialise to empty */
                        HttpStream(Streamable *clientStream);

    /** */
    virtual             ~HttpStream(){}

    /** Reads Header and leaves body up to the user  */
    bool                ReadHeader();

    /** begins a Http reply message: sends out the HTTPHeader as currently formed
        and the outstanding body
        any further write will operate directly on the stream
        if @param complete is True then the whole body has been cached
        and therefore we can announce the body size !*/
    bool                WriteReplyHeader(
                            bool                bodyCompleted,
                            uint32              httpErrorCode   =   200)
    {
        return WriteHeader(bodyCompleted,GenerateReplyCode(httpErrorCode),NULL);
    }

    /** begins a Http message: sends out the HTTPHeader as currently formed
        and the outstanding body
        any further write will operate directly on the stream
        if @param complete is True then the whole body has been cached
        and therefore we can announce the body size
        If command is a reply we are generating a reply message!*/
    inline bool         WriteHeader(
                            bool                bodyCompleted,
                            HSHttpCommand       command,
                            const char *        url)
    {
        return HSWriteHeader(*this,bodyCompleted,command,url);
    }

    /** Flag that the writing of the body has been completed */
    bool                BodyCompleted();

    /** the sender thread waits for the completion of this activity */
    bool                WaitForBodyCompleted(
                            TimeoutType         msecTimeout);

    /** to check a request against a security module */
    bool                SecurityCheck(GCRTemplate<HttpRealm> realm,uint32 ipNumber);

    /** to complete the reading of a Http message
        throwing away the content */
    bool                CompleteReadOperation(
                            Streamable *        s               =   NULL,
                            TimeoutType         msecTimeout     =   TTInfiniteWait);

    /** how long since the last write operation */
    double              IdleTime();

    /** @name Simple Streams INTERFACE
        @{ */

    /** Maps into Read  */
    virtual bool        SSRead(
                            void*               buffer,
                            uint32 &            size,
                            TimeoutType         msecTimeout     =   TTDefault);

    /** Maps into Write */
    virtual bool        SSWrite(
                            const void*         buffer,
                            uint32 &            size,
                            TimeoutType         msecTimeout     =   TTDefault);

    /** can we read from it. When in the final write to client mode we cannot!*/
    virtual bool        CanRead()
    {
        if (operationMode == HSOMCompleted) return False;
        if (operationMode == HSOMWriteToClient) return False;
        return True;
    }

    /** we can always write*/
    virtual bool        CanWrite()
    {
        if (operationMode == HSOMCompleted) return False;
        return True;
    }

    // RANDOM ACCESS INTERFACE

    /** The size of the stream
        Note that we cannot read back the size of the actual client stream */
    virtual int64       Size();

    /** Move within the stream
        Note that we cannot move on the actual client stream*/
    virtual bool        Seek(int64 pos) ;

    /** Position within the stream
        Note that we cannot get position of the actual client stream */
    virtual int64       Position();

    /**  Clip the stream size to a specified point
        Note that we cannot do this on the actual client stream */
    virtual bool        SetSize(int64 size);

    /** when on write to client we cannot seek !*/
    virtual bool        CanSeek();

    /** @} */

    /** @name Extended Attributes or Multiple Streams INTERFACE
        @{ */

    /** how many streams are available in total.
        counts from root all the leaves     */
    virtual uint32      NumberOfStreams();

    /** select the stream to read from. Switching may reset the stream to the start. */
    virtual bool        Switch(uint32 n);

    /** select the stream to read from. Switching may reset the stream to the start. */
    virtual bool        Switch(const char *name);

    /** which is the selected stream */
    virtual uint32      SelectedStream();

    /** add a new stream to write to. NB selectedStream will not be up to date.... */
    virtual bool        AddStream(const char *name);

    /** remove an existing stream . */
    virtual bool        RemoveStream(const char *name);

    /**
     * @return the number of InputCommands encoded in the stream (the query string usually sent as a GET)
     */
    int32               NumberOfInputCommands();

    /**
     * @param idx the index of the InputCommands
     * @param name if found the name will be stored here
     * @return True if the InputCommand for the position idx exists
     */ 
    bool                InputCommandName(FString &name, int32 idx);

    /**
     * @param idx the index of the InputCommands 
     * @param value if found the name will be stored here
     * @return True if the InputCommand for the position idx exists
     */
    bool               InputCommandValue(FString &value, int32 idx);

    /**
     * @param value if found the value will be stored here
     * @param name the name of the InputCommands 
     * @return True if the InputCommand for the position idx exists
     */
    bool InputCommandValue(FString &value, const char *name);
};

#endif

