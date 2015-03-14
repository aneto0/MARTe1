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
 * $Id$
 *
 **/

#ifndef ATOMIC_TEST_H
#define ATOMIC_TEST_H

#include "Atomic.h"

template<class T>
class AtomicTest {

private:
    volatile T testValue;

public:

    /**
     * @param testValue the value to be tested by the all
     * the test functions
     */
    AtomicTest(T testValue) {
        this->testValue = testValue;
    }

    /**
     * Tests the increment function
     */
    bool TestIncrement() {
        bool testResult = false;
        T auxValue = testValue;

        Atomic::Increment(&auxValue);
        testResult = (auxValue == (testValue + 1));

        return testResult;
    }

    /**
     * Tests the decrement function
     */
    bool TestDecrement() {
        bool testResult = false;
        T auxValue = testValue;

        Atomic::Decrement(&auxValue);
        testResult = (auxValue == (testValue - 1));

        return testResult;
    }

    /**
     * Tests the test and set function
     */
    bool TestTestAndSet() {
        T testVal = 0;
        bool ok = false;

        // Set the semaphore
        ok = Atomic::TestAndSet(&testVal);

        // When the semaphore is set, the test and set function should fail
        ok = ok && !Atomic::TestAndSet(&testVal);

        // Unset the semaphore and set it again
        testVal = 0;
        ok = ok && Atomic::TestAndSet(&testVal);

        return ok;
    }

    /**
     * Tests the exchange function
     */
    bool TestExchange() {
        bool testResult = false;
        T auxValue = testValue;

        Atomic::Exchange(&auxValue, 1234);
        testResult = (auxValue == 1234);

        return testResult;
    }

    /**
     * Tests the addition function
     */
    bool TestAdd() {
        bool testResult = false;
        T auxValue = testValue;

        Atomic::Add(&auxValue, 10);
        testResult = (auxValue == testValue + 10);

        return testResult;
    }

    /**
     * Tests the subtraction function
     */
    bool TestSub() {
        bool testResult = false;
        T auxValue = testValue;

        Atomic::Sub(&auxValue, 10);
        testResult = (auxValue == testValue - 10);

        return testResult;
    }

    /**
     * Executes all the tests
     */
    bool All() {
        bool ok = TestIncrement();
        ok = ok && TestDecrement();
        ok = ok && TestTestAndSet();
        return ok;
    }

};

#endif
