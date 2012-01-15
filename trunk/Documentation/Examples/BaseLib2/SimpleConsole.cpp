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
 * @file Console example
 */
#include "ErrorManagement.h"
#include "FString.h"
#include "Console.h"

void PrintColourMenu(Console &con){
    con.Printf("1->Red\n");
    con.Printf("2->Green\n");
    con.Printf("3->White\n");
}

int main(int argc, char *argv[]){
    //Output logging messages to the console
    LSSetUserAssembleErrorMessageFunction(NULL); 

    //Create a console in single character read input
    Console con(PerformCharacterInput);
    con.Clear();
    //Change the color of the text
    con.Printf("Select foreground color\n");
    PrintColourMenu(con);
    char c1;
    //Read a single char
    con.GetC(c1);
    switch(c1){
        case '1':
            con.SetColour(Red, Black); 
            break;
        case '2':
            con.SetColour(Green, Black); 
            break;
        case '3':
            con.SetColour(White, Black); 
            break;
        default:
            con.Printf("Unrecognised colour code...");
            break;

    }
    //Read a line
    FString line;
    con.Printf("\nWrite a line\n");
    con.GetLine(line);
    con.Printf("You wrote: %s\n", line.Buffer());
    return 0;
}

