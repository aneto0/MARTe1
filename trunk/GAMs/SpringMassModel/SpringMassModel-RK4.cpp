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
 * $Id: SpringMassModel.cpp 2013-07-26 10:18:00 avr $
 *
**/

#include "SpringMassModel.h"
#include "CDBExtended.h"
#include "File.h"

#include "DDBInputInterface.h"
#include "DDBOutputInterface.h"

bool SpringMassModel::Initialise(ConfigurationDataBase& cdbData) {

    CDBExtended cdb(cdbData);
    ////////////////////////////////////////////////////
    //                Add interfaces to DDB           //
    ////////////////////////////////////////////////////
    if(!AddInputInterface(input,"InputInterface")){
        AssertErrorCondition(InitialisationError,"SpringMassModel::Initialise: %s failed to add input interface",Name());
        return False;
    }

    if(!AddOutputInterface(output,"OutputInterface")){
        AssertErrorCondition(InitialisationError,"SpringMassModel::Initialise: %s failed to add output interface",Name());
        return False;
    }


    ////////////////////////////////////////////////////
    //                Add input signals               //
    ////////////////////////////////////////////////////
    if(!cdb->Move("InputSignals")){
        AssertErrorCondition(InitialisationError,"SpringMassModel::Initialise: %s did not specify InputSignals entry",Name());
        return False;
    }
    int32 nOfSignals = cdb->NumberOfChildren();
    if(nOfSignals != 2){
        AssertErrorCondition(InitialisationError, "SpringMassModel::Initialise: %s ObjectLoadSetup. SpringMassModel expects the current time and piston force as inputs ",Name());
        return False;
    }
    if(!input->ObjectLoadSetup(cdb,NULL)){
        AssertErrorCondition(InitialisationError,"SpringMassModel::Initialise: %s ObjectLoadSetup Failed DDBInterface %s ",Name(),input->InterfaceName());
        return False;
    }   
    cdb->MoveToFather();

    ////////////////////////////////////////////////////
    //                Add output signals              //
    ////////////////////////////////////////////////////
    if(!cdb->Move("OutputSignals")){
        AssertErrorCondition(InitialisationError,"SpringMassModel::Initialise: %s did not specify OutputSignals entry",Name());
        return False;
    }
    if(!output->ObjectLoadSetup(cdb,NULL)){
        AssertErrorCondition(InitialisationError,"SpringMassModel::Initialise: %s ObjectLoadSetup Failed DDBInterface %s ",Name(),output->InterfaceName());
        return False;
    }
    nOfSignals = cdb->NumberOfChildren();
    if(nOfSignals > 7){
        AssertErrorCondition(Warning,"SpringMassModel::Initialise: %s ObjectLoadSetup. Only the extension1, extension2, position1, position2, velocity1, velocity2 and piston force signals are going to be given as output of this GAM",Name());
    }
    cdb->MoveToFather();

    ////////////////////////////////////////////////////
    //            Configuration parameters            //
    ////////////////////////////////////////////////////
    {       
	if(!cdb.ReadFloat(initialExtension1, "initialExtension1", 0)){
            AssertErrorCondition(Information,"SpringMassModel %s::Initialise: initial extension of spring 1 not specified. Using default %f", Name(), lastExtension1);
        }

        if(!cdb.ReadFloat(initialExtension2, "initialExtension2", 0)){
            AssertErrorCondition(Information,"SpringMassModel %s::Initialise: initial extension of spring 2 not specified. Using default %f", Name(), lastExtension2);
        }

	if(!cdb.ReadFloat(initialVelocity1, "initialVelocity1", 0)){
            AssertErrorCondition(Information,"SpringMassModel %s::Initialise: initial extension velocity of spring 1 not specified. Using default %f", Name(), lastVelocity1);
        }

        if(!cdb.ReadFloat(initialVelocity2, "initialVelocity2", 0)){
            AssertErrorCondition(Information,"SpringMassModel %s::Initialise: initial extension velocity of spring 2 not specified. Using default %f", Name(), lastVelocity2);
        }
		
	if(!cdb.ReadFloat(mass1, "mass1", 2)){
            AssertErrorCondition(Information,"SpringMassModel %s::Initialise: mass of body 1 not specified. Using default %f", Name(), mass1);
        }
		
	if(!cdb.ReadFloat(mass2, "mass2", 2)){
            AssertErrorCondition(Information,"SpringMassModel %s::Initialise: mass of body 2 not specified. Using default %f", Name(), mass2);
        }
		
	if(!cdb.ReadFloat(spring1, "spring1", 5)){
            AssertErrorCondition(Information,"SpringMassModel %s::Initialise: stiffness of spring 1 not specified. Using default %f", Name(), spring1);
        }
		
	if(!cdb.ReadFloat(spring2, "spring2", 5)){
            AssertErrorCondition(Information,"SpringMassModel %s::Initialise: stiffness of spring 2 not specified. Using default %f", Name(), spring2);
        }
		
	if(!cdb.ReadFloat(damper1, "damper1", 3)){
            AssertErrorCondition(Information,"SpringMassModel %s::Initialise: damping coefficient of damper 1 not specified. Using default %f", Name(), damper1);
        }
		
	if(!cdb.ReadFloat(damper2, "damper2", 3)){
            AssertErrorCondition(Information,"SpringMassModel %s::Initialise: damping coefficient of damper 2 not specified. Using default %f", Name(), damper2);
        }

	if(!cdb.ReadFloat(spring1Length, "spring1Length", 2)){
            AssertErrorCondition(Information,"SpringMassModel %s::Initialise: rest length of spring 1 not specified. Using default %f", Name(), spring1Length);
        }

	if(!cdb.ReadFloat(spring2Length, "spring2Length", 2)){
            AssertErrorCondition(Information,"SpringMassModel %s::Initialise: rest length of spring 2 not specified. Using default %f", Name(), spring1Length);
        }

	if(!cdb.ReadFloat(mass1Width, "mass1Width", 1)){
            AssertErrorCondition(Information,"SpringMassModel %s::Initialise: width of mass 1 not specified. Using default %f", Name(), mass1Width);
        }

	if(!cdb.ReadFloat(mass2Width, "mass2Width", 1)){
            AssertErrorCondition(Information,"SpringMassModel %s::Initialise: width of mass 2 not specified. Using default %f", Name(), mass2Width);
        }

	if(!cdb.ReadFloat(restitution1, "restitution1", 1)){
            AssertErrorCondition(Information,"SpringMassModel %s::Initialise: Coefficient of restituion 1 not specified. Using default %f", Name(), restitution1);
        }

	if(!cdb.ReadFloat(restitution2, "restitution2", 1)){
            AssertErrorCondition(Information,"SpringMassModel %s::Initialise: Coefficient of restituion 2 not specified. Using default %f", Name(), restitution1);
        }

	int32 saturationEnabledInt;
	if(!cdb.ReadInt32(saturationEnabledInt, "saturationEnabled")){	
		AssertErrorCondition(Information,"SpringMassModel %s::Initialise: Force saturation on/off not specified. It will be disabled.", Name(), saturationEnabledInt);
	}
	else{
		saturationEnabled = (saturationEnabledInt != 0);
		// Only need to define maximum/minimum force if force saturation has been enabled 	
		if(saturationEnabled)
		{
			if(!cdb.ReadFloat(maxForce, "maxForce", 100)){
			    AssertErrorCondition(Information,"SpringMassModel %s::Initialise: maximum force not specified. Using default %f", Name(), maxForce);
			}

			if(!cdb.ReadFloat(minForce, "minForce", 0)){
			    AssertErrorCondition(Information,"SpringMassModel %s::Initialise: minimum force not specified. Using default %f", Name(), minForce);
			}
		}
	}

        FString svgLocation;
        if(!cdb.ReadFString(svgLocation, "SVGFileLocation")){
            AssertErrorCondition(Information,"SpringMassModel %s::Initialise: SVGLocation web location not set, switching off animation", Name());
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
                    AssertErrorCondition(Warning,"SpringMassModel %s::Initialise: Could not open the svg file: %s", Name(), svgLocation.Buffer());
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


bool SpringMassModel::Execute(GAM_FunctionNumbers functionNumber){
    // Get input and output data pointers
    input->Read();
    int32 usecTime          = *((int32*)input->Buffer());
    float force		    = ((float *)input->Buffer())[1];
    float *outputBuff = (float*)  output->Buffer();
    float extension1  = 0;
	float extension2  = 0;
	float velocity1   = 0;
	float velocity2   = 0;
	float acceleration1 = 0;
	float acceleration2 = 0;

    switch( functionNumber ){
        case GAMPrepulse:{
            // Reset last usec time, extensions, velocities
            lastUsecTime   = 0;
	    lastExtension1 = initialExtension1;
	    lastExtension2 = initialExtension2;
	    lastVelocity1  = initialVelocity1;
	    lastVelocity2  = initialVelocity2;
        }break;
        case GAMOnline:{
	    
            // Saturate force
            if(saturationEnabled){
                 if(force > maxForce) force = maxForce;
                 if(force < minForce) force = minForce;
            }
                  
            // Runge-Kutta order 4 method
            float dt = (usecTime - lastUsecTime) * 1e-6;
            float l, l1, l2, l3, l4,	// Extension1
                  p, p1, p2, p3, p4,	// Velocity1
                  q, q1, q2, q3, q4,	// Extension2
                  r, r1, r2, r3, r4;	// Velocity2

            l1 = lastVelocity1 * dt;
            p1 = ( - spring1*lastExtension1 - damper1*lastVelocity1 + spring2*lastExtension2 + damper2*lastVelocity2) / mass1 * dt;
            q1 = lastVelocity2 * dt;
            r1 = ( spring1 * lastExtension1 + damper1*lastVelocity1 - (mass1 + mass2)/mass2 * spring2 * lastExtension2 - (mass1 + mass2)/mass2 * damper2 * lastVelocity2) / mass1 * dt;

            l2 = (lastVelocity1 + p1/2) * dt;
            p2 = ( - spring1 * (lastExtension1 + l1/2) - damper1 * (lastVelocity1 + p1/2) + spring2*(lastExtension2 + q1/2) + damper2*(lastVelocity2 + r1/2)) / mass1 * dt;
            q2 = (lastVelocity2 + r1/2) * dt;
            r2 = ( spring1 * (lastExtension1 + l1/2) + damper1 * (lastVelocity1 + p1/2) - (mass1 + mass2)/mass2 * spring2 * (lastExtension2 + q1/2) - (mass1 + mass2)/mass2 * damper2 * (lastVelocity2 + r1/2)) / mass1 * dt;
			
            l3 = (lastVelocity1 + p2/2) * dt;
            p3 = ( - spring1 * (lastExtension1 + l2/2) - damper1 * (lastVelocity1 + p2/2) + spring2*(lastExtension2 + q2/2) + damper2*(lastVelocity2 + r2/2)) / mass1 * dt;
            q3 = (lastVelocity2 + r2/2) * dt;
            r3 = ( spring1 * (lastExtension1 + l2/2) + damper1 * (lastVelocity1 + p2/2) - (mass1 + mass2)/mass2 * spring2 * (lastExtension2 + q2/2) - (mass1 + mass2)/mass2 * damper2 * (lastVelocity2 + r2/2)) / mass1 * dt;
			
            l4 = (lastVelocity1 + p3) * dt;
            p4 = ( - spring1 * (lastExtension1 + l3) - damper1 * (lastVelocity1 + p3) + spring2*(lastExtension2 + q3) + damper2*(lastVelocity2 + r3)) / mass1 * dt;
            q4 = (lastVelocity2 + r3) * dt;
            r4 = ( spring1 * (lastExtension1 + l3) + damper1 * (lastVelocity1 + p3) - (mass1 + mass2)/mass2 * spring2 * (lastExtension2 + q3) - (mass1 + mass2)/mass2 * damper2 * (lastVelocity2 + r3)) / mass1 * dt;
			
            l = (1.0/6.0)*(l1 + l2 + l3 + l4);
            p =  (1.0/6.0)*(p1 + p2 + p3 + p4) + force / mass1 * dt;
            q = (1.0/6.0)*(q1 + q2 + q3 + q4);
            r =  (1.0/6.0)*(r1 + r2 + r3 + r4) - force / mass1 * dt;

            extension1 = lastExtension1 + l;
            velocity1  = lastVelocity1  + p;
            extension2 = lastExtension2 + q;
            velocity2  = lastVelocity2  + r;
 
	    // Collision detection between mass1 and mass2
            if ( -extension2 > spring2Length){
            extension2 = -spring2Length;
            velocity1 = (restitution2 * mass2 * velocity2 + mass1 * velocity1 + mass2 * (velocity1 + velocity2) ) / (mass1 + mass2);
            velocity2 = -restitution2 * velocity2;         
            }

            // Collision detection between mass1 and wall
            if ( -extension1 > spring1Length){
            extension1 = -spring1Length; 
            velocity2 = velocity2 + 2*velocity1;	// Conserves momentum of the second mass
            velocity1 = -restitution1 * velocity1;
            }

            lastExtension1 = extension1;
            lastExtension2 = extension2;
            lastVelocity1  = velocity1;
            lastVelocity2  = velocity2;
            lastUsecTime   = usecTime;
	    lastForce	   = force;

            float position1 = spring1Length + extension1;
            float position2 = spring1Length + spring2Length + extension1 + extension2;

	    // Need to ensure that these match up with the outputs listed in the cfg file
            *outputBuff     = extension1;
	    *(outputBuff+1) = extension2;
            *(outputBuff+2) = position1;
            *(outputBuff+3) = position2;
            *(outputBuff+4) = velocity1;
            *(outputBuff+5) = velocity2;
            *(outputBuff+6) = force;
        }break;
    }

    // Update the data output buffer
    output->Write();

    return True;
}	

bool SpringMassModel::ProcessHttpMessage(HttpStream &hStream) {
    if(hStream.Switch("InputCommands.svgFile")){
        hStream.SSPrintf("OutputHttpOtions.Content-Type","image/svg+xml");
        hStream.Switch((uint32)0);
        hStream.Printf("%s", svg.Buffer());
    }else if(hStream.Switch("InputCommands.Extensions")) {
        hStream.SSPrintf("OutputHttpOtions.Content-Type","text/html");
        hStream.Switch((uint32)0);
        hStream.Printf("%f %f %f\n", lastExtension1, lastExtension2, lastForce);
    }else if(hStream.Switch("InputCommands.Parameters")) {
        hStream.SSPrintf("OutputHttpOtions.Content-Type","text/html");
        hStream.Switch((uint32)0);
        hStream.Printf("%f %f %f %f %f %f\n", mass1Width, mass2Width, spring1Length, spring2Length, spring1, spring2);
    }
    else{
        hStream.SSPrintf("OutputHttpOtions.Content-Type","text/html");
        hStream.Printf("<html><head><title>springMassModel</title></head><body>\n");
        if(animationEnabled){
            hStream.Printf("<object id=\"springMassModelID\" data=\"?svgFile\" width=\"100%%\" height=\"100%%\" type=\"image/svg+xml\">\n");
        }
        else{
            hStream.Printf("<p>Animation disabled</p>");
            hStream.Printf("<h2>extension2 = %f</h2>", lastExtension2);
        }
        hStream.Printf("</body></html>");        
    }
    hStream.WriteReplyHeader(True);
    return True;
}

OBJECTLOADREGISTER(SpringMassModel, "$Id: SpringMassModel.cpp 2013-07-26 10:18:00 avr $")
