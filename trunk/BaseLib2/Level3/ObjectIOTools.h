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
 * @file
 */
#if !defined (OBJECT_IO_TOOLS_H)
#define OBJECT_IO_TOOLS_H

#include "Object.h"
#include "EAFile.h"

extern "C" {

    /** Save an object into a EAFile.
        @param the object content will taken from the CDB created by ObjectSaveSetup
               any data on the root.binary CDB entry will be saved in the main file,
               while all the other entries will be saved in other streams with corresponding names
        @param userId and @param flags are stored in their own special CDB entries       */
    bool OBJObjectSave(Object &object,EAFile &file,uint32 userId,uint32 flags,StreamInterface *err);

    /** Initialise or create an object from a EAFile.
        @param object if NULL the object is created otherwise initialised
        @param file the object content will written into a CDB to be used by ObjectLoadSetup
               binary data from the main file will be written on root.binary CDB entry ,
               all the other entries in the other streams will be copied into CDB before calling ObjectLoadSetup
        @param userId and @param flags are retrieved from their own special CDB entries
        returns NULL in case of failure, otherwise the object address */
    Object *OBJObjectLoadCreate(Object *object,EAFile &file,uint32 &userId,uint32 &flags,StreamInterface *err);

    /** Retrieves information about an object contained in a EAFile.
        @param userId and @param flags and @param classId are retrieved from the corresponding streams     */
    bool OBJObjectReadFlags(EAFile &file,uint32 &classId,uint32 &userId,uint32 &flags,StreamInterface *err);

    /** Modifies information about an object contained in a EAFile.
        @param userId and @param flags and @param classId are retrieved from the corresponding streams     */
    bool OBJObjectUpdateFlags(EAFile &file,uint32 classId,uint32 userId,uint32 flags,StreamInterface *err);

}

#endif

