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
 * Extension to ConfigurationDataBase with several utility methods to read and write basic types
 */
#if !defined(CONFIGURATION_DATABASE_EXENDED)
#define CONFIGURATION_DATABASE_EXENDED

#include "ConfigurationDataBase.h"
#include "CDBDataTypes.h"

/** contains a reference to CDB of type CDBVirtual.
Forces call to CDB to be made via the virtual table */
class CDBExtended: public ConfigurationDataBase{

#if !defined(_CINT)

public:

    /** Constructor of a CDB from a generic base type    */
    inline CDBExtended(const char *coreClassName="CDB"):
        ConfigurationDataBase(coreClassName){
    }

    /** Constructor of a CDB. If base is specified it creates a new reference to an existing database
        @param base        If NULL creates a new database. If not NULL it creates a reference to an existing one
        @param cdbcm       Use CDBCM_CopyAddress to copy the address from the reference.
                           In case of two or more flags, after oring one must cast back to the enum type     */
    inline CDBExtended(ConfigurationDataBase &base,CDBCreationMode cdbcm=CDBCM_CopyAddress):
        ConfigurationDataBase(base,cdbcm){
    }

    inline CDBExtended(GCReference cdb,CDBCreationMode cdbcm=CDBCM_CopyAddress) : ConfigurationDataBase(cdb,cdbcm){
    }

    /** Copy Operator of a CDB. It creates a new reference to an existing database */
    inline void operator=(ConfigurationDataBase &base){
        ConfigurationDataBase::operator=(base);
    }
//##################################################################/
//##                                                              ##
//##            WriteArray and ReadArray derivatives              ##
//##                                                              ##
//##################################################################/

    /* UTILITY FUNCTIONS */

    /** */
    inline bool ReadFloat(float &value,const char *configName,float defaultValue = 0,bool caseSensitive = True){
        bool ret = typeTObjectPointer->ReadArray(&value,CDBTYPE_float,NULL,0,configName,caseSensitive);
        if (!ret) value = defaultValue;
        return ret;
    }

    /** */
    inline bool ReadDouble(double &value,const char *configName,double defaultValue = 0,bool caseSensitive = True){
        bool ret = typeTObjectPointer->ReadArray(&value,CDBTYPE_double,NULL,0,configName,caseSensitive);
        if (!ret) value = defaultValue;
        return ret;
    }

    /**  reads int64 decimal */
    inline bool ReadInt64(int64 &value,const char *configName,int64 defaultValue = 0,bool caseSensitive = True){
        bool ret = typeTObjectPointer->ReadArray(&value,CDBTYPE_int64,NULL,0,configName,caseSensitive);
        if (!ret) value = defaultValue;
        return ret;
    }

    /**  reads int32 decimal */
    inline bool ReadInt32(int32 &value,const char *configName,int32 defaultValue = 0, bool caseSensitive = True){
        bool ret = typeTObjectPointer->ReadArray(&value,CDBTYPE_int32,NULL,0,configName,caseSensitive);
        if (!ret) value = defaultValue;
        return ret;
    }

    /**  reads uint32 decimal */
    inline bool ReadUint32(uint32 &value,const char *configName,uint32 defaultValue = 0,bool caseSensitive = True){
        bool ret = typeTObjectPointer->ReadArray(&value,CDBTYPE_uint32,NULL,0,configName,caseSensitive);
        if (!ret) value = defaultValue;
        return ret;
    }

    /**  reads octal hex decimal */
    inline bool ReadPointer(intptr &value,const char *configName,int32 defaultValue = 0,bool caseSensitive = True){
        bool ret = typeTObjectPointer->ReadArray(&value,CDBTYPE_Pointer,NULL,0,configName,caseSensitive);
        if (!ret) value = defaultValue;
        return ret;
    }

    /** reads a string into a FString */
    inline bool ReadFString(FString &value,const char *configName,const char *defaultValue = "", bool caseSensitive=True){
        bool ret = typeTObjectPointer->ReadArray(&value,CDBTYPE_FString,NULL,0,configName,caseSensitive);
        if (!ret) value = defaultValue;
        return ret;
    }

    /** reads a string into a BString */
    inline bool ReadBString(BString &value,const char *configName,const char *defaultValue = "", bool caseSensitive=True){
        bool ret = typeTObjectPointer->ReadArray(&value,CDBTYPE_BString,NULL,0,configName,caseSensitive);
        if (!ret) value = defaultValue;
        return ret;
    }

    /** */
    inline bool WriteInt32(uint32 value,const char *configName){
        return typeTObjectPointer->WriteArray(&value,CDBTYPE_int32,NULL,0,configName);
    }

