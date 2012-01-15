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
#ifndef FCOMM_U_API_H
#define	FCOMM_U_API_H

#include "../share/fcomm_share.h"

typedef int (*gen_call_function)(mem_function *);
#define MAX_GEN_FUNCTION 256
gen_call_function gen_function_table[MAX_GEN_FUNCTION];

#define RTAI_FIFO_INPUT "/tmp/RTAICIn"
#define RTAI_FIFO_OUTPUT "/tmp/RTAICOut"

void init_gen_function_table(void);
int init_hashtables(void);
int init_api(void);
void close_api(void);
void destroy_hashtables(void);
int register_file_descriptor(int fildes, const char *mode);
void clear_screen(void);

#endif
