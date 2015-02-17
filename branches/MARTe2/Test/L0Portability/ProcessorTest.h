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

#ifndef PROCESSOR_TEST_H
#define PROCESSOR_TEST_H

#include "Processor.h"

class ProcessorTest {

public:
	ProcessorTest(){
    }

    /**
     * Tests the vendor identifier function
     */
    bool TestVendorId(){
    	bool testResult = false;
    	const char * vendorId = NULL;

    	vendorId = Processor::VendorId();
    	testResult = (vendorId != NULL);

        return testResult;
    }

    /**
     * Tests the family function
     */
    bool TestFamily(){
    	bool testResult = false;
    	uint32 family = -1;

    	family = Processor::Family();
    	testResult = (family != -1);

        return testResult;
    }

    /**
     * Tests the model function
     */
    bool TestModel(){
    	bool testResult = false;
    	uint32 model = -1;

    	model = Processor::Model();
		testResult = (model != -1);

        return testResult;
    }

    /**
     * Tests the available function
     */
    bool TestAvailable(){
    	bool testResult = false;
    	uint32 available = -1;

    	available = Processor::Available();
		testResult = (available != -1);

        return testResult;
    }

};

#endif
