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
 * a wrapper stream to help build HTML pages
 */
#if !defined (HTML_STREAM)
#define HTML_STREAM

#include "Streamable.h"
#include "StreamAttributes.h"
#include "FString.h"

class HtmlStream;

extern "C" {
    /** write using html format conversions */
    bool HSWWrite(HtmlStream &hsw, const void* buffer, uint32 &size);

    /** flush out the buffer */
    bool HSWFlush(HtmlStream &hsw);
}

class HtmlStream: public Streamable{
    /** store the temporary output here when stream mode not 0 */
    FString buffer;

    /** the html encoded will be written here */
    StreamInterface *wrappedStream;

    friend bool HSWWrite(HtmlStream &hsw, const void* buffer, uint32 &size);

    friend bool HSWFlush(HtmlStream &hsw);

public:

    /** initialise with the stream to be wrapped */
    HtmlStream(StreamInterface &wrappedStream){
        this->wrappedStream = &wrappedStream;
    }

    /** flush on exit */
    virtual ~HtmlStream(){
        HSWFlush(*this);
    }


    /** True if the stream can be read from */
    virtual bool CanRead(){
        return False;
    }

    /** True if the stream can be written to */
    virtual bool CanWrite(){
        return True;
    }

protected:

    /** Not implemented */
    virtual bool        SSRead(
                            void*               buffer,
                            uint32 &            size,
                            TimeoutType         msecTimeout     = TTDefault){
        return False;
    }

    /** Used by the Streamable::Write
        It converts no HTML characters into equivalent html codes
        It support multistreams  */
    virtual bool        SSWrite(
                            const void*         buffer,
                            uint32 &            size,
                            TimeoutType         msecTimeout     = TTDefault)
    {
        return HSWWrite(*this, buffer, size);
    }

public:
    /** how many streams are available
        replace with a different value if multiple stream are supported */
    virtual uint32 NOfStreams(){
        return 7;
    }

    /** Use to change the console attributes. Valid names are
        "colour"  R/W fg and bg colours,    two integers first is the fg second the bg
     to switch back to console use Switch (ConsoleStreamMode) */
    virtual bool Switch(const char *name){
        HSWFlush(*this);
        selectedStream = SANameToStreamModes(name);
        if (selectedStream == NullSelectedStream) return False;
        return True;
    }

    /** Use to change the console attributes. Valid names are
        "colour"  R/W fg and bg colours,    two integers first is the fg second the bg
     to switch back to console use Switch (ConsoleStreamMode) */
    virtual bool Switch(uint32 selectedStream){
        HSWFlush(*this);
        this->selectedStream = selectedStream;
        return True;
    }
};

#endif
