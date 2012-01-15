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

#include "Filter.h"
#include "System.h"
#include "CDBExtended.h"

static const double pi = 3.1415926535897;

OBJECTREGISTER(Filter,"$Id$")

bool FilterResize(Filter &f,int inputSize,int outputSize){
    return f.Resize(inputSize,outputSize);
}

///
bool Filter::Resize(int inputSize,int outputSize){
    if (inputStatus        != NULL)  free((void *&)inputStatus       );
    if (outputStatus       != NULL)  free((void *&)outputStatus      );
    if (inputCoefficients  != NULL)  free((void *&)inputCoefficients );
    if (outputCoefficients != NULL)  free((void *&)outputCoefficients);

    this->inputSize  = inputSize;
    this->outputSize = outputSize;

    if((inputSize == 0) || (outputSize == 0))
    return True;

    inputStatus         = (float *) malloc(sizeof(float) * inputSize);
    outputStatus        = (float *) malloc(sizeof(float) * outputSize);
    inputCoefficients   = (double *) malloc(sizeof(double) * inputSize);
    outputCoefficients  = (double *) malloc(sizeof(double) * outputSize);

    if ((inputStatus        == NULL) ||
        (outputStatus       == NULL) ||
        (inputCoefficients  == NULL) ||
        (outputCoefficients == NULL)){
        AssertErrorCondition(FatalError,"Resize(%i,%i) failed malloc",inputSize,outputSize);
        return False;
    }

    return True;
}

bool FilterInit(Filter &f,ConfigurationDataBase &cdb){
    return f.InitP(cdb);
}

