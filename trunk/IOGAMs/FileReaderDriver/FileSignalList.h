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

#if !defined (FILE_SIGNAL_LIST)
#define FILE_SIGNAL_LIST

/**
 * @file Contains a signal list sharing the same time source and signal type.
 * The input files are expected to be organised in columns and separated by spaces.
 */

#include "System.h"
#include "Threads.h"
#include "GCNamedObject.h"
#include "BasicTypes.h"
#include "GenericAcqModule.h"

OBJECT_DLL(FileSignalList)
class FileSignalList:public GCNamedObject{
OBJECT_DLL_STUFF(FileSignalList)
private:
    /**
     * Time vector in microseconds
     */
    int64 *time;
    /**
     * Data vector
     */
    void *data;
    /**
     * The number of samples
     */
    int32 numberOfSamples;
    /**
     * The latest sample read
     */
    int32 sampleCounter;
    /**
     * The filename where data is stored
     */
    FString filename;
    /*
     * The separator for the columns in the file
     */
    FString separator;
public:
    /**
     * Number of signals
     */
    int32 numberOfSignals;
    /**
     * The signals type
     */    
    BasicTypeDescriptor signalType; 


    FileSignalList(){
        numberOfSignals = 0;
        numberOfSamples = 0;
        sampleCounter   = 0;
        time            = NULL;
        data            = NULL;
        
    }     
   
    virtual ~FileSignalList(){
        if(time != NULL){
            delete []time;
        }
        if(data != NULL){
            free((void *&)data);
        }
    }

    /**
     * Load and configure object parameters
     * @param info the configuration database
     * @param err the error stream
     * @return True if no errors are found during object configuration
     */
    bool ObjectLoadSetup(ConfigurationDataBase &info,StreamInterface *err);

    /**
     * Resets the sample counter
     */
    void Reset(){
        sampleCounter = 0;
    }

    /**
     * Returns the array of samples for a given time in microseconds.
     * Assumes monotonic growing time.
     * @param usecTime the time in microseconds
     * @return the data for this particular time
     */
    void *GetNextSample(uint32 usecTime);

private:
    /**
     *Read the actual data from the file
     *@return True if data is successfully read
     */
    bool LoadData();

    /**
     * Checks if the line is a comment
     * @param line the line to check
     * @return True if it starts with a comment characted
     */
    bool IsComment(FString &line){
        if(line.Size() > 1){
            char c = line.Buffer()[0];
            if((c == '#') || (c == '/') || (c == '%')){
                return True;
            }
        }
        return False;
    }
};
#endif

