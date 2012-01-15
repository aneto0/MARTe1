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

#include "CDBStringDataNode.h"
#include "Object.h"

OBJECTREGISTER(CDBDataNode,"$Id: CDBStringDataNode.cpp,v 1.23 2011/10/20 18:25:23 aneto Exp $")

OBJECTLOADREGISTER(CDBStringDataNode,"$Id: CDBStringDataNode.cpp,v 1.23 2011/10/20 18:25:23 aneto Exp $")

static inline char *StrLimitedDup(const char *s, int size){

    if (s == NULL) return NULL;
    if (size < 0) return strdup(s);
    char *ret = (char *)malloc(size+1);
    memcpy(ret,s,size);
    ret[size] = 0;
    return ret;
}

static inline bool strstrNoCase(const char *s1,const char *s2){
    if (s1==NULL) return False;
    if (s2==NULL) return False;
    if (s2[0] == 0) return True;
    if (s1[0] == 0) return False;

    while(s1[0] != 0){
        char c1 = toupper(s1[0]);
        char c2 = toupper(s2[0]);
        if (c1 == c2) {
            if (strncasecmp(s1,s2,strlen(s2))==0) return True;
        }
        s1++;
    }
    return False;
}

static inline double stringToDouble(const char *value){
    double ret = 0.0;
    if ((value == NULL)             ||
        strstrNoCase(value,"NAN")   ||
        strstrNoCase(value,"IND")) {
        int32 data = IEEE_FLOAT_NAN;
        ret =  *((float *)&data);
    } else
    if (strstrNoCase(value,"+INF")) {
        int32 data = IEEE_FLOAT_INF;
        ret =  *((float *)&data);
    } else
    if (strstrNoCase(value,"-INF")) {
        int32 data = IEEE_FLOAT_INF_NEG;
        ret =  *((float *)&data);
    } else {
        ret = atof(value);
    }
    return ret;
}
static bool IsOctal(const char *p){
    if (p[0]!='0') return False;

    int ix = 1;
    while ((p[ix] =='0') && (p[ix] != 0)) ix++;
    if ((p[ix] >= '1')  && (p[ix]  <='9')) return True;

    return False;

}

static inline intptr stringToPointer(const char *p){
    if (p == NULL) return 0;
    intptr value;
    sscanf(p, "%p",&value);
    return value;
}

static inline int stringToInteger32(const char *p){
    if (p == NULL) return 0;
    int32 value;
    if(strncmp(p,"0x",2)==0){
        sscanf(p+2,"%x",&value);
    } else
    if (IsOctal(p)){
        sscanf(p+1,"%o",&value);
    } else {
        sscanf(p,"%i",&value);
    }
    return value;
}

static inline int64 stringToInteger64(const char *p){
    if (p == NULL) return 0;
    int64 value;
    if(strncmp(p,"0x",2)==0){
        sscanf(p+2,"%llx",&value);
    } else
    if (IsOctal(p)){
        sscanf(p+1,"%llo",&value);
    } else {
        sscanf(p,"%lld",&value);
    }
    return value;
}

static inline void SkipSeps(const char *&s,const char *sep){
    while((s[0] != 0) && (strchr(sep,s[0])!=NULL))s++;
}

static inline void FindSep(const char *&s,const char *sep){
    while((s[0] != 0) && (strchr(sep,s[0])==NULL))s++;
}

static const int ctInvalid             = 0;
static const int ctValid               = 1;
static const int ctNonPrintable        = 2;

static inline bool Contained(char c, const char *list){
    while (*list!= 0){
        if (c == *list) return True;
        list++;
    }
    return False;
}

static inline int CharType(const char c){
    if (Contained(c," \n\t\r(),;{}<>=/")) return ctInvalid;
    if (c == '&')  return ctNonPrintable;
    if (c == '"')  return ctNonPrintable;
    if (c < 32)    return ctNonPrintable;
    if (c > 126)   return ctNonPrintable;
    return ctValid;
}

