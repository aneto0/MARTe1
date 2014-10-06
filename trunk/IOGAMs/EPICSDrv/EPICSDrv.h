//******************************************************************************
//    $Log: EPICSDrv.h,v $
//
//******************************************************************************

/*
 * EPICS  IOGAM for reading and writing EPICS Process Variables
 * author: Gabriele Manduchi
 *
 * The EPICSDrv GACQM provides Input/Output from/to a set of EPICS process variables.
 * The same GACQM instance can be used by InputGAM and OutputGAM devices. When handling inputs, the GACQM
 * listens to the specified set of EPICS Process Variables (PVs) via ChannelAccess (CA) Monitor and keeps
 * the latest value in internal variable which is read whenever input data are required. 
 * When handling outputs, the GACQM instance will store the passed values in internal variable, and will update 
 * the corresponding PVs at the specified scan rate.  
 * Both output PV write and CA monitor for input PV are carried out by a separate thread which is run on the 
 * core specified by RunOnCpu configuration parameter.
 * The other configuration parameters are:
 *  - InputVariables defining the list of CA names for the input PVs. Meaningful when NumberOfInputs > 0.
 *       The number of inputPV is NumberOfInputs - 1 (the first input is the current time). Specified as a 
 *       string where the PV names are spaced by one or more blanks.
 *  - OutputVariables defining the list of CA names for the output PVs. Meaningful when NumberOfOutputs > 0.
 *       The number of variables must be equal to NumberOfOutputs. Specified as a 
 *       string where the PV names are spaced by one or more blanks.
 *  - OutputTypes defining the type array (1 for integer, 2 for float) for the output PVs. By defaults output
 *       PVs are considered float.
 *  - OutScanPeriod defining the minimum time (in milliseconds) bwtween two consecutive output PV update.
 *
 * Following is a sample declaration of EPICSDrv instance in DriverPool where the associated thread for EPICS managemen runs on Core 2;
 * Process Variable NB-SPIS-SYSM:H0SYSM-SR_IN is the input and Process Variable NB-SPIS-SYSM:H0SYSM-SR_OUT is the output. Both
 * variables are float (default) The scan period for output is 1 second. 


      +EPICSInterface = {
            NumberOfInputs = 2
            NumberOfOutputs = 1
            Class = EPICSDrv
            RunOnCpu = 2
            InputVariables = "NB-SPIS-SYSM:H0SYSM-SR_IN"
            OutputVariables = "NB-SPIS-SYSM:H0SYSM-SR_OUT"
            OutScanPeriod=1000
	}
 
*/


 
#ifndef EPICSDRV_H_
#define EPICSDRV_H_

#include "System.h"
#include "GenericAcqModule.h"
#include "cadef.h"
#include "MutexSem.h"
#include "EventSem.h"

OBJECT_DLL(EPICSDrv)

#define MAX_PV 256
#define IS_INT 1
#define IS_FLOAT 2
/*typedef struct{
    char		pvName[256];
    chid		thisChid;
    evid		thisEvid;
} PvInfo;
*/

class EPICSDrv:public GenericAcqModule {
private:
    int numInputs;
    int numOutputs;
    
    // Input PVs are always converted to float
    char *inPvNames[MAX_PV];
 //   PvInfo inCallbackInfos[MAX_PV];

    //Ouput PVs can be converted either to float or to int
    int outTypes[MAX_PV];
    char *outPvNames[MAX_PV];
    chid outChids[MAX_PV];
    chid inChids[MAX_PV];

    //Out Scan rate (in milliseconds). 
    int outScanPeriod;

    int serverCore;

    float incoming[MAX_PV];     //protected by inMutex
    float outcoming[MAX_PV];    //protected by outMutex

    uint lastScanCount;  //Time counter of last output update

    
    struct ca_client_context *epicsContext; //The EPICS context

    //MARTe stuff
	int64 lastCycleUsecTime;
    int runOnCpu;
    const char *css;
    MutexSem inputSem;
    MutexSem outputSem;
    EventSem writeEventSem;

public:
    /** Constructor */
    EPICSDrv();
    /** Deconstructor */
    virtual ~EPICSDrv();
    //Wait for new output and writes PVs
    void handleWrite();
    void awakeEPICSWriter();

    // Standard GAM methods
     /* Load setup from CDB.
        This IOGAM peculiar parameters are PeriodSleep_usec and FastSleep_usec. 
        @param info: CDB from which load data
        @param err: not used 
        @returns true if all ok*/
    virtual bool ObjectLoadSetup(ConfigurationDataBase &info,StreamInterface *err);
 		
     /* Print internal GAM informations 
        @param s: StreamInterface in which print infos
        @param full: not used
        @param err: not used 
        @returns true if all ok*/
    virtual bool ObjectDescription(StreamInterface &s,bool full = False, StreamInterface *err=NULL);
 		
     /* Saves the data into the DDB
        @param usecTime: not used
        @param buffer: pointer to the data buffer to be filled
        @returns 1 if all ok*/
    int32 GetData(uint32 usecTime, int32 *buffer, int32 bufferNum = 0);
 		
    bool WriteData(uint32 usecTime, const int32 *buffer);
 		
     // Set board used as input
    virtual bool SetInputBoardInUse(bool on = True){
         if(numInputs == 0 && on){
             AssertErrorCondition(InitialisationError, "EPICSDrv::SetInputBoardInUse: No inputs defined for Board %s", Name());
             return False;
         }
          return True;
    }


    virtual bool SetOutputBoardInUse(bool on = True){
         if(numOutputs == 0 && on){
             AssertErrorCondition(InitialisationError, "EPICSDrv::SetOutputBoardInUse: No outputs defined for Board %s", Name());
             return False;
         }
         return True;
    }

    virtual bool EnableAcquisition();

    virtual bool DisableAcquisition();

    // *** NOT IMPLEMENTED ***
    int64 GetUsecTime(){return lastCycleUsecTime;}
    
     /**
      * Output an HTML page with the current value in mV of the acquired signals
      */
     virtual bool ProcessHttpMessage(HttpStream &hStream);
     /**
      * Polling method
      */
     virtual bool Poll();
     
     /**
      * Pulse start
      */
     virtual bool PulseStart();
 
    friend void dataCallback(struct event_handler_args eha);
    friend void *registerCallbacks(void *arg);


private:
	OBJECT_DLL_STUFF(EPICSDrv)
};

#endif /*EPICSDRV_H_*/
