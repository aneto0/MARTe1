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

/*
 * @file
 * Streamable eXtension for the standard C FILE 
 */
#if !defined (SX__FILE__H)
#define SX__FILE__H

#include "System.h"
#include "Streamable.h"

class SXFILE: public Streamable{
    /** */
#ifndef _RTAI
    FILE *file;
#else
    int file;
#endif
public:
    /** */
    virtual ~SXFILE(){}

    /** */
#ifndef _RTAI
    SXFILE(FILE *f){ file = f; }
#else
    SXFILE(int f){ file = f; }
#endif

    // PURE STREAMING
    /** */
    bool SSRead(void* buffer, uint32 &size,TimeoutType msecTimeout){
        if (file == NULL) {
            size = 0;
            return False;
        }
        int ret = fread(buffer,1,size,file);
        if (ret >= 0){
            size = ret;
            return True;
        }
        size = 0;
        return False;
    }

    /** */
    bool CanRead(){ return (file != NULL); };

    /** */
    bool SSWrite(const void* buffer, uint32 &size,TimeoutType msecTimeout){
        if (file == NULL) {
            size = 0;
            return False;
        }
        int ret = fwrite(buffer,1,size,file);
        if (ret >= 0){
            size = ret;
            return True;
        }
        size = 0;
        return False;
    }

    /** */
    virtual bool CanWrite(){ return (file != NULL); };

    // RANDOM ACCESS INTERFACE

    /** */
    int64 Size(){
        if (file == NULL) return -1;
        int32 pos = ftell(file);
        fseek(file,0,SEEK_END);
        int32 size = ftell(file);
        fseek(file,pos,SEEK_SET);
        return size;
    }

    /** */
    bool  Seek(int64 pos){
        if (file == NULL) return False;
        int32 p = pos;
        fseek(file,p,SEEK_SET);
        return True;
    }

    /** */
    int64 Position(void){
        if (file == NULL) return -1;
        return ftell(file);
    }

    /** */
    bool SetSize(int64 size){
        if (file == NULL) return False;
        return False;
    }

    /** */
    virtual bool  CanSeek(){ return (file != NULL); };
};



#endif
