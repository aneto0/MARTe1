//******************************************************************************
//
//      Template Code for automatic GAM generation from Simulink
//      
//	Author: G. Manduchi
//
//******************************************************************************

#include "<<Name>>GAM.h"
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
extern "C" RT_MODEL_<<Name>>0 *<<Name>>0(void);

bool <<Name>>GAM::Initialise(ConfigurationDataBase& cdbData) {

    CDBExtended cdb(cdbData);
    if(!AddInputInterface(input,"InputInterface")){
        AssertErrorCondition(InitialisationError,"<<Name>>GAM::Initialise: %s failed to add input interface",Name());
        return False;
    }
    if(!AddOutputInterface(output,"OutputInterface")){
        AssertErrorCondition(InitialisationError,"<<Name>>GAM::Initialise: %s failed to add output interface",Name());
        return False;
    }
    if(!cdb->Move("InputSignals")){
        AssertErrorCondition(InitialisationError,"<<Name>>GAM::Initialise: %s did not specify InputSignals entry",Name());
        return False;
    }

    if(!input->ObjectLoadSetup(cdb,NULL)){
        AssertErrorCondition(InitialisationError,"<<Name>>GAM::Initialise: %s ObjectLoadSetup Failed DDBInterface %s ",Name(),input->InterfaceName());
        return False;
    }

    cdb->MoveToFather();

    if(!cdb->Move("OutputSignals")){
        AssertErrorCondition(InitialisationError,"<<Name>>GAM::Initialise: %s did not specify OutputSignals entry",Name());
        return False;
    }

    if(!output->ObjectLoadSetup(cdb,NULL)){
        AssertErrorCondition(InitialisationError,"<<Name>>GAM::Initialise: %s ObjectLoadSetup Failed DDBInterface %s ",Name(),output->InterfaceName());
        return False;
    }

    cdb->MoveToFather();

    <<LoadParameters>>

    return True;
}


bool <<Name>>GAM::Execute(GAM_FunctionNumbers functionNumber) {

    switch(functionNumber) {
      case GAMPrepulse: 
	<<CopyParameters>>

	if(!initialized)
	{
	    initialized = true;
	    <<Name>>0();
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

bool <<Name>>GAM::ProcessHttpMessage(HttpStream &hStream) {
	hStream.SSPrintf("OutputHttpOtions.Content-Type","text/html");
	hStream.keepAlive = False;
	hStream.WriteReplyHeader(False);
	hStream.Printf("<html><head><title><<Name>>GAM %s</title></head><body>\n", Name());
	hStream.Printf("<p>Num. Input Signals: %d</p>\n", <<NumInputs>>);
	hStream.Printf("<p>Num. Output Signals: %d</p>\n", <<NumOutputs>>);

	hStream.Printf("<br><br><table><tr><td>Parameter Name</td><td>Parameter Value</td></tr>\n");

	<<ShowParameters>>

	hStream.Printf("</table></body></html>");
	hStream.WriteReplyHeader(True);
	return True;
}


OBJECTLOADREGISTER(<<Name>>GAM,"$Id: <<Name>>GAM.cpp, generated from Simulink $")
