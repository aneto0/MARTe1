//*****************************************************************************
//    $Log: MDSSegments.cpp,v $
//    Revision 1.26  2009/10/16 08:05:45  abarb
//    added GAM casting instead of OutputGAM (using this BaseLib doesn't load it)
//
//    Revision 1.25  2009/10/15 16:59:19  abarb
//    *** empty log message ***
//
//
//*****************************************************************************

#include "MDSInterface.h"
#include "ObjectRegistryDataBase.h"
#include "MessageHandler.h"
#include "GlobalObjectDataBase.h"

#include <string.h>
#include <iostream>
#define DEBUG
OBJECTLOADREGISTER(MDSInterface,"$Id: MDSInterface.cpp,v 1.26 2009/10/16 08:05:45 gmandu Exp $")


//Semaphore and Event management for signal buffering management
static bool sharedSemCreated;
static MutexSem sharedSem;
static EventSem sharedEvent;

//Static MDSInterface class field
std::vector<MDSInterface::DeviceHandler *> MDSInterface::deviceHandlers;
MDSInterface::TreeWriter *MDSInterface::treeWriter;
int MDSInterface::segmentSize;
MDSInterface::SignalBufferQueue *MDSInterface::signalBufferQueue;

MDSInterface::~MDSInterface()
{
    for(int i = 0; i < deviceHandlers.size(); i++)
	delete deviceHandlers[i];
    deviceHandlers.clear();
    delete signalBufferQueue;
    if(treeWriter)
	delete treeWriter;
}

//Support methods and variables for signal streaming management
static void triggerSegment(Tree *tree, void *buffer, int type, int sampleSize, int numSamples, int nid, float *times);
static void handleSignalWrite(MDSInterface::TreeWriter *tw);  
static bool dataStorageThreadStarted = false; 

// ObjectLoasSetup. Read configuration parameters from confguration file
bool MDSInterface::ObjectLoadSetup(ConfigurationDataBase& info,StreamInterface* err) 
{
    CDBExtended cdbx(info);
//Create once for all synchronization stuff for signal buffer management
    if(!sharedSemCreated)
    {
	sharedSemCreated = true;
	sharedSem.Create();
	sharedEvent.Create();
    }

//Superclass' ObjectLoadSetup calls
    bool ret = GCNamedObject::ObjectLoadSetup(info,err);
    ret = ret && HttpInterface::ObjectLoadSetup(info,err);
    if(!ret) return False;

//Read the name in the configuration file of the MARTe StateMachine. It will be used to advance the MARTe State machine under MDSplus control.
    if(!cdbx.ReadBString(stateMachineName, "TargetStateMachine")){
      AssertErrorCondition(InitialisationError,"MDSInterface::ObjectLoadSetup: TargetStateMachine has not been specified. No channel will be written in the MDSplus tree");
      return False;
    }
//Read the name of the MDSplus event to listen for
    if(!cdbx.ReadBString(mdsEventName, "MdsEvent")){
        AssertErrorCondition(InitialisationError,"MDSInterface::ObjectLoadSetup: MdsEventName has not been specified. No channel will be written in the MDSplus tree");
    	return False;
    }
    const char *evName = mdsEventName.Buffer();
    mdsEvent = new MdsEvent((char *)evName, this);

//Start signal streaming thread and associate it with the core specified in RunOnCpu configuration parameter (mandatory)
    if(!treeWriter)
    {
	dataStorageThreadStarted = true;
	int runOnCpu;
	if(!cdbx.ReadInt32(runOnCpu, "ExecuteOnCpu", 1))
	{
	    AssertErrorCondition(InitialisationError,"MDSInterface::ObjectLoadSetup: ExecuteOnCpu has not been specified");
      	    return False;
	}
	treeWriter = new TreeWriter(runOnCpu, signalBufferQueue);
	treeWriter->start();
}

    return True;
}


bool MDSInterface::ProcessHttpMessage( HttpStream &hStream )
{

    hStream.SSPrintf("OutputHttpOtions.Content-Type","text/html");
    hStream.keepAlive = False;

    hStream.Printf("<html><head><title>%s</title>", Name());
    hStream.Printf( "<style type=\"text/css\">\n" );
    hStream.Printf("%s\n", css);
    hStream.Printf( "</style>\n" );
    hStream.Printf("<script type = \"text/javascript\">\n");
    for(int i = 0; i < deviceHandlers.size(); i++)
    {
      char *msg;
      hStream.Printf(msg = deviceHandlers[i]->getJavaScript());
      delete [] msg;
    }
    hStream.Printf("</script>");
    hStream.Printf( "</head><body>\n" );
    
    hStream.Printf("<h1> MDSInterface </h1>\n");
    hStream.Printf("<BR>\n");
    for(int i = 0; i < deviceHandlers.size(); i++)
    {
      if(deviceHandlers[i])
      {
	 hStream.Printf("<button type = \"button\" onclick=\"displayPars%d()\"> %s </button>", deviceHandlers[i]->getDeviceIdx(), deviceHandlers[i]->getControlName());
      }
    }
    hStream.Printf("<BR>\n");
    hStream.Printf("<p id = \"pars\"> Control Parameters </p>\n");
    hStream.Printf("<BR>\n");
    hStream.Printf("</body></html>");
    hStream.WriteReplyHeader(True);
    return True;
}
 


const char* MDSInterface::css = "table.bltable {"
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



void MdsEvent::run()
{
    int size;
    Data *data = getData();
    mdsInterface->handleMdsEvent(data);
    deleteData(data);
}






