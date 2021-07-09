/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "gpio_if.h"
#include <linux/dma-mapping.h>
#include "audio_core.h"
#include <linux/slab.h>
#include "hi3516_aiao.h"
#include "hi3516_codec.h"
#include "osal_io.h"
#include "osal_mem.h"

#define HDF_LOG_TAG platform
const int AIAO_BUFF_OFFSET = 128;
const int AIAO_BUFF_POINT = 320;
const int AIAO_BUFF_TRANS = 16 * 1024;
const int AIAO_BUFF_SIZE = 128 * 1024;
const int RENDER_TRAF_BUF_SIZE = 1024;
const int CAPTURE_TRAF_BUF_SIZE = 1024 * 16;

const int AUDIO_BUFF_MIN = 128;
const int AUDIO_RECORD_MIN = 1024 * 16;
const int BITSTOBYTE = 8;

static int g_captureBuffFreeCount = 0;
static int g_captureBuffInitCount = 0;
static int g_renderBuffFreeCount = 0;
static int g_renderBuffInitCount = 0;

const int MIN_PERIOD_SIZE = 4096;
const int MAX_PERIOD_SIZE = 1024 * 16;
const int MIN_PERIOD_COUNT = 8;
const int MAX_PERIOD_COUNT = 32;
const int MAX_AIAO_BUFF_SIZE = 128 * 1024;
const int MIN_AIAO_BUFF_SIZE = 16 * 1024;

/* HdfDriverEntry implementations */
static int32_t PlatformDriverBind(struct HdfDeviceObject *device)
{
    struct PlatformHost *platformHost = NULL;
    AUDIO_DRIVER_LOG_DEBUG("entry!");

    if (device == NULL) {
        AUDIO_DRIVER_LOG_ERR("input para is NULL.");
        return HDF_FAILURE;
    }

    platformHost = (struct PlatformHost *)OsalMemCalloc(sizeof(*platformHost));
    if (platformHost == NULL) {
        AUDIO_DRIVER_LOG_ERR("malloc host fail!");
        return HDF_FAILURE;
    }

    platformHost->device = device;
    platformHost->platformInitFlag = false;
    device->service = &platformHost->service;

    AUDIO_DRIVER_LOG_DEBUG("success!");
    return HDF_SUCCESS;
}

int32_t AudioRenderBuffInit(struct PlatformHost *platformHost)
{
    unsigned int buffSize;
    if (platformHost == NULL) {
        AUDIO_DRIVER_LOG_ERR("input para is NULL.");
        return HDF_FAILURE;
    }

    if (platformHost->renderBufInfo.virtAddr != NULL) {
        return HDF_SUCCESS;
    }

    buffSize = platformHost->renderBufInfo.periodCount * platformHost->renderBufInfo.periodSize;
    if (buffSize < MIN_AIAO_BUFF_SIZE || buffSize > MAX_AIAO_BUFF_SIZE) {
        AUDIO_DRIVER_LOG_ERR("buffSize is invalid.");
        return HDF_FAILURE;
    }
    AUDIO_DRIVER_LOG_INFO("buffSize = %d ", buffSize);

    platformHost->renderBufInfo.phyAddr = 0;
    platformHost->renderBufInfo.virtAddr = dma_alloc_writecombine(NULL, buffSize,
        (dma_addr_t *)&platformHost->renderBufInfo.phyAddr, GFP_DMA | GFP_KERNEL);

    if (platformHost->renderBufInfo.virtAddr == NULL) {
        AUDIO_DRIVER_LOG_ERR("mem alloc failed.");
        return HDF_FAILURE;
    }
    platformHost->renderBufInfo.cirBufSize = buffSize;

    AUDIO_DRIVER_LOG_DEBUG("phyAddr = %x virtAddr = %x",
        platformHost->renderBufInfo.phyAddr, platformHost->renderBufInfo.virtAddr);
    AUDIO_DRIVER_LOG_DEBUG("g_renderBuffInitCount: %d", g_renderBuffInitCount++);

    return HDF_SUCCESS;
}

int32_t AudioRenderBuffFree(struct PlatformHost *platformHost)
{
    if (platformHost == NULL) {
        AUDIO_DRIVER_LOG_ERR("input para is NULL.");
        return HDF_FAILURE;
    }

    if (platformHost->renderBufInfo.virtAddr != NULL) {
        dma_free_writecombine(NULL, platformHost->renderBufInfo.cirBufSize, platformHost->renderBufInfo.virtAddr,
                              platformHost->renderBufInfo.phyAddr);
    }

    AUDIO_DRIVER_LOG_DEBUG("g_renderBuffFreeCount: %d", g_renderBuffFreeCount++);

    platformHost->renderBufInfo.virtAddr = NULL;
    platformHost->renderBufInfo.phyAddr = 0;
    return HDF_SUCCESS;
}

int32_t AudioCaptureBuffInit(struct PlatformHost *platformHost)
{
    unsigned int buffSize;
    if (platformHost == NULL) {
        AUDIO_DRIVER_LOG_ERR("input para is NULL.");
        return HDF_FAILURE;
    }
    if (platformHost->captureBufInfo.virtAddr != NULL) {
        return HDF_SUCCESS;
    }
    buffSize = platformHost->captureBufInfo.periodCount * platformHost->captureBufInfo.periodSize;
    if (buffSize < MIN_AIAO_BUFF_SIZE || buffSize > MAX_AIAO_BUFF_SIZE) {
        AUDIO_DRIVER_LOG_ERR("buffSize is invalid.");
        return HDF_FAILURE;
    }
    AUDIO_DRIVER_LOG_INFO("buffSize = %d ", buffSize);

    platformHost->captureBufInfo.phyAddr = 0;
    platformHost->captureBufInfo.virtAddr = dma_alloc_writecombine(NULL, buffSize,
        (dma_addr_t *)&platformHost->captureBufInfo.phyAddr, GFP_DMA | GFP_KERNEL);

    if (platformHost->captureBufInfo.virtAddr == NULL) {
        AUDIO_DRIVER_LOG_ERR("mem alloc failed.");
        return HDF_FAILURE;
    }
    platformHost->captureBufInfo.cirBufSize = buffSize;

    AUDIO_DRIVER_LOG_DEBUG("phyAddr = %x virtAddr = %x",
        platformHost->captureBufInfo.phyAddr, platformHost->captureBufInfo.virtAddr);
    AUDIO_DRIVER_LOG_DEBUG("g_captureBuffInitCount: %d", g_captureBuffInitCount++);

    return HDF_SUCCESS;
}

