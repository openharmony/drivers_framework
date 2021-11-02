/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "audio_core_test.h"
#include "audio_core.h"
#include "audio_parse.h"
#include "devsvc_manager_clnt.h"

#define HDF_LOG_TAG audio_core_test
#define AUDIO_CORE_SERVICE_TEST_NAME    "hdf_audio_codec_dev0"
#define PLATFORM_TEST_NAME              "codec_service_1"
#define PLATFORM_CODEC_TEST_NAME        "codec_service_0"
#define ACCESSORY_DAI_TEST_NAME         "accessory_dai"
#define DSP_TEST_NAME                   "dsp_service_0"

const struct AudioMixerControl g_audioMixerRegParams = {
    .reg = 0x2004,  /* [0] output volume */
    .rreg = 0x2004, /* register value */
    .shift = 8,     /* offset */
    .rshift = 8,    /* right offset */
    .min = 0x28,    /* min value */
    .max = 0x7F,    /* max value */
    .mask = 0x7F,   /* mask value */
    .invert = 0,    /* invert */
};

int32_t AudioSocTestRegisterPlatform(void)
{
    HDF_LOGI("%s: enter", __func__);

    if (AudioSocRegisterPlatform(NULL, NULL) == HDF_SUCCESS) {
        HDF_LOGE("%s_[%d] AudioSocRegisterPlatform fail", __func__, __LINE__);
        return HDF_FAILURE;
    }

    struct HdfDeviceObject *device = DevSvcManagerClntGetDeviceObject(AUDIO_CORE_SERVICE_TEST_NAME);
    struct PlatformData data = {
        .drvPlatformName = PLATFORM_TEST_NAME,
    };
    if (AudioSocRegisterPlatform(device, &data) != HDF_SUCCESS) {
        HDF_LOGE("%s_[%d] AudioSocRegisterPlatform fail", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGI("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t AudioSocTestRegisterDai(void)
{
    HDF_LOGI("%s: enter", __func__);

    if (AudioSocRegisterDai(NULL, NULL) == HDF_SUCCESS) {
        HDF_LOGE("%s_[%d] AudioSocRegisterDai fail", __func__, __LINE__);
        return HDF_FAILURE;
    }

    struct HdfDeviceObject *device = DevSvcManagerClntGetDeviceObject(AUDIO_CORE_SERVICE_TEST_NAME);
    struct DaiData data = {
        .drvDaiName = ACCESSORY_DAI_TEST_NAME,
    };
    if (AudioSocRegisterDai(device, &data) != HDF_SUCCESS) {
        HDF_LOGE("%s_[%d] AudioSocRegisterDai fail", __func__, __LINE__);
        return HDF_FAILURE;
    }

    HDF_LOGI("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t AudioTestRegisterAccessory(void)
{
    HDF_LOGI("%s: enter", __func__);

    if (AudioRegisterAccessory(NULL, NULL, NULL) == HDF_SUCCESS) {
        HDF_LOGE("%s_[%d] AudioRegisterAccessory fail", __func__, __LINE__);
        return HDF_FAILURE;
    }

    struct HdfDeviceObject *device = DevSvcManagerClntGetDeviceObject(AUDIO_CORE_SERVICE_TEST_NAME);
    struct AccessoryData data = {
        .drvAccessoryName = ACCESSORY_DAI_TEST_NAME,
    };
    struct DaiData daiData = {
        .drvDaiName = ACCESSORY_DAI_TEST_NAME,
    };

    if (AudioRegisterAccessory(device, &data, &daiData) != HDF_SUCCESS) {
        HDF_LOGE("%s_[%d] AudioRegisterAccessory fail", __func__, __LINE__);
        return HDF_FAILURE;
    }
    HDF_LOGI("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t AudioTestRegisterCodec(void)
{
    HDF_LOGI("%s: enter", __func__);

    if (AudioRegisterCodec(NULL, NULL, NULL) == HDF_SUCCESS) {
        HDF_LOGE("%s_[%d] AudioRegisterCodec fail", __func__, __LINE__);
        return HDF_FAILURE;
    }

    struct HdfDeviceObject *device = DevSvcManagerClntGetDeviceObject(AUDIO_CORE_SERVICE_TEST_NAME);
    struct CodecData codecData = {
        .drvCodecName = ACCESSORY_DAI_TEST_NAME,
    };
    struct DaiData daiData = {
        .drvDaiName = ACCESSORY_DAI_TEST_NAME,
    };
    if (AudioRegisterCodec(device, &codecData, &daiData) != HDF_SUCCESS) {
        HDF_LOGE("%s_[%d] AudioRegisterCodec fail", __func__, __LINE__);
        return HDF_FAILURE;
    }

    HDF_LOGI("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t AudioTestRegisterDsp(void)
{
    HDF_LOGI("%s: enter", __func__);

    if (AudioRegisterDsp(NULL, NULL, NULL) == HDF_SUCCESS) {
        HDF_LOGE("%s_[%d] AudioSocRegisterDsp fail", __func__, __LINE__);
        return HDF_FAILURE;
    }

    struct HdfDeviceObject *device = DevSvcManagerClntGetDeviceObject(AUDIO_CORE_SERVICE_TEST_NAME);
    struct DaiData daiData = {
        .drvDaiName = ACCESSORY_DAI_TEST_NAME,
    };
    struct DspData dspData = {
        .drvDspName = DSP_TEST_NAME,
    };

    if (AudioRegisterDsp(device, &dspData, &daiData) != HDF_SUCCESS) {
        HDF_LOGE("%s_[%d] AudioSocRegisterDsp fail", __func__, __LINE__);
        return HDF_FAILURE;
    }

    HDF_LOGI("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t AudioTestSocDeviceRegister(void)
{
    struct {
        char *name;
    } data;

    enum AudioDeviceType deviceType = AUDIO_DAI_DEVICE;
    HDF_LOGI("%s: enter", __func__);

    if (AudioSocDeviceRegister(NULL, NULL, deviceType) == HDF_SUCCESS) {
        HDF_LOGE("%s_[%d] AudioSocDeviceRegister fail", __func__, __LINE__);
        return HDF_FAILURE;
    }

    struct HdfDeviceObject *device = DevSvcManagerClntGetDeviceObject(AUDIO_CORE_SERVICE_TEST_NAME);
    data.name = ACCESSORY_DAI_TEST_NAME;
    if (AudioSocDeviceRegister(device, &data, deviceType) != HDF_SUCCESS) {
        HDF_LOGE("%s_[%d] AudioSocDeviceRegister fail", __func__, __LINE__);
        return HDF_FAILURE;
    }

    deviceType = AUDIO_PLATFORM_DEVICE;
    data.name = PLATFORM_TEST_NAME;
    if (AudioSocDeviceRegister(device, &data, deviceType) != HDF_SUCCESS) {
        HDF_LOGE("%s_[%d] AudioSocDeviceRegister fail", __func__, __LINE__);
        return HDF_FAILURE;
    }
    HDF_LOGI("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t AudioTestBindDaiLink(void)
{
    HDF_LOGI("%s: enter", __func__);

    if (AudioBindDaiLink(NULL, NULL) == HDF_SUCCESS) {
        HDF_LOGE("%s_[%d] AudioBindDaiLink fail", __func__, __LINE__);
        return HDF_FAILURE;
    }

    struct AudioCard *card = (struct AudioCard *)OsalMemCalloc(sizeof(struct AudioCard));
    if (card == NULL) {
        ADM_LOG_ERR("%s_[%d] Malloc audioCard fail!", __func__, __LINE__);
        return HDF_FAILURE;
    }
    struct HdfDeviceObject *device = DevSvcManagerClntGetDeviceObject(AUDIO_CORE_SERVICE_TEST_NAME);
    if (AudioFillConfigData(device, &(card->configData)) != HDF_SUCCESS) {
        ADM_LOG_ERR("%s_[%d] AudioFillConfigData fail", __func__, __LINE__);
        OsalMemFree(card);
        card = NULL;
        return HDF_FAILURE;
    }

    if (AudioBindDaiLink(card, &(card->configData)) != HDF_SUCCESS) {
        HDF_LOGE("%s_[%d] AudioBindDaiLink fail", __func__, __LINE__);
        OsalMemFree(card);
        card = NULL;
        return HDF_FAILURE;
    }
    OsalMemFree(card->rtd);
    card->rtd = NULL;
    OsalMemFree(card);
    card = NULL;
    HDF_LOGI("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t ReadCodecReg(unsigned long virtualAddress, uint32_t reg, uint32_t *val)
{
    return HDF_SUCCESS;
}

int32_t WriteCodecReg(unsigned long virtualAddress, uint32_t reg, uint32_t val)
{
    return HDF_SUCCESS;
}
int32_t AudioTestUpdateCodecRegBits(void)
{
    int32_t value = 0;
    if (AudioUpdateCodecRegBits(NULL, NULL, value) == HDF_SUCCESS) {
        HDF_LOGE("%s_[%d] AudioUpdateCodecRegBits fail", __func__, __LINE__);
        return HDF_FAILURE;
    }

    struct CodecData codecData = {
        .Read = ReadCodecReg,
        .Write = WriteCodecReg,
    };
    struct CodecDevice codec;
    codec.devCodecName = PLATFORM_CODEC_TEST_NAME;
    OsalMutexInit(&codec.mutex);
    codec.devData = &codecData;
    value = g_audioMixerRegParams.min + 1;
    if (AudioUpdateCodecRegBits(&codec, &g_audioMixerRegParams, value) != HDF_SUCCESS) {
        HDF_LOGE("%s_[%d] AudioUpdateCodecRegBits fail", __func__, __LINE__);
        OsalMutexDestroy(&codec.mutex);
        return HDF_FAILURE;
    }

    OsalMutexDestroy(&codec.mutex);
    HDF_LOGI("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t AudioTestUpdateAccessoryRegBits(void)
{
    struct AccessoryDevice *accessory = NULL;
    struct AudioMixerControl *mixerControl = NULL;
    int32_t value = 0;

    int32_t ret = AudioUpdateAccessoryRegBits(accessory, mixerControl, value);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s_[%d] AudioTestUpdateAccessoryRegBits fail", __func__, __LINE__);
    }

    HDF_LOGI("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t AudioTestKcontrolGetCodec(void)
{
    struct CodecDevice *codecDevice = NULL;
    const struct AudioKcontrol *kcontrol = NULL;
    HDF_LOGI("%s: enter", __func__);

    codecDevice = AudioKcontrolGetCodec(kcontrol);
    if (codecDevice == NULL) {
        HDF_LOGE("%s_[%d] AudioKcontrolGetCodec fail!", __func__, __LINE__);
    }

    HDF_LOGI("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t AudioTestKcontrolGetAccessory(void)
{
    struct AudioKcontrol *kcontrol = NULL;
    struct AccessoryDevice *accessory = AudioKcontrolGetAccessory(kcontrol);
    if (accessory == NULL) {
        HDF_LOGE("%s_[%d] AudioKcontrolGetAccessory fail!", __func__, __LINE__);
    }

    HDF_LOGI("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t AudioTestAddControl(void)
{
    struct AudioCard *audioCard = NULL;
    const struct AudioKcontrol *control = NULL;
    struct AudioKcontrol *audioKcontrol = NULL;
    HDF_LOGI("%s: enter", __func__);

    audioKcontrol = AudioAddControl(audioCard, control);
    if (audioKcontrol == NULL) {
        HDF_LOGE("%s_[%d] AudioAddControl fail!", __func__, __LINE__);
    }

    HDF_LOGI("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t AudioTestAddControls(void)
{
    struct AudioCard *audioCard = NULL;
    const struct AudioKcontrol *controls = NULL;
    int32_t controlMaxNum = 0x03;
    int32_t ret;
    HDF_LOGI("%s: enter", __func__);

    ret = AudioAddControls(audioCard, controls, controlMaxNum);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s_[%d] AudioAddControls fail", __func__, __LINE__);
    }

    HDF_LOGI("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t AudioTestCodecReadReg(void)
{
    struct CodecDevice *codec = NULL;
    uint32_t reg = 0;
    uint32_t val = 0;

    int32_t ret = AudioCodecReadReg(codec, reg, &val);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s_[%d] AudioCodecReadReg fail", __func__, __LINE__);
    }

    HDF_LOGI("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t AudioTestAccessoryReadReg(void)
{
    struct AccessoryDevice *accessory = NULL;
    uint32_t reg = 0;
    uint32_t val = 0;

    int32_t ret = AudioAccessoryReadReg(accessory, reg, &val);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s_[%d] AudioAccessoryReadReg fail", __func__, __LINE__);
    }

    HDF_LOGI("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t AudioTestCodecWriteReg(void)
{
    struct CodecDevice *codec = NULL;
    uint32_t reg = 0;
    uint32_t val = 0;

    int32_t ret = AudioCodecWriteReg(codec, reg, val);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s_[%d] AudioCodecWriteReg fail", __func__, __LINE__);
    }

    HDF_LOGI("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t AudioTestAccessoryWriteReg(void)
{
    struct AccessoryDevice *accessory = NULL;
    uint32_t reg = 0;
    uint32_t val = 0;

    int32_t ret = AudioAccessoryWriteReg(accessory, reg, val);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s_[%d] AudioAccessoryWriteReg fail", __func__, __LINE__);
    }

    HDF_LOGI("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t AudioTestInfoCtrlOps(void)
{
    struct AudioKcontrol *kcontrol = NULL;
    struct AudioCtrlElemInfo *elemInfo = NULL;

    int32_t ret = AudioInfoCtrlOps(kcontrol, elemInfo);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s_[%d] AudioInfoCtrlOps fail", __func__, __LINE__);
    }

    HDF_LOGI("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t AudioTestCodecGetCtrlOps(void)
{
    struct AudioKcontrol *kcontrol = NULL;
    struct AudioCtrlElemValue *elemValue = NULL;

    int32_t ret = AudioCodecGetCtrlOps(kcontrol, elemValue);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s_[%d] AudioCodecGetCtrlOps fail", __func__, __LINE__);
    }

    HDF_LOGI("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t AudioTestAccessoryGetCtrlOps(void)
{
    struct AudioKcontrol *kcontrol = NULL;
    struct AudioCtrlElemValue *elemValue = NULL;

    int32_t ret = AudioAccessoryGetCtrlOps(kcontrol, elemValue);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s_[%d] AudioAccessoryGetCtrlOps fail", __func__, __LINE__);
    }

    HDF_LOGI("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t AudioTestCodecSetCtrlOps(void)
{
    struct AudioKcontrol *kcontrol = NULL;
    struct AudioCtrlElemValue *elemValue = NULL;

    int32_t ret = AudioCodecSetCtrlOps(kcontrol, elemValue);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s_[%d] AudioCodecSetCtrlOps fail", __func__, __LINE__);
    }

    HDF_LOGI("%s: success", __func__);
    return HDF_SUCCESS;
}

int32_t AudioTestAccessorySetCtrlOps(void)
{
    struct AudioKcontrol *kcontrol = NULL;
    struct AudioCtrlElemValue *elemValue = NULL;

    int32_t ret = AudioAccessorySetCtrlOps(kcontrol, elemValue);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s_[%d] AudioAccessorySetCtrlOps fail", __func__, __LINE__);
    }

    HDF_LOGI("%s: success", __func__);
    return HDF_SUCCESS;
}
