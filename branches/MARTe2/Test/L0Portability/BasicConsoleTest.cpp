/* Copyright 2015 F4E | European Joint Undertaking for
 * ITER and the Development of Fusion Energy ('Fusion for Energy')
 *
 * Licensed under the EUPL, Version 1.1 or - as soon they
 will be approved by the European Commission - subsequent
 versions of the EUPL (the "Licence");
 * You may not use this work except in compliance with the
 Licence.
 * You may obtain a copy of the Licence at:
 *
 * http: //ec.europa.eu/idabc/eupl
 *
 * Unless required by applicable law or agreed to in
 writing, software distributed under the Licence is
 distributed on an "AS IS" basis,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 express or implied.
 * See the Licence
 permissions and limitations under the Licence.
 *
 * $Id:$
 *
 **/

#include "GeneralDefinitions.h"
#include "BasicConsoleTest.h"

//Returns the size of the string.
int32 sizeOfString(char* string) {
    int32 i = 0;
    if (string == NULL) {
        return -1;
    }

    while (string[i] != '\0') {
        i++;
    }
    return i;
}

//Returns true if the strings are equal, false otherwise
bool stringCompare(char* string1, char* string2) {
    int32 i = 0;
    while (1) {
        if (string1[i] != string2[i]) {
            return False;
        }
        if (string1[i] == '\0' && string2[i] == '\0') {
            return True;
        }
        if (string1[i] == '\0' || string2[i] == '\0') {
            return False;
        }
        i++;
    }
}

//Concatenate two strings
bool stringAppend(char* string1, char* string2, char* result) {
    int32 i = 0;
    int32 j = 0;
    while (1) {
        result[i] = string1[i];
        if (string1[i] == '\0') {
            break;
        }
        i++;
    }
    while (1) {
        result[i] = string2[j];
        if (string2[j] == '\0') {
            return True;
        }
        i++;
        j++;
    }
}

//Open the console with in the mode passed by argument
bool BasicConsoleTest::TestOpen(ConsoleOpeningMode openingMode) {
    return BasicConsoleOpen(myConsole, openingMode, N_COLUMNS, N_ROWS,
                            TTInfiniteWait);
}

//write the string passed by argument
bool BasicConsoleTest::TestWrite(char* string, int32 padding) {
    //since this function is used by other tests, open the console only if padding !=0.
    if (padding != 0) {
        if (!TestOpen(ConsoleDefault)) {
            return False;
        }
    }
    int32 stringSize;

    //calculate the size of the string
    if ((stringSize = sizeOfString(string)) < 0) {
        return False;
    }

    //add something to the size to pass as argument to test the write function
    uint32 size = stringSize + padding;

    //Only to return true in this case
    if (padding < 0) {
        stringSize -= padding;
    }

    bool condition1 = BasicConsoleWrite(myConsole, string, size,
                                        TTInfiniteWait);

    //return true if the size is correct
    return condition1 && (size == stringSize);

}

//compare the read string with the string passed by argument
bool BasicConsoleTest::TestRead(char* stringArg, int32 sizeArg) {
    //size must be positive
    if (sizeArg < 0) {
        return False;
    }

    //since this function is used by other tests, open the console only if padding !=0.
    if (sizeArg != 0) {
        if (!TestOpen(ConsoleDefault)) {
            return False;
        }
    }

    char string[N_COLUMNS];
    char result[N_COLUMNS + 20];
    uint32 size = N_COLUMNS;
    int32 stringSize;

    //calculate the size of the string
    if ((stringSize = sizeOfString(stringArg)) < 0) {
        return False;
    }

    //define the request to print
    stringAppend("\nPut: ", stringArg, result);

    //print the request: the user must insert the string passed by argument
    TestWrite(result, 0);

    //read the string
    bool condition1 = BasicConsoleRead(myConsole, string, size, TTInfiniteWait);

    //compare the read string with the argument
    bool condition2 = stringCompare(string, stringArg);

    //return true if the read string is equal to the argument
    if (sizeArg != 0) {
        return condition1 && condition2 && (size == sizeArg);
    }
    else {
        string[size] = '\n';
        string[size + 1] = '\0';
        return (size == 1) && TestWrite(string, 0);
    }

}

//Test the paging feature
bool BasicConsoleTest::TestPaging(int32 overflow, int32 rows, int32 columns) {

    //open the console in enable paging mode
    if (!TestOpen(EnablePaging)) {
        return False;
    }

    if (!myConsole.SetSize(rows, columns)) {
        return False;
    }

    //define the size of the string to print
    int32 n = 0;
    int32 limit = 2 * (rows + overflow - 1);
    char string[limit];

    //define the string to print
    while (n < limit) {
        string[n] = '-';
        string[n + 1] = '\n';
        n += 2;
    }
    string[limit] = '\0';

    //print the string
    return TestWrite(string, 0);
}

//Test the perform character input feature
bool BasicConsoleTest::TestPerfChar() {

    //open the console in perform character input mode
    if (!TestOpen(PerformCharacterInput)) {
        return False;
    }

    //return true if the size of the read string is one as aspected.
    return TestRead("press any key\n", 0);
}

