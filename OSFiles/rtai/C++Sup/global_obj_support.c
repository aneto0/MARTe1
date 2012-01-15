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

#include "global_obj_support.h"
void init_all_constructors(void) {
    extern void(*__CTOR_LIST__)(void);
    void (**constructor)(void) = &__CTOR_LIST__;
    int total = *(int*)constructor;
    constructor++;
    while(total) {
        (*constructor)();
        total--;
        constructor++;
    }
}

void delete_all_deconstructors(void) {
    extern void(*__DTOR_LIST__)(void);
    void(**deconstructor)(void) = &__DTOR_LIST__;
    int total = *(int*)deconstructor;
    deconstructor++;
    while(total) {
        (*deconstructor)();
        total--;
        deconstructor++;
    }
}