int MDSInterface::getEventCode(char *evName, int len)
{
    int i, code, nEvents;
    char *upName = new char[len+1];
    memcpy(upName, evName, len);
    upName[len] = 0;

    for(int i = 0; i < len; i++)
        upName[i] = toupper(upName[i]);
    nEvents = sizeof(evDescriptors)/sizeof(EventDescr);
    code = -1;
    for(i = 0; i < nEvents; i++)
    {
        if(!strcmp(evDescriptors[i].name, upName))
	{
	    code = evDescriptors[i].code;
	    break;
	}
    }
    delete [] upName;
    return code;
}

void MDSInterface::handleMdsEvent(Data *eventData)
{
    char eventName[16];
    char *eventBuf = ((Data *)eventData)->getString();
    sscanf(eventBuf, "%s", eventName);
    printf("Received Event %s\n", eventBuf);
    //Event SETUP defines the Tree and shot to be open and the nid of the field containing the configuation file (as a string array)
    if(!strcmp(eventName, "SETUP"))
    {
      	char treeName[256];
	char controlName[512];
      	int shot, parameterNid, waveParameterNid, signalNid, deviceIdx;
//Event data brinf information about tree name, tree shot, device Id, parameters root nid, signals root nid
      	sscanf(eventBuf, "%s %s %s %d %d %d %d %d", eventName, treeName, controlName, &shot, &deviceIdx, &parameterNid, &waveParameterNid, &signalNid);
	delete[] eventBuf;
	int handlerIdx = 0;
	for(handlerIdx = 0; handlerIdx < deviceHandlers.size(); handlerIdx++)
	{
	    if(deviceHandlers[handlerIdx]->getDeviceIdx() == deviceIdx)
		break;
	}
	if(handlerIdx == deviceHandlers.size())
	    deviceHandlers.push_back(new DeviceHandler(deviceIdx, controlName));
	deviceHandlers[handlerIdx]->loadParameters(treeName, shot, parameterNid, waveParameterNid, signalNid);
        return; //Error message already assered by method
    }
    if(!strcmp(eventName, "LOAD"))
    {
	loadConfiguration();
	return;
    }

    if(!strcmp(eventName, "STORE"))
    {
      int deviceIdx;
      sscanf(eventBuf, "%s %d", eventName, &deviceIdx);
      delete [] eventBuf;
      for(int i = 0; i < deviceHandlers.size(); i++)
      {
	  if(deviceHandlers[i]->getDeviceIdx() == deviceIdx)
      	      deviceHandlers[i]->flushLastSegment(signalBufferQueue); 
          return;
      }
    }
    if(!strcmp(eventName, "PRE_REQ"))
    {
      int deviceIdx;
      sscanf(eventBuf, "%s %d", eventName, &deviceIdx);
      delete[] eventBuf;
      for(int i = 0; i < deviceHandlers.size(); i++)
      {
	  if(deviceHandlers[i]->getDeviceIdx() == deviceIdx)
      	  	deviceHandlers[i]->resetSignals();
      }
    }
// If execution arrives here, this is a request for a state transition
    int evCode = getEventCode(eventName, strlen(eventName));
    MessageCode msgCode(evCode);
    GCRTemplate<Message> msg(GCFT_Create);
//    msg->Init(msgCode, eventBuf);
    msg->Init(msgCode, eventName);
    GCRTemplate<MessageEnvelope> msgEnvelope(GCFT_Create);
    msgEnvelope->PrepareMessageEnvelope(msg, stateMachineName.Buffer());
    MessageHandler::SendMessage(msgEnvelope);
}

/** get parameter lists for selected device */
std::vector<char *> MDSInterface::getParameterNames(int deviceIdx)
{
    std::vector<char *> res; //Empty
    for(int i = 0; i < deviceHandlers.size(); i++)
    {
	if(deviceHandlers[i]->getDeviceIdx() == deviceIdx)
	    return deviceHandlers[i]->getParameterNames();
    }
    return res;
}

std::vector<MDSplus::Data *> MDSInterface::getParameterValues(int deviceIdx)
{
    std::vector<MDSplus::Data *> res; //Empty
    for(int i = 0; i < deviceHandlers.size(); i++)
    {
	if(deviceHandlers[i]->getDeviceIdx() == deviceIdx)
	    return deviceHandlers[i]->getParameterValues();
    }
    return res;
}

