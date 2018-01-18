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
 * @file Extends StreamInterface adding buffering  
 */ 
#if !defined (STREAMABLE_H)
#define STREAMABLE_H

#include "System.h"
#include "CStream.h"
#include "CStreamBuffering.h"
#include "StreamInterface.h"
#include "Object.h"
#include "StreamAttributes.h"

/** the selected stream does not exist and
therefore accesses go to nothing */
const uint32 NullSelectedStream = (uint32)NullStreamMode;

class Streamable;

extern "C"{
    /** Implements StreamInterface::CompleteRead*/
    bool StreamCompleteRead(Streamable &s,void* buffer, uint32 &size, TimeoutType msecTimeout, int32 pollMs);

    /** Implements StreamInterface::CompleteWrite*/
    bool StreamCompleteWrite(Streamable &s,const void* buffer, uint32 &size, TimeoutType msecTimeout, int32 pollMs);
};

OBJECT_DLL(Streamable)

/** The prototype for all streams. Adds to StreamInterface many useful functions.*/
class Streamable: public StreamInterface/*,public Object */{
OBJECT_DLL_STUFF(Streamable)

protected:
    /** The input buffering */
    CStreamBuffering *csbIn;

    /** The output buffering */
    CStreamBuffering *csbOut;

    /** 0 means the default stream
        0xFFFFFFFF means no stream */
    uint32 selectedStream;

public:
    /** constructor */
                        Streamable()
    {
        csbIn=NULL;
        csbOut=NULL;
        selectedStream = 0;
    }

    /** destructor */
    virtual             ~Streamable(){}

    /** @name PURE STREAMING
        @{
    */

    /** True if the stream can be read from */
    virtual bool        CanRead()
    {
        return False;
    }

    /** True if the stream can be written to */
    virtual bool        CanWrite()
    {
        return False;
    }

protected:

    /** This function is for the final class to implement. */
    virtual bool        SSRead(
                            void*               buffer,
                            uint32 &            size,
                            TimeoutType         msecTimeout     = TTDefault)=0;


    /** This function is for the final class to implement. */
    virtual bool        SSWrite(
                            const void*         buffer,
                            uint32 &            size,
                            TimeoutType         msecTimeout     = TTDefault)=0;

public:

    /** Used also as call back from the C buffer functions.
        Do not remap this function unless the class needs to deal
        with multiple stream in a different way.
        if a non existing stream has been selected the call will return False*/
    virtual bool        Read(
                            void*               buffer,
                            uint32 &            size,
                            TimeoutType         msecTimeout     = TTDefault)
    {
        if (selectedStream == NullSelectedStream) return False;
        return SSRead(buffer,size,msecTimeout);
    }

    /** Used also as call back from the C buffer functions.
        Do not remap this function unless the class needs to deal
        with multiple stream in a different way.
        if a non existing stream has been selected the call will return False*/
    virtual bool        Write(
                            const void*         buffer,
                            uint32 &            size,
                            TimeoutType         msecTimeout     = TTDefault)
    {
        if (selectedStream == NullSelectedStream) return True;
        return SSWrite(buffer,size,msecTimeout);
    };

    /** Single character write */
    inline bool         PutC(char c)
    {
        uint32 size = 1;
        return (Write(&c, size) && (size == 1));
    }

    /** Single character read */
    inline bool         GetC(char &c)
    {
        uint32 size = 1;
        return (Read(&c, size) && (size == 1));
    }

    /** @}
        @name Random Access Interface
        @{
    */

    /** The size of the file */
    virtual int64       Size()
    {
        return -1;
    }

    /** Moves within the file to an absolute location */
    virtual bool        Seek(int64 pos)
    {
        return False;
    }

    /** Returns current position */
    virtual int64       Position(void)
    {
        return -1;
    }

    /** Clips the file size to a specified point */
    virtual bool        SetSize(int64 size)
    {
        return False;
    }

    /** whether it is read */
    virtual bool        CanSeek()
    {
        return False;
    }

    /**
        @}
        @name Multiple Stream Interface
        @{
    */

