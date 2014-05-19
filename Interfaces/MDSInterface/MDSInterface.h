/**

      
      $Log: MDSPlusHandler.h,v $
      Revision 1.4  2009/10/07 10:52:35  abarb
      

      Revision 1.1  2011/03/03 16:56:23  Gabriele
      notes

Class MDSInterface provides MDSplus support in MARTe. A single instance of MDSInterface is defined as a MARTe 
service and it will provide support for every MDSplus-aware Component (GAM or ACQGAM). 

Its functionality  consistes:
	1) Support for the storage of signals. A signal with any name can be declared during init() and then data can be
          stored in real time and are soon available for readout in the pulse file. Data storing will be carried out 
          in a separated thread possibly in a separate core in order not to interfere with realtime operation.
	2) Servicing of the following MDSplus events:
		2.1) Configuration Event: brings pulse and shot information, the ID of the associated MDSplus device and its start nid
		     for parameters. These will be read from the MDSplus tree and stored internally. Parameters are then available 
		     via static MDSInterface methods  Parameters will be also reported in the configuation file upon the receipt of the
		     Load Parameters Event (see below)
		2.2) Load Parameters Event: load in memory the current configuration and then recognizes GAMs containing the configuration line
		     MdsDevice = <idx>. For such GAMS, the parameters currenlty stored for the corresponding device idx will be added 
		     in the textual configuration definition. 
		2.3) State Machine advance Event: drives the MARTe State Machine


******************************************************************************/

#if !defined(MDS_INTERFACE_H)
#define MDS_INTERFACE_H

#include "HttpInterface.h"
#include "GCNamedObject.h"
#include "HttpStream.h"
#include "MessageHandler.h"
#include "MutexSem.h"
#include "EventSem.h"

#include <mdsobjects.h>
#include <vector>



using namespace MDSplus;

//Internal signal type definitions
#define USER_FLOAT 0
#define USER_DOUBLE 1
#define USER_INT 2
#define USER_BYTE 3

//Definition of the default MDSplus segment size in data storage.
#define DEFAULT_SEGMENT_SIZE 10000

//Maximum number of handled signals
#define MAX_NUM_SIGNALS 10000

class MdsEvent;

static struct EventDescr {
  const char *name;
  int code;
} evDescriptors[] = {{"PRE_REQ", 0x701}, {"ABORT", 0x704},
{"PULSE_REQ", 0x708}, {"POST_REQ", 0x709}, {"COLLECTION_COMPLETE", 0x703}};

OBJECT_DLL(MDSInterface)

/** Definition of the MARTe MDSInterface class.
    Static methods for MDSplus management available to MDSplus-aware GAMs. The dispatch to the GAM-specific data structure is carried out
    via the passed idx argument;
*/
class MDSInterface: public GCNamedObject, public HttpInterface, public MessageHandler {



OBJECT_DLL_STUFF(MDSInterface)

class DeviceHandler;
class SignalBuffer;

private:
    BString stateMachineName;
    BString mdsEventName;
    MdsEvent *mdsEvent;
    static const char *css;
    int getEventCode(char *name, int size);

