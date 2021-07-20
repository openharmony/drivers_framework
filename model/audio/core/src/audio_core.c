/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "audio_core.h"

#define HDF_LOG_TAG audio_core

AUDIO_LIST_HEAD(daiController);
AUDIO_LIST_HEAD(platformController);
AUDIO_LIST_HEAD(codecController);
AUDIO_LIST_HEAD(dspController);
AUDIO_LIST_HEAD(accessoryController);

int32_t AudioSocRegisterDsp(struct HdfDeviceObject *device, struct DaiData *data)
{
    struct DaiDevice *dsp = NULL;

    if ((device == NULL) || (data == NULL)) {
        ADM_LOG_ERR("Input params check error: device=%p, data=%p.", device, data);
        return HDF_ERR_INVALID_OBJECT;
    }

    dsp = (struct DaiDevice *)OsalMemCalloc(sizeof(*dsp));
    if (dsp == NULL) {
        ADM_LOG_ERR("Malloc dsp device fail!");
        return HDF_ERR_MALLOC_FAIL;
    }

    dsp->devDaiName = data->drvDaiName;
    dsp->devData = data;
    dsp->device = device;
    DListInsertHead(&dsp->list, &daiController);
    ADM_LOG_INFO("Register [%s] success.", dsp->devDaiName);

    return HDF_SUCCESS;
}

int32_t AudioSocRegisterPlatform(struct HdfDeviceObject *device, struct PlatformData *data)
{
    struct PlatformDevice *platform = NULL;

    if ((device == NULL) || (data == NULL)) {
        ADM_LOG_ERR("Input params check error: device=%p, data=%p.", device, data);
        return HDF_ERR_INVALID_OBJECT;
    }

    platform = (struct PlatformDevice *)OsalMemCalloc(sizeof(*platform));
    if (platform == NULL) {
        ADM_LOG_ERR("Malloc platform device fail!");
        return HDF_ERR_MALLOC_FAIL;
    }

    platform->devPlatformName = data->drvPlatformName;
    platform->devData = data;
    platform->device = device;
    DListInsertHead(&platform->list, &platformController);
    ADM_LOG_INFO("Register [%s] success.", platform->devPlatformName);

    return HDF_SUCCESS;
}

int32_t AudioSocRegisterDai(struct HdfDeviceObject *device, struct DaiData *data)
{
    struct DaiDevice *dai = NULL;

    if ((device == NULL) || (data == NULL)) {
        ADM_LOG_ERR("Input params check error: device=%p, data=%p.", device, data);
        return HDF_ERR_INVALID_OBJECT;
    }

    dai = (struct DaiDevice *)OsalMemCalloc(sizeof(*dai));
    if (dai == NULL) {
        ADM_LOG_ERR("Malloc dai device fail!");
        return HDF_ERR_MALLOC_FAIL;
    }

    dai->devDaiName = data->drvDaiName;
    dai->devData = data;
    dai->device = device;
    DListInsertHead(&dai->list, &daiController);
    ADM_LOG_INFO("Register [%s] success.", dai->devDaiName);

    return HDF_SUCCESS;
}

int32_t AudioRegisterAccessory(struct HdfDeviceObject *device, struct AccessoryData *data, struct DaiData *daiData)
{
    struct AccessoryDevice *accessory = NULL;
    int32_t ret;

    if (device == NULL || data == NULL || daiData == NULL) {
        ADM_LOG_ERR("Input params check error: device=%p, data=%p, daiData=%p.", device, data, daiData);
        return HDF_ERR_INVALID_OBJECT;
    }

    accessory = (struct AccessoryDevice *)OsalMemCalloc(sizeof(*accessory));
    if (accessory == NULL) {
        ADM_LOG_ERR("Malloc accessory device fail!");
        return HDF_ERR_MALLOC_FAIL;
    }

    accessory->devAccessoryName = data->drvAccessoryName;
    accessory->devData = data;
    accessory->device = device;

    ret = AudioSocDeviceRegister(device, (void *)daiData, AUDIO_DAI_DEVICE);
    if (ret != HDF_SUCCESS) {
        OsalMemFree(accessory);
        ADM_LOG_ERR("Register accessory device fail ret=%d", ret);
        return HDF_ERR_IO;
    }
    DListInsertHead(&accessory->list, &accessoryController);
    ADM_LOG_INFO("Register [%s] success.", accessory->devAccessoryName);

    return HDF_SUCCESS;
}

int32_t AudioRegisterCodec(struct HdfDeviceObject *device, struct CodecData *codecData, struct DaiData *daiData)
{
    struct CodecDevice *codec = NULL;
    int32_t ret;

    if ((device == NULL) || (codecData == NULL) || (daiData == NULL)) {
        ADM_LOG_ERR("Input params check error: device=%p, codecData=%p, daiData=%p.",
            device, codecData, daiData);
        return HDF_ERR_INVALID_OBJECT;
    }

    codec = (struct CodecDevice *)OsalMemCalloc(sizeof(*codec));
    if (codec == NULL) {
        ADM_LOG_ERR("Malloc codec device fail!");
        return HDF_ERR_MALLOC_FAIL;
    }

    OsalMutexInit(&codec->mutex);
    codec->devCodecName = codecData->drvCodecName;
    codec->devData = codecData;
    codec->device = device;

    ret = AudioSocDeviceRegister(device, (void *)daiData, AUDIO_DAI_DEVICE);
    if (ret != HDF_SUCCESS) {
        OsalMemFree(codec);
        ADM_LOG_ERR("Register dai device fail ret=%d", ret);
        return HDF_ERR_IO;
    }
    DListInsertHead(&codec->list, &codecController);
    ADM_LOG_INFO("Register [%s] success.", codec->devCodecName);

    return HDF_SUCCESS;
}

