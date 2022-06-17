/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef AUDIO_CODEC_BASE_H
#define AUDIO_CODEC_BASE_H

#include "audio_codec_if.h"
#include "audio_core.h"
#include "osal_io.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

enum I2sFrequency {
    I2S_SAMPLE_FREQUENCY_8000  = 8000,    /* 8kHz sample_rate */
    I2S_SAMPLE_FREQUENCY_11025 = 11025,   /* 11.025kHz sample_rate */
    I2S_SAMPLE_FREQUENCY_12000 = 12000,   /* 12kHz sample_rate */
    I2S_SAMPLE_FREQUENCY_16000 = 16000,   /* 16kHz sample_rate */
    I2S_SAMPLE_FREQUENCY_22050 = 22050,   /* 22.050kHz sample_rate */
    I2S_SAMPLE_FREQUENCY_24000 = 24000,   /* 24kHz sample_rate */
    I2S_SAMPLE_FREQUENCY_32000 = 32000,   /* 32kHz sample_rate */
    I2S_SAMPLE_FREQUENCY_44100 = 44100,   /* 44.1kHz sample_rate */
    I2S_SAMPLE_FREQUENCY_48000 = 48000,   /* 48kHz sample_rate */
    I2S_SAMPLE_FREQUENCY_64000 = 64000,   /* 64kHz sample_rate */
    I2S_SAMPLE_FREQUENCY_88200 = 88200,   /* 88.2kHz sample_rate */
    I2S_SAMPLE_FREQUENCY_96000 = 96000    /* 96kHz sample_rate */
};

enum I2sFrequencyRegVal {
    I2S_SAMPLE_FREQUENCY_REG_VAL_8000  = 0x0,   /* 8kHz sample_rate */
    I2S_SAMPLE_FREQUENCY_REG_VAL_11025 = 0x1,   /* 11.025kHz sample_rate */
    I2S_SAMPLE_FREQUENCY_REG_VAL_12000 = 0x2,   /* 12kHz sample_rate */
    I2S_SAMPLE_FREQUENCY_REG_VAL_16000 = 0x3,   /* 16kHz sample_rate */
    I2S_SAMPLE_FREQUENCY_REG_VAL_22050 = 0x4,   /* 22.050kHz sample_rate */
    I2S_SAMPLE_FREQUENCY_REG_VAL_24000 = 0x5,   /* 24kHz sample_rate */
    I2S_SAMPLE_FREQUENCY_REG_VAL_32000 = 0x6,   /* 32kHz sample_rate */
    I2S_SAMPLE_FREQUENCY_REG_VAL_44100 = 0x7,   /* 44.1kHz sample_rate */
    I2S_SAMPLE_FREQUENCY_REG_VAL_48000 = 0x8,   /* 48kHz sample_rate */
    I2S_SAMPLE_FREQUENCY_REG_VAL_64000 = 0x9,   /* 64kHz sample_rate */
    I2S_SAMPLE_FREQUENCY_REG_VAL_88200 = 0xA,   /* 88.2kHz sample_rate */
    I2S_SAMPLE_FREQUENCY_REG_VAL_96000 = 0xB    /* 96kHz sample_rate */
};

struct I2cTransferParam {
    uint16_t i2cDevAddr;
    uint16_t i2cBusNumber;
    uint16_t i2cRegDataLen; // default 16 bit
};

struct DaiParamsVal {
    uint32_t frequencyVal;
    uint32_t formatVal;
    uint32_t channelVal;
};

int32_t CodecGetServiceName(const struct HdfDeviceObject *device, const char **drvCodecName);
int32_t CodecGetDaiName(const struct HdfDeviceObject *device, const char **drvDaiName);
int32_t CodecGetConfigInfo(const struct HdfDeviceObject *device, struct CodecData *codecData);
int32_t CodecSetConfigInfo(struct CodecData *codeData, struct DaiData *daiData);
int32_t CodecSetCtlFunc(struct CodecData *codeData, const void *aiaoGetCtrl, const void *aiaoSetCtrl);
int32_t CodecDeviceReadReg(const struct CodecDevice *codec, uint32_t reg, uint32_t *value);
int32_t CodecDeviceWriteReg(const struct CodecDevice *codec, uint32_t reg, uint32_t value);
int32_t CodecDaiRegI2cRead(const struct DaiDevice *dai, uint32_t reg, uint32_t *value);
int32_t CodecDaiRegI2cWrite(const struct DaiDevice *dai, uint32_t reg, uint32_t value);
int32_t CodecDeviceRegI2cRead(const struct CodecDevice *codec, uint32_t reg, uint32_t *value);
int32_t CodecDeviceRegI2cWrite(const struct CodecDevice *codec, uint32_t reg, uint32_t value);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif
