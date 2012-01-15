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

#if !defined(_MESSAGE_CODE)
#define _MESSAGE_CODE

/** @file
    The codes of the messages. Some already defined codes
*/


#include "System.h"

/** Used to describe the content of a message.
    Just an integer but it enforces type checks.
    User defined MessageCodes start from UserMessageCode  */
class MessageCode{
    /** the actual code */
    int32 code;

#if !defined(_CINT)

public:
    /** to create a MessageCode */
                        MessageCode(int32 code)
    {
        this->code = code;
    }

    /** access the code */
    inline      int32   Code() const
    {
        return code;
    }

    /** compare with MessageCode */
    inline      bool    operator==(const MessageCode x) const
    {
        return code == x.code;
    }

    /** compare with integers */
    inline      bool    operator==(int32 x) const
    {
        return code == x;
    }

    /** compare with MessageCode */
    inline      bool    operator!=(const MessageCode x) const
    {
        return code != x.code;
    }

    /** compare with integers */
    inline      bool    operator!=(int32 x) const
    {
        return code != x;
    }

    /** compare with integers */
    inline      bool    operator>(int32 x) const
    {
        return code > x;
    }

    /** compare with integers */
    inline      bool    operator>(MessageCode x) const
    {
        return code > x.code;
    }

    /** compare with integers */
    inline      bool    operator<(MessageCode x) const
    {
        return code < x.code;
    }

#endif 
};

#if !defined(_CINT)

/** difference */
static inline MessageCode operator-(MessageCode a,MessageCode b)
{
    return MessageCode(a.Code()-b.Code());
}

/** sum */
static inline MessageCode operator+(MessageCode a,MessageCode b)
{
    return MessageCode(a.Code()+b.Code());
}


/** this message is the result of a message rejected by the original recipient */
static const   MessageCode     RejectedMessage(0xFFFFFFFF);

/** An empty message. Used when there is nothing to say  */
static const   MessageCode     NullMessage(0x0);

/** Finished using the data in the original message
    This is the standard reply to fully acknowledge a message */
static const   MessageCode     FinishedMessage(0x1);

/** a message directed to a MenuInterface
    the message payload must contain an OUTPUTSTREAM object
    and possibly an INPUTSTREAM one. Both must be streamable */
static const   MessageCode     MenuMessage(0x1001);

/** a message directed to a HttpInterface */
static const   MessageCode     HttpMessage(0x1002);

/** a message directed to a HttpInterface */
static const   MessageCode     UserMessageCode(0x100000);

/** how many user message codes */
static const   int             MaxUserMessageCode = 0x100000;

#endif // cint

#endif


