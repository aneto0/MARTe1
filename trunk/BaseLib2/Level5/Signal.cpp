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

#include "Signal.h"

OBJECTLOADREGISTER(Signal,"$Id$")

/** re-allocate memory
    returns False on error  */
bool                    Signal::Allocate(
        BasicTypeDescriptor         type,
        uint32                      numberOfSamples,
	MemoryAllocationFlags       allocFlags)
{
    if ((buffer != NULL) && (bufferByteSize > 0)){
        free ((void *&)buffer);
        bufferByteSize = 0;
    }
    this->numberOfSamples = 0;

    this->type            = type;

    // check new size
    if (numberOfSamples == 0) return True;

    if ((type.Type() != BTDTInteger) &&
        (type.Type() != BTDTFloat)){
        AssertErrorCondition(FatalError,"Allocate: unsupported data type format for object %s",Name());
        return False;
    }

    bufferByteSize  = (numberOfSamples * type.BitSize() + 31)/32;
    bufferByteSize *= sizeof(uint32);

    buffer = (char *)malloc(bufferByteSize, allocFlags);

    if (buffer == NULL) {
        bufferByteSize = 0;
        this->numberOfSamples = 0;
        return False;
    }
    this->numberOfSamples   = numberOfSamples;
    return True;
}

/** use an existing allocated buffer  */
bool                    Signal::Refer(
        BasicTypeDescriptor         type,
        uint32                      numberOfSamples,
        void *                      buffer)
{
    if ((this->buffer != NULL) && (bufferByteSize > 0)){
        free ((void *&)this->buffer);
        bufferByteSize = 0;
    }
    this->buffer            = NULL;
    this->numberOfSamples   = 0;
    this->type              = type;

    // check new size
    if (numberOfSamples == 0) return True;

    if ((type.Type() != BTDTInteger) &&
        (type.Type() != BTDTFloat)){
        AssertErrorCondition(FatalError,"Refer: unsupported data type format for object %s",Name());
        return False;
    }

    bufferByteSize = (numberOfSamples * type.BitSize() + 31)/32;
    bufferByteSize      *= sizeof(uint32);
    bufferByteSize      *= -1; // mark as referred

    if (buffer == NULL) {
        bufferByteSize = 0;
        this->numberOfSamples = 0;
        return False;
    }
    this->buffer = buffer;
    this->numberOfSamples   = numberOfSamples;
    return True;
}


/** constructor */
                        Signal::Signal()
{
    type            = BTDInt32;
    numberOfSamples = 0;
    bufferByteSize  = 0;
    buffer          = NULL;
};

/** destructor */
                        Signal::~Signal()
{
    Refer(BTDInt32,0,NULL);
};

/** initialise the object by copying from a buffer */
bool                    Signal::CopyData(
            BasicTypeDescriptor         type            ,
            uint32                      numberOfSamples ,
            const void *                buffer,
	    MemoryAllocationFlags       allocFlags)
{
 
    if (!Allocate(type,numberOfSamples,allocFlags)) {
        return False;
    }

    if (buffer != NULL){
        memcpy(this->buffer, buffer, bufferByteSize);
    }

    return True;
}

/** initialise the object by referring to an existing buffer */
bool                    Signal::ReferData(
            BasicTypeDescriptor         type            ,
            uint32                      numberOfSamples ,
            void *                      buffer
        )
{
    if (!Refer(type,numberOfSamples,buffer)) {
        return False;
    }
    return True;
}

/** copy constructor */
                        Signal::Signal(
            const Signal &              signal)
{
    SetObjectName(signal.Name());
    CopyData(signal.type,signal.numberOfSamples,signal.buffer);
}

/** Initialise the object from a cdb */
bool                    Signal::ObjectLoadSetup(
            ConfigurationDataBase &     info,
            StreamInterface *           err)
{
    bool ret = GCNamedObject::ObjectLoadSetup(info,err);

    CDBExtended cdbx(info);

    BString temp;
    if (!cdbx.ReadBString(temp,"DataType")){
        AssertErrorCondition(FatalError,"Cannot load DataType for object %s",Name());
        return False;
    }
    BasicTypeDescriptor newType;
    if (!newType.ConvertFromString(temp.Buffer())){
        AssertErrorCondition(FatalError,"Cannot interpret DataType %s for object %s",temp.Buffer(),Name());
        return False;
    }

    int sizes[2] = { 0, 0 };
    int maxDim = 1;

    if (!info->GetArrayDims(sizes,maxDim,"Data",CDBAIM_Strict)){
        AssertErrorCondition(FatalError,"Cannot get dimension for Data in object %s",Name());
        return False;
    }

    
    if (!Allocate(newType,sizes[0])){
        AssertErrorCondition(FatalError,"Cannot allocate memory for Data in object %s",Name());
        return False;
    }

    CDBTYPE custom(type,type.ByteSize(),NULL);
    if (!info->ReadArray(buffer,custom,sizes,1,"Data")){
        AssertErrorCondition(FatalError,"Cannot save Data for object %s",Name());
        ret = False;
    }

    return ret;
}

/** Save the object to a cdb */
bool                    Signal::ObjectSaveSetup(
            ConfigurationDataBase &     info,
            StreamInterface *           err)
{
    bool ret = GCNamedObject::ObjectSaveSetup(info,err);

    CDBExtended cdbx(info);

    BString temp;
    if (!cdbx.WriteString(type.ConvertToString(temp),"DataType")){
        AssertErrorCondition(FatalError,"Cannot save DataType for object %s",Name());
        ret = False;
    }

    CDBTYPE custom(type,type.ByteSize(),NULL);
    int sizes[2] = { numberOfSamples, 0 };
    if (!cdbx->WriteArray(buffer,custom,sizes,1,"Data")){
        AssertErrorCondition(FatalError,"Cannot save Data for object %s",Name());
        ret = False;
    }

    return ret;
}


