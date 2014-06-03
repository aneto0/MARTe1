//******************************************************************************
//    $Log: NI6259Drv.cpp,v $
//
//******************************************************************************

/*
 * NI6259 Linux User-Space IOGAM to Driver interface
 * author: Antonio Barbalace
 * date: oct 2011
 */



#include "CDBExtended.h"

#include "NI6259Drv.h"


NI6259Drv::NI6259Drv() {
	numInputChannels = 0;
	numOutputChannels = 0;
	lastCycleUsecTime = -1;
	mode = IS_NI6259_UNDEF;
	adcFd = -1;
	dacFd = -1;
	dioFd = -1;
	for(int i = 0; i < 32; i++)
	    chanAdcFd[i] = -1;
	for(int i = 0; i < 4; i++)
	    chanDacFd[i] = -1;
	chanDioFd = -1;
	configLoaded = false;
	dataAvailable = false;
	prevDacClock = -1;
	sampleCount = 0;
	acquisitionEnabled = false;
	isAnalog = true;
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
NI6259Drv::~NI6259Drv() 
{
    for(int i = 0; i < 32; i++)
	if(chanAdcFd[i] != -1)
	    close(chanAdcFd[i]);
    for(int i = 0; i < 4; i++)
	if(chanDacFd[i] != -1)
	    close(chanDacFd[i]);
    if(chanDioFd != -1)
	close(chanDioFd);
    if(adcFd != -1)
	close(adcFd);
    if(dacFd != -1)
	close(dacFd);
    if(dioFd != -1)
	close(dioFd);
}

/*
 * Enable System Acquisition
 */
bool NI6259Drv::EnableAcquisition() {
    char buf[64];
printf("ENABLE ACQUISITION MODE: %d\n", mode);
    switch(mode) {
	case IS_NI6259_ADC:
	    if(!configLoaded)
	    {
       	    sprintf(buf, "/dev/pxi6259.%d.ai", boardId);
       	    adcFd = open(buf, O_RDWR);
       	    if (adcFd < 0)
	    	{
		    	AssertErrorCondition(FatalError, "NI6259Drv::EnableAcquisition: %s ADC open FAILED.", Name());
		    	return False;
	    	}
            adcConf = pxi6259_create_ai_conf();
       /* Add selecetd channel */
       	    for (int i=0; i< numInputChannels - 1; i++)
	        {
                if(pxi6259_add_ai_channel(&adcConf, i,  polarity, gain, inputMode, 0))
            	{
		    	    AssertErrorCondition(FatalError, "NI6259Drv::EnableAcquisition: %s ADC add FAILED for channel %d.", Name(), i);
		    	    return False;
		    	}
	    	}
	/* Set selected clock */
	        if(frequency > 0)
	    	{
		    	int divisions = (int)(20000000/frequency + 0.5);
//		    	if(pxi6259_set_ai_sample_clk(&adcConf, divisions, 3, AI_SAMPLE_SELECT_SI_TC, AI_SAMPLE_POLARITY_RISING_EDGE))
		    	if(pxi6259_set_ai_sample_clk(&adcConf, divisions, 3, AI_SAMPLE_SELECT_SI_TC,AI_SAMPLE_POLARITY_ACTIVE_HIGH_OR_RISING_EDGE))
            	{
		    	    AssertErrorCondition(FatalError, "NI6259Drv::EnableAcquisition: %s ADC add FAILED for Clock setting", Name());
		    	    return False;
		    	}
		    }
	        else
	        { 

//		    	if(pxi6259_set_ai_sample_clk(&adcConf, 16, 3, AI_SAMPLE_SELECT_PFI0, AI_SAMPLE_POLARITY_RISING_EDGE))
		    	if(pxi6259_set_ai_sample_clk(&adcConf, 16, 3, AI_SAMPLE_SELECT_PFI0, AI_SAMPLE_POLARITY_ACTIVE_HIGH_OR_RISING_EDGE))
            	{
		    	    AssertErrorCondition(FatalError, "NI6259Drv::EnableAcquisition: %s ADC add FAILED for External Clock setting", Name());
		    	    return False;
		        }
	    	}
       /* load the configuration on the card */
            if(pxi6259_load_ai_conf(adcFd, &adcConf))
            {
		    	AssertErrorCondition(FatalError, "NI6259Drv::EnableAcquisition: %s ADC configuration load  FAILED.", Name());
		    	return False;
	    	}

       /* Wait for device nodes to appear in /dev and open it */
           	sleep(1);
       /* Open channels */
       		for (int chan = 0;chan < numInputChannels - 1; chan++)
		    {
               	sprintf(buf, "/dev/pxi6259.%d.ai.%d",  boardId, chan);
               	chanAdcFd[chan] = open(buf, O_RDWR | O_NONBLOCK);
		    	if(chanAdcFd[chan] < 0)
            	{
		    	    AssertErrorCondition(FatalError, "NI6259Drv::EnableAcquisition: %s ADC open FAILED for channel %d.", Name(), chan);
		    	    return False;
		    	}
		    }
		    configLoaded = true;
	    }
        return True;
	case IS_NI6259_DIO://Digital inputs
        if(!configLoaded)
        {
printf("LOAD DIO CONFIGURATION\n");
            sprintf(buf, "/dev/pxi6259.%d.dio", boardId);
       	    dioFd = open(buf, O_RDWR);
       	    if (dioFd < 0)
	        {
		        AssertErrorCondition(FatalError, "NI6259Drv::EnableAcquisition: %s DIO open FAILED.", Name());
		        return False;
	        }
            dioConfig = pxi6259_create_dio_conf();
		/* PortMask defines input and output pins (P0)*/
       		if (pxi6259_add_dio_channel(&dioConfig,DIO_PORT0,dioMask)) 
	    	{
		    	AssertErrorCondition(FatalError, "NI6259Drv::EnableAcquisition: %s DIO channel add  FAILED.", Name());
		    	return False;
	    	}
        	if (pxi6259_load_dio_conf(dioFd, &dioConfig)) 
 	    	{
		    	AssertErrorCondition(FatalError, "NI6259Drv::EnableAcquisition: %s load configuration  FAILED.", Name());
		    	return False;
	    	}
        	sleep (1);
		/* Open channel corresponding to the selected port */
            sprintf(buf,"/dev/pxi6259.%d.dio.0",boardId);
            chanDioFd = open(buf, O_RDWR | O_NONBLOCK);
            if (chanDioFd < 0) 
 	    	{
		    	AssertErrorCondition(FatalError, "NI6259Drv::EnableAcquisition: %s open DIO channel FAILED.", Name());
		    	return False;
	    	}
		    configLoaded = true;
	    }
	    return True;
	case IS_NI6259_DAC:
	    if(!configLoaded)
	    {
printf("LOAD AO CONFIGURATION\n");
// Reset previousDacSamples
		    for(int i = 0; i < 4; i++)
		    	prevDacSamples[i] = 1E6;
/* DAC configuration */
            sprintf(buf,"/dev/pxi6259.%d.ao",boardId);
       	    dacFd = open(buf, O_RDWR | O_NONBLOCK);
        	if (dacFd < 0) 
 	    	{
		        AssertErrorCondition(FatalError, "NI6259Drv::EnableAcquisition: %s open Analog Output FAILED.", Name());
		        return False;
	    	}
         	dacConf = pxi6259_create_ao_conf();
        	if (pxi6259_set_ao_attribute(&dacConf,
                        AO_SIGNAL_GENERATION, AO_SIGNAL_GENERATION_STATIC))
		    {
		        AssertErrorCondition(FatalError, "NI6259Drv::EnableAcquisition: %s set AO attributes FAILED.", Name());
		        return False;
        	}        
		    if (pxi6259_set_ao_attribute(&dacConf, AO_CONTINUOUS, 0))
 	    	{
		        AssertErrorCondition(FatalError, "NI6259Drv::EnableAcquisition: %s set AO attributes FAILED.", Name());
		        return False;
	    	}
		    for(int chan = 0; chan < numOutputChannels; chan++)
		    {
        	    if (pxi6259_add_ao_channel(&dacConf,chan, AO_DAC_POLARITY_BIPOLAR))
	    	    {
		    	    AssertErrorCondition(FatalError, "NI6259Drv::EnableAcquisition: %s add AO channel FAILED.", Name());
		    	    return False;
	    	    }
        	} 
            if(pxi6259_load_ao_conf(dacFd, &dacConf))
 	    	{
		        AssertErrorCondition(FatalError, "NI6259Drv::EnableAcquisition: %s load AO configuration FAILED.", Name());
		        return False;
	    	}
        	sleep(1);
		    for(int chan = 0; chan < numOutputChannels; chan++)
		    {
		        sprintf(buf,"/dev/pxi6259.%d.ao.%d", boardId, chan);
        	    chanDacFd[chan] = open(buf, O_RDWR);
		        if(chanDacFd[chan] < 0)
            	{
		    	    AssertErrorCondition(FatalError, "NI6259Drv::EnableAcquisition: %s DAC open FAILED for channel %d.", Name(), chan);
		    	    return False;
		        }
 		    }
		    configLoaded = true;
	    }
	    return True;
	default:
	    AssertErrorCondition(FatalError, "NI6259Drv::EnableAcquisition: %s sEnable Acquisition called when mode not defined.", Name());
	    return False;

    }
    return True;
}

bool NI6259Drv::DisableAcquisition() {
printf("NI6259::DisableAcquisition\n");

    char buf[64];
    acquisitionEnabled = false;
    switch(mode) {
	if(!configLoaded) return True;
	case IS_NI6259_ADC:
      /* Stop the data acquisition */
       	if(pxi6259_stop_ai(adcFd))
	    {
		    AssertErrorCondition(FatalError, "NI6259Drv::DisableAcquisition: %s stop ADC FAILED.", Name());
		    return False;
	    }
	    return True;
    case IS_NI6259_DIO:
       	    if(pxi6259_stop_dio(dioFd))
	    	{
		        AssertErrorCondition(FatalError, "NI6259Drv::DisableAcquisition: %s stop DIO FAILED.", Name());
		        return False;
	    	}
	    	return True;
	case IS_NI6259_DAC:
            if(pxi6259_stop_ao(dacFd))
	        {
		        AssertErrorCondition(FatalError, "NI6259Drv::DisableAcquisition: %s stop DAC FAILED.", Name());
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
bool NI6259Drv::ObjectLoadSetup(ConfigurationDataBase &info,StreamInterface *err) 
{
    CDBExtended cdb(info);
	// load configuration setup
    if (!GenericAcqModule::ObjectLoadSetup(info,err)) {
        AssertErrorCondition(FatalError, "NI6259Drv::ObjectLoadSetup: %s GenericAcqModule::ObjectLoadSetup Failed", Name());
        return False;
    }

    // read the board ID
    if (!cdb.ReadInt32((int32&) boardId, "BoardId", 0))
    {
	    boardId = 0;
	    AssertErrorCondition(Warning, "NI6259Drv::ObjectLoadSetup: %s did not specify boardId. Assuming 0", Name());
    }

//Read whether analog or digital inputs  (in case of input module)
    // read if the module is synchronous
    FString analogInputsStr;
    if (!cdb.ReadFString(analogInputsStr, "IsAnalog")) 
    {
    	AssertErrorCondition(Warning, "NI6259Drv::ObjectLoadSetup: %s did not specify UseAnalogInputs entry. Assuming True", Name());
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
	    AssertErrorCondition(Warning, "NI6259Drv::ObjectLoadSetup: %s did not specify correct NumberOfInputs entry. Assuming 1 channel", Name());
    }
    FString gainStr;
    gain = 1; //Default +- 10V
    if (!cdb.ReadFString(gainStr, "InputRange")) 
    {
	    AssertErrorCondition(Warning, "NI6259Drv::ObjectLoadSetup: %s did not specify InputRange/ Assuming 10V", Name());
    }
    else
    {
	    if(gainStr == "10V")
	        gain = 1;
	    else if (gainStr == "5V")
	        gain = 2;
	    else if(gainStr == "2V")
	        gain = 3;
	    else if(gainStr == "1V")
	        gain = 4;
	    else if(gainStr == "500mV")
	        gain = 5;
	    else if(gainStr == "200mV")
	        gain = 6;
	    else if(gainStr == "100mV")
	        gain = 7;
	    else
	        AssertErrorCondition(Warning, "NI6259Drv::ObjectLoadSetup: %s did not specify correct InputRange: %s / Assuming +-10V", Name(), gainStr.Buffer());
    }
    polarity = AI_POLARITY_BIPOLAR;
    FString polarityStr;
    if (!cdb.ReadFString(polarityStr, "Polarity")) 
    {
	    AssertErrorCondition(Warning, "NI6259Drv::ObjectLoadSetup: %s did not specify Polarity/ Assuming Bipolar", Name());
    }
    else
    {
	    if(polarityStr == "Unipolar")
	        polarity = AI_POLARITY_UNIPOLAR;
	    else if(polarityStr == "Bipolar")
	        polarity = AI_POLARITY_BIPOLAR;
	    else
		    AssertErrorCondition(Warning, "NI6259Drv::ObjectLoadSetup: %s did not specify Correct Polarity: %s/ Assuming Bipolar", Name(), polarityStr.Buffer());
    }
    inputMode = AI_CHANNEL_TYPE_RSE;
    FString inputModeStr;
    if (!cdb.ReadFString(polarityStr, "InputMode")) 
    {
	    AssertErrorCondition(Warning, "NI6259Drv::ObjectLoadSetup: %s did not specify InputMode/ Assuming Differential", Name());
    }
    else
    {
	    if(inputModeStr == "Differential")
	        inputMode = AI_CHANNEL_TYPE_DIFFERENTIAL;
	    else if(inputModeStr == "NRSE")
	        inputMode = AI_CHANNEL_TYPE_NRSE;
	    else if(inputModeStr == "RSE")
	        inputMode = AI_CHANNEL_TYPE_RSE;
	    else
		    AssertErrorCondition(Warning, "NI6259Drv::ObjectLoadSetup: %s did not specify Correct InputMode: %s/ Assuming RSE", Name(), inputModeStr.Buffer());
    }
    if (!cdb.ReadFloat(frequency, "Frequency", -1))
    {
	    AssertErrorCondition(Warning, "NI6259Drv::ObjectLoadSetup: %s did not specify Clock Frequency/ Assuming External Clock", Name());
	    frequency = -1;
    }
    if (!cdb.ReadInt32((int32&) numOutputChannels, "NumberOfOutputs", 1) || numOutputChannels < 0 || numOutputChannels > 4)
    {
	    AssertErrorCondition(Warning, "NI6259Drv::ObjectLoadSetup: %s did not specify correct NumberOfOutputs entry. Assuming 1 channel", Name());
    }
    if (!cdb.ReadInt32((int32&) dioMask, "DigitalMask", 0))
    {
	    AssertErrorCondition(Warning, "NI6259Drv::ObjectLoadSetup: %s did not specify DigitalMask. Assuming all digital inputs", Name());
    }
    AssertErrorCondition(Information,"NI6259Drv::ObjectLoadSetup %s Correctly Initialized ", Name());
    return True;
}

/*
 * object description
 */
bool NI6259Drv::ObjectDescription(StreamInterface &s,bool full,StreamInterface *err){
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
bool NI6259Drv::WriteData(uint32 usecTime, const int32 *buffer) {
	if (!buffer) {
	    AssertErrorCondition(FatalError,"NI6259Drv::WriteData: %s. buffer is null.", Name());
	    return False;		
	}
    if(!configLoaded)
	{
	    AssertErrorCondition(FatalError,"NI6259Drv::WriteData: %s. Module is not  initialized.", Name());
	    return False;		
	}
    switch(mode)
    {
        case IS_NI6259_DAC:
        {
	        int retVal;
	        for(int chan = 0; chan < numOutputChannels; chan++)
	        {
	            float outVal = ((float *)buffer)[chan];
	            if(prevDacSamples[chan] == 1E6 || prevDacSamples[chan] != outVal)
	            {
		            int retval = pxi6259_write_ao(chanDacFd[chan], &outVal, 1);
		            if(retval != 1)
		            {
		    	        AssertErrorCondition(FatalError,"NI6259Drv::WriteData: %s. Error writing channel %d. Retval: %d", Name(), chan, retval);
		    	        return False;		
		            }
	             }
	            prevDacSamples[chan] = outVal;
	        }
	        sampleCount++;
	        return True;
        }
        case IS_NI6259_DIO:
            {   
                int outDigital = *(int *)buffer; 
                if(dioMask == 0) //All inputs
		        {
		            AssertErrorCondition(FatalError,"NI6259Drv::WriteData: %s. No pin configured for output", Name());
		    	    return False;		
		        }
            
                if (write(chanDioFd, &outDigital, 4) != 4)
		        {
		    	        AssertErrorCondition(FatalError,"NI6259Drv::WriteData: %s. Error writing Digital channel", Name());
		    	        return False;		
		        }
            }
            return True;
        default:
		    AssertErrorCondition(FatalError,"NI6259Drv::WriteData: %s. Cannot output to an ADC Device ", Name());
		    return False;		
    }   
}

/*
 * Get Data FROM Board
 */
int32 NI6259Drv::GetData(uint32 usecTime, int32 *buffer, int32 bufferNumber) 
{

    if(!buffer) 
    {
	    AssertErrorCondition(FatalError,"NI6259Drv::GetData: %s. buffer is null.", Name());
	    return -1;
    }
    if(!configLoaded)
    {
	    AssertErrorCondition(FatalError,"NI6259Drv::GetData: %s. Module is not initialized.", Name());
	    return -1;		
    }

    switch (mode) {
        case IS_NI6259_ADC:
    	    if(!dataAvailable)
	            Poll();
    	    memcpy(&buffer[1], dataBuffer, (numInputChannels - 1)* sizeof(float));
    	    buffer[0] = (uint32)(HRT::HRTCounter());
    	    dataAvailable = false;
    	    sampleCount++;
    	    return numInputChannels;
        case IS_NI6259_DIO:
            {
	            int32 currInput;
	            if (read (chanDioFd, &currInput, 4) != 4)
    	        {
	                AssertErrorCondition(FatalError,"NI6259Drv::GetData: %s. Error reading Digital Inputs.", Name());
	                return False;
    	        }

//printf("DIN: %d\n", currInput);
    	        buffer[0] = (uint32)(HRT::HRTCounter());
	            memcpy(&buffer[1], &currInput, sizeof(int32));
                return 1;
            }
        case IS_NI6259_DAC:
	        AssertErrorCondition(FatalError,"NI6259Drv::GetData: %s. Cannot read from a DAC device.", Name());
	        return False;
    }
}

/*
 * MUST be called before GetData
 * 
 */ 
bool NI6259Drv::Poll()
{
//printf("NI6259Drv::Poll() AcquisitionEnabled: %d ConfigLoaded: %d mode: %d \n", acquisitionEnabled, configLoaded, mode);
    if(!acquisitionEnabled)
    {
    	for(int i = 0; i < nOfTriggeringServices; i++)
            triggerService[i].Trigger();
	    return True;

    }
    if(!configLoaded)
    {
	    AssertErrorCondition(FatalError,"NI6259Drv::Poll: %s. Module is not configured.", Name());
	    return False;
    }
    if(mode == IS_NI6259_ADC)
    {
//Read data in buffer	
	    for(int chan = 0; chan < numInputChannels - 1; chan++)
	    {
            int readSamples;
	    	while((readSamples = pxi6259_read_ai(chanAdcFd[chan], &dataBuffer[chan], 1)) < 1);
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
    

bool NI6259Drv::PulseStart() 
{
printf("NI6259Drv::PULSE_START mode: %d\n", mode);
    if(!acquisitionEnabled)
    {
    	if(!EnableAcquisition())
	        return false;	
	    acquisitionEnabled = true;
    }
    switch(mode)  {
        case IS_NI6259_ADC:
       	    if(pxi6259_start_ai(adcFd))
	        {
	            AssertErrorCondition(InitialisationError, "NI6259Drv::EnableAcquisition: %s start ADC FAILED.", Name());
	            return False;
	        }
            return True;
        case IS_NI6259_DIO:
       	    if(pxi6259_start_dio(dioFd))
	        {
	            AssertErrorCondition(InitialisationError, "NI6259Drv::EnableAcquisition: %s start DIO FAILED.", Name());
	            return False;
	        }
            return True;
        case IS_NI6259_DAC:
            if(pxi6259_start_ao(dacFd))
	        {
	            AssertErrorCondition(InitialisationError, "NI6259Drv::EnableAcquisition: %s start DAC FAILED.", Name());
	            return False;
	        }
	        return True;
    }
    return True;
}

bool NI6259Drv::ProcessHttpMessage(HttpStream &hStream) {
    hStream.SSPrintf("OutputHttpOtions.Content-Type","text/html");
    hStream.keepAlive = False;

    hStream.Printf("<html><head><title>%s</title>", Name());
    hStream.Printf( "<style type=\"text/css\">\n" );
    hStream.Printf("%s\n", css);
    hStream.Printf( "</style></head><body>\n" );
    
    hStream.Printf("<tr>\n");
    switch(mode) {
        case IS_NI6259_ADC:
            hStream.Printf("Configuration: ADC<br>");
            hStream.Printf("<table class=\"bltable\">\n");
    	    hStream.Printf("<td> inputChannels </td> <td>%d</td>\n", numInputChannels);
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
	        switch(inputMode)
	        {
	            case AI_CHANNEL_TYPE_DIFFERENTIAL: hStream.Printf("<td> InputMode </td> <td>Differential</td>\n"); break;
	            case AI_CHANNEL_TYPE_NRSE: hStream.Printf("<td> InputMode </td> <td>NRSE</td>\n"); break;
	            case AI_CHANNEL_TYPE_RSE: hStream.Printf("<td> InputMode </td> <td>RSE</td>\n"); break;
	        }	
    	    hStream.Printf("</tr>\n");
    	    hStream.Printf("<tr>\n");
	        switch(polarity)
	        {
	            case AI_POLARITY_UNIPOLAR: hStream.Printf("<td> Polarity </td> <td>Unipolar</td>\n"); break;
	            case AI_POLARITY_BIPOLAR: hStream.Printf("<td> Polarity </td> <td>Bipolar</td>\n"); break;
	        }	
    	    hStream.Printf("</tr>\n");
    	    hStream.Printf("<tr>\n");
	        if(frequency > 0)
	            hStream.Printf("<td> Frequency</td> <td>%f</td>\n", frequency);
	        else
	            hStream.Printf("<td> Frequency</td> <td>External</td>\n");
    	    hStream.Printf("</tr></table>\n");
            break;
        case IS_NI6259_DAC:
            hStream.Printf("Configuration: ADC<br>");
            hStream.Printf("<table class=\"bltable\">\n");
    	    hStream.Printf("<td> outputChannels </td> <td>%d</td>\n", numOutputChannels);
    	    hStream.Printf("</tr></table>\n");
            break;
        case IS_NI6259_DIO:
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

OBJECTLOADREGISTER(NI6259Drv,"$Id:")
