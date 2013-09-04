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
 * $Id: SpringMassModel.h 2013-07-26 10:18:00 avr $
 *
**/

#ifndef SPRINGMASSMODEL_H_
#define SPRINGMASSMODEL_H_

#include "GAM.h"
#include "HttpInterface.h"

/**
 * A GAM  implementing the model for a water tank. The water pump enables
 * water to flow in proportionally to an input voltage and to the tank aperture. Water flows out
 * at a rate proportional to the output aperture and to the water height. The system receives as an input
 * the voltage and outputs an height accordingly to the following equation:
 * dh/dt = (bV - a x sqrt(h)) / A
 * where:
 * b - constant related to flow rate into the tank (cm^3/(V.s))
 * a - constant related to flow rate out of the tank (cm^2.5))
 * h - water height (cm)
 * A - tank area (cm^2)
 */
OBJECT_DLL(SpringMassModel)
class SpringMassModel : public GAM, public HttpInterface {
OBJECT_DLL_STUFF(SpringMassModel)

// DDB Interfaces
private:
    /** Input interface to read data from */
    DDBInputInterface                      *input;
    /** Output interface to write data to */
    DDBOutputInterface                     *output;
    
// Parameters
private:
    /** Last usec time (for the integral) */
    int32                                   lastUsecTime;
    /** Last extension of spring 1 (for the integral) */
    float                                   lastExtension1;
    /** Last extension of spring 2 (for the integral) */
    float                                   lastExtension2;
    /** Last extension velocity of spring 1 (for the integral) */
    float                                   lastVelocity1;
    /** Last extension velocity of spring 2 (for the integral) */
    float                                   lastVelocity2;
    /** Initial extension of the first spring */
    float                                   initialExtension1;
    /** Initial extension of the second spring */
    float                                   initialExtension2;
    /** Initial velocity of the first spring */
    float                                   initialVelocity1;
    /** Initial velociyu of the second spring */
    float                                   initialVelocity2;
    /** Last force value after saturation*/
    float                                   lastForce;
    /** The mass of the first body */
    float                                   mass1;
    /** The mass of the second body*/
    float                                   mass2;
    /** The stiffness of the first spring */
    float                                   spring1;
    /** The stiffness of the second spring*/
    float                                   spring2;
    /** The damping coefficient of the first damper */
    float                                   damper1;
    /** The damping coefficient of the second damper */
    float                                   damper2;
    /** Maximum force that can be requested */
    float                                   maxForce;
    /** Minimum force that can be requested */
    float                                   minForce;
    /** The width of the first body */
    float                                   mass1Width;
    /** The width of the second body */
    float                                   mass2Width;
    /** The rest length of the first spring */
    float                                   spring1Length;
    /** The rest length of the second spring */
    float                                   spring2Length;
    /** Coefficient of restitution for collisions between mass1 and the wall **/
    float                                   restitution1;
    /** Coefficient of restitution for collisions between mass1 and mass2 **/
    float                                   restitution2;
    /** The contents of the SVG file */
    FString                                 svg;
    /** Enable animation?*/
    bool                                    animationEnabled;
    /** Enable force saturation?*/
    bool                                    saturationEnabled;

public:
    /** Constructor */
    SpringMassModel(){
        input             = NULL;
        output            = NULL;
        lastExtension1	  = 0;
        lastExtension2	  = 0;
        lastVelocity1	  = 0;
        lastVelocity2	  = 0;
        lastUsecTime  	  = 0;
        lastForce	  = 0;
        mass1     	  = 0;
        mass2		  = 0;
        spring1		  = 0;
        spring2		  = 0;
        damper1		  = 0;
        damper2		  = 0;
	initialExtension1 = 0;
        initialExtension2 = 0;
	initialVelocity1  = 0;
	initialVelocity2  = 0;
	mass1Width        = 0;
	mass2Width        = 0;
	spring1Length     = 0;
        spring2Length     = 0;
	restitution1      = 0;
	restitution2      = 0;
    }

    /** Destructor */
    ~SpringMassModel(){
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


#endif /* SPRINGMASSMODEL_H_ */

