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
 * $Id:  $
 *
 **/
/**
 * @file
 * String basic management
 */

#ifndef STRING_HELPER_OS_H
#define STRING_HELPER_OS_H

#include <string.h>


//Concatenate 2 strings.
char* StringOsConcatenate(char* destination, const char* source){
	if(source == NULL || destination == NULL){
		return NULL;
	}
	return strcat(destination, source);
}

//Concatenate 2 strings until size chars.
char* StringOsConcatenateN(char* destination, const char* source, uint32 size){
	if(destination == NULL || source == NULL){
		return NULL;
	}
	return strncat(destination, source, size);
}

//Returns a pointer to the first occurrence of c in string.
char* StringOsSearchChar(char* string, char c){
	if(string == NULL){
		return NULL;
	}
	return strchr(string, c);	
}

//Compare two strings.
int32 StringOsCompare(const char* string1, const char* string2){	
	if(string1 == NULL || string2 == NULL){
		return -1;
	}

	int32 ret = strcmp(string1, string2);
	if(ret < 0){
		return 1; //1 if string1 < string2
	}
	if(ret >0){
		return 2; //2 if string1 > string2
	}
	return ret; //0 if string1=string2
}

//Compare two strings until size chars.
int32 StringOsCompareN(const char* string1, const char* string2, uint32 size){	
	if(string1 == NULL || string2 == NULL){
		return -1;
	}

	int32 ret = strncmp(string1, string2, size);
	if(ret < 0){
		return 1; //1 if string1 < string2
	}
	if(ret >0){
		return 2; //2 if string1 > string2
	}
	return ret; //0 if string1=string2
}


//Copy source in destination.
bool StringOsCopy(char* destination, const char* source){
	if(source == NULL || destination == NULL){
		return False;
	}
	return strcpy(destination, source) != NULL;
}

//Copy source in destination until size chars.
bool StringOsCopyN(char* destination, const char* source, uint32 size){
	if(source == NULL || destination == NULL){
		return False;
	}
	return strncpy(destination, source, size) != NULL;
}

//Return the index position of the first char in string2 founded in string1 -> "abcde" "12d" returns 3.
int32 StringOsSearchIndex(const char* string1, const char* string2){
	if(string1 == NULL || string2 == NULL){
		return -1;
	}
	return (int32)(strcspn(string1, string2));
}

//Return the length of a string.
int32 StringOsLength(const char* string){
	if(string == NULL){
		return -1;
	}
	return (int32)(strlen(string));	
}


//Return the pointer of the first char in string1 matched with one of chars in string2.
char* StringOsSearchChars(char* string1, const char* string2){
	if(string1 == NULL || string2 == NULL){
		return NULL;
	}
	return strpbrk(string1, string2);
}

//Return a pointer at the last char c founded in string
char* StringOsSearchLastChar(char* string, char c){
	if(string == NULL){
		return NULL;
	}
	return strrchr(string, c);
}


//Return a pointer to the first occurrence of substring in string.
char* StringOsSearchString(char* string, const char* substring){
	if(string == NULL || substring == NULL){
		return NULL;
	}
	return strstr(string, substring);
}

//if string is NULL the function tokenize from the pointer of the previous success strtok function.
char* StringOsTokenizer(char* string, const char* delimiter){
	return strtok(string, delimiter);
}

#endif


