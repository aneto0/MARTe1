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
 * $Id: $
 *
 **/

#ifndef BASELIB2_ADAPTER_MESSAGE_LISTENER_H_
#define BASELIB2_ADAPTER_MESSAGE_LISTENER_H_

/**
 * @brief Classes that implement this interface know how to handle BaseLib2 messages and will be called by the Adapter 
 */
namespace BaseLib2 {
/**
 * Interface type definition
 */
typedef unsigned int uint32;
typedef char char8;

class AdapterMessageListener {
public:
    /**
     * @brief Receives a Message from BaseLib2 (replies are currently not supported).
     * @details This function shall be implemented by the library that wants to interface to BaseLib2.
     * @param[in] destination the name of the destination object in the GlobalObjectDatabase.
     * @param[in] content the message content.
     * @param[in] code the message code.
     * @return true if the message was successfully received.
     */
    virtual bool HandleBaseLib2Message(const char8 *destination, const char8 *content, uint32 code) = 0;

    virtual ~AdapterMessageListener() {
    }

};
}
#endif

