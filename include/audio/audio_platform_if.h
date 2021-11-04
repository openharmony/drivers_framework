/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef AUDIO_PLATFORM_IF_H
#define AUDIO_PLATFORM_IF_H

#include "audio_host.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define I2S_IOCFG2_BASE1 0x0020
#define I2S_IOCFG2_BASE2 0x0024
#define I2S_IOCFG2_BASE3 0x0028
#define I2S_IOCFG2_BASE4 0x002C
#define I2S_IOCFG2_BASE5 0x0030

#define I2S_IOCFG2_BASE1_VAL 0x663
#define I2S_IOCFG2_BASE2_VAL 0x673
#define I2S_IOCFG2_BASE3_VAL 0x573
#define I2S_IOCFG2_BASE4_VAL 0x473
#define I2S_IOCFG2_BASE5_VAL 0x433

#define I2S_IOCFG3_BASE1 0x44
#define I2S_IOCFG3_BASE1_VAL 0x0600

#define GPIO_BASE1 0x2010
#define GPIO_BASE2 0x2400
#define GPIO_BASE3 0x2010

#define GPIO_BASE2_VAL 0x000000ff
#define GPIO_BASE3_VAL 0x00000000

#define IOCFG2_BASE_ADDR 0x112F0000
#define IOCFG3_BASE_ADDR 0x10FF0000
#define GPIO_BASE_ADDR 0x120D0000
#define BASE_ADDR_REMAP_SIZE 0x10000

enum PcmStatus {
    PCM_STOP = 0,
    PCM_PAUSE,
    PCM_START,
};

struct CircleBufInfo {
    uint32_t cirBufSize;
    uint32_t trafBufSize;
    uint32_t period;
    uint32_t periodSize;
    uint32_t periodCount;
    unsigned long phyAddr;
    uint32_t *virtAddr;
    uint32_t wbufOffSet;
    uint32_t wptrOffSet;
    uint32_t rbufOffSet;
    uint32_t rptrOffSet;
    enum PcmStatus runStatus;
    uint32_t chnId;
    uint32_t enable;
    struct OsalMutex buffMutex;
    uint32_t framesPosition;
    uint32_t pointer;
    uint32_t periodsMax;
    uint32_t periodsMin;
    uint32_t cirBufMax;
    uint32_t curTrafSize;
};

struct PlatformData {
    const char *drvPlatformName;
    /* platform driver callbacks */
    int32_t (*PlatformInit)(const struct AudioCard *, const struct PlatformDevice *);
    /* platform stream ops */
    struct AudioDmaOps *ops;
    struct CircleBufInfo renderBufInfo;
    struct CircleBufInfo captureBufInfo;
    struct PcmInfo pcmInfo;
    bool platformInitFlag;
    struct AudioMmapData mmapData;
    uint32_t mmapLoopCount;
    void *dmaPrv;
};

/* dma related definitions */
struct AudioDmaOps {
    int32_t (*DmaBufAlloc)(struct PlatformData *, enum AudioStreamType);
    int32_t (*DmaBufFree)(struct PlatformData *,  enum AudioStreamType);
    int32_t (*DmaRequestChannel)(struct PlatformData *);
    int32_t (*DmaConfigChannel)(struct PlatformData *);
    int32_t (*DmaPrep)(struct PlatformData *);
    int32_t (*DmaSubmit)(struct PlatformData *);
    int32_t (*DmaPending)(struct PlatformData *);
    int32_t (*DmaPause)(struct PlatformData *);
    int32_t (*DmaResume)(struct PlatformData *);
    int32_t (*DmaPointer)(struct PlatformData *, uint32_t *);
};

struct PlatformDevice {
    const char *devPlatformName;
    struct PlatformData *devData;
    struct HdfDeviceObject *device;
    struct DListHead list;
};

/* Platform host is defined in platform driver */
struct PlatformHost {
    struct IDeviceIoService service;
    struct HdfDeviceObject *device;
    void *priv;
};

static inline struct PlatformHost *PlatformHostFromDevice(struct HdfDeviceObject *device)
{
    return (device == NULL) ? NULL : (struct PlatformHost *)device->service;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif
