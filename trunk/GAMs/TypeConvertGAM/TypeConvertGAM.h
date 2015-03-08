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
 * $Id: WebStatisticGAM.cpp 3 2012-01-15 16:26:07Z aneto $
 *
**/
#if !defined (TYPE_CONVERT_GAM_H)
#define TYPE_CONVERT_GAM_H

#include "GAM.h"
#include "DDBInputInterface.h"
#include "DDBOutputInterface.h"
#include "TypeConvertGAM.h"

enum ConvertType{
    Unknown,
    Int32ToInt32,
    Int32ToUint32,
    Int32ToFloat,
    FloatToInt32,
    Int32ToInt64,
    Int64ToInt32,
    FloatToInt64,
    Int64ToFloat,
    Int32ToDouble,
    DoubleToInt32,
    Int64ToDouble,
    DoubleToInt64,
    DoubleToFloat,
    FloatToDouble
};

OBJECT_DLL(TypeConvertGAM)
class TypeConvertGAM : public GAM {
OBJECT_DLL_STUFF(TypeConvertGAM)

private:
    /** Input interface to read data from */
    DDBInputInterface  *input;
    /** Output interface to write data to */
    DDBOutputInterface *output;
    /** Stores all the types to convert*/
    ConvertType        *convertType;
    /** Number of signals to be converted*/
    int32               numberOfSignals;

    /**Translates a convert type from the cdb to an enum type*/
    ConvertType TranslateConvertType(FString &input, FString &output){
        if(input == "int" || input == "int32"){
            if(output == "int" || output == "int32") return Int32ToInt32;
	    if(output == "uint32")                   return Int32ToUint32;
            if(output == "int64")                    return Int32ToInt64;
            if(output == "float")                    return Int32ToFloat;
            if(output == "double")                   return Int32ToDouble;
        }
        else if(input == "int64"){
            if(output == "int" || output == "int32") return Int64ToInt32;
            if(output == "float")                    return Int64ToFloat;
            if(output == "double")                   return Int64ToDouble;
        }
        else if(input == "float"){
            if(output == "int" || output == "int32") return FloatToInt32;
            if(output == "int64")                    return FloatToInt64;
            if(output == "double")                   return FloatToDouble;
        }       
        else if(input == "double"){
            if(output == "int" || output == "int32") return DoubleToInt32;
            if(output == "int64")                    return DoubleToInt64;
            if(output == "float")                    return DoubleToFloat;
        }
        return Unknown;
    }

public:

    TypeConvertGAM(){
        input           = NULL;
        output          = NULL;
        convertType     = NULL;
        numberOfSignals = 0;
    }

    virtual ~TypeConvertGAM(){
        if(convertType != NULL){
            delete[] convertType;
        }
    }

    /// GAM configuration
    bool  Initialise(ConfigurationDataBase &cdb);

    /// GAM execution method
    bool  Execute(GAM_FunctionNumbers execFlag);
};
#endif
