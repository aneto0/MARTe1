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
    ASSERT_TRUE(streamtest.TestGetC("HelloWorld"));
}

TEST_F(StreamableGTest,TestPutC) {
    StreamableTest streamtest;
    ASSERT_TRUE(streamtest.TestPutC("HelloWorld"));
}


TEST_F(StreamableGTest,TestGetCAndPutC) {
    StreamableTest streamtest;
    ASSERT_TRUE(streamtest.TestGetCAndPutC("HelloWorld"));
}


TEST_F(StreamableGTest,TestRead) {
    StreamableTest streamtest;
    ASSERT_TRUE(streamtest.TestRead("HelloWorld"));
}

TEST_F(StreamableGTest,TestWrite) {
    StreamableTest streamtest;
    ASSERT_TRUE(streamtest.TestWrite("HelloWorld"));
}


TEST_F(StreamableGTest,TestReadAndWrite) {
    StreamableTest streamtest;
    ASSERT_TRUE(streamtest.TestReadAndWrite("ThisIsTheStringToRead","ThisIsTheStringToWrite"));
}


TEST_F(StreamableGTest,TestSeek) {
    StreamableTest streamtest;
    ASSERT_TRUE(streamtest.TestSeek("ThisIsTheStringToRead","ThisIsTheStringToWrite"));
}


TEST_F(StreamableGTest,TestSwitch) {
    StreamableTest streamtest;
    ASSERT_TRUE(streamtest.TestSwitch("ThisIsTheStringToRead","ThisIsTheStringToWrite"));
}
