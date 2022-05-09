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
#include "platform_dumper_test.h"

using namespace testing::ext;

class HdfPlatformDumperTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void HdfPlatformDumperTest::SetUpTestCase()
{
    HdfTestOpenService();
}

void HdfPlatformDumperTest::TearDownTestCase()
{
    HdfTestCloseService();
}

void HdfPlatformDumperTest::SetUp() {}

void HdfPlatformDumperTest::TearDown() {}

/**
 * @tc.name: HdfPlatformDumperTestAddUint8Data001
 * @tc.desc: platform dumper function test
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(HdfPlatformDumperTest, HdfPlatformDumperTestAddUint8Data001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_DUMPER_TYPE, PLAT_DUMPER_TEST_ADD_UINT8, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.name: HdfPlatformDumperTestAddUint16Data001
 * @tc.desc: platform dumper function test
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(HdfPlatformDumperTest, HdfPlatformDumperTestAddUint16Data001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_DUMPER_TYPE, PLAT_DUMPER_TEST_ADD_UINT16, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.name: HdfPlatformDumperTestAddUint32Data001
 * @tc.desc: platform dumper function test
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(HdfPlatformDumperTest, HdfPlatformDumperTestAddUint32Data001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_DUMPER_TYPE, PLAT_DUMPER_TEST_ADD_UINT32, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.name: HdfPlatformDumperTestAddUint64Data001
 * @tc.desc: platform dumper function test
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(HdfPlatformDumperTest, HdfPlatformDumperTestAddUint64Data001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_DUMPER_TYPE, PLAT_DUMPER_TEST_ADD_UINT64, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.name: HdfPlatformDumperTestAddInt8Data001
 * @tc.desc: platform dumper function test
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(HdfPlatformDumperTest, HdfPlatformDumperTestAddInt8Data001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_DUMPER_TYPE, PLAT_DUMPER_TEST_ADD_INT8, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.name: HdfPlatformDumperTestAddInt16Data001
 * @tc.desc: platform dumper function test
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(HdfPlatformDumperTest, HdfPlatformDumperTestAddInt16Data001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_DUMPER_TYPE, PLAT_DUMPER_TEST_ADD_INT16, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.name: HdfPlatformDumperTestAddInt32Data001
 * @tc.desc: platform dumper function test
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(HdfPlatformDumperTest, HdfPlatformDumperTestAddInt32Data001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_DUMPER_TYPE, PLAT_DUMPER_TEST_ADD_INT32, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.name: HdfPlatformDumperTestAddInt64Data001
 * @tc.desc: platform dumper function test
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(HdfPlatformDumperTest, HdfPlatformDumperTestAddInt64Data001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_DUMPER_TYPE, PLAT_DUMPER_TEST_ADD_INT64, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.name: HdfPlatformDumperTestAddFloatData001
 * @tc.desc: platform dumper function test
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(HdfPlatformDumperTest, HdfPlatformDumperTestAddFloatData001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_DUMPER_TYPE, PLAT_DUMPER_TEST_ADD_FLOAT, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.name: HdfPlatformDumperTestAddDoubleData001
 * @tc.desc: platform dumper function test
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(HdfPlatformDumperTest, HdfPlatformDumperTestAddDoubleData001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_DUMPER_TYPE, PLAT_DUMPER_TEST_ADD_DOUBLE, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.name: HdfPlatformDumperTestAddCharData001
 * @tc.desc: platform dumper function test
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(HdfPlatformDumperTest, HdfPlatformDumperTestAddCharData001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_DUMPER_TYPE, PLAT_DUMPER_TEST_ADD_CHAR, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.name: HdfPlatformDumperTestAddStringData001
 * @tc.desc: platform dumper function test
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(HdfPlatformDumperTest, HdfPlatformDumperTestAddStringData001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_DUMPER_TYPE, PLAT_DUMPER_TEST_ADD_STRING, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.name: HdfPlatformDumperTestAddRegisterData001
 * @tc.desc: platform dumper function test
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(HdfPlatformDumperTest, HdfPlatformDumperTestAddRegisterData001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_DUMPER_TYPE, PLAT_DUMPER_TEST_ADD_REGISTER, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.name: HdfPlatformDumperTestAddArrayData001
 * @tc.desc: platform dumper function test
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(HdfPlatformDumperTest, HdfPlatformDumperTestAddArrayData001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_DUMPER_TYPE, PLAT_DUMPER_TEST_ADD_ARRAY_DATA, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.name: HdfPlatformDumperTestSetOps001
 * @tc.desc: platform dumper function test
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(HdfPlatformDumperTest, HdfPlatformDumperTestSetOps001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_DUMPER_TYPE, PLAT_DUMPER_TEST_SET_OPS, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.name: HdfPlatformDumperTestMultiThread001
 * @tc.desc: platform dumper function test
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(HdfPlatformDumperTest, HdfPlatformDumperTestMultiThread001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_DUMPER_TYPE, PLAT_DUMPER_TEST_MULTI_THREAD, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.name: HdfPlatformDumperTestDelData001
 * @tc.desc: platform dumper function test
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(HdfPlatformDumperTest, HdfPlatformDumperTestDelData001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_DUMPER_TYPE, PLAT_DUMPER_TEST_DEL_DATA, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

/**
 * @tc.name: HdfPlatformDumperTestReliability001
 * @tc.desc: platform dumper reliability test
 * @tc.type: FUNC
 * @tc.require: NA
 */
HWTEST_F(HdfPlatformDumperTest, HdfPlatformDumperTestReliability001, TestSize.Level1)
{
    struct HdfTestMsg msg = {TEST_PAL_DUMPER_TYPE, PLAT_DUMPER_TEST_RELIABILITY, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}
