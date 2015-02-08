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
 * $Id: Endianity.h 3 2012-01-15 16:26:07Z aneto $
 *
**/

/** 
 * @file
 * A NameSpace holder of all the Endianity conversion routines. 
 */
#ifndef ENDIANITY_H
#define ENDIANITY_H

#include INCLUDE_FILE_ARCHITECTURE(ARCHITECTURE,EndianityA.h)
class Endianity {
private:
    /**
     * Set to True when the Endianity::Type() is called for the first time
     */
    static bool endianityTypeChecked;
    /**
     * The target machine endianity
     */
    static uint8 endianityType;
public:
    /**
     * Definition of little endian
     */
    static const uint8 LITTLE_ENDIAN = 1;
    /**
     * Definition of big endian
     */
    static const uint8 BIG_ENDIAN = 2;
    /**
     * @return LITTLE_ENDIAN if the target architecture is little endian. BIG_ENDIAN if the target architecture is big endian.
     */
    static inline uint8 Type(){
        if(!endianityTypeChecked){
            uint16 value = 0xAABB;
            uint16 test  = value;
            ToLittleEndian(test);
            (test == value) ? endianityType = LITTLE_ENDIAN : endianityType = BIG_ENDIAN; //If the value has not changed then this platform is little endian
            endianityTypeChecked = True;
        }
        return endianityType;
    }
    /**
     * Converts a number from big endian to the target operating system endianity
     * @param x the number to convert
     */
    static inline void FromBigEndian(volatile double &x){ EndianityFromBigEndianDouble(x); }
    /**
     * Converts a number from big endian to the target operating system endianity
     * @param x the number to convert
     */
    static inline void FromBigEndian(volatile float &x) { EndianityFromBigEndianFloat(x); }
    /**
     * Converts a number from big endian to the target operating system endianity
     * @param x the number to convert
     */
    static inline void FromBigEndian(volatile uint64 &x){ EndianityFromBigEndianUInt64(x); }
    /**
     * Converts a number from big endian to the target operating system endianity
     * @param x the number to convert
     */
    static inline void FromBigEndian(volatile uint32 &x){ EndianityFromBigEndianUInt32(x); }
    /**
     * Converts a number from big endian to the target operating system endianity
     * @param x the number to convert
     */
    static inline void FromBigEndian(volatile uint16 &x){ EndianityFromBigEndianUInt16(x); }
    /**
     * Converts a number from big endian to the target operating system endianity
     * @param x the number to convert
     */
    static inline void FromBigEndian(volatile int64 &x) { EndianityFromBigEndianInt64(x); }
    /**
     * Converts a number from big endian to the target operating system endianity
     * @param x the number to convert
     */
    static inline void FromBigEndian(volatile int32 &x) { EndianityFromBigEndianInt32(x); }
    /**
     * Converts a number from big endian to the target operating system endianity
     * @param x the number to convert
     */
    static inline void FromBigEndian(volatile int16 &x) { EndianityFromBigEndianInt16(x); }
    /**
     * Converts a number from little endian to the target operating system endianity
     * @param x the number to convert
     */
    static inline void FromLittleEndian(volatile double &x){ EndianityFromLittleEndianDouble(x); }
    /**
     * Converts a number from little endian to the target operating system endianity
     * @param x the number to convert
     */
    static inline void FromLittleEndian(volatile float &x) { EndianityFromLittleEndianFloat(x); }
    /**
     * Converts a number from little endian to the target operating system endianity
     * @param x the number to convert
     */
    static inline void FromLittleEndian(volatile uint64 &x){ EndianityFromLittleEndianUInt64(x); }
    /**
     * Converts a number from little endian to the target operating system endianity
     * @param x the number to convert
     */
    static inline void FromLittleEndian(volatile uint32 &x){ EndianityFromLittleEndianUInt32(x); };
    /**
     * Converts a number from little endian to the target operating system endianity
     * @param x the number to convert
     */
    static inline void FromLittleEndian(volatile uint16 &x){ EndianityFromLittleEndianUInt16(x); };
    /**
     * Converts a number from little endian to the target operating system endianity
     * @param x the number to convert
     */
    static inline void FromLittleEndian(volatile int64 &x) { EndianityFromLittleEndianInt64(x); };
    /**
     * Converts a number from little endian to the target operating system endianity
     * @param x the number to convert
     */
    static inline void FromLittleEndian(volatile int32 &x) { EndianityFromLittleEndianInt32(x); };
    /**
     * Converts a number from little endian to the target operating system endianity
     * @param x the number to convert
     */
    static inline void FromLittleEndian(volatile int16 &x) { EndianityFromLittleEndianInt16(x); };
    /** 
     * Converts a number from the target operating system endianity to big endian
     * @param x the number to convert
     */
    static inline void ToBigEndian(volatile double &x){ EndianityToBigEndianDouble(x); }
    /** 
     * Converts a number from the target operating system endianity to big endian
     * @param x the number to convert
     */
    static inline void ToBigEndian(volatile float &x) { EndianityToBigEndianFloat(x); }
    /** 
     * Converts a number from the target operating system endianity to big endian
     * @param x the number to convert
     */
    static inline void ToBigEndian(volatile uint64 &x){ EndianityToBigEndianUInt64(x); }
    /** 
     * Converts a number from the target operating system endianity to big endian
     * @param x the number to convert
     */
    static inline void ToBigEndian(volatile uint32 &x){ EndianityToBigEndianUInt32(x); }
    /** 
     * Converts a number from the target operating system endianity to big endian
     * @param x the number to convert
     */
    static inline void ToBigEndian(volatile uint16 &x){ EndianityToBigEndianUInt16(x); }
    /** 
     * Converts a number from the target operating system endianity to big endian
     * @param x the number to convert
     */
    static inline void ToBigEndian(volatile int64 &x) { EndianityToBigEndianInt64(x); }
    /** 
     * Converts a number from the target operating system endianity to big endian
     * @param x the number to convert
     */
    static inline void ToBigEndian(volatile int32 &x) { EndianityToBigEndianInt32(x); }
    /** 
     * Converts a number from the target operating system endianity to big endian
     * @param x the number to convert
     */
    static inline void ToBigEndian(volatile int16 &x) { EndianityToBigEndianInt16(x); }
    /** 
     * Converts a number from the target operating system endianity to little endian
     * @param x the number to convert
     */
    static inline void ToLittleEndian(volatile double &x){ EndianityToLittleEndianDouble(x); };
    /** 
     * Converts a number from the target operating system endianity to little endian
     * @param x the number to convert
     */
    static inline void ToLittleEndian(volatile float &x) { EndianityToLittleEndianFloat(x); };
    /** 
     * Converts a number from the target operating system endianity to little endian
     * @param x the number to convert
     */
    static inline void ToLittleEndian(volatile uint64 &x){ EndianityToLittleEndianUInt64(x); };
    /** 
     * Converts a number from the target operating system endianity to little endian
     * @param x the number to convert
     */
    static inline void ToLittleEndian(volatile uint32 &x){ EndianityToLittleEndianUInt32(x); };
    /** 
     * Converts a number from the target operating system endianity to little endian
     * @param x the number to convert
     */
    static inline void ToLittleEndian(volatile uint16 &x){ EndianityToLittleEndianUInt16(x); };
    /** 
     * Converts a number from the target operating system endianity to little endian
     * @param x the number to convert
     */
    static inline void ToLittleEndian(volatile int64 &x) { EndianityToLittleEndianInt64(x); };
    /** 
     * Converts a number from the target operating system endianity to little endian
     * @param x the number to convert
     */
    static inline void ToLittleEndian(volatile int32 &x) { EndianityToLittleEndianInt32(x); };
    /** 
     * Converts a number from the target operating system endianity to little endian
     * @param x the number to convert
     */
    static inline void ToLittleEndian(volatile int16 &x) { EndianityToLittleEndianInt16(x); };
    /** 
     * Copies a block of memory and converts from big endian to the target operating system endianity
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyFromBigEndian(double *dest,const double *src,uint32 size) { EndianityMemCopyFromBigEndianDouble(dest,src,size); }
    /** 
     * Copies a block of memory and converts from big endian to the target operating system endianity
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyFromBigEndian(uint64 *dest,const uint64 *src,uint32 size) { EndianityMemCopyFromBigEndianUInt64(dest,src,size); }
    /** 
     * Copies a block of memory and converts from big endian to the target operating system endianity
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyFromBigEndian(int64 *dest,const int64 *src,uint32 size) { EndianityMemCopyFromBigEndianInt64(dest,src,size); }
    /** 
     * Copies a block of memory and converts from big endian to the target operating system endianity
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyFromBigEndian(float *dest,const float *src,uint32 size) { EndianityMemCopyFromBigEndianFloat(dest,src,size); }
    /** 
     * Copies a block of memory and converts from big endian to the target operating system endianity
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyFromBigEndian(uint32 *dest,const uint32 *src,uint32 size) { EndianityMemCopyFromBigEndianUInt32(dest,src,size); }
    /** 
     * Copies a block of memory and converts from big endian to the target operating system endianity
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyFromBigEndian(uint16 *dest,const uint16 *src,uint32 size) { EndianityMemCopyFromBigEndianUInt16(dest,src,size); }
    /** 
     * Copies a block of memory and converts from big endian to the target operating system endianity
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyFromBigEndian(int32 *dest,const int32 *src,uint32 size)   { EndianityMemCopyFromBigEndianInt32(dest,src,size); }
    /** 
     * Copies a block of memory and converts from big endian to the target operating system endianity
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyFromBigEndian(int16 *dest,const int16 *src,uint32 size)   { EndianityMemCopyFromBigEndianInt16(dest,src,size); }
    /** 
     * Copies a block of memory and converts from little endian to the target operating system endianity
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyFromLittleEndian(double *dest,const double *src,uint32 size)  { EndianityMemCopyFromLittleEndianDouble(dest,src,size); }
    /** 
     * Copies a block of memory and converts from little endian to the target operating system endianity
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyFromLittleEndian(uint64 *dest,const uint64 *src,uint32 size)  { EndianityMemCopyFromLittleEndianUInt64(dest,src,size); }
    /** 
     * Copies a block of memory and converts from little endian to the target operating system endianity
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyFromLittleEndian(int64 *dest,const int64 *src,uint32 size)  { EndianityMemCopyFromLittleEndianInt64(dest,src,size); }
    /** 
     * Copies a block of memory and converts from little endian to the target operating system endianity
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyFromLittleEndian(float *dest,const float *src,uint32 size)  { EndianityMemCopyFromLittleEndianFloat(dest,src,size); }
    /** 
     * Copies a block of memory and converts from little endian to the target operating system endianity
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyFromLittleEndian(uint32 *dest,const uint32 *src,uint32 size) { EndianityMemCopyFromLittleEndianUInt32(dest,src,size); }
    /** 
     * Copies a block of memory and converts from little endian to the target operating system endianity
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyFromLittleEndian(uint16 *dest,const uint16 *src,uint32 size) { EndianityMemCopyFromLittleEndianUInt16(dest,src,size); }
    /** 
     * Copies a block of memory and converts from little endian to the target operating system endianity
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyFromLittleEndian(int32 *dest,const int32 *src,uint32 size)   { EndianityMemCopyFromLittleEndianInt32(dest,src,size); }
    /** 
     * Copies a block of memory and converts from little endian to the target operating system endianity
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyFromLittleEndian(int16 *dest,const int16 *src,uint32 size)   { EndianityMemCopyFromLittleEndianInt16(dest,src,size); }
    /** 
     * Copies a block of memory and converts from the target operating system endianity to big endian
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyToBigEndian(double *dest,const double *src,uint32 size)   { EndianityMemCopyToBigEndianDouble(dest,src,size); }
    /** 
     * Copies a block of memory and converts from the target operating system endianity to big endian
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyToBigEndian(uint64 *dest,const uint64 *src,uint32 size)   { EndianityMemCopyToBigEndianUInt64(dest,src,size); }
    /** 
     * Copies a block of memory and converts from the target operating system endianity to big endian
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyToBigEndian(int64 *dest,const int64 *src,uint32 size)   { EndianityMemCopyToBigEndianInt64(dest,src,size); }
    /** 
     * Copies a block of memory and converts from the target operating system endianity to big endian
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyToBigEndian(float *dest,const float *src,uint32 size)   { EndianityMemCopyToBigEndianFloat(dest,src,size); }
    /** 
     * Copies a block of memory and converts from the target operating system endianity to big endian
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyToBigEndian(uint32 *dest,const uint32 *src,uint32 size) { EndianityMemCopyToBigEndianUInt32(dest,src,size); }
    /** 
     * Copies a block of memory and converts from the target operating system endianity to big endian
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyToBigEndian(uint16 *dest,const uint16 *src,uint32 size) { EndianityMemCopyToBigEndianUInt16(dest,src,size); }
    /** 
     * Copies a block of memory and converts from the target operating system endianity to big endian
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyToBigEndian(int32 *dest,const int32 *src,uint32 size)   { EndianityMemCopyToBigEndianInt32(dest,src,size); }
    /** 
     * Copies a block of memory and converts from the target operating system endianity to big endian
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyToBigEndian(int16 *dest,const int16 *src,uint32 size)   { EndianityMemCopyToBigEndianInt16(dest,src,size); }
    /** 
     * Copies a block of memory and converts from the target operating system endianity to little endian
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyToLittleEndian(double *dest,const double *src,uint32 size)   { EndianityMemCopyToLittleEndianDouble(dest,src,size); }
    /** 
     * Copies a block of memory and converts from the target operating system endianity to little endian
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyToLittleEndian(uint64 *dest,const uint64 *src,uint32 size)   { EndianityMemCopyToLittleEndianUInt64(dest,src,size); }
    /** 
     * Copies a block of memory and converts from the target operating system endianity to little endian
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyToLittleEndian(int64 *dest,const int64 *src,uint32 size)   { EndianityMemCopyToLittleEndianInt64(dest,src,size); }
    /** 
     * Copies a block of memory and converts from the target operating system endianity to little endian
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyToLittleEndian(float *dest,const float *src,uint32 size)   { EndianityMemCopyToLittleEndianFloat(dest,src,size); }
    /** 
     * Copies a block of memory and converts from the target operating system endianity to little endian
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyToLittleEndian(uint32 *dest,const uint32 *src,uint32 size) { EndianityMemCopyToLittleEndianUInt32(dest,src,size); }
    /** 
     * Copies a block of memory and converts from the target operating system endianity to little endian
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyToLittleEndian(uint16 *dest,const uint16 *src,uint32 size) { EndianityMemCopyToLittleEndianUInt16(dest,src,size); }
    /** 
     * Copies a block of memory and converts from the target operating system endianity to little endian
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyToLittleEndian(int32 *dest,const int32 *src,uint32 size)   { EndianityMemCopyToLittleEndianInt32(dest,src,size); }
    /** 
     * Copies a block of memory and converts from the target operating system endianity to little endian
     * @param dest the destination
     * @param src the source
     * @param size the number of elements
     */
    static inline void MemCopyToLittleEndian(int16 *dest,const int16 *src,uint32 size)   { EndianityMemCopyToLittleEndianInt16(dest,src,size); }
};
#endif

