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

#include "ClassStructure.h"
#include "ObjectRegistryItem.h"
#include "ObjectMacros.h"
#include "BasicTypes.h"

extern "C"{
    BASICTYPEREGISTER(float                 ,BTDFloat)
    BASICTYPEREGISTER(double                ,BTDDouble)
    BASICTYPEREGISTER(int                   ,BTDInt32)
    BASICTYPEREGISTER(long                  ,BTDInt32)
    BASICTYPEREGISTER2(long,int             ,BTDInt32)
    BASICTYPEREGISTER2(unsigned,int         ,BTDUint32)
    BASICTYPEREGISTER2(unsigned,long        ,BTDUint32)
    BASICTYPEREGISTER3(unsigned,long,int    ,BTDUint32)
    BASICTYPEREGISTER(short                 ,BTDInt16)
    BASICTYPEREGISTER2(short,int            ,BTDInt16)
    BASICTYPEREGISTER2(unsigned,short       ,BTDUint16)
    BASICTYPEREGISTER3(unsigned,short,int   ,BTDUint16)
    BASICTYPEREGISTER(char                  ,BTDInt8)
    BASICTYPEREGISTER2(unsigned,char        ,BTDUint8)
    BASICTYPEREGISTER(uint64                ,BTDUint64)
    BASICTYPEREGISTER(uint32                ,BTDUint32)
    BASICTYPEREGISTER(uint16                ,BTDUint16)
    BASICTYPEREGISTER(uint8                 ,BTDUint8)
    BASICTYPEREGISTER(int64                 ,BTDInt64)
    BASICTYPEREGISTER(int32                 ,BTDInt32)
    BASICTYPEREGISTER(int16                 ,BTDInt16)
    BASICTYPEREGISTER(int8                  ,BTDInt8)
};