int32_t AudioCaptureBuffFree(struct PlatformHost *platformHost)
{
    if (platformHost == NULL) {
        AUDIO_DRIVER_LOG_ERR("input para is NULL.");
        return HDF_FAILURE;
    }

    if (platformHost->captureBufInfo.virtAddr != NULL) {
        dma_free_writecombine(NULL, platformHost->captureBufInfo.cirBufSize, platformHost->captureBufInfo.virtAddr,
                              platformHost->captureBufInfo.phyAddr);
    }
    AUDIO_DRIVER_LOG_DEBUG("g_captureBuffFreeCount: %d", g_captureBuffFreeCount++);

    platformHost->captureBufInfo.virtAddr = NULL;
    platformHost->captureBufInfo.phyAddr = 0;
    return HDF_SUCCESS;
}

int32_t AudioAoInit(const struct PlatformHost *platformHost)
{
    int ret;

    if (platformHost == NULL) {
        AUDIO_DRIVER_LOG_ERR("input para is NULL.");
        return HDF_FAILURE;
    }

    if (platformHost->renderBufInfo.phyAddr == 0) {
        AUDIO_DRIVER_LOG_ERR("phyAddr is error");
        return HDF_FAILURE;
    }
    ret = AopHalSetBufferAddr(0, platformHost->renderBufInfo.phyAddr);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("failed.");
        return HDF_FAILURE;
    }

    ret = AopHalSetBufferSize(0, platformHost->renderBufInfo.cirBufSize);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("AopHalSetBufferSize: failed.");
        return HDF_FAILURE;
    }

    AUDIO_DRIVER_LOG_DEBUG("AopHalSetBufferSize: %d", platformHost->renderBufInfo.cirBufSize);

    ret = AopHalSetTransSize(0, platformHost->renderBufInfo.trafBufSize);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("aop_hal_set_trans_size:  faile.");
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

int32_t AudioAiInit(const struct PlatformHost *platformHost)
{
    int ret;
    if (platformHost == NULL) {
        AUDIO_DRIVER_LOG_ERR("input para is NULL.");
        return HDF_FAILURE;
    }

    if (platformHost->captureBufInfo.phyAddr == 0) {
        AUDIO_DRIVER_LOG_ERR("phyAddr is error");
        return HDF_FAILURE;
    }

    ret = AipHalSetBufferAddr(0, platformHost->captureBufInfo.phyAddr);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("AipHalSetBufferAddr: failed.");
        return HDF_FAILURE;
    }

    ret = AipHalSetBufferSize(0, platformHost->captureBufInfo.cirBufSize);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("AipHalSetBufferSize: failed.");
        return HDF_FAILURE;
    }

    ret = AipHalSetTransSize(0, AIAO_BUFF_POINT);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("AipHalSetTransSize:  faile.");
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

static void SysWritelI2s(unsigned long addr, unsigned int value)
{
    *(volatile unsigned int *)(addr) = value;
}

int32_t AiaoSysPinMux(void)
{
    static void *regIocfg2Base = 0;
    static void *regIocfg3Base = 0;
    static void *regGpioBase = 0;

    regIocfg2Base = OsalIoRemap(IOCFG2_BASE_ADDR, BASE_ADDR_REMAP_SIZE);
    if (regIocfg2Base == NULL) {
        AUDIO_DRIVER_LOG_ERR("regIocfg2Base is NULL.");
        return HDF_FAILURE;
    }

    regIocfg3Base = OsalIoRemap(IOCFG3_BASE_ADDR, BASE_ADDR_REMAP_SIZE);
    if (regIocfg3Base == NULL) {
        AUDIO_DRIVER_LOG_ERR("g_regIocfg3Base is NULL.");
        return HDF_FAILURE;
    }

    regGpioBase = OsalIoRemap(GPIO_BASE_ADDR, BASE_ADDR_REMAP_SIZE);
    if (regGpioBase == NULL) {
        AUDIO_DRIVER_LOG_ERR("g_regGpioBase is NULL.");
        return HDF_FAILURE;
    }

    AUDIO_DRIVER_LOG_DEBUG("I2s0PinMuxAmpUnmute i2s0_pin_mux");
    SysWritelI2s((uintptr_t)regIocfg2Base + I2S_IOCFG2_BASE1, I2S_IOCFG2_BASE1_VAL);
    SysWritelI2s((uintptr_t)regIocfg2Base + I2S_IOCFG2_BASE2, I2S_IOCFG2_BASE2_VAL);
    SysWritelI2s((uintptr_t)regIocfg2Base + I2S_IOCFG2_BASE3, I2S_IOCFG2_BASE3_VAL);
    SysWritelI2s((uintptr_t)regIocfg2Base + I2S_IOCFG2_BASE4, I2S_IOCFG2_BASE4_VAL);
    SysWritelI2s((uintptr_t)regIocfg2Base + I2S_IOCFG2_BASE5, I2S_IOCFG2_BASE5_VAL);

    AUDIO_DRIVER_LOG_DEBUG("I2s0PinMuxAmpUnmute AmpUnmute");
    SysWritelI2s((uintptr_t)regIocfg3Base + I2S_IOCFG3_BASE1, I2S_IOCFG3_BASE1_VAL);
    SysWritelI2s((uintptr_t)regGpioBase + GPIO_BASE1, GPIO_BASE3_VAL);
    SysWritelI2s((uintptr_t)regGpioBase + GPIO_BASE2, GPIO_BASE2_VAL);
    SysWritelI2s((uintptr_t)regGpioBase + GPIO_BASE3, GPIO_BASE3_VAL);

    return HDF_SUCCESS;
}


