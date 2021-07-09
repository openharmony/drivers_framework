/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "audio_common_test.h"
#include <gtest/gtest.h>
#include "hdf_uhdf_test.h"

using namespace testing::ext;

class AudioCoreTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void AudioCoreTest::SetUpTestCase()
{
    HdfTestOpenService();
}

void AudioCoreTest::TearDownTestCase()
{
    HdfTestCloseService();
}

void AudioCoreTest::SetUp()
{
}

void AudioCoreTest::TearDown()
{
}

HWTEST_F(AudioCoreTest, AudioCoreTest_RegisterDai, TestSize.Level0)
{
    struct HdfTestMsg msg = {g_testAudioType, TESTREGISTERDAI, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(AudioCoreTest, AudioCoreTest_RegisterPlatform, TestSize.Level0)
{
    struct HdfTestMsg msg = {g_testAudioType, TESTREGISTERPLATFORM, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(AudioCoreTest, AudioCoreTest_RegisterCodec, TestSize.Level0)
{
    struct HdfTestMsg msg = {g_testAudioType, TESTREGISTERCODEC, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(AudioCoreTest, AudioCoreTest_BindDaiLink, TestSize.Level0)
{
    struct HdfTestMsg msg = {g_testAudioType, TESTBINDDAILINK, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(AudioCoreTest, AudioCoreTest_SocDeviceRegister, TestSize.Level0)
{
    struct HdfTestMsg msg = {g_testAudioType, TESTDEVICEREGISTER, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(AudioCoreTest, AudioCoreTest_SocRegisterDsp, TestSize.Level0)
{
    struct HdfTestMsg msg = {g_testAudioType, TESTREGISTERDSP, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(AudioCoreTest, AudioCoreTest_RegisterAccessory, TestSize.Level0)
{
    struct HdfTestMsg msg = {g_testAudioType, TESTREGISTERACCESSORY, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(AudioCoreTest, AudioCoreTest_UpdateRegBits, TestSize.Level0)
{
    struct HdfTestMsg msg = {g_testAudioType, TESTUPDATEREGBITS, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(AudioCoreTest, AudioCoreTest_AiaoUpdateRegBits, TestSize.Level0)
{
    struct HdfTestMsg msg = {g_testAudioType, TESTAIAOUPDATEREGBITS, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(AudioCoreTest, AudioCoreTest_KcontrolGetCodec, TestSize.Level0)
{
    struct HdfTestMsg msg = {g_testAudioType, TESTKCONTROLGETCODEC, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(AudioCoreTest, AudioCoreTest_AddControls, TestSize.Level0)
{
    struct HdfTestMsg msg = {g_testAudioType, TESTADDCONTROLS, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(AudioCoreTest, AudioCoreTest_AddControl, TestSize.Level0)
{
    struct HdfTestMsg msg = {g_testAudioType, TESTADDCONTROL, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(AudioCoreTest, AudioCoreTest_DeviceReadReg, TestSize.Level0)
{
    struct HdfTestMsg msg = {g_testAudioType, TESTDEVICEREADREG, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(AudioCoreTest, AudioCoreTest_AiaoDeviceReadReg, TestSize.Level0)
{
    struct HdfTestMsg msg = {g_testAudioType, TESTAIAODEVICEREADREG, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(AudioCoreTest, AudioCoreTest_InfoCtrlSw, TestSize.Level0)
{
    struct HdfTestMsg msg = {g_testAudioType, TESTINFOCTRLSW, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(AudioCoreTest, AudioCoreTest_GetCtrlSw, TestSize.Level0)
{
    struct HdfTestMsg msg = {g_testAudioType, TESTGETCTRLSW, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(AudioCoreTest, AudioCoreTest_PutCtrlSw, TestSize.Level0)
{
    struct HdfTestMsg msg = {g_testAudioType, TESTPUTCTRLSW, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(AudioCoreTest, AudioCoreTest_AiaoGetCtrlSw, TestSize.Level0)
{
    struct HdfTestMsg msg = {g_testAudioType, TESTAIAOGETCTRLSW, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}

HWTEST_F(AudioCoreTest, AudioCoreTest_AiaoPutCtrlSw, TestSize.Level0)
{
    struct HdfTestMsg msg = {g_testAudioType, TESTAIAOPUTCTRLSW, -1};
    EXPECT_EQ(0, HdfTestSendMsgToService(&msg));
}