static inline int hexValue(char c){
    int hexValue = 0;
    if ((c>='0') && (c <='9')) hexValue = c - '0';
    else
    if ((c>='A') && (c <='Z')) hexValue = c - 'A';
    else
    if ((c>='a') && (c <='z')) hexValue = c - 'a';
    else {
        CStaticAssertErrorCondition(IllegalOperation,"character %c cannot be converted from hex",c);
//        SleepSec(0.1);
        return -1;
    }
    return hexValue;
}

static inline char hex(int n){
    if ((n >= 0)  && (n <= 9))  return n +'0';
    if ((n >= 10) && (n <= 15)) return n +'A' - 10;
    else {
        CStaticAssertErrorCondition(IllegalOperation,"number %i cannot be converted to hex",n);
//        SleepSec(0.1);
        return (char)-1;
    }
    return 0;
}

static const int EXP_normal        = 0;
static const int EXP_expandExcapes = 1;
static const int EXP_expandHex1    = 2;
static const int EXP_expandHex2    = 3;
static const int EXP_ExcapeEnd     = 4;
static const int EXP_expandString  = 5;

struct XMLEscapes{ const char *string; char character; }
static const XMLEscapes[]={
    {"amp",'&'},
    {"quot",'"'},
    {"lt",'<'},
    {"gt",'>'},
    {"apos",'\''},
    {NULL,0}
};

static inline char XMLDecodeEscape(const char *name){
    if (name == NULL) return 0;
    int index = 0;
    while(XMLEscapes[index].string != NULL){
        if (strcmp(XMLEscapes[index].string,name)==0){
            return XMLEscapes[index].character;
        }
        index++;
    }
    CStaticAssertErrorCondition(IllegalOperation,"&%s; sequence is not a recognized escape sequence",name);
//    SleepSec(0.1);
    return 0;
}

static inline const char *XMLEncodeEscape(const char c){
    int index = 0;
    while(XMLEscapes[index].string != NULL){
        if (XMLEscapes[index].character==c){
            return XMLEscapes[index].string;
        }
        index++;
    }
    return NULL;
}

static inline const char *Decode(FString &buffer,const char *data,int size = -1){
    buffer.SetSize(0);
    int status = EXP_normal;

    int hex=0;
    FString excapeString;
    while ((*data != 0) && (size != 0)){
        size--;

        char c = *data++;
        switch (status){
            case EXP_normal:{
                if (c == '&') status = EXP_expandExcapes;
                else buffer += c;
            } break;
            case EXP_expandExcapes:{
                if (c == '#') status = EXP_expandHex1;
                else {
                    status = EXP_expandString;
                    excapeString = c;
                }
            } break;
            case EXP_expandHex1:{
                status = EXP_expandHex2;
                int value = hexValue(c);
                if (value < 0) return NULL;
                hex = value;
            } break;
            case EXP_expandHex2:{
                status = EXP_ExcapeEnd;
                int value = hexValue(c);
                if (value < 0) return NULL;
                hex = hex * 16 + value;
            } break;
            case EXP_expandString:{
                if (c ==';') {
                    char cc = XMLDecodeEscape(excapeString.Buffer());
                    if (cc != 0) buffer +=cc;
                    else {
                        buffer += '&';
                        buffer += excapeString.Buffer();
                        buffer += ';';
                    }
                    status = EXP_normal;
                } else {
                    excapeString += c;
                }
            } break;
            case EXP_ExcapeEnd:{
                if (c !=';') {
                    CStaticAssertErrorCondition(FatalError,"escape sequence starting from & must terminate with ;");
//                    SleepSec(0.1);
                    return NULL;
                }
                status = EXP_normal;
                buffer += (char)hex;
            } break;
            default:{
                return NULL;
            }
        }
    }
    return buffer.Buffer();
}

static inline const char *Encode(FString &buffer,const char *value){
    buffer.SetSize(0);

    while(*value != 0){
        unsigned char c = *value++;
        switch (CharType(c)){
            case ctInvalid:
            case ctValid:{
                buffer += c;
            } break;
            default:
            case ctNonPrintable:{
                buffer += '&';
                const char *escapeString = XMLEncodeEscape(c);
                if (escapeString == NULL){
                    buffer += '#';
                    buffer += hex((c & 0xF0)>>4);
                    buffer += hex(c & 0x0F);
                } else {
                    buffer += escapeString;
                }
                buffer +=';';
            } break;
        }
    }
    return buffer.Buffer();
}

static inline bool IsHexNumber(const char *s){
    const char *p = s;
    if(p[0] != '0') return False;
    p++;
    if(*p == 0) return False;
    if(toupper(*p) != 'X') return False;
    p++;
    if(*p == 0) return False;
    while (*p != 0){
        if(((toupper(*p) >= 'A') && (toupper(*p) <= 'F')) ||
            ((*p >= '0') && (*p <= '9'))){
            p++;
        }
        else{
            return False;
        }
    }
    return True;
}

static inline bool IsNumber(const char *s){
    const char *p                = s;
    bool        dotAlreadyFound  = False;
    bool        expAlreadyFound  = False;
    bool        signAlreadyFound = False;

    if((p[0] == '-') || (p[0] == '+')) p++;
    while (*p != 0){
        if((*p == '.') && !dotAlreadyFound){
            dotAlreadyFound = True;
        }
        else if((*p == '.') && dotAlreadyFound){
            return False;
        }
        else if((toupper(*p) == 'E') && !expAlreadyFound){
            expAlreadyFound = True;
        }
        else if((toupper(*p) == 'E') && expAlreadyFound){
            return False;
        }
        else if(((*p == '-') || (*p == '+')) && expAlreadyFound){
            signAlreadyFound = True;
        }
        else if(((*p == '-') || (*p == '+')) && !expAlreadyFound){
            return False;
        }
        else if(((*p == '-') || (*p == '+')) && signAlreadyFound){
            return False;
        }        
        else if((*p < '0') || (*p > '9')){
            return False;
        }       
        p++;
    }
    return True;
}

static inline bool NeedsQuotes(const char *s){
    if ( s == NULL) return True;
    if (s[0] == 0) return True;
    if (IsNumber(s)) return False;
    if (IsHexNumber(s)) return False;
    //It is not a number but starts with a number
    if(s[0] >= '0' && s[0] <= '9') return True;

    while (*s != 0){
        if (CharType(*s) == ctInvalid) return True;
        s++;
    }
    return False;
}

/*#################################################################################################*/
/*#################################################################################################*/
/*######                 CDBDataNode                                                        #######*/
/*#################################################################################################*/
/*#################################################################################################*/


void CDBDataNode::CleanUp(){
    if (elements){
        if (numberOfElements > 1){
            uint32 index;
            for (index = 0;index < numberOfElements;index++){
                if (elements[index])
                    free((void *&)elements[index]);
            }
        }
        free((void *&)elements);
    }
    elements            = NULL;
    numberOfElements    = 0;
}

void CDBDataNode::SetSize(int size){
    CleanUp();
    numberOfElements = size;
    if (size == 1) return;

    int allocSize = sizeof(char *) * numberOfElements;
    elements = (char **)malloc (allocSize);

    uint32 index;
    for (index = 0;index < numberOfElements;index++){
        elements[index] = NULL;
    }

}

/** elementNumber can be any value. The elements buffer will be realloced    */
bool CDBDataNode::WriteElement(uint32 elementNumber,const char *value,int32 maxSize){

    // single element. save from using double indirection
    if (numberOfElements == 1){
        if (elements != NULL) {
            AssertErrorCondition(IllegalOperation,"WriteElement: slot %i already FULL = %s",elementNumber,(char *)elements);
            return False;
        }
        elements = (char **)StrLimitedDup(value,maxSize);
        return True;
    }

    if (elements[elementNumber] != NULL) {
        AssertErrorCondition(IllegalOperation,"WriteElement: slot %i already FULL = %s",elementNumber,elements[elementNumber]);
        return False;
    }
    elements[elementNumber] = StrLimitedDup(value,maxSize);

    return True;
}

