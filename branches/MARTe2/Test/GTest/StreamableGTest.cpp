/*
 *  StreamableGTest.cpp
 *
 *  Created on: Mar 5, 2015
 */
#include <limits.h>
#include "gtest/gtest.h"
#include "StreamableTest.h"

class StreamableGTest: public ::testing::Test {
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

TEST_F(StreamableGTest,TestGetC) {
    StreamableTest streamtest;
    ASSERT_TRUE(streamtest.TestGetC());
}

TEST_F(StreamableGTest,TestPutC) {
    StreamableTest streamtest;
    ASSERT_TRUE(streamtest.TestPutC());
}
