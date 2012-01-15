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

#include "WaterTank.h"
#include "CDBExtended.h"
#include "File.h"

#include "DDBInputInterface.h"
#include "DDBOutputInterface.h"

bool WaterTank::Initialise(ConfigurationDataBase& cdbData) {

    CDBExtended cdb(cdbData);
    ////////////////////////////////////////////////////
    //                Add interfaces to DDB           //
    ////////////////////////////////////////////////////
    if(!AddInputInterface(input,"InputInterface")){
        AssertErrorCondition(InitialisationError,"WaterTank::Initialise: %s failed to add input interface",Name());
        return False;
    }

    if(!AddOutputInterface(output,"OutputInterface")){
        AssertErrorCondition(InitialisationError,"WaterTank::Initialise: %s failed to add output interface",Name());
        return False;
    }


    ////////////////////////////////////////////////////
    //                Add input signals               //
    ////////////////////////////////////////////////////
    if(!cdb->Move("InputSignals")){
        AssertErrorCondition(InitialisationError,"WaterTank::Initialise: %s did not specify InputSignals entry",Name());
        return False;
    }
    int32 nOfSignals = cdb->NumberOfChildren();
    if(nOfSignals != 2){
        AssertErrorCondition(InitialisationError, "WaterTank::Initialise: %s ObjectLoadSetup. WaterTank expects the current time and voltage as inputs ",Name());
        return False;
    }
    if(!input->ObjectLoadSetup(cdb,NULL)){
        AssertErrorCondition(InitialisationError,"WaterTank::Initialise: %s ObjectLoadSetup Failed DDBInterface %s ",Name(),input->InterfaceName());
        return False;
    }   
    cdb->MoveToFather();

    ////////////////////////////////////////////////////
    //                Add output signals              //
    ////////////////////////////////////////////////////
    if(!cdb->Move("OutputSignals")){
        AssertErrorCondition(InitialisationError,"WaterTank::Initialise: %s did not specify OutputSignals entry",Name());
        return False;
    }
    if(!output->ObjectLoadSetup(cdb,NULL)){
        AssertErrorCondition(InitialisationError,"WaterTank::Initialise: %s ObjectLoadSetup Failed DDBInterface %s ",Name(),output->InterfaceName());
        return False;
    }
    nOfSignals = cdb->NumberOfChildren();
    if(nOfSignals > 2){
        AssertErrorCondition(Warning,"WaterTank::Initialise: %s ObjectLoadSetup. Only the height signal and pump voltage are going to be given as output of this GAM ",Name());
    }
    cdb->MoveToFather();

    ////////////////////////////////////////////////////
    //            Configuration parameters            //
    ////////////////////////////////////////////////////
    {
        if(!cdb.ReadFloat(aFlowRate, "aFlowRate", 20)){
            AssertErrorCondition(Information,"WaterTank %s::Initialise: output flow rate not specified. Using default %f", Name(), aFlowRate);
        }

        if(!cdb.ReadFloat(bFlowRate, "bFlowRate", 50)){
            AssertErrorCondition(Information,"WaterTank %s::Initialise: input flow rate not specified. Using default %f", Name(), bFlowRate);
        }

        if(!cdb.ReadFloat(tankArea, "TankArea", 20)){
            AssertErrorCondition(Information,"WaterTank %s::Initialise: tank area not specified. Using default %f", Name(), tankArea);
        }

        if(!cdb.ReadFloat(maxVoltage, "MaxVoltage", 5)){
            AssertErrorCondition(Information,"WaterTank %s::Initialise: maximum voltage not specified. Using default %f", Name(), maxVoltage);
        }

        if(!cdb.ReadFloat(minVoltage, "MinVoltage", 0)){
            AssertErrorCondition(Information,"WaterTank %s::Initialise: minimum voltage not specified. Using default %f", Name(), minVoltage);
        }

        FString svgLocation;
        if(!cdb.ReadFString(svgLocation, "SVGFileLocation")){
            AssertErrorCondition(Information,"WaterTank %s::Initialise: SVGLocation web location not set, switching off animation", Name());
            animationEnabled = False;
        }
        else{
            int32 animationEnabledInt = 0;
            cdb.ReadInt32(animationEnabledInt, "AnimationEnabled", 1);
            animationEnabled = (animationEnabledInt != 0);
            //Try to load the svg file
            if(animationEnabled){
                File svgFile;
                if(!svgFile.OpenRead(svgLocation.Buffer())){                    
                    AssertErrorCondition(Warning,"WaterTank %s::Initialise: Could not open the svg file: %s", Name(), svgLocation.Buffer());
                    animationEnabled = 0;
                }
                else{
                    char   buffer[1024];
                    uint32 size = 1024;
                    while(svgFile.Read(buffer, size)){
                        svg.Write(buffer, size);
                        size = 1024;
                    }
                    svgFile.Close();
                }
            }
        }
    }
    return True;
}


