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
/**
 * @file File handling example
 */
#include "ErrorManagement.h"
#include "FString.h"
#include "File.h"

int main(int argc, char *argv[]){
    //Output logging messages to the console
    LSSetUserAssembleErrorMessageFunction(NULL); 

    //Create an output file
    FString filename = "output.txt";
    File    output;
    if(!output.OpenWrite(filename.Buffer())){
        CStaticAssertErrorCondition(FatalError, "Failed to open the output file: %s", filename.Buffer());
        return -1;
    }
    //Write something to it
    FString line = "Write something\ninto this\nfile\n";
    uint32  size = line.Size();
    if(!output.Write(line.Buffer(), size)){
        CStaticAssertErrorCondition(FatalError, "Failed to write to the output file: %s", filename.Buffer());
        return -1;
    }
    //Housekeeping
    output.Close();

    //Open the file for reading
    File input;
    if(!input.OpenRead(filename.Buffer())){
        CStaticAssertErrorCondition(FatalError, "Failed to open the input file: %s", filename.Buffer());
        return -1;
    }
    //Reset the line string
    line.SetSize(0);
    //Read the file
    while(input.GetLine(line)){
        CStaticAssertErrorCondition(Information, "Read: %s", line.Buffer());
        line.SetSize(0);
    }
    input.Close();
    return 0;
}

