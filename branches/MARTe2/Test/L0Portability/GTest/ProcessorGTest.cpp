/*
 *  FastMathGTest.cpp
 *
 *  Created on: Feb 13, 2015
 */
#include <limits.h>
#include "gtest/gtest.h"
#include "../ProcessorTest.h"


class ProcessorGTest : public ::testing::Test {
protected:
	virtual void SetUp() {
		// Code here will be called immediately after the constructor 
		// (right before each test).

	}

	virtual void TearDown() {
		// Code here will be called immediately after each test
		// (right before the destructor).
	}
};


TEST_F(ProcessorGTest,VendorId){
	ProcessorTest processorTest;
	ASSERT_TRUE(processorTest.TestVendorId());
}

TEST_F(ProcessorGTest,Family){
	ProcessorTest processorTest;
	ASSERT_TRUE(processorTest.TestFamily());
}

TEST_F(ProcessorGTest,Model){
	ProcessorTest processorTest;
	ASSERT_TRUE(processorTest.TestModel());
}

TEST_F(ProcessorGTest,Available){
	ProcessorTest processorTest;
	ASSERT_TRUE(processorTest.TestAvailable());
}