/** if elementNumber is outside range returns False    */
const char *CDBDataNode::ReadElement(uint32 elementNumber) const {
    if (elements == NULL) return NULL;
    if (numberOfElements == 1){
        return (const char *)elements;
    }
    return elements[elementNumber];
}

/*#################################################################################################*/
/*#################################################################################################*/
/*#################################################################################################*/


bool CDBStringDataNode::WriteContent(const void *value,const CDBTYPE &valueType,int size){
    bool ret = True;
    if (value == NULL) {
        AssertErrorCondition(ParametersError,"Write:value=NULL");
        return False;
    }

    if (size <= 0) size = 1;

    // free if any allocated memory
    CleanUp();
    SetSize(size);

    CDBDataType cdbdt = valueType.dataType;

//    switch(valueType.dataType.Value()){
    if (cdbdt == CDB_double){
            double *dv = (double *)value;
            ret = WriteDouble(dv,size);
    } else
    if (cdbdt == CDB_Pointer){
            ret = WritePointer(&value, size);
    } else
    if (cdbdt == CDB_float){
            float *fv = (float *)value;
            ret = WriteFloat(fv,size);
    } else
    if (cdbdt == CDB_int32){
            int32 *di = (int32 *)value;
            ret = WriteInteger(di,size);
    } else
    if (cdbdt == CDB_uint32){
            uint32 *di = (uint32 *)value;
            ret = WriteUnsignedInteger(di,size);
    } else
    if (cdbdt == CDB_char){
            char *dc = (char *)value;
            ret = WriteChar(dc,size);
    } else
    if (cdbdt == CDB_FString){
            FString *ds = (FString *)value;
            ret = WriteString(ds,size);
    } else
    if (cdbdt == CDB_BString){
            BString *ds = (BString *)value;
            ret = WriteString(ds,size);
    } else
    if (cdbdt == CDB_String){
            const char **ds = (const char **)value;
            ret = WriteString(ds,size);
    } else
    if (cdbdt == CDB_Interpret){
        const char **ds = (const char **)value;
        ret = WriteFormatted(ds[0]," {}\n\t\r");
    } else {
        BString bs;
        BasicTypeDescriptor btd(0,BTDTString,BTDSTCString);
        bool ret = False;
        if (size > 1) {
            ret = BTConvert(size,btd,elements,cdbdt,value);
        } else
        if (size == 1){
            char *p[1] = { NULL };
            ret = BTConvert(size,btd,p,cdbdt,value);
            elements = (char **)p[1];
        }

        if (!ret) AssertErrorCondition(ParametersError,"WriteEntry:unknown or unsupported data type %i",valueType.dataType.Value());
        return ret;
    }

    return ret;
}

bool CDBStringDataNode::WriteDouble(const double *values,uint32 size){
    if (values    == NULL) return False;

    bool ret = True;

    char buffer[64];
    char *endbuffer = &buffer[63];
    for (uint32 i = 0;i < size;i++){
        buffer[0] = 0;
        sprintf(buffer,"%14e",values[i]);
        char *p = buffer;
        // remove spaces at the beginning
        while(p[0] == ' ')
        {
            if(p == endbuffer)
                return False;
            p++;
        }
        ret = ret && WriteElement(i,p);
    }
    return ret;
}