    inline bool WritePointer(void *value,const char *configName){
        return typeTObjectPointer->WriteArray(value,CDBTYPE_Pointer,NULL,0,configName);
    }


    /** */
//    inline bool WriteHex(uint32 value,const char *configName){
//        return typeTObjectPointer->WriteArray(&value,CDBTYPE_hex,NULL,0,configName);
//    }

    /** */
//    inline bool WriteOctal(uint32 value,const char *configName){
//        return typeTObjectPointer->WriteArray(&value,CDBTYPE_octal,NULL,0,configName);
//    }

    /** */
    inline bool WriteFloat(float value,const char *configName){
        return typeTObjectPointer->WriteArray(&value,CDBTYPE_float,NULL,0,configName);
    }

    /** */
    inline bool WriteDouble(double value,const char *configName){
        return typeTObjectPointer->WriteArray(&value,CDBTYPE_double,NULL,0,configName);
    }

    /** writes a zero tereminated array of chars */
    inline bool WriteString(const char *value,const char *configName){
        return typeTObjectPointer->WriteArray(&value,CDBTYPE_String,NULL,0,configName);
    }

    /** writes a FString */
    inline bool WriteFString(FString &value,const char *configName){
        return typeTObjectPointer->WriteArray(&value,CDBTYPE_FString,NULL,0,configName);
    }

    /** size is a vector of dimensions in the same order as the C declaration of an equivalent structure */
    inline bool ReadFStringArray(FString *value,int *size,int nDim,const char *configName,bool caseSensitive = True){
        return typeTObjectPointer->ReadArray(value,CDBTYPE_FString,size,nDim,configName,caseSensitive);
    }

    /** size is a vector of dimensions in the same order as the C declaration of an equivalent structure */
    inline bool ReadBStringArray(BString *value,int *size,int nDim,const char *configName,bool caseSensitive = True){
        return typeTObjectPointer->ReadArray(value,CDBTYPE_BString,size,nDim,configName,caseSensitive);
    }

    /** size is a vector of dimensions in the same order as the C declaration of an equivalent structure */
    inline bool WriteFStringArray(const FString *value,int *size,int nDim,const char *configName){
        return typeTObjectPointer->WriteArray(value,CDBTYPE_FString,size,nDim,configName);
    }

    /** size is a vector of dimensions in the same order as the C declaration of an equivalent structure */
    inline bool WriteBStringArray(const BString *value,int *size,int nDim,const char *configName){
        return typeTObjectPointer->WriteArray(value,CDBTYPE_BString,size,nDim,configName);
    }

    /** size is a vector of dimensions in the same order as the C declaration of an equivalent structure */
    inline bool ReadFloatArray(float *value,int *size,int nDim,const char *configName,bool caseSensitive = True){
        return typeTObjectPointer->ReadArray(value,CDBTYPE_float,size,nDim,configName,caseSensitive);
    }

    /** size is a vector of dimensions in the same order as the C declaration of an equivalent structure */
    inline bool WriteFloatArray(const float *value,int *size,int nDim,const char *configName){
        return typeTObjectPointer->WriteArray(value,CDBTYPE_float,size,nDim,configName);
    }

    /** */
    inline bool ReadDoubleArray(double *value,int *size,int nDim,const char *configName,bool caseSensitive = True){
        return typeTObjectPointer->ReadArray(value,CDBTYPE_double,size,nDim,configName,caseSensitive);
    }

    /** */
    inline bool WriteDoubleArray(const double *value,int *size,int nDim,const char *configName){
        return typeTObjectPointer->WriteArray(value,CDBTYPE_double,size,nDim,configName);
    }

    /** */
    inline bool ReadInt32Array(int32 *value,int *size,int nDim,const char *configName,bool caseSensitive = True){
        return typeTObjectPointer->ReadArray(value,CDBTYPE_int32,size,nDim,configName,caseSensitive);
    }

    /** */
    inline bool ReadUint32Array(uint32 *value,int *size,int nDim,const char *configName,bool caseSensitive = True){
        return typeTObjectPointer->ReadArray(value,CDBTYPE_uint32,size,nDim,configName,caseSensitive);
    }

    /** */
    inline bool ReadInt64Array(int64 *value,int *size,int nDim,const char *configName,bool caseSensitive = True){
        return typeTObjectPointer->ReadArray(value,CDBTYPE_int64,size,nDim,configName,caseSensitive);
    }

    /** */
    inline bool WriteInt32Array(const int32 *value,int *size,int nDim,const char *configName){
        return typeTObjectPointer->WriteArray(value,CDBTYPE_int32,size,nDim,configName);
    }

    /** */
    inline bool WriteUint32Array(const uint32 *value,int *size,int nDim,const char *configName){
        return typeTObjectPointer->WriteArray(value,CDBTYPE_uint32,size,nDim,configName);
    }

    /** Translates an identifier to a value
        identifierList is a zero terminated vector of const char *    */
    inline bool ReadOptions(int32 &value,const char *configName,const char **identifierList,const int32 *values,int32 defaultValue,bool caseSensitive = True){
        value = defaultValue;
        FString sValue;
        bool ret = typeTObjectPointer->ReadArray(&sValue,CDBTYPE_FString,NULL,0,configName,caseSensitive);
        if (!ret ) return False;
        int ix = 0;
        while(identifierList[ix] != NULL){
            if (sValue == identifierList[ix] ){
                value = values[ix];
                return True;
            }
            ix++;
        }
        typeTObjectPointer->AssertErrorCondition(FatalError,"ReadOptions option %s not found",sValue.Buffer());
        return False;
    }

    /** This routine finds the matrix dimension first then allocates memory and finally read data
        @param size is a vector of dimensions of size @param maxDim
        @param value will be allocated with at maximum @param maxSize floats
        this routine uses malloc */
    inline bool AllocateAndReadFloatArray(float *&value,int *size,int &maxDim,int maxSize,const char *configName,CDBArrayIndexingMode cdbaim = CDBAIM_Strict,bool caseSensitive = True){
        value = NULL;
        if (!typeTObjectPointer->GetArrayDims(size,maxDim,configName,cdbaim)) return False;
        int nOfElements = 1;
        int i;
        for (i = 0;i < maxDim;i++){
            if (size[i] > 0) nOfElements = nOfElements * size[i];
        }
        if (nOfElements > maxSize) {
            typeTObjectPointer->AssertErrorCondition(FatalError,"AllocateAndReadFloatArray: too many elements %i > %i",nOfElements,maxSize);
            return False;
        }
        value = (float *)malloc(sizeof(float)*nOfElements);
        if (value == NULL) {
            typeTObjectPointer->AssertErrorCondition(FatalError,"AllocateAndReadFloatArray: malloc(%i) failed",sizeof(float)*nOfElements);
            return False;
        }
        return typeTObjectPointer->ReadArray(value,CDBTYPE_float,size,maxDim,configName,caseSensitive);
    }

    /** This routine finds the matrix dimension first then allocates memory and finally read data
        @param size is a vector of dimensions of size @param maxDim
        @param value will be allocated with at maximum @param maxSize FString
        this routine used new[]*/
    inline bool AllocateAndReadFStringArray(FString *&value,int *size,int &maxDim,int maxSize,const char *configName,CDBArrayIndexingMode cdbaim = CDBAIM_Strict,bool caseSensitive = True){
        value = NULL;
        if (!typeTObjectPointer->GetArrayDims(size,maxDim,configName,cdbaim)) return False;
        int nOfElements = 1;
        int i;
        for (i = 0;i < maxDim;i++){
            if (size[i] > 0) nOfElements = nOfElements * size[i];
        }
        if (nOfElements > maxSize) {
            typeTObjectPointer->AssertErrorCondition(FatalError,"AllocateAndReadFStringArray: too many elements %i > %i",nOfElements,maxSize);
            return False;
        }
        value = new FString[nOfElements];
        if (value == NULL) {
            typeTObjectPointer->AssertErrorCondition(FatalError,"AllocateAndReadFStringArray: new[%i] failed",nOfElements);
            return False;
        }
        return typeTObjectPointer->ReadArray(value,CDBTYPE_FString,size,maxDim,configName,caseSensitive);
    }

    /** Translates a vector of identifiers to a bitset value
        identifierList is a zero terminated vector of const char *
        the values associated to each identifier is in @param values */
    inline bool ReadFlags(int32 &value,const char *configName,const char **identifierList,const int32 *values,int32 defaultValue,bool caseSensitive = True){

        value = defaultValue;
        FString *flags;
        int size;
        int maxDim = 1;

        if (!AllocateAndReadFStringArray(flags,&size,maxDim,100,configName,CDBAIM_Strict,caseSensitive)){
            return False;
        }

        int i;
        for (i = 0;i<size;i++){
            int ix = 0;
            while((identifierList[ix] != NULL) && !(flags[i] == identifierList[ix])){
                ix++;
            }
            if (identifierList[ix] != NULL){
                value |= values[ix];
            } else {
                typeTObjectPointer->AssertErrorCondition(FatalError,"ReadFlags: flag %s not found",flags[i].Buffer());
                delete[] flags;
                return False;
            }
        }

        delete[] flags;

        return True;
    }
#endif //CINT
};

#endif

