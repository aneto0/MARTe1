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


#include "BString.h"
#include "System.h"
#include "Memory.h"


const int32 StringGranularity     = 0x00000020;
const int32 StringGranularityMask = 0xFFFFFFE0;

bool FSAlloc(BString &s,int32 allocSize){
    allocSize++; // +1 to accomodate string terminator
    int32 deltaSize = allocSize - s.allocatedSize;
    if ((deltaSize <=0) && (deltaSize >= -10*StringGranularity)) return True;

    // at least a granule
    int32 newSize = allocSize;
    newSize += StringGranularity - 1 ;
    newSize &= StringGranularityMask;

    char *newBuffer;
    if (s.buffer == NULL) {
        newBuffer = (char *) malloc(newSize);
        newBuffer[0] = 0;
    } else {
        newBuffer = (char *) realloc((void * &)s.buffer,newSize);
    }
    if (newBuffer == NULL)  return False;
    s.buffer        = newBuffer;
    s.allocatedSize = newSize;
    return True;
}

bool FSRead(BString &s,void* buffer, uint32 &size){
    // adjust position
    if (s.position > s.Size()) s.position = s.Size();  // append
    //
    uint32 sizeLeft = s.Size() - s.position;
    if(sizeLeft == 0){
        size = 0;
        return False;
    }
    if (size > sizeLeft) size = sizeLeft;
    memcpy(buffer,&(s.buffer[s.position]),size);
    s.position += size;
    return True;
}

bool FSWrite(BString &s,const void* buffer, uint32 &size){
    // adjust position
    if (s.position > s.Size()) s.position = s.Size();  // append
    // adjust size
    uint32 neededSize = s.position + size;
    if ((neededSize+1) > s.allocatedSize)FSAlloc(s,neededSize);
//    s.SetSize(neededSize);

    // just in case check if that worked
    uint32 sizeLeft = s.allocatedSize - s.position -1;
    if (size > sizeLeft) size = sizeLeft;
    memcpy(&(s.buffer[s.position]),buffer,size);

    s.position += size;
    if (s.size < s.position) s.size = s.position;
    s.buffer[s.position] = 0;

    return True;
}

#if  defined(_VXWORKS)
// Implementation of missing vsnprintf

/*  A container of the following type is allocated from the stack
    on entry to vsnprintf(), initialized with details about the
    caller's output buffer, and passed to fioFormatV() for use by
    sn_output_fn(). */
struct PrintData {
    char *  s;          /* The user's output buffer */
    int     nmax;       /* The length of the user's output buffer */
    int     length;     /* The current length of the string in s[] */
} ;

static STATUS sn_output_fn(char *buffer, int nc, int arg);

/*  I don't have access to an implementation of snprintf() so I am guessing
  at the order and types of its arguments.  */
int snprintf(char *s, size_t n, const char *fmt, ...)
{
    int nc;      /* The number of characters succesfully output (-1 on error) */
    va_list ap;  /* The variable argument list */
    va_start(ap, fmt);
    nc = vsnprintf(s, n, fmt, ap);
    va_end(ap);
    return nc;
}

int vsnprintf(char *s, size_t n, const char *fmt, va_list ap)
{
    PrintData data;  /* A container for details about the output buffer */

/*  Record details about the output buffer in 'data'. This can
    then be passed to the fioFormatV() output function.  */
    data.s = s;
    data.nmax = n;
    data.length = 0;

/*  Format output into data.s[] and record the accumulated length
    of the string in data.length. */
    if (fioFormatV(fmt, ap, (FUNCPTR)sn_output_fn, (int) &data) == ERROR)
    {
        return ERROR;
    }

/*  Terminate the accumulated string.
    Note that sn_output_fn() guarantees that data.length is < data.nmax. */
    data.s[data.length] = '\0';

/*  Return a count of the number of characters in s[], excluding the
    terminating '\0'. Note that this will be less than the number
    expected, if the output had to be truncated. */
    return data.length;
}

/*  This is the function that fioFormatV() calls to render output into
    the snprintf() caller's output buffer.  */
static STATUS sn_output_fn(char *buffer, int nc, int arg)
{
    PrintData *data = (PrintData *) arg;

/*  Work out how many of the nc characters in buffer[] can be appended
    to data->s[]. Leave room for a '\0' terminator at the end of
    data->s[]. */
    int nnew = nc;
    if (data->length + nnew + 1 >= data->nmax){
        nnew = data->nmax - data->length - 1;
    }

/*  Append nnew characters from buffer[] to data.s[].*/
    if (nnew > 0) {
        memcpy(data->s + data->length, buffer, nnew);
        data->length += nnew;
    }
    return OK;
}

#endif
