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
 * Class to store array data in a certain format (data type)
 * allowing for data requests in various other formats
 */
#ifndef __BASIC_TYPE_DATA__
#define __BASIC_TYPE_DATA__

#include "System.h"
#include "BasicTypes.h"

class BasicTypeData {

private:

    // Data holder
    void               *data;

    // Type descriptor of the data value
    BasicTypeDescriptor dataType;

    // Data array length
    uint32              dataLength;

    // Data array size in bytes
    uint32              dataByteSize;

public:

    /**
     * Class Construtor of the object
     * @param type a BasicTypeDescriptor describing the data type
     * @param length a uint32 holding the array length
     */
    BasicTypeData(BasicTypeDescriptor type = BTDTNone, uint32 length = 1) {
        /// Check for negative and zero lengths
        if(length <= 0) {
            CStaticAssertErrorCondition(FatalError, "BasicTypeData(): length <= 0");
            return;
        }
        
        dataLength = length;
        if(type != BTDTNone) {
            dataType     = type;
            dataByteSize = dataLength*(dataType.ByteSize());
            if((data = (void *)malloc(dataByteSize)) == NULL) {
            CStaticAssertErrorCondition(FatalError, "BasicTypeData(): Unable to allocate memory");
            return;
            }
        } else {
            dataType     = BTDTNone;
            dataByteSize = 0;
            data         = NULL;
        }
    };

    /**
     * Copy constructor
     */
    BasicTypeData(const BasicTypeData &btd) {
        // Basic copy
        dataType     = btd.dataType;
        dataLength   = btd.dataLength;
        dataByteSize = btd.dataByteSize;
        
        // Allocate memory
        if((data = (void *)malloc(dataByteSize)) == NULL) {
            CStaticAssertErrorCondition(FatalError, "BasicTypeData(const BasicTypeData &): Unable to allocate memory");
            return;
        }
        // Fill the data
        memcpy(data, btd.data, dataByteSize);
    }
    
    /**
     * Class Destructor
     */
    ~BasicTypeData() {
        if(data != NULL) {
            free((void*&)data);
        }
    };

    /**
     * Assignment operator
     */
    void operator=(const BasicTypeData &btd) {
        if(this->data == NULL) {
            if((this->data = (void *)malloc(btd.dataByteSize)) == NULL) {
            CStaticAssertErrorCondition(FatalError, "BasicTypeData: operator=(): Unable to allocate memory");
            return;
            }
        } else if(this->dataByteSize != btd.dataByteSize) {
            if(realloc(this->data, btd.dataByteSize) == NULL) {
            CStaticAssertErrorCondition(FatalError, "BasicTypeData: operator=(): Unable to reallocate memory");
            return;
            }
        }         
        // Copy basic data
        this->dataType     = btd.dataType;
        this->dataLength   = btd.dataLength;
        this->dataByteSize = btd.dataByteSize;
        
        // Copy data array
        memcpy(this->data, btd.data, this->dataByteSize);
    }

    /**
     * Check equal operator
     */
    bool operator==(const BasicTypeData &btd) const {
        bool ret = True;
        ret &= (this->dataType     == btd.dataType);
        ret &= (this->dataLength   == btd.dataLength);
        ret &= (this->dataByteSize == btd.dataByteSize);

        // Only do this check if so far the objects are equal
        // Check data arrays byte by byte
        if(ret == True) {
            /// Warning: this assumes that sizeof(char) == 1
            for(int i = 0 ; i < dataByteSize ; i++)
            ret &= (((const char)*((const char *)(this->data)+i)) == ((const char)*((const char *)(btd.data)+i)));
        }
        
        return ret;
    }

    /**
     * Check not equal operator
     */
    bool operator!=(const BasicTypeData &btd) const {
        return !(this->operator==(btd));
    }

