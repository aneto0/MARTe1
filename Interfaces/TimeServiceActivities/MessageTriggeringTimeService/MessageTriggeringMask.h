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

#ifndef MESSAGE_TRIGGERING_MASK
#define MESSAGE_TRIGGERING_MASK

#include "GCReferenceContainer.h"

//The various operations that can be performed between the input and the mask
enum OperType{
    AND,
    OR,
    XOR,
    NAND,
    NOR,
    NXOR,
    XORandAND
};

//The mask can either be applied against the requested input, or only to the bits
//which changed between two iterations. If CHANGED_BITS_0_1 or CHANGED_BITS_1_0 is used
//it will return False if the bits changed in the wrong direction (all bits must change in
//the same direction for the same instant)
enum TestType{
    ORIGINAL_INPUT,
    CHANGED_BITS_ANY,
    CHANGED_BITS_0_1,
    CHANGED_BITS_1_0
};

OBJECT_DLL(MessageTriggeringMask)
class MessageTriggeringMask : public GCReferenceContainer{
OBJECT_DLL_STUFF(MessageTriggeringMask)
public:
    /**
     * The bit mask, read as hex
     */
    int64 mask;

    /**
     * The latest value of the tested value
     */
    int64 oldTestValue;

    /**
     * The driver channel number where data is going to be read from
     */
    int32 channelNumber;

    /**
     * If latched, it only fires the messages if the new test value
     * differs from the new test value
     */
    bool latched;

    /**
     * The desired result for the mask operation
     */
    int64 result;

    /**
     * Operation type to be tested
     */
    OperType operation;

    /**
     * The type of input
     */
    TestType testType;
    /**
     * Tests a value against the provided mask and if required latches
     * @param testValue the value to test against the mask
     * @return True if the mask is successfully applied. If the object is latched it only returns
     * True the first time
     */
    bool AssertMask(int64 testValue){
        if(latched && (oldTestValue == testValue)){
            return False;
        }
        int64 test = 0;
        if(testType == CHANGED_BITS_ANY){
            //Check what bits changed
            test = oldTestValue ^ testValue;
        }
        else if(testType == CHANGED_BITS_0_1){
            //Check what bits changed and keep only those from 0->1
            test = (oldTestValue ^ testValue) & testValue;
        }
        else if(testType == CHANGED_BITS_1_0){
            //Check what bits changed and keep only those from 1->0
            test = (oldTestValue ^ testValue) & oldTestValue;
        }
        else{
            test = testValue;
        }
        oldTestValue = testValue;
        if(operation == AND){
            return (test & mask) == result;
        }
        else if(operation == OR){
            return (test | mask) == result;
        }
        else if(operation == XOR){
            return (test ^ mask) == result;
        }
        else if(operation == NAND){
            return !(test | mask) == result;
        }
        else if(operation == NOR){
            return !(test & mask) == result;
        }
        else if(operation == NXOR){
            return !(test ^ mask) == result;
        }
        return False; 
     }   

    MessageTriggeringMask(){
        mask          = 0;
        oldTestValue  = 0;
        testType      = ORIGINAL_INPUT;
        channelNumber = 0;
        result        = 0;
        latched       = True;
        operation     = AND;
    }

    virtual ~MessageTriggeringMask(){
    }


    bool ObjectLoadSetup(ConfigurationDataBase &info,StreamInterface *err);
};

#endif

