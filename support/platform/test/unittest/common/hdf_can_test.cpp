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

#include "can_test.h"
#include "hdf_uhdf_test.h"

using namespace testing::ext;

class HdfCanTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void HdfCanTest::SetUpTestCase()
{
    HdfTestOpenService();
}

void HdfCanTest::TearDownTestCase()
{
    HdfTestCloseService();
}

void HdfCanTest::SetUp() {}

void HdfCanTest::TearDown() {}

/**
 * @tc.name: HdfCanTestr001
 * @tc.desc: can bus function test
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(HdfCanTest, HdfCanTest001_SendAndRead, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_CAN_TYPE, CAN_TEST_SEND_AND_READ, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.name: HdfCanTest002
 * @tc.desc: can bus function test
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(HdfCanTest, HdfCanTest002_ReadNoBlock, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_CAN_TYPE, CAN_TEST_NO_BLOCK_READ, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.name: HdfCanTest003
 * @tc.desc: can bus function test
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(HdfCanTest, HdfCanTest003_ReadBlock, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_CAN_TYPE, CAN_TEST_BLOCK_READ, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.name: HdfCanTest004
 * @tc.desc: can bus function test
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(HdfCanTest, HdfCanTest004_AddAndDelFilter, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_CAN_TYPE, CAN_TEST_ADD_DEL_FILTER, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.name: HdfCanTest005
 * @tc.desc: can bus function test
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(HdfCanTest, HdfCanTest005_AddMultiFilters, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_CAN_TYPE, CAN_TEST_ADD_MULTI_FILTER, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.name: HdfCanTest006
 * @tc.desc: can bus function test
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(HdfCanTest, HdfCanTest006_GetBusState, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_CAN_TYPE, CAN_TEST_GET_BUS_STATE, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.name: HdfCanTest007
 * @tc.desc: can bus function test
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(HdfCanTest, HdfCanTest007_ReadSameHandleInMultiThread, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_CAN_TYPE, CAN_TEST_MULTI_THREAD_READ_SAME_HANDLE, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.name: HdfCanTest008
 * @tc.desc: can bus function test
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(HdfCanTest, HdfCanTest008_ReadMultiHandleInMultiThread, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_CAN_TYPE, CAN_TEST_MULTI_THREAD_READ_MULTI_HANDLE, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.name: HdfCanTest009
 * @tc.desc: can bus function test
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(HdfCanTest, HdfCanTest009_WriteSameHandleInMultiThread, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_CAN_TYPE, CAN_TEST_MULTI_THREAD_SEND_SAME_HANDLE, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.name: HdfCanTest010
 * @tc.desc: can bus function test
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(HdfCanTest, HdfCanTest010_WriteMultiHandleInMultiThread, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_CAN_TYPE, CAN_TEST_MULTI_THREAD_SEND_MULTI_HANDLE, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}
