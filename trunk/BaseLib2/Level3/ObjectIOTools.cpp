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

#include "ObjectIOTools.h"
#include "CDB.h"
#include "ObjectRegistryDataBase.h"

bool OBJObjectReadFlags(EAFile &file,uint32 &classId,uint32 &userId,uint32 &flags,StreamInterface *err){

    uint32 originalStream = file.SelectedStream();

    classId = 0;
    userId = 0;
    flags = 0;

    if (file.Switch("Class")==True){
        file.Seek(0);
        char buffer[256];
        file.GetToken(buffer,"\n\r \t",sizeof(buffer));
        ObjectRegistryItem *ori = ObjectRegistryDataBaseFind(buffer);
        if (ori == NULL) {
            if (err!=NULL)
                err->Printf("class %s is unknown\n",buffer);
            return False;
        }
        classId = ori->ClassId();
        flags   = ori->DefaultFlags();
    } else {
        // either the url or the file name or in general how the object is identified
        char fileName[128];
        file.StreamName(0,fileName,sizeof(fileName)-1);

        ObjectRegistryItem *ori = ObjectRegistryDataBaseFindByExtension(fileName);
        if (ori == NULL) {
            if (err!=NULL)
                err->Printf("class of %s is unknown\n",fileName);
            return False;
        }
        classId = ori->ClassId();
        flags   = ori->DefaultFlags();
    }

    if (file.Switch("Flags")==True){
        file.Seek(0);
        char buffer[256];
        file.GetToken(buffer,"\n\r \t",sizeof(buffer));
        uint32 flags2;
        sscanf(buffer,"0x%x",&flags2);
        flags ^= flags2;
    }

    if (file.Switch("UserId")==True){
        file.Seek(0);
        char buffer[256];
        file.GetToken(buffer,"\n\r \t",sizeof(buffer));
        userId=atoi(buffer);
    }

    file.Switch(originalStream);
    return True;
}

bool OBJObjectUpdateFlags(EAFile &file,uint32 classId,uint32 userId,uint32 flags,StreamInterface *err){

    ObjectRegistryItem *ori = ObjectRegistryDataBaseFindByCode(classId);
    if (ori == NULL) return False;

    uint32 originalStream = file.SelectedStream();

    file.AddStream("Class");
    if (file.Switch("Class")==True){
        file.Seek(0);
        file.Printf((char *)ori->ClassName());
    }

    file.AddStream("Flags");
    if (file.Switch("Flags")==True){
        file.Seek(0);
        flags ^= ori->DefaultFlags();
        file.Printf("0x%x",flags);
    }

    file.AddStream("UserId");
    if (file.Switch("UserId")==True){
        file.Seek(0);
        file.Printf("%i",userId);
    }

    file.Switch(originalStream);

    return (classId!=0);
}


bool OBJObjectSave(Object &object,EAFile &file,uint32 userId,uint32 flags,StreamInterface *err){
    ObjectRegistryItem *i = object.Info();
    if (i == NULL) return False;

    if (OBJObjectUpdateFlags(file,object.ClassId(),userId,flags,err)!= True) return False;

    CDBExtended &cdb = file.GetCDB();

    bool ret = object.ObjectSaveSetup(cdb,err);
    if (!ret){
        CStaticAssertErrorCondition(FatalError,"OBJObjectSave  ObjectSaveSetup failed ");
        return False;
    }

    file.Switch((uint32)0);

    cdb->MoveToRoot();

    if (cdb->Exists("binary")){
        cdb->Move("binary");
        if (cdb->NumberOfChildren() < 0){
            FString buffer;
            cdb.ReadFString(buffer,"");
            file.SetSize(0);
            uint32 size = buffer.Size();
            if (!file.Write(buffer.Buffer(),size)){
                CStaticAssertErrorCondition(FatalError,"OBJObjectSave  failed writing %i bytes on main file",size);
            }
            cdb->MoveToRoot();
            cdb->Delete("binary");

            return True;
        } else {
            CStaticAssertErrorCondition(FatalError,"OBJObjectSave  node root.binary is not a leaf");
            return False;
        }
    }

    // it was text only
    return True;
}

Object *OBJObjectLoadCreate(Object *object,EAFile &file,uint32 &userId,uint32 &flags,StreamInterface *err){
    uint32 classId = 0;
    if (OBJObjectReadFlags(file,classId,userId,flags,err)!=True) return NULL;

    bool objectCreated = False;

    if (object != NULL){
        if (object->ClassId() != classId) {
            if (err!=NULL) {
                CStaticAssertErrorCondition(FatalError,"%s:OBJObjectLoadCreate read class id %x instead of %x\n",object->ClassName(),classId,object->ClassId());
                err->Printf("%s:OBJObjectLoadCreate read class id %x instead of %x\n",object->ClassName(),classId,object->ClassId());
                ObjectRegistryItem *ori = ObjectRegistryDataBaseFindByCode(classId);
                if (ori == NULL) err->Printf("class id %x is unknown\n",classId);
                else             err->Printf("class id %x is %s\n",ori->ClassName());
            }
            return NULL;
        }
    } else {
        object = OBJObjectCreateById(classId);
        if (object == NULL){
            CStaticAssertErrorCondition(FatalError,"OBJObjectLoadCreate cannot create class id %x \n",classId);
            return NULL;
        }
        objectCreated = True;
    }


    CDBExtended &cdb = file.GetCDB();
    cdb->MoveToRoot();

    file.Switch((uint32)0);
    file.Seek(0);
    uint32 size = file.Size();
    char *buffer = (char *)malloc(size);
    if (buffer == NULL){
        CStaticAssertErrorCondition(OSError,"OBJObjectLoad  failed allocating %i bytes",size);
        return NULL;
    }
    file.Read(buffer,size);
    bool ret = cdb.WriteString(buffer,"binary");
    free((void *&)buffer);

    if (!ret){
        CStaticAssertErrorCondition(FatalError,"OBJObjectLoad  failed writing to root.binary");
        return NULL;
    }

    cdb->MoveToRoot();

    if (object->ObjectLoadSetup(cdb,err)) return object;
    CStaticAssertErrorCondition(ParametersError,"OBJObjectLoad  failed initialising object");

    if (objectCreated) delete object;

    return NULL;

}




