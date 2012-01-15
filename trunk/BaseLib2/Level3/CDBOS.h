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

#if !defined(CDBOS_H)
#define CDBOS_H

#include "System.h"
#include "CDBVirtual.h"
#include "GCRTemplate.h"
#include "ErrorManagement.h"
#include "CDBOSStream.h"
#include "SegmentedName.h"

/**
 * @file
 * A class to implement direct writing of cdb to stream
 */
OBJECT_DLL(CDBOS)

class CDBOS: public CDBVirtual{

OBJECT_DLL_STUFF(CDBOS)

private:
    /** */
    GCRTemplate<CDBOSStream> stream;

    /** */
    SegmentedName path;

    /** */
    const char* separator;

    /** */
    int32 level;

public:

    /** Constructor */
    CDBOS(GCRTemplate<CDBOSStream> cdbosstream) : separator("/"), path("",&separator) {
        level = 0;
        stream = cdbosstream;
    }

    /** Constructor */
    CDBOS() : stream(GCFT_Create), separator("/"), path("",&separator) {
        level = 0;
    }


    /** Deconstructor */
    virtual ~CDBOS() {
    }

    /** creates a new reference to a database, or if that is not possible it creates a copy
        @param cdbcm = CDBCM_CopyAddress ensures that the new object points at the same location  */
    CDBVirtual *Clone(CDBCreationMode cdbcm){
        return new CDBOS(stream);
    }

    /** locks the main database for exclusive access: use if a group of transactions should be atomic */
    virtual bool Lock() {
        stream->Lock();
        return True;
    }

    /** unlocks the main database: rememver unlocking a locked database after use */
    virtual void UnLock() {
        stream->UnLock();
    }

    /** the function can be used to create a new subtree */
    virtual bool AddChildAndMove(const char *subTreeName,SortFilterFn *sorter=NULL) {
        if (sorter != NULL) {
            AssertErrorCondition(FatalError, "CDBOS::AddChildAndMove only works with SortFilterFn=NULL");
            return False;
        }
        if (subTreeName == NULL) {
            AssertErrorCondition(FatalError, "CDBOS::AddChildAndMove subTreeName==NULL");
            return False;
        }
        path.AddSegment(subTreeName,strlen(subTreeName),0);
        if (!stream->Relocate(path)){
            AssertErrorCondition(FatalError, "CDBOS::AddChildAndMove failed to Relocate to %s", subTreeName);
            return False;
        }
        level++;
        return True;
    }


    /** -1 = Root, one level up for each positive */
    virtual bool MoveToFather(int steps = 1) {
        if ( (steps < -1) || ((level-steps)<0) ) return False;

        if (steps == -1) {
            for (int i=0; i<level; i++) path.RemoveLastSegment();
            level = 0;
            return stream->Close();
        } else {
            for (int i=0; i<steps; i++) path.RemoveLastSegment();
            level -= steps;
            return stream->Relocate(path);
        }
    }

    /** if size == NULL it treats the input as a monodimensional array of size nDim */
    virtual bool WriteArray(const void *array,const CDBTYPE &valueType,const int *size,int nDim,const char *configName,SortFilterFn *sorter=NULL){
        int32 totalDim = 0;


        if (sorter != NULL) {
            AssertErrorCondition(FatalError, "CDBOS::WriteArray only works with SortFilterFn=NULL");
            return False;
        }
        if (array == NULL) {
            AssertErrorCondition(FatalError, "CDBOS::WriteArray size or array=NULL");
            return False;
        }
        
        if (size != NULL) {
            AssertErrorCondition(Warning, "CDBOS::WriteArray correct output of matrices still to be implemented, matrix will be printed as an array");
            for (int i=0;i<nDim;i++) {
                totalDim += size[i];
            }
        } else {
            totalDim = nDim;
        }

        if(valueType.dataType == CDB_String){
            totalDim = 1;
        }

        return stream->Write(configName, totalDim,array,valueType.dataType,path);
    }

    /** Used to assign the stream to write to */
    virtual bool WriteToStream(StreamInterface &streamout,StreamInterface *err=NULL,CDBWriteMode mode=CDBWM_Tree){
        Streamable* s = dynamic_cast<Streamable*>(&streamout);
        if (s==NULL) {
            AssertErrorCondition(FatalError, "CDBOS::WriteToStream cannot cast streamout to Streamable");
            return False;
        }
        stream->SetStream(s);
        return True;
    }

    /////////////////////////////////////////////
    //            UNUSED FUNCTIONS             //
    /////////////////////////////////////////////

    /** */
    virtual bool MoveToChildren(int childNumber=0) {
        AssertErrorCondition(FatalError, "CDBOS::MoveToChildren not implementable!");
        return False;
    }

