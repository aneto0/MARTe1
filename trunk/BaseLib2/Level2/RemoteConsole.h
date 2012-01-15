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
 * Implements a console that can be accessed remotely
 */
#if !defined (REMOTE_CONSOLE_)
#define REMOTE_CONSOLE_

#include "System.h"
#include "Console.h"
#include "MutexSem.h"
#include "EventSem.h"

class TCPSocket;
class RemoteConsole;


extern "C" {
    /** */
    void RemoteConsoleInit(RemoteConsole &con);

    /** */
    void RemoteConsoleFinish(RemoteConsole &con);

    /** */
    bool RemoteConsoleWrite(RemoteConsole &con,const void* buffer, uint32 &size,TimeoutType msecTimeout);

    /** */
    bool RemoteConsoleRead(RemoteConsole &con,void* buffer, uint32 &size,TimeoutType msecTimeout);

    /** */
    void __thread_decl RCConsoleThread(void *con);

    /** */
    void __thread_decl RCRemoteThread(void *con);

    /** */
    void RemoteConsoleClient(const char *server="localhost",uint32 port=32769);
}

OBJECT_DLL(RemoteConsole)

/** simulates a Console for all purposes but allows a remote connection to take over */
class RemoteConsole: public Console {
OBJECT_DLL_STUFF(RemoteConsole)

    friend void RemoteConsoleInit(RemoteConsole &con);
    friend void RemoteConsoleFinish(RemoteConsole &con);
    friend void  __thread_decl RCConsoleThread(void *con);
    friend void  __thread_decl RCRemoteThread(void *con);
    friend bool RemoteConsoleWrite(RemoteConsole &con,const void* buffer, uint32 &size,TimeoutType msecTimeout);
    friend bool RemoteConsoleRead(RemoteConsole &con,void* buffer, uint32 &size,TimeoutType msecTimeout);

private:
    /** the port from which messages are read.  */
    int         listeningPort;

    /** */
    TID         consoleThread;

    /** */
    TID         remoteConnectionThread;

    /** */
    MutexSem    resourceLock;

    /** */
    EventSem    newMessage;

    /** */
    bool        connected;

    /** */
    TCPSocket   *connectedSocket;

    /** */
    char        *bufferPtr;

    /** */
    uint32      bufferReadSize;

    /** */
    EventSem    bufferRequest;

    /** */
    bool        hasToContinue;

public:
    /** */
    RemoteConsole(int listeningPort = 32769){
        this->listeningPort = listeningPort;
        RemoteConsoleInit(*this);
    }

    /** */
    virtual ~RemoteConsole(){
        RemoteConsoleFinish(*this);
    }

    /** Uses RemoteConsoleRead. */
    virtual bool        SSRead(
                            void*               buffer,
                            uint32 &            size,
                            TimeoutType         msecTimeout     = TTDefault)
    {
        return RemoteConsoleRead(*this,buffer,size,msecTimeout);
    }


    /** Uses RemoteConsoleWrite. */
    virtual bool        SSWrite(
                            const void*         buffer,
                            uint32 &            size,
                            TimeoutType         msecTimeout     = TTDefault)
    {
        if (connected) return RemoteConsoleWrite(*this,buffer,size,msecTimeout);
        return Console::SSWrite(buffer,size,msecTimeout);
    }


    /** */
    virtual bool CanWrite(){
        return True;
    }

    /** */
    virtual bool CanRead(){
        return True;
    }

    /** */
    virtual bool Switch(uint32 selectedStream){
        if (connected) {
            return Printf("\001\002%i\n",selectedStream);
        }
        return Console::Switch(selectedStream);
    }

    /** */
    virtual bool Switch(const char *name){
        if (connected) {
            return Printf("\001\003%s\n",name);
        }
        return Console::Switch(name);
    }

    /**  */
    virtual uint32 NOfStreams(){
        return Console::NOfStreams();
    }
};

#endif