int32_t AudioPlatformDeviceInit(const struct AudioCard *card, const struct PlatformDevice *platform)
{
    struct PlatformHost *platformHost = NULL;
    int ret;
    unsigned int chnId = 0;

    if (platform == NULL || platform->device == NULL) {
        AUDIO_DRIVER_LOG_ERR("platform is NULL.");
        return HDF_FAILURE;
    }
    (void)card;

    platformHost = (struct PlatformHost *)platform->device->service;
    if (platformHost == NULL) {
        AUDIO_DRIVER_LOG_ERR("platform host is NULL.");
        return HDF_FAILURE;
    }

    if (platformHost->platformInitFlag == true) {
        AUDIO_DRIVER_LOG_DEBUG("platform init complete!");
        return HDF_SUCCESS;
    }
    platformHost->platformInitFlag = true;

    ret = AiaoHalSysInit();
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("AiaoHalSysInit:  faile.");
        return HDF_FAILURE;
    }

    /* PIN MUX */
    ret = AiaoSysPinMux();
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("AiaoSysPinMux fail.");
        return HDF_FAILURE;
    }

    /* CLK reset */
    ret = AiaoClockReset();
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("AiaoClockReset:  faile.");
        return HDF_FAILURE;
    }

    /* aiao init */
    ret = AiaoDeviceInit(chnId);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("AiaoClockReset:  faile.");
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t RenderSetAiaoAttr(struct PlatformHost *platformHost, const struct AudioPcmHwParams *param)
{
    if (platformHost == NULL || param == NULL) {
        AUDIO_DRIVER_LOG_ERR("platform is NULL.");
        return HDF_FAILURE;
    }
    platformHost->renderBufInfo.period = param->period;
    if (param->periodSize < MIN_PERIOD_SIZE || param->periodSize > MAX_PERIOD_SIZE) {
        AUDIO_DRIVER_LOG_ERR("periodSize is invalid.");
        return HDF_FAILURE;
    }
    platformHost->renderBufInfo.periodSize = param->periodSize;
    if (param->periodCount < MIN_PERIOD_COUNT || param->periodCount > MAX_PERIOD_COUNT) {
        AUDIO_DRIVER_LOG_ERR("periodCount is invalid.");
        return HDF_FAILURE;
    }
    platformHost->renderBufInfo.periodCount = param->periodCount;

    platformHost->renderBufInfo.trafBufSize = RENDER_TRAF_BUF_SIZE;
    return HDF_SUCCESS;
}

int32_t CaptureSetAiaoAttr(struct PlatformHost *platformHost, const struct AudioPcmHwParams *param)
{
    if (platformHost == NULL || param == NULL) {
        AUDIO_DRIVER_LOG_ERR("platform is NULL.");
        return HDF_FAILURE;
    }

    platformHost->captureBufInfo.period = param->period;
    if (param->periodSize < MIN_PERIOD_SIZE || param->periodSize > MAX_PERIOD_SIZE) {
        AUDIO_DRIVER_LOG_ERR("periodSize is invalid %d.", param->periodSize);
        return HDF_FAILURE;
    }
    platformHost->captureBufInfo.periodSize = param->periodSize;
    if (param->periodCount < MIN_PERIOD_COUNT || param->periodCount > MAX_PERIOD_COUNT) {
        AUDIO_DRIVER_LOG_ERR("periodCount is invalid %d.", param->periodCount);
        return HDF_FAILURE;
    }
    platformHost->captureBufInfo.periodCount = param->periodCount;

    if (param->silenceThreshold < MIN_PERIOD_SIZE || param->silenceThreshold > MAX_PERIOD_SIZE) {
        AUDIO_DRIVER_LOG_ERR("silenceThreshold is invalid %d.", param->silenceThreshold);
        platformHost->captureBufInfo.trafBufSize = CAPTURE_TRAF_BUF_SIZE;
    } else {
        platformHost->captureBufInfo.trafBufSize = param->silenceThreshold;
    }
    return HDF_SUCCESS;
}

