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

#ifndef BASELIB2_ADAPTER_H_
#define BASELIB2_ADAPTER_H_

#include "AdapterMessageListener.h"

/**
 * @brief Allows to load a BaseLib2 objects using an anonymous C interface.
 */
namespace BaseLib2 {
/**
 * Interface type definition
 */
typedef unsigned int uint32;
typedef char char8;

class Adapter {
public:
    /**
     * @brief Singleton interface to the Adapter.
     */
    static Adapter *Instance();

    /**
     * @brief Destructor
     * @post
     *   ProcessorType::defaultCPUs = 1u;
     */
    ~Adapter();

    /**
     * @brief Loads the config into the GlobalObjectDatabase.
     * @param[in] config the configuration to load.
     * @return true if the configuration can be successfully loaded.
     */
    bool LoadObjects(const char8 *config);

    /**
     * @brief Destroys the GlobalObjectDatase
     */
    void UnloadObjects();

    /**
     * @brief Sends a Message to BaseLib2 (replies are currently not supported).
     * @param[in] destination the name of the destination object in the GlobalObjectDatabase.
     * @param[in] content the message content.
     * @param[in] code the message code.
     * @return true if the message was successfully received.
     */
    bool SendMessageToBaseLib2(const char8 *destination,
                               const char8 *content,
                               uint32 code);

    /**
     * @brief Receives a Message from BaseLib2 (replies are currently not supported) and calls AdapterMessageListener::HandleBaseLib2Message
     * @param[in] destination the name of the destination object in the GlobalObjectDatabase.
     * @param[in] content the message content.
     * @param[in] code the message code.
     * @return true if the message was successfully received.
     */
    bool ReceiveMessageFromBaseLib2(const char8 *destination,
                                    const char8 *content,
                                    uint32 code);


    /**
     * @brief Sets the AdapterMessageListener
     * @param[in] listener the AdapterMessageListener to be called when new messages arrive
     */
    void SetAdapterMessageListener(AdapterMessageListener *listener);

private:
    /**
     * @brief Constructor. NOOP.
     */
    Adapter();

    /**
     * The AdapterMessageListener 
     */
    AdapterMessageListener *messageListener;
};
}
#endif