bool CDBStringDataNode::WriteFloat(const float *values,uint32 size){
    if (values    == NULL) return False;

    bool ret = True;

    char buffer[64];
    char *endbuffer = &buffer[63];
    for (uint32 i = 0;i < size;i++){
        buffer[0] = 0;
        sprintf(buffer,"%14e",values[i]);
        char *p = buffer;
        // remove spaces at the beginning
        while(p[0] == ' ')
        {
            if(p == endbuffer)
                return False;
            p++;
        }
        ret = ret && WriteElement(i,p);
    }
    return ret;
}

bool CDBStringDataNode::WriteInteger(const int32 *values,uint32 size){
    if (values    == NULL) return False;

    bool ret = True;

    char buffer[64];
    for (uint32 i = 0;i < size;i++){
        buffer[0] = 0;
        sprintf(buffer,"%d",values[i]);
        ret = ret && WriteElement(i,buffer);
    }
    return ret;
}

bool CDBStringDataNode::WriteUnsignedInteger(const uint32 *values,uint32 size){
    if (values    == NULL) return False;

    bool ret = True;

    char buffer[64];
    for (uint32 i = 0;i < size;i++){
        buffer[0] = 0;
        sprintf(buffer,"%d",values[i]);
        ret = ret && WriteElement(i,buffer);
    }
    return ret;
}

bool CDBStringDataNode::WritePointer(const void **values,uint32 size){
    if (values    == NULL) return False;

    bool ret = True;

    char buffer[64];
    for (uint32 i = 0;i < size;i++){
        buffer[0] = 0;
        sprintf(buffer,"%p",values[i]);
        ret = ret && WriteElement(i,buffer);
    }
    return ret;
}


bool CDBStringDataNode::WriteChar(const char *values,uint32 size){
    if (values    == NULL) return False;

    bool ret = True;

    char buffer[2];
    for (uint32 i = 0;i < size;i++){
        buffer[1] = 0;
        buffer[0] = values[i];
        ret = ret && WriteElement(i,buffer);
    }
    return ret;
}

bool CDBStringDataNode::WriteString(const char **values,uint32 size){
    if (values    == NULL) return False;

    bool ret = True;

    for (uint32 i = 0;i < size;i++){
        if (values[i] == NULL) return False;
        ret = ret && WriteElement(i,values[i]);
    }
    return ret;
}

bool CDBStringDataNode::WriteString(const FString *values,uint32 size){
    if (values    == NULL) return False;

    bool ret = True;

    for (uint32 i = 0;i < size;i++){
        ret = ret && WriteElement(i,values[i].Buffer());
    }
    return ret;
}

bool CDBStringDataNode::WriteString(const BString *values,uint32 size){
    if (values    == NULL) return False;

    bool ret = True;

    for (uint32 i = 0;i < size;i++){
        ret = ret && WriteElement(i,values[i].Buffer());
    }
    return ret;
}


bool CDBStringDataNode::WriteFormatted(const char *formatted,const char *seps){

    if (formatted == NULL) return True;

    int index = 0;
    bool ret = True;

    SkipSeps(formatted,seps);

    const char *start = formatted;

    // read vector
    while (formatted[0] != 0){

        const char *stringEnd;
        if (formatted[0] == '"'){
            formatted++;
            stringEnd = formatted;
            FindSep(stringEnd,"\"");
        } else {
            stringEnd = formatted;
            FindSep(stringEnd,seps);
        }

        formatted = stringEnd;
        if (formatted[0] == '"') formatted++;
        SkipSeps(formatted,seps);
        index++;

    }

    CleanUp();
    SetSize(index);

    formatted = start;
    index = 0;

    // read vector
    while (formatted[0] != 0){

        const char *stringEnd;
        if (formatted[0] == '"'){
            formatted++;
            stringEnd = formatted;
            FindSep(stringEnd,"\"");
        } else {
            stringEnd = formatted;
            FindSep(stringEnd,seps);
        }
        int size = stringEnd - formatted;
        FString tempBuffer;
        ret = ret && WriteElement(index,Decode(tempBuffer,formatted,size));

        formatted = stringEnd;
        if (formatted[0] == '"') formatted++;
        SkipSeps(formatted,seps);
        index++;

    }

    return ret ;
}


