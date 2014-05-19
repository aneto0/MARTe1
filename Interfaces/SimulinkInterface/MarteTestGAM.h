//******************************************************************************
//
//      Template file for the automatic generation of MARTe GAMs 
//      From code produced by Simulink.
//	
//	Author: G. Manduchi
//
//******************************************************************************

#ifndef MarteTestGAM_H_
#define MarteTestGAM_H_

#include "GAM.h"
#include "HttpInterface.h"

#include "MarteTest0.h"
#define NUM_INPUTS  2 
#define NUM_OUTPUTS  2 

OBJECT_DLL(MarteTestGAM)
class MarteTestGAM : public GAM, public HttpInterface {
OBJECT_DLL_STUFF(MarteTestGAM)

// DDB Interfaces
private:
    /** Input interface to read data from */
    DDBInputInterface                      *input;
    /** Output interface to write data to */
    DDBOutputInterface                     *output;

    bool initialized;

   	real_T ki;
	real_T kp;


public:
    /** Constructor */
    MarteTestGAM() 
    {
      input = 0;
      output   = 0;
      initialized = false;
    }


    /** Destructor */
    ~MarteTestGAM()
    {
    }


    /**
    * Loads GAM parameters from a CDB
    * @param cdbData the CDB
    * @return True if the initialisation went ok, False otherwise
    */
    virtual bool Initialise(ConfigurationDataBase& cdbData);

    /**
    * GAM main body
    * @param functionNumber The current state of MARTe
    * @return False on error, True otherwise
    */
    virtual bool Execute(GAM_FunctionNumbers functionNumber);

    /**
    * Saves parameters to a CDB
    * @param info the CDB to save to
    * @return True
    */
    virtual bool ObjectSaveSetup(ConfigurationDataBase &info, StreamInterface *err){ return True; };


    /**
    * The message is actually a multi stream (HttpStream)
    * with a convention described in HttpStream
    */
    virtual bool ProcessHttpMessage(HttpStream &hStream);
};


#endif /* MarteTestGAM_H_ */