bool WaterTank::Execute(GAM_FunctionNumbers functionNumber){
    // Get input and output data pointers
    input->Read();
    int32 usecTime    = *((int32*)input->Buffer());
    float voltage     = ((float *)input->Buffer())[1];
    float *outputBuff = (float*)  output->Buffer();
    float height      = 0;

    switch( functionNumber ){
        case GAMPrepulse:{
            // Reset last usec time and height
            lastUsecTime = 0;
            lastHeight   = 0;
        }break;
        case GAMOnline:{
            //Saturate voltage
            if(voltage > maxVoltage){
                voltage = maxVoltage;
            }
            if(voltage < minVoltage){
                voltage = minVoltage;
            }            
            //simple Euler method
            height   = (voltage * bFlowRate - aFlowRate * sqrt(lastHeight)) / tankArea * (usecTime - lastUsecTime) * 1e-6 + lastHeight;
            if(height < 0){
                AssertErrorCondition(Warning, "WaterTank %s::Initialise: Tank height is negative: %f", Name(), height);
                height = 0;
            }
            lastHeight      = height;
            lastUsecTime    = usecTime;
            lastVoltage     = voltage;
            *outputBuff     = height;
            *(outputBuff+1) = voltage;
        }break;
    }

    // Update the data output buffer
    output->Write();

    return True;
}

bool WaterTank::ProcessHttpMessage(HttpStream &hStream) {
    if(hStream.Switch("InputCommands.svgFile")){
        hStream.SSPrintf("OutputHttpOtions.Content-Type","image/svg+xml");
        hStream.Switch((uint32)0);
        hStream.Printf("%s", svg.Buffer());
    }else if(hStream.Switch("InputCommands.TankHeight")) {
        hStream.SSPrintf("OutputHttpOtions.Content-Type","text/html");
        hStream.Switch((uint32)0);
        hStream.Printf("%f\n", lastHeight);
    }
    else if(hStream.Switch("InputCommands.Voltage")) {
        hStream.SSPrintf("OutputHttpOtions.Content-Type","text/html");
        hStream.Switch((uint32)0);
        hStream.Printf("%f\n", lastVoltage);
    }else{
        hStream.SSPrintf("OutputHttpOtions.Content-Type","text/html");
        hStream.Printf("<html><head><title>WaterTank</title><head><body>\n");
        if(animationEnabled){
            hStream.Printf("<object id=\"waterTankID\" data=\"?svgFile\" width=\"100%%\" height=\"100%%\" type=\"image/svg+xml\">\n");
        }
        else{
            hStream.Printf("<p>Animation disabled</p>");
            hStream.Printf("<h2>Water height = %f</h2>", lastHeight);
        }
        hStream.Printf("</body></html>");        
    }
    hStream.WriteReplyHeader(True);
    return True;
}


OBJECTLOADREGISTER(WaterTank, "$Id$")
