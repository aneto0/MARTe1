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

#ifndef C_FUNC_SUP_H
#define C_FUNC_SUP_H

#if !defined(__cplusplus)
#include "linux_version.h"
#include <linux/types.h>
#endif
#include <stddef.h>
#if defined(__cplusplus)
extern "C"
{
#endif
void *malloc(size_t size);
void free(void *ptr);
void *realloc(void *data, size_t newSize);

void *rtai_memcpy(void *dest, const void *src, size_t n);
void *rtai_memset(void *src, int c, size_t n);
int rtai_memcmp(const void *cs, const void *ct, size_t count);
size_t rtai_strlen(const char *s);
int rtai_strcmp(const char * cs,const char * ct);
char *rtai_strchr(const char *s, int c);    
char *rtai_strstr(const char *haystack, const char *needle);
char *rtai_strncpy(char *dest, const char *src, size_t n);
char *rtai_strcpy(char *dest, const char *src);
char *rtai_strcat(char *dest, const char *src);
int rtai_strncmp(const char *cs, const char *ct, size_t count);    
char *srtai_trncat(char *dest, const char *src, size_t n);
double rtai_strtod(const char *nptr, char **endptr);
char *rtai_strcasestr(const char *haystack, const char *needle);
int rtai_toupper(int c);
int rtai_tolower(int c);
int rtai_atoi(const char *nptr);
long rtai_atol(const char *cp);
double rtai_atof(const char *str);
char *rtai_strncat(char *dest, const char *src, size_t count);
char *rtai_strpbrk(const char *cs, const char *ct);
long simple_strtol(const char *cp, char **endp, unsigned int base);

#if defined(__cplusplus)
}
#endif
#endif
