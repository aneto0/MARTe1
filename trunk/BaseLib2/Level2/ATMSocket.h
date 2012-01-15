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
 * ATMSocket Streamable 
 */
#if !defined(ATM_SOCKET_H)
#define ATM_SOCKET_H

#include "BasicATMSocket.h"
#include "Streamable.h"

OBJECT_DLL(ATMSocket)

/** Implements ATM sockets AAL5 */
class ATMSocket: public Streamable, public BasicATMSocket {
OBJECT_DLL_STUFF(ATMSocket)

public:
    /** constructor */
                        ATMSocket(int32 socket = 0)
    {
        connectionSocket = socket;
    }

    /** destructor */
                        ~ATMSocket()
    {
        Close();
    }

private:

    /** Reads a block of data: size is the maximum size. on return size is what was read
        timeout not supported yet */
    virtual bool        SSRead(
                            void*               buffer,
                            uint32 &            size,
                            TimeoutType         msecTimeout     = TTDefault)
    {
        return BasicATMSocket::Read(buffer,size,msecTimeout);
    }


    /** Writes a block of data: size is its size. on return size is what was written.
        timeout not supported yet */
    virtual bool        SSWrite(
                            const void*         buffer,
                            uint32 &            size,
                            TimeoutType         msecTimeout     = TTDefault)
    {
        return BasicATMSocket::Write(buffer,size,msecTimeout);
    }
public:
    /** maps to Streamable ambiguous function */
    virtual bool        Read(
                            void*               buffer,
                            uint32 &            size,
                            TimeoutType         msecTimeout     = TTDefault)
    {
        return Streamable::Read(buffer,size,msecTimeout);
    }


    /** maps to Streamable ambiguous function */
    virtual bool        Write(
                            const void*         buffer,
                            uint32 &            size,
                            TimeoutType         msecTimeout     = TTDefault)
    {
        return Streamable::Write(buffer,size,msecTimeout);
    }




};

#endif



