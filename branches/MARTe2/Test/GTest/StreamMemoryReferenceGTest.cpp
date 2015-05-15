/*
 *  StreamaStringGTest.cpp
 *
 *  Created on: May 14, 2015
 */
#include <limits.h>
#include "gtest/gtest.h"
#include "StreamMemoryReferenceTest.h"

class StreamMemoryReferenceGTest: public ::testing::Test {
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

TEST_F(StreamMemoryReferenceGTest,TestGetC) {
    StreamMemoryReferenceTest stringtest;
    ASSERT_TRUE(stringtest.TestGetC());
}

TEST_F(StreamMemoryReferenceGTest,TestPutC) {
    StreamMemoryReferenceTest stringtest;
    ASSERT_TRUE(stringtest.TestPutC());
}


TEST_F(StreamMemoryReferenceGTest,TestRead) {
    StreamMemoryReferenceTest stringtest;
    ASSERT_TRUE(stringtest.TestRead());
}
/*
TEST_F(StreamMemoryReferenceGTest,TestWrite) {
    StreamMemoryReferenceTest stringtest;
    ASSERT_TRUE(stringtest.TestWrite("HelloWorld"));
}


TEST_F(StreamMemoryReferenceGTest,TestSeek) {
    StreamMemoryReferenceTest stringtest;
    ASSERT_TRUE(stringtest.TestSeek("ThisIsTheStringToRead"));
}


TEST_F(StreamMemoryReferenceGTest,TestOperators) {
    StreamMemoryReferenceTest stringtest;
    ASSERT_TRUE(stringtest.TestOperators("ThisIsTheStringToRead","ThisIsTheStringToWrite"));
}


TEST_F(StreamMemoryReferenceGTest,TestUseless) {
    StreamMemoryReferenceTest stringtest;
    ASSERT_TRUE(stringtest.TestUseless());
}


TEST_F(StreamMemoryReferenceGTest,TestPrint) {
    StreamMemoryReferenceTest stringtest;
    ASSERT_TRUE(stringtest.TestPrint());
}


TEST_F(StreamMemoryReferenceGTest,TestToken) {
    StreamMemoryReferenceTest stringtest;
    ASSERT_TRUE(stringtest.TestToken());
}*/