/*#################################################################################################*/
/*#################################################################################################*/
/*#################################################################################################*/


static inline void WriteIndent(Streamable *s,int level){
    uint32 size = 16;
    const char *buffer = "                ";
    while (level > 0) {
        size = 16;
        if (size > (uint32)level) size = level;
        if (!s->Write(buffer,size)) return;
        level-=size;
    }
}

bool CDBStringDataNode::ReadContent(void *value,const CDBTYPE &valueType,int size,va_list argList){

    bool ret = True;
    if (value == NULL){
        AssertErrorCondition(ParametersError,"Read:value=NULL");
        return False;
    }

    int fillSize = 0;
    if ((size != NumberOfElements()) &&
        ((valueType.dataType != CDB_CDBStyle) &&
         (valueType.dataType != CDB_CDBEval)
        )){
        AssertErrorCondition(Warning,"node %s:Matrix Row size is %i not %i",Name(),NumberOfElements(),size);
        if (size > NumberOfElements()){
            fillSize = size - NumberOfElements();
            size = NumberOfElements();
        }
    }

    CDBDataType cdbdt = valueType.dataType;

    if (cdbdt == CDB_double){
            double *dv = (double *)value;
            ret = ReadDouble(dv,size);
            for (int i = 0;i < fillSize;i++) dv[i+size] = 0.0;
    } else
    if (cdbdt == CDB_Pointer){
            intptr *pv = (intptr *)value;
            ret = ReadPointer(pv,size);
            for (int i = 0;i < fillSize;i++) pv[i+size] = 0;
    } else
    if (cdbdt == CDB_float){
            float *fv = (float *)value;
            ret = ReadFloat(fv,size);
            for (int i = 0;i < fillSize;i++) fv[i+size] = 0.0;
    } else
    if (cdbdt == CDB_int32){
            int32 *di = (int32 *)value;
            ret = ReadInteger(di,size);
            for (int i = 0;i < fillSize;i++) di[i+size] = 0;
    } else
    if (cdbdt == CDB_int64){
            int64 *di = (int64 *)value;
            ret = ReadInteger64(di,size);
            for (int i = 0;i < fillSize;i++) di[i+size] = 0;
    } else
    if (cdbdt == CDB_uint32){
            uint32 *di = (uint32 *)value;
            ret = ReadUnsignedInteger(di,size);
            for (int i = 0;i < fillSize;i++) di[i+size] = 0;
    } else
    if (cdbdt == CDB_char){
            char *dc = (char *)value;
            ret = ReadChar(dc,size);
            for (int i = 0;i < fillSize;i++) dc[i+size] = ' ';
    } else
    if ((cdbdt == CDB_Content) ||
        (cdbdt == CDB_FString)){
            FString *ds = (FString *)value;
            ret = ReadString(ds,size);
            for (int i = 0;i < fillSize;i++) ds[i+size] = "";
    } else
    if (cdbdt == CDB_BString){
            BString *ds = (BString *)value;
            ret = ReadString(ds,size);
            for (int i = 0;i < fillSize;i++) ds[i+size] = "";
    } else
    if (cdbdt == CDB_String){
            char **ds = (char **)value;
            ret = ReadString(ds,size);
            for (int i = 0;i < fillSize;i++) ds[i+size] = NULL;
    } else
    if ((cdbdt == CDB_CDBEval) ||
        (cdbdt == CDB_CDBStyle)){
            uint32          indentChars = va_arg(argList,int);
            uint32          maxElements = va_arg(argList,int);
            CDBWriteMode    mode        = (CDBWriteMode)va_arg(argList,int);
            CDBWriteMode    wmode       = (CDBWriteMode)(mode & CDBWM_Modes);

//            Streamable **ds = (Streamable **)value;
//            Streamable *stream = ds[0];
            Streamable *stream = (Streamable *)value;
            if (stream == NULL) return False;

            if ((!(mode & (CDBWM_NoIndent | CDBWM_NameJoin)))
               &&  (wmode == CDBWM_Tree)) {
                WriteIndent(stream,indentChars);
            }
            stream->Printf("%s = ",Name());
            ret = ReadFormatted(stream,indentChars,maxElements);

            if (wmode == CDBWM_Tree) {
                stream->PutC('\n');
            } else
            if (wmode == CDBWM_Comma){
                stream->PutC(',');
            }

    } else
    if (cdbdt == CDB_Interpret){
            AssertErrorCondition(ParametersError,"ReadEntry:unsupported data type CDB_Interpret");
            return False;
    } else {
            BString bs;
            BasicTypeDescriptor btd(0,BTDTString,BTDSTCString);
            bool ret = False;
            if (size > 1) {
                ret = BTConvert(size,cdbdt,value,btd,elements);
            } else
            if (size == 1){
                char *p[1] = { (char *)elements };
                ret = BTConvert(size,cdbdt,value,btd,p);
            }

            if (!ret) AssertErrorCondition(ParametersError,"ReadContent:unknown data type %i",valueType.dataType.Value());
            return ret;
        }
    return ret;
}

