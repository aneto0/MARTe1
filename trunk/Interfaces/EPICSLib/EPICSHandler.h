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
#if !defined (EPICS_HANDLER)
#define EPICS_HANDLER

#include "System.h"
#include "GCNamedObject.h"
#include "HttpInterface.h"
#include "FString.h"
#include "CountSem.h"


//#include "epicsGuard.h"
#include "exServer.h"





//
// special gddDestructor guarantees same form of new and delete
//
class float32Destructor: public gddDestructor {
    virtual void run ( void * pUntyped ) {
        aitFloat32 * pf = reinterpret_cast < aitFloat32 * > ( pUntyped );
        delete [] pf;    	
    }
};
class float64Destructor: public gddDestructor {
    virtual void run ( void * pUntyped ) {
        aitFloat64 * pf = reinterpret_cast < aitFloat64 * > ( pUntyped );
        delete [] pf;    	
    }
};
class int8Destructor: public gddDestructor {
    virtual void run ( void * pUntyped ) {
        aitInt8 * pf = reinterpret_cast < aitInt8 * > ( pUntyped );
        delete [] pf;    	
    }
};
class int16Destructor: public gddDestructor {
    virtual void run ( void * pUntyped ) {
        aitInt16 * pf = reinterpret_cast < aitInt16 * > ( pUntyped );
        delete [] pf;    	
    }
};
class int32Destructor: public gddDestructor {
    virtual void run ( void * pUntyped ) {
        aitInt32 * pf = reinterpret_cast < aitInt32 * > ( pUntyped );
        delete [] pf;    	
    }
};

/*
// aitInt64 not supported by GDD
class int64Destructor: public gddDestructor {
    virtual void run ( void * pUntyped ) {
        aitInt64 * pf = reinterpret_cast < aitInt64 * > ( pUntyped );
        delete [] pf;    	
    }
};
*/




/*
void callback (void * args);
void callback_event (void * args);
*/

// nel buffer circolare ogni entry dev'essere casta in questo modo
// value è il contenitore dei dati.
//nel buffer non ce frammentazione perche sfruttiamo un buffer circolare


#define CB_HEADER_MAGIC 0xBEEF


typedef struct _cbHeader {
	uint16 magic;
	uint16 id;
	uint32 timestamp; // aggiunto con dolore per finire il progetto.. :-|
	uint32 value [];
} cbHeader;


typedef struct _subscriber {
	int count;
	int size;
	BasicTypeDescriptor type;
	int statisticPut;
	int statisticGet;
} subscriber;

OBJECT_DLL(EPICSHandler)
;

class EPICSHandler
: public GCNamedObject, public HttpInterface
{
	
OBJECT_DLL_STUFF(EPICSHandler)


//friend void callback (void * args);
//friend void callback_event (void * args);


private:
	//process variables prefix
	FString pvPrefix;
	
	bool setup_complete; // setup status (yes complete no nou complete)

	//debug level
	unsigned debugLevel;
	//scanOn true or false
	bool scanOn;
	//asyncScan true or false
	bool asyncScan;
	//asyncDelay
	double asyncDelay;
	//maxsimulasyncio
	unsigned maxSimultAsyncIO; 
	
	TID threadID;
	int32 cpuMask;

	TID threadID_event;
	int32 cpuMask_event;
	
    //Number of process variables
    int32 numberOfPVs;
    // PVs are registered in the pCAS
    exServer * pCAS;
    // list of PV to register to the pCAS
    pvInfo ** pvList ;

    
    // subscriber support list
    subscriber * subList;
    int subListSize;
    
    // buffer support data
    int buffer_size ;
    int buffer_align ;
    int32 buffer_free ;
    
    cbHeader * buffer_head;
    cbHeader * buffer_tail;
    
    int buffer_err;
    
    char * buffer_ptr;
    
    CountSem sem;
    TimeoutType timeout;
    
    char * subBuffer;
    unsigned subSize;
    
public:
	static const char * css;
	static const char * aitEnum_strings[];
	static const char * excasIoType_strings[];
	
	static const char * tf_strings[];
	static const int tf_values[];
	
	static const char * menuAlarmSevr_strings[];
	static const char * menuAlarmStat_strings[];
	static const char * menuScan_strings[];
	static const char * menuYesNo_strings[];
	
	static const int menu_values[];
	static const float menuScan_values[];
	
    /**
     * Converts a null terminated string to an aitEnum value
     */
    static aitEnum ConvertToaitEnum (const char * s );
    static excasIoType ConvertToexcasIoType(const char * s);
    	
   inline const exServer * getCaServer() const {
	   return pCAS;
   } ;
     
    // TODO
    EPICSHandler() {
    	setup_complete = false;
    	
        numberOfPVs = 0;
        pCAS = 0;
        pvList = 0;
        
        subList = 0;
        subListSize = 0;
        buffer_size = 0; 
        buffer_align = 0;
     
        buffer_head = 0;
        buffer_tail = 0;
        
        buffer_ptr = 0;
        
        buffer_err = 0; // last error issued by the circular buffer
        
        sem.Create();
        timeout = TTInfiniteWait;
        
        subBuffer = 0;
        subSize = 0;
    }

    // TODO
    virtual ~EPICSHandler(){
        //if(pvList != NULL){
        //    delete [] pvList;
        //}
    	setup_complete = false;
    	
    	callback_finalize--;
    	callback_event_finalize--;
    	
    	sem.Post();
    	sem.Close();
    	
    	// wait for event...
    	// devo aspettare che i thread escano prima di eliminare gli objects..
    	
    	if (pCAS)
    		delete pCAS;
    	
    	for(int i=0; i<numberOfPVs; i++)
    		delete pvList[i];
    	
    	if (pvList)
    		delete [] pvList;
    	
    	if (buffer_ptr)
    		delete [] buffer_ptr;
    	
    	if (subBuffer)
    		delete [] subBuffer;
    }

    /**
     * Load and configure object parameters
     * @param info the configuration database
     * @param err the error stream
     * @return True if no errors are found during object configuration
     */
    bool ObjectLoadSetup(ConfigurationDataBase &info,StreamInterface *err);

    /**
     * Http Page that reflects the state of the object
     */
    bool ProcessHttpMessage(HttpStream &hStream);
    
    /**
     * the data pool stayes in a circular memory buffer
     * symbolic name that identify a pv or a mdsplus tree...
     * type is the MARTe's basictype on which this signal appears on the DDB 
     * count is the number of elements of the same type (array support)
     * id is the PV's List entry index (handle)
     */
    
    // probabilmente sarebbe da tenere anche un puntatore all'oggetto che fa il subscribing
    // -> il numero di buckets dev'essere settato da cfg file
    // -> ogni bucket è della stessa grandezza, i bukets sono grandi come la union di tutti 
    // i tipi base
    bool subscribe ( const char * nameIn, BasicTypeDescriptor typeIn, int countIn, unsigned &idOut);
    int put (unsigned idIn, void * bufferIn, unsigned timestamp); // al massimo si può estendere per fare put multipli
    int get (unsigned &idOut, void * bufferIn, unsigned sizeIn, unsigned &timestamp); // anche in questo caso il buffer dev'essere pre allocato
    bool unsubscribe (unsigned id);

private:
    static void callback (void * args); //threadID [this must be static also]
    static void callback_event (void * args); //threadID_event 
    
    static int callback_finalize; //threadID
    static int callback_event_finalize; //threadID_event
};

#endif
