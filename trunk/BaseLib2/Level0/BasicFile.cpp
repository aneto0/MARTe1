/*
 * Copyright 2011 EFDA | European Fusion Development Agreement
 *
 * Licensed under the EUPL, Version 1.1 or – as soon they 
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

#include "BasicFile.h"
#include "ErrorManagement.h"

void FileSetFileName(BasicFile &f,const char *name){
    if (f.fileName != NULL) free((void *&)f.fileName);
    f.fileName = NULL;
    if (name!=NULL) f.fileName = strdup((char *)name);
}

bool FileRead(BasicFile &f,void* buffer, uint32 &size,TimeoutType msecTimeout){
#if (defined (_RTAI)|| defined(_LINUX) || defined(_SOLARIS) || defined(_MACOSX))
    uint32 ret = read(f.file, (void *)buffer, size);
    if (ret<0) {
        f.action = OSError;
        CStaticAssertPlatformErrorCondition(OSError,"FileRead");
    }
    else
        size=ret;
    return (ret>0);
#elif defined(_OS2)
    unsigned long actual;
    APIRET rc;
    rc = DosRead(f.file,(void *)buffer,size,&actual);
    if (rc!=0)  {
        f.action = OSError;
        CStaticAssertPlatformErrorCondition(OSError,rc,"FileRead");
    }
    size = actual;
    // reading 0 means eof!
    if (size==0) return False;
    return (rc==0);
#elif defined(_WIN32)
    DWORD actual;
    BOOL flag = ReadFile(f.file,(void *)buffer,size,&actual,NULL);
    if (flag==FALSE) {
        CStaticAssertPlatformErrorCondition(OSError,"FileRead");
        f.action = OSError;
    }
    size = actual;
    // reading 0 means eof!
    if (size==0) return False;
    return (flag!=FALSE);
#elif defined(_VXWORKS)
    int r = read(f.file,(char *)buffer,size);
    if (r<0)  {
        f.action = OSError;
        CStaticAssertPlatformErrorCondition(OSError,"FileRead");
    }
    else size = r;
    return (r>0);
#endif
}


bool FileWrite(BasicFile &f,const void* buffer, uint32 &size,TimeoutType msecTimeout){
#if (defined(_RTAI)|| defined(_LINUX) || defined(_SOLARIS) || defined(_MACOSX))
    int ret = write(f.file, (void *)buffer, size);
    if (ret<0) {
        f.action = OSError;
        CStaticAssertPlatformErrorCondition(OSError,"FileWrite");
    }
    else
        size=ret;
    return (ret>=0);

#elif defined(_OS2)
    unsigned long actual;
    APIRET rc = DosWrite(f.file,(void *)buffer,size,&actual);
    if (rc!=0){
        f.action = OSError;
        CStaticAssertPlatformErrorCondition(OSError,rc,"FileWrite");
    }
    size = actual;
    return (rc==0);
#elif defined(_WIN32)
    DWORD actual;
    BOOL flag = WriteFile(f.file,buffer,size,&actual,NULL);
    if (flag==FALSE) {
        f.action = OSError;
        CStaticAssertPlatformErrorCondition(OSError,"FileWrite");
    }
    size = actual;
    return (flag!=FALSE);
#elif defined(_VXWORKS)
    int r = write(f.file,(char *)buffer,size);
    if (r<0){
        f.action = OSError;
        CStaticAssertPlatformErrorCondition(OSError,"FileWrite");
    }
    else size = r;
    return (r>=0);
#endif
}

int64 FileSize(BasicFile &f){
#if (defined(_RTAI) || defined(_LINUX)  || defined(_SOLARIS) || defined(_MACOSX))
    struct stat statistiche;
    int ret;
    int64 size;
    statistiche.st_size = 0;
    ret = fstat(f.file, &statistiche);
    if(ret < 0)
        return 0;
    
    size=(int64)statistiche.st_size;
    return size;
#elif defined(_OS2)
    FILESTATUS3 finfo;
    DosQueryFileInfo(f.file,FIL_STANDARD,(PVOID)&finfo,sizeof(finfo));
    return  finfo.cbFile;  // as longs
#elif defined(_WIN32)
    return GetFileSize(f.file,NULL);
#elif defined(_VXWORKS)
    int save = lseek(f.file,0,SEEK_CUR);
    int sz = lseek(f.file,0,SEEK_END);
    lseek(f.file,save,SEEK_SET);
    if (sz==ERROR)  {
        f.action = OSError;
        CStaticAssertPlatformErrorCondition(OSError,"FileSize");
    }
    return sz;
#endif
}

bool FileSeek(BasicFile &f,int64 pos){

    uint32 position = pos;
    uint32 newpos;
#if defined(_OS2)
    DosSetFilePtr(f.file,position,FILE_BEGIN,&newpos);
#elif defined(_WIN32)
    newpos = SetFilePointer(f.file, position,NULL,FILE_BEGIN);
#elif defined(_VXWORKS)
    newpos = lseek(f.file,position,SEEK_SET);
    if (newpos==(uint32)ERROR){
        f.action = OSError;
        CStaticAssertPlatformErrorCondition(OSError,"BasicFile::Seek");
    }
#elif (defined(_RTAI)|| defined(_LINUX) || defined(_SOLARIS) || defined(_MACOSX))
    newpos = lseek(f.file, position, SEEK_SET);
    if (newpos<0) {
        f.action = OSError;
        CStaticAssertPlatformErrorCondition(OSError,"BasicFile::Seek");
    }
#endif
    if (pos != newpos){
        pos = newpos;
        return False;
    } else {
        return True;
    }
}

int64 FilePosition(BasicFile &f){
#if defined(_OS2)
    ULONG position;
    DosSetFilePtr(f.file,0,FILE_CURRENT,(PULONG)&position);
    return  position;  // as longs
#elif defined(_WIN32)
    return SetFilePointer(f.file, 0,NULL,FILE_CURRENT);
#elif defined(_VXWORKS)
    int sz = lseek(f.file,0,SEEK_CUR);
    if (sz==ERROR){
        f.action = OSError;
        CStaticAssertPlatformErrorCondition(OSError,"FilePosition");
    }
    return sz;
#elif (defined(_RTAI)|| defined(_LINUX)  || defined(_SOLARIS) || defined(_MACOSX))
    int sz = lseek(f.file,0,SEEK_CUR);
    if (sz<0) {
        f.action = OSError;
        CStaticAssertPlatformErrorCondition(OSError,"FilePosition");
    }
    return sz;
#endif
}

/// Clip the file size to a specified point
bool FileSetSize(BasicFile &f,int64 size){
#if defined(_OS2)
    APIRET ret = DosSetFileSize(f.file,size);
    if (ret!=0) {
        CStaticAssertPlatformErrorCondition(ret,"FileSetSize failed");
        return False;
    }
    return True;

#elif defined(_WIN32)
    int64 position = FilePosition(f);
    SetFilePointer(f.file, size,NULL,FILE_BEGIN);
    BOOL ret = SetEndOfFile(f.file);
    if (position < size) SetFilePointer(f.file, position,NULL,FILE_BEGIN);
    return (ret == TRUE);
#elif  (defined(_SOLARIS ) || defined(_RTAI) || defined(_LINUX) || defined(_MACOSX))
    int ret = ftruncate(f.file,size);
    return (ret == 0);
#elif defined(_VXWORKS)
    return False;
#endif

}

/// Moves upwards or backwards so many bytes
bool  FileRelativeSeek(BasicFile &f,int64 pos){
    uint32 position = pos;
#if defined(_OS2)
    unsigned long newpos;
    DosSetFilePtr(f.file,position,FILE_CURRENT,&newpos);
#elif defined(_WIN32)
    unsigned long newpos;
    newpos = SetFilePointer(f.file, position,NULL,FILE_CURRENT);
#elif defined(_VXWORKS)
    int newpos = lseek(f.file,position,SEEK_CUR);
    if (newpos==ERROR){
        f.action = OSError;
        CStaticAssertPlatformErrorCondition(OSError,"BasicFile::RelativeSeek");
    }
    return True;
#elif (defined(_RTAI)|| defined(_LINUX)  || defined(_SOLARIS) || defined(_MACOSX))
    int newpos = lseek(f.file, position,SEEK_CUR);
    if (newpos<0) {
        f.action = OSError;
        CStaticAssertPlatformErrorCondition(OSError,"BasicFile::RelativeSeek");
    }
    return True;
#endif
    if (pos != newpos){
        pos = newpos;
        return False;
    } else {
        return True;
    }
}

/// Moves to position pos and returns new position on pos
bool  FilePositionSeek(BasicFile &f,int64 &pos){
#if defined(_OS2)
    ULONG posz;
    APIRET ret;
    ret = DosSetFilePtr(f.file,pos,FILE_BEGIN,(PULONG)&posz);
    if (ret==0) pos = posz;
    return  (ret==0);  // as longs
#elif defined(_WIN32)
    uint32 posz = SetFilePointer(f.file, pos,NULL,FILE_BEGIN);
    if (posz!=0xFFFFFFFF) {
        pos = posz;
        return True;
    }
    return False;
#elif defined(_VXWORKS)
    int p = pos;
    int pp = lseek(f.file,p,SEEK_SET);
    if (pp==ERROR)  {
        f.action = OSError;
        CStaticAssertPlatformErrorCondition(OSError,"BasicFile::PositionSeek");
        return False;
    }
    pos = pp;
    return True;
#elif (defined(_RTAI)|| defined(_LINUX) || defined(_SOLARIS) || defined(_MACOSX))
    int p = pos;
    int pp = lseek(f.file,p,SEEK_SET);
    if (pp<0)  {
        f.action = OSError;
        CStaticAssertPlatformErrorCondition(OSError,"BasicFile::PositionSeek");
        return False;
    }
    pos = pp;
    return True;
#endif
}

bool FileOpen(BasicFile &f,const char *name){
    f.SetFileName(name);
#if defined(_SOLARIS)|| defined(_LINUX) || defined (_RTAI) || defined(_MACOSX)
    int flags = 0;
    int mode = 0777;

    uint32 tmp = f.fileMode & openingFlagsMask1;
    tmp = (tmp | (openingFlagsMask1 << 16));
    switch (tmp) {
        case createNewFile   : flags |= O_CREAT; break;
        case createOverwrite : flags |= O_CREAT; break;
    }

    tmp = f.fileMode & openingModeMask3;
    tmp = tmp | (openingModeMask3 << 16);
        
    switch (tmp) {
        case accessModeR  : flags |= O_RDONLY; break;
        case accessModeW  : flags |= O_WRONLY; break;
        case accessModeRW : flags |= O_RDWR;   break;
    }

    bool openDevDriver = f.fileMode & (devDriverMode & 0xFFFF);
    f.file = open (name, flags, mode);
    if (f.file != -1) {        
        if(openDevDriver){
            return True;
        }
        
        struct stat statistiche;        
        int ret = fstat(f.file, &statistiche);        
        if(ret < 0 || !S_ISREG(statistiche.st_mode)){
            close(f.file);
            return False;
        }
        
        if (flags & O_CREAT) {
            f.action = openWasCreate;
            f.SetSize(0);
        } else {
            f.action = openWasOpen;
        }
        return True;
    }

    if ((flags & O_CREAT) == 0){

        tmp = f.fileMode & openingFlagsMask1;
        tmp = (tmp | (openingFlagsMask1 << 16));
        switch (tmp){
            case openCreate : flags |= O_CREAT; break;
        }

        f.file = open(name,flags,mode);
        if (f.file != -1) {
            struct stat statistiche;        
            int ret = fstat(f.file, &statistiche);            
            if(ret < 0 || !S_ISREG(statistiche.st_mode)){
                close(f.file);
                return False;
            }
            f.action = openWasCreate;
            return True;
        }
    }

    int32 errCode = OSError;
    switch (errCode){
        case  EACCES:{
            errCode = ErrorAccessDenied;
        } break;
    }
    return False;
#elif defined(_OS2)
    uint32 openingFlags = 0;
    uint32 tmp = f.fileMode & openingFlagsMask1;
    switch (tmp | (openingFlagsMask1 << 16)){
        case createNewFile     : openingFlags |= (OPEN_ACTION_FAIL_IF_EXISTS    | OPEN_ACTION_CREATE_IF_NEW)  ; break;
        case createOverwrite   : openingFlags |= (OPEN_ACTION_REPLACE_IF_EXISTS | OPEN_ACTION_CREATE_IF_NEW)  ; break;
        case openFile          : openingFlags |= (OPEN_ACTION_OPEN_IF_EXISTS    | OPEN_ACTION_FAIL_IF_NEW)    ; break;
        case openCreate        : openingFlags |= (OPEN_ACTION_OPEN_IF_EXISTS    | OPEN_ACTION_CREATE_IF_NEW)  ; break;
    }
    uint32 openingMode = OPEN_FLAGS_FAIL_ON_ERROR;
    tmp = f.fileMode & openingModeMask1;
    switch (tmp | (openingModeMask1 << 16)){
        case localityNone       : openingMode |= OPEN_FLAGS_NO_LOCALITY      ; break;
        case localitySequential : openingMode |= OPEN_FLAGS_SEQUENTIAL       ; break;
        case localityRandom     : openingMode |= OPEN_FLAGS_RANDOM           ; break;
        case localityMixed      : openingMode |= OPEN_FLAGS_RANDOMSEQUENTIAL ; break;
    }
    tmp = f.fileMode & openingModeMask2;
    switch (tmp | (openingModeMask2 << 16)){
        case shareModeNoRW      : openingMode |= OPEN_SHARE_DENYREADWRITE    ; break;
        case shareModeNoW       : openingMode |= OPEN_SHARE_DENYWRITE        ; break;
        case shareModeNoR       : openingMode |= OPEN_SHARE_DENYREAD         ; break;
        case shareModeAll       : openingMode |= OPEN_SHARE_DENYNONE         ; break;
    }
    tmp = f.fileMode & openingModeMask3;
    switch (tmp | (openingModeMask3 << 16)){
        case accessModeR        : openingMode |= OPEN_ACCESS_READONLY        ; break;
        case accessModeW        : openingMode |= OPEN_ACCESS_WRITEONLY       ; break;
        case accessModeRW       : openingMode |= OPEN_ACCESS_READWRITE       ; break;
        default: return False;
    }

    ULONG       _action;
    APIRET ret = DosOpen((PSZ)name,&f.file,&_action,0,FILE_NORMAL,openingFlags,openingMode,NULL);
    if (ret!=0){
        int32 errCode = OSError;
        switch (ret){
            case  ERROR_ACCESS_DENIED:{
                errCode = ErrorAccessDenied;
            } break;
        }
        f.action = errCode;
        CStaticAssertPlatformErrorCondition(errCode,ret,"BasicFile::Open failed");

        return False;
    }
    switch(_action){
        case FILE_EXISTED  : f.action = openWasOpen      ;break;
        case FILE_CREATED  : f.action = openWasCreate    ;break;
        case FILE_TRUNCATED: f.action = openWasOverWrite ;break;
    }
#elif defined(_WIN32)
    uint32 desiredAccess         = 0;
    uint32 shareMode             = 0;
    uint32 creationDistribution  = 0;
    uint32 flagsAndAttributes    = 0;

    uint32 tmp = f.fileMode & openingFlagsMask1;
    switch (tmp | (openingFlagsMask1 << 16)){
        case createNewFile     : creationDistribution |= CREATE_NEW                 ; break;
        case createOverwrite   : creationDistribution |= CREATE_ALWAYS              ; break;
        case openFile          : creationDistribution |= OPEN_EXISTING              ; break;
        case openCreate        : creationDistribution |= OPEN_ALWAYS                ; break;
    }
    tmp = f.fileMode & openingModeMask1;
    switch (tmp | (openingModeMask1 << 16)){
        case localityNone       : flagsAndAttributes |= FILE_FLAG_RANDOM_ACCESS     ; break;
        case localitySequential : flagsAndAttributes |= FILE_FLAG_SEQUENTIAL_SCAN   ; break;
        case localityRandom     : flagsAndAttributes |= FILE_FLAG_RANDOM_ACCESS     ; break;
        case localityMixed      : flagsAndAttributes |= FILE_FLAG_RANDOM_ACCESS     ; break;
    }
    tmp = f.fileMode & openingModeMask2;
    switch (tmp | (openingModeMask2 << 16)){
        case shareModeNoRW      : shareMode |= 0                                    ; break;
        case shareModeNoW       : shareMode |= FILE_SHARE_READ                      ; break;
        case shareModeNoR       : shareMode |= FILE_SHARE_WRITE                     ; break;
        case shareModeAll       : shareMode |= (FILE_SHARE_WRITE | FILE_SHARE_READ) ; break;
    }
    tmp = f.fileMode & openingModeMask3;
    switch (tmp | (openingModeMask3 << 16)){
        case accessModeR        : desiredAccess |= GENERIC_READ                     ; break;
        case accessModeW        : desiredAccess |= GENERIC_WRITE                    ; break;
        case accessModeRW       : desiredAccess |= (GENERIC_READ | GENERIC_WRITE)   ; break;
        default: return False;
    }
    f.file = CreateFile(name,desiredAccess,shareMode,NULL,creationDistribution,flagsAndAttributes,NULL);
    if (f.file == INVALID_HANDLE_VALUE){
        int32 errCode = OSError;
        switch (GetLastError()){
             case  ERROR_SHARING_VIOLATION:{
                errCode = ErrorSharing;
            } break;
             case  ERROR_ACCESS_DENIED:{
                errCode = ErrorAccessDenied;
            } break;
        }
        f.action = errCode;
//        CStaticAssertPlatformErrorCondition(errCode,"BasicFile::Open failed");
        return False;
    }
    if        (creationDistribution == CREATE_NEW)    f.action = openWasCreate;
    else if   (creationDistribution == OPEN_EXISTING) f.action = openWasOpen;
    else if   (GetLastError()==ERROR_ALREADY_EXISTS){
         if   (creationDistribution == CREATE_ALWAYS) f.action = openWasOverWrite;
    } else if (creationDistribution == CREATE_ALWAYS) f.action = openWasCreate;
    else                                              f.action = openWasOpen;
#elif defined(_VXWORKS)
    int flags = 0;
    int mode = 0777;

    uint32 tmp = f.fileMode & openingModeMask3;
    switch (tmp | (openingModeMask3 << 16)){
        case accessModeR        : flags |= O_RDONLY                                 ; break;
        case accessModeW        : flags |= O_WRONLY                                 ; break;
        case accessModeRW       : flags |= O_RDWR                                   ; break;
        default: return False;
    }

    tmp = f.fileMode & openingFlagsMask1;
    switch (tmp | (openingFlagsMask1 << 16)){
        case createNewFile     : flags |= O_CREAT                                   ; break;
        case createOverwrite   : flags |= O_CREAT                                   ; break;
    }

    f.file = open(name,flags,mode);
    if (f.file != ERROR) {
        struct stat statistiche;        
        int ret = fstat(f.file, &statistiche);        
        if(ret < 0 || !S_ISREG(statistiche.st_mode)){
            close(f.file);
            return False;
        }

        if (flags & O_CREAT) f.action = openWasCreate;
        f.action = openWasOpen;
        return True;
    }

    if ((flags & O_CREAT) == 0){

        tmp = f.fileMode & openingFlagsMask1;
        switch (tmp | (openingFlagsMask1 << 16)){
            case openCreate        : flags |= O_CREAT                               ; break;
        }

        f.file = open(name,flags,mode);
        if (f.file != ERROR) {
            f.action = openWasCreate;
            return True;
        }
    }

    int32 errCode = OSError;
    switch (errnoGet()){
        case  EACCES:{
            errCode = ErrorAccessDenied;
        } break;
    }

    f.action = errCode;
    CStaticAssertPlatformErrorCondition(OSError,"BasicFile::Open failed");
    return False;
#endif
    return True;
}


bool FileLock(BasicFile &f,int64 start,int64 size,TimeoutType msecTimeout){
#if defined(_OS2)
    FILELOCK LockArea   = {0};
    FILELOCK UnLockArea = {0};
    LockArea.lOffset = start;
    LockArea.lRange  = size;
    APIRET ret = DosProtectSetFileLocks(f.file,&UnLockArea,&LockArea,msecTimeout.msecTimeout,0,lock);
    if (ret!=0) {
        AssertDosErrorCondition(ret,"LockingFileCore::Lock failed");
        return False;
    }
    return True;
#elif defined(_WIN32)
    OVERLAPPED overLapped;
    uint32 sizeLo = size & 0xFFFFFFFF;
    uint32 sizeHi = size >> 32;
    overLapped.Offset = start & 0xFFFFFFFF;
    overLapped.OffsetHigh = start >> 32;
    EventSem hev;
    hev.Create();
    overLapped.hEvent = hev.Handle();
    BOOL ret = LockFileEx(f.file,LOCKFILE_EXCLUSIVE_LOCK,0,sizeLo,sizeHi,&overLapped);
    if (ret == FALSE){
        hev.Wait(msecTimeout.msecTimeout);
        ret = LockFileEx(f.file,LOCKFILE_EXCLUSIVE_LOCK,0,sizeLo,sizeHi,&overLapped);
    }
    hev.Close();
    return (ret == TRUE);
#elif (defined(_RTAI) || defined(_LINUX) || defined(_MACOSX))
    // blocca, ma senza timeout!!!
    int ret=flock(f.file, LOCK_EX);
    if (ret<0) return False;
    return True;
#elif (defined(_VXWORKS)  || defined(_SOLARIS))
    return False;
#endif
}

/// Undo the locking.
bool FileUnLock(BasicFile &f,int64 start,int64 size,TimeoutType msecTimeout){
#if defined(_OS2)
    FILELOCK LockArea   = {0};
    FILELOCK UnLockArea = {0};
    UnLockArea.lOffset = start;
    UnLockArea.lRange  = size;
    APIRET ret = DosProtectSetFileLocks(f.file,&UnLockArea,&LockArea,msecTimeout.msecTimeout,0,lock);
    if (ret!=0) {
        AssertDosErrorCondition(ret,"LockingFileCore::UnLock failed");
        return False;
    }
    return True;

#elif defined(_WIN32)
//    timeout = 0;
    OVERLAPPED overLapped;
    uint32 sizeLo = size & 0xFFFFFFFF;
    uint32 sizeHi = size >> 32;
    overLapped.Offset = start & 0xFFFFFFFF;
    overLapped.OffsetHigh = start >> 32;
    BOOL ret = LockFileEx(f.file,LOCKFILE_EXCLUSIVE_LOCK,0,sizeLo,sizeHi,&overLapped);
    return (ret == TRUE);
#elif (defined(_RTAI) || defined(_LINUX) || defined(_MACOSX))
    // blocca, ma senza timeout!!!
    int ret=flock(f.file, LOCK_UN);
    if (ret<0) return False;
    return True;
#elif (defined(_VXWORKS)  || defined(_SOLARIS))
    return False;
#endif
}
//@}

/// Set the system value for the number of files
bool FileSetMaxNumberOfFiles(uint32 number){
#if defined(_OS2)
    LONG added = 0;
    ULONG newMax = 0;
    APIRET ret = DosSetRelMaxFH(&added,&newMax);
    if (ret!=0) return False;
    if (newMax > number) return True;
    added = number - newMax;
    ret = DosSetRelMaxFH(&added,&newMax);
    if (ret!=0) return False;
    return True;
#elif defined(_WIN32)
    SetHandleCount(number);
    return True;
#elif (defined(_VXWORKS) || defined(_SOLARIS))
    return True;
#endif
}

///
bool FileEraseFile(const char *fname,...){
    char name[256];
    va_list argList;
    va_start(argList,fname);
    vsnprintf(name,256,fname,argList);
    va_end(argList);
#if defined(_OS2)
    return False;
#elif defined(_WIN32)
    BOOL ret = DeleteFile( name );
    return (ret == TRUE);
#elif defined(_VXWORKS)
    int ret = remove(name);
    return (ret != ERROR);
#elif (defined(_RTAI)|| defined(_LINUX) || defined(_SOLARIS) || defined(_MACOSX))
    int ret = remove(name);
    if (ret<0) return False;
    return True;
#endif

}


#if defined(_VXWORKS)
bool BasicFile::Open(const char *fname,...){
    char name[256];
    va_list argList;
    va_start(argList,fname);
    vsnprintf(name,256,fname,argList);
    va_end(argList);
    return FileOpen(*this,name);
}

bool BasicFile::OpenWrite(const char *fname,...){
    SetOpeningModes(openCreate | accessModeRW);
    char name[256];
    va_list argList;
    va_start(argList,fname);
    vsnprintf(name,256,fname,argList);
    va_end(argList);
    return FileOpen(*this,name);
}

bool BasicFile::OpenRead(const char *fname,...){

    SetOpeningModes(openFile | accessModeR);
    char name[256];
    va_list argList;
    va_start(argList,fname);
    vsnprintf(name,256,fname,argList);
    va_end(argList);
    return FileOpen(*this,name);
}

bool BasicFile::OpenNew(const char *fname,...){
    SetOpeningModes(createOverwrite | accessModeRW);
    char name[256];
    va_list argList;
    va_start(argList,fname);
    vsnprintf(name,256,fname,argList);
    va_end(argList);
    return FileOpen(*this,name);
}

#endif



