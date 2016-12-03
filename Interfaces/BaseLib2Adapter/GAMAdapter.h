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
 * @brief Allows to load a GAM using an anonymous C interface.
 */
class GAMAdapter {
public:
    /**
     * @brief Singleton interface to the GAMAdapter.
     */
    static GAMAdapter *Instance();

    /**
     * @brief NOOP.
     */
    ~GAMAdapter();

    /**
     * @brief Registers a GAM in the GlobalObjectDabase.
     * @param[in] gamName The name of the GAM to register (shall match the name of the GAM defined in the gamInfo).
     * @param[in] gamInfo The GAM configuration file (shall contain the +GAMName = { Class = ...} where GAMName is the name of the GAM.
     * @param[out] gamIdx index of the registered GAM to be used with the other functions.
     * @return true if the GAM can be sucessfully registered.
     */
    bool AddGAM(const char8 *gamName,
                const char8 *gamInfo,
                uint32 &gamIdx);

    /**
     * @brief Removes the GAM from the GlobalObjectDabase.
     * @param[in] gamIdx index of the registered GAM.
     * @return true if the GAM could be successfully removed.
     * @pre
     *    AddGAM
     */
    bool RemoveGAM(uint32 gamIdx);

    /**
     * @brief Adds an input signal to the GAM .
     * @param[in] gamIdx index of the registered GAM.
     * @param[in] gamSignalName the signal name.
     * @param[in] gamSignalType a string defining the signal type (as understood by the DDB).
     * @return true if the signal was successfully added to the GAM.
     * @pre
     *    AddGAM
     */
    bool AddGAMInputSignal(uint32 gamIdx,
                           const char8 *gamSignalName,
                           const char8 *gamSignalType);

    /**
     * @brief Adds an output signal to the GAM .
     * @param[in] gamIdx index of the registered GAM.
     * @param[in] gamSignalName the signal name.
     * @param[in] gamSignalType a string defining the signal type (as understood by the DDB).
     * @return true if the signal was successfully added to the GAM.
     * @pre
     *    AddGAM
     */
    bool AddGAMOutputSignal(uint32 gamIdx,
                            const char8 *gamSignalName,
                            const char8 *gamSignalType);

    /**
     * @brief Verifies that all the GAM input/output signals were successfully added.
     * @param[in] gamIdx index of the registered GAM.
     * @param[in] inputToGAM memory of signals that are to be fed into the GAM.
     * @param[in] outputFromGAM a memory of the that are to be fed from the GAM.
     * @return true if all the signals required by the GAM were added (see AddGAMInputSignal and AddGAMOutputSignal).
     * @pre
     *    AddGAM
     */
    bool FinaliseGAM(uint32 gamIdx,
                     void *& inputToGAM,
                     void *&outputFromGAM);

    /**
     * @brief Calls the GAM Execute method.
     * @param[in] gamIdx index of the registered GAM.
     * @param[in] executionCode the GAM_FunctionNumbers as the defined in GAM.
     * @return true if the GAM Execute method succeeds.
     * @pre
     *    FinaliseGAM
     */
    bool ExecuteGAM(uint32 gamIdx,
                    uint32 executionCode);

private:
    /**
     * @brief Default contructor.
     */
    GAMAdapter();
};
}
#endif

