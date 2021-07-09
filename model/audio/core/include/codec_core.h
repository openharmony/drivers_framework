/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef CODEC_CORE_H
#define CODEC_CORE_H

#include "codec_adapter.h"
#include "audio_core.h"
#include "osal_io.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

int32_t CodecDeviceReadReg(struct CodecDevice *codec, uint32_t reg, uint32_t *value);
int32_t CodecDeviceWriteReg(struct CodecDevice *codec, uint32_t reg, uint32_t value);
int32_t AiaoDeviceReadReg(struct CodecDevice *codec, uint32_t reg, uint32_t *value);
int32_t AiaoDeviceWriteReg(struct CodecDevice *codec, uint32_t reg, uint32_t value);
int32_t CodecGetServiceName(struct HdfDeviceObject *device, const char **drvCodecName);
int32_t CodecGetDaiName(struct HdfDeviceObject *device, const char **drvDaiName);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* CODEC_CORE_H */