///
bool Filter::InitP(ConfigurationDataBase &cdb){

    int maxDim = 1;
    int dimensions[1]={0};
    CDBExtended localCdb(cdb);
    int32 bcnos;

    if (localCdb->Move("Numerator")){

        maxDim = 1;
        if (!localCdb->GetArrayDims(dimensions,maxDim,"",CDBAIM_Strict)){
            AssertErrorCondition(ParametersError,"failed getting size of Numerator");
            return False;
        }
        int inputSize = dimensions[0];

        localCdb->MoveToFather();

        if (!localCdb->Move("Denominator") ){
            AssertErrorCondition(ParametersError,"Denominator not defined");
            return False;
        }

        maxDim = 1;
        if (!localCdb->GetArrayDims(dimensions,maxDim,"",CDBAIM_Strict)){
            AssertErrorCondition(ParametersError,"failed getting size of Denominator");
            return False;
        }
        int outputSize = dimensions[0];

        localCdb->MoveToFather();

        AssertErrorCondition(Information,"Creating Filter of size %i %i",inputSize,outputSize);
        if (!Resize(inputSize,outputSize)) return False;

        maxDim = 1;
        dimensions[0] = inputSize;
        if (!localCdb.ReadDoubleArray(inputCoefficients,dimensions,maxDim,"Numerator")) {
            AssertErrorCondition(ParametersError,"Error reading the Numerator coefficients");
            return False;
        }

        maxDim = 1;
        dimensions[0] = outputSize;
        if (!localCdb.ReadDoubleArray(outputCoefficients,dimensions,maxDim,"Denominator")) {
            AssertErrorCondition(ParametersError,"Error reading the Denominator coefficients");
            return False;
        }

        float b0 = outputCoefficients[0];
        if (b0 == 0.0){
            AssertErrorCondition(ParametersError,"This filter is anti-causal!!! cannot be implemented");
            Resize(0,0);
            return False;
        }

        int i;
        for (i = 0;i < inputSize ; i++){
            inputCoefficients[i] /= b0;
        }

        for (i = 0;i < outputSize ; i++){
            outputCoefficients[i] /= -b0;
        }

    } else
    if (localCdb->Move("0")){
        maxDim = 1;
        if (!localCdb->GetArrayDims(dimensions,maxDim,"",CDBAIM_Strict)){
            AssertErrorCondition(ParametersError,"failed getting size of 0 (numerator)");
            return False;
        }
        int inputSize = dimensions[0];

        localCdb->MoveToFather();

        if (!localCdb->Move("1")){
            AssertErrorCondition(ParametersError,"1 (denominator) not defined");
            return False;
        }
        maxDim = 1;
        if (!localCdb->GetArrayDims(dimensions,maxDim,"",CDBAIM_Strict)){
            AssertErrorCondition(ParametersError,"failed getting size of 1 (denominator)");
            return False;
        }
        // in this case the filter data omits the b0 coeffs assuming 1
        // we need to reintroduce it
        int outputSize = dimensions[0] + 1;

        localCdb->MoveToFather();

        AssertErrorCondition(Information,"Created Filter of size %i %i",inputSize,outputSize);
        if (!Resize(inputSize,outputSize)) return False;

        dimensions[0] = inputSize;
        maxDim = 1;
        if (!localCdb.ReadDoubleArray(inputCoefficients,dimensions,maxDim,"0")) {
            AssertErrorCondition(ParametersError,"Error reading the Numerator Coefficients");
            return False;
        }
        dimensions[0] = outputSize-1;
        maxDim = 1;
        if (!localCdb.ReadDoubleArray(outputCoefficients+1,dimensions,maxDim,"1")) {
            AssertErrorCondition(ParametersError,"Error reading the Denominator Coefficients (negated)");
            return False;
        }
        outputCoefficients[0] = 1;
    } else
      if (localCdb->Exists("Poles") || localCdb->Exists("Zeros") ){
        maxDim = 1;
        int numberOfPoles = 0;
        if (localCdb->GetArrayDims(dimensions,maxDim,"Poles",CDBAIM_Strict)){
            numberOfPoles = dimensions[0];
        }

        maxDim = 1;
        int numberOfZeros = 0;
        if (localCdb->GetArrayDims(dimensions,maxDim,"Zeros",CDBAIM_Strict)){
            numberOfZeros = dimensions[0];
        }

        float samplingTime;
        if (!localCdb.ReadFloat(samplingTime,"SamplingTime")){
            AssertErrorCondition(ParametersError,"SamplingTimeS is needed when Poles/Zeros are used");
            return False;
        }

        double gain = 1.0;
        if (!localCdb.ReadDouble(gain,"Gain",1.0)){
            AssertErrorCondition(Information,"Gain is set to 1.0");
        }


        int maxPolZero = numberOfPoles;
        if (numberOfZeros > numberOfPoles) maxPolZero = numberOfZeros;

        if (maxPolZero == 0){
            AssertErrorCondition(ParametersError,"no Poles nor Zeros?");
            return False;
        }

        double *poles = (double *)malloc(sizeof(double)*maxPolZero);
        double *zeros = (double *)malloc(sizeof(double)*maxPolZero);

        if ((poles == NULL)||(zeros == NULL)){
            if (poles) free((void *&)poles);
            if (zeros) free((void *&)zeros);
            AssertErrorCondition(FatalError,"memory allocation failure for poles/zeros");
        }

        if (numberOfPoles > 0){
            dimensions[0] = numberOfPoles;
            maxDim = 1;
            if (!localCdb.ReadDoubleArray(poles,dimensions,maxDim,"Poles")) {
                AssertErrorCondition(ParametersError,"Error reading the Poles");
                return False;
            }
        }

        if (numberOfZeros > 0){
            dimensions[0] = numberOfZeros;
            maxDim = 1;
            if (!localCdb.ReadDoubleArray(zeros,dimensions,maxDim,"Zeros")) {
                AssertErrorCondition(ParametersError,"Error reading the Zeros");
                return False;
            }
        }

        //  (s+a)/a = (aT+2)/aT * (1+Z^-1*(aT-2)/(aT+2))/(1+Z^-1)
        //

        int i = 0;
        for (i = 0;i < numberOfPoles; i++) {
            float poleT = poles[i] * samplingTime;
            if (poleT != 0){
                poles[i] = (poleT-2)/(poleT+2);
                gain = gain * (poleT/(poleT+2));
            } else {
                poles[i] = -1;
                gain = gain * (samplingTime/2);
            }
        }
        for (;i < maxPolZero; i++) {
                poles[i] = 1;
        }
        for (i = 0;i < numberOfZeros; i++) {
            float zeroT = zeros[i] * samplingTime;
            if (zeroT != 0){
                zeros[i] = (zeroT-2)/(zeroT+2);
                gain = gain * ((zeroT+2)/zeroT);
            } else{
                zeros[i] = -1;
                gain = gain * (2/samplingTime);
            }
        }

        for (;i < maxPolZero; i++) {
            zeros[i] = 1;
        }

        AssertErrorCondition(Information,"Created Filter of size %i %i",maxPolZero+1,maxPolZero+1);
        if (!Resize(maxPolZero + 1,maxPolZero + 1)) return False;


        inputCoefficients[0]  = gain;
        outputCoefficients[0] = 1.0;
        for (i=1;i < (maxPolZero + 1); i++) {
            inputCoefficients[i] = 0.0;
            outputCoefficients[i] = 0.0;
        }

        for (i=0;i < maxPolZero ; i++) {
            int j;
            for (j = (maxPolZero-1); j >= 0; j--) {
                inputCoefficients [j+1] += inputCoefficients [j] * zeros[i];
                outputCoefficients[j+1] += outputCoefficients[j] * poles[i];
            }
        }

        // convert denominator into usable form
        for (i = 0;i < outputSize ; i++){
            outputCoefficients[i] *= -1;
        }

        if (poles) free((void *&)poles);
        if (zeros) free((void *&)zeros);

    } else
      if (localCdb.ReadInt32(bcnos, "BoxCarNumberOfSamples")){
	  if(bcnos < 0) {
	      AssertErrorCondition(ParametersError, "Filter requires BoxCarNumberOfSamples > 0");
	      return False;
	  } else {
	      CDBExtended cdbLocal(cdb);
	      cdbLocal->AddChildAndMove("NumAndDenArea");
	      double *coeffValues;
	      coeffValues = (double *)malloc(sizeof(double)*bcnos);
	      for(int i = 0 ; i < bcnos ; i++)
		  coeffValues[i] = (double)((double)(1.0)/bcnos);
	      cdbLocal.WriteDoubleArray(coeffValues,(int *) &bcnos, 1, "Numerator");
	      free((void*&)coeffValues);
	      cdbLocal.WriteDouble(1.0, "Denominator");
	      return (InitP((ConfigurationDataBase&)cdbLocal));
	  }
    } else {
        AssertErrorCondition(ParametersError,"Filter requires definition of Poles/Zeros or of Numerator/Denominator");
        return False;
    }

    FString name;
    cdb->NodeName(name);
    AssertErrorCondition(Information,"Filter %s = {",name.Buffer());
    FString polynomial;
    polynomial = "    inputCoeffs = [";
    int i;
    for (i=0;i < inputSize; i++) {
        polynomial.Printf("%12.6e ",inputCoefficients [i]);
    }
    polynomial += "]";
    AssertErrorCondition(Information,"%s",polynomial.Buffer());
    polynomial = "    outputCoeffs = [";
    for (i=0;i < outputSize; i++) {
        polynomial.Printf("%12.6e ",outputCoefficients [i]);
    }
    polynomial += "]";
    AssertErrorCondition(Information,"%s",polynomial.Buffer());
    AssertErrorCondition(Information,"}");


    Reset(0,0);
    return True;
}