    /**
     * Update data values
     * @param value pointer to data to be stored
     * @return True if successful, False otherwise
     */
    inline bool UpdateData(const void *value) {
        /// Check passed pointer
        if(value == NULL) {
            CStaticAssertErrorCondition(FatalError, "BasicTypeData: UpdateData(): passed pointer == NULL");
            return False;
        }
        /// Check local data pointer
        if(data == NULL) {
            CStaticAssertErrorCondition(FatalError, "BasicTypeData: UpdateData(): local data pointer == NULL");
            return False;
        }
        /// Check data size in bytes
        if(dataByteSize <= 0) {
            CStaticAssertErrorCondition(FatalError, "BasicTypeData: UpdateData(): data size in bytes <= 0");
            return False;
        }
        
        /// Copy all data cross
        memcpy(data, value, dataByteSize);
        
        return True;
    };
    
    /**
     * @param btd BasicTypeDescriptor to be used as the source
     * @return True if successful, False otherwise
     */
    inline bool UpdateData(BasicTypeData &btd) {   
        /// Copy all data cross
        memcpy(data, btd.data, dataByteSize);
        return True;
    }

    /** 
     * Update object data type and/or length
     * @param type type of data to be stored
     * @param length length of data to be stored
     * @return True if successful, False otherwise
     */
    inline bool SetTypeAndLength(BasicTypeDescriptor type, uint32 length) {
        if(type == BTDTNone) {
            CStaticAssertErrorCondition(FatalError, "BasicTypeData: SetTypeAndLength(): data type == BTDTNone");
            return False;
        }
        if(length <= 0) {
            CStaticAssertErrorCondition(FatalError, "BasicTypeData: SetTypeAndLength(): data length <= 0");
            return False;
        }
        
        dataType     = type;
        dataLength   = length;
        dataByteSize = dataLength*(dataType.ByteSize());

        /// Allocate memory to hold data
        if(data == NULL) {
            if((data = (void *)malloc(dataByteSize)) == NULL) {
            CStaticAssertErrorCondition(FatalError, "BasicTypeData: SetTypeAndLength(): Unable to reallocate memory");
            return False;
            }
        } else {
            if(realloc(data, dataByteSize) == NULL) {
            CStaticAssertErrorCondition(FatalError, "BasicTypeData: SetTypeAndLength(): Unable to reallocate memory");
            return False;
            }
        }

        return True;
    }

    /** 
     * Update object data type
     * @param type type of data to be stored
     * @return True if successful, False otherwise
     */
    inline bool SetType(BasicTypeDescriptor type) {
        return SetTypeAndLength(type, dataLength);
    }

    /**
     * Update object data length
     * @param length length of data to be stored
     * @return True if successful, False otherwise
     */
    inline bool SetLength(uint32 length) {
        return SetTypeAndLength(dataType, length);
    }

    /** Get local data array
     * @param btd BasicTypeDescriptor of the data type to be sent
     * @param output pointer to hold the data
     * @return True if successful, False otherwise
     */
    inline bool GetData(const BasicTypeDescriptor btd, void *output) {

        // Check if output and/or local data pointer's are NULL
        if(btd == BTDTNone) {
            CStaticAssertErrorCondition(FatalError, "BasicTypeData: GetData(): data type == BTDTNone");
            return False;
        }
            /// Check if output and/or local data pointer's are NULL
        if(output == NULL) {
            CStaticAssertErrorCondition(FatalError, "BasicTypeData: GetData(): output value pointer == NULL");
            return False;
        }
        if(data == NULL) {
            CStaticAssertErrorCondition(FatalError, "BasicTypeData: GetData(): local data pointer == NULL");
            return False;
        }
    	return BTConvert(dataLength, btd, output, dataType, data);
    }

    /** 
     * Retrieve stored data type
     */
    inline BasicTypeDescriptor &GetDataType() {
        return dataType;
    }

    /** 
     * Retrieve stored data length
     */
    inline uint32 GetDataLength() {
        return dataLength;
    }

    /**
     * Retrieve stored data size in bytes
     */
    inline uint32 GetDataByteSize() {
        return dataByteSize;
    }
};
#endif