    static int segmentSize ;
    int runOnCpu;

public:
///////////////////////////////////////////////////////////////////////////////
//                           CDBVirtual Interface                            //
///////////////////////////////////////////////////////////////////////////////    
    MDSInterface()
    {
	segmentSize = DEFAULT_SEGMENT_SIZE;
	treeWriter = 0;
	signalBufferQueue = new SignalBufferQueue();
    }
/** Destructor */
    ~MDSInterface();

/** Empty implemenation of ObjectSaveSetup */
    virtual     bool   ObjectSaveSetup(
            ConfigurationDataBase &     info,
            StreamInterface *           err){ return True;}

/** ObjectLoadSetup */
    virtual bool ObjectLoadSetup(ConfigurationDataBase& info,StreamInterface* err);

/** handle reception of MDSplus event, triggering parameter readout */
    void handleMdsEvent(Data *eventData);

/** Visualization of parameters via Web  */
    virtual bool ProcessHttpMessage( HttpStream &hStream );

/** Update the current configuration. Called in response to Load Parameters Event */
    void loadConfiguration();

/** Dynamic configuration load recursive method */
    void loadSubtree(CDBExtended &cdbx);

/** get parameter lists for selected device */
    std::vector<char *> getParameterNames(int deviceIdx);
    std::vector<MDSplus::Data *> getParameterValues(int deviceIdx);

//Static methods used by MDSplus-aware GAMS
/** Declare a new signal to be stored in MDSplus database. The first argument specifies the MDSplus device which will receive the 
  signal data. Signals are stored under the SIGNALS.USER subtree of the corresponding device tree. The method will return an internal
  integer id which will be used in real-time to report signal samples.   */ 
  static int declareSignal(int deviceIdx, char const * name, char const * description);
/** Methods for reporting signal samples in real-time, based on the id returned by declareSignal() method */
  static bool reportSignal(int deviceIdx, int userIdx, float val, float time);
/** Reset all out signal information */
  static void resetSignals(int deviceIdx);
 /** Force flushing of the last portion of the signal still in memory buffer */ 
  static bool flushSignals(int deviceIdx);

/** Get the dimension of the specified parameter, up to MAX_DIMS. Return true if parameter found, false otherwise */
  static bool getParameterDimensions(int deviceIdx, char *name, int &nDims, int *dims);
/** Get Scalar Parameters. return false if not found */
  static bool getIntParameter(int deviceIdx, char *name, int &retVal);
  static bool getDoubleParameter(int deviceIdx, char *name, double &retVal);
  static bool getFloatParameter(int deviceIdx, char *name, float &retVal);
/** Get the specified parameter. Memory for data and dims is allocated by the caller. If the parameter is Scalar return nDims = 0; 
    return NULL if parameter not found */ 
  static int *getIntArrayParameter(int deviceIdx, char *name);
  static double *getDoubleArrayParameter(int deviceIdx, char *name);
  static float *getFloatArrayParameter(int deviceIdx, char *name);

/** Preparation of realtime waveform sample readout. The returned integer id will be use to get interpolated waveform samples.
    -1 is returned if the waveform name is not found; */
  static int prepareWaveParameterReadout(int deviceIdx, char *name);
/** Realtine interpolated data readout */
  static double getWaveParameter(int deviceIdx, int waveId, double time);

class SignalBufferQueue;

/** Class SignalBuffer contains all the information about a current MDSplus segment. Floating point samples and times are managed */
class SignalBuffer
{
    int dimension, currDimension;
    float *samples, *times;
    MDSplus::TreeNode *targetNode;
    bool saved;
public:
    SignalBuffer(int dimension, MDSplus::TreeNode *targetNode);
    ~SignalBuffer();
/** Save (in realtime) a new (sample, time) pair */
    void putSample(float sample, float time);
/** Return saved flag */
    bool isSaved();
/** Set as saved (after write error) */
    void setSaved();
/** Save (in a separate non realtime core) the buffer content as a MDSplus segment */
    void save();
/** Report whether the bufer is full */
    bool isFull(); 
}; //End SignalBuffer

/** Class SignalBufferQueue handles SignalBuffer enqueueing up to theits MaxCapacity */
class SignalBufferQueue
{
    int capacity;
    SignalBuffer **buffers;
    int headIdx, tailIdx;
    MutexSem sharedSem;
    EventSem sharedEvent;
public:
    SignalBufferQueue(int capacity = 1024);
    ~SignalBufferQueue();
/** Enqueue (in realtime) a filled buffer for saving. If room available return 0, -1 otherwise */
    int putSignalBuffer(SignalBuffer *sigBuf);
/** Get (and Wait) the next enqueued buffer */
    SignalBuffer *getSignalBuffer();
}; //End SignalBufferQueue

/** Class TreeWriter handler writing signals in MDSplus tree in run time */
class TreeWriter
{
    SignalBufferQueue *queue;
    int runOnCpu;

public:
    TreeWriter(int runOnCpu, SignalBufferQueue *queue);
/** Start execution on a separate thread */
    void start();
/** Method for reading buffers and storing on tree */
    void run();
}; //End TreeWriter		


/** Inner class ParameterHanldler will manage all the parameters related to a fiven MDSplus device (identifier by its ID) */
  class DeviceHandler
  {
    MDSplus::Tree *tree;
    int rootSignalNid;
    int deviceIdx; //The corresponding identifier
    char *controlName;
    bool verbose; //If true print parameter messages
    std::vector<MDSplus::Data *> paramValues;
    std::vector<char *> paramNames;
    std::vector<char *> waveNames;
//Managenent of wave data to be carried out in realtime, so aviid using std::vect
    int *waveDims, *prevWaveIdxs;
    double **wavesX, **wavesY, **wavesDiff;
//Buffer management for output signals
    std::vector<char *>signalNames;
    std::vector<char *>signalDescriptions;
    std::vector<SignalBuffer *>signalBuffers;
    std::vector<SignalBuffer *>signalAltBuffers;
    std::vector<bool>isAltBuffer;
//Used to discard fake write outside pulses
    bool segmentsFlushed;



public:
    DeviceHandler(int deviceIdx, char *controlName, bool verbose = true);
/** Read configuration from the pulse file, called in response to a MDSplus Configuration event. 
    Return true of all parameters read, false otherwise */
    int getDeviceIdx() {return deviceIdx;}
    char *getControlName() {return controlName;}
    bool loadParameters(char *treeName, int shot, int rootParameterNid, int rootWaveNid, int rootSignalNid);
/** Get the dimension of the specified parameter, up to MAX_DIMS. Return true if parameter found, false otherwise */
    bool getParameterDimensions(char *name, int &nDims, int *dims);
/** Get Scalar Parameters. return false if not found */
    bool getIntParameter(char *name, int &retVal);
    bool getDoubleParameter(char *name, double &retVal);
    bool getFloatParameter(char *name, float &retVal);
/** Get the specified parameter. Memory for data and dims is allocated by the caller. If the parameter is Scalar return nDims = 0; 
    return NULL if parameter not found */ 
    int *getIntArrayParameter(char *name);
    double *getDoubleArrayParameter(char *name);
    float *getFloatArrayParameter(char *name);
/** Preparation of realtime waveform sample readout. The returned integer id will be use to get interpolated waveform samples.
    -1 is returned if the waveform name is not found; */
    int prepareWaveParameterReadout(char *name);
/** Realtine interpolated data readout */
    double getWaveParameter(int waveId, double time);
/** Get the names of the parameters */
    std::vector<char *> getParameterNames() {return paramNames;}
/** Get the values of the parameters */
    std::vector<MDSplus::Data *> getParameterValues() {return paramValues;}
//Signal store management
/** Declare a new signal to be stored in MDSplus database. Signals are stored under the SIGNALS subtree of the corresponding device tree. 
    The method will return an internal integer id which will be used in real-time to report signal samples.   */ 
    int declareSignal(char const * name, char const * description, int segmentSize);
/** Method for reporting signal samples in real-time, based on the id returned by declareSignal() method */
    bool reportSignal(int userIdx, float val, float time, SignalBufferQueue *queue);
/** Force flushing of the last portion of the signals still in memory buffer */ 
    void flushLastSegment(SignalBufferQueue *queue);
/** Reset all out signal information */
    void resetSignals();
/** Present it to browser */
    char  *getJavaScript(); 
/** Destructor */
    ~DeviceHandler();
  }; //End Inner class DeviceHandler

private:
    static std::vector<DeviceHandler *> deviceHandlers;
    static SignalBufferQueue *signalBufferQueue;
    static TreeWriter *treeWriter;


};

/** Definition of the MDSplus Event class for andling MDSPlus events */
class MdsEvent:public Event 
{ 
    MDSInterface *mdsInterface;
    public: 
      MdsEvent(char *name, MDSInterface *mdsInterface):Event(name)
      {
	  this->mdsInterface = mdsInterface; 
      }
    virtual void run();
};


#endif