    /** how many streams are available
        replace with a different value if multiple stream are supported */
    virtual uint32      NOfStreams()
    {
        return 1;
    }

    /** select the stream to read from.
        Switching may reset the stream to the start.
        0 is the default stream
        False is only returned if the switch was not performed */
    virtual bool        Switch(uint32 selectedStream)
    {
        if (selectedStream >= NOfStreams() ){
            this->selectedStream = NullSelectedStream;
            return False;
        }
        this->selectedStream = selectedStream;
        return True;
    }

    /** select the stream to read from.
        Switching may reset the stream to the start.
        Override with a method providing name to number mapping!
        False is only returned if the switch was not performed */
    virtual bool        Switch(const char *name)
    {
        this->selectedStream = NullSelectedStream;
        return True;
    }

    /** what stream has been selected */
    virtual uint32      SelectedStream()
    {
        return selectedStream;
    }

    /** the name of the stream we are using */
    virtual bool        StreamName(
                                uint32          n,
                                char *          name,
                                int             nameSize)
    {
#if defined(_MACOSX)
	snprintf(name,nameSize,"stream%u",selectedStream);
#else
        snprintf(name,nameSize,"stream%lu",(long unsigned)selectedStream);
#endif
        return True;
    }

    /** add a new stream to write to. */
    virtual bool        AddStream(const char *name)
    {
        return False;
    }

    /** remove an existing stream . */
    virtual bool        RemoveStream(const char *name)
    {
        return False;
    }

    /**
        @}
        @name Utility functions
        @{
    */

    /** the function performs the job of a the Read function but guarantees
        the completion. In case of failure size returns the actual data read.
        @param timeOutMs is the total allowed wait time cehcked using HRT.
        Any value below 0 implies infinite wait. */
    virtual bool        CompleteRead(
                            void*           buffer,
                            uint32 &        size,
                            TimeoutType     msecTimeout = TTInfiniteWait)

    {
        return StreamCompleteRead(*this,buffer,size,msecTimeout,0);
    }

    /** the function performs the job of a the Write function but guarantees
        the completion. In case of failure size returns the actual data read.
        @param timeOutMs is the total allowed wait time checked using HRT.
        Any value below 0 implies infinite wait. */
    virtual bool        CompleteWrite(
                            const void*     buffer,
                            uint32 &        size,
                            TimeoutType     msecTimeout = TTInfiniteWait)
    {
        return StreamCompleteWrite(*this,buffer,size,msecTimeout,0);
    }

#if 0
    /** will use CompleteWrite */
    inline void operator<<(char *p){
        if (p!=NULL){
            uint32 size= strlen(p);
            CompleteWrite(p,size);
        }
    }
#endif
    /**
        @}
        @name Buffered operations
        @{
    */

    /** buffer is written from the stream.
        Do not remap this function unless the class needs to deal
        with buffering in a different way. */
    virtual bool        BufferedRead(
                            void*               buffer,
                            uint32 &            size,
                            TimeoutType         msecTimeout     = TTDefault)
    {
        if (csbIn == NULL) return Read(buffer,size);
        return CRead(csbIn->UseAsInput(),buffer,size);
    }

    /** buffer is read and copied into the stream.
        Do not remap this function unless the class needs to deal
        with buffering in a different way. */
    virtual bool        BufferedWrite(
                            const void*         buffer,
                            uint32 &            size,
                            TimeoutType         msecTimeout     = TTDefault)
    {
        if (csbOut == NULL) return Write(buffer,size);
        return CWrite(csbOut->UseAsOutput(),buffer,size);
    }

    /** needed to complete buffered operations */
    inline void         Flush()
    {
        if (csbIn != NULL)  csbIn->Flush();
        if (csbOut != NULL) csbOut->Flush();
    }

#if defined(_VXWORKS)

    /** */
    bool                VPrintf(const char *format,va_list argList);

#else


    /** will write any text using printf format
        {BUFFERED if buffering activated} */
    inline bool         VPrintf(
                                const char *        format,
                                va_list             argList)
    {
        if (csbOut == NULL){
            char stackBuffer[StreamableBufferingSize];
            CStreamBuffering csb(this,stackBuffer,sizeof(stackBuffer));
            return VCPrintf(csb.UseAsOutput(),format,argList);
        }
        return VCPrintf(csbOut->UseAsOutput(),format,argList);
    }

#endif

    /** will write an integer with a specific format (i X x o) desiredSize is for padding
        {BUFFERED if buffering activated} */
    inline bool         Print(
                                int32               n,
                                int                 desiredSize     =0,
                                char                desiredPadding  =0,
                                char                mode            = 'i')
    {
        if (csbOut == NULL){
            char stackBuffer[StreamableBufferingSize];
            CStreamBuffering csb(this,stackBuffer,sizeof(stackBuffer));
            return CPrintInt32(csb.UseAsOutput(),n,desiredSize,desiredPadding,mode);
        }
        return CPrintInt32(csbOut->UseAsOutput(),n,desiredSize,desiredPadding,mode);
    }

    /** will write a float with a specific format (f or d)     desiredSize is for padding
        {BUFFERED if buffering activated} */
    inline bool         Print(
                                double              f,
                                int                 desiredSize     =0,
                                int                 desiredSubSize  =6,
                                char                desiredPadding  =0,
                                char                mode            = 'f')
    {
        if (csbOut == NULL){
            char stackBuffer[StreamableBufferingSize];
            CStreamBuffering csb(this,stackBuffer,sizeof(stackBuffer));
            return CPrintDouble(csb.UseAsOutput(),f,desiredSize,desiredSubSize,desiredPadding,mode);
        }
        return CPrintDouble(csbOut->UseAsOutput(),f,desiredSize,desiredSubSize,desiredPadding,mode);
    }

    /** will write a string                                    desiredSize is for padding
        {BUFFERED if buffering activated} */
    inline bool         Print(
                                const char *        s,
                                int                 desiredSize     =0,
                                char                desiredPadding  =0)
    {
        if (csbOut == NULL){
            char stackBuffer[StreamableBufferingSize];
            CStreamBuffering csb(this,stackBuffer,sizeof(stackBuffer));
            return CPrintString(csb.UseAsOutput(),s,desiredSize,desiredPadding);
        }
        return CPrintString(csbOut->UseAsOutput(),s,desiredSize,desiredPadding);
    }

    /** extract a token from the stream into a string data until a terminator or 0 is found.
        Skips all skip characters and those that are also terminators at the beginning
        returns true if some data was read before any error or file termination. False only on error and no data available
        The terminator (just the first encountered) is consumed in the process and saved in saveTerminator if provided
        skipCharacters=NULL is equivalent to skipCharacters = terminator
        {BUFFERED}    */
    virtual bool        GetToken(
                                char *              buffer,
                                const char *        terminator,
                                uint32              maxSize,
                                char *              saveTerminator  =NULL,
                                const char *        skipCharacters  =NULL)
    {
        if (csbIn == NULL){
            if (CanSeek()){
                char stackBuffer[StreamableBufferingSize];
                CStreamBuffering csb(this,stackBuffer,sizeof(stackBuffer));
                return CGetToken(csb.UseAsInput(),buffer,terminator,maxSize,saveTerminator,skipCharacters);
            } else {
                char stackBuffer[1];
                CStreamBuffering csb(this,stackBuffer,sizeof(stackBuffer));
                return CGetToken(csb.UseAsInput(),buffer,terminator,maxSize,saveTerminator,skipCharacters);
            }
        }
        return CGetToken(csbIn->UseAsInput(),buffer,terminator,maxSize,saveTerminator,skipCharacters);
    }

