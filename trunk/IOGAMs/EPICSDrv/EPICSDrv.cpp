//******************************************************************************
//    $Log: NI6259Drv.cpp,v $
//
//******************************************************************************

/*
 * EPICS  IOGAM for reading and writing EPICS Process Variables
 * author: Gabriele Manduchi
 * date: July 2013
 */



#include "HRT.h"

#include "Endianity.h"
#include "CDBExtended.h"

#include "EPICSDrv.h"

static void *handler(void *arg)
{
    EPICSDrv *drv = (EPICSDrv *)arg;
    drv->handleWrite();
}

void dataCallback(struct event_handler_args eha)
{
    chid	chid = eha.chid;
    if(eha.status != ECA_NORMAL) return;
    EPICSDrv *obj = (EPICSDrv *)eha.usr;
    obj->inputSem.Lock();

//printf("DATA CALLBACK: %f\n", *(float *)eha.dbr);
    
    for(int i = 0; i < obj->numInputs; i++)
    {
        if(obj->inChids[i] == eha.chid)
        {
            switch(eha.type) {
                case DBR_CHAR:
                    obj->incoming[i] = *(char *)eha.dbr;
                    break;
                case DBR_SHORT:
                    obj->incoming[i] = *(short *)eha.dbr;
                    break;
                case DBR_LONG:
                    obj->incoming[i] = *(int *)eha.dbr;
                    break;
                case DBR_FLOAT:
                    obj->incoming[i] = *(float *)eha.dbr;
                    break;
                case DBR_DOUBLE:
                    obj->incoming[i] = *(double *)eha.dbr;
                    break;
                default:
                    obj->incoming[i] = 0;
            }
            break;
        }
    }
    obj->inputSem.UnLock();
}   
    
void *registerCallbacks(void *arg)
{
    EPICSDrv * drv = (EPICSDrv *)arg;
//Attach EPICS Context
    if(ca_attach_context(drv->epicsContext) != ECA_NORMAL)
    {
        drv->AssertErrorCondition(FatalError, "EPICSDrv::ObjectLoadSetup: %s Cannot attach EPICS context for input", drv->Name());
        return NULL;
    }
//Register input variables
    for(int i = 0; i < drv->numInputs; i++)
    {
 
        evid evid;
        if(ca_add_event(DBR_FLOAT,drv->inChids[i],dataCallback, drv,&evid) != ECA_NORMAL)
        {
    	    drv->AssertErrorCondition(FatalError, "EPICSDrv::ObjectLoadSetup: %s Cannot connect callback to EPICS variable %s", drv->Name(), drv->inPvNames[i]);
        }
    }
    if(ca_pend_event(0.0) != ECA_NORMAL)
    {
    	    drv->AssertErrorCondition(FatalError, "EPICSDrv::ObjectLoadSetup: %s Cannot listen to EPICS Monitor", drv->Name());
    }
    printf("NON DOVEVO ARRIVARE QUI!!!!\n");
}

EPICSDrv::EPICSDrv() {
	numInputs = 0;
	numOutputs = 0;
	lastCycleUsecTime = -1;
    lastScanCount = 0;
    inputSem.Create();
    outputSem.Create();
    writeEventSem.Create();
    
// copied from ATCAadcDrv.cpp
    	css = "table.bltable {"
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
}

/*
 * Release the resources
 */
EPICSDrv::~EPICSDrv() 
{
}


void EPICSDrv::awakeEPICSWriter()
{
    writeEventSem.Post();
}


