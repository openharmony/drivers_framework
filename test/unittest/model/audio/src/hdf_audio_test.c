/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "hdf_log.h"
#include "hdf_audio_test.h"
#include "audio_host_test.h"
#include "audio_core_test.h"
#include "audio_parse_test.h"
#include "audio_sapm_test.h"
#include "audio_stream_dispatch_test.h"

#define HDF_LOG_TAG hdf_audio_test

// add test case entry
static HdfTestCaseList g_hdfAudioTestCaseList[] = {
    {AUDIO_DRIVER_TESTGETCODEC, AudioKcontrolTestGetCodec},
    {AUDIO_DRIVER_TESTGETCARDINSTANCE, GetCardTestInstance},
    {AUDIO_DRIVER_TESTGETCCNFIGDATA, AudioFillTestConfigData},
    
    {AUDIO_DRIVER_TESTREGISTERPLATFORM, AudioSocTestRegisterPlatform},
    {AUDIO_DRIVER_TESTREGISTERDAI, AudioSocTestRegisterDai},
    {AUDIO_DRIVER_TESTREGISTERACCESSORY, AudioTestRegisterAccessory},
    {AUDIO_DRIVER_TESTREGISTERCODEC, AudioTestRegisterCodec},
    {AUDIO_DRIVER_TESTREGISTERDSP, AudioTestRegisterDsp},
    {AUDIO_DRIVER_TESTSOCDEVICEREGISTER, AudioTestSocDeviceRegister},
    {AUDIO_DRIVER_TESTBINDDAILINK, AudioTestBindDaiLink},
    {AUDIO_DRIVER_TESTUPDATECODECREGBITS, AudioTestUpdateCodecRegBits},
    {AUDIO_DRIVER_TESTUPDATEACCESSORYREGBITS, AudioTestUpdateAccessoryRegBits},
    {AUDIO_DRIVER_TESTKCONTROLGETCODEC, AudioTestKcontrolGetCodec},
    {AUDIO_DRIVER_TESTKCONTROLGETACCESSORY, AudioTestKcontrolGetAccessory},
    {AUDIO_DRIVER_TESTADDCONTROL, AudioTestAddControl},
    {AUDIO_DRIVER_TESTADDCONTROLS, AudioTestAddControls},
    {AUDIO_DRIVER_TESTCODECREADREG, AudioTestCodecReadReg},
    {AUDIO_DRIVER_TESTACCESSORYREADREG, AudioTestAccessoryReadReg},
    {AUDIO_DRIVER_TESTCODECWRITEREG, AudioTestCodecWriteReg},
    {AUDIO_DRIVER_TESTACCESSORYWRITEREG, AudioTestAccessoryWriteReg},
    {AUDIO_DRIVER_TESTINFOCTRLOPS, AudioTestInfoCtrlOps},
    {AUDIO_DRIVER_TESTCODECGETCTRLOPS, AudioTestCodecGetCtrlOps},
    {AUDIO_DRIVER_TESTACCESSORYGETCTRLOPS, AudioTestAccessoryGetCtrlOps},
    {AUDIO_DRIVER_TESTCODECSETCTRLOPS, AudioTestCodecSetCtrlOps},
    {AUDIO_DRIVER_TESTACCESSORYSETCTRLOPS, AudioTestAccessorySetCtrlOps},

    {AUDIO_DRIVER_TESTNEWCOMPONENT, AudioSapmTestNewComponents},
    {AUDIO_DRIVER_TESTADDROUTES, AudioSapmTestAddRoutes},
    {AUDIO_DRIVER_TESTNEWCONTROLS, AudioSapmTestNewControls},
    {AUDIO_DRIVER_TESTPOWERCOMPONET, AudioSapmTestPowerComponents},
    {AUDIO_DRIVER_TESTREFRESHTIME, AudioSapmTestRefreshTime},
    {AUDIO_DRIVER_TESTSTREAMDISPATCH, AudioControlDispatchTestStreamDispatch},
};

int32_t HdfAudioEntry(HdfTestMsg *msg)
{
    int32_t result, i;

    if (msg == NULL) {
        HDF_LOGE("%s is fail: HdfTestMsg is NULL!", __func__);
        return HDF_SUCCESS;
    }

    for (i = 0; i < sizeof(g_hdfAudioTestCaseList) / sizeof(g_hdfAudioTestCaseList[0]); ++i) {
        if ((msg->subCmd == g_hdfAudioTestCaseList[i].subCmd) && (g_hdfAudioTestCaseList[i].testFunc != NULL)) {
            result = g_hdfAudioTestCaseList[i].testFunc();
            HDF_LOGE("HdfTest:Audio test result[%s-%u]", ((result == 0) ? "pass" : "fail"), msg->subCmd);
            msg->result = (result == 0) ? HDF_SUCCESS : HDF_FAILURE;
            return HDF_SUCCESS;
        }
    }
    return HDF_SUCCESS;
}
