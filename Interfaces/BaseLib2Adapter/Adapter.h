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

#ifndef BASELIB2_ADAPTER_INTERFACE_H_
#define BASELIB2_ADAPTER_INTERFACE_H_

/**
 * @brief TODO
 */
namespace BaseLib2 {
class Adapter {
public:
    /**
     * @brief TODO
     */
    static Adapter *Instance();

    /**
     * @brief TODO
     */
    ~Adapter();

    /**
     * @brief TODO
     */
    bool LoadObjects(const char *config);
    
    /**
     *@brief TODO
     */
    bool SendMessageTo(const char *destination, const char *content, uint32 code);

    /**
     *@brief TODO
     */
    bool ReceiveMessageFrom(const char *destination, const char *content, uint32 code);

private:
    /**
     * @brief TODO
     */
    Adapter();
};
}
#endif

