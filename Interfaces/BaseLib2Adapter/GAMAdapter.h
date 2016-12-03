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

#ifndef GAM_ADAPTER_INTERFACE_H_
#define GAM_ADAPTER_INTERFACE_H_


namespace BaseLib2 {
/**
 * Interface type definition
 */
typedef unsigned int uint32;
typedef char char8;

/**
 * @brief TODO
 */
class GAMAdapter {
public:
    /**
     * @brief TODO
     */
    static GAMAdapter *Instance();

    /**
     * @brief TODO
     */
    ~GAMAdapter();

    /**
     * @brief TODO
     */
    bool AddGAM(const char8 *gamName, const char8 *gamInfo, uint32 &gamIdx);

    /**
     * @brief TODO
     */
    bool RemoveGAM(uint32 gamIdx);

    /**
     * @brief TODO
     */
    bool AddGAMInputSignal(uint32 gamIdx, const char8 *gamSignalName, const char8 *gamSignalType);

    /**
     * @brief TODO
     */
    bool AddGAMOutputSignal(uint32 gamIdx, const char8 *gamSignalName, const char8 *gamSignalType);

    /**
     * @brief TODO
     */
    bool FinaliseGAM(uint32 gamIdx, void *& inputToGAM, void *&outputFromGAM);
    
    /**
     * @brief TODO
     */
    bool ExecuteGAM(uint32 gamIdx);

    /**
     * @brief TODO
     */
    bool LoadObjects(const char8 *config);
    
    /**
     *@brief TODO
     */
    bool SendMessage(const char8 *destination, const char8 *content, uint32 code);

private:
    /**
     * @brief TODO
     */
    GAMAdapter();
};
}
#endif

