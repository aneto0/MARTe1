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
 * $Id: $
 *
 **/
/**
 * @class EndianityTest
 * @brief Tests all the Endianity class functions.
 * This class is designed to test all the Endianity functions,
 * for all the supported MARTe types.
 *
 */
#ifndef ENDIANITY_TEST_H
#define ENDIANITY_TEST_H

#include "Endianity.h"

template<class T>
class EndianityTest {
private:
    T testValue;
    T testArray[3];

public:
    /**
     * @param testValue the value to be tested by the all
     * the test functions.
     */
    EndianityTest(T testValue) {
        this->testValue = testValue;
        this->testArray[0] = testValue;
        this->testArray[1] = testValue + 1;
        this->testArray[2] = testValue - 1;
    }

    /**
     * @brief Converts the testValue to little endian and to big endian.
     *
     * If the target architecture is little endian:
     * - the test value and the little endian value must be the same
     * - the test value and the big endian value must be different
     * If the target architecture is big endian:
     * - the test value and the little endian value must be different 
     * - the test value and the big endian value must the same
     * @return True if the test is successful
     */
    bool ToEndian() {
        T valueLittleEndian = testValue;
        T valueBigEndian = testValue;

        Endianity::ToLittleEndian(valueLittleEndian);
        Endianity::ToBigEndian(valueBigEndian);

        bool isLittleEndian = (testValue == valueLittleEndian);
        bool isBigEndian = (testValue == valueBigEndian);
        if (Endianity::Type() == Endianity::ENDIANITY_LITTLE_ENDIAN) {
            return isLittleEndian && !isBigEndian;
        }
        else {
            return !isLittleEndian && isBigEndian;
        }
    }

    /**
     * Converts the testValue from little endian and from big endian
     * If the target architecture is little endian:
     * - the test value and the little endian value must be the same
     * - the test value and the big endian value must be different
     * If the target architecture is big endian:
     * - the test value and the little endian value must be different 
     * - the test value and the big endian value must the same
     * @return True if the test is successful
     */
    bool FromEndian() {
        T valueLittleEndian = testValue;
        T valueBigEndian = testValue;

        Endianity::FromLittleEndian(valueLittleEndian);
        Endianity::FromBigEndian(valueBigEndian);

        bool isLittleEndian = (testValue == valueLittleEndian);
        bool isBigEndian = (testValue == valueBigEndian);
        if (Endianity::Type() == Endianity::ENDIANITY_LITTLE_ENDIAN) {
            return isLittleEndian && !isBigEndian;
        }
        else {
            return !isLittleEndian && isBigEndian;
        }
    }

    /**
     * Converts the testArray to little endian and to big endian
     * If the target architecture is little endian:
     * - each element of the test array and the little endian array must be the same
     * - each element of the test array and the big endian array must be different same
     * If the target architecture is big endian:
     * - each element of the test array and the little endian array must be different 
     * - each element of the test array and the big endian array must be the same
     * @return True if the test is successful
     */
    bool MemCopyToEndian() {
        T arrayLittleEndian[3];
        T arrayBigEndian[3];
        Endianity::MemCopyToLittleEndian((T *) arrayLittleEndian,
                                         (T *) testArray, 3);
        Endianity::MemCopyToBigEndian((T *) arrayBigEndian, (T *) testArray, 3);

        bool isLittleEndian = (testArray[0] == arrayLittleEndian[0]);
        isLittleEndian = isLittleEndian
                && (testArray[1] == arrayLittleEndian[1]);
        isLittleEndian = isLittleEndian
                && (testArray[2] == arrayLittleEndian[2]);

        bool isBigEndian = (testArray[0] == arrayBigEndian[0]);
        isBigEndian = isBigEndian && (testArray[1] == arrayBigEndian[1]);
        isBigEndian = isBigEndian && (testArray[2] == arrayBigEndian[2]);

        if (Endianity::Type() == Endianity::ENDIANITY_LITTLE_ENDIAN) {
            return isLittleEndian && !isBigEndian;
        }
        else {
            return !isLittleEndian && isBigEndian;
        }
    }