bool CDBStringDataNode::ReadDouble(double *values,uint32 size){
    if (values    == NULL) return False;

    for (uint32 i = 0;i < size;i++){
        const char *value = ReadElement(i);
        if (value == NULL) return False;
        values[i] = stringToDouble(value);
    }
    return True;
}

bool CDBStringDataNode::ReadFloat(float *values,uint32 size){
    if (values    == NULL) return False;

    for (uint32 i = 0;i < size;i++){
        const char *value = ReadElement(i);
        if (value == NULL) return False;
        values[i] = stringToDouble(value);
    }
    return True;
}

bool CDBStringDataNode::ReadInteger(int32 *values,uint32 size){
    if (values    == NULL) return False;

    for (uint32 i = 0;i < size;i++){
        const char *value = ReadElement(i);
        if (value == NULL) return False;
        values[i] = stringToInteger32(value);
    }
    return True;
}

bool CDBStringDataNode::ReadInteger64(int64 *values,uint32 size){
    if (values    == NULL) return False;

    for (uint32 i = 0;i < size;i++){
        const char *value = ReadElement(i);
        if (value == NULL) return False;
        values[i] = stringToInteger64(value);
    }
    return True;
}

bool CDBStringDataNode::ReadPointer(intptr *values,uint32 size){
    if (values    == NULL) return False;

    for (uint32 i = 0;i < size;i++){
        const char *value = ReadElement(i);
        if (value == NULL) return False;
        values[i] = stringToPointer(value);
    }
    return True;
}

bool CDBStringDataNode::ReadUnsignedInteger(uint32 *values,uint32 size){
    if (values    == NULL) return False;

    for (uint32 i = 0;i < size;i++){
        const char *value = ReadElement(i);
        if (value == NULL) return False;
        values[i] = stringToInteger32(value);
    }
    return True;
}

bool CDBStringDataNode::ReadChar(char *values,uint32 size){
    if (values    == NULL) return False;

    for (uint32 i = 0;i < size;i++){
        const char *value = ReadElement(i);
        if (value == NULL) return False;
        values[i] = value[0];
    }
    return True;
}

bool CDBStringDataNode::ReadString(char **values,uint32 size){
    if (values    == NULL) return False;

    for (uint32 i = 0;i < size;i++){
        if (values[i] != NULL) free((void *&)values[i]);
        const char *value = ReadElement(i);
        if (value == NULL) return False;
        values[i] = strdup(value);
    }
    return True;
}

bool CDBStringDataNode::ReadString(FString *values,uint32 size){
    if (values    == NULL) return False;

    for (uint32 i = 0;i < size;i++){
        const char *data = ReadElement(i);
        if (data == NULL) return False;
        values[i] = data;
    }
    return True;
}

