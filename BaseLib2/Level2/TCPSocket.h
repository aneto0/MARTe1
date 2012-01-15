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

#if !defined( TCP_SOCKET_H)
#define TCP_SOCKET_H

#include "System.h"
#include "BasicTCPSocket.h"
#include "SocketSelect.h"
#include "InternetService.h"
#include "Streamable.h"
#include "SocketTimer.h"

#define TCPSocketVersion "$Id: TCPSocket.h,v 1.8 2007/09/12 14:22:36 fpiccolo Exp $"

class TCPSocket;

OBJECT_DLL(TCPSocket)

/** 
 * @file
 * Implements TCP stream-sockets basics
 */
class TCPSocket: public Streamable, public BasicTCPSocket {
OBJECT_DLL_STUFF(TCPSocket)

public:

    // PURE STREAMING

    /** Maps BasicTCP into Streams
        if msecTimeout == NoWait then the behaviour depends on the blocking or not blocking
        otherwise a SocketTimer will be used */
    virtual bool        SSRead(
                            void*               buffer,
                            uint32 &            size,
                            TimeoutType         msecTimeout     = TTDefault)
    {
        if (msecTimeout == TTDefault ) return BasicRead(buffer,size);
        SocketTimer st;
        BasicTCPSocket *bts = this;
        st.AddWaitOnReadReady(bts);
        st.SetMsecTimeout(msecTimeout);
        if (st.WaitRead()){
            return BasicRead(buffer,size);
        } else return False;
    }


    /** Maps BasicTCP into Streams*/
    virtual bool        SSWrite(
                            const void*         buffer,
                            uint32 &            size,
                            TimeoutType         msecTimeout     = TTDefault)
    {
        if (msecTimeout == TTDefault ) return BasicWrite(buffer,size);
        SocketTimer st;
        BasicTCPSocket *bts = this;
        st.AddWaitOnWriteReady(bts);
        st.SetMsecTimeout(msecTimeout);
        if (st.WaitRead()){
            return BasicWrite(buffer,size);
        } else return False;
    }

    /** YES */
    virtual bool CanRead(){
        return True;
    }

    /** obviously  YES*/
    virtual bool CanWrite(){
        return True;
    }

    /** maps WaitConnection */
    TCPSocket *WaitConnection(TimeoutType msecTimeout = TTInfiniteWait,TCPSocket *client = NULL){
        TCPSocket *client2 = client;
        if (!client) client2 = new TCPSocket();
        if (BasicTCPSocket::WaitConnection(msecTimeout,client2)){
            return client2;
        }
        if (!client) delete client2;
        return NULL;
    }

};

#endif













