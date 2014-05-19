//******************************************************************************
//
//      Template Code for automatic GAM generation from Simulink
//      
//	Author: G. Manduchi
//
//******************************************************************************

#include "MarteTestGAM.h"
#include "CDBExtended.h"
#include "HRT.h"
#include "DDBInputInterface.h"
#include "DDBOutputInterface.h"

#include "LoadCDBObjectClass.h"

extern "C" void MdlOutputs(int_T tid);
extern "C" void MdlUpdate(int_T tid);
extern "C" void MdlInitializeSizes(void);
extern "C" void MdlInitializeSampleTimes(void);
extern "C" void MdlInitialize(void);
extern "C" void MdlStart(void);
extern "C" void MdlTerminate(void);
extern "C" RT_MODEL_MarteTest0 *MarteTest0(void);

bool MarteTestGAM::Initialise(ConfigurationDataBase& cdbData) {

    CDBExtended cdb(cdbData);
    if(!AddInputInterface(input,"InputInterface")){
        AssertErrorCondition(InitialisationError,"MarteTestGAM::Initialise: %s failed to add input interface",Name());
        return False;
    }
    if(!AddOutputInterface(output,"OutputInterface")){
        AssertErrorCondition(InitialisationError,"MarteTestGAM::Initialise: %s failed to add output interface",Name());
        return False;
    }
    if(!cdb->Move("InputSignals")){
        AssertErrorCondition(InitialisationError,"MarteTestGAM::Initialise: %s did not specify InputSignals entry",Name());
        return False;
    }

    if(!input->ObjectLoadSetup(cdb,NULL)){
        AssertErrorCondition(InitialisationError,"MarteTestGAM::Initialise: %s ObjectLoadSetup Failed DDBInterface %s ",Name(),input->InterfaceName());
        return False;
    }

    cdb->MoveToFather();

    if(!cdb->Move("OutputSignals")){
        AssertErrorCondition(InitialisationError,"MarteTestGAM::Initialise: %s did not specify OutputSignals entry",Name());
        return False;
    }

    if(!output->ObjectLoadSetup(cdb,NULL)){
        AssertErrorCondition(InitialisationError,"MarteTestGAM::Initialise: %s ObjectLoadSetup Failed DDBInterface %s ",Name(),output->InterfaceName());
        return False;
    }

    cdb->MoveToFather();

    	float currFloat;
	int currInt;
	if(!cdb.ReadFloat(currFloat,"ki")){AssertErrorCondition(InitialisationError,"MarteTestGAM::Initialise: %s ObjectLoadSetup Missing Definition",Name()); return false;}
	ki = currFloat;
	if(!cdb.ReadFloat(currFloat,"kp")){AssertErrorCondition(InitialisationError,"MarteTestGAM::Initialise: %s ObjectLoadSetup Missing Definition",Name()); return false;}
	kp = currFloat;
	

    return True;
}


bool MarteTestGAM::Execute(GAM_FunctionNumbers functionNumber) {

    switch(functionNumber) {
      case GAMPrepulse: 
	rtP.ki = ki;
	rtP.kp = kp;
	

	if(!initialized)
	{
	    initialized = true;
	    MarteTest0();
	}
       	MdlInitializeSizes();
        MdlInitializeSampleTimes(); // This actually does nothing but in principle should be called.
        MdlInitialize();
        MdlStart(); // This also calls MdlInitialize, so the previous call is superfluos
	break;
      
      case GAMOnline: 
      {
	float  *inputData  = (float  *)input->Buffer();
	float *outputData = (float *)output->Buffer();
	input->Read();
	for(int i = 0; i < NUM_INPUTS; i++)
	    rtU.In[i] = inputData[i];

        MdlOutputs(0);
        MdlUpdate(0);

	for(int i = 0; i < NUM_OUTPUTS; i++)
	    outputData[i] = rtY.Out[i];

	output->Write();
	break;
      }
      case GAMPostpulse:
	MdlTerminate();
	break;
    }
    return True;
}

bool MarteTestGAM::ProcessHttpMessage(HttpStream &hStream) {
	hStream.SSPrintf("OutputHttpOtions.Content-Type","text/html");
	hStream.keepAlive = False;
	hStream.WriteReplyHeader(False);
	hStream.Printf("<html><head><title>MarteTestGAM %s</title></head><body>\n", Name());
	hStream.Printf("<p>Num. Input Signals: %d</p>\n", 2);
	hStream.Printf("<p>Num. Output Signals: %d</p>\n", 2);

	hStream.Printf("<br><br><table><tr><td>Parameter Name</td><td>Parameter Value</td></tr>\n");

	hStream.Printf("<tr><td>ki</td><td>%f</td></tr>",ki);
	hStream.Printf("<tr><td>kp</td><td>%f</td></tr>",kp);
	

	hStream.Printf("</table></body></html>");
	hStream.WriteReplyHeader(True);
	return True;
}


OBJECTLOADREGISTER(MarteTestGAM,"$Id: MarteTestGAM.cpp, generated from Simulink $")
