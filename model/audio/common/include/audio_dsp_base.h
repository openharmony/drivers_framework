/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef AUDIO_DSP_BASE_H
#define AUDIO_DSP_BASE_H

#include "audio_host.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

int32_t DspGetServiceName(const struct HdfDeviceObject *device, const char **drvDspName);
int32_t DspGetDaiName(const struct HdfDeviceObject *device, const char **drvDaiName);
int32_t DspLinkDeviceInit(struct AudioCard *card, const struct DaiDevice *device);
int32_t DspDeviceInit(const struct DspDevice *device);
int32_t DspDeviceReadReg(const struct DspDevice *device, uint8_t *buf, uint32_t len);
int32_t DspDeviceWriteReg(const struct DspDevice *device, uint8_t *buf, uint32_t len);
int32_t DspLinkStartup(const struct AudioCard *card, const struct DaiDevice *device);
int32_t DspLinkHwParams(const struct AudioCard *card, const struct AudioPcmHwParams *param);
int32_t DspDecodeAudioStream(const struct AudioCard *card, const uint8_t *buf, const struct DspDevice *device);
int32_t DspEncodeAudioStream(const struct AudioCard *card, const uint8_t *buf, const struct DspDevice *device);
int32_t DspEqualizerActive(const struct AudioCard *card, const uint8_t *buf, const struct DspDevice *device);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* AUDIO_DSP_BASE_H */