void EPICSDrv::handleWrite()
{
    float outBuf[MAX_PV];
    if(ca_attach_context(epicsContext) != ECA_NORMAL)
    {   
        AssertErrorCondition(FatalError, "EPICSDrv::handleWrite: %s Cannot attach EPICS context for output", Name());
        return;
    }
    while(true)
    {
//Attach EPICS Context

         writeEventSem.Wait();
         writeEventSem.Reset();
         outputSem.Lock();

//printf("EPICS OUT HANDLER:  %f %d\n", outcoming[0], numOutputs);
        for(int i = 0; i < numOutputs; i++)
        {
            outBuf[i] = outcoming[i];
        }
        outputSem.UnLock();
        for(int i = 0; i < numOutputs; i++)
        {
            int currInt;
            if(outTypes[i] == IS_FLOAT)
            {
                if(ca_put (DBR_FLOAT, outChids[i], &outBuf[i]) != ECA_NORMAL)
                {
                    AssertErrorCondition(FatalError, "EPICSDrv::handleWrite: %s Cannot put EPICS variable", Name(), outPvNames[i]);
                    return;
                }
            }
            else //IS_INT
            {
                currInt = outcoming[i]; 
                if(ca_put (DBR_INT, outChids[i], &currInt) != ECA_NORMAL)
                {
                    AssertErrorCondition(FatalError, "EPICSDrv::handleWrite: %s Cannot put EPICS variable", Name(), outPvNames[i]);
                    return;
                }
            }
        }
        if(ca_pend_io(5.0) != ECA_NORMAL)
        {
            AssertErrorCondition(FatalError, "EPICSDrv::handleWrite: %s Cannot put EPICS variables", Name());
            return;
        }
    }
}




/*
 * Enable System Acquisition
 */
bool EPICSDrv::EnableAcquisition() {
    return True;
}

bool EPICSDrv::DisableAcquisition() {
    return True;
}

/*
 * Board Parameters are:
 * - input channels (int)
 * - output channels (int)
 * - board PCI address (char*)
 * - board IRQ num (int)
 */
