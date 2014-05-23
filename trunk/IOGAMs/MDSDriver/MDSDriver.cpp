/*
 * Gabriele Manduchi
 * 2014
 */


// private includes
#include "MDSDriver.h"

// framework include to use GODBFindByName
#include "GlobalObjectDataBase.h"

// framework includes
#include "CDBExtended.h"
#include "MDSInterface.h"
#include <time.h>


static char *css = "table.bltable {"
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


    /** ObjectLoadSetup 
     * Configuration parameters are:
     * Name - name of the pulse file
     * Shot - shot number of the pulse file
     * NumberOfInputs - number of nodes containing input data
     * FirstNodePath - path name of the first node of the list
     */
     bool MDSDriver::ObjectLoadSetup(ConfigurationDataBase &info,StreamInterface *err)
     {
    	CDBExtended cdb(info);
	//Free previously allocated stuff (if any)
	//freeData();
    // reads NumberOfInputs and NumberOfOutputs
    // (both must be set, take a look at GenericAcqModule)
        if ( !GenericAcqModule::ObjectLoadSetup(info, NULL) ) 
	{
            AssertErrorCondition(FatalError,
        		"MDSDriver::ObjectLoadSetup: %s GenericAcqModule wrong or missing info", Name() );
            return False;
    	}

	//Output signals: time(32 bits), trigger(32 bits), signals (integer, 32 bits) 
	numRefWaveforms = NumberOfInputs();
	if(numRefWaveforms > 0)
	{
	    BString *refNames = new BString[numRefWaveforms];
	    int retNumRefs = numRefWaveforms;
	    if(!cdb.ReadBStringArray(refNames, &retNumRefs, 1, "ReferenceWaveNames"))
	    {
   	    	AssertErrorCondition(FatalError,
    			"MDSDriver::ObjectLoadSetup: %s ReferenceWaveNames not set", Name() );
    	    	return False;
	    }
	    if(retNumRefs != numRefWaveforms)
	    {
    	    	AssertErrorCondition(FatalError,
    			"MDSDriver::ObjectLoadSetup: %s the number %d of ReferenceWaveNames is different from the number %d of input signals", Name(), retNumRefs, numRefWaveforms );
    	    	return False;
	    }
	    refWaveformNames = new char *[numRefWaveforms];
	    for(int i = 0; i < numRefWaveforms; i++)
	    {
		char *currName = (char *)refNames[i].Buffer();
		refWaveformNames[i] = new char[strlen(currName) + 1];
		strcpy(refWaveformNames[i], currName);
		printf("%s\n", currName);
	    }
	    refWaveformIdxs = new int[numRefWaveforms];
	    currInputs = new float[numRefWaveforms];
	}
	numOutSignals = NumberOfOutputs();
	if(numOutSignals > 0)
	{
	    BString outNames[numOutSignals];
	    int retNumOuts =  numOutSignals;
	    if(!cdb.ReadBStringArray(outNames, &retNumOuts, 1, "OutSignalNames"))
	    {
    	    	AssertErrorCondition(FatalError,
    			"MDSDriver::ObjectLoadSetup: %s OutSignalNames not set", Name() );
    	    	return False;
	    }
	    if(retNumOuts != numOutSignals)
	    {
    	    	AssertErrorCondition(FatalError,
    			"MDSDriver::ObjectLoadSetup: %s the number %d of OutSignalNames is different from the number %d of output signals", Name(), retNumOuts, numOutSignals );
    	    	return False;
	    }
	    outSignalNames = new char *[numOutSignals];
	    for(int i = 0; i < numOutSignals; i++)
	    {
		char *currName = (char *)outNames[i].Buffer();
		outSignalNames[i] = new char[strlen(currName) + 1];
		strcpy(outSignalNames[i], currName);
		printf("%s\n", currName);
	    }
	    outSignalIdxs = new int[numOutSignals];
	    currOutputs = new float[numOutSignals];
	//Read Optional Descriptions
	    outSignalDescriptions = new char *[numOutSignals];
	    BString outSignalDescs[numOutSignals];
	    if(cdb.ReadBStringArray(outSignalDescs, &retNumOuts, numOutSignals, "OutSignalDescriptions"))
	    {
	    	if(retNumOuts != numOutSignals)
	    	{
    	    	    AssertErrorCondition(FatalError,
    			"MDSDriver::ObjectLoadSetup: %s the number %d of OutSignalDescriptions is different from the number %d of input signals", Name(), retNumOuts, numOutSignals );
    	    	    return False;
	    	}
		for(int i = 0; i < numOutSignals; i++)
		{
		    char *currName = (char *)outSignalDescs[i].Buffer();
		    outSignalDescriptions[i] = new char[strlen(currName) + 1];
		    strcpy(outSignalDescriptions[i], currName);
		    printf("%s\n", currName);
		}
	    }
	    else
	    {
		for(int i = 0; i < numOutSignals; i++)
		    outSignalDescriptions[i] = 0;
	    }
	}
        if( !cdb.ReadInt32(deviceIdx, "MdsId") ) 
	{
    	    AssertErrorCondition(FatalError,
    			"MDSDriver::ObjectLoadSetup: %s MdsId is not declared", Name() );
    	    return False;
    	}
	return true;
    }

    bool MDSDriver::PulseStart()
    {
	for(int i = 0; i < numRefWaveforms; i++)
	{
	    if((refWaveformIdxs[i] = MDSInterface::prepareWaveParameterReadout(deviceIdx, refWaveformNames[i])) == -1)
 	    {
    	    AssertErrorCondition(FatalError,
    			"MDSDriver::PulseStart: %s Reference Waveform %s is not declared in the target MDS device", Name(),  refWaveformNames[i]);
    	    return False;
    	}
	}
	MDSInterface::resetSignals(deviceIdx);
 	for(int i = 0; i < numOutSignals; i++)
	{
         if((outSignalIdxs[i] = MDSInterface::declareSignal(deviceIdx, outSignalNames[i], outSignalDescriptions[i])) == -1)
 	    {
    	    	AssertErrorCondition(FatalError,
    			"MDSDriver::PulseStart: %s Error declaring out signal %s for device idx %d", Name(),  outSignalNames[i], deviceIdx);
    	    	return False;
    	}
	}
    prevTime = -1;
	return true;
    }
  
    bool MDSDriver::WriteData(uint32 usecTime, const int32 *buffer)
    {
        if(usecTime == prevTime) return True;
        prevTime = usecTime;
	    float time = usecTime * 1E-6;
//	printf("WRITE SIGNALS: %d %d", usecTime, numOutSignals);
	    for(int i = 0; i < numOutSignals; i++)
	    {
	        currOutputs[i] = ((float *)buffer)[i];
	        MDSInterface::reportSignal(deviceIdx, outSignalIdxs[i], ((float *)buffer)[i], time);

//	    printf("%f ", currOutputs[i]);
	    }
//	printf("\n");
	    return true;
    }

    int32 MDSDriver::GetData(uint32 usecTime, int32 *buffer, int32 bufferNumber)
    {
	double time = usecTime * 1E-6;
//	printf("GET WAVES: \n");
	for(int i = 0; i < numRefWaveforms; i++)
	{
	    currInputs[i] = ((float *)buffer)[i] = (float)MDSInterface::getWaveParameter(deviceIdx, refWaveformIdxs[i], time);

//	    printf("%f ", currInputs[i]);
	}
//	printf("\n");
	return numRefWaveforms * sizeof(float);
    }
    int64 MDSDriver::GetUsecTime(){return HRTRead64();}

    bool MDSDriver::ProcessHttpMessage(HttpStream &hStream) 
    {
    	hStream.SSPrintf("OutputHttpOtions.Content-Type","text/html");
    	hStream.keepAlive = False;

    	hStream.Printf("<html><head><title>%s</title>", Name());
    	hStream.Printf( "<style type=\"text/css\">\n" );
    	hStream.Printf("%s\n", css);
    	hStream.Printf( "</style></head><body>\n" );
    	hStream.Printf("Device Id: %d<br>", deviceIdx);
    	hStream.Printf("<table class=\"bltable\">\n");
    	hStream.Printf("<tr><td> Input Name </td> <td>Current Input Value</td></tr>\n");
	for(int i = 0; i < numRefWaveforms; i++)
	    hStream.Printf("<tr><td>%s</td><td>%f</td></tr>\n", refWaveformNames[i], currInputs[i]);
	hStream.Printf("</table><br><br>\n");

   	hStream.Printf("<table class=\"bltable\">\n");
    	hStream.Printf("<tr><td> Output Name </td> <td>Current Output Value</td></tr>\n");
	for(int i = 0; i < numOutSignals; i++)
	    hStream.Printf("<tr><td>%s</td><td>%f</td></tr>\n", outSignalNames[i], currOutputs[i]);
	hStream.Printf("</table><br><br>\n");

    	hStream.Printf("</body></html>");
    	hStream.WriteReplyHeader(True);
    	return True;
    }


OBJECTLOADREGISTER(MDSDriver, "$Id: MDSDriver.cpp,v 1.0 2014/04/16 12:12:12 gab Exp $")
