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

#if !defined (_GENACQMODULE)
#define _GENACQMODULE

#include "GCNamedObject.h"
#include "TimeTriggeringServiceInterface.h"
#include "HttpInterface.h"

class GenericAcqModule;

/** */
const int MAX_NUMBER_OF_TRIGGERING_SERVICE  =  4;

extern "C" {

    /** */
    bool GQMObjectLoadSetup(GenericAcqModule &gqm,ConfigurationDataBase &info,StreamInterface *err);

    /** */
    bool GQMObjectSaveSetup(GenericAcqModule &gqm,ConfigurationDataBase &info,StreamInterface *err);

}

OBJECT_DLL(GenericAcqModule)

/** Abstract driver for a generic acquisition board. */
class GenericAcqModule: public GCReferenceContainer, public HttpInterface{

    OBJECT_DLL_STUFF(GenericAcqModule)

    friend bool GQMObjectLoadSetup(GenericAcqModule &gqm,ConfigurationDataBase &info,StreamInterface *err);
    friend bool GQMObjectSaveSetup(GenericAcqModule &gqm,ConfigurationDataBase &info,StreamInterface *err);

protected:

    ////////////////////////
    // Used by GAMs Flags //
    ////////////////////////

    /** True if one GAM is using this module as input.
        Only one GAM can use a module as input or output
    */
    bool inputBoardInUse;

    /** True if one GAM is using this module as output.
        Only one GAM can use a module as input or output
    */
    bool outputBoardInUse;

    /////////////////////////////
    // Input Modules Paramters //
    /////////////////////////////

    /** Number of input channels in the module
    It's equal to the size of the data buffer */
    int32 numberOfInputChannels;

    //////////////////////////////
    // Output Modules Paramters //
    //////////////////////////////

    /** Number of input channels in the module
    It's equal to the size of the data buffer */
    int32 numberOfOutputChannels;

    //////////////////////////////
    // Time Modules Paramters   //
    //////////////////////////////

    class TriggerTimeService{
    private:

        /** Reference to the Time Triggering Service Object */
        TimeTriggeringServiceInterface                 *trigger;

    public:

        /** constructor */
        TriggerTimeService(){
            Disable();
        }

        /** Distructor */
        ~TriggerTimeService(){
            Disable();
        }

        bool Enable(TimeTriggeringServiceInterface *trigContext){
            if(trigContext == NULL) return False; 
            trigger                = trigContext;
            return True;
        }
        
        bool Disable(){
            trigger = NULL;
            return True;
        }
        
        void Trigger(){
            if(trigger != NULL) trigger->Trigger();
        }
    };

    /** Triggering services associated with the time module */
    TriggerTimeService           triggerService[MAX_NUMBER_OF_TRIGGERING_SERVICE];

    /** Number of enabled triggering services */
    int32                        nOfTriggeringServices;

public:

    /////////////////////////////////////////////////////////////////////////////////
    //                            Modules CommonMethods                            //
    /////////////////////////////////////////////////////////////////////////////////

    /** Constructor */
    GenericAcqModule(){
        inputBoardInUse        = False;
        outputBoardInUse       = False;

        // For the InputModule
        numberOfInputChannels  = -1;

        // For the OutputModule
        numberOfOutputChannels = -1;

        // Number of Services associated with time module
        nOfTriggeringServices = 0;
    }

    /** Distructor */
    virtual ~GenericAcqModule(){}

    /** Load parameter */
    virtual bool ObjectLoadSetup(ConfigurationDataBase &info,StreamInterface *err){
        if(!GCReferenceContainer::ObjectLoadSetup(info, err)){
            AssertErrorCondition(FatalError, "%s::ObjectLoadSetup:Failed initialising GCReferenceContainer", Name());
            return False;
        }
        return GQMObjectLoadSetup(*this, info, err);
    }

//    /** Save parameter */
//    virtual bool ObjectSaveSetup(ConfigurationDataBase &info,StreamInterface *err);

    /** Save parameter */
    virtual bool ObjectDescription(StreamInterface &s,bool full=False,StreamInterface *err=NULL) = 0;

    /** Returns if the module is Synchronising */
    virtual bool IsSynchronizing(){
        return False;
    }

    /** Set the input board in use
        Each module should use these method appropriately.
        For instance ATM Module can only be used either as input module
        or output module. In this case both flags must be set by the Set*BoardInUse
        methods. For modules that can perform both input and output activities
        each flag should be set accordingly.
        @return True if the flag has been successfully set. False if the
        flag was already set.
    */
    virtual bool SetInputBoardInUse(bool on = True) = 0;

