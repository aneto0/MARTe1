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
#include "EPICSHandler.h"
#include "Atomic.h"
#include "CDBExtended.h"

// EPICS version
#include "../src/ca/nciu.h"
#include "caProto.h"
#include "epicsVersion.h"
#include "epicsRelease.h"

// EPICS FileDescriptor Manager
#include <fdmgr.h>
#include <fdManager.h>

// EPICS pCAS GDD stuff
#include "gddAppFuncTable.h"
#include "smartGDDPointer.h"
#include "gddApps.h"
#include "aitTypes.h"

#include "FString.h"
#include "BasicTypes.h" // BasicTypeDescriptor
#include "Sleep.h"

#define DEFAULT_EPICS_DBG 0
#define DEFAULT_EPICS_HOPR 10.0
#define DEFAULT_EPICS_LOPR -10.0
#define DEFAULT_EPICS_LEN 1
#define DEFAULT_EPICS_PREC 4
//#define DEFAULT_EPICS_SCAN 1.0
#define DEFAULT_EPICS_SCAN 0
#define DEFAULT_EPICS_EGU ""
#define DEFAULT_EPICS_SYNC "excasIoSync"
#define DEFAULT_EPICS_ASYNC "excasIoAsync"
#define DEFAULT_EPICS_SCANON true
#define DEFAULT_EPICS_ASYNCSCAN true
#define DEFAULT_EPICS_ASYNCDELAY 0.1
#define DEFAULT_EPICS_MAXSIMULTASYNCIO 1000u
#define DEFAULT_EPICS_HYST 0.0
#define DEFAULT_EPICS_ADEL 0.0
#define DEFAULT_EPICS_MDEL 0.0

#define DEFAULT_EPICS_CPUMASK 0xFFFF
#define DEFAULT_EVENT_CPUMASK 0xFFFF

#define DEFAULT_BUFFER_SIZE 0xFFFF
#define DEFAULT_BUFFER_ALIGN 4

const char * EPICSHandler::tf_strings[] = { "false", "true" };
const int32  EPICSHandler::tf_values[] = { false, true };

/*
typedef enum {
	aitEnumInvalid=0,
	aitEnumInt8,
	aitEnumUint8,
	aitEnumInt16,
	aitEnumUint16,
	aitEnumEnum16,
	aitEnumInt32,
	aitEnumUint32,
	aitEnumFloat32,
	aitEnumFloat64,
	aitEnumFixedString,
	aitEnumString,
	aitEnumContainer
} aitEnum;
*/

// TODO
//see base-3-14-11/src/gdd/aitTypes.c:41 aitName
//see base-3-14-11/src/gdd/aitTypes.h:131 ... 
//aitSize aitName aitPrintf aitScanf
const char * EPICSHandler::aitEnum_strings[] = {
	"aitEnumInvalid",
	"aitEnumInt8",
	"aitEnumUint8",
	"aitEnumInt16",
	"aitEnumUint16",
	"aitEnumEnum16",
	"aitEnumInt32",
	"aitEnumUint32",
	"aitEnumFloat32",
	"aitEnumFloat64",
	"aitEnumFixedString",
	"aitEnumString",
	"aitEnumContainer",
	0 };

// the following must match epics/base/include/menuAlarmSevr.h
#include "menuAlarmSevr.h"
const char * EPICSHandler::menuAlarmSevr_strings[] = {
		"NO_ALARM",
		"MINOR",
		"MAJOR",
		"INVALID",
		0 };

// the following must match epics/base/include/menuAlarmStat.h
#include "menuAlarmStat.h"
const char * EPICSHandler::menuAlarmStat_strings[] = {
		"NO_ALARM",
		"READ",
		"WRITE",
		"HIHI",
		"HIGH",
		"LOLO",
		"LOW",
		"STATE",
		"COS",
		"COMM",
		"TIMEOUT",
		"HWLIMIT",
		"CALC",
		"SCAN",
		"LINK",
		"SOFT",
		"BAD_SUB",
		"UDF",
		"DISABLE",
		"SIMM",
		"READ_ACCESS",
		"WRITE_ACCESS",
		0 };

// the following must match epics/base/include/menuScan.h
#include "menuScan.h"
const char * EPICSHandler::menuScan_strings[] = {
		"Passive",
		"Event",
		"I/O Intr",
		"10 second",
		"5 second",
		"2 second",
		"1 second",
		".5 second",
		".2 second",
		".1 second",
		0 };

const float EPICSHandler::menuScan_values[] = {
		-1.0, -1.0, -1.0,
		10.0, 5.0, 2.0, 1.0,
		0.5, 0.2, 0.1,
		0 };

// the following must match epics/base/include/menuYesNo.h
#include "menuYesNo.h"
const char * EPICSHandler::menuYesNo_strings[] = {
	"NO",
	"YES",
	0 };

/*
enum excasIoType { excasIoSync, excasIoAsync };
*/ 
const char * EPICSHandler::excasIoType_strings[] = {
	DEFAULT_EPICS_SYNC,
	DEFAULT_EPICS_ASYNC
	};

const int EPICSHandler::menu_values[] = {
		0, 1, 2, 3, 4, 5, 6, 7,
		8, 9, 10, 11, 12, 13, 14, 15,
		16, 17, 18, 19, 20, 21, 22, 23
};


