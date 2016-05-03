//******************************************************************************
//    $Log: NI6368Drv.cpp,v $
//
//******************************************************************************

/*
 * NI6368 Linux User-Space IOGAM to Driver interface
 * author: Antonio Barbalace
 * date: oct 2011
 */



#include "CDBExtended.h"

#include "NI6368Drv.h"


NI6368Drv::NI6368Drv() {
	numInputChannels = 0;
	numOutputChannels = 0;
	lastCycleUsecTime = 0;
	mode = IS_NI6368_UNDEF;
	adcFd = -1;
	dacFd = -1;
	dioFd = -1;
	for(int i = 0; i < 32; i++)
	    chanAdcFd[i] = -1;
	for(int i = 0; i < 4; i++)
	    chanDacFd[i] = -1;
	configLoaded = false;
	dataAvailable = false;
	prevDacClock = -1;
	sampleCount = 0;
	acquisitionEnabled = false;
	isAnalog = true;
	started = false;
        dataBuffer = NULL;
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
NI6368Drv::~NI6368Drv() 
{
    for(int i = 0; i < 32; i++)
	if(chanAdcFd[i] != -1)
	    close(chanAdcFd[i]);
    for(int i = 0; i < 4; i++)
	if(chanDacFd[i] != -1)
	    close(chanDacFd[i]);
    if(adcFd != -1)
	close(adcFd);
    if(dacFd != -1)
	close(dacFd);
    if(dioFd != -1)
	close(dioFd);
    if(dataBuffer)
        delete [] dataBuffer;
}

/*
 * Enable System Acquisition
 */
bool NI6368Drv::EnableAcquisition() {
    char buf[64];
    switch(mode) {
	case IS_NI6368_ADC:
	    if(!configLoaded)
	    {
		//Allocate enough space for dataBuffer
                if(dataBufferSize == 0)     
	    	{
		    	AssertErrorCondition(FatalError, "NI6368Drv::EnableAcquisition: Data Buffer Size not yet defined.");
		    	return False;
	    	}
                if(dataBuffer)
                    delete [] dataBuffer;
                dataBuffer = new float[32 * dataBufferSize];
       	        sprintf(buf, "/dev/pxie-6368.%d.ai", boardId);
       	        adcFd = open(buf, O_RDWR);
       	        if (adcFd < 0)
	    	{
		    	AssertErrorCondition(FatalError, "NI6368Drv::EnableAcquisition: %s ADC open FAILED.", Name());
		    	return False;
	    	}
	/* Stop the segment */
	        xseries_stop_ai(adcFd);

	/* reset AI segment */
	        if(xseries_reset_ai(adcFd))
	        {
		    	AssertErrorCondition(FatalError, "NI6368Drv::EnableAcquisition: %s ADC reset FAILED.", Name());
		    	return False;
	    	}
                adcConf = xseries_continuous_ai();
       /* Add selecetd channel */
       	        for (int i=0; i< numInputChannels - 1; i++)
	        {
                    if(xseries_add_ai_channel(&adcConf, i,  gain, XSERIES_AI_CHANNEL_TYPE_DIFFERENTIAL, 0))
            	    {
		    	    AssertErrorCondition(FatalError, "NI6368Drv::EnableAcquisition: %s ADC add FAILED for channel %d.", Name(), i);
		    	    return False;
		    }
	    	}
	/* Set selected clock */
	        if(frequency > 0)
	    	{
		    int divisions = (int)(100000000/frequency + 0.5);
                    if(xseries_set_ai_scan_interval_counter(&adcConf, XSERIES_SCAN_INTERVAL_COUNTER_TB3, XSERIES_SCAN_INTERVAL_COUNTER_POLARITY_RISING_EDGE, divisions, 2))
            	    {
		    	    AssertErrorCondition(FatalError, "NI6368Drv::EnableAcquisition: %s ADC add FAILED for Clock Scan setting", Name());
		    	    return False;
		    }
		    if(xseries_set_ai_sample_clock(&adcConf,  XSERIES_AI_SAMPLE_CONVERT_CLOCK_INTERNALTIMING, XSERIES_AI_POLARITY_ACTIVE_HIGH_OR_RISING_EDGE, 1))
            	    {
		    	AssertErrorCondition(FatalError, "NI6368Drv::EnableAcquisition: %s ADC add FAILED for Clock setting", Name());
		    	    return False;
		    }
		}
	        else
	        { 
		    if(xseries_set_ai_sample_clock(&adcConf, XSERIES_AI_SAMPLE_CONVERT_CLOCK_PFI0, XSERIES_AI_POLARITY_ACTIVE_HIGH_OR_RISING_EDGE, 1))
            	    {
		         AssertErrorCondition(FatalError, "NI6368Drv::EnableAcquisition: %s ADC add FAILED for External Clock setting", Name());
		    	 return False;
		     }
                     if(xseries_set_ai_convert_clock(&adcConf, XSERIES_AI_SAMPLE_CONVERT_CLOCK_INTERNALTIMING, XSERIES_AI_POLARITY_ACTIVE_HIGH_OR_RISING_EDGE))
            	    {
		    	    AssertErrorCondition(FatalError, "NI6368Drv::EnableAcquisition: %s ADC add FAILED for External Clock conversion setting", Name());
		    	    return False;
		    }
                    if(xseries_set_ai_scan_interval_counter(&adcConf, XSERIES_SCAN_INTERVAL_COUNTER_TB3, XSERIES_SCAN_INTERVAL_COUNTER_POLARITY_RISING_EDGE, 50, 2))
            	    {
		    	    AssertErrorCondition(FatalError, "NI6368Drv::EnableAcquisition: %s ADC add FAILED for External Clock conversion scan setting", Name());
		    	    return False;
		    }
                 }
       /* load the configuration on the card */
                if(xseries_load_ai_conf(adcFd, adcConf))
                {
		    	AssertErrorCondition(FatalError, "NI6368Drv::EnableAcquisition: %s ADC configuration load  FAILED.", Name());
		    	return False;
	    	}

       /* Wait for device nodes to appear in /dev and open it */
           	sleep(1);
       /* Open channels */
       		for (int chan = 0;chan < numInputChannels - 1; chan++)
		{
               	    sprintf(buf, "/dev/pxie-6368.%d.ai.%d",  boardId, chan);
//               	    chanAdcFd[chan] = open(buf, O_RDWR | O_NONBLOCK);
               	    chanAdcFd[chan] = open(buf, O_RDWR);
      	            if(chanAdcFd[chan] < 0)
            	    {
		    	    AssertErrorCondition(FatalError, "NI6368Drv::EnableAcquisition: %s ADC open FAILED for channel %d.", Name(), chan);
		    	    return False;
		    }
		}
		configLoaded = true;
	    }
        return True;
	case IS_NI6368_DIO://Digital inputs
        if(!configLoaded)
        {
            sprintf(buf, "/dev/pxie-6368.%d.dio", boardId);
       	    dioFd = open(buf, O_RDWR);
       	    if (dioFd < 0)
	    {
		AssertErrorCondition(FatalError, "NI6368Drv::EnableAcquisition: %s DIO open FAILED.", Name());
		return False;
	    }
	    xseries_stop_di(dioFd);
	    xseries_stop_do(dioFd);
            diConfig = xseries_static_di(dioMask);
            doConfig = xseries_static_do(dioMask);
	    if(xseries_reset_dio(dioFd))
	    {
		AssertErrorCondition(FatalError, "NI6368Drv::EnableAcquisition: %s reset DIO FAILED.", Name());
		return False;
	    }
            sleep(1);
            if (xseries_load_di_conf(dioFd, diConfig)) 
 	    {
 		AssertErrorCondition(FatalError, "NI6368Drv::EnableAcquisition: %s load DI configuration  FAILED.", Name());
		return False;
	    }
            if (xseries_load_do_conf(dioFd, doConfig)) 
 	    {
		AssertErrorCondition(FatalError, "NI6368Drv::EnableAcquisition: %s load DO configuration  FAILED.", Name());
		return False;
	    }
            sleep (1);
	    configLoaded = True;
	    }
	return True;
	case IS_NI6368_DAC:
	    if(!configLoaded)
	    {
// Reset previousDacSamples
		    for(int i = 0; i < 4; i++)
		    	prevDacSamples[i] = 1E6;
/* DAC configuration */
            sprintf(buf,"/dev/pxie-6368.%d.ao",boardId);
       	    dacFd = open(buf, O_RDWR | O_NONBLOCK);
        	if (dacFd < 0) 
 	    	{
		        AssertErrorCondition(FatalError, "NI6368Drv::EnableAcquisition: %s open Analog Output FAILED.", Name());
		        return False;
	    	}
	        xseries_stop_ao(dacFd);
            sleep(1);
	        if(xseries_reset_ao(dacFd))
 	    	{
		        AssertErrorCondition(FatalError, "NI6368Drv::EnableAcquisition: %s reset Analog Output FAILED.", Name());
		        return False;
	    	}
            sleep(1);
         	dacConf = xseries_static_ao();
		    for(int chan = 0; chan < numOutputChannels; chan++)
		    {
        	    if (xseries_add_ao_channel(&dacConf,chan, XSERIES_OUTPUT_RANGE_10V))
	    	    {
		    	    AssertErrorCondition(FatalError, "NI6368Drv::EnableAcquisition: %s add AO channel FAILED.", Name());
		    	    return False;
	    	    }
        	} 

            if(xseries_load_ao_conf(dacFd, dacConf))
 	    	{
		        AssertErrorCondition(FatalError, "NI6368Drv::EnableAcquisition: %s load AO configuration FAILED.", Name());
		        return False;
	    	}
        	sleep(1);
		    for(int chan = 0; chan < numOutputChannels; chan++)
		    {
		        sprintf(buf,"/dev/pxie-6368.%d.ao.%d", boardId, chan);
        	        chanDacFd[chan] = open(buf, O_RDWR | O_NONBLOCK);
		        if(chanDacFd[chan] < 0)
            	        {
		    	    AssertErrorCondition(FatalError, "NI6368Drv::EnableAcquisition: %s DAC open FAILED for channel %d.", Name(), chan);
		    	    return False;
		        }
 		    }
        	sleep(1);
		    configLoaded = true;
	    }
	    return True;
	default:
	    AssertErrorCondition(FatalError, "NI6368Drv::EnableAcquisition: %s sEnable Acquisition called when mode not defined.", Name());
	    return False;

    }
    return True;
}

bool NI6368Drv::DisableAcquisition() {
    char buf[64];
    acquisitionEnabled = false;
    switch(mode) {
	if(!configLoaded) return True;
	case IS_NI6368_ADC:
      /* Stop the data acquisition */
       	if(xseries_stop_ai(adcFd))
	    {
		    AssertErrorCondition(FatalError, "NI6368Drv::DisableAcquisition: %s stop ADC FAILED.", Name());
		    return False;
	    }
	    return True;
    case IS_NI6368_DIO:
       	    if(xseries_stop_di(dioFd))
	    	{
		        AssertErrorCondition(FatalError, "NI6368Drv::DisableAcquisition: %s stop DI FAILED.", Name());
		        return False;
	    	}
       	    if(xseries_stop_do(dioFd))
	    	{
		        AssertErrorCondition(FatalError, "NI6368Drv::DisableAcquisition: %s stop DO FAILED.", Name());
		        return False;
	    	}
	    	return True;
	case IS_NI6368_DAC:
            if(xseries_stop_ao(dacFd))
	        {
		        AssertErrorCondition(FatalError, "NI6368Drv::DisableAcquisition: %s stop DAC FAILED.", Name());
		        return False;
	        }
	        return True;
    }
    return True;
}

/*
 * Board Parameters are:
 * - input channels (int)
 * - output channels (int)
 * - board PCI address (char*)
 * - board IRQ num (int)
 */
bool NI6368Drv::ObjectLoadSetup(ConfigurationDataBase &info,StreamInterface *err) 
{
    CDBExtended cdb(info);
	// load configuration setup
    if (!GenericAcqModule::ObjectLoadSetup(info,err)) {
        AssertErrorCondition(FatalError, "NI6368Drv::ObjectLoadSetup: %s GenericAcqModule::ObjectLoadSetup Failed", Name());
        return False;
    }

    // read the board ID
    if (!cdb.ReadInt32((int32&) boardId, "BoardId", 0))
    {
	    boardId = 0;
	    AssertErrorCondition(Warning, "NI6368Drv::ObjectLoadSetup: %s did not specify boardId. Assuming 0", Name());
    }

//Read whether analog or digital inputs  (in case of input module)
    // read if the module is synchronous
    FString analogInputsStr;
    if (!cdb.ReadFString(analogInputsStr, "IsAnalog")) 
    {
    	AssertErrorCondition(Warning, "NI6368Drv::ObjectLoadSetup: %s did not specify UseAnalogInputs entry. Assuming True", Name());
	    isAnalog = true;
    }
    else 
    {
	    if(analogInputsStr == "True")
	        isAnalog = true;
	    else
	        isAnalog = false;
    }

    // input channels number
    numInputChannels = 1;
    if (!cdb.ReadInt32((int32&) numInputChannels, "NumberOfInputs", 1) || numInputChannels < 0 || numInputChannels > 32)
    {
	    AssertErrorCondition(Warning, "NI6368Drv::ObjectLoadSetup: %s did not specify correct NumberOfInputs entry. Assuming 1 channel", Name());
    }
    //Buffer size
    cdb.ReadInt32((int32&) dataBufferSize, "BufferSize", 1);
    if( dataBufferSize < 0)
    {
            dataBufferSize = 1;
	    AssertErrorCondition(Warning, "NI6368Drv::ObjectLoadSetup: %s did not specify correct Buffer Size entry. Assuming Buffer Size 1", Name());
    }
    numInputChannels /= dataBufferSize;

    //Input Range
    FString gainStr;
    gain = XSERIES_INPUT_RANGE_10V; //Default +- 10V
    if (!cdb.ReadFString(gainStr, "InputRange")) 
    {
	    AssertErrorCondition(Warning, "NI6368Drv::ObjectLoadSetup: %s did not specify InputRange/ Assuming 10V", Name());
    }
    else
    {
	    if(gainStr == "10V")
	        gain = XSERIES_INPUT_RANGE_10V;
	    else if (gainStr == "5V")
	        gain = XSERIES_INPUT_RANGE_5V;
	    else if(gainStr == "2V")
	        gain = XSERIES_INPUT_RANGE_2V;
	    else if(gainStr == "1V")
	        gain = XSERIES_INPUT_RANGE_1V;
	    else if(gainStr == "500mV")
	        gain = XSERIES_INPUT_RANGE_500mV;
	    else if(gainStr == "200mV")
	        gain = XSERIES_INPUT_RANGE_200mV;
	    else if(gainStr == "100mV")
	        gain = XSERIES_INPUT_RANGE_100mV;
	    else
	        AssertErrorCondition(Warning, "NI6368Drv::ObjectLoadSetup: %s did not specify correct InputRange: %s / Assuming +-10V", Name(), gainStr.Buffer());
    }
    if (!cdb.ReadFloat(frequency, "Frequency", -1))
    {
	    AssertErrorCondition(Warning, "NI6368Drv::ObjectLoadSetup: %s did not specify Clock Frequency/ Assuming External Clock", Name());
	    frequency = -1;
	    period = 0;
    }
    if(frequency <= 0)
    {
	    AssertErrorCondition(FatalError, "NI6368Drv::ObjectLoadSetup: %s did  specify Invalid frequency", Name());
	    return False;
    }
    period = 1./frequency;
    if (!cdb.ReadInt32((int32&) numOutputChannels, "NumberOfOutputs", 1) || numOutputChannels < 0 || numOutputChannels > 4)
    {
	    AssertErrorCondition(Warning, "NI6368Drv::ObjectLoadSetup: %s did not specify correct NumberOfOutputs entry. Assuming 1 channel", Name());
    }
    if (!cdb.ReadInt32((int32&) dioMask, "DigitalMask", 0))
    {
	    AssertErrorCondition(Warning, "NI6368Drv::ObjectLoadSetup: %s did not specify DigitalMask. Assuming all digital inputs", Name());
    }
    AssertErrorCondition(Information,"NI6368Drv::ObjectLoadSetup %s Correctly Initialized ", Name());
    return True;
}

/*
 * object description
 */
bool NI6368Drv::ObjectDescription(StreamInterface &s,bool full,StreamInterface *err){
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
bool NI6368Drv::WriteData(uint32 usecTime, const int32 *buffer) {
	if (!buffer) {
	    AssertErrorCondition(FatalError,"NI6368Drv::WriteData: %s. buffer is null.", Name());
	    return False;		
	}
    if(!configLoaded)
	{
	    AssertErrorCondition(FatalError,"NI6368Drv::WriteData: %s. Module is not  initialized.", Name());
	    return False;		
	}
    switch(mode)
    {
        case IS_NI6368_DAC:
        {
	        int retVal;
	        for(int chan = 0; chan < numOutputChannels; chan++)
	        {
	            outVal = ((float *)buffer)[chan];
	            if(prevDacSamples[chan] == 1E6 || prevDacSamples[chan] != outVal)
	            {
		            int retval = xseries_write_ao(chanDacFd[chan], &outVal, 1);
		            if(retval != 1)
		            {
		    	        AssertErrorCondition(FatalError,"NI6368Drv::WriteData: %s. Error writing channel %d. Retval: %d", Name(), chan, retval);
		    	        return False;		
		             }
	             }
	            prevDacSamples[chan] = outVal;
	        }
	        sampleCount++;
	        return True;
        }
        case IS_NI6368_DIO:
            {   
                uint32_t outDigital = *(uint32_t *)buffer; 
                if(dioMask == 0) //All inputs
		        {
		            AssertErrorCondition(FatalError,"NI6368Drv::WriteData: %s. No pin configured for output", Name());
		    	    return False;		
		        }
            
                if (xseries_write_do(dioFd, &outDigital, 1) != 1)
		        {
		    	        AssertErrorCondition(FatalError,"NI6368Drv::WriteData: %s. Error writing Digital channel. dioFd: %d", Name(), dioFd);
		    	        return False;		
		        }
            }
            return True;
        default:
		    AssertErrorCondition(FatalError,"NI6368Drv::WriteData: %s. Cannot output to an ADC Device ", Name());
		    return False;		
    }   
}

/*
 * Get Data FROM Board
 */
int32 NI6368Drv::GetData(uint32 usecTime, int32 *buffer, int32 bufferNumber) 
{
    if(!buffer) 
    {
	    AssertErrorCondition(FatalError,"NI6368Drv::GetData: %s. buffer is null.", Name());
	    return -1;
    }
    if(!configLoaded)
    {
	    AssertErrorCondition(FatalError,"NI6368Drv::GetData: %s. Module is not initialized.", Name());
	    return -1;		
    }

    switch (mode) {
        case IS_NI6368_ADC:
    	    if(!dataAvailable)
	        Poll();

    	    memcpy(&buffer[1], dataBuffer, (numInputChannels - 1)* dataBufferSize * sizeof(float));
	    buffer[0] = lastCycleUsecTime;
    	    dataAvailable = false;

    	    return numInputChannels * dataBufferSize;
        case IS_NI6368_DIO:
            {
	            uint32_t currInput;
	            if (xseries_read_di(dioFd, &currInput, 1) != 1)
    	        {
	                AssertErrorCondition(FatalError,"NI6368Drv::GetData: %s. Error reading Digital Inputs.", Name());
	                return False;
    	        }

    	        buffer[0] = (uint32)(HRT::HRTCounter());
	        memcpy(&buffer[1], &currInput, sizeof(int32));
                return 1;
            }
        case IS_NI6368_DAC:
	        AssertErrorCondition(FatalError,"NI6368Drv::GetData: %s. Cannot read from a DAC device.", Name());
	        return False;
    }
}

/*
 * MUST be called before GetData
 * 
 */ 
bool NI6368Drv::Poll()
{
//printf("NI6368Drv::Poll() AcquisitionEnabled: %d ConfigLoaded: %d mode: %d \n", acquisitionEnabled, configLoaded, mode);
    if(!acquisitionEnabled)
    {
    	for(int i = 0; i < nOfTriggeringServices; i++)
            triggerService[i].Trigger();
	    return True;

    }
    if(!configLoaded)
    {
	    AssertErrorCondition(FatalError,"NI6368Drv::Poll: %s. Module is not configured.", Name());
	    return False;
    }
    if(mode == IS_NI6368_ADC)
    {
//Read data in buffer	
	    for(int chan = 0; chan < numInputChannels - 1; chan++)
	    {
                int readSamples = 0;
                while(readSamples < dataBufferSize)
                {
                    int currSamples = xseries_read_ai(chanAdcFd[chan], &dataBuffer[chan * dataBufferSize + readSamples], dataBufferSize - readSamples);
                    if(currSamples  > 0)
		    {
                    	readSamples += currSamples;
			if(chan == 0)
			{
    	    		    sampleCount+= currSamples;
	    		    lastCycleUsecTime = sampleCount * period * 1E6;
			}
		    }
                }
	    }
	    dataAvailable = true;
    	    for(int i = 0; i < nOfTriggeringServices; i++)
                triggerService[i].Trigger();
	    return True;
    }
    else
    {
    	for(int i = 0; i < nOfTriggeringServices; i++)
            triggerService[i].Trigger();
	    return True;
    }
}
    

bool NI6368Drv::PulseStart() 
{
    if(!acquisitionEnabled)
    {
    	if(!EnableAcquisition())
	        return false;	
	    acquisitionEnabled = true;
    }
    switch(mode)  {
        case IS_NI6368_ADC:
            if(started)
	    {
	/* Stop the segment */
	        xseries_stop_ai(adcFd);
       		for (int chan = 0;chan < numInputChannels - 1; chan++)
              	    close(chanAdcFd[chan]);
		close(adcFd);
		sleep(1);
		configLoaded = false;
		EnableAcquisition();				
	    }
	    started = true;
       	    if(xseries_start_ai(adcFd))
	    {
	        AssertErrorCondition(InitialisationError, "NI6368Drv::EnableAcquisition: %s start ADC FAILED.", Name());
	        return False;
	    }
            return True;
        case IS_NI6368_DIO:
       	    if(xseries_start_di(dioFd))
	        {
	            AssertErrorCondition(InitialisationError, "NI6368Drv::EnableAcquisition: %s start DI FAILED.", Name());
	            return False;
	        }
       	    if(xseries_start_do(dioFd))
	        {
	            AssertErrorCondition(InitialisationError, "NI6368Drv::EnableAcquisition: %s start DO FAILED.", Name());
	            return False;
	        }
            return True;
        case IS_NI6368_DAC:
            if(xseries_start_ao(dacFd))
	        {
	            AssertErrorCondition(InitialisationError, "NI6368Drv::EnableAcquisition: %s start DAC FAILED.", Name());
	            return False;
	        }
	        return True;
    }
    return True;
}

bool NI6368Drv::ProcessHttpMessage(HttpStream &hStream) {
    hStream.SSPrintf("OutputHttpOtions.Content-Type","text/html");
    hStream.keepAlive = False;

    hStream.Printf("<html><head><title>%s</title>", Name());
    hStream.Printf( "<style type=\"text/css\">\n" );
    hStream.Printf("%s\n", css);
    hStream.Printf( "</style></head><body>\n" );
    
    hStream.Printf("<tr>\n");
    switch(mode) {
        case IS_NI6368_ADC:
            hStream.Printf("Configuration: ADC<br>");
            hStream.Printf("<table class=\"bltable\">\n");
    	    hStream.Printf("<td> Input Channels </td> <td>%d</td>\n", numInputChannels);
    	    hStream.Printf("</tr>\n");
    	    hStream.Printf("<tr>\n");
    	    hStream.Printf("<td> Buffer size </td> <td>%d</td>\n", dataBufferSize);
    	    hStream.Printf("</tr>\n");
    	    hStream.Printf("<tr>\n");
	        switch(gain)
	        {
	            case 1: hStream.Printf("<td> InputRange </td> <td>10V</td>\n"); break;
	            case 2: hStream.Printf("<td> InputRange </td> <td>5V</td>\n"); break;
	            case 3: hStream.Printf("<td> InputRange </td> <td>2V</td>\n"); break;
	            case 4: hStream.Printf("<td> InputRange </td> <td>1V</td>\n"); break;
	            case 5: hStream.Printf("<td> InputRange </td> <td>500mVV</td>\n"); break;
	            case 6: hStream.Printf("<td> InputRange </td> <td>200mV</td>\n"); break;
	            case 7: hStream.Printf("<td> InputRange </td> <td>100mV</td>\n"); break;
	        }	
    	    hStream.Printf("</tr>\n");
    	    hStream.Printf("<tr>\n");
	        if(frequency > 0)
	            hStream.Printf("<td> Frequency</td> <td>%f</td>\n", frequency);
	        else
	            hStream.Printf("<td> Frequency</td> <td>External</td>\n");
    	    hStream.Printf("</tr></table>\n");
            break;
        case IS_NI6368_DAC:
            hStream.Printf("Configuration: ADC<br>");
            hStream.Printf("<table class=\"bltable\">\n");
    	    hStream.Printf("<td> outputChannels </td> <td>%d</td>\n", numOutputChannels);
    	    hStream.Printf("</tr></table>\n");
            break;
        case IS_NI6368_DIO:
            hStream.Printf("Configuration: DIO<br>");
            hStream.Printf("<table class=\"bltable\">\n");
            hStream.Printf("<tr>");
            for(int i = 31; i >= 0; i--)
                hStream.Printf("<td>%d</td>", i);
            hStream.Printf("</tr>\n<tr>");
            for(int i = 31; i >= 0; i--)
            {
                if(dioMask & (1 << i))
                    hStream.Printf("<td>O</td>");
                else
                    hStream.Printf("<td>I</td>");
            }
            hStream.Printf("</tr></table>\n");
    }
//For all modes show sample counter
    hStream.Printf("Current samples: %d <br>\n", sampleCount);
    hStream.Printf("</body></html>");
    hStream.WriteReplyHeader(True);
    return True;
}

OBJECTLOADREGISTER(NI6368Drv,"$Id:")
