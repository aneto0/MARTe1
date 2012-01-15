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

#if !defined (_RTDATASTORAGESYS)
#define _RTDATASTORAGESYS

#include "System.h"
#include "Object.h"
#include "QueueHolder.h"
#include "RTCollectionBuffer.h"

#include "GCRTemplate.h"
#include "SignalInterface.h"
#include "HttpStream.h"

const int32 RTDataStorageMaxWindows = 16;
const int32 RTMaxSamplingPeriodUsec = 10000000;
const int32 RTMinSamplingPeriodUsec = -10000000;

/** Real-Time Storage Data System Class.
    It's a linked list of RTCollectionBuffer objects.
    In each buffer are stored all the signals for
    a particular time-slot.  */
OBJECT_DLL(RTDataStorageSystem)
class RTDataStorageSystem:public Object, protected QueueHolder{
OBJECT_DLL_STUFF(RTDataStorageSystem)
private:

    /**Clock node structure  */
    struct CLOCKInitStruct{
        /// Number of sequences
        int32 nOfSequences;

        /// NOfSamples
        int32 nOfSamples[RTDataStorageMaxWindows];

        /// The sampling period (in usec)
        int32 samplingPeriodUsec[RTDataStorageMaxWindows];
    
        /// The number of samples already read in a particular window
        int32 nOfReadSamples;
    }CLOCKInit;

private:

    /// Required slow sample times (times are stored in usec)
    int32   *acquisitionTimesBuffer;

    /// Size of vector of times : initialised by GAP
    int32   acquisitionTimesBufferSize;

    /// position in the acquisitionTimesBuffer vector : set to 0 at every pulse start
    uint32  acquisitionTimesBufferIndex;

private:

    /// Clean up time windows structures and deallocates memory
    void    CleanUpTimeVector();

    /**  */
    bool    CreateTimeVectorFromTimeWindow();

private:

    /** Max number of fast acquisition samples : from configuration */
    int32   maxFastAcquisitionPoints;

    /** Fast acquisition points left : setup before every pulse */
    int32   fastAcquisitionPointsLeft;

    /**  Points for a single fast acquisition window : setup at every fast trigger */
    int32   pointsForSingleFastAcquisition;

    /** Fast acquisition points to do for this window : setup at every fast
        trigger and decremented from then on */
    int32   fastAcquisitionPointsToDo;


    /*******************************************************************************************************
    /*
    /* Avoid the user from making copies of the RTDataStorageSystem and forgetting to handle the memory allocation
    /*
    ********************************************************************************************************/

    /// Copy constructors (since it is defined private it won't allow a public use!!)
    RTDataStorageSystem(const RTDataStorageSystem&){};

    /// Operator=  (since it is defined private it won't allow a public use!!)
    RTDataStorageSystem& operator=(const RTDataStorageSystem&){};

    /** */
    RTCollectionBuffer *List(){ return (RTCollectionBuffer *)QueueHolder::List(); }

public:

    /** Constructo r*/
    RTDataStorageSystem();

    /** Destructor */
    ~RTDataStorageSystem();

    /** */
    bool ObjectLoadSetup(ConfigurationDataBase &info,StreamInterface *err);

    /** */
    bool ObjectDescription(StreamInterface &s,bool full=False,StreamInterface *err=NULL);

    /** Stores a RTCollectionBuffer.
        Stores RTCollectionBuffers at dataSampleRate.
        Returns rtBuffer if it isn't stored.
        Returns NULL pointer if rtBuffer is stored in RTDataStorageSystem object.
        Returns rtBuffer if RTDataStorageSystem is full.
        If fastTrigger == true and the RTDataStrageSystem is not full
        force the acquisition.*/
    RTCollectionBuffer *StoreData(RTCollectionBuffer &rtBuffer,bool fastTrigger);


    /** Resets everything so to be ready for pulsing */
    void PrepareForNextPulse();

    /** Export Reset() method */
    inline void Reset(){ QueueHolder::Reset(); }

    /** Get Stored Data */
    bool GetSignalData(int32 signalOffset, GCRTemplate<SignalInterface> &signal);

    /** */
    uint32 Size(){ return ListSize(); }

    /// Set time windows menu
    void TimeWindowsMenu(StreamInterface &in,StreamInterface &out);

    void HTMLInfo(HttpStream &hStream);

};

#endif
