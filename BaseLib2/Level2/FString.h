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
 * A BString with Streaming 
 */
#if !defined (FSTRING_H)
#define FSTRING_H

#include "BString.h"
#include "Streamable.h"

/** a string simulating a file. Reading a nd writing are done on a specific position */
class FString: public BString,public Streamable {

public:

    /** actual read function */
    virtual bool        SSRead(
                            void*               buffer,
                            uint32 &            size,
                            TimeoutType         msecTimeout     = TTDefault){
        return FSRead(*this,buffer,size);
    }

    /** */
    virtual bool        CanRead(){
        return True;
    }

    /** actual write function */
    virtual bool        SSWrite(
                            const void*         buffer,
                            uint32 &            size,
                            TimeoutType         msecTimeout     = TTDefault){
        return FSWrite(*this,buffer,size);
    }


    /** */
    virtual bool        CanWrite(){
        return True;
    }

    // RANDOM ACCESS INTERFACE

    /** The size of the string */
    virtual int64 Size(){
        return size;
    }

    /** */
    virtual bool        Seek(int64 pos)
    {
        uint32 maxPos = Size();
        if (pos > maxPos) pos = maxPos;
        if (pos < 0)      pos = 0;
        position = pos;
        return True;
    }

    /** */
    virtual int64 Position(void){
        // adjust position
        if (position > Size()) position = Size();
        return position;
    }

    /**  Clip the string size to a specified point*/
    virtual bool SetSize(int64 size){
        if (FSAlloc(*this,size)==False) return False;

        if (size < position) position = size;
        this->size = size;
        buffer[size]=0;
        return True;
    }

    /** */
    virtual bool  CanSeek()
        { return True; };

    // STRING SPECIFICS

    /** */
    FString(){
    }

    /** */
    FString(const FString &x){
        *this = x;
    }

    /** */
    FString(const char *s){
        *this = s;
    }

    /** */
    virtual ~FString(){
    }


    /** set string to be a copy of the input parameter */
    inline bool operator=(const FString &s){
        return Copy(s);
    }

    /** set string to be a copy of the input parameter */
    inline bool operator=(char c){
        return Copy(c);
    }

    /** set string to be a copy of the input parameter */
    inline bool operator=(const char *s){
        return Copy(s);
    }

    /** set string to be a copy of the input parameter */
    inline bool operator=(const BString &s){
        return Copy(s);
    }

    /** concatenate to the end of string  */
    inline bool operator+=(const char c){
        position = size;
        uint32 wsize = 1;
        char temp = c;
        return FSWrite(*this,&temp,wsize);
    }

    /** concatenate to the end of string  */
    inline bool operator+=(const char *s){
        if (s == NULL) return False;
        position = size;
        uint32 wsize = strlen(s);
        return FSWrite(*this,s,wsize);
    }

    /** concatenate to the end of string  */
    inline bool operator+=(BString &s){
        position = size;
        uint32 wsize = s.Size();
        return FSWrite(*this,s.Buffer(),wsize);
    }

    /** compare with parameter  */
    inline bool operator==(BString &s) const {
        if (size!=s.Size()) return False;
        if (strcmp(buffer,s.Buffer())!=0) return False;
        return True;
    }

    /** compare with parameter  */
    inline bool operator==(const char *s) const {
        if (s==NULL) return False;
        if (strcmp(buffer,s)!=0) return False;
        return True;
    }

};

#endif

