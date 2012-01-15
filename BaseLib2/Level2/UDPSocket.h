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

#if !defined( UDP_SOCKET_H)
#define UDP_SOCKET_H

#include "BasicUDPSocket.h"
#include "Streamable.h"
#include "SocketTimer.h"


#define UDPSocketVersion "$Id$"

class UDPSocket;

OBJECT_DLL(UDPSocket)

/** 
 * @file
 * Implements UDP socket
 */
class UDPSocket: public Streamable, public BasicUDPSocket {
OBJECT_DLL_STUFF(UDPSocket)

public:

    /** Maps BasicTCP into Streams
        if msecTimeout  == NoWait then the behaviour depends on the blocking or not blocking
        otherwise a SocketTimer will be used */
    virtual bool        SSRead(
                            void*               buffer,
                            uint32 &            size,
                            TimeoutType         msecTimeout     = TTDefault)
    {
        if (msecTimeout == TTDefault ) return BasicRead(buffer,size);
        SocketTimer st;
        BasicUDPSocket *bts = this;
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
        BasicUDPSocket *bts = this;
        st.AddWaitOnWriteReady(bts);
        st.SetMsecTimeout(msecTimeout);
        if (st.WaitRead()){
            return BasicWrite(buffer,size);
        } else return False;
    }

    /** Yest UDP Socket can be read */
    virtual bool CanRead(){
        return True;
    }

    /** YES we can*/
    virtual bool CanWrite(){
        return True;
    }

// class specific functions
    /** a constructor */
    UDPSocket(int32 socket = 0){
        connectionSocket = socket;
    }

};

#endif