bool EPICSHandler::ObjectLoadSetup( ConfigurationDataBase &info,StreamInterface *err) {
	// we use an Extended CDB in order to read double, strings and etc..
    CDBExtended cdb(info);
    int32 _i32;
    
    if ( !GCNamedObject::ObjectLoadSetup(info,err) ) {
        AssertErrorCondition(InitialisationError,
        		"EPICSHandler::ObjectLoadSetup: ?!?!: GCNamedObject::ObjectLoadSetup Failed");
        return False;
    }
    AssertErrorCondition(Information,
    		"EPICSHandler::ObjectLoadSetup: %s: Loading Process Variables",
    		Name());

    // read the common prefix of all Process Variables (is not mandatory
    if ( !cdb.ReadFString(pvPrefix, "PREFIX", "") ) {
        AssertErrorCondition(Information,
        		"EPICSHandler::ObjectLoadSetup: %s: PREFIX not specified. Assuming no prefix.",
        		Name());
    }
    
    if ( !cdb.ReadOptions(_i32, "scanOn", tf_strings, tf_values, DEFAULT_EPICS_SCANON) ) {
        AssertErrorCondition(Information,
        		"EPICSHandler::ObjectLoadSetup: %s: scanOn not specified. Assuming %d.",
        		Name(), DEFAULT_EPICS_SCANON);    	
    }
    scanOn = _i32;
    if ( !cdb.ReadOptions(_i32, "asyncScan", tf_strings, tf_values, DEFAULT_EPICS_ASYNCSCAN) ) {
    	AssertErrorCondition(Information,
    			"EPICSHandler::ObjectLoadSetup: %s: asyncScan not specified. Assuming %d.",
    			Name(), DEFAULT_EPICS_ASYNCSCAN);    	
    }    
    asyncScan = _i32;
    if ( !cdb.ReadDouble(asyncDelay, "asyncDelay", DEFAULT_EPICS_ASYNCDELAY) ) {
        AssertErrorCondition(Information,
        		"EPICSHandler::ObjectLoadSetup: %s: asyncDelay not specified. Assuming %f.",
        		Name(), DEFAULT_EPICS_ASYNCDELAY);
    }
    if ( !cdb.ReadInt32( _i32, "maxSimultAsyncIO", (int32)DEFAULT_EPICS_MAXSIMULTASYNCIO) ) {
        AssertErrorCondition(Information,
        		"EPICSHandler::ObjectLoadSetup: %s: maxSimultAsyncIO not specified. Assuming %d.",
        		Name(), DEFAULT_EPICS_MAXSIMULTASYNCIO);
    }
    maxSimultAsyncIO = _i32;
    if ( !cdb.ReadInt32( _i32, "debugLevel", (int32)DEFAULT_EPICS_DBG) ) {
        AssertErrorCondition(Information,
        		"EPICSHandler::ObjectLoadSetup: %s: debugLevel not specified. Assuming %d.",
        		Name(), DEFAULT_EPICS_DBG);
    }
    debugLevel = _i32;
  
    //PublishSubscriber Interface(TODO) configuration options

    if ( !cdb.ReadInt32(cpuMask,"RunOnCPU", DEFAULT_EPICS_CPUMASK) ) {
    	AssertErrorCondition(Information,
    			"EPICShandler ::ObjectLoadSetup: %s: RunOnCPU not specified. Assuming %d.",
    			Name(), cpuMask);
    }
    if ( !cdb.ReadInt32(cpuMask_event,"RunOnCPU_event", DEFAULT_EVENT_CPUMASK) ) {
    	AssertErrorCondition(Information,
    			"EPICShandler ::ObjectLoadSetup: %s: RunOnCPU_event not specified. Assuming %d.",
    			Name(), cpuMask_event);
    }
    
    //buffers number, buffer must be allocated before event's thread allocation

    if ( !cdb.ReadInt32(buffer_size,"BufferSize", DEFAULT_BUFFER_SIZE) ) {
    	AssertErrorCondition(Information,
    			"EPICShandler ::ObjectLoadSetup: %s: BufferSize not specified. Assuming %08x.",
    			Name(), buffer_size);
    }
    if ( !cdb.ReadInt32(buffer_align,"BufferAlign", DEFAULT_BUFFER_ALIGN) ) {
    	AssertErrorCondition(Information,
    			"EPICShandler ::ObjectLoadSetup: %s: BufferAlign not specified. Assuming %08x.",
    			Name(), buffer_align);
    }
        
    //create the array of Process Variable descriptors (pvInfo)
    if ( !cdb->Move("ProcessVariable") ) {
        AssertErrorCondition(InitialisationError,
        		"EPICSHandler::ObjectLoadSetup: %s: No Process Variable specified. Fatal error!",
        		Name());
        return False;
    }
    numberOfPVs = cdb->NumberOfChildren();
    if ( numberOfPVs == 0 ) {
        AssertErrorCondition(InitialisationError,
        		"EPICSHandler::ObjectLoadSetup: %s: Number of signals is zero. Fatal error!",
        		Name());
        return False;
    }
    pvList = new pvInfo* [numberOfPVs];
    if ( pvList == NULL ) {
        AssertErrorCondition(InitialisationError,
        		"EPICSHandler::ObjectLoadSetup: %s: Failed to allocate space for %d ProcessVariables. Fatal error!",
        		Name(), numberOfPVs);
        return False;
    }
    
    //create the pCAS here and then attach the PV's
    pCAS = new exServer( scanOn != 0, asyncScan== 0,
    		asyncDelay, maxSimultAsyncIO );
    if ( !pCAS ) {
        AssertErrorCondition(InitialisationError,
        		"EPICSHandler::ObjectLoadSetup: %s: Failed to allocate space for pCAS. Fatal error!",
        		Name() );
        return False;
    }
    pCAS->setDebugLevel(debugLevel);
    
    //read process variables properties loop
    //TODO every PV must have it's own custom ObjectLoadSetup
    //TODO at least do that for the DBR/DBF types
    int i=0;
    for(i=0; i<numberOfPVs; i++) {
        cdb->MoveToChildren(i);
        
        FString pvName;
        if ( !cdb.ReadFString(pvName, "NAME") ) {
            AssertErrorCondition(InitialisationError,
            		"EPICSHandler::ObjectLoadSetup: %s: NAME not specified. Fatal error!",
            		Name());
            return False;
        }
//        ((char *)(pvName.Buffer()))[pvName.Size()] = 0;
        FString pvType;
        aitEnum aitType;
        if ( !cdb.ReadFString(pvType, "TYPE") ) {
            AssertErrorCondition(InitialisationError,
            		"EPICSHandler::ObjectLoadSetup: %s: TYPE not specified. Fatal error!",
            		Name());
            return False;
        }
        // convert to ait type
        aitType = ConvertToaitEnum( pvType.Buffer() );
        
        FString pvSync;
        excasIoType excasSync;
        if ( !cdb.ReadFString(pvSync, "SYNC", DEFAULT_EPICS_SYNC) ) {
            AssertErrorCondition(InitialisationError,
            		"EPICSHandler::ObjectLoadSetup: %s: TYPE not specified. Assuming %s",
            		Name(), DEFAULT_EPICS_SYNC);
        }
        // convert to excasIoType
        excasSync = ConvertToexcasIoType( pvSync.Buffer() );

        unsigned pvElements;
        if ( !cdb.ReadInt32(_i32, "LEN", (int32)DEFAULT_EPICS_LEN) ) {
           	AssertErrorCondition(Information,
           			"EPICSHandler::ObjectLoadSetup: %s: LEN not specidfied. Assuming %d",
           			Name(), pvElements);
        }
        pvElements = _i32;
/* previous implementation        
        double pvScanPeriod;
        if ( !cdb.ReadDouble(pvScanPeriod, "SCAN", DEFAULT_EPICS_SCAN)) {
        	AssertErrorCondition(Information,
        			"EPICSHandler::ObjectLoadSetup: %s: SCAN not specified. Assuming %f", 
        			Name(), DEFAULT_EPICS_SCAN);
        }
*/        
        if( !cdb.ReadOptions(_i32, "SCAN", menuScan_strings, menu_values, DEFAULT_EPICS_SCAN) ) {
        	AssertErrorCondition(Information,
        			"EPICSHandler::ObjectLoadSetup: %s: SCAN not specified. Assuming \"%s\"", 
        			Name(), menuScan_strings[DEFAULT_EPICS_SCAN]);        	
        }
        float pvScanPeriod = menuScan_values[_i32];
        
        FString pvUnits;
        if ( !cdb.ReadFString(pvUnits, "EGU", DEFAULT_EPICS_EGU) ) {
            AssertErrorCondition(InitialisationError,
            		"EPICSHandler::ObjectLoadSetup: %s: EGU not specified. Assuming \"%s\"",
            		Name(), DEFAULT_EPICS_EGU);
        }
//        ((char *)(pvUnits.Buffer()))[pvUnits.Size()] = 0;
        
        // graphical limits "High/Low OPerational Range"
        double pvHopr;
        if(!cdb.ReadDouble(pvHopr, "HOPR", DEFAULT_EPICS_HOPR)) {
        	AssertErrorCondition(Information, 
        			"EPICSHandler::ObjectLoadSetup: %s: HOPR not specified. Assuming %f",
        			Name(), DEFAULT_EPICS_HOPR);
        }     
        double pvLopr;
        if ( !cdb.ReadDouble(pvLopr, "LOPR", DEFAULT_EPICS_LOPR)) {
        	AssertErrorCondition(Information,
        			"EPICSHandler::ObjectLoadSetup: %s: LOPR not specified. Assuming %f", 
        			Name(), DEFAULT_EPICS_LOPR);
        }

        // alarm limits
        // the policy is to set alarms to OPerational Ranges if not specified
        double pvHihi; //Hihi Alarm Limit
        if(!cdb.ReadDouble(pvHihi, "HIHI", pvHopr)) {
        	AssertErrorCondition(Information, 
        			"EPICSHandler::ObjectLoadSetup: %s: HIHI not specified. Assuming %f",
        			Name(), pvHopr);
        }
        double pvHigh; //High Alarm Limit 
        if(!cdb.ReadDouble(pvHigh, "HIGH", pvHopr)) {
        	AssertErrorCondition(Information, 
        			"EPICSHandler::ObjectLoadSetup: %s: HIGH not specified. Assuming %f",
        			Name(), pvHopr);
        }     
        double pvLow; //Low Alarm Limit
        if ( !cdb.ReadDouble(pvLow, "LOW", pvLopr)) {
        	AssertErrorCondition(Information,
        			"EPICSHandler::ObjectLoadSetup: %s: LOW not specified. Assuming %f", 
        			Name(), pvLopr);
        }
        double pvLolo; //Lolo Alarm Limit
        if ( !cdb.ReadDouble(pvLolo, "LOLO", pvLopr)) {
        	AssertErrorCondition(Information,
        			"EPICSHandler::ObjectLoadSetup: %s: LOLO not specified. Assuming %f", 
        			Name(), pvLopr);
        }
         
        // menu alarm severity
        int32 pvHhsv; //Hihi Alarm Severity 
        if( !cdb.ReadOptions(pvHhsv, "HHSV", menuAlarmSevr_strings, menu_values, 0) ) {
        	AssertErrorCondition(Information, 
        			"EPICSHandler::ObjectLoadSetup: %s: HHSV not specified. Assuming %s",
        			Name(), menuAlarmSevr_strings[pvHhsv] );
        }
        int32 pvHsv; //High Alarm Severity 
        if( !cdb.ReadOptions(pvHsv, "HSV", menuAlarmSevr_strings, menu_values, 0) ) {
        	AssertErrorCondition(Information, 
        			"EPICSHandler::ObjectLoadSetup: %s: HSV not specified. Assuming %s",
        			Name(), menuAlarmSevr_strings[pvHsv] );
        }     
        int32 pvLsv; //Low Alarm Severity
        if( !cdb.ReadOptions(pvLsv, "LSV", menuAlarmSevr_strings, menu_values, 0) ) {
        	AssertErrorCondition(Information,
        			"EPICSHandler::ObjectLoadSetup: %s: LSV not specified. Assuming %s", 
        			Name(), menuAlarmSevr_strings[pvLsv] );
        }
        int32 pvLlsv; //Lolo Alarm Severity
        if( !cdb.ReadOptions(pvLlsv, "LLSV", menuAlarmSevr_strings, menu_values, 0) ) {
        	AssertErrorCondition(Information,
        			"EPICSHandler::ObjectLoadSetup: %s: LLSV not specified. Assuming %s", 
        			Name(), menuAlarmSevr_strings[pvLlsv] );
        }      
        
        // hysteresis/deadband values
        double pvHyst; // Hysteresis Alarm deadband
        if ( !cdb.ReadDouble(pvHyst, "HYST", DEFAULT_EPICS_HYST)) {
        	AssertErrorCondition(Information,
        			"EPICSHandler::ObjectLoadSetup: %s: HYST not specified. Assuming %f", 
        			Name(), pvHyst);
        } 
        double pvAdel; // Archive deadband
        if ( !cdb.ReadDouble(pvAdel, "ADEL", DEFAULT_EPICS_ADEL)) {
        	AssertErrorCondition(Information,
        			"EPICSHandler::ObjectLoadSetup: %s: ADEL not specified. Assuming %f", 
        			Name(), pvAdel);
        }
        double pvMdel; // Monitor deadband
        if ( !cdb.ReadDouble(pvMdel, "MDEL", DEFAULT_EPICS_MDEL)) {
        	AssertErrorCondition(Information,
        			"EPICSHandler::ObjectLoadSetup: %s: MDEL not specified. Assuming %f", 
        			Name(), pvMdel);
        }         
        
        // read precision (realted only to ai/ao records)
        int32 pvPrec;
        if ( !cdb.ReadInt32(pvPrec, "PREC", DEFAULT_EPICS_PREC) ) {
           	AssertErrorCondition(Information,
           			"EPICSHandler::ObjectLoadSetup: %s: PREC not specidfied. Assuming %d",
           			Name(), pvPrec);
        }
        
        //--------------------------------------------------------------------- end of EPICS stuff
        
        
        // we have to allocate the memory for the pv's name (TODO modify pvInfo or dealloc)
        //FString * pvNameObj = new FString(pvName); //TODO deallocate this space -> DONE
        //FString * pvUnitsObj = new FString(pvUnits); //TODO deallocate this space -> DONE
        // create the pv's descriptor pvInfo
        pvInfo *pPVI = new pvInfo(pvScanPeriod, // scan period
        		pvName.Buffer(), // name of the pv (will be copied)
        		pvUnits.Buffer(), // units of the pv (will be copied)
        		(aitFloat32) pvHopr, (aitFloat32) pvLopr, // HOPR and LOPR, gr=graphical
        		pvHihi, pvHigh, pvLow, pvLolo,
        		aitType, excasSync, pvElements);
        if ( !pPVI ) {
        	AssertErrorCondition(InitialisationError,
            		"EPICSHandler::ObjectLoadSetup: %s: cannot create pvInfo structure for %s. Fatal error!",
            		Name(), pvName.Buffer() );
            return False;
        }
        // create the real process variable
        exPV * pPV = pPVI->createPV(*pCAS, true, scanOn, asyncDelay);
        if (!pPV) {
          	AssertErrorCondition(InitialisationError,
                		"EPICSHandler::ObjectLoadSetup: %s: cannot create exPV structure for %s. Fatal error!",
                		Name(), pPVI->getName() );
          	return False;
        }
        
        // Install canonical (root) name
        char pvAlias[256];
        const char * const pNameFmtStr = "%.100s%.20s";
        sprintf(pvAlias, pNameFmtStr, pvPrefix.Buffer(), pPVI->getName());
        pCAS->installAliasName(*pPVI, pvAlias);
        
        // add the process variable to the list by copy and delete the current object
        pvList[i] = pPVI; // copy the process variable to the list (by pointer)
        //--------------------------------------------------------------------- end of EPICS stuff
        
             
        cdb->MoveToFather();
    } // PVs loop

    cdb->MoveToFather();
    
    // Publish Subscribe Interface(TODO) stuff
    if ( subList )
    	delete subList;
    subList = new subscriber [numberOfPVs];
    if ( !subList ) {
       	AssertErrorCondition(InitialisationError,
                		"EPICSHandler::ObjectLoadSetup: %s: cannot create subList. Fatal error!",
                		Name() );
    		return False;    	
    }
    // TODO is this the correct way to initialize the array?!
    memset ( subList, 0, (numberOfPVs * sizeof(subscriber)) );

    // preparing the buffering infrastructure - this part will be moved in the abstract class
    // in the Process Variable loop we can calculate the minimum size required to work
    unsigned _size = buffer_size % buffer_align;
    if ( _size ) {
    	buffer_size += buffer_align - _size;
       	AssertErrorCondition(Information,
       			"EPICSHandler::ObjectLoadSetup: %s: circular buffer resized to %x (aligned)",
       			Name(), buffer_size);
    }
    // check if buffer was previously allocated
    if ( buffer_ptr )
    	delete [] buffer_ptr;
    buffer_ptr = new char [buffer_size];
    if ( !buffer_ptr ) {
      	AssertErrorCondition(InitialisationError,
            	"EPICSHandler::ObjectLoadSetup: %s: cannot allocate space for the circular buffer. Fatal error!",
            	Name() );
		return False;
    }
    // set head and tail and free bytes    
    buffer_head	= (cbHeader * ) buffer_ptr;
    buffer_tail = (cbHeader * ) buffer_ptr;
    buffer_free = buffer_size; 

// ***** circular buffer rules *****
    // head è sempre in una posizione libera (quando e full?)
    // tail è sempre nel primo elemento da mangiare tranne quando il ciruclar buffer e vuoto
    
    
    // check if the threads are already running: kill them and then reload them
    // TODO
    // in a single MARTe instance must exists only one instance of the "callback" thread
    // but you must register one instance of "callback_event" for every instance of this class
    
    
    // create the asynchronous EPICS thread --> from EPICS to MARTe
    callback_finalize++;
	threadID = Threads::BeginThread( (ThreadFunctionType) callback,
												pCAS, THREADS_DEFAULT_STACKSIZE,
												"EPICSHandler", XH_NotHandled, cpuMask);
	if ( !threadID ) {
      	AssertErrorCondition(InitialisationError,
            		"EPICSHandler::ObjectLoadSetup: %s: cannot create EpicsThreadCallback. Fatal error!",
            		Name() );
		return False;
	}
/*    AssertErrorCondition(Information,
    		"EPICSHandler::ObjectLoadSetup: %s: Service started with tid 0x%08x.",
    		Name(), threadID);
*/
    // create the asynchronous MARTe thread --> from MARTe to EPICS
	callback_event_finalize++;
	threadID_event = Threads::BeginThread( (ThreadFunctionType) callback_event,
												this, THREADS_DEFAULT_STACKSIZE,
												"EPICSHandler_events", XH_NotHandled, cpuMask_event);
	if ( !threadID_event ) {
      	AssertErrorCondition(InitialisationError,
            		"EPICSHandler::ObjectLoadSetup: %s: cannot create eventBufferCallback. Fatal error!",
            		Name() );
		return False;
	}
	
/*    AssertErrorCondition(Information,
    		"EPICSHandler::ObjectLoadSetup: %s: Service started with tid 0x%08x.",
    		Name(), threadID_event);
*/
	setup_complete = true;
    AssertErrorCondition(Information,
    		"EPICSHandler::ObjectLoadSetup: %s: Loaded %d Process Variables.",
    		Name(), numberOfPVs);
    return True;
}
//----------------------------------------------------------------------------- end ObjectLoadSetup

