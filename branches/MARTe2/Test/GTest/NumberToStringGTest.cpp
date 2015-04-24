/*
 *  NumberToStringGTest.cpp
 *
 *  Created on: Mar 5, 2015
 */
#include <limits.h>
#include "gtest/gtest.h"
#include "NumberToStringTest.h"

class NumberToStringGTest: public ::testing::Test {
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



TEST_F(NumberToStringGTest,TestDecimalMagnitude) {
    NumberToStringTest numbertest;
    ASSERT_TRUE(numbertest.TestDecimalMagnitude());
}

TEST_F(NumberToStringGTest,TestHexadecimalMagnitude) {
    NumberToStringTest numbertest;
    ASSERT_TRUE(numbertest.TestHexadecimalMagnitude());
}

TEST_F(NumberToStringGTest,TestOctalMagnitude) {
    NumberToStringTest numbertest;
    ASSERT_TRUE(numbertest.TestOctalMagnitude());
}

TEST_F(NumberToStringGTest,TestBinaryMagnitude) {
    NumberToStringTest numbertest;
    ASSERT_TRUE(numbertest.TestBinaryMagnitude());
}


TEST_F(NumberToStringGTest,TestDecimalStream) {
    NumberToStringTest numbertest;
    ASSERT_TRUE(numbertest.TestDecimalStream());
}


TEST_F(NumberToStringGTest,TestHexadecimalStream) {
    NumberToStringTest numbertest;
    ASSERT_TRUE(numbertest.TestHexadecimalStream());
}


TEST_F(NumberToStringGTest,TestOctalStream) {
    NumberToStringTest numbertest;
    ASSERT_TRUE(numbertest.TestOctalStream());
}


TEST_F(NumberToStringGTest,TestBinaryStream) {
    NumberToStringTest numbertest;
    ASSERT_TRUE(numbertest.TestBinaryStream());
}



TEST_F(NumberToStringGTest,TestDecimalPrint) {
    NumberToStringTest numbertest;
    ASSERT_TRUE(numbertest.TestDecimalPrint());
}


TEST_F(NumberToStringGTest,TestHexadecimalPrint) {
    NumberToStringTest numbertest;
    ASSERT_TRUE(numbertest.TestHexadecimalPrint());
}