int32_t AudioSocDeviceRegister(struct HdfDeviceObject *device, void *data, enum AudioDeviceType deviceType)
{
    struct PlatformData *platformData = NULL;
    struct DaiData *daiData = NULL;
    int32_t ret;

    if ((device == NULL) || (data == NULL) || (deviceType >= AUDIO_DEVICE_BUTT) || (deviceType < 0)) {
        ADM_LOG_ERR("Input params check error: device=%p, data=%p, deviceType=%d.",
            device, data, deviceType);
        return HDF_ERR_INVALID_OBJECT;
    }

    switch (deviceType) {
        case AUDIO_DAI_DEVICE: {
            daiData = (struct DaiData *)data;
            ret = AudioSocRegisterDai(device, daiData);
            if (ret != HDF_SUCCESS) {
                ADM_LOG_ERR("Register dai device fail ret=%d", ret);
                return HDF_FAILURE;
            }
            break;
        }
        case AUDIO_DSP_DEVICE: {
            daiData = (struct DaiData *)data;
            ret = AudioSocRegisterDsp(device, daiData);
            if (ret != HDF_SUCCESS) {
                ADM_LOG_ERR("Register dsp device fail ret=%d", ret);
                return HDF_FAILURE;
            }
            break;
        }
        case AUDIO_PLATFORM_DEVICE: {
            platformData = (struct PlatformData *)data;
            ret = AudioSocRegisterPlatform(device, platformData);
            if (ret != HDF_SUCCESS) {
                ADM_LOG_ERR("Register dma device fail ret=%d", ret);
                return HDF_FAILURE;
            }
            break;
        }
        default: {
            ADM_LOG_ERR("Invalid device type.");
            return HDF_FAILURE;
        }
    }

    return HDF_SUCCESS;
}

void AudioSeekPlatformDevice(struct AudioPcmRuntime *rtd, const struct AudioConfigData *configData)
{
    const struct AudioConfigData *data = configData;
    struct PlatformDevice *platform = NULL;

    if (rtd == NULL || data == NULL) {
        ADM_LOG_ERR("Input params check error: rtd=%p, data=%p.", rtd, data);
        return;
    }
    if (data->platformName == NULL) {
        ADM_LOG_ERR("Input devicesName check error: data->platformName is NULL.");
        return;
    }

    DLIST_FOR_EACH_ENTRY(platform, &platformController, struct PlatformDevice, list) {
        if (platform != NULL && platform->devPlatformName != NULL &&
                strcmp(platform->devPlatformName, data->platformName) == 0) {
            rtd->platform = platform;
            break;
        }
    }
    return;
}

void AudioSeekCpuDaiDevice(struct AudioPcmRuntime *rtd, const struct AudioConfigData *configData)
{
    const struct AudioConfigData *data = configData;
    struct DaiDevice *cpuDai = NULL;

    if (rtd == NULL || data == NULL) {
        ADM_LOG_ERR("Input params check error: rtd=%p, data=%p.", rtd, data);
        return;
    }
    if (data->cpuDaiName == NULL) {
        ADM_LOG_ERR("Input cpuDaiName check error: data->cpuDaiName is NULL.");
        return;
    }

    DLIST_FOR_EACH_ENTRY(cpuDai, &daiController, struct DaiDevice, list) {
        if (cpuDai != NULL && cpuDai->devDaiName != NULL && strcmp(cpuDai->devDaiName, data->cpuDaiName) == 0) {
            rtd->cpuDai = cpuDai;
            break;
        }
    }
    return;
}

void AudioSeekCodecDevice(struct AudioPcmRuntime *rtd, const struct AudioConfigData *configData)
{
    const struct AudioConfigData *data = configData;
    struct CodecDevice *codec = NULL;
    struct DaiDevice *codecDai = NULL;

    if ((rtd == NULL) || (data == NULL)) {
        ADM_LOG_ERR("Input params check error: rtd=%p, data=%p.", rtd, data);
        return;
    }
    if (data->codecName == NULL || data->codecDaiName == NULL) {
        ADM_LOG_ERR("Input devicesName check error: data->codecName=%p, data->codecDaiName=%p.",
                    data->codecName, data->codecDaiName);
        return;
    }

    DLIST_FOR_EACH_ENTRY(codec, &codecController, struct CodecDevice, list) {
        if (codec != NULL && codec->devCodecName != NULL && strcmp(codec->devCodecName, data->codecName) == 0) {
            rtd->codec = codec;

            DLIST_FOR_EACH_ENTRY(codecDai, &daiController, struct DaiDevice, list) {
                if (codecDai != NULL && codecDai->device != NULL && codec->device == codecDai->device &&
                        strcmp(codecDai->devDaiName, data->codecDaiName) == 0) {
                    rtd->codecDai = codecDai;
                    break;
                }
            }
            break;
        }
    }
    return;
}