/** Dynamic configuration load methods */
void MDSInterface::loadSubtree(CDBExtended &cdbx)
{
// Check if field MdsId is present
    int mdsId;
    if(cdbx.ReadInt32(mdsId, "MdsId"))
    {

//printf("LOAD SUBTREE found MDS Id = %d\n", mdsId);

	//Found the field for this subtree. This means that this is a MARTe component (GAM or IOGAM or even Service) using MDSplus parameters
	std::vector<char *> parNames = getParameterNames(mdsId);
	std::vector<MDSplus::Data *> parValues = getParameterValues(mdsId);
	for(int i = 0; i < parNames.size(); i++)
	{
	    char *parName = parNames[i];
//printf("PAR NAME: %s\n", parName);
	    if(!cdbx->Exists((const char *)parName))
	    {
//printf("Added\n");
	    	cdbx->AddChildAndMove(parName);
	    	cdbx->MoveToFather();
	    }
	    MDSplus::Data *currValue = parValues[i];
	    char *decompiled = currValue->decompile();
//printf("PAR Value: %s\n", decompiled);
	    cdbx.WriteString(decompiled, parName);
	    delete []decompiled;
	}
    } 
    else //Not a component holding MDSplus parameters, BUT whise descensdants may contain MDSplus parameters
    {
	for(int i=0; i < cdbx->NumberOfChildren(); i++)
	{
	    cdbx->MoveToChildren(i);
	    loadSubtree(cdbx);
	    //cdbx->moveToFather();
	    cdbx->MoveToFather();
	}
    }
}
	
	
/** Update the current configuration. Called in response to Load Parameters Event */
void MDSInterface::loadConfiguration()
{
// 1 Get latest config from MARTe 
//Ask MARTe for the last valid cdb
    GCRTemplate<MessageEnvelope> envelope(GCFT_Create);
    GCRTemplate<Message>         message(GCFT_Create);
    message->Init(0, "RetrieveLastConfiguration");
//    envelope->PrepareMessageEnvelope(message, MARTeLocation.Buffer(), MDRF_ManualReply, this);
    envelope->PrepareMessageEnvelope(message, "MARTe", MDRF_ManualReply, this);

    TimeoutType configTimeoutMs(1000);
    GCRTemplate<MessageEnvelope>   reply;
    SendMessageAndWait(envelope, reply, configTimeoutMs);
    if(!reply.IsValid()){
      AssertErrorCondition(InitialisationError,"MDSInterface::loadConfiguration : Reply from MARTe to configuration request isn't valid!");
      return;
    }
    GCRTemplate<Message> replyMessage = reply->GetMessage();
    if(!replyMessage.IsValid()){
       	AssertErrorCondition(InitialisationError,"MDSInterface::loadConfiguration : Reply message from MARTe to configuration request isn't valid!");
        return;
    }
    GCRTemplate<CDBVirtual> originalCDB = replyMessage->Find(0);
    if(!originalCDB.IsValid()){
        AssertErrorCondition(InitialisationError,"MDSInterface::loadConfiguration : Retrieved configuration from MARTe isn't valid!");
         return;
    }
// Change the CDB with whatever is received from MDSplus
    CDBExtended cdbx(originalCDB);
    cdbx->MoveToRoot();
    loadSubtree(cdbx);
//Send the new configuration to MARTe
    envelope->CleanUp();
    message->CleanUp();
    reply->CleanUp();
    message->Init(0, "ChangeConfigFile");
    message->Insert(cdbx);
    envelope->PrepareMessageEnvelope(message, "MARTe", MDRF_ManualReply, this);
    SendMessageAndWait(envelope, reply, configTimeoutMs);

    if(!reply.IsValid()){
        AssertErrorCondition(InitialisationError,"MDSInterface::loadConfiguration : Reply message from MARTe to configuration request change isn't valid!");
        return;
    }
    else{
        GCRTemplate<Message> replyMessage = reply->GetMessage();
        if(!replyMessage.IsValid()){
            AssertErrorCondition(InitialisationError,"MDSInterface::loadConfiguration : Reply message content from MARTe to configuration request change isn't valid!");
            return;
        }
        else{
            if(replyMessage->GetMessageCode() != 0){
                AssertErrorCondition(InitialisationError,"MDSInterface::loadConfiguration : Reply message content from MARTe to configuration was [%s] with code [%d]", replyMessage->Content(), replyMessage->GetMessageCode().Code());
                return;
            }
        }
    }
}


char *MDSInterface::DeviceHandler::getJavaScript()
{
    char *ans = new char[512 + 10024 * (paramNames.size() + waveNames.size())];

 //  sprintf(ans, "function displayPars%d() { document.getElementById(\"pars\").innerHTML= \"<table class=\\\"bltable\\\">\"", controlIdx);
    sprintf(ans, "function displayPars%d() { document.getElementById(\"pars\").innerHTML= \"<h2> Parameters </h2><br><table>\"", deviceIdx);
    for(int i = 0; i < paramNames.size(); i++)
    {
	strcpy(&ans[strlen(ans)], "+\"<tr>\"");
	char *decVal = paramValues[i]->decompile();
	sprintf(&ans[strlen(ans)], "+\"<tr><td>%s</td> <td>%s</td></tr>\"\n", paramNames[i], decVal);
	delete[] decVal;
    }
    strcpy(&ans[strlen(ans)], "+\"</table> <hr><h2> Wave Parameters</h2><br><table>\"");
    for(int i = 0; i < waveNames.size(); i++)
    {
	strcpy(&ans[strlen(ans)], "+\"<tr>\"");
	Data *currData = new Float64Array(wavesX[i], waveDims[i]);
	char *decXVal = currData->decompile();
	deleteData(currData);
	currData = new Float64Array(wavesY[i], waveDims[i]);
	char *decYVal = currData->decompile();
	deleteData(currData);
	sprintf(&ans[strlen(ans)], "+\"<tr><td>%s</td> <td>X: %s<br>Y: %s</td></tr>\"\n", waveNames[i], decXVal, decYVal);
	delete[] decXVal;
	delete[] decYVal;
    }
    strcpy(&ans[strlen(ans)], "+\"</table>\";}");
//printf("%s\n", ans);
    return ans;
}    




//Static methods used by MDSplus-aware GAMS
/** Reset stored signals information */
void  MDSInterface::resetSignals(int deviceIdx)
{
    return;
      for(int i = 0; i < deviceHandlers.size(); i++)
      {
	  if(deviceHandlers[i]->getDeviceIdx() == deviceIdx)
      	  	deviceHandlers[i]->resetSignals();
      }
}

/** Declare a new signal to be stored in MDSplus database. The first argument specifies the MDSplus device which will receive the 
  signal data. Signals are stored under the SIGNALS.USER subtree of the corresponding device tree. The method will return an internal
  integer id which will be used in real-time to report signal samples.   */ 
int  MDSInterface::declareSignal(int deviceIdx, char const * name, char const * description)
{
    for(int i = 0; i < deviceHandlers.size(); i++)
    {
	if(deviceHandlers[i]->getDeviceIdx() == deviceIdx)
	return deviceHandlers[i]->declareSignal(name, description, segmentSize);
    }
    return -1;
}	
	
/** Methods for reporting signal samples in real-time, based on the id returned by declareSignal() method */
bool MDSInterface::reportSignal(int deviceIdx, int userIdx, float val, float time)
{
    for(int i = 0; i < deviceHandlers.size(); i++)
    {
	if(deviceHandlers[i]->getDeviceIdx() == deviceIdx)
	return deviceHandlers[i]->reportSignal(userIdx, val, time, signalBufferQueue);
    }
    return false;
}	

 /** Force flushing of the last portion of the signal still in memory buffer */ 
bool MDSInterface::flushSignals(int deviceIdx)
{
    for(int i = 0; i < deviceHandlers.size(); i++)
    {
	if(deviceHandlers[i]->getDeviceIdx() == deviceIdx)
	deviceHandlers[i]->flushLastSegment(signalBufferQueue);
	return true;
    }
    return false;
}	

/** Get the dimension of the specified parameter, up to MAX_DIMS. Return true if parameter found, false otherwise */
bool MDSInterface::getParameterDimensions(int deviceIdx, char *name, int &nDims, int *dims)
{
    for(int i = 0; i < deviceHandlers.size(); i++)
    {
	if(deviceHandlers[i]->getDeviceIdx() == deviceIdx)
	return deviceHandlers[i]->getParameterDimensions(name, nDims, dims);
    }
    return false;
}
/** Get Scalar Parameters. return false if not found */
bool MDSInterface::getIntParameter(int deviceIdx, char *name, int &retVal)
{
    for(int i = 0; i < deviceHandlers.size(); i++)
    {
	if(deviceHandlers[i]->getDeviceIdx() == deviceIdx)
	return deviceHandlers[i]->getIntParameter(name, retVal);
    }
    return false;
}
bool MDSInterface::getDoubleParameter(int deviceIdx, char *name, double &retVal)
{
    for(int i = 0; i < deviceHandlers.size(); i++)
    {
	if(deviceHandlers[i]->getDeviceIdx() == deviceIdx)
	return deviceHandlers[i]->getDoubleParameter(name, retVal);
    }
    return false;
}

bool MDSInterface::getFloatParameter(int deviceIdx, char *name, float &retVal)
{
    for(int i = 0; i < deviceHandlers.size(); i++)
    {
	if(deviceHandlers[i]->getDeviceIdx() == deviceIdx)
	return deviceHandlers[i]->getFloatParameter(name, retVal);
    }
    return false;
}

/** Get the specified parameter. Memory for data and dims is allocated by the caller. If the parameter is Scalar return nDims = 0; 
    return NULL if parameter not found */ 
int *MDSInterface::getIntArrayParameter(int deviceIdx, char *name)
{
    for(int i = 0; i < deviceHandlers.size(); i++)
    {
	if(deviceHandlers[i]->getDeviceIdx() == deviceIdx)
	return deviceHandlers[i]->getIntArrayParameter(name);
    }
    return 0;
}

double *MDSInterface::getDoubleArrayParameter(int deviceIdx, char *name)
{
    for(int i = 0; i < deviceHandlers.size(); i++)
    {
	if(deviceHandlers[i]->getDeviceIdx() == deviceIdx)
	return deviceHandlers[i]->getDoubleArrayParameter(name);
    }
    return 0;
}

float *MDSInterface::getFloatArrayParameter(int deviceIdx, char *name)
{
    for(int i = 0; i < deviceHandlers.size(); i++)
    {
	if(deviceHandlers[i]->getDeviceIdx() == deviceIdx)
	return deviceHandlers[i]->getFloatArrayParameter(name);
    }
    return 0;
}

/** Preparation of realtime waveform sample readout. The returned integer id will be use to get interpolated waveform samples.
    -1 is returned if the waveform name is not found; */
int MDSInterface::prepareWaveParameterReadout(int deviceIdx, char *name)
{
    for(int i = 0; i < deviceHandlers.size(); i++)
    {
	if(deviceHandlers[i]->getDeviceIdx() == deviceIdx)
	return deviceHandlers[i]->prepareWaveParameterReadout(name);
    }
    return -1;
}

/** Realtine interpolated data readout */
double MDSInterface::getWaveParameter(int deviceIdx, int waveId, double time)
{
    for(int i = 0; i < deviceHandlers.size(); i++)
    {
	if(deviceHandlers[i]->getDeviceIdx() == deviceIdx)
	return deviceHandlers[i]->getWaveParameter(waveId, time);
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/** Inner class ParameterHandler implementation */
MDSInterface::DeviceHandler::DeviceHandler(int deviceIdx, char *controlName, bool verbose)
{
    this->deviceIdx = deviceIdx;
    this->verbose = verbose;
    this->controlName = new char[strlen(controlName) + 1];
    strcpy(this->controlName, controlName);
    waveDims = 0;
    wavesX = 0;
    wavesY = 0;
    segmentsFlushed = true;
    tree = 0;
}

MDSInterface::DeviceHandler::~DeviceHandler()
{
    delete [] controlName;
    for(int i = 0; i < paramValues.size(); i++)
	deleteData(paramValues[i]);
    for(int i = 0; i < paramNames.size(); i++)
	delete [] paramNames[i];
    for(int i = 0; i < waveNames.size(); i++)
	delete [] waveNames[i];

    if(waveNames.size() > 0)
    {
	delete[] waveDims;
	delete [] prevWaveIdxs;
	for(int i = 0; i < waveNames.size(); i++)
	{
	    delete [] wavesX[i];
	    delete [] wavesY[i];
	    delete [] wavesDiff[i];
	}
	delete [] wavesX;
	delete [] wavesY;
	delete [] wavesDiff;
    }
    for(int i = 0; i < signalNames.size(); i++)
	delete[] signalNames[i];
    for(int i = 0; i < signalDescriptions.size(); i++)
	delete[] signalDescriptions[i];
    for(int i = 0; i < signalBuffers.size(); i++)
	delete[] signalBuffers[i];
    for(int i = 0; i < signalAltBuffers.size(); i++)
	delete[] signalAltBuffers[i];
}

/** Read configuration from the pulse file, called in response to a MDSplus Configuration event */
bool MDSInterface::DeviceHandler::loadParameters(char *treeName, int shot, int rootParameterNid, int rootWaveNid, int rootSignalNid)
{
//    if(tree) delete tree;
    this->rootSignalNid = rootSignalNid;
    try {
//Deallocate and empty first data and name vectors
	for(int i = 0; i < paramValues.size(); i++)
	{
	    MDSplus::deleteData(paramValues.at(i));
	}
 	paramValues.clear();
	for(int i = 0; i < paramNames.size(); i++)
	{
	    delete [] paramNames[i];
	}
	paramNames.clear();
	if(waveNames.size() > 0)
	{
	    delete[] waveDims;
	    delete [] prevWaveIdxs;
	    for(int i = 0; i < waveNames.size(); i++)
	    {
		delete [] wavesX[i];
		delete [] wavesY[i];
		delete [] wavesDiff[i];
	    }
	    delete [] wavesX;
	    delete [] wavesY;
	    delete [] wavesDiff;
	    for(int i = 0; i < waveNames.size(); i++)
	    {
	    	delete []waveNames[i];
	    }
	    waveNames.clear();
	}
//Open new tree
	tree = new MDSplus::Tree(treeName, shot);
// rootParameterNid is the root nid of the parameter subtree. The following nid contains the number N of parameters
// followed by N subtree of 6 nodes (root, description, name, type, dims, data) of which only name and data are used.
	MDSplus::TreeNode *currNode = new MDSplus::TreeNode(rootParameterNid + 1, tree);
	MDSplus::Data *currData = currNode->getData();
	delete currNode;
	int numParameters = currData->getInt();
	MDSplus::deleteData(currData);
	int currNid = rootParameterNid + 2; //PAR_001
	for(int parIdx = 0; parIdx < numParameters; parIdx++)
	{
	    currNode = new TreeNode(currNid + 2, tree);
	    currData = currNode->data();
	    char *currStr = currData->getString();
	    paramNames.push_back(currStr);
	    if(verbose)
	    	std::cout << "Parameter " << parIdx << ":  " << currStr << "\t";
	    deleteData(currData);
	    delete currNode;
	    currNode = new TreeNode(currNid + 5, tree);
	    currData = currNode->data();
	    if(verbose)
	    {
	  	char *decompiled = currData->decompile();
		std::cout << decompiled << "\n";
		delete [] decompiled;
	    }
	    paramValues.push_back(currData);
	    delete currNode;
	    currNid += 6;
	}
//Read Waveform Parameters and prepare arrays for realtime waveform interpolation
//Note: while Parameters are not expected to be real in real-time, waveform samples will be read in realtime
	currNode = new MDSplus::TreeNode(rootWaveNid + 1, tree);
	currData = currNode->getData();
	int numWaves = currData->getInt();
	MDSplus::deleteData(currData);
	delete currNode;
	currNid = rootWaveNid + 2; //WAVE_001

   	waveDims = new int[numWaves];
	prevWaveIdxs = new int[numWaves];
	wavesX = new double *[numWaves];
	wavesY = new double *[numWaves];
	wavesDiff = new double *[numWaves];
 
	for(int waveIdx = 0; waveIdx < numWaves; waveIdx++)
	{
	    currNode = new TreeNode(currNid + 2 + waveIdx * 5, tree);
	    currData = currNode->data();
	    char *currStr = currData->getString();
	    waveNames.push_back(currStr);
	    if(verbose)
	    	std::cout << "Wave Parameter " << waveIdx << ":  " << currStr << " ";
	    deleteData(currData);
	    delete currNode;
	    currNode = new TreeNode(currNid + 3 + waveIdx * 5, tree);
	    currData = currNode->data();
	    int numPoints;
	    if(verbose)
		std::cout << "X: " << currData;
	    wavesX[waveIdx] = currData->getDoubleArray(&numPoints);
	    waveDims[waveIdx] = numPoints;
	    deleteData(currData);
	    delete currNode;
	    currNode = new TreeNode(currNid + 4 + waveIdx * 5, tree);
	    currData = currNode->data();
	    if(verbose)
		std::cout << " Y: " << currData << "\n";
	    wavesY[waveIdx] = currData->getDoubleArray(&numPoints);
	    if(numPoints < waveDims[waveIdx])
		waveDims[waveIdx] = numPoints;
	    deleteData(currData);
	    delete currNode;
	    currNode += 5;
//Make sure X Values are consistent
	    for(int i = 1; i < waveDims[waveIdx]; i++)
	    {
	    	if(wavesX[waveIdx][i] <= wavesX[waveIdx][i-1])
	      	    wavesX[waveIdx][i] = wavesX[waveIdx][i-1] + 1E-7;
	    }
//Create support for more efficient interpolation (1/dx)     
	    wavesDiff[waveIdx] = new double[waveDims[waveIdx]];
	    for(int i = 1; i < waveDims[waveIdx]; i++)
	    	wavesDiff[waveIdx][i] = 1./(wavesX[waveIdx][i] - wavesX[waveIdx][i-1]);
	    prevWaveIdxs[waveIdx] = -1;
	}
	return true;
    } catch (MDSplus::MdsException &exc)
    {
	std::cout << "Error loading parameters " << exc.what() << "\n";
	return false;
    }
}


/** Get the dimension of the specified parameter, up to MAX_DIMS. Return true if parameter found, false otherwise */
bool  MDSInterface::DeviceHandler::getParameterDimensions(char *name, int &nDims, int *dims)
{
    for(int parIdx = 0; parIdx < paramNames.size(); parIdx++)
    {
	if(!strcmp(name, paramNames.at(parIdx)))
	{
	    MDSplus::Data *data = paramValues.at(parIdx);
	    short length;
	    char clazz, dtype, numDims;
	    void *ptr;
	    int *currDims;
	    data->getInfo(&clazz, &dtype, &length, &numDims, &currDims, &ptr);
	    nDims = numDims;
	    for(int i = 0; i < numDims; i++)
		dims[i] = currDims[i];
	    if(numDims > 0)
	    	delete [] currDims;
	    return true;
	}
    }
    return false;
}

/** Get Scalar Parameters. return false if not found or not scalar */
bool MDSInterface::DeviceHandler::getIntParameter(char *name, int &retVal)
{
    for(int parIdx = 0; parIdx < paramNames.size(); parIdx++)
    {
	if(!strcmp(name, paramNames.at(parIdx)))
	{
	    try {
		MDSplus::Data *data = paramValues.at(parIdx);
		retVal = data->getInt();
		return true;
	    }catch(MdsException &exc) {return false;}
    	}
    }
    return false;
}
bool MDSInterface::DeviceHandler::getDoubleParameter(char *name, double &retVal)
{
    for(int parIdx = 0; parIdx < paramNames.size(); parIdx++)
    {
	if(!strcmp(name, paramNames.at(parIdx)))
	{ 
	    try {
		MDSplus::Data *data = paramValues.at(parIdx);
		retVal = data->getDouble();
		return true;
	    }catch(MdsException &exc) {return false;}
	}
    }
    return false;
}
bool MDSInterface::DeviceHandler::getFloatParameter(char *name, float  &retVal)
{
    for(int parIdx = 0; parIdx < paramNames.size(); parIdx++)
    {
	if(!strcmp(name, paramNames.at(parIdx)))
	{
	    try {
		MDSplus::Data *data = paramValues.at(parIdx);
		retVal = data->getFloat();
		return true;
	    }catch(MdsException &exc) {return false;}
	}
    }
    return false;
}

/** Get the specified parameter. Memory for data and dims is allocated by the caller. If the parameter is Scalar return nDims = 0; 
    return NULL if parameter not found */ 
int *MDSInterface::DeviceHandler::getIntArrayParameter(char *name)
{
    int nElements;
    for(int parIdx = 0; parIdx < paramNames.size(); parIdx++)
    {
	if(!strcmp(name, paramNames.at(parIdx)))
	{
	    try {
		MDSplus::Data *data = paramValues.at(parIdx);
		return data->getIntArray(&nElements);
	    }catch(MdsException &exc) {return NULL;}
	}
    }
    return NULL;
}
double *MDSInterface::DeviceHandler::getDoubleArrayParameter(char *name)
{
    int nElements;
    for(int parIdx = 0; parIdx < paramNames.size(); parIdx++)
    {
	if(!strcmp(name, paramNames.at(parIdx)))
	{
	    try {
		MDSplus::Data *data = paramValues.at(parIdx);
		return data->getDoubleArray(&nElements);
	    }catch(MdsException &exc) {return NULL;}
	}
    }
    return NULL;
}

float *MDSInterface::DeviceHandler::getFloatArrayParameter(char *name)
{
    int nElements;
    for(int parIdx = 0; parIdx < paramNames.size(); parIdx++)
    {
	if(!strcmp(name, paramNames.at(parIdx)))
	{
	    try {
		MDSplus::Data *data = paramValues.at(parIdx);
		return data->getFloatArray(&nElements);
	    }catch(MdsException &exc) {return NULL;}
	}
    }
    return NULL;
}

/** Preparation of realtime waveform sample readout. The returned integer id will be use to get interpolated waveform samples.
    -1 is returned if the waveform name is not found; */
int MDSInterface::DeviceHandler::prepareWaveParameterReadout(char *name)
{
//Return the index in internal wave arrays
    for(int waveIdx = 0; waveIdx < waveNames.size(); waveIdx++)
    {
	if(!strcmp(name, waveNames.at(waveIdx)))
	{
	    return waveIdx;
	}
    }
    return -1;
}


/** Realtine interpolated data readout */
double MDSInterface::DeviceHandler::getWaveParameter(int waveIdx, double time)
{
    int currIdx;
    double res;
    if(prevWaveIdxs[waveIdx] == -1)
    {
         if(time < wavesX[waveIdx][0])
	     return 0;
         else
         {
	     prevWaveIdxs[waveIdx] = 0;
	     if(time == wavesX[waveIdx][0])
	  	 return wavesX[waveIdx][0];
      	 }
    }
    if(time > wavesX[waveIdx][waveDims[waveIdx] - 1] )
    	return 0;

//Time may be different and even in advance
    for(currIdx = prevWaveIdxs[waveIdx]; time < wavesX[waveIdx][currIdx] && currIdx > 0; currIdx--);
    if(currIdx < prevWaveIdxs[waveIdx]) prevWaveIdxs[waveIdx] = currIdx;
 //////////////////////////////////////////////

    for(currIdx = prevWaveIdxs[waveIdx]; time > wavesX[waveIdx][currIdx]; currIdx++);
    res = wavesY[waveIdx][currIdx - 1] + (wavesY[waveIdx][currIdx] - wavesY[waveIdx][currIdx - 1]) *
     	(time - wavesX[waveIdx][currIdx - 1]) * wavesDiff[waveIdx][currIdx];
    prevWaveIdxs[waveIdx] = currIdx;
    return res;
}


//Signal store management
/** Declare a new signal to be stored in MDSplus database. Signals are stored under the SIGNALS subtree of the corresponding device tree. 
    The method will return an internal integer id which will be used in real-time to report signal samples.  
    Signals are stored as 1D floating point arrays */ 
int  MDSInterface::DeviceHandler::declareSignal(char const * name, char const * description, int segmentSize)
{
//   printf("DECLARE SIGNAL %x %d %d %s\n", tree, rootSignalNid, signalBuffers.size(), name);
   try {
    //SIGNAL_XXX subtree: NAME, DESCRIPTION, DATA
        MDSplus::TreeNode *currNode = new MDSplus::TreeNode(rootSignalNid + 3 + 4 * signalBuffers.size(), tree);
 

       MDSplus::Data *currData = new MDSplus::String(name);
	currNode->putData(currData);
	deleteData(currData);
	delete currNode;
	if(description)
	{
            currNode = new MDSplus::TreeNode(rootSignalNid + 4 + 4* signalBuffers.size(), tree);
            currData = new MDSplus::String(description);
	    currNode->putData(currData);
	    deleteData(currData);
	    delete currNode;
	}
	currNode = new MDSplus::TreeNode(rootSignalNid + 5 + 4 * signalBuffers.size(), tree);
	currNode->deleteData();
     	SignalBuffer *sigBuffer = new SignalBuffer(segmentSize, currNode);
 	currNode = new MDSplus::TreeNode(rootSignalNid + 5 + 4 * signalBuffers.size(), tree);
     	SignalBuffer *sigAltBuffer = new SignalBuffer(segmentSize, currNode);
    	bool isAlternate = false;
    	signalBuffers.push_back(sigBuffer);
    	signalAltBuffers.push_back(sigAltBuffer);
    	isAltBuffer.push_back(isAlternate);
	segmentsFlushed = false;
    	return signalBuffers.size() - 1;
    } catch(MdsException &exc)
    {
printf("DECLARE  SIGNAL %s FAILED %s\n", name, exc.what());
	return -1;
    }
}

bool MDSInterface::DeviceHandler::reportSignal(int userIdx, float val, float time, SignalBufferQueue *queue)
{
    if(segmentsFlushed)
    {
	return true;
    }
    if(userIdx < 0 || userIdx >= signalBuffers.size())
	return false;
    SignalBuffer *currBuffer, *nextBuffer;
    if(isAltBuffer[userIdx])
    {
	currBuffer = signalAltBuffers[userIdx];
	nextBuffer = signalBuffers[userIdx];
    }
    else
    {
	currBuffer = signalBuffers[userIdx];
	nextBuffer = signalAltBuffers[userIdx];
    }

    currBuffer->putSample(val, time);

    if(currBuffer->isFull())
    {
	queue->putSignalBuffer(currBuffer);
	if(!nextBuffer->isSaved())
	{
	    std::cout << "OVERFLOW!!!\n";
	    return false;
	}
	isAltBuffer[userIdx] = !isAltBuffer[userIdx];
    }
    return true;
}

void  MDSInterface::DeviceHandler::flushLastSegment(SignalBufferQueue *queue)
{
    SignalBuffer *currBuffer;
    for(int userIdx = 0; userIdx < signalBuffers.size(); userIdx++)
    {
    	if(isAltBuffer[userIdx])
	    currBuffer = signalAltBuffers[userIdx];
    	else
	    currBuffer = signalBuffers[userIdx];
    	queue->putSignalBuffer(currBuffer);
    }
//Write signal names as an array
    int numSignals = signalBuffers.size();
    char *names[numSignals];
    try {
    	for(int userIdx = 0; userIdx < numSignals; userIdx++)
    	{    
            MDSplus::TreeNode *currNode = new MDSplus::TreeNode(rootSignalNid + 3 + 4 * userIdx, tree);
	    MDSplus::Data *currData = currNode->getData();
	    names[userIdx] = currData->getString();
	    deleteData(currData);
	    delete currNode;
	}
	MDSplus::TreeNode *currNode = new MDSplus::TreeNode(rootSignalNid + 1, tree); //NAMES
	MDSplus::Data *namesData = new MDSplus::StringArray(names, numSignals);
	currNode->putData(namesData);
	deleteData(namesData);
	delete currNode;
	segmentsFlushed = true;
    }catch(MdsException &exc)
    {
	std::cout << "Cannot store signal names: " << exc.what() << "\n";
    }
}

void MDSInterface::DeviceHandler::resetSignals()
{
//Free all data structures for signals
    for(int i = 0; i < signalBuffers.size(); i++)
    {
	delete signalBuffers[i];
	delete signalAltBuffers[i];
    }
    signalBuffers.clear();
    signalAltBuffers.clear();
//Parameter data structues are freed by loadParameters itself
}


/** SignalBuffer methods */
MDSInterface::SignalBuffer::SignalBuffer(int dimension, MDSplus::TreeNode *targetNode)
{
    this->dimension = dimension;
    this->targetNode = targetNode;
    samples = new float[dimension];
    times = new float[dimension];
    saved = true;
    currDimension = 0;
}
MDSInterface::SignalBuffer::~SignalBuffer()
{
    if(dimension > 0)
    {
	delete [] samples;
	delete [] times;
    }
    delete targetNode;
}
void MDSInterface::SignalBuffer::putSample(float sample, float time)
{
    if(currDimension >= dimension)
	return; //Shold never happen
    samples[currDimension] = sample;
    times[currDimension] = time;
    currDimension++;
    saved = false;
}
bool MDSInterface::SignalBuffer::isFull() {return currDimension >= dimension;}
bool MDSInterface::SignalBuffer::isSaved() {return saved; }
void MDSInterface::SignalBuffer::setSaved() {saved = true; }
void MDSInterface::SignalBuffer::save()
{
//    printf("SAVE SEGMENT\n");
    if(currDimension == 0) return;
    MDSplus::Data *startData = new Float32(times[0]);
    MDSplus::Data *endData = new Float32(times[currDimension - 1]);
    MDSplus::Data *timesData = new Float32Array(times, currDimension);
    MDSplus::Data *samplesData = new Float32Array(samples, currDimension);
    try {
//      targetNode->beginSegment(startData, endData, timesData, (Array *)samplesData);
//      targetNode->putSegment((Array *)samplesData, -1);
    	targetNode->makeSegment(startData, endData, timesData, (Array *)samplesData);
     }catch(MdsException &exc)
    {
    	std::cout << "MAKE MDSplus segment Failed: %s" << exc.what() << "\n";
    }
    deleteData(startData);
    deleteData(endData);
    deleteData(timesData);
    deleteData(samplesData);
    currDimension = 0;
    saved = true;
}

/** SignalBufferQueue  class methods */
MDSInterface::SignalBufferQueue::SignalBufferQueue(int capacity)
{
    this->capacity = capacity;
    buffers = new SignalBuffer *[capacity];
    headIdx = tailIdx = 0;
    sharedSem.Create();
    sharedEvent.Create();
}
MDSInterface::SignalBufferQueue::~SignalBufferQueue()
{
    if(capacity > 0)
	delete [] buffers;
    capacity = 0;
}
/** Enqueue (in realtime) a filled buffer for saving. If room available return 0, -1 otherwise */
int MDSInterface::SignalBufferQueue::putSignalBuffer(SignalBuffer *sigBuf)
{
//headIdx: index of the next available buffer
//tailIdx: index of the last retrieved buffer 
   sharedSem.Lock();
    if((headIdx + 1) % capacity == tailIdx) //Overflow (one guard element)
    {
	sharedSem.UnLock();
	return -1;
    }
    buffers[headIdx] = sigBuf;

    if(headIdx == tailIdx) //if currently empty
	sharedEvent.Post();
    headIdx = (headIdx + 1) % capacity;
    sharedSem.UnLock();
    return 0;
}
/** Get (and Wait) the next enqueued buffer */
MDSInterface::SignalBuffer *MDSInterface::SignalBufferQueue::getSignalBuffer()
{
    sharedSem.Lock();
    while(headIdx == tailIdx)
    {
	sharedSem.UnLock();
	sharedEvent.Wait();
	sharedEvent.Reset();
	sharedSem.Lock();
    }
    SignalBuffer *retBuffer = buffers[tailIdx];
    tailIdx = (tailIdx + 1) % capacity;
    sharedSem.UnLock();
    return retBuffer;
}



static void handleSignalWrite(MDSInterface::TreeWriter *tw)
{
    tw->run();
}

/** TreeWriter class methods */
MDSInterface::TreeWriter::TreeWriter(int runOnCpu, SignalBufferQueue *queue)
{
    this->runOnCpu = runOnCpu;
    this->queue = queue;
}

/** Start execution on a separate thread */
void MDSInterface::TreeWriter::start()
{
    ProcessorType writerProcessorType(runOnCpu);
    writerProcessorType.SetMask(runOnCpu);
    int writerTid = ThreadsBeginThread((ThreadFunctionType)handleSignalWrite, this,10*THREADS_DEFAULT_STACKSIZE, NULL, XH_NotHandled, 
	writerProcessorType);
}
/** Method for reading buffers and storing on tree */
void MDSInterface::TreeWriter::run()
{
    while(true)
    {
	MDSInterface::SignalBuffer *buffer = queue->getSignalBuffer();
	try {
	    buffer->save();
	}catch(MdsException &exc)
	{
	    std::cout << "MDSInterface::segment write failed: " << exc.what() << "\n";
	    buffer->setSaved();
	}
    }
}

