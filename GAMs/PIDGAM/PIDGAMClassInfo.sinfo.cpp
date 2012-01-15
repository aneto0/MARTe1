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
#define protected public
#define private public
#include "PIDGAMClassInfo.h"
#include "ObjectRegistryItem.h"
#include "ClassStructure.h"
#include "ObjectMacros.h"
static ClassStructureEntry PIDGAMOutputStructure_controlSignal_CSE_EL("float","",0,0,0,0,0 ,"controlSignal",msizeof(PIDGAMOutputStructure,controlSignal),indexof(PIDGAMOutputStructure,controlSignal));
static ClassStructureEntry PIDGAMOutputStructure_feedback_CSE_EL("float","",0,0,0,0,0 ,"feedback",msizeof(PIDGAMOutputStructure,feedback),indexof(PIDGAMOutputStructure,feedback));
static ClassStructureEntry PIDGAMOutputStructure_error_CSE_EL("float","",0,0,0,0,0 ,"error",msizeof(PIDGAMOutputStructure,error),indexof(PIDGAMOutputStructure,error));
static ClassStructureEntry PIDGAMOutputStructure_integratorState_CSE_EL("float","",0,0,0,0,0 ,"integratorState",msizeof(PIDGAMOutputStructure,integratorState),indexof(PIDGAMOutputStructure,integratorState));
static ClassStructureEntry PIDGAMOutputStructure_fastDischargeAction_CSE_EL("float","",0,0,0,0,0 ,"fastDischargeAction",msizeof(PIDGAMOutputStructure,fastDischargeAction),indexof(PIDGAMOutputStructure,fastDischargeAction));
static ClassStructureEntry PIDGAMOutputStructure_antiwindupAction_CSE_EL("float","",0,0,0,0,0 ,"antiwindupAction",msizeof(PIDGAMOutputStructure,antiwindupAction),indexof(PIDGAMOutputStructure,antiwindupAction));
static ClassStructureEntry * PIDGAMOutputStructure__CSE__[] = {
    &PIDGAMOutputStructure_controlSignal_CSE_EL,
    &PIDGAMOutputStructure_feedback_CSE_EL,
    &PIDGAMOutputStructure_error_CSE_EL,
    &PIDGAMOutputStructure_integratorState_CSE_EL,
    &PIDGAMOutputStructure_fastDischargeAction_CSE_EL,
    &PIDGAMOutputStructure_antiwindupAction_CSE_EL,
    NULL
};
ClassStructure PIDGAMOutputStructure__CS__("PIDGAMOutputStructure",sizeof(PIDGAMOutputStructure),0 ,PIDGAMOutputStructure__CSE__);
STRUCTREGISTER("PIDGAMOutputStructure",PIDGAMOutputStructure__CS__)
static ClassStructureEntry PIDGAMInputStructure_usecTime_CSE_EL("unsigned int","",0,0,0,0,0 ,"usecTime",msizeof(PIDGAMInputStructure,usecTime),indexof(PIDGAMInputStructure,usecTime));
static ClassStructureEntry PIDGAMInputStructure_reference_CSE_EL("float","",0,0,0,0,0 ,"reference",msizeof(PIDGAMInputStructure,reference),indexof(PIDGAMInputStructure,reference));
static ClassStructureEntry PIDGAMInputStructure_measurement_CSE_EL("float","",0,0,0,0,0 ,"measurement",msizeof(PIDGAMInputStructure,measurement),indexof(PIDGAMInputStructure,measurement));
static ClassStructureEntry PIDGAMInputStructure_feedforward_CSE_EL("float","",0,0,0,0,0 ,"feedforward",msizeof(PIDGAMInputStructure,feedforward),indexof(PIDGAMInputStructure,feedforward));
static ClassStructureEntry * PIDGAMInputStructure__CSE__[] = {
    &PIDGAMInputStructure_usecTime_CSE_EL,
    &PIDGAMInputStructure_reference_CSE_EL,
    &PIDGAMInputStructure_measurement_CSE_EL,
    &PIDGAMInputStructure_feedforward_CSE_EL,
    NULL
};
ClassStructure PIDGAMInputStructure__CS__("PIDGAMInputStructure",sizeof(PIDGAMInputStructure),0 ,PIDGAMInputStructure__CSE__);
STRUCTREGISTER("PIDGAMInputStructure",PIDGAMInputStructure__CS__)
static ClassStructureEntry PIDGAMClassInfo_input_CSE_EL("PIDGAMInputStructure","",0,0,0,0,0 ,"input",msizeof(PIDGAMClassInfo,input),indexof(PIDGAMClassInfo,input));
static ClassStructureEntry PIDGAMClassInfo_output_CSE_EL("PIDGAMOutputStructure","",0,0,0,0,0 ,"output",msizeof(PIDGAMClassInfo,output),indexof(PIDGAMClassInfo,output));
static ClassStructureEntry * PIDGAMClassInfo__CSE__[] = {
    &PIDGAMClassInfo_input_CSE_EL,
    &PIDGAMClassInfo_output_CSE_EL,
    NULL
};
ClassStructure PIDGAMClassInfo__CS__("PIDGAMClassInfo",sizeof(PIDGAMClassInfo),0 ,PIDGAMClassInfo__CSE__);
STRUCTREGISTER("PIDGAMClassInfo",PIDGAMClassInfo__CS__)
ClassStructure * PIDGAMClassInfo_sinfo[] = {
    &PIDGAMOutputStructure__CS__,
    &PIDGAMInputStructure__CS__,
    &PIDGAMClassInfo__CS__,
    NULL
};