// TODO use CDBExtended::ReadOptions instead
aitEnum EPICSHandler::ConvertToaitEnum(const char * s) {
	int i;
	for (i=0; i<(sizeof(aitEnum_strings)/sizeof(char*)); i++ ) {
		if ( strcmp(aitEnum_strings[i], s)==0 )
		//if ( strcasecmp(aitEnum_string[i], s)==0 )
			return (aitEnum)i;
	}
	return aitEnumInvalid; //on error returns 0 (aitEnumInvalid)
}
//----------------------------------------------------------------------------- end ConvertToaitEnum

// TODO use CDBExtended::ReadOptions instead
excasIoType EPICSHandler::ConvertToexcasIoType(const char * s) {
	int i;
	for (i=0; i<(sizeof(excasIoType_strings)/sizeof(char*)); i++ ) {
		if ( strcmp(excasIoType_strings[i], s)==0 )
		//if ( strcasecmp(aitEnum_string[i], s)==0 )
			return (excasIoType)i;
	}
	return excasIoSync; //on error returns 0 (excasIoSync)
}
//----------------------------------------------------------------------------- end ConvertToexcasIoType

int EPICSHandler::callback_finalize = 0;
void EPICSHandler::callback (void * args) {	
	
	//osiTime delay(1000u, 0u);
	// TODO ?
	
	while( callback_finalize )
		fileDescriptorManager.process(0.1); // double seconds (timeout)
}
//----------------------------------------------------------------------------- end EPICSCallback