    /** extract a token from the stream into a string data until a terminator or 0 is found.
        Skips all skip characters and those that are also terminators at the beginning
        returns true if some data was read before any error or file termination. False only on error and no data available
        The terminator (just the first encountered) is consumed in the process and saved in saveTerminator if provided
        skipCharacters=NULL is equivalent to skipCharacters = terminator
        {BUFFERED}
        A character can be found in the terminator or in the skipCharacters list  in both or in none
        0) none                 the character is copied
        1) terminator           the character is not copied the string is terminated
        2) skip                 the character is not copied
        3) skip + terminator    the character is not copied, the string is terminated if not empty
*/
    virtual bool        GetToken(
                                StreamInterface &   output,
                                const char *        terminator,
                                char *              saveTerminator=NULL,
                                const char *        skipCharacters=NULL)
    {
        int bsize = StreamableBufferingSize;
        if (!CanSeek()) bsize = 1;
        if ( csbIn == NULL){
            char stackBufferIn[StreamableBufferingSize];
            char stackBufferOut[StreamableBufferingSize];
            CStreamBuffering csbIn(this,stackBufferIn,bsize);
            CStreamBuffering csbOut(&output,stackBufferOut,sizeof(stackBufferOut));
            return CGetCSToken(csbIn.UseAsInput(),csbOut.UseAsOutput(),terminator,saveTerminator,skipCharacters);
        }
        char stackBufferOut[StreamableBufferingSize];
        CStreamBuffering csbOut(&output,stackBufferOut,sizeof(stackBufferOut));
        return CGetCSToken(csbIn->UseAsInput(),csbOut.UseAsOutput(),terminator,saveTerminator,skipCharacters);
    }

    /** extract a token from the stream into a string data until a terminator or 0 is found.
        Skips all skip characters and those that are also terminators at the beginning
        returns true if some data was read before any error or file termination. False only on error and no data available
        The terminator (just the first encountered) is consumed in the process and saved in saveTerminator if provided
        skipCharacters=NULL is equivalent to skipCharacters = terminator
        {BUFFERED}
        A character can be found in the terminator or in the skipCharacters list  in both or in none
        0) none                 the character is copied
        1) terminator           the character is not copied the string is terminated
        2) skip                 the character is not copied
        3) skip + terminator    the character is not copied, the string is terminated if not empty
*/
    virtual bool        GetToken(
                                Streamable &        output,
                                const char *        terminator,
                                char *              saveTerminator=NULL,
                                const char *        skipCharacters=NULL)
    {
        int bsize = StreamableBufferingSize;
        if (!CanSeek()) bsize = 1;
        if ((csbIn == NULL)&&(output.csbOut == NULL)){
            char stackBufferIn[StreamableBufferingSize];
            char stackBufferOut[StreamableBufferingSize];
            CStreamBuffering csbIn(this,stackBufferIn,bsize);
            CStreamBuffering csbOut(&output,stackBufferOut,sizeof(stackBufferOut));
            return CGetCSToken(csbIn.UseAsInput(),csbOut.UseAsOutput(),terminator,saveTerminator,skipCharacters);
        }
        if (csbIn == NULL){
            char stackBufferIn[StreamableBufferingSize];
            CStreamBuffering csbIn(this,stackBufferIn,bsize);
            return CGetCSToken(csbIn.UseAsInput(),output.csbOut->UseAsOutput(),terminator,saveTerminator,skipCharacters);
        }
        if (output.csbOut == NULL){
            char stackBufferOut[StreamableBufferingSize];
            CStreamBuffering csbOut(&output,stackBufferOut,sizeof(stackBufferOut));
            return CGetCSToken(csbIn->UseAsInput(),csbOut.UseAsOutput(),terminator,saveTerminator,skipCharacters);
        }
        return CGetCSToken(csbIn->UseAsInput(),output.csbOut->UseAsOutput(),terminator,saveTerminator,skipCharacters);
    }