bool CDBStringDataNode::ReadString(BString *values,uint32 size){
    if (values    == NULL) return False;

    for (uint32 i = 0;i < size;i++){
        const char *data = ReadElement(i);
        if (data == NULL) return False;
        values[i] = data;
    }
    return True;
}

bool CDBStringDataNode::ReadFormatted(Streamable *formatted,uint32 indentChars,uint32 maxElements){
    if (maxElements <= 0) maxElements = 1000000000;

    uint32 numberOfElements = NumberOfElements();
    if (numberOfElements == 0) return False;
    //If the variable name is a number it is likely to be an array. Put the {
    if (numberOfElements > 1 || IsNumber(Name())) formatted->PutC('{');
    if (numberOfElements > maxElements) {
        formatted->PutC('\n');
        indentChars++;
        for (uint32 i = 0;i < indentChars; i++) {
            formatted->PutC(' ');
        }
    }

    for (uint32 i = 0; i < numberOfElements; i++){

        const char *el = ReadElement(i);
        if (el != NULL) {

            FString formattedElement;
            if (Encode(formattedElement,el) == NULL) return False;

            bool needsQuotes = NeedsQuotes(el);
            if (needsQuotes) formatted->PutC('"');
            uint32 size = formattedElement.Size();
            formatted->Write(formattedElement.Buffer(),size);
            if (needsQuotes) formatted->PutC('"');

            if ((i+1) < numberOfElements){
                if (((i % maxElements) == (maxElements-1)) &&
                    (i != (numberOfElements-1))){
                    formatted->PutC('\n');
                    for (uint32 j = 0;j < indentChars; j++) {
                        formatted->PutC(' ');
                    }
                } else {
                    formatted->PutC(' ');
                }
            }
        } else {
            AssertErrorCondition(Warning,"ReadFormatted: element %i of %s is NULL",i,Name());
        }
    }

    if (numberOfElements > maxElements) {
        formatted->PutC('\n');
        indentChars--;
        for (uint32 i = 0;i < indentChars; i++) {
            formatted->PutC(' ');
        }
    }
    //If the variable name is a number it is likely to be an array. Close the }
    if (numberOfElements > 1 || IsNumber(Name())) formatted->PutC('}');

    return True;
}


bool CDBStringDataNode::WriteArray(const char *configName,const void *array,const CDBTYPE &valueType,const int *size,int nDim,CDBNMode functionMode,SortFilterFn *sortFn){
    if ((configName != NULL) && (strlen(configName)>0)){
        return False;
    }

    if (nDim < 0) return False;

    int totalSize = nDim;

    if (size != NULL){
        totalSize = 1;
        int i;
        for (i = 0;i < nDim;i++){
            if (size[i] > 0){
                totalSize *= size[i];
            } else {
                AssertErrorCondition(Warning,"WriteArray: size[%i] contains %i which is less than 1", i,size[i]);
            }
        }
    }

    return WriteContent(array,valueType,totalSize);

}

// must perform recursive calls of virtual ReadArray to allow implementing custom array handling

bool CDBStringDataNode::ReadArray(const char *configName,void *array,const CDBTYPE &valueType,const int *size,int nDim, bool caseSensitive){
    if ((configName != NULL) && (strlen(configName)>0)){
        return False;
    }

    if (nDim < 0) return False;

    int totalSize = nDim;
    if ( nDim == 0 ) totalSize = 1;

    if (size != NULL){
        totalSize = 1;
        int i;
        for (i = 0;i < nDim;i++){
            if (size[i] > 0){
                totalSize *= size[i];
            } else {
                AssertErrorCondition(Warning,"ReadArray: size[%i] contains %i which is less than 1", i,size[i]);
            }
        }
    }


    return CDBNode::ReadContent(array,valueType,totalSize,0,-1,0);
}




