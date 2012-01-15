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
 */
#ifndef CDBOSSTREAM_H_
#define CDBOSSTREAM_H_

#include "System.h"
#include "GCNamedObject.h"
#include "Streamable.h"
#include "BasicTypes.h"
#include "SegmentedName.h"
#include "MutexSem.h"
#include "ErrorManagement.h"

class CDBOSStream : public GCNamedObject {
private:
    /** Separator used for path */
    const char* separator;
    /** Internal pointer to the Streamable to write to */
    Streamable* stream;
    /** Current path in the CDB */
    SegmentedName path;
    /** Current level in the hierarchy */
    int32 level;
    /** EventSem to protect operations */
    MutexSem mux;

    /** Find difference between two CDB addresses */
    bool FindAddressMismatch(const SegmentedName &newPath, int &levelsUp, SegmentedName &levelsDown) {
        if (stream==NULL) return False;
        int32 commonPathLength = 0;
        int32 oldAddressLength = path.NumberOfSegments();
        int32 newAddressLength = newPath.NumberOfSegments();
        int32 minLength;
        if (oldAddressLength < newAddressLength) minLength = oldAddressLength;
        else minLength = newAddressLength;

        // Find length of common path
        if (level!=0) {
            while ( (commonPathLength < minLength) && (strcmp(path[commonPathLength],newPath[commonPathLength])==0 ) ) {
                commonPathLength++;
            }
        }

        // Move up n levels
        int32 lup = oldAddressLength - commonPathLength;
        if (lup > 1) levelsUp = lup;
        else levelsUp = 0;

        // Move down n levels
        for (int i=commonPathLength; i < newAddressLength; i++){
            levelsDown.AddSegment(newPath[i],strlen(newPath[i]),0);
        }
        return True;
    }
    /** Print spaces to respect hierarchy */
    void PrintSpaces() {
        if (stream==NULL) return;
        for (int i = 0; i < level; i++) {
            stream->Printf("    ");
        }
    }

    /** Move n levels down the structure */
    bool MoveDown (const SegmentedName &levelsDown) {
        for (int i=0; i < levelsDown.NumberOfSegments(); i++){
            path.AddSegment(levelsDown[i],strlen(levelsDown[i]),0); //TODO: check
            PrintSpaces();
            stream->Printf("%s = {\n", levelsDown[i]);
            level++;
        }
        return True;
    }
    /** Move n levels up the structure */
    bool MoveUp (int numLevels) {
        if (stream==NULL) {
            return False;
        }
        if ( (level-numLevels) < 0) {
            return False;
        }
        for (int i=0; i < numLevels; i++) {
            path.RemoveLastSegment();
            level--;
            PrintSpaces();
            stream->Printf("}\n");
        }
        return True;
    }

public:
    /** Constructor */
    CDBOSStream(): separator("/"), path("",&separator){
        stream = NULL;
        level = 0;
        mux.Create();
    }
    /** Constructor */
    CDBOSStream(Streamable* CDBstream) : separator("/"),path("",&separator) {
        stream = CDBstream;
        level = 0;
        mux.Create();
    }
    /** Destructor */
    ~CDBOSStream() {
        bool ret = Close();
        stream = NULL;
        mux.Close();
    }

    /** Set the stream */
    void SetStream(Streamable* s) {
        stream = s;
    }

    /** Function to output one value at the current address */
    bool Write (const char* configName, int numOfElements, const void* value, BasicTypeDescriptor type, const SegmentedName &newPath) {
        if (stream == NULL) {
            CStaticAssertErrorCondition(FatalError, "CDBOSStream::Write failed, stream is NULL");
            return False;
        }
        mux.Lock();
        if (!Relocate(newPath)) {
            CStaticAssertErrorCondition(FatalError, "CDBOSStream::Write Relocate failed");
            mux.UnLock();
            return False;
        }
        PrintSpaces();

        if (numOfElements < 2) {
            stream->Printf("%s = ", configName);
        } else {
            stream->Printf("%s = {\n", configName);
        }


        BTDFormat format;
        format.stringInfo.separator = 3;
        BasicTypeDescriptor MyBTDStream (0,BTDTString,BTDSTStream,format);

        BTConvert(numOfElements, MyBTDStream, (void*)stream, type, value);
        if (numOfElements < 2) {
            stream->Printf("\n");
        } else {
            stream->Printf("\n}\n");
        }
        mux.UnLock();

        return True;
    }

    /** Function to move in the CDB and update internal path */
    bool Relocate (const SegmentedName &newPath) {
        int levelsUp;
        SegmentedName levelsDown;

        mux.Lock();
        if (!FindAddressMismatch(newPath, levelsUp, levelsDown)) {
            CStaticAssertErrorCondition(FatalError, "CDBOSStream::Relocate failed FindAddressMismatch, stream is NULL");
            mux.UnLock();
            return False;
        }

        if (!MoveUp(levelsUp)) {
            CStaticAssertErrorCondition(FatalError, "CDBOSStream::Relocate failed MoveUp in CDB structure, too many levels up or stream is NULL");
            mux.UnLock();
            return False;
        }
        if (!MoveDown(levelsDown)) {
            CStaticAssertErrorCondition(FatalError, "CDBOSStream::Relocate failed MoveDown in CDB structure, stream is NULL");
            mux.UnLock();
            return False;
        }
        mux.UnLock();
        return True;
    }

    /** Function to close the parenthesis opened till now */
    bool Close () {
        mux.Lock();
        return MoveUp(level);
        mux.UnLock();
    }

    /** Lock the stram's sem */
    void Lock() {
        mux.Lock();
    }

    /** UnLock the stram's sem */
    void UnLock() {
        mux.UnLock();
    }

};

//const char* separator = "/";

#endif /*CDBOSSTREAM_H_*/
