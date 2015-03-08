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

#if !defined (SSMINPUTSTATEOUTPUT_H)
#define SSMINPUTSTATEOUTPUT_H

#include "System.h"
#include "DDBInputInterface.h"
#include "GAM.h"

#define ADD  1
#define SUB  2
#define COPY 3
#define MUL  4

class SSMInputStateOutput{

    /** DDB interface. */
    DDBInputInterface   *ddbInputSignals;

    /** Vector of signal data. */
    float               *signalValues;

    /** Vector of signal data types. */
    int                 *signalTypes;

    /** Number of signals. */
    int                 numberOfSignals;

    /** Enabling the object. */
    bool                enable;

    /** All values loaded by DDB or not. */
    bool                allValues;

public:

    /** */
    SSMInputStateOutput();

    /** */
    ~SSMInputStateOutput();

    /** Load data from CDB. */
    bool LoadStartPoint(ConfigurationDataBase &cdbData, GAM &gam, char *entry, int dimension);

    /** Update data. */
    bool Update(float *dataIO, int typeOp);

    /** Catch data values. */
    bool Catch();

    /** Clean up class parameters. */
    void CleanUp();

private:

    /** Check if signal is defined as vector. */
    int IsVector(FString &signalName);

    /** Check if an entry is a number. */
    bool IsNumber(FString entry, float &value);
};

#endif
