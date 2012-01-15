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

#if !defined(_MESSAGE_)
#define _MESSAGE_

/**
 * @file
 * The message to be exchanged among objects
 * It is also an object container
 */
#include "GCNamedObject.h"
#include "BString.h"
#include "GCReferenceContainer.h"
#include "MessageCode.h"

class Message;

OBJECT_DLL(Message)

extern "C"{

    bool MSGObjectLoadSetup(Message &msg,ConfigurationDataBase &info,StreamInterface *err);

    bool MSGObjectSaveSetup(Message &msg,ConfigurationDataBase &info,StreamInterface *err);

    uint32 MSGGetUniqueId();
}

/** it is a code a string and an object reference */
class Message: public GCReferenceContainer{
OBJECT_DLL_STUFF(Message)

    friend bool MSGObjectLoadSetup(Message &msg,ConfigurationDataBase &info,StreamInterface *err);

    friend bool MSGObjectSaveSetup(Message &msg,ConfigurationDataBase &info,StreamInterface *err);


protected:

    /** system unique message id */
    int32           id;

    /** the message code */
    MessageCode     messageCode;

    /** the message string */
    BString         content;

public:


    /** constructor to build a standard message */
    void            Init(
                        MessageCode     code,
                        const char *    content )
    {
        this->messageCode       = code;
        this->content           = content;
    }

    /** constructor to build using ObjectLoadSetup */
                    Message(): messageCode(0)
    {
        this->content           = "";
        this->id                = MSGGetUniqueId();
    }

    /** destructor */
    virtual         ~Message()
    {
    }

    /** retrieve the message code */
    MessageCode     GetMessageCode()
    {
        return messageCode;
    }

    /** retrieve the message id */
    int32           Id()
    {
        return id;
    }

    /** retrieve the message content */
    const char *    Content()
    {
        return content.Buffer();
    }

    /**  initialise an object from a set of configs
        The syntax is
        Code = <a number>  or in alternative
        UserCode = <a number> in this case the code will be offsetted with UserCode index
        Content = <any text (within " if includes spaces) >
        Standard GCReferenceContainer syntax
    */
    virtual     bool                ObjectLoadSetup(
                        ConfigurationDataBase &         info,
                        StreamInterface *               err)
    {
        return MSGObjectLoadSetup(*this,info,err);
    }

    /**  save the object into a set of configs
        The syntax is
        Code = <a number>
        Id   = <a number>
        Content = <any text (within " if includes spaces) >
        Standard GCReferenceContainer syntax
    */
    virtual     bool                ObjectSaveSetup(
                        ConfigurationDataBase &         info,
                        StreamInterface *               err)
    {
        return MSGObjectSaveSetup(*this,info,err);
    }
};


#endif
