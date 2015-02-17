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

#ifndef SLEEP_TEST_H
#define SLEEP_TEST_H

#include "Sleep.h"
#include <time.h>

class SleepTest {

public:
    SleepTest(){
    }



    /**
     * Tests the SleepAtLeast function
     */
    bool TestSleepAtleast(double sec){
    	bool testResult = false;
    	double maxSleepTime = 2*1000*sec; /* 100% margin */
    	int initialTime = clock();

    	SleepAtLeast(sec);

    	testResult = (((clock()-initialTime)/(CLOCKS_PER_SEC/1000)) <= maxSleepTime);

        return testResult;
    }

    /**
     * Tests the SleepNoMore function
     */
    bool TestSleepNoMore(double sec){
    	bool testResult = false;
    	double maxSleepTime = 2*1000*sec; /* 100% margin */
    	int initialTime = clock();

    	SleepNoMore(sec);

    	testResult = (((clock()-initialTime)/(CLOCKS_PER_SEC/1000)) <= maxSleepTime);

        return testResult;
    }

    /**
     * Tests the SleepSec function
     * Sleep time indicated by a double
     */
    bool TestSleepSec(double sec){
    	bool testResult = false;
    	double maxSleepTime = 2*1000*sec; /* 100% margin */
    	int initialTime = clock();

    	SleepSec(sec);

    	testResult = (((clock()-initialTime)/(CLOCKS_PER_SEC/1000)) <= maxSleepTime);

        return testResult;
    }

    /**
	 * Tests the SleepSec function
	 * Sleep time indicated by a float
	 */
	bool TestSleepSec(float sec){
		bool testResult = false;
		float maxSleepTime = 2*1000*sec; /* 100% margin */
		int initialTime = clock();

		SleepSec(sec);

		testResult = (((clock()-initialTime)/(CLOCKS_PER_SEC/1000)) <= maxSleepTime);

		return testResult;
	}

	/**
	 * Tests the SleepMSec function
	 */
	bool TestSleepMSec(int32 msec){
		bool testResult = false;
		double maxSleepTime = 2*msec; /* 100% margin */
		int initialTime = clock();

		SleepMSec(msec);

		testResult = (((clock()-initialTime)/(CLOCKS_PER_SEC/1000)) <= maxSleepTime);

		return testResult;
	}

	/**
	 * Tests the SleepBusy function
	 */
	bool TestSleepBusy(double sec){
		bool testResult = false;
		double maxSleepTime = 2*1000*sec; /* 100% margin */
		int initialTime = clock();

		SleepBusy(sec);

		testResult = (((clock()-initialTime)/(CLOCKS_PER_SEC/1000)) <= maxSleepTime);

		return testResult;
	}

	/**
	 * Tests the SleepMSec function
	 */
	bool TestSemiBusy(double totalSleepSec, double nonBusySleepSec){
		bool testResult = false;
		double maxSleepTime = 2*1000*totalSleepSec; /* 100% margin */
		int initialTime = clock();

		SleepSemiBusy(totalSleepSec, nonBusySleepSec);

		testResult = (((clock()-initialTime)/(CLOCKS_PER_SEC/1000)) <= maxSleepTime);

		return testResult;
	}




    /**
     * Executes all the tests
     */
    bool All(){
        bool ok = TestSleepAtleast(1.2);
        ok      = ok && TestSleepNoMore(0.8);
        ok      = ok && TestSleepSec(0.3);
        ok      = ok && TestSleepSec(0.2);
        ok      = ok && TestSleepMSec(120);
        ok      = ok && TestSleepBusy(0.4);
        ok      = ok && TestSemiBusy(0.5,0.2);
        return ok;
    }
};

#endif