    /** */
    virtual int  NumberOfChildren() {
        AssertErrorCondition(FatalError, "CDBOS::NumberOfChildren not implementable!");
        return -1;
    }

    /** */
    virtual bool MoveToBrother(int steps = 1) {
        AssertErrorCondition(FatalError, "CDBOS::MoveToBrother not implementable!");
        return False;
    }

    /** */
    virtual bool FindSubTree(const char *configName,CDBAddressMode cdbam = CDBAM_None) {
        AssertErrorCondition(FatalError, "CDBOS::FindSubTree not implementable!");
        return False;
    }

    /** */
    virtual int  Size(CDBAddressMode cdbam = CDBAM_SubTreeOnly | CDBAM_LeafsOnly) {
        AssertErrorCondition(FatalError, "CDBOS::Size not implementable!");
        return 0;
    }

    /** */
    virtual bool GetArrayDims(int *size,int &maxDim,const char *configName,CDBArrayIndexingMode cdbaim = CDBAIM_Flexible, bool caseSensitive = True) {
        AssertErrorCondition(FatalError, "CDBOS::GetArrayDims not implementable!");
        return False;
    }

    /** */
    virtual bool ReadArray (void *array,const CDBTYPE &valueType,const int *size,int nDim,const char *configName, bool caseSensitive = True) {
        AssertErrorCondition(FatalError, "CDBOS::ReadArray not implementable!");
        return False;
    }

    /** */
    virtual bool Delete(const char *configName) {
        AssertErrorCondition(FatalError, "CDBOS::Delete not implementable!");
        return False;
    }

    /** */
    virtual bool Link(const char *linkFrom,const char *linkTo,SortFilterFn *sorter=NULL) {
        AssertErrorCondition(FatalError, "CDBOS::Link not implementable!");
        return False;
    }

    /** */
    virtual bool CopyFrom(CDBVirtual *cdbv){
        AssertErrorCondition(FatalError, "CDBOS::CopyFrom not implementable!");
        return False;
    }

    /** */
    virtual bool LoadFromEnvironment(char **env) {
        AssertErrorCondition(FatalError, "CDBOS::LoadFromEnvironment not implementable!");
        return False;
    }

    /** */
    virtual bool ReadStructure (const char *className,char *address,Streamable *err=NULL) {
        AssertErrorCondition(FatalError, "CDBOS::ReadStructure not implementable!");
        return False;
    }

    /** */
    virtual void EnableParserReports(bool flag) {
        AssertErrorCondition(FatalError, "CDBOS::EnableParserReports not implementable!");
    }

    /** */
    virtual void CleanUp(CDBAddressMode cdbam = CDBAM_FromRoot) {
        AssertErrorCondition(FatalError, "CDBOS::CleanUp not implementable!");
    }

    /** */
    virtual bool ReadFromStream(StreamInterface &stream,StreamInterface *err=NULL,SortFilterFn *sorter=NULL) {
        AssertErrorCondition(FatalError, "CDBOS::ReadFromStream not implementable!");
        return False;
    }

    /** */
    virtual bool Exists(const char *configName) {
        AssertErrorCondition(FatalError, "CDBOS::Exists not implementable!");
        return False;
    }

    /** */
    virtual bool Move(const char *subTreeName) {
        AssertErrorCondition(FatalError, "CDBOS::Move not implementable!");
        return False;
    }

    /** */
    virtual bool SubTreeName(Streamable &name,const char *sep = ".") {
        AssertErrorCondition(FatalError, "CDBOS::SubTreeName not implementable!");
        return False;
    }

    /** */
    virtual bool NodeName(BString &name) {
        AssertErrorCondition(FatalError, "CDBOS::NodeName not implementable!");
        return False;
    }

    /** */
    virtual bool NodeType(BString &name) {
        AssertErrorCondition(FatalError, "CDBOS::NodeType not implementable!");
        return False;
    }

    /** */
    virtual int  TreePosition(CDBAddressMode cdbam = CDBAM_LeafsOnly) {
        AssertErrorCondition(FatalError, "CDBOS::TreePosition not implementable!");
        return False;
    }

    /** */
    virtual bool TreeMove(int index,CDBAddressMode cdbam = CDBAM_LeafsOnly) {
        AssertErrorCondition(FatalError, "CDBOS::TreeMove not implementable!");
        return False;
    }

    // TO BE IMPLEMENTED

    /** */
    virtual bool WriteStructure(const char *className,char *address,const char *variableName=NULL,Streamable *err=NULL) {
        AssertErrorCondition(FatalError, "CDBOS::WriteStructure currently not implemented!");
        return False;
    }
};

#endif