void AudioSeekAccessoryDevice(struct AudioPcmRuntime *rtd, const struct AudioConfigData *configData)
{
    const struct AudioConfigData *data = configData;
    struct AccessoryDevice *accessory = NULL;
    struct DaiDevice *accessoryDai = NULL;

    if (rtd == NULL || data == NULL) {
        ADM_LOG_ERR("Input params check error: rtd=%p, data=%p.", rtd, data);
        return;
    }
    if (data->accessoryName == NULL) {
        ADM_LOG_ERR("Input devicesName check error: data->accessoryName is NULL.");
        return;
    }

    DLIST_FOR_EACH_ENTRY(accessory, &accessoryController, struct AccessoryDevice, list) {
        if (accessory != NULL && accessory->devAccessoryName != NULL &&
                strcmp(accessory->devAccessoryName, data->accessoryName) == 0) {
            rtd->accessory = accessory;

            DLIST_FOR_EACH_ENTRY(accessoryDai, &daiController, struct DaiDevice, list) {
                if (accessoryDai != NULL && accessoryDai->device != NULL && accessory->device == accessoryDai->device &&
                        strcmp(accessoryDai->devDaiName, data->accessoryDaiName) == 0) {
                    rtd->accessoryDai = accessoryDai;
                    break;
                }
            }
            break;
        }
    }
    return;
}

void AudioSeekDspDevice(struct AudioPcmRuntime *rtd, const struct AudioConfigData *configData)
{
    const struct AudioConfigData *data = configData;
    struct DspDevice *dsp = NULL;
    struct DaiDevice *dspDai = NULL;

    if ((rtd == NULL) || (data == NULL)) {
        ADM_LOG_ERR("Input params check error: rtd=%p, data=%p.", rtd, data);
        return;
    }
    if (data->dspName == NULL || data->dspDaiName == NULL) {
        ADM_LOG_ERR("Input devicesName check error: data->dspName=%p, data->dspDaiName=%p.",
                    data->codecName, data->codecDaiName);
        return;
    }

    DLIST_FOR_EACH_ENTRY(dsp, &dspController, struct DspDevice, list) {
        if (dsp != NULL && dsp->devDspName != NULL && strcmp(dsp->devDspName, data->dspName) == 0) {
            rtd->dsp = dsp;
            DLIST_FOR_EACH_ENTRY(dspDai, &daiController, struct DaiDevice, list) {
                if (dspDai != NULL && dspDai->device != NULL && dsp->device == dspDai->device &&
                        strcmp(dspDai->devDaiName, data->dspDaiName) == 0) {
                    rtd->dspDai = dspDai;
                    break;
                }
            }
            break;
        }
    }
    return;
}

int32_t AudioBindDaiLink(struct AudioCard *audioCard, struct AudioConfigData *configData)
{
    int32_t ret;

    if ((audioCard == NULL) || (configData == NULL)) {
        ADM_LOG_ERR("Input params check error: audioCard=%p, configData=%p.", audioCard, configData);
        return HDF_ERR_INVALID_OBJECT;
    }

    audioCard->rtd = (struct AudioPcmRuntime *)OsalMemCalloc(sizeof(struct AudioPcmRuntime));
    if (audioCard->rtd == NULL) {
        ADM_LOG_ERR("Malloc audioCard->rtd fail!");
        return HDF_ERR_MALLOC_FAIL;
    }

    audioCard->rtd->complete = AUDIO_DAI_LINK_UNCOMPLETE;

    AudioSeekPlatformDevice(audioCard->rtd, configData);
    AudioSeekCpuDaiDevice(audioCard->rtd, configData);
    AudioSeekCodecDevice(audioCard->rtd, configData);
    AudioSeekAccessoryDevice(audioCard->rtd, configData);
    AudioSeekDspDevice(audioCard->rtd, configData);
    if (!audioCard->rtd->codec && !audioCard->rtd->accessory) {
        OsalMemFree(audioCard->rtd);
        ret = HDF_FAILURE;
        ADM_LOG_DEBUG("CODEC [%s] not registered!", configData->codecName);
    } else if (!audioCard->rtd->codecDai && !audioCard->rtd->accessoryDai) {
        OsalMemFree(audioCard->rtd);
        ret = HDF_FAILURE;
        ADM_LOG_DEBUG("CODEC DAI [%s] not registered!", configData->codecDaiName);
    } else if (!audioCard->rtd->platform) {
        OsalMemFree(audioCard->rtd);
        ret = HDF_FAILURE;
        ADM_LOG_DEBUG("Platform [%s] not registered!", configData->platformName);
    } else if (!audioCard->rtd->cpuDai) {
        OsalMemFree(audioCard->rtd);
        ret = HDF_FAILURE;
        ADM_LOG_DEBUG("CPU DAI [%s] not registered!", configData->cpuDaiName);
    } else if (!audioCard->rtd->dsp) {
        OsalMemFree(audioCard->rtd);
        ret = HDF_FAILURE;
        ADM_LOG_ERR("DSP [%s] not registered!", configData->dspName);
    } else if (!audioCard->rtd->dspDai) {
        OsalMemFree(audioCard->rtd);
        ret = HDF_FAILURE;
        ADM_LOG_ERR("DSP DAI [%s] not registered!", configData->dspDaiName);
    } else {
        audioCard->rtd->complete = AUDIO_DAI_LINK_COMPLETE;
        ret = HDF_SUCCESS;
        ADM_LOG_DEBUG("All devices register complete!");
    }

    return ret;
}