int32_t PlatformHwParams(const struct AudioCard *card, const struct AudioPcmHwParams *param)
{
    int ret;
    const int chnlCntMin = 1;
    const int chnlCntMax = 2;
    struct PlatformHost *platformHost = NULL;
    AUDIO_DRIVER_LOG_DEBUG("entry.");

    if (card == NULL || card->rtd == NULL || card->rtd->platform == NULL ||
        param == NULL || param->cardServiceName == NULL) {
        AUDIO_DRIVER_LOG_ERR("input para is NULL.");
        return HDF_FAILURE;
    }

    if (param->channels < chnlCntMin || param->channels > chnlCntMax) {
        AUDIO_DRIVER_LOG_ERR("channels para is invalid.");
        return HDF_FAILURE;
    }

    platformHost = PlatformHostFromDevice(card->rtd->platform->device);
    if (platformHost == NULL) {
        AUDIO_DRIVER_LOG_ERR("platformHost is null.");
        return HDF_FAILURE;
    }

    platformHost->pcmInfo.rate     = param->rate;
    platformHost->pcmInfo.frameSize = param->channels * platformHost->pcmInfo.bitWidth / BITSTOBYTE;

    platformHost->renderBufInfo.chnId = 0;
    platformHost->captureBufInfo.chnId = 0;

    platformHost->pcmInfo.isBigEndian = param->isBigEndian;
    platformHost->pcmInfo.isSignedData = param->isSignedData;

    platformHost->pcmInfo.startThreshold = param->startThreshold;
    platformHost->pcmInfo.stopThreshold = param->stopThreshold;
    platformHost->pcmInfo.silenceThreshold = param->silenceThreshold;

    if (param->streamType == AUDIO_RENDER_STREAM) {
        ret = RenderSetAiaoAttr(platformHost, param);
        if (ret != HDF_SUCCESS) {
            AUDIO_DRIVER_LOG_ERR("AoSetClkAttr:  fail.");
            return HDF_FAILURE;
        }
    } else if (param->streamType == AUDIO_CAPTURE_STREAM) {
        ret = CaptureSetAiaoAttr(platformHost, param);
        if (ret != HDF_SUCCESS) {
            AUDIO_DRIVER_LOG_ERR("AiSetClkAttr:  fail.");
            return HDF_FAILURE;
        }
    } else {
        AUDIO_DRIVER_LOG_ERR("param streamType is invalid.");
        return HDF_FAILURE;
    }
    AUDIO_DRIVER_LOG_DEBUG("success.");
    return HDF_SUCCESS;
}

int32_t PlatformCreatePlatformHost(const struct AudioCard *card, struct PlatformHost **platformHost)
{
    if (platformHost == NULL) {
        AUDIO_DRIVER_LOG_ERR("input param platformHost is invalid.");
        return HDF_ERR_INVALID_PARAM;
    }
    if (card == NULL || card->rtd == NULL || card->rtd->platform == NULL) {
        AUDIO_DRIVER_LOG_ERR("input para is NULL.");
        return HDF_FAILURE;
    }

    *platformHost = PlatformHostFromDevice(card->rtd->platform->device);
    if (*platformHost == NULL) {
        AUDIO_DRIVER_LOG_ERR("PlatformHostFromDevice faile.");
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t PlatformRenderPrepare(const struct AudioCard *card)
{
    int ret;
    struct PlatformHost *platformHost = NULL;
    AUDIO_DRIVER_LOG_DEBUG("PlatformPrepare: entry.");

    ret = PlatformCreatePlatformHost(card, &platformHost);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("PlatformCreatePlatformHost failed.");
        return HDF_FAILURE;
    }

    if (platformHost->renderBufInfo.virtAddr == NULL) {
        ret = AudioRenderBuffInit(platformHost);
        if (ret != HDF_SUCCESS) {
            AUDIO_DRIVER_LOG_ERR("AudioRenderBuffInit: fail.");
            return HDF_FAILURE;
        }

        ret = AudioAoInit(platformHost);
        if (ret != HDF_SUCCESS) {
            AUDIO_DRIVER_LOG_ERR("AudioAoInit: fail.");
            return HDF_FAILURE;
        }
    }

    (void)memset_s(platformHost->renderBufInfo.virtAddr,
                   platformHost->renderBufInfo.cirBufSize, 0,
                   platformHost->renderBufInfo.cirBufSize);
    platformHost->renderBufInfo.wbufOffSet = 0;
    platformHost->renderBufInfo.wptrOffSet = 0;
    platformHost->pcmInfo.totalStreamSize = 0;

    ret = AopHalSetBuffWptr(0, 0);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("AopHalSetBuffWptr: failed.");
        return HDF_FAILURE;
    }

    ret = AopHalSetBuffRptr(0, 0);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("AopHalSetBuffRptr: failed.");
        return HDF_FAILURE;
    }
    AUDIO_DRIVER_LOG_DEBUG("success.");
    return HDF_SUCCESS;
}

int32_t PlatformCapturePrepare(const struct AudioCard *card)
{
    int ret;
    struct PlatformHost *platformHost = NULL;
    AUDIO_DRIVER_LOG_DEBUG("entry.");

    ret = PlatformCreatePlatformHost(card, &platformHost);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("PlatformCreatePlatformHost failed.");
        return HDF_FAILURE;
    }

    if (platformHost->captureBufInfo.virtAddr == NULL) {
        ret = AudioCaptureBuffInit(platformHost);
        if (ret != HDF_SUCCESS) {
            AUDIO_DRIVER_LOG_ERR("AudioCaptureBuffInit: fail.");
            return HDF_FAILURE;
        }
        ret = AudioAiInit(platformHost);
        if (ret != HDF_SUCCESS) {
            AUDIO_DRIVER_LOG_ERR("AudioAiInit: fail.");
            return HDF_FAILURE;
        }
    }

    (void)memset_s(platformHost->captureBufInfo.virtAddr,
                   platformHost->captureBufInfo.cirBufSize, 0,
                   platformHost->captureBufInfo.cirBufSize);
    platformHost->captureBufInfo.wbufOffSet = 0;
    platformHost->captureBufInfo.wptrOffSet = 0;
    platformHost->captureBufInfo.chnId = 0;
    platformHost->pcmInfo.totalStreamSize = 0;

    ret = AipHalSetBuffWptr(0, 0);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("AipHalSetBuffWptr: failed.");
        return HDF_FAILURE;
    }

    ret = AipHalSetBuffRptr(0, 0);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("AipHalSetBuffRptr: failed.");
        return HDF_FAILURE;
    }

    AUDIO_DRIVER_LOG_DEBUG("success.");
    return HDF_SUCCESS;
}

int32_t AudioDataBigEndianChange(char *srcData, uint32_t audioLen, enum AuidoBitWidth bitWidth)
{
    uint64_t i;
    uint16_t framesize;
    char temp;
    if (srcData == NULL) {
        AUDIO_DRIVER_LOG_ERR("srcData is NULL.");
        return HDF_FAILURE;
    }

    switch (bitWidth) {
        case BIT_WIDTH8:
            framesize = 1; /* 1 byte */
            break;
        case BIT_WIDTH16:
            framesize = 2; /* 2 bytes */
            break;
        case BIT_WIDTH24:
            framesize = 3; /* 3 bytes */
            break;
        default:
            framesize = 2; /* default 2 bytes */
            break;
    }

    for (i = 0; i < audioLen; i += framesize) {
        temp = srcData[i];
        srcData[i] = srcData[i + framesize - 1];
        srcData[i + framesize - 1] = temp;
    }
    AUDIO_DRIVER_LOG_DEBUG("audioLen = %d\n", audioLen);
    return HDF_SUCCESS;
}


static int32_t UpdateWriteBufData(struct PlatformHost *platformHost, unsigned int wptr,
    unsigned int buffSize, unsigned int *buffOffset, const char *buf)
{
    int ret;
    unsigned int buffFirstSize;
    unsigned int buffSecordSize;
    if (platformHost == NULL || buffOffset == NULL || buf == NULL) {
        AUDIO_DRIVER_LOG_ERR("input param is invalid.");
        return HDF_ERR_INVALID_PARAM;
    }

    if (platformHost->renderBufInfo.cirBufSize - wptr >= buffSize) {
        *buffOffset = wptr + buffSize;
        ret = memcpy_s((char *)(platformHost->renderBufInfo.virtAddr) + wptr,
                       buffSize, buf, buffSize);
        if (ret != 0) {
            AUDIO_DRIVER_LOG_ERR("memcpy_s failed.");
            return HDF_FAILURE;
        }
        if (*buffOffset >= platformHost->renderBufInfo.cirBufSize) {
            *buffOffset = 0;
        }
    } else {
        buffFirstSize = platformHost->renderBufInfo.cirBufSize - wptr;
        ret = memcpy_s((char *)(platformHost->renderBufInfo.virtAddr) + wptr,
                       buffFirstSize, buf, buffFirstSize);
        if (ret != 0) {
            AUDIO_DRIVER_LOG_ERR("memcpy_s failed.");
            return HDF_FAILURE;
        }

        buffSecordSize = buffSize - buffFirstSize;
        ret = memcpy_s((char *)platformHost->renderBufInfo.virtAddr,
                       buffSecordSize, buf + buffFirstSize,
                       buffSecordSize);
        if (ret != 0) {
            AUDIO_DRIVER_LOG_ERR("memcpy_s failed.");
            return HDF_FAILURE;
        }
        *buffOffset = buffSecordSize;
    }
    return HDF_SUCCESS;
}

static int32_t UpdateWriteBuffOffset(struct PlatformHost *platformHost,
    unsigned int buffSize, unsigned int *buffOffset, struct AudioTxData *txData)
{
    unsigned int buffAvailable;
    int ret;
    unsigned int wptr;
    unsigned int rptr;
    int devId;
    if (platformHost == NULL || buffOffset == NULL || txData == NULL) {
        AUDIO_DRIVER_LOG_ERR("input param is invalid.");
        return HDF_ERR_INVALID_PARAM;
    }

    devId = platformHost->renderBufInfo.chnId;
    rptr = AiaoHalReadReg(AopBuffRptrReg(devId));
    wptr = AiaoHalReadReg(AopBuffWptrReg(devId));
    AUDIO_DRIVER_LOG_DEBUG("rptrReg = [0x%08x, wptrReg = [0x%08x], input size = [%u]",
        rptr, wptr, buffSize);

    if (wptr >= rptr) {
        // [S ... R ... W ... E]
        buffAvailable = platformHost->renderBufInfo.cirBufSize - (wptr - rptr);

        if (buffAvailable < buffSize + AUDIO_BUFF_MIN) {
            AUDIO_DRIVER_LOG_DEBUG("not available buffer.");
            txData->status = ENUM_CIR_BUFF_FULL;
            return HDF_SUCCESS;
        }

        ret = UpdateWriteBufData(platformHost, wptr, buffSize, buffOffset, txData->buf);
        if (ret != HDF_SUCCESS) {
            AUDIO_DRIVER_LOG_ERR("aop_hal_set_buff_wptr failed.");
            return HDF_FAILURE;
        }
    } else {
        // [S ... W ... R ... E]
        buffAvailable = rptr - wptr;

        if (buffAvailable < buffSize + AUDIO_BUFF_MIN) {
            AUDIO_DRIVER_LOG_DEBUG("not available buffer.");
            txData->status = ENUM_CIR_BUFF_FULL;
            return HDF_SUCCESS;
        }

        *buffOffset = wptr + buffSize;
        ret = memcpy_s((char *)(platformHost->renderBufInfo.virtAddr) + wptr,
                       buffSize, txData->buf, buffSize);
        if (ret != 0) {
            AUDIO_DRIVER_LOG_ERR("memcpy_s failed.");
            return HDF_FAILURE;
        }
    }
    return HDF_SUCCESS;
}

