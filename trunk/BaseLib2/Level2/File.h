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
 * A Streamable version of File 
 */
#if !defined (FILE_CLASS_H)
#define FILE_CLASS_H

#include "System.h"
#include "Streamable.h"
#include "BasicFile.h"

OBJECT_DLL(File)

/**  The file */
class File: public Streamable, public BasicFile {

    OBJECT_DLL_STUFF(File)


public:
    /** constructor */
                        File()
    {
    }

    /** destructor  */
    virtual             ~File()
    {
        Close();
        SetFileName(NULL);
    }

    // PURE STREAMING
    /** This function is for the final class to implement. */
    virtual bool        SSRead(
                            void*               buffer,
                            uint32 &            size,
                            TimeoutType         msecTimeout     = TTDefault)
    {
        return BasicFile::Read(buffer,size,msecTimeout);
    }

    /** This function is for the final class to implement. */
    virtual bool        SSWrite(
                            const void*         buffer,
                            uint32 &            size,
                            TimeoutType         msecTimeout     = TTDefault)
    {
        return BasicFile::Write(buffer,size,msecTimeout);
    }

    /** avoid confusion between BasicFile::Read and Streamable::Read. */
    inline bool          Read(
                            void*               buffer,
                            uint32 &            size,
                            TimeoutType         msecTimeout     = TTDefault)
    {
        return Streamable::Read(buffer,size,msecTimeout);
    }

    /** avoid confusion between BasicFile::Write and Streamable::Write. */
    inline bool          Write(
                            const void*         buffer,
                            uint32 &            size,
                            TimeoutType         msecTimeout     = TTDefault)
    {
        return Streamable::Write(buffer,size,msecTimeout);
    }

    /** Can you actually Read from this stream */
    virtual bool        CanRead()
    {
        return BasicFile::CanRead();
    }

    /** Can you actually write to this stream */
    virtual bool        CanWrite()
    {
        return BasicFile::CanWrite();
    }

    // RANDOM ACCESS INTERFACE

    /** The file size */
    virtual int64       Size()
    {
        return BasicFile::Size();
    }

    /** Move the file pointer */
    virtual bool        Seek(int64 pos)
    {
        return BasicFile::Seek(pos);
    }

    /** The current file pointer */
    virtual int64       Position(void)
    {
        return BasicFile::Position();
    }

    /** Change file size: truncate or extend */
    virtual bool        SetSize(int64 size)
    {
        return BasicFile::SetSize(size);
    }

    /** Whether one can move the pointer in the file! */
    virtual bool        CanSeek()
    {
        return BasicFile::CanSeek();
    }

    // Extended Attributes or Multiple Streams INTERFACE

    /** The file name or the name of any other substream (only 0 available)*/
    /** the name of the stream we are using */
    virtual bool        StreamName(
                                uint32          n,
                                char *          name,
                                int             nameSize)
    {
        snprintf(name,nameSize,FileName(),selectedStream);
        return True;
    }

};


#endif