int32_t AudioUpdateCodecRegBits(struct CodecDevice *codec, struct AudioMixerControl *mixerControl, int32_t value)
{
    int32_t ret;
    uint32_t curValue = 0;
    int32_t mixerControlMask;
    if (codec == NULL || codec->devData == NULL || codec->devData->Write == NULL || mixerControl == NULL) {
        ADM_LOG_ERR("Invalid accessory param.");
        return HDF_ERR_INVALID_OBJECT;
    }

    value = value << mixerControl->shift;
    mixerControlMask = mixerControl->mask << mixerControl->shift;

    OsalMutexLock(&codec->mutex);
    ret = AudioCodecDeviceReadReg(codec, mixerControl->reg, &curValue);
    if (ret != HDF_SUCCESS) {
        OsalMutexUnlock(&codec->mutex);
        ADM_LOG_ERR("Read reg fail ret=%d.", ret);
        return HDF_FAILURE;
    }
    curValue = (curValue & ~mixerControlMask) | (value & mixerControlMask);
    ret = codec->devData->Write(codec, mixerControl->reg, curValue);
    if (ret != HDF_SUCCESS) {
        OsalMutexUnlock(&codec->mutex);
        ADM_LOG_ERR("Write reg fail ret=%d", ret);
        return HDF_FAILURE;
    }
    OsalMutexUnlock(&codec->mutex);

    ADM_LOG_DEBUG("Success.");
    return HDF_SUCCESS;
}

int32_t AudioUpdateAccessoryRegBits(struct AccessoryDevice *accessory,
    struct AudioMixerControl *mixerControl, int32_t value)
{
    int32_t ret;
    uint32_t curValue = 0;
    int32_t mixerControlMask;
    if (accessory == NULL || accessory->devData == NULL ||
        accessory->devData->Write == NULL || mixerControl == NULL) {
        ADM_LOG_ERR("Invalid accessory param.");
        return HDF_ERR_INVALID_OBJECT;
    }

    value = value << mixerControl->shift;
    mixerControlMask = mixerControl->mask << mixerControl->shift;

    OsalMutexLock(&accessory->mutex);
    ret = AudioAccessoryDeviceReadReg(accessory, mixerControl->reg, &curValue);
    if (ret != HDF_SUCCESS) {
        OsalMutexUnlock(&accessory->mutex);
        ADM_LOG_ERR("Read reg fail ret=%d", ret);
        return HDF_FAILURE;
    }
    curValue = (curValue & ~mixerControlMask) | (value & mixerControlMask);
    ret = accessory->devData->Write(accessory, mixerControl->reg, curValue);
    if (ret != HDF_SUCCESS) {
        OsalMutexUnlock(&accessory->mutex);
        ADM_LOG_ERR("Write reg fail ret=%d", ret);
        return HDF_FAILURE;
    }
    OsalMutexUnlock(&accessory->mutex);

    ADM_LOG_DEBUG("Success.");
    return HDF_SUCCESS;
}

int32_t AudioUpdateRegBits(enum AudioDeviceType deviceType, void *device,
    struct AudioMixerControl *mixerControl, int32_t value)
{
    int32_t ret;
    struct CodecDevice *codec = NULL;
    struct AccessoryDevice *accessory = NULL;
    ADM_LOG_DEBUG("Entry.");

    switch (deviceType) {
        case AUDIO_CODEC_DEVICE: {
            codec = (struct CodecDevice *)device;
            ret = AudioUpdateCodecRegBits(codec, mixerControl, value);
            break;
        }
        case AUDIO_ACCESSORY_DEVICE: {
            accessory = (struct AccessoryDevice *)device;
            ret = AudioUpdateAccessoryRegBits(accessory, mixerControl, value);
            break;
        }
        default: {
            ADM_LOG_ERR("Invalid device type.");
            return HDF_FAILURE;
        }
    }

    return ret;
}

int32_t AudioAiaoUpdateRegBits(struct CodecDevice *codec, uint32_t reg, uint32_t mask, uint32_t shift, int32_t value)
{
    int32_t ret;
    uint32_t curValue = 0;
    ADM_LOG_DEBUG("Entry to update AIAO reg bits.");

    if (codec == NULL || codec->devData == NULL || codec->devData->AiaoWrite == NULL) {
        ADM_LOG_ERR("Invalid AIAO codec param.");
        return HDF_ERR_INVALID_OBJECT;
    }

    value = value << shift;
    mask = mask << shift;

    OsalMutexLock(&codec->mutex);
    ret = AudioAiaoDeviceReadReg(codec, reg, &curValue);
    if (ret != HDF_SUCCESS) {
        OsalMutexUnlock(&codec->mutex);
        ADM_LOG_ERR("Read AIAO reg fail ret=%d.", ret);
        return HDF_FAILURE;
    }
    curValue = (curValue & ~mask) | (value & mask);
    ret = codec->devData->AiaoWrite(codec, reg, curValue);
    if (ret != HDF_SUCCESS) {
        OsalMutexUnlock(&codec->mutex);
        ADM_LOG_ERR("Write AIAO reg fail ret=%d", ret);
        return HDF_FAILURE;
    }
    OsalMutexUnlock(&codec->mutex);

    ADM_LOG_DEBUG("Update AIAO reg bits successful.");
    return HDF_SUCCESS;
}

struct CodecDevice *AudioKcontrolGetCodec(const struct AudioKcontrol *kcontrol)
{
    struct AudioCard *audioCard = NULL;
    if (kcontrol == NULL || kcontrol->pri == NULL) {
        ADM_LOG_ERR("Input param kcontrol is NULL.");
        return NULL;
    }

    audioCard = (struct AudioCard *)(kcontrol->pri);
    if (audioCard == NULL || audioCard->rtd == NULL) {
        ADM_LOG_ERR("Get codec or rtd fail.");
        return NULL;
    }

    return audioCard->rtd->codec;
}

struct AccessoryDevice *AudioKcontrolGetAccessory(const struct AudioKcontrol *kcontrol)
{
    struct AudioCard *audioCard = NULL;
    if (kcontrol == NULL || kcontrol->pri == NULL) {
        ADM_LOG_ERR("Input param kcontrol is NULL.");
        return NULL;
    }

    audioCard = (struct AudioCard *)(kcontrol->pri);
    if (audioCard == NULL || audioCard->rtd == NULL) {
        ADM_LOG_ERR("Get accessory or rtd fail.");
        return NULL;
    }

    return audioCard->rtd->accessory;
}

struct AudioKcontrol *AudioAddControl(const struct AudioCard *audioCard, const struct AudioKcontrol *ctrl)
{
    struct AudioKcontrol *control = NULL;

    if ((audioCard == NULL) || (ctrl == NULL)) {
        ADM_LOG_ERR("Input params check error: audioCard=%p, ctrl=%p.", audioCard, ctrl);
        return NULL;
    }

    control = (struct AudioKcontrol *)OsalMemCalloc(sizeof(*control));
    if (control == NULL) {
        ADM_LOG_ERR("Malloc control fail!");
        return NULL;
    }
    control->name = ctrl->name;
    control->iface = ctrl->iface;
    control->Info = ctrl->Info;
    control->Get = ctrl->Get;
    control->Put = ctrl->Put;
    control->pri = (void *)audioCard;
    control->privateValue = ctrl->privateValue;

    return control;
}

int32_t AudioAddControls(struct AudioCard *audioCard, const struct AudioKcontrol *controls, int32_t controlMaxNum)
{
    struct AudioKcontrol *control = NULL;
    int32_t i;
    ADM_LOG_DEBUG("Entry.");

    if ((audioCard == NULL) || (controls == NULL) || (controlMaxNum <= 0)) {
        ADM_LOG_ERR("Input params check error: audioCard=%p, controls=%p, controlMaxNum=%d.",
            audioCard, controls, controlMaxNum);
        return HDF_FAILURE;
    }

    for (i = 0; i < controlMaxNum; i++) {
        control = AudioAddControl(audioCard, &controls[i]);
        if (control == NULL) {
            ADM_LOG_ERR("Add control fail!");
            return HDF_FAILURE;
        }
        DListInsertHead(&control->list, &audioCard->controls);
    }
    ADM_LOG_DEBUG("Success.");
    return HDF_SUCCESS;
}

