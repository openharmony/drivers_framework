/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef AUDIO_HOST_H
#define AUDIO_HOST_H

#include "hdf_base.h"
#include "hdf_dlist.h"
#include "hdf_device_desc.h"
#include "hdf_log.h"
#include "device_resource_if.h"
#include "osal_mem.h"
#include "osal_mutex.h"
#include "securec.h"
#include "audio_types.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define ADM_LOG_ERR(fmt, arg...) do { \
    HDF_LOGE("[%s][line:%d]: " fmt, __func__, __LINE__, ##arg); \
    } while (0)

#define ADM_LOG_INFO(fmt, arg...) do { \
    HDF_LOGI("[%s][line:%d]: " fmt, __func__, __LINE__, ##arg); \
    } while (0)

#define ADM_LOG_WARNING(fmt, arg...) do { \
    HDF_LOGW("[%s][line:%d]: " fmt, __func__, __LINE__, ##arg); \
    } while (0)

#define ADM_LOG_DEBUG(fmt, arg...) do { \
    HDF_LOGD("[%s][line:%d]: " fmt, __func__, __LINE__, ##arg); \
    } while (0)

#define BUFF_SIZE_MAX 64

#define AUDIO_LIST_HEAD_INIT(name) { &(name), &(name) }

#define AUDIO_LIST_HEAD(name) \
    struct DListHead name = AUDIO_LIST_HEAD_INIT(name)

struct AudioConfigData {
    const char *cardServiceName;
    const char *codecName;
    const char *platformName;
    const char *cpuDaiName;
    const char *codecDaiName;
    const char *accessoryName;
    const char *accessoryDaiName;
    const char *dspName;
    const char *dspDaiName;
};

struct AudioCard {
    struct AudioPcmRuntime *rtd;
    struct AudioConfigData configData;

    /* Card-specific routes and components. */
    const struct AudioSapmComponent *sapmComponents;
    int32_t sapmComponentsNum;
    const struct AudioSapmRoute *sapmRoutes;
    int32_t sapmRoutesNum;

    struct DListHead list;
    struct DListHead controls; /* all controls for this card */
    struct DListHead components; /* all components for this card */
    struct DListHead paths; /* all paths for this card */
    struct DListHead sapmDirty; /* all dirty for this card */
};

enum CriBuffStatus {
    ENUM_CIR_BUFF_NORMAL = 1,
    ENUM_CIR_BUFF_FULL,
    ENUM_CIR_BUFF_EMPTY,
};

enum AudioStreamType {
    AUDIO_CAPTURE_STREAM = 0,
    AUDIO_RENDER_STREAM,
};

struct AudioPcmHwParams {
    /* The stream type in a frame */
    enum AudioStreamType streamType;
    /* The number of channels in a frame */
    uint32_t channels;
    /* The number of frames per second */
    uint32_t rate;
    /* The number of frames in a period */
    uint32_t periodSize;
    /* The number of periods in a PCM */
    uint32_t periodCount;
    /* The sample format of a PCM */
    enum AudioFormat format; /* < Audio data format. For details, see {@link AudioFormat}. */
    const char *cardServiceName;
    uint32_t period;
    uint32_t frameSize;
    bool isBigEndian;
    bool isSignedData;
    uint32_t startThreshold;
    uint32_t stopThreshold;
    uint32_t silenceThreshold;
};

struct AudioHost {
    struct IDeviceIoService service;
    struct HdfDeviceObject *device;
    void *priv;
};

struct AudioTxData {
    enum CriBuffStatus status;
    char *buf; /* buf address */
    unsigned long frames; /* frames number */
};

struct AudioRxData {
    enum CriBuffStatus status;
    char *buf;
    unsigned long bufSize;
    unsigned long frames; /* frames number */
};

struct AudioPcmRuntime       {
    /* runtime devices */
    struct CodecDevice *codec;
    struct PlatformDevice *platform;
    struct DaiDevice *codecDai;
    struct DaiDevice *cpuDai;
    struct DaiDevice *accessoryDai;
    struct AccessoryDevice *accessory;
    struct DspDevice *dsp;
    struct DaiDevice *dspDai;
    uint8_t complete;
    uint32_t frameBits;
    uint8_t *dmaArea;
    unsigned long bufferSize;
};

struct AudioHost *AudioHostCreateAndBind(struct HdfDeviceObject *device);
void AudioHostDestroy(struct AudioHost *host);

static inline struct HdfDeviceObject *AudioHostToDevice(struct AudioHost *host)
{
    return (host == NULL) ? NULL : host->device;
}

static inline struct AudioHost *AudioHostFromDevice(struct HdfDeviceObject *device)
{
    return (device == NULL) ? NULL : (struct AudioHost *)device->service;
}

/* Get a sound card instance */
struct AudioCard *GetCardInstance(const char *serviceName);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* AUDIO_HOST_H */