static int32_t SetWriteBuffWptr(struct PlatformHost *platformHost,
    unsigned int buffSize, struct AudioTxData *txData)
{
    int ret;
    int devId;
    int buffOffset;

    if (platformHost == NULL || txData == NULL) {
        AUDIO_DRIVER_LOG_ERR("input param is invalid.");
        return HDF_ERR_INVALID_PARAM;
    }

    devId = platformHost->renderBufInfo.chnId;
    ret = UpdateWriteBuffOffset(platformHost, buffSize, &buffOffset, txData);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("UpdateWptrRptr: failed.");
        return HDF_FAILURE;
    }
    if (txData->status == ENUM_CIR_BUFF_FULL) {
        AUDIO_DRIVER_LOG_DEBUG("not available buffer wait a minute.");
        return HDF_SUCCESS;
    }

    if (platformHost->renderBufInfo.runStatus == 1) {
        platformHost->pcmInfo.totalStreamSize += buffSize;
        ret = AopHalSetBuffWptr(devId, buffOffset);
        if (ret != HDF_SUCCESS) {
            AUDIO_DRIVER_LOG_ERR("AopHalSetBuffWptr failed.");
            return HDF_FAILURE;
        }
    }
    txData->status = ENUM_CIR_BUFF_NORMAL;
    return HDF_SUCCESS;
}

int32_t PlatformWrite(const struct AudioCard *card, struct AudioTxData *txData)
{
    int buffSize;

    unsigned int startThreshold;
    struct PlatformHost *platformHost = NULL;
    AUDIO_DRIVER_LOG_DEBUG("entry.");

    if (card == NULL || card->rtd == NULL || card->rtd->platform == NULL ||
        txData == NULL || txData->buf == NULL) {
        AUDIO_DRIVER_LOG_ERR("param is null.");
        return HDF_FAILURE;
    }

    platformHost = PlatformHostFromDevice(card->rtd->platform->device);
    if (platformHost == NULL) {
        AUDIO_DRIVER_LOG_ERR("PlatformHostFromDevice is invalid.");
        return HDF_FAILURE;
    }

    OsalMutexLock(&platformHost->renderBufInfo.buffMutex);
    if (platformHost->renderBufInfo.virtAddr == NULL) {
        AUDIO_DRIVER_LOG_ERR("renderBufInfo.virtAddr is nullptr.");
        OsalMutexUnlock(&platformHost->renderBufInfo.buffMutex);
        return HDF_FAILURE;
    }
    buffSize = txData->frames * platformHost->pcmInfo.frameSize;
    startThreshold = platformHost->pcmInfo.startThreshold * platformHost->pcmInfo.frameSize;

    if (buffSize >= platformHost->renderBufInfo.cirBufSize) {
        AUDIO_DRIVER_LOG_ERR("stream data too long.");
        OsalMutexUnlock(&platformHost->renderBufInfo.buffMutex);
        return HDF_FAILURE;
    }

    if (platformHost->pcmInfo.isBigEndian) {
        if (AudioDataBigEndianChange(txData->buf, buffSize, platformHost->pcmInfo.bitWidth) != HDF_SUCCESS) {
            AUDIO_DRIVER_LOG_ERR("AudioDataBigEndianChange: failed.");
            OsalMutexUnlock(&platformHost->renderBufInfo.buffMutex);
            return HDF_FAILURE;
        }
    }

    if (platformHost->renderBufInfo.runStatus == 1) {
        if ((platformHost->renderBufInfo.enable == 0) &&
            (platformHost->pcmInfo.totalStreamSize > startThreshold)) {
            AopHalDevEnable(platformHost->captureBufInfo.chnId);
            platformHost->renderBufInfo.enable = 1;
        }
    }

    if (SetWriteBuffWptr(platformHost, buffSize, txData) != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("SetWriteBuffWptr: failed.");
        OsalMutexUnlock(&platformHost->renderBufInfo.buffMutex);
        return HDF_FAILURE;
    }
    OsalMutexUnlock(&platformHost->renderBufInfo.buffMutex);

    AUDIO_DRIVER_LOG_DEBUG("now total = %d", platformHost->pcmInfo.totalStreamSize);
    return HDF_SUCCESS;
}

static int32_t UpdateReadBuffData(const struct PlatformHost *platformHost,
                                  unsigned int *tranferSize, unsigned int *buffOffset,
                                  struct AudioRxData *rxData)
{
    unsigned int dataAvailable;
    unsigned int validData;
    unsigned int wptr;
    unsigned int rptr;
    int devId;
    devId = platformHost->captureBufInfo.chnId;
    // Buf/DMA offset
    rptr = AiaoHalReadReg(AipBuffRptrReg(devId));
    wptr = AiaoHalReadReg(AipBuffWptrReg(devId));
    // Buf manage
    if (wptr >= rptr) {
        // [S ... R ... W ... E]
        dataAvailable = wptr - rptr;

        if (dataAvailable >= platformHost->captureBufInfo.trafBufSize) {
            rxData->buf = (char *)(platformHost->captureBufInfo.virtAddr) + rptr;
            *tranferSize = platformHost->captureBufInfo.trafBufSize;
            *buffOffset = rptr + *tranferSize;
        } else {
            AUDIO_DRIVER_LOG_DEBUG("PlatformRead: not available data.");
            rxData->buf = (char *)(platformHost->captureBufInfo.virtAddr) + rptr;
            rxData->status = ENUM_CIR_BUFF_EMPTY;
            rxData->bufSize = 0;
            rxData->frames = 0;
            return HDF_SUCCESS;
        }

    AUDIO_DRIVER_LOG_DEBUG("tranferSize : %d  buffOffset : %d ", *tranferSize, *buffOffset);
    } else {
        // [S ... W ... R ... E]
        validData = rptr + platformHost->captureBufInfo.trafBufSize;
        if (validData < platformHost->captureBufInfo.cirBufSize) {
            rxData->buf = (char *)(platformHost->captureBufInfo.virtAddr) + rptr;
            *tranferSize = platformHost->captureBufInfo.trafBufSize;
            *buffOffset = rptr + *tranferSize;
        } else {
            rxData->buf = (char *)(platformHost->captureBufInfo.virtAddr) + rptr;
            *tranferSize = platformHost->captureBufInfo.cirBufSize - rptr;
            *buffOffset = 0;
        }
        AUDIO_DRIVER_LOG_DEBUG("tranferSize : %d  rptrReg.u32 : %d ", *tranferSize, rptr);
    }

