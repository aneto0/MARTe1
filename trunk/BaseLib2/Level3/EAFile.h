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

#if !defined (EA_FILE_H)
#define EA_FILE_H

#include "System.h"
#include "Streamable.h"
#include "File.h"
#include "StreamConfigurationDataBase.h"

/** 
 * @file
 * The file extension of the attributes file
 */
#define EA_EXT ".!EA!"

class EAFile;


extern "C" {
    /** Creates a file with extended attributes */
    bool EAFileOpen(EAFile &f,const char *name);

    /** Add a new extended attribute to a file */
    bool EAFileAddStream(EAFile &f,const char *name);

    /** Closes the file and the extended attributes */
    bool EAFileClose(EAFile &f);
}

OBJECT_DLL(EAFile)

/**  Extended attributes file */
class EAFile: public File{
OBJECT_DLL_STUFF(EAFile)

    friend bool EAFileOpen(EAFile &f,const char *name);
    friend bool EAFileClose(EAFile &f);
    friend bool EAFileAddStream(EAFile &f,const char *name);

protected:
    /** The file containing the attributes */
    File                        eaFile;

    /** True if any attributes have been set */
    bool                        hasEa;

//    /** The attributes*/

    /** A stream interface to the attributes */
    StreamConfigurationDataBase attributes;


public:
    /** destructor */
    virtual ~EAFile(){
        Close();
    }

    /** constructor */
    EAFile(){
        selectedStream = 0;
        hasEa  = False;
    }

#if defined(_VXWORKS)

    virtual bool Open(const char *fname,...);

#else
    /**  open for read/write create if missing */
    virtual bool Open(const char *fname,...){
        char name[256];
        va_list argList;
        va_start(argList,fname);
        vsnprintf(name,256,fname,argList);
        va_end(argList);
        return EAFileOpen(*this,name);
    }
#endif

    /** close the file and the attribute file */
    bool Close(){
        return EAFileClose(*this);
    }

    /** Get the attribute container */
    CDBExtended &GetCDB(){ return attributes.GetCDB(); }

    // PURE STREAMING

    /** Read from the selected stream */
    virtual bool SSRead (void* buffer, uint32 &size,TimeoutType msecTimeout){
        if (selectedStream == 0) return File::Read(buffer,size,msecTimeout);
        else return attributes.Read(buffer,size,msecTimeout);
    }


    /** */
    virtual bool CanRead(){
        if (selectedStream == 0) return File::CanRead();
        else return attributes.CanRead();
    }


    /** Write to the selected stream */
    virtual bool SSWrite(const void* buffer, uint32 &size,TimeoutType msecTimeout){
        if (selectedStream == 0) return File::Write(buffer,size,msecTimeout);
        else return attributes.Write(buffer,size,msecTimeout);
    }


    /** */
    virtual bool CanWrite(){
        if (selectedStream == 0) return File::CanWrite();
        else return attributes.CanWrite();
    }


    // RANDOM ACCESS INTERFACE

    /** The size of the file */
    virtual int64 Size(){
        if (selectedStream == 0) return File::Size();
        else return attributes.Size();
    }


    /** Moves within the file to an absolute location */
    virtual bool  Seek(int64 pos){
        if (selectedStream == 0) return File::Seek(pos);
        else return attributes.Seek(pos);
    }


    /** Returns current position */
    virtual int64 Position(void){
        if (selectedStream == 0) return File::Position();
        else return attributes.Position();
    }


    /** Clip the file size to a specified point */
    virtual bool  SetSize(int64 size) {
        if (selectedStream == 0) return File::SetSize(size);
        else return attributes.SetSize(size);
    }

    /** */
    virtual bool  CanSeek(){
        if (selectedStream == 0) return File::CanSeek();
        else return attributes.Size();
    }

    // Extended Attributes or Multiple Streams INTERFACE

    /** how many streams are available */
    virtual uint32 NOfStreams(){
        return 1 + attributes.NOfStreams();
    }

    /** select the stream to read from. Switching may reset the stream to the start. */
    virtual bool Switch(uint32 n) {
        if (n == 0)  {
            selectedStream = 0;
            return True;
        }
//        if (hasEa==False) return False;
        if (attributes.Switch(n-1)== True){
            selectedStream = n;
            Seek(0);
            return True;
        }
        selectedStream = NullSelectedStream;
        return False;
    }

    /** use the fileName or "" or NULL to reset to the file stream */
    virtual bool Switch(const char *name){
        if ((name == NULL) ||
            (name[0] == 0) ||
            (strcmp(fileName,name)==0)) {
            selectedStream = 0;
            return True;
        }
//        if (hasEa == False) return False;
        if (attributes.Switch(name)== True){
            selectedStream = attributes.SelectedStream()+1;
            Seek(0);
            return True;
        }

        selectedStream = NullSelectedStream;
        return False;
    }

    /** returns the name of the stream n.
        stream 0 is the file and therefore you get the fileName */
    virtual bool StreamName(uint32 n,char *name,int nameSize){
        if (n==0) {
            strncpy(name,fileName,nameSize-1);
            return True;
        }
        else return attributes.StreamName(n-1,name,nameSize);
    }

    /** add a new stream to write to.
        selectedStream is reset to 0 */
    virtual bool AddStream(const char *name){
        selectedStream = 0;
        return EAFileAddStream(*this,name);
    }

    /** add a new stream to write to.
        selectedStream is reset to 0 */
    virtual bool RemoveStream(const char *name){
        selectedStream = 0;
        return attributes.RemoveStream(name);
    }

};

#endif
