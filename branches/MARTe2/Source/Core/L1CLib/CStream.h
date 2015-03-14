/*
 * Copyright 2015 F4E | European Joint Undertaking for
 * ITER and the Development of Fusion Energy ('Fusion for Energy')
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
 * See the Licence
 permissions and limitations under the Licence.
 *
 * $Id: $
 *
 **/
/** 
 * @file 
 * A portable C stream mechanism. 
 * It provides the base infrastructure for the most advanced streaming classes
 * (e.g. File, Socket and FString) to do printf like operations
 */
#if !defined (CSTREAM_H)
#define CSTREAM_H

#include "GeneralDefinitions.h"
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

struct CStream;

typedef void (*CStreamNewBufferFN)(CStream *p);

struct CStream {

    void * context;

    /** Pointer to the buffer for read/write ops. */
    char * bufferPtr;

    uint32 sizeLeft;

    CStreamNewBufferFN NewBuffer;
};

static inline bool CPut(CStream * p, char c) {
    if (p->sizeLeft == 0) {
        p->NewBuffer(p);
        if (p->sizeLeft == 0)
            return False;
    }
    p->sizeLeft--;
    p->bufferPtr[0] = c;
    (p->bufferPtr)++;
    return True;
}

static inline bool CGet(CStream * p, char & c) {
    if (p->sizeLeft == 0) {
        p->NewBuffer(p);
        if (p->sizeLeft == 0)
            return False;
    }
    p->sizeLeft--;
    c = *(p->bufferPtr)++;
    return True;
}

extern "C" {
bool CRead(CStream *cs, void *buffer, uint32 &size);

bool CWrite(CStream *cs, const void *buffer, uint32 &size);

bool CPrintInt32(CStream *cs, int32 n, uint32 desiredSize = 0,
                 char desiredPadding = 0, char mode = 'i');

bool CPrintInt64(CStream *cs, int64 n, uint32 desiredSize = 0,
                 char desiredPadding = 0, char mode = 'i');

bool CPrintDouble(CStream *cs, double ff, int desiredSize = 0,
                  int desiredSubSize = 6, char desiredPadding = 0, char mode =
                          'f');

bool CPrintString(CStream *cs, const char *s, uint32 desiredSize = 0,
                  char desiredPadding = 0, bool rightJustify = True);

/** Supported format flags %<pad><size><type>
 type can be o d i X x Lo Ld Li LX Lx f e s c
 */
bool VCPrintf(CStream *cs, const char *format, va_list argList);

bool CPrintf(CStream *cs, const char *format, ...);

bool CGetToken(CStream *cs, char *buffer, const char *terminator,
               uint32 maxSize, char *saveTerminator = NULL, const char *skip =
               NULL);

bool CGetStringToken(CStream *cs, CStream *out, const char *terminator,
                     uint32 totalSize);

bool CSkipTokens(CStream *cs, uint32 count, const char *terminator);

bool CGetCStringToken(const char *&input, char *buffer, const char *terminator,
                      uint32 maxSize);

char *CDestructiveGetCStringToken(char *&input, const char *terminator,
                                  char *saveTerminator = NULL,
                                  const char *skip = "");

bool CGetCSToken(CStream *csIn, CStream *csOut, const char *terminator,
                 char *saveTerminator = NULL, const char *skip = NULL);

/** BaseLib2 replacement of vsprintf */
int bl2_vsprintf(char *buffer, const char *format, va_list argList);

/** BaseLib2 replacement of sprintf */
int bl2_sprintf(char *buffer, const char *format, ...);

/** BaseLib2 replacement of vsnprintf */
int bl2_vsnprintf(char *buffer, size_t size, const char *format,
                  va_list argList);

/** BaseLib2 replacement of snprintf */
int bl2_snprintf(char *buffer, size_t size, const char *format, ...);
}

#endif