    AUDIO_DRIVER_LOG_DEBUG("rptrReg = [0x%08x], wptrReg = [0x%08x], max size = [%u]",
        rptr, wptr, platformHost->captureBufInfo.trafBufSize);

    return HDF_SUCCESS;
}

int32_t PlatformRead(const struct AudioCard *card, struct AudioRxData *rxData)
{
    unsigned int buffOffset;
    struct PlatformHost *platformHost = NULL;
    int devId;
    unsigned int tranferSize;

    if (rxData == NULL || card == NULL || card->rtd == NULL || card->rtd->platform == NULL) {
        AUDIO_DRIVER_LOG_ERR("param is null.");
        return HDF_FAILURE;
    }

    platformHost = PlatformHostFromDevice(card->rtd->platform->device);
    if (platformHost == NULL) {
        AUDIO_DRIVER_LOG_ERR("PlatformHostFromDevice: fail.");
        return HDF_FAILURE;
    }
    devId = platformHost->captureBufInfo.chnId;

    OsalMutexLock(&platformHost->captureBufInfo.buffMutex);
    if (platformHost->captureBufInfo.virtAddr == NULL) {
        AUDIO_DRIVER_LOG_ERR("PlatformWrite: capture data buffer is not initialized.");
        OsalMutexUnlock(&platformHost->captureBufInfo.buffMutex);
        return HDF_FAILURE;
    }

    if (UpdateReadBuffData(platformHost, &tranferSize, &buffOffset, rxData) != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("UpdateReadBuffData failed.");
        OsalMutexUnlock(&platformHost->captureBufInfo.buffMutex);
        return HDF_FAILURE;
    }
    OsalMutexUnlock(&platformHost->captureBufInfo.buffMutex);

    if (rxData->status == ENUM_CIR_BUFF_EMPTY) {
        AUDIO_DRIVER_LOG_DEBUG("not available data wait a minute.");
        return HDF_SUCCESS;
    }

    if (!platformHost->pcmInfo.isBigEndian) {
        if (AudioDataBigEndianChange(rxData->buf, tranferSize, platformHost->pcmInfo.bitWidth) != HDF_SUCCESS) {
            AUDIO_DRIVER_LOG_ERR("AudioDataBigEndianChange: failed.");
            return HDF_FAILURE;
        }
    }

    rxData->frames = tranferSize / platformHost->pcmInfo.frameSize;
    rxData->bufSize = tranferSize;
    rxData->status = ENUM_CIR_BUFF_NORMAL;

    if (buffOffset >= platformHost->captureBufInfo.cirBufSize) {
        buffOffset = 0;
    }

    if (AipHalSetBuffRptr(devId, buffOffset) != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("AipHalSetBuffRptr failed.");
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t PlatformRenderStart(const struct AudioCard *card)
{
    int ret;
    struct PlatformHost *platformHost = NULL;

    ret = PlatformCreatePlatformHost(card, &platformHost);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("PlatformCreatePlatformHost failed.");
        return HDF_FAILURE;
    }

    platformHost->renderBufInfo.runStatus = 1;
    platformHost->renderBufInfo.enable = 0;
    ShowAllAcodecRegister();
    ShowAllAiaoRegister();

    AUDIO_DRIVER_LOG_INFO("audio render start");
    return HDF_SUCCESS;
}

int32_t PlatformCaptureStart(const struct AudioCard *card)
{
    int ret;
    struct PlatformHost *platformHost = NULL;

    ret = PlatformCreatePlatformHost(card, &platformHost);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("PlatformCreatePlatformHost failed.");
        return HDF_FAILURE;
    }

    AipHalSetRxStart(platformHost->captureBufInfo.chnId, HI_TRUE);
    ShowAllAcodecRegister();
    ShowAllAiaoRegister();
    AUDIO_DRIVER_LOG_INFO("audio capture start");
    return HDF_SUCCESS;
}

int32_t PlatformRenderStop(const struct AudioCard *card)
{
    int ret;
    struct PlatformHost *platformHost = NULL;

    ret = PlatformCreatePlatformHost(card, &platformHost);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("PlatformCreatePlatformHost failed.");
        return HDF_FAILURE;
    }

    OsalMutexLock(&platformHost->renderBufInfo.buffMutex);
    platformHost->renderBufInfo.runStatus = 0;
    AopHalSetTxStart(platformHost->renderBufInfo.chnId, HI_FALSE);
    ret = AudioRenderBuffFree(platformHost);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("AudioRenderBuffFree failed.");
        OsalMutexUnlock(&platformHost->renderBufInfo.buffMutex);
        return HDF_FAILURE;
    }
    OsalMutexUnlock(&platformHost->renderBufInfo.buffMutex);
    AUDIO_DRIVER_LOG_INFO("audio stream stop");

    return HDF_SUCCESS;
}

int32_t PlatformCaptureStop(const struct AudioCard *card)
{
    int ret;
    struct PlatformHost *platformHost = NULL;

    ret = PlatformCreatePlatformHost(card, &platformHost);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("PlatformCreatePlatformHost failed.");
        return HDF_FAILURE;
    }
    OsalMutexLock(&platformHost->captureBufInfo.buffMutex);
    AipHalSetRxStart(platformHost->captureBufInfo.chnId, HI_FALSE);
    ret = AudioCaptureBuffFree(platformHost);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("AudioCaptureBuffFree failed.");
        OsalMutexUnlock(&platformHost->captureBufInfo.buffMutex);
        return HDF_FAILURE;
    }
    OsalMutexUnlock(&platformHost->captureBufInfo.buffMutex);
    AUDIO_DRIVER_LOG_INFO("audio stream stop");
    return HDF_SUCCESS;
}