    /** Sets the output board in use.
        Each module should use these method appropriately.
        For instance ATM Module can only be used either as input module
        or output module. In this case both flags must be set by the Set*BoardInUse
        methods. For modules that can perform both input and output activities
        each flag should be set accordingly.
        @return True if the flag has been successfully set. False if the
        flag was already set.
     */
    virtual bool SetOutputBoardInUse(bool on = True) = 0;

    /////////////////////////////////////////////////////////////////////////////////
    //                             Input Modules Methods                           //
    /////////////////////////////////////////////////////////////////////////////////

    /** Returns the number of Inputs to be read */
    int32 NumberOfInputs()const{
        return numberOfInputChannels;
    }

    ////////////////////////
    // Overloaded Methods //
    ////////////////////////

    /** Copy local input buffer in destination buffer:
        return  0 if data not ready
        return <0 if error
        return >0 if OK
    */
    virtual int32 GetData(uint32 usecTime, int32 *buffer, int32 bufferNumber = 0) = 0;
    
    /** Enable Board Acquisition */
    virtual  bool EnableAcquisition(){
        return False;
    }

    /** To be used by timming modules which support polling*/
    virtual bool Poll(){
        return False;
    }

    /** Enable Board Acquisition */
    virtual  bool DisableAcquisition(){
        return False;
    }

    /** Start input acquisition - only used with boards need to be started
        otherwise is a dummy method */
    virtual bool StartSingleAcquisition(){
        return True;
    }

    /////////////////////////////////////////////////////////////////////////////////
    //                            Output Modules Methods                           //
    /////////////////////////////////////////////////////////////////////////////////

    /** Update the output of the module using n = numberOfOutputChannels data word
        of the source buffer starting from absoluteOutputPosition */
    virtual bool WriteData(uint32 usecTime, const int32 *buffer) = 0;

    /** Returns the number of Outputs to be written */
    int32 NumberOfOutputs()const{
        return numberOfOutputChannels;
    }

    /////////////////////////////////////////////////////////////////////////////////
    //                             Time Modules Methods                            //
    /////////////////////////////////////////////////////////////////////////////////

    /** does whatever is needed to begin providing to the ExternalTimeTriggeringService */
    bool EnableTimeService(TimeTriggeringServiceInterface *ts)
    {
        if (nOfTriggeringServices >= MAX_NUMBER_OF_TRIGGERING_SERVICE){
            AssertErrorCondition(InitialisationError,"Module %s: no more serices can be attached to this module",Name());
            return False;
        }

        if (!triggerService[nOfTriggeringServices].Enable(ts)){
            AssertErrorCondition(InitialisationError,"Module %s: Failed anabling Time Service",Name());
            return False;
        }

        nOfTriggeringServices++;
        return True;
    }

    /** does whatever is needed to stop providing to the ExternalTimeTriggeringService */
    bool DisableTimeService(){

        nOfTriggeringServices = 0;
        for(int i = 0; i < MAX_NUMBER_OF_TRIGGERING_SERVICE; i++){
            triggerService[i].Disable();
        }

        return True;
    }

    /** If the board is a Time Module it has the triggering service enabled */
    bool IsTimeModule() const{
        return (nOfTriggeringServices > 0);
    }

    ////////////////////////
    // Overloaded Methods //
    ////////////////////////

    /** Get CTS data word */
    virtual uint16 GetCTS(){
        return 0xFFFF;
    }

    /** Get RTTN data word */
    virtual uint32 GetRTTN(){
        return 0xFFFFFFFF;
    }

    /** Return the actual Time as microseconds */
    virtual int64 GetUsecTime(){
        return -1;
    }

    //////////////////////////////////
    // Simulation Purpose Functions //
    //////////////////////////////////

    virtual bool PulseStart(){return True;}
    
    ///////////////////////////////
    // Multiple Buffer Functions //
    ///////////////////////////////
    
    /** Returns the number of buffers used for acquisition 
     *  Only used by boards with circular buffers and multiple
     *  acquisitions methods. */
    virtual int32  NumberOfBuffers(){return 0;}

    /**
     * Outputs an html page with information about the acquisition driver
     */
    virtual bool ProcessHttpMessage(HttpStream &hStream){
       hStream.WriteReplyHeader(True);
       return True; 
    }
    
};

#endif