#define SYNCHRONIZING
int EPICSHandler::callback_event_finalize= 0;
void EPICSHandler::callback_event (void * args) {	
	unsigned id;
	unsigned timestamp;
	int _ret;
	EPICSHandler * THIS = static_cast<EPICSHandler *>(args);
	
	epicsTimeStamp current = epicsTime::getCurrent();
	
	while( callback_event_finalize ) {
		// if there are no subscribers wait a bit and continue loop
		if ( !(THIS->subListSize) ) {
			SleepMsec(100);
			continue;
		}

		// collect data from the producer
		_ret = THIS->get ( id, THIS->subBuffer, THIS->subSize, timestamp);
		if ( _ret == 0 ) // no data, no problem
			continue;
		if ( _ret == -1 ) { // error
			CStaticAssertErrorCondition(FatalError,
					"EPICSHandler::callback_event: thread: get returns %d",
					_ret );
			// TODO read buffer_err to write which kind of error happens
			break; }
		// check id
		if ( !(id < THIS->numberOfPVs) ) { // error again
			CStaticAssertErrorCondition(Warning,
					"EPICSHandler::callback_event: thread: id %d not in PVs range (0..%d)",
					id, THIS->numberOfPVs);
			continue; }
		
		// conversion is done by EPICS side (i.e. GDD)
		smartGDDPointer pDD;
		
		switch ( (THIS->subList[id].type).Type() ) {
		case BTDTInteger:
			//TODO switch on flag
			switch ( (THIS->subList[id].type).BitSize() ) {
/* not supported by the DDB
 * 			case 8:
				if ( (THIS->subList[id].count) == 1 ) // scalar
        			pDD = new gddScalar (gddAppType_value, aitEnumInt8);
        		else // --------------------------- vector
        			pDD = new gddAtomic (gddAppType_value, aitEnumInt8, 1u, THIS->subList[id].count);	        		
        		break;
			case 16:
	        	if ( (THIS->subList[id].count) == 1 ) // scalar
	        		pDD = new gddScalar (gddAppType_value, aitEnumInt16);
	        	else // --------------------------- vector
	        		pDD = new gddAtomic (gddAppType_value, aitEnumInt16, 1u, THIS->subList[id].count);	        		
				break;
*/
			case 32:
				switch ( (THIS->subList[id].type).Flags() ) {
				case BTDSTUnsigned:
		        	if ( (THIS->subList[id].count) == 1 ) // scalar
		        		pDD = new gddScalar (gddAppType_value, aitEnumUint32);
		        	else // --------------------------- vector
		        		pDD = new gddAtomic (gddAppType_value, aitEnumUint32, 1u, THIS->subList[id].count);
		        	break;
				case BTDSTNone:
					if ( (THIS->subList[id].count) == 1 ) // scalar
						pDD = new gddScalar (gddAppType_value, aitEnumInt32);
					else // --------------------------- vector
						pDD = new gddAtomic (gddAppType_value, aitEnumInt32, 1u, THIS->subList[id].count);
					break;
				default:
				   	BString bsbuf;
				   	CStaticAssertErrorCondition(Warning,
							"EPICSHandler::callback_event: thread: BTDTInteger, %s BasicTypeDescriptor not supported for id %d (%s) (32bit)",
							(THIS->subList[id].type).ConvertToString(bsbuf), id,
							(THIS->subList[id].count > 1) ? "array" : "scalar" );
				}
				break;
			default:
				// in this case it is EPICS that does not support Int64 (MARTe supports it already)
			   	BString bsbuf;
			   	CStaticAssertErrorCondition(Warning,
						"EPICSHandler::callback_event: thread: BTDTInteger, %s BasicTypeDescriptor not supported for id %d (%s)",
						(THIS->subList[id].type).ConvertToString(bsbuf), id,
						(THIS->subList[id].count > 1) ? "array" : "scalar" );
			}
			break;
		case BTDTFloat:
			switch ( (THIS->subList[id].type).BitSize() ) {
			case 32:
				if ( (THIS->subList[id].count) == 1 ) // scalar
					pDD = new gddScalar (gddAppType_value, aitEnumFloat32);
	        	else // --------------------------- vector
	        		pDD = new gddAtomic (gddAppType_value, aitEnumFloat32, 1u, THIS->subList[id].count);
				break;
			case 64:
				if ( (THIS->subList[id].count) == 1 ) // scalar
					pDD = new gddScalar (gddAppType_value, aitEnumFloat64);
	        	else // --------------------------- vector
	        		pDD = new gddAtomic (gddAppType_value, aitEnumFloat64, 1u, THIS->subList[id].count);
				break;
			default:
	        	BString bsbuf;
				CStaticAssertErrorCondition(Warning,
		    			"EPICSHandler::callback_event: thread: BTDTFloat, %s BasicTypeDescriptor not supported for id %d (%s)",
						(THIS->subList[id].type).ConvertToString(bsbuf), id,
						(THIS->subList[id].count > 1) ? "array" : "scalar" );
			}
			break;
		default:
			// error not supported by the driver
        	BString bsbuf;
			CStaticAssertErrorCondition(Warning,
	    			"EPICSHandler::callback_event: thread: %s BasicTypeDescriptor not supported for id %d (%s)",
					(THIS->subList[id].type).ConvertToString(bsbuf), id,
					(THIS->subList[id].count > 1) ? "array" : "scalar" );
			continue;
		} //------------------------------------------------------------------- end switch ( (THIS->subList[id].type).Type() )

		if ( !pDD.valid() ) {
			CStaticAssertErrorCondition(Warning,
					"EPICSHandler::callback_event: thread: pDD not valid cannot continue (id is %d, ->count is %d)",
					id, THIS->subList[id].count);
			continue;
		}
		
	    gddStatus gdds= pDD->unreference ();
	    if ( gdds )  {// assert ( ! gddStatus );
	    	CStaticAssertErrorCondition(Warning,
	    			"EPICSHandler::callback_event: thread: assert (!gddStatus) cannot continue"
	    			);
	    	continue;
		}

	    // copy data scalar or array 
		switch ( (THIS->subList[id].type).Type() ) {
		case BTDTInteger:
			switch ( (THIS->subList[id].type).BitSize() ) {
/* THE FOLLOWINGS ARE NOT SUPPORTED BY the DDB :-(
 * 			case 8:
        		if ( (THIS->subList[id].count) == 1 ) // scalar
        			pDD->put( (aitInt8) * (aitInt8 *)(THIS->subBuffer) );
        		else { //-------------------------- vector
	        		aitInt8 * pI8 = new aitInt8 [THIS->subList[id].count];
	        		if ( !pI8 )
	        			goto assert_allocation_error;
	        		// create destructor
	        		int8Destructor * pBTDFd = new int8Destructor;
	        		if ( !pBTDFd ) {
	        			delete [] pI8;
	        			goto assert_destructor_error;
	        		}
	        		// set the data
	        		pDD->putRef (pI8, pBTDFd);
	        		// copy array data
	        		memcpy(pI8, THIS->subBuffer, THIS->subList[id].size);	        			        			
	        	}  		
        		break;
			case 16:
        		if ( (THIS->subList[id].count) == 1 ) // scalar
        			pDD->put( (aitInt16) * (aitInt16 *)(THIS->subBuffer) );
        		else { //-------------------------- vector
	        		aitInt16 * pI16 = new aitInt16 [THIS->subList[id].count];
	        		if ( !pI16 )
	        			goto assert_allocation_error;
	        		// create destructor
	        		int16Destructor * pBTDFd = new int16Destructor;
	        		if ( !pBTDFd ) {
	        			delete [] pI16;
	        			goto assert_destructor_error;
	        		}
	        		// set the data
	        		pDD->putRef (pI16, pBTDFd);
	        		// copy array data
	        		memcpy(pI16, THIS->subBuffer, THIS->subList[id].size);	        			        			
	        	}  		
        		break;
        		*/        		
			case 32:
				switch ( (THIS->subList[id].type).Flags() ) {
				case BTDSTUnsigned:
	        		if ( (THIS->subList[id].count) == 1 ) // scalar
	        			pDD->put( (aitUint32) * (aitUint32 *)(THIS->subBuffer) );
	        		else { //-------------------------- vector
		        		aitUint32 * pI32 = new aitUint32 [THIS->subList[id].count];
		        		if ( !pI32 )
		        			goto assert_allocation_error;
		        		// create destructor
		        		int32Destructor * pBTDFd = new int32Destructor;
		        		if ( !pBTDFd ) {
		        			delete [] pI32;
		        			goto assert_destructor_error;
		        		}
		        		// set the data
		        		pDD->putRef (pI32, pBTDFd);
		        		// copy array data
		        		memcpy(pI32, THIS->subBuffer, THIS->subList[id].size);	        			        			
		        	}  							
					break;
				case BTDSTNone:
	        		if ( (THIS->subList[id].count) == 1 ) // scalar
	        			pDD->put( (aitInt32) * (aitInt32 *)(THIS->subBuffer) );
	        		else { //-------------------------- vector
		        		aitInt32 * pI32 = new aitInt32 [THIS->subList[id].count];
		        		if ( !pI32 )
		        			goto assert_allocation_error;
		        		// create destructor
		        		int32Destructor * pBTDFd = new int32Destructor;
		        		if ( !pBTDFd ) {
		        			delete [] pI32;
		        			goto assert_destructor_error;
		        		}
		        		// set the data
		        		pDD->putRef (pI32, pBTDFd);
		        		// copy array data
		        		memcpy(pI32, THIS->subBuffer, THIS->subList[id].size);	        			        			
		        	}  		
					break;
				//default:
				}	
        		break;      
			//default:
			}
			break;
		case BTDTFloat:
			switch ( (THIS->subList[id].type).BitSize() ) {
			case 32:
        		if ( (THIS->subList[id].count) == 1 ) // scalar
        			pDD->put( (aitFloat32) * (aitFloat32 *)(THIS->subBuffer) );
        		else { //-------------------------- vector
	        		aitFloat32 * pF32 = new aitFloat32 [THIS->subList[id].count];
	        		if ( !pF32 )
	        			goto assert_allocation_error;
	        		// create destructor
	        		float32Destructor * pBTDFd = new float32Destructor;
	        		if ( !pBTDFd ) {
	        			delete [] pF32;
	        			goto assert_destructor_error;
	        		}
	        		// set the data
	        		pDD->putRef (pF32, pBTDFd);
	        		// copy array data
	        		memcpy(pF32, THIS->subBuffer, THIS->subList[id].size);	        			        			
	        	}  		
        		break;
			case 64:
        		if ( (THIS->subList[id].count) == 1 ) // scalar
        			pDD->put( (aitFloat64) * (aitFloat64 *)(THIS->subBuffer) );
        		else { //-------------------------- vector
	        		aitFloat64 * pF64 = new aitFloat64 [THIS->subList[id].count];
	        		if ( !pF64 )
	        			goto assert_allocation_error;
	        		// create destructor
	        		float64Destructor * pBTDFd = new float64Destructor;
	        		if ( !pBTDFd ) {
	        			delete [] pF64;
	        			goto assert_destructor_error;
	        		}
	        		// set the data
	        		pDD->putRef (pF64, pBTDFd);
	        		// copy array data
	        		memcpy(pF64, THIS->subBuffer, THIS->subList[id].size);	        			        			
	        	}  		
        		break;
			}
			break;
		default:
	        CStaticAssertErrorCondition(FatalError,
	        		"EPICSHandler::callback_event: static method: %d This point cannot be reached",
	        		id );
	        continue;
		}	
		goto update;
		
assert_allocation_error: {
		BString bsbuf;
		CStaticAssertErrorCondition(Warning,
				"EPICSHandler::callback_event: static method: assert array allocation error %s for id %d",
				(THIS->subList[id].type).ConvertToString(bsbuf), id );
		continue;
}
	
assert_destructor_error: {
		BString bsbuf;
		CStaticAssertErrorCondition(Warning,
				"EPICSHandler::callback_event: static method: assert array destructor error %s for id %d",
				(THIS->subList[id].type).ConvertToString(bsbuf), id );
		continue;
}
	
update:
		//convert to EPICS timestamp (DDB time is usec)
		long nsec = current.nsec + ((timestamp % 1000000) *1000);
		long sec = nsec / 1000000000;
		if (current.nsec < nsec)
			sec++;
		sec += current.secPastEpoch;
		//add timestamp
	 	aitTimeStamp gddts ( sec , nsec);
	 	pDD->setTimeStamp ( & gddts );

		// update value!
		//status = (pvList[id]->getPV())->update ( *pDD );
	 	//if ( !(THIS->pvList[id]->getScanPeriod() > 00) )
	 	(THIS->pvList[id]->getPV())->update ( *pDD,
	 			(THIS->pvList[id]->getScanPeriod() > 00) ? false : true,
	 			true );
	} //----------------------------------------------------------------------- end while ( callback_event_finalize )
}//---------------------------------------------------------------------------- end MARTeCallback


bool EPICSHandler::subscribe ( const char * nameIn, BasicTypeDescriptor typeIn, int countIn, unsigned &idOut) {
	if ( !setup_complete )
		return False;
	
	// qui e gia stato aggiunto sia sulla lista che sull'hash table..
	// ottimo per cerarlo by name e tornare l'id della lista.. (come ottenere l'id?)
    char pvAlias[256];
    const char * const pNameFmtStr = "%.100s%.20s";
    
    pvExistReturn _exist = (this->pCAS)->pvExistTest(nameIn);
    if ( _exist.getStatus() == pverDoesNotExistHere ) {
     	AssertErrorCondition(FatalError,
            		"EPICSHandler::subscribe: %s: Process Variable %s does NOT exist in the lookup table",
            		Name(), nameIn );
		return False;
    }
    
	idOut = -1;	
	for (int i=0; i<numberOfPVs; i++) {
		sprintf(pvAlias, pNameFmtStr, pvPrefix.Buffer(), pvList[i]->getName() );	
		if ( strcmp(nameIn, pvAlias) == 0) {//string match
			idOut = i;
			break;
		}
	}
	if (idOut == -1) // variable not found -> return false
		return False;

	// Process Variable already subscribed? -> error
	if ( subList[idOut].count != 0 ) {
     	AssertErrorCondition(ParametersError,
            		"EPICSHandler::subscribe: %s: Process Variable %s already subscribed",
            		 Name(), nameIn);
		return False;
	}
		
	// if typeIn has no type (i.e. BTDTNone) has no sense to subscribe on it or also to write it
	if ( typeIn == BTDTNone ) {
     	AssertErrorCondition(ParametersError,
            		"EPICSHandler::subscribe: %s: Process Variable %s type BTDTNone not valid!",
            		Name(), nameIn);
		return False;
	}
	
	// TODO the PV creation must be done here or in another place? avrebbe senso crearla
	// solo se qualcuno ci si iscrive..
	// altrimenti la registri cmoe eusbscirber e ne registri il tipo e quindi il numeo di byte da copiare
	//ricordiamoci che countin e il numero di elementi dell'array.
	// se facciamo che la PV viene creata solo se sottoscritta allora non serve avere una variabile 
	// per sapere se e sottoscritta (una sottoscrizione per variabile) se è creata allora ok
	
	// check if the buffer has enough space to hold the data container (+ header)
	int _size = (typeIn.ByteSize() * countIn);
	if ( buffer_size < (sizeof(cbHeader) + _size) ) {
     	AssertErrorCondition(ParametersError,
            		"EPICSHandler::subscribe: %s: Process Variable %s circular buffer is too small!",
            		 Name(), nameIn);
		return False;
	}
	
	// check if the process variable has at least the number of elements of the DDB array
	// otherwise signal an error (EPICS code checks for that, so we maintain the same)
	if ( countIn > pvList[idOut]->getElementCount()  ) {
		AssertErrorCondition(ParametersError,
			   "EPICSHandler::subscribe: %s: Process Variable %s holds %d elements, less then %d required",
			   Name(), nameIn, pvList[idOut]->getElementCount(), countIn);
		return False;
	}
	
	subList[idOut].size = _size;
	subList[idOut].type = typeIn;
	subList[idOut].count = countIn;
	subList[idOut].statisticGet = 0;
	subList[idOut].statisticPut = 0;
	
	subListSize++;
	
	// if the buffer was allocated but is too small delete and allocate
	if (subSize < _size) {
		if ( subBuffer ) // consider when subBuffer is 0 (never allocated)
			delete subBuffer;
		subSize =_size;
		subBuffer = (char *) malloc (subSize);		
	}

	return True;
}
//----------------------------------------------------------------------------- end subscribe

bool EPICSHandler::unsubscribe (unsigned idIn) {
	if ( !(idIn < numberOfPVs) )
		return False; // index out of bounds
	
	// TODO vedi sopra se decidiamo di aggiungerla al server solo quando facciamo subscribe allora
	// qui in unsubscribe lo dobbiamo togliere dal server (che sarebbe la migliore idea)
	// per il momento il thread è always running!
	if ( subList[idIn].count <= 0 ) {
     	AssertErrorCondition(ParametersError,
            		"EPICSHandler::subscribe: %s: Process Variable id %d is not subscribed!",
            		idIn, Name() );
		return False; // no one is subscribed
	}

	subList[idIn].type = BTDTNone;
	subList[idIn].count = 0;
	subListSize--;
	
	return True;
}
//----------------------------------------------------------------------------- end unsubscribe

// REALTIME - put -> head
int EPICSHandler::put (unsigned idIn, void * bufferIn, unsigned timestamp) {
	if ( !(idIn < numberOfPVs) )
		return -1; // index out of bounds
	if ( !bufferIn )
		return -1; // buffer is not a valid pointer
	
	int _size = subList[idIn].size + sizeof(cbHeader);
	if ( buffer_free < _size )
		return 0; // check if there is room for data
	                  // if there is no room the user can retry many times

	// copy data to circular buffer handling cross bounding
	cbHeader _tmpHeader = { CB_HEADER_MAGIC, idIn, timestamp};	
	long offset = (char *) buffer_head - buffer_ptr;
	int len = (buffer_size - offset);
	
	// write the header and data in the circular buffer
	if ( len < sizeof(cbHeader) ) {
		memcpy ((char *)buffer_head, (char *)&_tmpHeader, len ); // fill last bytes of the buffer
		buffer_head = (cbHeader *) buffer_ptr; // rewind the buffer
		memcpy ((char *)buffer_head, (((char *)&_tmpHeader) + len), sizeof(cbHeader) - len);
		buffer_head = (cbHeader *) (buffer_ptr + (sizeof(cbHeader) - len));
		
		// now I can copy the data , for sure not sliced
		memcpy((char*)buffer_head, bufferIn, subList[idIn].size);
		buffer_head = (cbHeader *)((char *)buffer_head + subList[idIn].size); // always subList .size
	}
	else {		
		memcpy ((char *)buffer_head, (char *)&_tmpHeader, sizeof(cbHeader) ); // fill last bytes of the buffer
		buffer_head = (cbHeader *) (((char *)buffer_head) + sizeof(cbHeader));
		if ( (len - sizeof(cbHeader)) == 0) {
			buffer_head = (cbHeader *)buffer_ptr; // rewind the buffer
			len = subList[idIn].size; 
		}
		else
			len -= sizeof(cbHeader);
		
		//check if buffer must be sliced
		if ( len < subList[idIn].size ) {
			memcpy ((char *)buffer_head, bufferIn, len ); // fill last bytes of the buffer
			buffer_head = (cbHeader *)buffer_ptr; // rewind the buffer
			memcpy ((char *)buffer_head, ((char *)bufferIn + len), (subList[idIn].size - len) );
			buffer_head = (cbHeader *)(buffer_ptr + (subList[idIn].size - len) );			
		}
		else {
			memcpy((char*)buffer_head, bufferIn, subList[idIn].size);
			buffer_head = (cbHeader *)((char *) buffer_head + subList[idIn].size);
		}
	}

	//buffer_free -= _size;
	Atomic::Sub(&buffer_free, _size);
	subList[idIn].statisticPut++;
	
	//synchronizza EPICS !! semaphore del get! (next version)
#ifdef SYNCHRONIZING
		sem.Post();
#endif
	
	return subList[idIn].size;
}
//----------------------------------------------------------------------------- end put

/*
 * you can use get with sizeIn 0 to query about the status of the first buffer
 * in the sense you want know the id to provide the correct buffer size
 * 
 * temporaneamente facciamo che:
 * idOut viene scritto dalla routine "get" bufferIn dev'essere valido
 * sizeIn != 0
 * l'idea e che esiste un'ulteriore routine che ti dice la size e l'id per il momento
 * e quindi passi il giusto buffer (con la giusta size) altrimenti
 * allochi un max buffer in subscribe.. or something similar.. cmq relato agli iscritti..
 * 
 * return the amount of bytes written - TODO make the same in method put
 * 
 * if size is less then the buffer just fill the buffer and move to the next buffer
 */
// REALTIME - get <- get
int EPICSHandler::get (unsigned &idOut, void * bufferIn, unsigned sizeIn, unsigned &timestamp) {
	if ( !bufferIn ) // buffer cannot be zero
		return -1;
	if ( !sizeIn ) // size cannot be zero
		return -1;

	// if there is no data in the buffer return 0
	if ( buffer_free == buffer_size )
#ifdef SYNCHRONIZING
		sem.Wait(); //ok, but we can also check for timeout here..
	if ( buffer_free == buffer_size )
		return 0;
#else
	{
		idOut = -1;
		return 0;
	}
#endif
	
	cbHeader _tmpHeader = {0, 0, 0};	
	long offset = (char *) buffer_tail - buffer_ptr;
	int len = (buffer_size - offset);
	int dataLen = 0;

	// fragmented header or payload around the circular buffer
	if (len < sizeof(cbHeader)) {
		memcpy ( (char *)&_tmpHeader, (char *)buffer_tail, len ); // fill last bytes of the buffer
		buffer_tail = (cbHeader *)buffer_ptr; // rewind the buffer
		memcpy ( (((char *)&_tmpHeader) + len), (char *)buffer_tail , (sizeof(cbHeader) - len) );
		buffer_tail = (cbHeader *)((char*)buffer_tail + (sizeof(cbHeader) - len) );
		
		//check data validity.. i.e. check MAGIC and id validity
		if ( (_tmpHeader.magic != CB_HEADER_MAGIC) )
			return -1; // corrupted buffer do nothing (nor realign or any thing else)
		if ( !(_tmpHeader.id < numberOfPVs) )  
			return -1; // corrupted id number cannot do nothing because cannot infere size
		idOut = _tmpHeader.id;
		timestamp = _tmpHeader.timestamp;
		
		//check how much data can be copied
		dataLen = (sizeIn < subList[idOut].size) ? sizeIn : subList[idOut].size;
		
		// now I can copy the data , for sure not sliced
		memcpy((char*)bufferIn, buffer_tail, dataLen);
		buffer_tail = (cbHeader *) ((char *)buffer_tail + subList[idOut].size);
	}
	else { // here at least the header is all together
		memcpy ((char *)&_tmpHeader, (char *)buffer_tail, sizeof(cbHeader) ); // fill last bytes of the buffer
		
		// check data validity.. MAGIC and id validity
		if ( (_tmpHeader.magic != CB_HEADER_MAGIC) )
			return -1; // corrupted buffer do nothing (nor realign or any thing else)
		if ( !(_tmpHeader.id < numberOfPVs) ) 
			return -1; // corrupted id number cannot do nothing because cannot infere size

		buffer_tail = (cbHeader *) ((char *)buffer_tail + sizeof(cbHeader));
		idOut = _tmpHeader.id;
		timestamp = _tmpHeader.timestamp;
		
		//check how much data can be copied
		dataLen = (sizeIn < subList[idOut].size) ? sizeIn : subList[idOut].size;		
		
		len -= sizeof(cbHeader);
		
		if ( len == 0) { // data is at the beginning of the data buffer
			memcpy(bufferIn, (char*)buffer_ptr, dataLen);
			buffer_tail = (cbHeader *)(buffer_ptr + subList[idOut].size);
		}
		else if ( len >= subList[idOut].size ) { // the data is contiguous
			memcpy(bufferIn, (char*)buffer_tail, dataLen);
			buffer_tail = (cbHeader *)((char *)buffer_tail + subList[idOut].size);
		}
		else { // segmented data
			memcpy (bufferIn, (char *)buffer_tail, (dataLen > len) ? len : dataLen ); // fill last bytes of the buffer
			buffer_tail = (cbHeader *)buffer_ptr; // rewind the buffer
			memcpy (((char*)bufferIn + len), (char *)buffer_tail, (dataLen > len) ? (dataLen - len) : 0);
			buffer_tail = (cbHeader *)(buffer_ptr + (subList[idOut].size -len));			
		}
	}

	//buffer_free += (sizeof(cbHeader) + subList[_tmpHeader.id].size); // one instruction beacause we hope to be atomic
	int _size = (sizeof(cbHeader) + subList[_tmpHeader.id].size); // one instruction beacause we hope to be atomic

/*	
	int32 * atomic_access = &buffer_free;
	int32 c, d;
	
	c = *atomic_access;
	d = Atomic::Exchange(atomic_access, (c+_size));
	if (c != d) {
		// something goes wrong
			d = Atomic::Exchange(atomic_access, (d+_size));
			c = (c+_size);
			if (c != d) {
				// something goes wrong again
				AssertErrorCondition(FatalError,
								"EPICSHandler::get: Circular Buffer Fatal error TWICE ERROR");
			}
			else
				AssertErrorCondition(FatalError,
								"EPICSHandler::get: Circular Buffer Fatal error ONE ERROR");
	}
*/
	Atomic::Add(&buffer_free, _size);
	subList[idOut].statisticGet++;

// comment non e proprio necessario, i.e. buffer_head/buffer_tail dovrebbero venir modificate in
	// nello stesso istante di buffer_free in modo tale che questo check funzioni
/*	
	char* a = (char*) buffer_head;
	int b = buffer_free;
	if ( ((((char*)buffer_tail -buffer_ptr)
			+ (buffer_size -b)) % buffer_size) !=
			(a -buffer_ptr) )
		AssertErrorCondition(FatalError,
				"EPICSHandler::get: Circular Buffer Fatal error _ptr %p, _head %p, _tail %p, size %d, free%d",
				buffer_ptr, a, buffer_tail, buffer_size, b);
*/
	
	// TODO invalidate cbHeader buffer
	//free the space and mark magic as not valid 0x0000
	
	return dataLen;
}
//----------------------------------------------------------------------------- end get

// TODO why not writing a buffer walker? :-)
		