int32_t PlatformCapturePause(const struct AudioCard *card)
{
    int ret;
    struct PlatformHost *platformHost = NULL;

    ret = PlatformCreatePlatformHost(card, &platformHost);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("PlatformCreatePlatformHost failed.");
        return HDF_FAILURE;
    }

    AipHalSetRxStart(platformHost->captureBufInfo.chnId, HI_FALSE);
    AUDIO_DRIVER_LOG_INFO("audio stream pause");
    return HDF_SUCCESS;
}

int32_t PlatformRenderPause(const struct AudioCard *card)
{
    int ret;
    struct PlatformHost *platformHost = NULL;

    ret = PlatformCreatePlatformHost(card, &platformHost);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("PlatformCreatePlatformHost failed.");
        return HDF_FAILURE;
    }

    platformHost->renderBufInfo.runStatus = 0;
    AopHalSetTxStart(platformHost->renderBufInfo.chnId, HI_FALSE);

    AUDIO_DRIVER_LOG_INFO("audio stream pause");
    return HDF_SUCCESS;
}

int32_t PlatformRenderResume(const struct AudioCard *card)
{
    int ret = PlatformRenderStart(card);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("PlatformRenderResume failed.");
        return HDF_FAILURE;
    }
    AUDIO_DRIVER_LOG_INFO("audio stream resume");
    return HDF_SUCCESS;
}

int32_t PlatformCaptureResume(const struct AudioCard *card)
{
    int ret;
    (void)card;
    ret = PlatformCaptureStart(card);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("PlatformCaptureStart failed.");
        return HDF_FAILURE;
    }
    AUDIO_DRIVER_LOG_INFO("audio stream resume");
    return HDF_SUCCESS;
}

struct AudioPlatformOps g_platformDeviceOps = {
    .HwParams = PlatformHwParams,
    .Write = PlatformWrite,
    .Read = PlatformRead,
    .RenderPrepare = PlatformRenderPrepare,
    .CapturePrepare = PlatformCapturePrepare,
    .RenderStart = PlatformRenderStart,
    .CaptureStart = PlatformCaptureStart,
    .RenderStop = PlatformRenderStop,
    .CaptureStop = PlatformCaptureStop,
    .RenderPause = PlatformRenderPause,
    .CapturePause = PlatformCapturePause,
    .RenderResume = PlatformRenderResume,
    .CaptureResume = PlatformCaptureResume,
};

struct PlatformData g_platformData = {
    .PlatformInit = AudioPlatformDeviceInit,
    .ops = &g_platformDeviceOps,
};

static int32_t PlatformGetServiceName(const struct HdfDeviceObject *device)
{
    const struct DeviceResourceNode *node = NULL;
    struct DeviceResourceIface *drsOps = NULL;
    int32_t ret;

    if (device == NULL) {
        AUDIO_DRIVER_LOG_ERR("para is NULL.");
        return HDF_FAILURE;
    }

    node = device->property;
    if (node == NULL) {
        AUDIO_DRIVER_LOG_ERR("node is NULL.");
        return HDF_FAILURE;
    }

    drsOps = DeviceResourceGetIfaceInstance(HDF_CONFIG_SOURCE);
    if (drsOps == NULL || drsOps->GetString == NULL) {
        AUDIO_DRIVER_LOG_ERR("get drsops object instance fail!");
        return HDF_FAILURE;
    }

    ret = drsOps->GetString(node, "serviceName", &g_platformData.drvPlatformName, 0);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("read serviceName fail!");
        return ret;
    }

    return HDF_SUCCESS;
}

static int32_t PlatformDriverInit(struct HdfDeviceObject *device)
{
    int32_t ret;

    AUDIO_DRIVER_LOG_DEBUG("entry.\n");
    struct PlatformHost *platformHost = NULL;

    if (device == NULL) {
        AUDIO_DRIVER_LOG_ERR("device is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }

    ret = PlatformGetServiceName(device);
    if (ret !=  HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("get service name fail.");
        return ret;
    }

    ret = AudioSocDeviceRegister(device, (void *)&g_platformData, AUDIO_PLATFORM_DEVICE);
    if (ret !=  HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("register dai fail.");
        return ret;
    }

    platformHost = (struct PlatformHost *)device->service;
    if (NULL != platformHost) {
        OsalMutexInit(&platformHost->renderBufInfo.buffMutex);
        OsalMutexInit(&platformHost->captureBufInfo.buffMutex);
    }

    AUDIO_DRIVER_LOG_DEBUG("success.\n");
    return HDF_SUCCESS;
}

static void PlatformDriverRelease(struct HdfDeviceObject *device)
{
    struct PlatformHost *platformHost = NULL;

    if (device == NULL) {
        AUDIO_DRIVER_LOG_ERR("device is NULL");
        return;
    }

    platformHost = (struct PlatformHost *)device->service;
    if (platformHost == NULL) {
        AUDIO_DRIVER_LOG_ERR("platformHost is NULL");
        return;
    }

    OsalMutexDestroy(&platformHost->renderBufInfo.buffMutex);
    OsalMutexDestroy(&platformHost->captureBufInfo.buffMutex);
    OsalMemFree(platformHost);
}

/* HdfDriverEntry definitions */
struct HdfDriverEntry g_platformDriverEntry = {
    .moduleVersion = 1,
    .moduleName = "DMA_HI3516",
    .Bind = PlatformDriverBind,
    .Init = PlatformDriverInit,
    .Release = PlatformDriverRelease,
};
HDF_INIT(g_platformDriverEntry);
