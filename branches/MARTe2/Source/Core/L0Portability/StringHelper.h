/*
 * Copyright 2015 F4E | European Joint Undertaking for 
 * ITER and the Development of Fusion Energy ('Fusion for Energy')
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
 * See the Licence  
 permissions and limitations under the Licence. 
 *
 * $Id: $
 *
 **/
/**
 * @file
 * Basic string management
 */

#ifndef STRING_HELPER_H
#define STRING_HELPER_H

#include "GeneralDefinitions.h"

/**
 * @brief Implementation of functions to manipulate strings. 
 *
 * These methods allows the most used and basic functions to manipulate char* strings and
 * are particularly useful in the implementation of higher levels string and stream implementations.
 *
 * Most of the implementation is delegated to StringHelperOS.h.
 */

extern "C" {
/** @brief Concatenate two strings.
 * @param destination is the string where source must be appended.
 * @param source is the string to append to destination.
 * @return destination. */
char* StringConcatenate(char* destination, const char* source);

/** @brief Concatenate two strings until 'size' chars.
 * @param destination is the string where source must be appended.
 * @param source is the string to append to destination.
 * @return destination. */
char* StringConcatenateN(char* destination, const char* source, uint32 size);

/** @brief Search a character in a string.
 * @param string is the source string.
 * @param c is the character to find in c.
 * @return a pointer to the first occurrence of c in string, NULL if c is not found. */
char* StringSearchChar(char* string, char c);

/** @brief Compare two strings.
 * @param string1 is the first string.
 * @param string2 is the second string.
 * @return (0 if string1 = string2), (1 if string1 < string2), (2 if string1 > string2), (-1 in case of NULL strings). */
int32 StringCompare(const char* string1, const char* string2);

/** @brief Compare two strings until 'size' characters.
 * @param string1 is the first string.
 * @param string2 is the second string.
 * @param size is the maximum number of char to compare.
 * @return (0 if string1 = string2), (1 if string1 < string2), (2 if string1 > string2), (-1 in case of NULL strings). */
int32 StringCompareN(const char* string1, const char* string2, uint32 size);

/** @brief Copy source in destination.
 * @param destination is the destination string.
 * @param source is the string to copy in destination.
 * @return destination. */
bool StringCopy(char* destination, const char* source);

/** @brief Copy source in destination until 'size' chars.
 * @param destination is the destination string.
 * @param source is the string to copy in destination.
 * @param size is the maximum numbero of byte to copy.
 * @return destination. */
bool StringCopyN(char* destination, const char* source, uint32 size);

/** @brief Return the index position of the first char in string2 founded in string1 -> "abcde" "12d" returns 3.
 * @param string1 is the source string.
 * @param string2 contains characters which must be searched in string1.
 * @return the index at the first occurrence of the first character in string2 found in string1. */
int32 StringSearchIndex(const char* string1, const char* string2);

/** @brief Return the length of a string.
 * @param string is the source string.
 * @return the length of the string. */
int32 StringLength(const char* string);

/** @brief Return the pointer of the first char in string1 matched with one of chars in string2.
 * @param string1 is the source string.
 * @param string2 contains characters which must be searched in string1.
 * @return a pointer to the first occurrence of a character in string2 contained in string1. */
char* StringSearchChars(char* string1, const char* string2);

/** @brief Return a pointer at the last char c founded in string.
 * @param string is the source string.
 * @return the last occurrence of c in string. */
char* StringSearchLastChar(char* string, char c);

/** @brief Return a pointer to the first occurrence of substring in string.
 * @param string is the source string.
 * @param substring is the string which must be searched in string.
 * @returns a pointer to the first occurrence of substring in string. */
char* StringSearchString(char* string, const char* substring);

/** @brief Returns the substring of string until delimiter.
 * If string is NULL the function tokenize from the pointer of the previous success strtok function.
 * @param string is the source string.
 * @param delimiter is the string delimiter.
 * @return a pointer to string terminated where delimiter is found. */
char* StringTokenizer(char* string, const char* delimiter);
}

#endif
