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

#include "File.h"
#include "EAFile.h"

OBJECTREGISTER(EAFile,"$Id$")

bool EAFileOpen(EAFile &f,const char *name){
    f.GetCDB()->CleanUp();
    f.hasEa  = False;
    f.selectedStream = 0;

    if (FileOpen(f,name)!= True) return False;

    uint32 eaFileMode = f.fileMode & ~(openingFlagsMask1 | openingModeMask2);
    eaFileMode |= shareModeNoRW;

    char nameext[512];
    sprintf(nameext, "%s%s",name,EA_EXT);

    bool ret;
    if ((f.fileMode & openingFlagsMask1)==(createOverwrite & openingFlagsMask1)){
        FileEraseFile(nameext);
        ret = False;
    } else {
        f.eaFile.SetOpeningModes(eaFileMode | openFile);
        ret = f.eaFile.Open(nameext);
    }
    if ((ret == False) && (f.eaFile.ErrorReason() == ErrorSharing)){
        f.action = f.eaFile.ErrorReason();
        f.File::Close();
        return False;
    }
    if (ret != False){
        f.hasEa = True;
        // wipe clean and reload
        f.GetCDB()->CleanUp();
        f.GetCDB()->ReadFromStream(f.eaFile,NULL);
    }

    return True;
}

bool EAFileAddStream(EAFile &f,const char *name){
    if (f.hasEa == False){

        uint32 eaFileMode = f.fileMode & ~(openingFlagsMask1 | openingModeMask2);
        eaFileMode |= shareModeNoRW;

        f.eaFile.SetOpeningModes(eaFileMode | openCreate);
        f.eaFile.SetOpeningModes(f.fileMode);

        char nameext[512];
        sprintf(nameext, "%s%s" ,f.fileName,EA_EXT);
        if (f.eaFile.Open(nameext)== False) return False;
        f.hasEa=True;
    }
    return f.attributes.AddStream(name);
}

bool EAFileClose(EAFile &f){
    if (f.hasEa){
        if (f.eaFile.CanWrite()){
            f.eaFile.Seek(0);
            f.eaFile.SetSize(0);
            f.attributes.Commit();
            f.GetCDB()->WriteToStream(f.eaFile,NULL);
        }
        f.eaFile.Close();
        f.hasEa = False;
    }
    f.GetCDB()->CleanUp();
    f.selectedStream = 0;
    return f.File::Close();
}






#if defined(_VXWORKS)

bool EAFile::Open(const char *fname,...){
    char name[256];
    va_list argList;
    va_start(argList,fname);
    vsnprintf(name,256,fname,argList);
    va_end(argList);
    return EAFileOpen(*this,name);
}

#endif