    /**
     * Converts the testArray from little endian and from big endian
     * If the target architecture is little endian:
     * - each element of the test array and the little endian array must be the same
     * - each element of the test array and the big endian array must be different same
     * If the target architecture is big endian:
     * - each element of the test array and the little endian array must be different 
     * - each element of the test array and the big endian array must be the same
     * @return True if the test is successful
     */
    bool MemCopyFromEndian() {
        T arrayLittleEndian[3];
        T arrayBigEndian[3];
        Endianity::MemCopyFromLittleEndian((T *) arrayLittleEndian,
                                           (T *) testArray, 3);
        Endianity::MemCopyFromBigEndian((T *) arrayBigEndian, (T *) testArray,
                                        3);

        bool isLittleEndian = (testArray[0] == arrayLittleEndian[0]);
        isLittleEndian = isLittleEndian
                && (testArray[1] == arrayLittleEndian[1]);
        isLittleEndian = isLittleEndian
                && (testArray[2] == arrayLittleEndian[2]);

        bool isBigEndian = (testArray[0] == arrayBigEndian[0]);
        isBigEndian = isBigEndian && (testArray[1] == arrayBigEndian[1]);
        isBigEndian = isBigEndian && (testArray[2] == arrayBigEndian[2]);

        if (Endianity::Type() == Endianity::ENDIANITY_LITTLE_ENDIAN) {
            return isLittleEndian && !isBigEndian;
        }
        else {
            return !isLittleEndian && isBigEndian;
        }
    }

    /**
     * Converts the testValue to little endian and to big endian and then back
     * from little and big endian. The values should return to the original test
     * value
     * @return True if the test is successful
     */
    bool ToFromEndian() {
        T valueLittleEndian = testValue;
        T valueBigEndian = testValue;

        Endianity::ToLittleEndian(valueLittleEndian);
        Endianity::FromLittleEndian(valueLittleEndian);
        Endianity::ToBigEndian(valueBigEndian);
        Endianity::FromBigEndian(valueBigEndian);

        return (testValue == valueLittleEndian) && (testValue == valueBigEndian);
    }

    /**
     * Converts the testArray to little endian and to big endian and then back
     * from little and big endian. All the array values should return to the original test
     * value
     * @return True if the test is successful
     */
    bool MemCopyToFromEndian() {
        T arrayLittleEndian[3];
        T arrayBigEndian[3];
        T arrayToFrom[3];
        Endianity::MemCopyToLittleEndian((T *) arrayLittleEndian,
                                         (T *) testArray, 3);
        Endianity::MemCopyFromLittleEndian((T *) arrayToFrom,
                                           (T *) arrayLittleEndian, 3);

        bool okLittleEndian = (testArray[0] == arrayToFrom[0]);
        okLittleEndian = okLittleEndian && (testArray[1] == arrayToFrom[1]);
        okLittleEndian = okLittleEndian && (testArray[2] == arrayToFrom[2]);

        Endianity::MemCopyToBigEndian((T *) arrayBigEndian, (T *) testArray, 3);
        Endianity::MemCopyFromBigEndian((T *) arrayToFrom, (T *) arrayBigEndian,
                                        3);
        bool okBigEndian = (testArray[0] == arrayToFrom[0]);
        okBigEndian = okBigEndian && (testArray[1] == arrayToFrom[1]);
        okBigEndian = okBigEndian && (testArray[2] == arrayToFrom[2]);

        return (okLittleEndian && okBigEndian);
    }

    /**
     * Executes all the tests
     */
    bool All() {
        bool ok = ToEndian();
        ok = ok && FromEndian();
        ok = ok && MemCopyToEndian();
        ok = ok && MemCopyFromEndian();
        ok = ok && ToFromEndian();
        ok = ok && MemCopyToFromEndian();
        return ok;
    }
};

#endif

