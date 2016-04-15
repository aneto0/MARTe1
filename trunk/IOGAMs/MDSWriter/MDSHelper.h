/**
 * @file MDSHelper.h
 * @brief Header file for MDSHelper.h 
 * @date 14/03/2016
 * @author LLorenc Capella 
 * @author Andre' Neto
 *
 * @copyright Copyright 2015 F4E | European Joint Undertaking for ITER and
 * the Development of Fusion Energy ('Fusion for Energy').
 * Licensed under the EUPL, Version 1.1 or - as soon they will be approved
 * by the European Commission - subsequent versions of the EUPL (the "Licence")
 * You may not use this work except in compliance with the Licence.
 * You may obtain a copy of the Licence at: http://ec.europa.eu/idabc/eupl
 *
 * @warning Unless required by applicable law or agreed to in writing, 
 * software distributed under the Licence is distributed on an "AS IS"
 * basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
 * or implied. See the Licence permissions and limitations under the Licence.
 */

#ifndef MDS_HELPER_H
#define MDS_HELPER_H

#include "System.h"
#include "Message.h"
#include "MessageHandler.h"

/**
 * @brief Helper functions to allow the storate of asynchronous data in an MDSPlus tree
 * @details The interface between the caller object and the MDSWriter object is performed using MARTe messages.
 * The main reason is to allow the processing of the messages in the context of a different thread.
 * The message context contains the name of the MDSPlus tree node and the message must contain a CDB with the following structure
 * Type = string|float|int32|double
 * Value = Value to be stored
 * Append = if 1 data will be appended to the node (only valid for the string Type)
 */
namespace MDSHelper{
    /**
     * @brief Encodes the cdbe in a Message and sends it to an MDSWriter object @ location = mdsWriterLocation
     */
    static void PutValue(FString mdsWriterLocation, FString nodeName, CDBExtended cdbe) {
        GCRTemplate<MessageEnvelope> envelope(GCFT_Create);
        GCRTemplate<Message>         message(GCFT_Create);
        message->Init(0, nodeName.Buffer());
        message->Insert(cdbe);
        envelope->PrepareMessageEnvelope(message, mdsWriterLocation.Buffer());
        MessageHandler::SendMessage(envelope);
    }

    /**
     * @brief Encodes the value with Type=string and calls PutValue
     */
    static void PutString(FString mdsWriterLocation, FString nodeName, FString value, bool append) {
        ConfigurationDataBase cdb;
        CDBExtended cdbe(cdb);
        FString str = "string";
        cdbe.WriteFString(str, "Type");
        cdbe.WriteFString(value, "Value");
        int32 appendInt = append ? 1 : 0;
        cdbe.WriteInt32(append, "Append");
        PutValue(mdsWriterLocation, nodeName, cdbe);
    }

    /**
     * @brief Encodes the value with Type=int32 and calls PutValue
     */
    static void PutNumeric(FString mdsWriterLocation, FString nodeName, int32 value) {
        ConfigurationDataBase cdb;
        CDBExtended cdbe(cdb);
        FString str = "int32";
        cdbe.WriteFString(str, "Type");
        cdbe.WriteInt32(value, "Value");
        PutValue(mdsWriterLocation, nodeName, cdbe);
    }

    /**
     * @brief Encodes the value with Type=float and calls PutValue
     */
    static void PutNumeric(FString mdsWriterLocation, FString nodeName, float value) {
        ConfigurationDataBase cdb;
        CDBExtended cdbe(cdb);
        FString str = "float";
        cdbe.WriteFString(str, "Type");
        cdbe.WriteFloat(value, "Value");
        PutValue(mdsWriterLocation, nodeName, cdbe);
    }

    /**
     * @brief Encodes the value with Type=double and calls PutValue
     */
    static void PutNumeric(FString mdsWriterLocation, FString nodeName, double value) {
        ConfigurationDataBase cdb;
        CDBExtended cdbe(cdb);
        FString str = "double";
        cdbe.WriteFString(str, "Type");
        cdbe.WriteDouble(value, "Value");
        PutValue(mdsWriterLocation, nodeName, cdbe);
    }
};

#endif