bool EPICSDrv::ObjectLoadSetup(ConfigurationDataBase &info,StreamInterface *err) 
{

    printf("HRT CLOCK CYCLE: %e\n", HRTClockCycle());


    CDBExtended cdb(info);
	// load configuration setup
    if (!GenericAcqModule::ObjectLoadSetup(info,err)) {
        AssertErrorCondition(FatalError, "EPICSDrv::ObjectLoadSetup: %s GenericAcqModule::ObjectLoadSetup Failed", Name());
        return False;
    }
    
    int numberOfInputs;
    numInputs = 0 ;
    if (!cdb.ReadInt32((int32&) numberOfInputs, "NumberOfInputs", 0))
    {
	    AssertErrorCondition(Warning, "EPICSDrv::ObjectLoadSetup: %s did not specify NumberOfInputs.", Name());
        return False;
    }
    if (!cdb.ReadInt32((int32&) runOnCpu, "RunOnCpu", 1))
    {
	    AssertErrorCondition(Warning, "EPICSDrv::ObjectLoadSetup: %s did not specify RunOnCpu.", Name());
        return False;
    }
    if(numberOfInputs > 0)
        numInputs = numberOfInputs - 1;
    
    BString inVarsStr;
    if (!cdb.ReadBString(inVarsStr, "InputVariables")) 
    {
        if(numInputs > 0)
    	AssertErrorCondition(FatalError, "EPICSDrv::ObjectLoadSetup: %s did not specify expected Input variable names", Name());
	    return False;
    }
    const char *inVars = inVarsStr.Buffer();
    int currNumInVars = 0;
    
    char *token = strtok((char *)inVars, " ");
    while( token != NULL && currNumInVars != MAX_PV ) 
    {
        inPvNames[currNumInVars] = new char[strlen(token) + 1];
        strcpy(inPvNames[currNumInVars], token);
        token = strtok(NULL, " ");
        currNumInVars++;


    }

// Input Configuration
    if(currNumInVars != numInputs)
    {
    	AssertErrorCondition(FatalError, "EPICSDrv::ObjectLoadSetup: %s number on input PVs %d is different from number of declared inputs %d", Name(), currNumInVars, numInputs);
	    return False;
    }
    if (!cdb.ReadInt32((int32&) numOutputs, "NumberOfOutputs", 0))
    {
	    AssertErrorCondition(Warning, "EPICSDrv::ObjectLoadSetup: %s did not specify NumOutputs. Assuming 0", Name());
    }
//Output configuration

    BString outVarsStr;
    if (!cdb.ReadBString(outVarsStr, "OutputVariables")) 
    {
        if(numOutputs > 0)
    	AssertErrorCondition(FatalError, "EPICSDrv::ObjectLoadSetup: %s did not specify expected Output variable names", Name());
	    return False;
    }
    const char *outVars = outVarsStr.Buffer();
    int currNumOutVars = 0;
    
    token = strtok((char *)outVars, " ");
    while( token != NULL && currNumOutVars != MAX_PV ) 
    {
        outPvNames[currNumOutVars] = new char[strlen(token) + 1];
        strcpy(outPvNames[currNumOutVars], token);
        token = strtok(NULL, " ");
        currNumOutVars++;
    }
    if(currNumOutVars != numOutputs)
    {
    	AssertErrorCondition(FatalError, "EPICSDrv::ObjectLoadSetup: %s number on output PVs %d is different from number of declared outputs %d", Name(), currNumOutVars, numOutputs);
	    return False;
    }
    int size = numOutputs;
    if(!cdb.ReadInt32Array(outTypes, &size, 1, "OutputTypes"))
    {
     	AssertErrorCondition(Warning, "EPICSDrv::ObjectLoadSetup: %s did not specify output types. Float assumed.", Name());
        for(int i = 0; i < numOutputs; i++)
            outTypes[i] = IS_FLOAT;
    }
    else
    {
        if(size != numOutputs)
        {
    	    AssertErrorCondition(FatalError, "EPICSDrv::ObjectLoadSetup: %s number on output types %d is different from number of declared outputs %d", Name(), size, numOutputs);
	        return False;
        }
    }

    if (!cdb.ReadInt32((int32&) outScanPeriod, "OutScanPeriod", 0))
    {
	    AssertErrorCondition(Warning, "EPICSDrv::ObjectLoadSetup: %s did not specify OutScanPeriod. Assuming 0", Name());
    }

// Make EPICS context
    int status = ca_context_create(ca_enable_preemptive_callback);
    epicsContext = ca_current_context();
    if(status != ECA_NORMAL)
    {
    	AssertErrorCondition(FatalError, "EPICSDrv::ObjectLoadSetup: %s Cannot create EPICS context", Name());
	    return False;
    }
    printf("EPICS CONTEXT Created\n");

// Create chid for output variables
    for(int currChan = 0; currChan < numOutputs; currChan++)
    { 
        status = ca_create_channel(outPvNames[currChan],NULL,NULL,10,&outChids[currChan]);  
        if(status != ECA_NORMAL)
        {
    	    AssertErrorCondition(FatalError, "EPICSDrv::ObjectLoadSetup: %s Cannot connect to EPICS variable %s", Name(), outPvNames[currChan]);
	        return False;
        }
printf("CREATO OUT CHID %d %d: %s\n", currChan, outChids[currChan], outPvNames[currChan]);
    }
//Create chid for input variables
    for(int currChan = 0; currChan < numInputs; currChan++)
    { 
        status = ca_create_channel(inPvNames[currChan],NULL,NULL,10,&inChids[currChan]);  
        if(status != ECA_NORMAL)
        {
    	    AssertErrorCondition(FatalError, "EPICSDrv::ObjectLoadSetup: %s Cannot connect to EPICS variable %s", Name(), outPvNames[currChan]);
	        return False;
        }

printf("CREATO IN CHID %d %d: %s\n", currChan, inChids[currChan], inPvNames[currChan]);

    }

     AssertErrorCondition(Information,"NI6259Drv::ObjectLoadSetup %s Correctly Initialized ", Name());

//Start writeHandler in case
    if(numOutputs > 0)
    {
        ProcessorType writerProcessorType(runOnCpu);
        writerProcessorType.SetMask(runOnCpu);
        int writerTid = ThreadsBeginThread((ThreadFunctionType)handler, this,10*THREADS_DEFAULT_STACKSIZE, 
            NULL, XH_NotHandled, writerProcessorType);
    }
    if(numInputs > 0)
    {
        ProcessorType writerProcessorType(runOnCpu);
        writerProcessorType.SetMask(runOnCpu);
        int writerTid = ThreadsBeginThread((ThreadFunctionType)registerCallbacks, this,
            10*THREADS_DEFAULT_STACKSIZE, NULL, XH_NotHandled, writerProcessorType);
    }


    return True;
}

/*
 * object description
 */
bool EPICSDrv::ObjectDescription(StreamInterface &s,bool full,StreamInterface *err){
    s.Printf("%s %s\n", ClassName(), Version());
    s.Printf("Module Name --> %s\n", Name());
	return True;
}

