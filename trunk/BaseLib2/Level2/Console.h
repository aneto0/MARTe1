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
 * Adds Streaming to BasicConsole 
 */
#if !defined (CONSOLE_)
#define CONSOLE_

#include "Streamable.h"
#include "BasicConsole.h"
#include "CStreamBuffering.h"
#include "StreamAttributes.h"
#include "FString.h"

class Console;


extern "C" {

    /** */
    bool ConsoleSSWrite(Console &con,const void* buffer,uint32 &size,TimeoutType msecTimeout);

    /** */
    bool ConsoleSSRead(Console &con,void* buffer,uint32 &size,TimeoutType msecTimeout);

}

OBJECT_DLL(Console)

/** Implements a tream interface to the console */
class Console: public Streamable, public BasicConsole {

OBJECT_DLL_STUFF(Console)

    /** to buffer the console operations */
    char                streamBuffer[32];

    /** */
    FString             attribute;

    /** */
    bool                attributeReadMode;

private:

    friend bool ConsoleSSWrite(Console &con,const void* buffer, uint32 &size,TimeoutType msecTimeout);
    friend bool ConsoleSSRead(Console &con,void* buffer, uint32 &size,TimeoutType msecTimeout);

public:
    /** Creates a console stream with the desired parameters */
                        Console(
                                ConsoleOpeningMode  openingMode     = ConsoleDefault,
                                int             numberOfColumns     = -1,
                                int             numberOfRows        = -1,
                                TimeoutType     msecTimeout         = TTInfiniteWait)
    {
        ConsoleOpen(*this,openingMode,numberOfColumns,numberOfRows,msecTimeout);
        selectedStream      = NormalStreamMode;
        csbIn = new CStreamBuffering(this,streamBuffer,sizeof(streamBuffer));
        attributeReadMode = False;
    }

    /** destructor  */
    virtual             ~Console(){
        ConsoleClose(*this);
        delete csbIn;
    }

    /** can yoy write to it */
    virtual bool        CanWrite()
    {
        return True;
    }

    /** can you read from it  */
    virtual bool        CanRead()
    {
        return True;
    }

protected:
    /** This write function supports different streams */
    virtual bool        SSWrite(
                                const void*     buffer,
                                uint32 &        size,
                                TimeoutType     msecTimeout     = TTDefault)
    {
        if (selectedStream == 0) return BasicConsole::Write(buffer,size,msecTimeout);

        // we are not interested in reading so wipe it
        if (attributeReadMode) {
            attribute="";
            attributeReadMode = False;
        }

        return attribute.Write(buffer,size,msecTimeout);
    }

    /** This read function supports different streams */
    virtual bool        SSRead(
                                void*           buffer,
                                uint32 &        size,
                                TimeoutType     msecTimeout     = TTDefault)
    {
        if (selectedStream == 0) return BasicConsole::Read(buffer,size,msecTimeout);
        if (attributeReadMode != True) {
            size = 0;
            return False;
        }
        return attribute.Read(buffer,size,msecTimeout);
    }
public:
    /** To avoid confusion in mapping: specify Streamable */
    inline bool         Write(
                                const void*     buffer,
                                uint32 &        size,
                                TimeoutType     msecTimeout     = TTDefault)
    {
        return Streamable::Write(buffer,size,msecTimeout);
    }

    /** To avoid confusion in mapping: specify Streamable */
    inline bool         Read(
                                void*           buffer,
                                uint32 &        size,
                                TimeoutType     msecTimeout     = TTDefault)
    {
        return Streamable::Read(buffer,size,msecTimeout);
    }

public:

    /** change stream to write to */
    virtual bool        Switch(uint32 newSelectedStream)
    {
        if ((selectedStream > 0) && (attributeReadMode != True)){
            uint32 size = attribute.Size();
//            Console::SSWrite(attribute.Buffer(),size);
            ConsoleSSWrite(*this,attribute.Buffer(),size,0);
        }
        attribute="";
        if ( Streamable::Switch(newSelectedStream)){
            if (selectedStream > 0){
                char buffer[256];
                uint32 size = sizeof(buffer);
                ConsoleSSRead(*this,buffer,size,0);
//                Console::SSRead(buffer,size);
                attribute.Write(buffer,size);
                attribute.Seek(0);
                attributeReadMode = True;
            }
            return True;
        }
        return False;
    }

    /** Use to change the console attributes. Valid names are
        "colour"  R/W fg and bg colours,    two integers first is the fg second the bg
        "cursor"  R/W the cursor position,  two integers X,Y
        "size"    R/W the buffer size  ,    two integers DX,DY
        "window"  R/W the window size  ,    two integers DX,DY
     to switch back to console use Switch (ConsoleStreamMode) */
    virtual bool        Switch(const char *name){
        uint32 newSelectedStream = SANameToStreamModes(name);
        if (newSelectedStream == NullSelectedStream) return False;
        return Switch(newSelectedStream);
    }

    /** how many streams are available
        replace with a different value if multiple stream are supported */
    virtual uint32 NOfStreams(){
        return 5;
    }

#if defined (_WIN32)
    /** for WIN32 only since the Read function does not return the \n character */
    virtual bool        GetLine(
                                Streamable &        output,
                                bool                skipTerminators =True)
    {
        return GetToken(output,"\15",NULL,"\n");
    }

    /** for WIN32 only since the Read function does not return the \n character */
    virtual bool        GetLine(
                                char *              buffer,
                                uint32              maxSize,
                                bool                skipTerminators =True)
    {
        return GetToken(buffer,"\15",maxSize,NULL,"\n");
    }
#endif

    /** Sets the size of the buffer */
    inline bool         SetSize(
                                int             numberOfColumns,
                                int             numberOfRows)
    {
        return BasicConsole::SetSize(numberOfColumns, numberOfRows);
    }


};

#endif