const char* EPICSHandler::css = "table.bltable {"
	"margin: 1em 1em 1em 2em;"
	"background: whitesmoke;"
	"border-collapse: collapse;"
	"}"
	"table.bltable th, table.bltable td {"
	"border: 1px silver solid;"
	"padding: 0.2em;"
	"}"
	"table.bltable th {"
	"background: gainsboro;"
	"text-align: left;"
	"}"
	"table.bltable caption {"
	"margin-left: inherit;"
	"margin-right: inherit;"
	"}";
// ----------------------------------------------------------------------------

#define TABLE_NEWROW hStream.Printf("<tr>\n")
#define TABLE_ENDROW hStream.Printf("</tr>\n")

bool EPICSHandler::ProcessHttpMessage( HttpStream &hStream )
{
    hStream.SSPrintf("OutputHttpOtions.Content-Type","text/html");
    hStream.keepAlive = False;

    hStream.Printf("<html><head><title>%s</title>", Name());
    hStream.Printf( "<style type=\"text/css\">\n" );
    hStream.Printf("%s\n", css);
    hStream.Printf( "</style></head><body>\n" );
    
    //the following version numeration comes from
    //epics/base-3-14-11/src/cas/generic/caServerI.cc
    //EPICS/base-3-14-11/configure/CONFIG_BASE_VERSION
    //epics/base-3-14-11/src/misc/epicsRelease.c
    hStream.Printf("<h1> EPICSLib Version </h1>\n");
    hStream.Printf("Channel Access V%s\n", CA_VERSION_STRING ( CA_MINOR_PROTOCOL_REVISION ) );
    hStream.Printf("<BR>\n");
//    hStream.Printf("revision @(#) %s\n", EPICS_VERSION_STRING );
    hStream.Printf("%s\n", epicsReleaseVersion );
    hStream.Printf("<BR>\n");        

    hStream.Printf("<h1> EPICSLib Parameters </h1>\n");
    hStream.Printf("<table class=\"bltable\">\n");
    TABLE_NEWROW; hStream.Printf("<td> PV prefix </td> <td>%s</td>\n", pvPrefix.Buffer() ); TABLE_ENDROW;
    TABLE_NEWROW; hStream.Printf("<td> scan on </td> <td>%s</td>\n", (scanOn==true) ? "true" : "false"); TABLE_ENDROW; 
    TABLE_NEWROW; hStream.Printf("<td> async scan on </td> <td>%s</td>\n", (asyncScan==true) ? "true" : "false"); TABLE_ENDROW;
    TABLE_NEWROW; hStream.Printf("<td> async delay </td> <td>%lf</td>\n", asyncDelay); TABLE_ENDROW;
    TABLE_NEWROW; hStream.Printf("<td> max simultaneous async </td> <td>%d</td>\n", maxSimultAsyncIO); TABLE_ENDROW;
    TABLE_NEWROW; hStream.Printf("<td> PV number </td> <td>%d</td>\n", numberOfPVs); TABLE_ENDROW;
    hStream.Printf("</table>\n");
    hStream.Printf("<BR>\n");

    // per process variable description
    hStream.Printf("<h1> Detailed Process Variables description </h1>\n");
    hStream.Printf("<table class=\"bltable\">\n");
    // table header
	TABLE_NEWROW; hStream.Printf("<td>PV</td> <td>NAME</td> <td>EGU</td> <td>TYPE</td> <td>SYNC</td> "
			"<td>HOPR</td> <td>LOPR</td> <td>HIHI</td> <td>HIGH</td> <td>LOW</td> <td>LOLO</td> "
			"<td>LEN</td> <td>SCAN</td>\n"); 
	hStream.Printf("<td>PREC</td>"
			"<td>HHSV</td> <td>HSV</td> <td>LSV</td> <td>LLSV</td> "
			"<td>ACKS</td> <td>ACKT</td>");
	TABLE_ENDROW;	
    for (int pv=0; pv<numberOfPVs; pv++) {
    	// build the PV alias (prefix + name) 
        char pvAlias[256];
        const char * const pNameFmtStr = "%.100s%.20s";
        sprintf(pvAlias, pNameFmtStr, pvPrefix.Buffer(), pvList[pv]->getName());
    	// output the HTTP data 
    	TABLE_NEWROW;
    	hStream.Printf("<td>%s</td> <td>%s</td> <td>%s</td> <td>%s</td> <td>%s</td> " // pvAlias, pvName, pvUnits, pvType, pvSync
    			"<td>%lf</td> <td>%lf</td> <td>%lf</td> <td>%lf</td> <td>%lf</td> <td>%lf</td> " //hopr, lopr, hihi, high, low, lolo
    			"<td>%d</td>\n",
    			pvAlias,
    			pvList[pv]->getName(), pvList[pv]->getUnits(),  
    			aitEnum_strings[ (unsigned) pvList[pv]->getType() ],
    			excasIoType_strings[ (unsigned) pvList[pv]->getIOType() ],
    			pvList[pv]->getHopr(), pvList[pv]->getLopr(),
    			pvList[pv]->getHihi(), pvList[pv]->getHigh(),
    			pvList[pv]->getLow(), pvList[pv]->getLolo(),
    			pvList[pv]->getElementCount() );
    	
    	int index = 0;
    	do {
    		if ( menuScan_values[index] == pvList[pv]->getScanPeriod() )
    			break;
    		index++;
    	} while (menuScan_values[index] != 0);
    	if (menuScan_values[index] == 0)
    		index = 0;
    	hStream.Printf ("<td>%s</td>\n", menuScan_strings[index]);     			
/*    	TABLE_ENDROW;
    	// additional information regarding the same PV
    	TABLE_NEWROW;*/
    	hStream.Printf("<td>%d </td>" 
    			"<td>%s</td> <td>%s</td> <td>%s</td> <td>%s</td> " // - - hhsv hsv lsv llsv 
    			"<td>%s</td> <td>%s</td>\n", // acks ackt
    			pvList[pv]->prec,
    			menuAlarmSevr_strings[ pvList[pv]->hhsv ], menuAlarmSevr_strings[ pvList[pv]->hsv ],
    			menuAlarmSevr_strings[ pvList[pv]->lsv ], menuAlarmSevr_strings[ pvList[pv]->llsv ],
    			menuAlarmSevr_strings[ pvList[pv]->acks ], menuYesNo_strings [pvList[pv]->ackt ] );
    	TABLE_ENDROW;	

    }
    hStream.Printf("</table>\n");
    hStream.Printf("<BR>\n");

    
    // per process variable description
    hStream.Printf("<h1> Subscriber table (num of subscriber %d)</h1>\n", subListSize);
    hStream.Printf("<table class=\"bltable\">\n");
    // table header
    TABLE_NEWROW; hStream.Printf("<td>index</td> <td>elements</td> <td>element size</td> <td>DDB type</td> "
    		"<td>statistic Get</td> <td>statistic Put</td>"); TABLE_ENDROW;
    for (int si=0; si<numberOfPVs; si++) {
    	BString bsbuf;
    	
    	TABLE_NEWROW;
    	hStream.Printf("<td> %d</td> <td>%d </td><td> %d</td> <td>%s </td> <td>%d </td> <td>%d </td>",
    			si, subList[si].count, subList[si].size,
    			(subList[si].size == 0) ? "not assigned" : (subList[si].type).ConvertToString(bsbuf),
    			subList[si].statisticGet, subList[si].statisticPut);
    	TABLE_ENDROW;
    }
    hStream.Printf("</table>\n");
    hStream.Printf("<BR>\n");
    
   
    hStream.Printf("</body></html>");
    hStream.WriteReplyHeader(True);
    return True;
}
//----------------------------------------------------------------------------- end ProcessHttpMessage

OBJECTLOADREGISTER(EPICSHandler,"$Id: EPICSHandler.cpp,v 1.1 2011/06/17 14:19:12 abarb Exp $")