/*
 * Write Data TO Board
 * 
 * NOTE buffer data MUST be in u16 format array
 * NOTE passing f32 you can specify a voltage
 */
bool EPICSDrv::WriteData(uint32 usecTime, const int32 *buffer) {
	if (!buffer) {
	    AssertErrorCondition(FatalError,"EPICSDrv::WriteData: %s. buffer is null.", Name());
	    return False;		
	}
    if(numOutputs == 0)
	{
	    AssertErrorCondition(FatalError,"EPICSDrv::WriteData: %s. Module is either not configured for output or not initialized.", Name());
	    return False;		
	}
    //Copy current output in outcoming
    outputSem.Lock();
    for(int i = 0; i < numOutputs; i++)
    {
        if(outTypes[i] == IS_FLOAT)
        {
            outcoming[i] = *((float *)&buffer[i]);
        }
        else //IS_INT
            outcoming[i] = *((int *)&buffer[i]);
    }
    outputSem.UnLock();

    //If it is time, awake writer thread
    uint currTimeCount = (uint32)(HRT::HRTCounter32());
    if((currTimeCount > lastScanCount && (currTimeCount - lastScanCount) * HRTClockCycle()/2. * 1000. > outScanPeriod) ||
        (currTimeCount < lastScanCount && (currTimeCount + pow(2,32) - lastScanCount)  * HRTClockCycle()/2. * 1000. > outScanPeriod))
    {
        awakeEPICSWriter();
        lastScanCount = currTimeCount;
    }
	return True;
}

/*
 * Get Data FROM Board
 */
int32 EPICSDrv::GetData(uint32 usecTime, int32 *buffer, int32 bufferNumber) 
{
    if(!buffer) 
    {
	    AssertErrorCondition(FatalError,"EPICSDrv::GetData: %s. buffer is null.", Name());
	    return -1;
    }
    buffer[0] = (uint32)(HRT::HRTCounter32());
	memcpy(&buffer[1], incoming, numInputs * sizeof(int32)); //Inputs are always assumed to be float
    return numInputs;
}

/*
 * MUST be called before GetData
 * 
 */ 
bool EPICSDrv::Poll()
{
    for(int i = 0; i < nOfTriggeringServices; i++)
        triggerService[i].Trigger();
	return True;

}
    

bool EPICSDrv::PulseStart() 
{
    return True;
}

bool EPICSDrv::ProcessHttpMessage(HttpStream &hStream) {
    hStream.SSPrintf("OutputHttpOtions.Content-Type","text/html");
    hStream.keepAlive = False;

    hStream.Printf("<html><head><title>%s</title>", Name());
    hStream.Printf( "<style type=\"text/css\">\n" );
    hStream.Printf("%s\n", css);
    hStream.Printf( "</style></head><body>\n" );
    hStream.Printf("<table class=\"bltable\">\n");
    hStream.Printf("<br>Input PVs<br>\n");
    hStream.Printf("<tr>\n");
    hStream.Printf("<td> Name </td> <td>Value</td>\n");
    hStream.Printf("</tr>\n");
    for(int i = 0; i < numInputs; i++)
    {
        hStream.Printf("<tr>\n");
        hStream.Printf("<td> %s </td><td> %f </td>\n", inPvNames[i], incoming[i]);
        hStream.Printf("</tr>\n");
    }
    hStream.Printf("</table>\n");
    hStream.Printf("<br><br>Output PVs<br>\n");

    hStream.Printf("<table class=\"bltable\">\n");
    
    hStream.Printf("<tr>\n");
    hStream.Printf("<td> Name </td> <td>Value</td>\n");
    hStream.Printf("</tr>\n");
    for(int i = 0; i < numInputs; i++)
    {
        hStream.Printf("<tr>\n");
        hStream.Printf("<td> %s </td><td> %f </td>\n", outPvNames[i], outcoming[i]);
        hStream.Printf("</tr>\n");
    }
    hStream.Printf("</table>\n");
    hStream.Printf("<br>LastScanCount: %d\n", lastScanCount);
    
    hStream.Printf("</body></html>");
    hStream.WriteReplyHeader(True);
    return True;
}

OBJECTLOADREGISTER(EPICSDrv,"$Id:")