    /**
     * Searches for a full string inside another string
     * @param output the string between terminators
     * @param terminator the token to search
     * @return True if a valid token was found
     */
    virtual bool        GetStringToken(
                                Streamable &        output,
                                const char *        terminator)
    {
        int bsize = StreamableBufferingSize;
        if (!CanSeek()) bsize = 1;
        if ( csbIn == NULL){
            char stackBufferIn[StreamableBufferingSize];
            char stackBufferOut[StreamableBufferingSize];
            memset(stackBufferIn, 0, StreamableBufferingSize);
            CStreamBuffering csbIn(this,stackBufferIn,bsize);
            CStreamBuffering csbOut(&output,stackBufferOut,sizeof(stackBufferOut));
            return CGetStringToken(csbIn.UseAsInput(),csbOut.UseAsOutput(),terminator, Size());
        }
        char stackBufferOut[StreamableBufferingSize];
        CStreamBuffering csbOut(&output,stackBufferOut,sizeof(stackBufferOut));
        return CGetStringToken(csbIn->UseAsInput(),csbOut.UseAsOutput(),terminator, Size());
    }

    /** @param skipTerminators will skip an empty line or any part of a line termination */
    virtual bool        GetLine(
                                Streamable &        output,
                                bool                skipTerminators =True)
    {
        const char *skip = "\r";
#if defined (_WIN32)
        if (!skipTerminators) skip = "\r";
#else
        if (!skipTerminators) skip = "";
#endif
        return GetToken(output,"\n",NULL,skip);
    }

    /** to skip a series of tokens delimited by terminators or 0
        {BUFFERED}    */
    virtual bool        SkipTokens(
                                uint32              count,
                                const char *        terminator)
    {
        if (csbIn == NULL){
            if (CanSeek()){
                char stackBuffer[StreamableBufferingSize];
                CStreamBuffering csb(this,stackBuffer,sizeof(stackBuffer));
                return CSkipTokens(csb.UseAsInput(),count,terminator);
            } else {
                char stackBuffer[1];
                CStreamBuffering csb(this,stackBuffer,sizeof(stackBuffer));
                return CSkipTokens(csb.UseAsInput(),count,terminator);
            }
        }
        return CSkipTokens(csbIn->UseAsInput(),count,terminator);
    }

    /** extract a token from the string into a string until a terminator or 0 is found.
        The maximum string size is maxSize -1
        Skips any number of leading terminators
        returns true if some data was read. False only on no data available
        The input pointer will be left pointing at the terminator. The terminator is not consumed    */
    static inline bool  GetCStringToken(
                                const char *&       input,
                                char *              buffer,
                                const char *        terminator,
                                uint32              maxSize)
    {
        return CGetCStringToken(input,buffer,terminator,maxSize);
    }

    /** extract a token from the string into a string until a terminator or 0 is found.
        Skips any number of leading characters that are skipCharacters and terminator!
        affects the input by placing 0 at the end of each token
        Never returs NULL unless the input is NULL;
        The terminator (just the first encountered) is consumed in the process and saved in saveTerminator if provided
    */
    static inline char *DestructiveGetCStringToken(
                                char *&             input,
                                const char *        terminator,
                                char *              saveTerminator  =NULL,
                                const char *        skipCharacters  ="")
    {
        return CDestructiveGetCStringToken(input,terminator,saveTerminator,skipCharacters);
    }

    /** makes this overload immediately available */
    virtual bool        GetLine(
                                char *              buffer,
                                uint32              maxSize,
                                bool                skipTerminators =True)
    {
        return StreamInterface::GetLine(buffer,maxSize,skipTerminators);
    }


    /** extract data from the stream until the matching string is found
        the pointer is to the first character after the pattern    */
    inline bool         FindPattern(const char *pattern)
    {
        if (pattern == NULL) return False;
        int fullMatch = strlen(pattern);
        int match = 0;
        char c;
        while((match < fullMatch) && (GetC(c))){
            if (c == pattern[match]) match++;
            else {
                if (match > 0){
                    bool selfMatched = False;
                    int step = 1;
                    while ((step <= match) && !selfMatched){
                        while ((step <= match) && (c != pattern[match-step])) step++;
                        if     (step <= match){
                            int index = 0;
                            selfMatched = True;
                            while((index < (match-step)) && selfMatched){
                                if (pattern[index] != pattern[index+step]) selfMatched = False;
                                index++;
                            }
                        }
                    }
                    match = match - step + 1;
                }
            }
        }

        return (match==fullMatch);

    }

    /**
        @}
    */

};

#endif
