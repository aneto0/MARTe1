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
#include "BStringTest.h"


//Returns the size of the string.
uint32 sizeOfString(char* string) {
    uint32 i = 0;
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

bool BStringTest::TestStringOperators(const char* string1,
                                      const char* string2) {

    //Test the constructor with const char*
    BString stringOne(string1);
    BString stringTwo(string2);

    //Check if BString were initialized correctly
    if (!(stringCompare((char*) stringOne.Buffer(), (char*) string1))
            || !(stringCompare((char*) stringTwo.Buffer(), (char*) string2))) {
        return False;
    }

    //Test the equality operator between a BString and a const char*
    if (!(stringOne == string1) || !(stringTwo == string2)) {
        return False;
    }

    //Test the inequality operator between a BString and a const char*
    if ((stringOne != string1) || (stringTwo != string2)) {
        return False;
    }
    uint32 size1 = stringOne.Size();
    uint32 size2 = stringTwo.Size();

    //Check if the size of the BString is correct
    if (size1 != sizeOfString((char*) string1)
            || size2 != sizeOfString((char*) string2)) {
        return False;
    }

    uint32 sizeConcatenate = size1 + size2;

    char result[sizeConcatenate + 2];
    stringAppend((char*) string1, (char*) string2, result);

    //Concatenate operator between BStrings
    BString Bresult;
    Bresult += stringOne;
    Bresult += stringTwo;

    //Test if the concatenate operator works correctly
    if (!(stringCompare(result, (char*) Bresult.Buffer()))) {
        return False;
    }

    //Test the concatenate operator between Bstrings and const char*
    Bresult = "";
    Bresult += string1;
    Bresult += string2;

    if (!(stringCompare(result, (char*) Bresult.Buffer()))) {
        return False;
    }

    //Test the In() function for characters (true is the character is in the BString).
    //Test also the assignment by const char*
    Bresult = "Hello";

    if (!Bresult.In('H') || !Bresult.In('l') || !Bresult.In('o')) {
        return False;
    }

    BString newBString = "I'm New_Hello_World!";

    //Test the In() function for BStrings (true if a BString is contained in the other).
    if (!newBString.In(Bresult)) {
        return False;
    }

    newBString = Bresult;

    if (!newBString.In(Bresult)) {
        return False;
    }

    if (newBString != Bresult) {
        return False;
    }

    //The function setSize can cut the string if the new size is minor than before.
    Bresult.SetSize(3);
    if (Bresult != "Hel" || Bresult.Size() != 3) {
        return False;
    }

    //Otherwise if the new size is bigger than before, the size be increased. The terminate character remains, then a printf of the previous and final BStrings
    //give the same result (and also the insider function stringCompare which terminate the comparison at the first terminate character), but the equality operator
    //observes also the size of the BStrings then it returns that the two BStrings are different.
    Bresult = newBString;
    newBString.SetSize(10);
    if (newBString == Bresult
            || !stringCompare((char*) Bresult.Buffer(),
                              (char*) newBString.Buffer())) {
        return False;
    }

    //Test the return value if the position of the requested character of the string is out of bounds.
    if (Bresult[100] != 0) {
        return False;
    }

    //Test if the [] operator works correctly.
    return Bresult[Bresult.Size() - 1] == 'o';

}

bool BStringTest::TestCharOperators(char char1, char char2) {
    BString stringOne;
    BString stringTwo;

    //Assignment operator with char
    stringOne = char1;
    stringTwo = char2;

    //Build two "character" strings
    char string1[] = { char1, '\0' };
    char string2[] = { char2, '\0' };

    //Compare them with the BString Buffers
    if (!(stringCompare((char*) stringOne.Buffer(), (char*) string1))
            || !(stringCompare((char*) stringTwo.Buffer(), (char*) string2))) {
        return False;
    }

    int32 size1 = stringOne.Size();
    int32 size2 = stringTwo.Size();

    //Test if the size is correct (it should be one)
    if (size1 != 1 || size2 != 1) {
        return False;
    }

    int32 sizeConcatenate = size1 + size2;

    char result[sizeConcatenate + 2];
    stringAppend((char*) string1, (char*) string2, result);

    //Test the concatenate operator between BStrings 
    BString Bresult;
    Bresult += stringOne;
    Bresult += stringTwo;

    if (!(stringCompare(result, (char*) Bresult.Buffer()))) {
        return False;
    }
    //Test the concatenate operator between BStrings and characters
    Bresult = "";
    Bresult += char1;
    Bresult += char2;

    if (!(stringCompare(result, (char*) Bresult.Buffer()))) {
        return False;
    }

    return True;
}