int32_t AudioCodecDeviceReadReg(struct CodecDevice *codec, uint32_t reg, uint32_t *val)
{
    int32_t ret;
    if (codec == NULL || codec->devData == NULL || codec->devData->Read == NULL || val == NULL) {
        ADM_LOG_ERR("Input param codec is NULL.");
        return HDF_FAILURE;
    }
    ret = codec->devData->Read(codec, reg, val);
    if (ret != HDF_SUCCESS) {
        ADM_LOG_ERR("Device read fail.");
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AudioAccessoryDeviceReadReg(struct AccessoryDevice *accessory, uint32_t reg, uint32_t *val)
{
    int32_t ret;
    if (accessory == NULL || accessory->devData == NULL || accessory->devData->Read == NULL || val == NULL) {
        ADM_LOG_ERR("Input param accessory is NULL.");
        return HDF_FAILURE;
    }
    ret = accessory->devData->Read(accessory, reg, val);
    if (ret != HDF_SUCCESS) {
        ADM_LOG_ERR("Device read fail.");
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AudioAiaoDeviceReadReg(struct CodecDevice *codec, uint32_t reg, uint32_t *val)
{
    int32_t ret;
    if (codec == NULL || codec->devData == NULL || codec->devData->AiaoRead == NULL || val == NULL) {
        ADM_LOG_ERR("Input param codec is NULL.");
        return HDF_FAILURE;
    }
    ret = codec->devData->AiaoRead(codec, reg, val);
    if (ret != HDF_SUCCESS) {
        ADM_LOG_ERR("Device read fail.");
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AudioInfoCtrlSw(struct AudioKcontrol *kcontrol, struct AudioCtrlElemInfo *elemInfo)
{
    struct AudioMixerControl *mixerCtrl = NULL;

    if (kcontrol == NULL || kcontrol->privateValue <= 0 || elemInfo == NULL) {
        ADM_LOG_ERR("Input param kcontrol is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }

    elemInfo->count = CHANNEL_MIN_NUM;
    mixerCtrl = (struct AudioMixerControl *)kcontrol->privateValue;
    /* stereo */
    if (mixerCtrl->reg != mixerCtrl->rreg || mixerCtrl->shift != mixerCtrl->rshift) {
        elemInfo->count = CHANNEL_MAX_NUM;
    }
    elemInfo->type = 1; /* volume type */
    elemInfo->min = mixerCtrl->min;
    elemInfo->max = mixerCtrl->max;

    return HDF_SUCCESS;
}

static int32_t AudioGetCtrlSwSubRReg(struct AudioKcontrol *kcontrol, struct AudioCtrlElemValue *elemValue,
    enum AudioDeviceType deviceType, void *device)
{
    int32_t ret = HDF_FAILURE;
    uint32_t rcurValue;
    struct AudioMixerControl *mixerCtrl = NULL;
    mixerCtrl = (struct AudioMixerControl *)kcontrol->privateValue;
    struct CodecDevice *codec = (struct CodecDevice *)device;
    struct AccessoryDevice *accessory = (struct AccessoryDevice *)device;

    if (mixerCtrl->reg != mixerCtrl->rreg || mixerCtrl->shift != mixerCtrl->rshift) {
        if (codec != NULL && codec->devData != NULL && codec->devData->Read != NULL) {
            ret = codec->devData->Read(codec, mixerCtrl->rreg, &rcurValue);
        } else if (accessory != NULL && accessory->devData != NULL && accessory->devData->Read != NULL) {
            ret = accessory->devData->Read(accessory, mixerCtrl->rreg, &rcurValue);
        }
        if (ret != HDF_SUCCESS) {
            ADM_LOG_ERR("Device read fail.");
            return HDF_FAILURE;
        }

        if (mixerCtrl->reg == mixerCtrl->rreg) {
            rcurValue = (rcurValue >> mixerCtrl->rshift) & mixerCtrl->mask;
        } else {
            rcurValue = (rcurValue >> mixerCtrl->shift) & mixerCtrl->mask;
        }
        if (rcurValue > mixerCtrl->max || rcurValue < mixerCtrl->min) {
            ADM_LOG_ERR("Audio invalid rcurValue=%d", rcurValue);
            return HDF_FAILURE;
        }
        if (mixerCtrl->invert) {
            rcurValue = mixerCtrl->max - rcurValue;
        }
        elemValue->value[1] = rcurValue;
    }

    return HDF_SUCCESS;
}

static int32_t AudioGetCtrlSwSubReg(struct AudioKcontrol *kcontrol, struct AudioCtrlElemValue *elemValue,
    enum AudioDeviceType deviceType, void *device)
{
    int32_t ret = HDF_FAILURE;
    uint32_t curValue;
    struct AudioMixerControl *mixerCtrl = NULL;
    mixerCtrl = (struct AudioMixerControl *)kcontrol->privateValue;
    struct CodecDevice *codec = (struct CodecDevice *)device;
    struct AccessoryDevice *accessory = (struct AccessoryDevice *)device;

    if (codec != NULL && codec->devData != NULL && codec->devData->Read != NULL) {
        ret = codec->devData->Read(codec, mixerCtrl->reg, &curValue);
    } else if (accessory != NULL && accessory->devData != NULL && accessory->devData->Read != NULL) {
        ret = accessory->devData->Read(accessory, mixerCtrl->reg, &curValue);
    }
    if (ret != HDF_SUCCESS) {
        ADM_LOG_ERR("Device read fail.");
        return HDF_FAILURE;
    }
    curValue = (curValue >> mixerCtrl->shift) & mixerCtrl->mask;
    if (curValue > mixerCtrl->max || curValue < mixerCtrl->min) {
        ADM_LOG_ERR("Audio invalid curValue=%d", curValue);
        return HDF_FAILURE;
    }
    if (mixerCtrl->invert) {
        curValue = mixerCtrl->max - curValue;
    }
    elemValue->value[0] = curValue;
    return HDF_SUCCESS;
}

int32_t AudioGetCtrlSw(struct AudioKcontrol *kcontrol, struct AudioCtrlElemValue *elemValue)
{
    enum AudioDeviceType deviceType;
    struct CodecDevice *codec = NULL;
    struct AccessoryDevice *accessory = NULL;
    ADM_LOG_DEBUG("Entry to get audio ctrl switch.");

    if (kcontrol == NULL || kcontrol->privateValue <= 0 || elemValue == NULL) {
        ADM_LOG_ERR("Audio input param kcontrol is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }

    codec = AudioKcontrolGetCodec(kcontrol);
    accessory = AudioKcontrolGetAccessory(kcontrol);
    if (codec != NULL && codec->devData != NULL && codec->devData->Read != NULL) {
        deviceType = AUDIO_CODEC_DEVICE;
        if (AudioGetCtrlSwSubReg(kcontrol, elemValue, deviceType, codec) ||
            AudioGetCtrlSwSubRReg(kcontrol, elemValue, deviceType, codec)) {
            ADM_LOG_ERR("Audio Codec Get Ctrl Reg fail.");
            return HDF_FAILURE;
        }
    } else {
        deviceType = AUDIO_ACCESSORY_DEVICE;
        if (AudioGetCtrlSwSubReg(kcontrol, elemValue, deviceType, accessory) ||
            AudioGetCtrlSwSubRReg(kcontrol, elemValue, deviceType, accessory)) {
            ADM_LOG_ERR("Audio Accessory Get Ctrl Reg fail.");
            return HDF_FAILURE;
        }
    }

    ADM_LOG_DEBUG("Get audio ctrl switch successful.");
    return HDF_SUCCESS;
}

static int32_t AiaoGetRightCtrlSw(struct CodecDevice *codec, struct AudioMixerControl *mixerCtrl,
    struct AudioCtrlElemValue *elemValue)
{
    int ret;
    uint32_t rcurValue;
    if (mixerCtrl->reg != mixerCtrl->rreg || mixerCtrl->shift != mixerCtrl->rshift) {
        ret = codec->devData->AiaoRead(codec, mixerCtrl->rreg, &rcurValue);
        if (ret != HDF_SUCCESS) {
            ADM_LOG_ERR("AIAO read rreg fail rcurValue=%d", rcurValue);
            return HDF_FAILURE;
        }
        if (mixerCtrl->reg == mixerCtrl->rreg) {
            rcurValue = (rcurValue >> mixerCtrl->rshift) & mixerCtrl->mask;
        } else {
            rcurValue = (rcurValue >> mixerCtrl->shift) & mixerCtrl->mask;
        }
        if (rcurValue > mixerCtrl->max) {
            ADM_LOG_ERR("AIAO Invalid rcurValue=%d", rcurValue);
            return HDF_FAILURE;
        }
        if (mixerCtrl->invert) {
            rcurValue = mixerCtrl->max - rcurValue;
        }
        elemValue->value[1] = rcurValue;
    }

    return HDF_SUCCESS;
}

int32_t AiaoGetCtrlSw(struct AudioKcontrol *kcontrol, struct AudioCtrlElemValue *elemValue)
{
    int32_t ret;
    struct CodecDevice *codec = NULL;
    struct AudioMixerControl *mixerCtrl  = NULL;
    uint32_t curValue;

    ADM_LOG_DEBUG("Entry to get AIAO ctrl switch");

    if (kcontrol == NULL || kcontrol->privateValue <= 0 || elemValue == NULL) {
        ADM_LOG_ERR("AIAO input param is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }

    codec = AudioKcontrolGetCodec(kcontrol);
    if (codec == NULL || codec->devData == NULL || codec->devData->AiaoRead == NULL) {
        ADM_LOG_ERR("AIAO codec device is NULL.");
        return HDF_FAILURE;
    }
    mixerCtrl = (struct AudioMixerControl *)kcontrol->privateValue;
    ret = codec->devData->AiaoRead(codec, mixerCtrl->reg, &curValue);
    if (ret != HDF_SUCCESS) {
        ADM_LOG_ERR("AIAO read rreg fail rcurValue=%d", curValue);
        return HDF_FAILURE;
    }
    curValue = (curValue >> mixerCtrl->shift) & mixerCtrl->mask;
    if (curValue > mixerCtrl->max) {
        ADM_LOG_ERR("AIAO invalid curValue=%d", curValue);
        return HDF_FAILURE;
    }

    if (mixerCtrl->invert) {
        curValue = mixerCtrl->max - curValue;
    }
    elemValue->value[0] = curValue;

    ret = AiaoGetRightCtrlSw(codec, mixerCtrl, elemValue);
    if (ret != HDF_SUCCESS) {
        ADM_LOG_ERR("AIAO get right ctrl is fail");
        return HDF_FAILURE;
    }

    ADM_LOG_DEBUG("Get AIAO ctrl switch successful.");
    return HDF_SUCCESS;
}

static int32_t AudioPutCtrlSwSub(struct AudioKcontrol *kcontrol, struct AudioCtrlElemValue *elemValue,
    enum AudioDeviceType deviceType, void *device)
{
    int32_t value;
    int32_t rvalue;
    int32_t rshift;
    int32_t ret;
    struct AudioMixerControl *mixerCtrl = NULL;
    mixerCtrl = (struct AudioMixerControl *)kcontrol->privateValue;

    value = elemValue->value[0];
    if (value < mixerCtrl->min || value > mixerCtrl->max) {
        ADM_LOG_ERR("Audio invalid value=%d", value);
        return HDF_ERR_INVALID_OBJECT;
    }

    if (mixerCtrl->invert) {
        value = mixerCtrl->max - value;
    }

    ret = AudioUpdateRegBits(deviceType, device, mixerCtrl, value);
    if (ret != HDF_SUCCESS) {
        ADM_LOG_ERR("Audio update reg bits fail ret=%d", ret);
        return HDF_FAILURE;
    }

    if (mixerCtrl->reg != mixerCtrl->rreg || mixerCtrl->shift != mixerCtrl->rshift) {
        rvalue = elemValue->value[1];
        if (rvalue < mixerCtrl->min || rvalue > mixerCtrl->max) {
            ADM_LOG_ERR("Audio invalid rvalue=%d", rvalue);
            return HDF_FAILURE;
        }
        if (mixerCtrl->invert) {
            rvalue = mixerCtrl->max - rvalue;
        }
        if (mixerCtrl->reg == mixerCtrl->rreg) {
            rshift = mixerCtrl->rshift;
        } else {
            rshift = mixerCtrl->shift;
        }
        mixerCtrl->shift = rshift;
        ret = AudioUpdateRegBits(deviceType, device, mixerCtrl, rvalue);
        if (ret != HDF_SUCCESS) {
            ADM_LOG_ERR("Audio stereo update reg bits fail ret=%d", ret);
            return HDF_FAILURE;
        }
    }

    return HDF_SUCCESS;
}

int32_t AudioPutCtrlSw(struct AudioKcontrol *kcontrol, struct AudioCtrlElemValue *elemValue)
{
    void *device = NULL;
    enum AudioDeviceType deviceType;
    int32_t ret = HDF_FAILURE;
    ADM_LOG_DEBUG("Entry to put audio ctrl switch.");

    if (kcontrol == NULL || (kcontrol->privateValue <= 0) || elemValue == NULL) {
        ADM_LOG_ERR("Audio input param is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }

    struct CodecDevice *codec = AudioKcontrolGetCodec(kcontrol);
    struct AccessoryDevice *accessory = AudioKcontrolGetAccessory(kcontrol);
    if (codec != NULL && codec->devData != NULL && codec->devData->Write != NULL) {
        deviceType = AUDIO_CODEC_DEVICE;
        device = (void *)codec;
        ret = AudioPutCtrlSwSub(kcontrol, elemValue, deviceType, device);
    } else if (accessory != NULL && accessory->devData != NULL && accessory->devData->Write != NULL) {
        deviceType = AUDIO_ACCESSORY_DEVICE;
        device = (void *)accessory;
        ret = AudioPutCtrlSwSub(kcontrol, elemValue, deviceType, device);
    }

    if (ret != HDF_SUCCESS) {
        ADM_LOG_ERR("Audio update fail ret=%d", ret);
        return HDF_FAILURE;
    }

    ADM_LOG_DEBUG("Put audio ctrl switch successful.");
    return HDF_SUCCESS;
}

int32_t AiaoPutCtrlSw(struct AudioKcontrol *kcontrol, struct AudioCtrlElemValue *elemValue)
{
    struct CodecDevice *codec = NULL;
    struct AudioMixerControl *mixerCtrl = NULL;
    int32_t value;
    int32_t rvalue;
    uint32_t rshift;
    int32_t ret;
    ADM_LOG_DEBUG("Entry to put AIAO ctrl switch.");

    if (kcontrol == NULL || (kcontrol->privateValue <= 0) || elemValue == NULL) {
        ADM_LOG_ERR("AIAO input param is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }

    codec = AudioKcontrolGetCodec(kcontrol);
    mixerCtrl = (struct AudioMixerControl *)kcontrol->privateValue;
    value = elemValue->value[0];
    if (value < mixerCtrl->min || value > mixerCtrl->max) {
        ADM_LOG_ERR("AIAO invalid value=%d", value);
        return HDF_ERR_INVALID_OBJECT;
    }

    if (mixerCtrl->invert) {
        value = mixerCtrl->max - value;
    }

    ret = AudioAiaoUpdateRegBits(codec, mixerCtrl->reg, mixerCtrl->mask, mixerCtrl->shift, value);
    if (ret != HDF_SUCCESS) {
        ADM_LOG_ERR("AIAO update reg bits fail ret=%d", ret);
        return HDF_FAILURE;
    }

    if (mixerCtrl->reg != mixerCtrl->rreg || mixerCtrl->shift != mixerCtrl->rshift) {
        rvalue = elemValue->value[1];
        if (rvalue < mixerCtrl->min || rvalue > mixerCtrl->max) {
            ADM_LOG_ERR("AIAO invalid rvalue=%d", rvalue);
            return HDF_ERR_INVALID_OBJECT;
        }
        if (mixerCtrl->invert) {
            rvalue = mixerCtrl->max - rvalue;
        }
        if (mixerCtrl->reg == mixerCtrl->rreg) {
            rshift = mixerCtrl->rshift;
        } else {
            rshift = mixerCtrl->shift;
        }
        ret = AudioAiaoUpdateRegBits(codec, mixerCtrl->rreg, mixerCtrl->mask, rshift, rvalue);
        if (ret != HDF_SUCCESS) {
            ADM_LOG_ERR("AIAO stereo update reg bits fail ret=%d", ret);
            return HDF_FAILURE;
        }
    }

    ADM_LOG_DEBUG("Put AIAO ctrl switch successful.");
    return HDF_SUCCESS;
}

int32_t AudioRegisterDeviceDsp(struct HdfDeviceObject *device, struct DspData *dspData, struct DaiData *DaiData)
{
    struct DspDevice *dspDev = NULL;
    int32_t ret;

    if ((device == NULL) || (dspData == NULL) || (DaiData == NULL)) {
        ADM_LOG_ERR("Input params check error: device=%p, dspData=%p, daiData=%p.",
            device, dspData, DaiData);
        return HDF_ERR_INVALID_OBJECT;
    }

    dspDev = (struct DspDevice *)OsalMemCalloc(sizeof(*dspDev));
    if (dspDev == NULL) {
        ADM_LOG_ERR("Malloc codec device fail!");
        return HDF_ERR_MALLOC_FAIL;
    }

    dspDev->devDspName = dspData->drvDspName;
    dspDev->devData = dspData;
    dspDev->device = device;

    ret = AudioSocDeviceRegister(device, (void *)DaiData, AUDIO_DSP_DEVICE);
    if (ret != HDF_SUCCESS) {
        OsalMemFree(dspDev);
        ADM_LOG_ERR("Register dai device fail ret=%d", ret);
        return HDF_ERR_IO;
    }
    DListInsertHead(&dspDev->list, &dspController);
    ADM_LOG_INFO("Register [%s] success.", dspDev->devDspName);

    return HDF_SUCCESS;
}
