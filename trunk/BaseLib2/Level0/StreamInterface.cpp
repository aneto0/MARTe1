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

#include "StreamInterface.h"

#if defined(_VXWORKS)

/** Supported format flags %<pad><size><type>
    type can be o d i X x Lo Ld Li LX Lx f e s c   */
bool StreamInterface::Printf(const char *format,...){
    if (format==NULL) return False;

    va_list argList;
    va_start(argList,format);
    bool ret = VPrintf(format,argList);
    va_end(argList);

    return ret;
}

/** selects a stream and writes to it, then returns to previous
    buffer is a zero terminated string  */
bool StreamInterface::SSPrintf(uint32 streamNo,const char *format,...){
    if (format==NULL) return False;

    uint32 save = SelectedStream();
    bool ret = False;
    if (Switch(streamNo)){
        va_list argList;
        va_start(argList,format);
        ret = VPrintf(format,argList);
        va_end(argList);
    }
    Switch(save);
    return ret;
}


bool StreamInterface::SSPrintf(const char *streamName,const char *format,...){
    if (format==NULL) return False;

    uint32 save = SelectedStream();
    bool ret = False;

    AddStream(streamName);
    if (Switch(streamName)){
	va_list argList;
	va_start(argList,format);
	ret = VPrintf(format,argList);
	va_end(argList);
    }
    Switch(save);
    return ret;
}


#endif
