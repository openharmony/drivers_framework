/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <gtest/gtest.h>
#include <string>
#include <unistd.h>

#include "hdf_uhdf_test.h"
#include "platform_trace_test.h"

using namespace testing::ext;

class HdfPlatformTraceTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void HdfPlatformTraceTest::SetUpTestCase()
{
    HdfTestOpenService();
}

void HdfPlatformTraceTest::TearDownTestCase()
{
    HdfTestCloseService();
}

void HdfPlatformTraceTest::SetUp() {}

void HdfPlatformTraceTest::TearDown() {}

/**
 * @tc.name: HdfPlatformTraceTestSetUintptrInfo001
 * @tc.desc: platform trace function test
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(HdfPlatformTraceTest, HdfPlatformTraceTestSetUintptrInfo001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_TRACE_TYPE, PLATFORM_TRACE_TEST_SET_UINT, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.name: HdfPlatformTraceTestFmtInfo001
 * @tc.desc: platform trace function test
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(HdfPlatformTraceTest, HdfPlatformTraceTestFmtInfo001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_TRACE_TYPE, PLATFORM_TRACE_TEST_SET_FMT, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.name: HdfPlatformTraceTestReliability001
 * @tc.desc: platform trace function test
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(HdfPlatformTraceTest, HdfPlatformTraceTestReliability001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_TRACE_TYPE, PLATFORM_TRACE_TEST_RELIABILITY, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}
