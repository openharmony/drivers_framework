/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef ACCESSORY_ADAPTER_H
#define ACCESSORY_ADAPTER_H

#include "audio_host.h"
#include "audio_control.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

struct AccessoryDevice {
    const char *devAccessoryName;
    struct AccessoryData *devData;
    struct HdfDeviceObject *device;
    struct DListHead list;
    struct OsalMutex mutex;
};

struct AudioAccessoryOps {
    const char *devAccessoryName;
    struct AccessoryData *devData;
    struct HdfDeviceObject *device;
    struct DListHead list;
};

struct AccessoryData {
    const char *drvAccessoryName;
    /* Accessory driver callbacks */
    int32_t (*AccessoryInit)(struct AudioCard *, const struct AccessoryDevice *device);
    int32_t (*Read)(const struct AccessoryDevice *, uint32_t, uint32_t *);
    int32_t (*Write)(const struct AccessoryDevice *, uint32_t, uint32_t);

    const struct AudioKcontrol *controls;
    int numControls;
};

/* Accessory host is defined in accessory driver */
struct AccessoryHost {
    struct IDeviceIoService service;
    struct HdfDeviceObject *device;
    void *priv;
};


int32_t ExternalCodecDeviceInit(struct AudioCard *audioCard, const struct AccessoryDevice *device);
int32_t ExternalCodecDeviceReadReg(const struct AccessoryDevice *codec, uint32_t reg, uint32_t *value);
int32_t ExternalCodecDeviceWriteReg(const struct AccessoryDevice *codec, uint32_t reg, uint32_t value);

int32_t ExternalCodecDaiStartup(const struct AudioCard *card, const struct DaiDevice *device);
int32_t ExternalCodecDaiHwParams(const struct AudioCard *card, const struct AudioPcmHwParams *param,
                                 const struct DaiDevice *device);
int32_t ExternalCodecDaiDeviceInit(const struct AudioCard *card, const struct DaiDevice *device);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif
