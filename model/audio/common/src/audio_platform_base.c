/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "audio_platform_base.h"
#include "osal_time.h"
#include "osal_uaccess.h"
#include "audio_driver_log.h"
#include "audio_dma_base.h"
#include "audio_sapm.h"
#include "audio_stream_dispatch.h"

#define HDF_LOG_TAG audio_platform_base
const int MAX_PERIOD_SIZE = 1024 * 16;
const int MAX_PERIOD_COUNT = 32;
const int MIN_PERIOD_COUNT = 4;
const int RENDER_TRAF_BUF_SIZE = 1024;
const int MIN_BUFF_SIZE = 16 * 1024;
const int TIME_OUT_CONST = 50;
const int SLEEP_TIME = 5;

unsigned int SysReadl(unsigned long addr)
{
    return *(volatile unsigned int *)(addr);
}

void SysWritel(unsigned long addr, unsigned int value)
{
    *(volatile unsigned int *)(addr) = value;
}

struct PlatformData *PlatformDataFromCard(const struct AudioCard *card)
{
    if (card == NULL || card->rtd == NULL || card->rtd->platform == NULL) {
        AUDIO_DRIVER_LOG_ERR("param is null.");
        return NULL;
    }
    return card->rtd->platform->devData;
}

uint32_t AudioBytesToFrames(uint32_t frameBits, uint32_t size)
{
    if (size == 0) {
        AUDIO_DRIVER_LOG_ERR("size is null.");
        return 0;
    } else {
        return (frameBits / size);
    }
}


int32_t AudioDataBigEndianChange(char *srcData, uint32_t audioLen, enum DataBitWidth bitWidth)
{
    if (srcData == NULL) {
        AUDIO_DRIVER_LOG_ERR("srcData is NULL.");
        return HDF_FAILURE;
    }
    uint64_t i;
    uint16_t framesize;
    char *changeData = srcData;
    uint32_t *pData = (uint32_t *)changeData;

    switch (bitWidth) {
        case DATA_BIT_WIDTH8:
            return HDF_SUCCESS;
        case DATA_BIT_WIDTH24:
            framesize = 3; /* 3 byte , convert step is 3 byte */
            for (i = 0; i < audioLen; i += framesize) {
                // swap the first and the third byte, second and fourth unchanged
                *pData = ((((*pData) >> 0x10) & 0x000000FF) |
                          ((*pData) & 0xFF00FF00) |
                          (((*pData) << 0x10) & 0x00FF0000));
                changeData += framesize;
                pData = (uint32_t *)changeData;
            }
            break;
        case DATA_BIT_WIDTH16:
        default:
            framesize = 4; /* 2 byte, convert step is 4 byte */
            for (i = 0; i < audioLen; i += framesize) {
                // swap the first and second byte, swap the third and fourth byte
                *pData = ((((*pData) << 0x08) & 0xFF00FF00) |
                          (((*pData) >> 0x08) & 0x00FF00FF));
                pData++;
            }
            break;
    }
    return HDF_SUCCESS;
}

int32_t AudioFramatToBitWidth(enum AudioFormat format, unsigned int *bitWidth)
{
    if (bitWidth == NULL) {
        AUDIO_DRIVER_LOG_ERR("bitWidth is null.");
        return HDF_FAILURE;
    }
    switch (format) {
        case AUDIO_FORMAT_PCM_16_BIT:
            *bitWidth = DATA_BIT_WIDTH16;
            break;

        case AUDIO_FORMAT_PCM_24_BIT:
            *bitWidth = DATA_BIT_WIDTH24;
            break;

        default:
            AUDIO_DRIVER_LOG_ERR("format: %d is not define.", format);
            return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AudioSetPcmInfo(struct PlatformData *platformData, const struct AudioPcmHwParams *param)
{
    if (platformData == NULL || param == NULL) {
        AUDIO_DRIVER_LOG_ERR("platform is NULL.");
        return HDF_FAILURE;
    }
    platformData->pcmInfo.rate = param->rate;
    platformData->pcmInfo.frameSize = param->channels * platformData->pcmInfo.bitWidth / BITSTOBYTE;
    platformData->pcmInfo.channels = param->channels;

    platformData->renderBufInfo.chnId = 0;
    platformData->captureBufInfo.chnId = 0;

    platformData->pcmInfo.isBigEndian = param->isBigEndian;
    platformData->pcmInfo.isSignedData = param->isSignedData;

    platformData->pcmInfo.startThreshold = param->startThreshold;
    platformData->pcmInfo.stopThreshold = param->stopThreshold;
    platformData->pcmInfo.silenceThreshold = param->silenceThreshold;

    platformData->pcmInfo.interleaved = 1;
    platformData->pcmInfo.channels = param->channels;
    platformData->pcmInfo.streamType = param->streamType;

    return HDF_SUCCESS;
}

int32_t AudioSetRenderBufInfo(struct PlatformData *data, const struct AudioPcmHwParams *param)
{
    uint32_t size;

    if (data == NULL || param == NULL) {
        AUDIO_DRIVER_LOG_ERR("platform is NULL.");
        return HDF_FAILURE;
    }
    data->renderBufInfo.period = param->period;
    if (param->periodSize < MIN_PERIOD_SIZE || param->periodSize > MAX_PERIOD_SIZE) {
        AUDIO_DRIVER_LOG_ERR("periodSize is invalid.");
        return HDF_FAILURE;
    }
    data->renderBufInfo.periodSize = param->periodSize;
    if (param->periodCount < MIN_PERIOD_COUNT || param->periodCount > MAX_PERIOD_COUNT) {
        AUDIO_DRIVER_LOG_ERR("periodCount is invalid.");
        return HDF_FAILURE;
    }
    data->renderBufInfo.periodCount = param->periodCount;

    data->renderBufInfo.trafBufSize = RENDER_TRAF_BUF_SIZE;

    size = data->renderBufInfo.periodCount * data->renderBufInfo.periodSize;
    if (size < MIN_BUFF_SIZE || size > MAX_BUFF_SIZE) {
        AUDIO_DRIVER_LOG_ERR("buffSize is invalid.");
        return HDF_FAILURE;
    }

    data->renderBufInfo.cirBufSize = size;
    return HDF_SUCCESS;
}

int32_t AudioSetCaptureBufInfo(struct PlatformData *data, const struct AudioPcmHwParams *param)
{
    if (data == NULL || param == NULL) {
        AUDIO_DRIVER_LOG_ERR("platform is NULL.");
        return HDF_FAILURE;
    }

    data->captureBufInfo.period = param->period;
    if (param->periodSize < MIN_PERIOD_SIZE || param->periodSize > MAX_PERIOD_SIZE) {
        AUDIO_DRIVER_LOG_ERR("periodSize is invalid %d.", param->periodSize);
        return HDF_FAILURE;
    }
    data->captureBufInfo.periodSize = param->periodSize;
    if (param->periodCount < MIN_PERIOD_COUNT || param->periodCount > MAX_PERIOD_COUNT) {
        AUDIO_DRIVER_LOG_ERR("periodCount is invalid %d.", param->periodCount);
        return HDF_FAILURE;
    }
    data->captureBufInfo.periodCount = param->periodCount;

    if (param->silenceThreshold < MIN_PERIOD_SIZE || param->silenceThreshold > MAX_PERIOD_SIZE) {
        AUDIO_DRIVER_LOG_ERR("silenceThreshold is invalid %d.", param->silenceThreshold);
        return HDF_FAILURE;
    }
    data->captureBufInfo.trafBufSize = param->silenceThreshold;
    data->captureBufInfo.cirBufSize = param->periodSize * param->periodCount;
    if (data->captureBufInfo.cirBufSize > data->captureBufInfo.cirBufMax) {
        AUDIO_DRIVER_LOG_ERR("cirBufSize is invalid %d.", data->captureBufInfo.cirBufSize);
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AudioWriteProcBigEndian(const struct PlatformData *data, struct AudioTxData *txData)
{
    uint64_t buffSize;

    if (data == NULL || txData == NULL || txData->buf == NULL) {
        AUDIO_DRIVER_LOG_ERR("input param is null.");
        return HDF_FAILURE;
    }

    buffSize = txData->frames * data->pcmInfo.frameSize;
    if (data->pcmInfo.isBigEndian) {
        if (AudioDataBigEndianChange(txData->buf, buffSize, data->pcmInfo.bitWidth) != HDF_SUCCESS) {
            AUDIO_DRIVER_LOG_ERR("AudioDataBigEndianChange: failed.");
            return HDF_FAILURE;
        }
    }
    return HDF_SUCCESS;
}

static enum CriBuffStatus AudioDmaBuffStatus(const struct AudioCard *card)
{
    uint32_t dataAvailable;
    uint32_t residual;
    uint32_t pointer;
    struct PlatformData *data = NULL;
    uint32_t wptr;
    uint32_t rptr;

    data = PlatformDataFromCard(card);
    if (data == NULL || data->ops == NULL) {
        AUDIO_DRIVER_LOG_ERR("PlatformDataFromCard failed.");
        return HDF_FAILURE;
    }

    if (AudioPcmPointer(card, &pointer) != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("get Pointer failed.");
        return ENUM_CIR_BUFF_FULL;
    }

    if (data->pcmInfo.streamType == AUDIO_RENDER_STREAM) {
        data->renderBufInfo.pointer = pointer;
        rptr = data->renderBufInfo.pointer * data->pcmInfo.frameSize;
        dataAvailable = (data->renderBufInfo.wbufOffSet - rptr) % data->renderBufInfo.cirBufSize;
        residual = data->renderBufInfo.cirBufSize - dataAvailable;
        if ((residual > data->renderBufInfo.trafBufSize)) {
            return ENUM_CIR_BUFF_NORMAL;
        }
        return ENUM_CIR_BUFF_FULL;
    } else if (data->pcmInfo.streamType == AUDIO_CAPTURE_STREAM) {
        rptr = data->captureBufInfo.rptrOffSet;
        wptr = pointer * data->pcmInfo.frameSize;
        data->captureBufInfo.pointer = pointer;

        if (wptr >= rptr) {
            // [S ... R ... W ... E]
            dataAvailable = wptr - rptr;
            if (dataAvailable < data->captureBufInfo.trafBufSize) {
                AUDIO_DRIVER_LOG_DEBUG("empty rptr: %d wptr: %d trafBufSize: %d ", rptr,
                    wptr, data->captureBufInfo.trafBufSize);
                return ENUM_CIR_BUFF_EMPTY;
            }
        }
        AUDIO_DRIVER_LOG_DEBUG("rptr: %d wptr: %d trafBufSize: %d ", rptr, wptr, data->captureBufInfo.trafBufSize);
        return ENUM_CIR_BUFF_NORMAL;
    } else {
        AUDIO_DRIVER_LOG_ERR("streamType is invalead.");
        return ENUM_CIR_BUFF_FULL;
    }
}

int32_t AudioPcmWrite(const struct AudioCard *card, struct AudioTxData *txData)
{
    struct PlatformData *data = NULL;
    enum CriBuffStatus status;
    uint32_t wPtr;
    int ret;

    if (card == NULL || txData == NULL || txData->buf == NULL) {
        AUDIO_DRIVER_LOG_ERR("input param is null.");
        return HDF_FAILURE;
    }

    data = PlatformDataFromCard(card);
    if (data == NULL) {
        AUDIO_DRIVER_LOG_ERR("from PlatformDataFromCard get platformData is NULL.");
        return HDF_FAILURE;
    }

    // 1. Computed buffer size
    data->renderBufInfo.trafBufSize = txData->frames * data->pcmInfo.frameSize;

    // 2. Big Small Exchange
    if (data->pcmInfo.isBigEndian) {
        if (AudioDataBigEndianChange(txData->buf, data->renderBufInfo.trafBufSize,
            data->pcmInfo.bitWidth) != HDF_SUCCESS) {
            AUDIO_DRIVER_LOG_ERR("AudioDataBigEndianChange: failed.");
            return HDF_FAILURE;
        }
    }
    // 3. Buffer state checking
    status = AudioDmaBuffStatus(card);
    if (status != ENUM_CIR_BUFF_NORMAL) {
        txData->status = ENUM_CIR_BUFF_FULL;
        return HDF_SUCCESS;
    }

    // 4. write buffer
    if (data->renderBufInfo.trafBufSize > data->renderBufInfo.cirBufSize) {
        AUDIO_DRIVER_LOG_ERR("transferFrameSize is tool big.");
        return HDF_FAILURE;
    }
    wPtr = data->renderBufInfo.wbufOffSet % data->renderBufInfo.cirBufSize;

    if (data->renderBufInfo.virtAddr == NULL) {
        AUDIO_DRIVER_LOG_ERR("render buffer is null.");
        return HDF_FAILURE;
    }
    ret = memcpy_s((char *)(data->renderBufInfo.virtAddr) + wPtr,
        data->renderBufInfo.trafBufSize, txData->buf, data->renderBufInfo.trafBufSize);
    if (ret != 0) {
        AUDIO_DRIVER_LOG_ERR("memcpy_s failed.");
        return HDF_FAILURE;
    }
    txData->status = ENUM_CIR_BUFF_NORMAL;
    data->renderBufInfo.wptrOffSet = wPtr + data->renderBufInfo.trafBufSize;
    data->renderBufInfo.wbufOffSet += data->renderBufInfo.trafBufSize;

    return HDF_SUCCESS;
}

static int32_t PcmReadData(struct PlatformData *data, struct AudioRxData *rxData)
{
    uint32_t wptr;
    uint32_t rptr;
    uint32_t validDataSize;

    if (data == NULL || rxData == NULL) {
        AUDIO_DRIVER_LOG_ERR("input param is null.");
        return HDF_FAILURE;
    }

    rxData->buf = (char *)(data->captureBufInfo.virtAddr) + data->captureBufInfo.rptrOffSet;
    wptr = data->captureBufInfo.pointer * data->pcmInfo.frameSize;
    rptr = data->captureBufInfo.rptrOffSet;
    data->captureBufInfo.curTrafSize = data->captureBufInfo.trafBufSize;
    if (rptr > wptr) {
        validDataSize = data->captureBufInfo.cirBufSize - rptr;
        if (validDataSize < data->captureBufInfo.trafBufSize) {
            data->captureBufInfo.curTrafSize = validDataSize;
        }
    }

    // 3. Big Small Exchange
    if (!data->pcmInfo.isBigEndian) {
        if (rxData->buf == NULL ||
            AudioDataBigEndianChange(rxData->buf, data->captureBufInfo.curTrafSize,
                data->pcmInfo.bitWidth) != HDF_SUCCESS) {
                AUDIO_DRIVER_LOG_ERR("AudioDataBigEndianChange: failed.");
                return HDF_FAILURE;
        }
    }

    rxData->frames = data->captureBufInfo.curTrafSize / data->pcmInfo.frameSize;
    rxData->bufSize = data->captureBufInfo.curTrafSize;
    rxData->status = ENUM_CIR_BUFF_NORMAL;

    return HDF_SUCCESS;
}

int32_t AudioPcmRead(const struct AudioCard *card, struct AudioRxData *rxData)
{
    struct PlatformData *data = NULL;
    enum CriBuffStatus status;

    if (card == NULL || rxData == NULL) {
        AUDIO_DRIVER_LOG_ERR("input param is null.");
        return HDF_FAILURE;
    }

    data = PlatformDataFromCard(card);
    if (data == NULL) {
        AUDIO_DRIVER_LOG_ERR("from PlatformDataFromCard get platformData is NULL.");
        return HDF_FAILURE;
    }
    if (data->captureBufInfo.virtAddr == NULL) {
        AUDIO_DRIVER_LOG_ERR("capture buffer is null.");
        return HDF_FAILURE;
    }

    // 1. Buffer state checking
    status = AudioDmaBuffStatus(card);
    if (status != ENUM_CIR_BUFF_NORMAL) {
        rxData->status = ENUM_CIR_BUFF_EMPTY;
        rxData->buf = (char *)(data->captureBufInfo.virtAddr) + data->captureBufInfo.rptrOffSet;
        rxData->frames = 0;
        rxData->bufSize = 0;
        AUDIO_DRIVER_LOG_DEBUG("buff is empty.");
        return HDF_SUCCESS;
    }

    // 2. read buffer
    if (PcmReadData(data, rxData) != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("Pcm Read Data fail.");
        return HDF_FAILURE;
    }

    // 4. update rptr
    data->captureBufInfo.rptrOffSet += data->captureBufInfo.curTrafSize;
    if (data->captureBufInfo.rptrOffSet >= data->captureBufInfo.cirBufSize) {
        data->captureBufInfo.rptrOffSet = 0;
    }
    return HDF_SUCCESS;
}

static int32_t MmapWriteData(struct PlatformData *data, char *tmpBuf)
{
    uint32_t wPtr;
    int ret;

    if (data->renderBufInfo.trafBufSize > data->renderBufInfo.cirBufSize) {
        AUDIO_DRIVER_LOG_ERR("transferFrameSize is tool big.");
        return HDF_FAILURE;
    }

    wPtr = data->renderBufInfo.wbufOffSet % data->renderBufInfo.cirBufSize;
    ret = CopyFromUser(tmpBuf, (char *)data->mmapData.memoryAddress + data->mmapData.offset,
        data->renderBufInfo.trafBufSize);
    if (ret != EOK) {
        AUDIO_DRIVER_LOG_ERR("CopyFromUser failed.");
        return HDF_FAILURE;
    }

    ret = memcpy_s((char *)(data->renderBufInfo.virtAddr) + wPtr, data->renderBufInfo.trafBufSize,
        tmpBuf, data->renderBufInfo.trafBufSize);
    if (ret != 0) {
        AUDIO_DRIVER_LOG_ERR("memcpy_s failed.");
        return HDF_FAILURE;
    }

    data->renderBufInfo.wptrOffSet = wPtr + data->renderBufInfo.trafBufSize;
    data->renderBufInfo.wbufOffSet += data->renderBufInfo.trafBufSize;
    data->renderBufInfo.framesPosition += data->renderBufInfo.trafBufSize / data->pcmInfo.frameSize;
    data->mmapData.offset += data->renderBufInfo.trafBufSize;
    data->mmapLoopCount++;
    return HDF_SUCCESS;
}

static int32_t AudioMmapWriteTransfer(const struct AudioCard *card)
{
    struct PlatformData *data = NULL;
    enum CriBuffStatus status;
    uint32_t timeout = 0;

    data = PlatformDataFromCard(card);
    if (data == NULL) {
        AUDIO_DRIVER_LOG_ERR("PlatformDataFromCard failed.");
        return HDF_FAILURE;
    }
    if (data->renderBufInfo.virtAddr == NULL) {
        AUDIO_DRIVER_LOG_ERR("render buffer is null.");
        return HDF_FAILURE;
    }

    uint32_t totalSize = data->mmapData.totalBufferFrames * data->pcmInfo.frameSize;
    uint32_t lastBuffSize = ((totalSize % MIN_PERIOD_SIZE) == 0) ? MIN_PERIOD_SIZE : (totalSize % MIN_PERIOD_SIZE);
    uint32_t loopTimes = (lastBuffSize == MIN_PERIOD_SIZE) ?
        (totalSize / MIN_PERIOD_SIZE) : (totalSize / MIN_PERIOD_SIZE + 1);
    data->mmapLoopCount = 0;
    char *tmpBuf = OsalMemCalloc(MIN_PERIOD_SIZE);

    while (data->mmapLoopCount < loopTimes && data->renderBufInfo.runStatus != PCM_STOP) {
        if (data->renderBufInfo.runStatus == PCM_PAUSE) {
            OsalMSleep(5);
            continue;
        }
        status = AudioDmaBuffStatus(card);
        if (status != ENUM_CIR_BUFF_NORMAL) {
            OsalMSleep(SLEEP_TIME);
            AUDIO_DRIVER_LOG_DEBUG("dma buff status ENUM_CIR_BUFF_FULL.");
            timeout++;
            if (timeout >= TIME_OUT_CONST) {
                AUDIO_DRIVER_LOG_ERR("timeout failed.");
                OsalMemFree(tmpBuf);
                return HDF_FAILURE;
            }
            continue;
        }
        timeout = 0;
        data->renderBufInfo.trafBufSize = (data->mmapLoopCount < (loopTimes - 1)) ? MIN_PERIOD_SIZE : lastBuffSize;

        if (MmapWriteData(data, tmpBuf) != HDF_SUCCESS) {
            AUDIO_DRIVER_LOG_ERR("MmapWriteData fail.");
            OsalMemFree(tmpBuf);
            return HDF_FAILURE;
        }
    }

    if (data->mmapLoopCount > loopTimes) {
        data->renderBufInfo.runStatus = PCM_STOP;
    }

    OsalMemFree(tmpBuf);
    return HDF_SUCCESS;
}

int32_t AudioPcmMmapWrite(const struct AudioCard *card, const struct AudioMmapData *txMmapData)
{
    int32_t ret;
    struct PlatformData *data = NULL;
    AUDIO_DRIVER_LOG_DEBUG("entry.");

    data = PlatformDataFromCard(card);
    if (data == NULL) {
        AUDIO_DRIVER_LOG_ERR("PlatformDataFromCard failed.");
        return HDF_FAILURE;
    }

    if (txMmapData == NULL || txMmapData->memoryAddress == NULL || txMmapData->transferFrameSize <= 0 ||
        txMmapData->totalBufferFrames <= 0) {
        AUDIO_DRIVER_LOG_ERR("param is invalid.");
        return HDF_ERR_INVALID_PARAM;
    }

    data->mmapData.offset = txMmapData->offset;
    data->mmapData.memoryAddress = txMmapData->memoryAddress;
    data->mmapData.totalBufferFrames = txMmapData->totalBufferFrames;
    data->mmapData.transferFrameSize = txMmapData->transferFrameSize;
    data->mmapLoopCount = 0;

    AUDIO_DRIVER_LOG_DEBUG("*******offset=0x%x memoryAddress=0x%x totalBufferFrames=0x%x transferFrameSize=0x%x ",
        data->mmapData.offset, data->mmapData.memoryAddress, data->mmapData.totalBufferFrames,
        data->mmapData.transferFrameSize);

    ret = AudioMmapWriteTransfer(card);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("AudioMmapTransfer fail.");
        return HDF_FAILURE;
    }

    AUDIO_DRIVER_LOG_DEBUG("render mmap write success.");
    return HDF_SUCCESS;
}

static int32_t MmapReadData(struct PlatformData *data, const struct AudioMmapData *rxMmapData, uint32_t offset)
{
    uint32_t wPtr;
    uint32_t rPtr;
    uint32_t validDataSize;

    if (data == NULL || rxMmapData == NULL) {
        AUDIO_DRIVER_LOG_ERR("data is null.");
        return HDF_FAILURE;
    }

    rPtr = data->captureBufInfo.rptrOffSet;
    wPtr = data->captureBufInfo.pointer * data->pcmInfo.frameSize;
    if (rPtr > wPtr) {
        validDataSize = data->captureBufInfo.cirBufSize - rPtr;
        if (validDataSize < data->captureBufInfo.trafBufSize) {
            data->captureBufInfo.curTrafSize = validDataSize;
        }
    }
    if (!data->pcmInfo.isBigEndian) {
        if (AudioDataBigEndianChange((char *)(data->captureBufInfo.virtAddr) + rPtr,
            data->captureBufInfo.curTrafSize, data->pcmInfo.bitWidth) != HDF_SUCCESS) {
                AUDIO_DRIVER_LOG_ERR("AudioDataBigEndianChange: failed.");
                return HDF_FAILURE;
        }
    }

    if (CopyToUser((char *)rxMmapData->memoryAddress + offset,
        (char *)(data->captureBufInfo.virtAddr) + rPtr, data->captureBufInfo.curTrafSize) != 0) {
        AUDIO_DRIVER_LOG_ERR("CopyToUser failed.");
        return HDF_FAILURE;
    }

    // 4. update rptr
    data->captureBufInfo.rptrOffSet += data->captureBufInfo.curTrafSize;
    if (data->captureBufInfo.rptrOffSet >= data->captureBufInfo.cirBufSize) {
        data->captureBufInfo.rptrOffSet = 0;
    }
    data->captureBufInfo.framesPosition += data->captureBufInfo.curTrafSize / data->pcmInfo.frameSize;

    return HDF_SUCCESS;
}

int32_t AudioMmapReadTransfer(const struct AudioCard *card, const struct AudioMmapData *rxMmapData)
{
    uint32_t offset = 0;
    enum CriBuffStatus status;
    uint32_t timeout = 0;
    struct PlatformData *data = NULL;

    if (!card || !rxMmapData || !rxMmapData->memoryAddress || rxMmapData->totalBufferFrames <= 0) {
        AUDIO_DRIVER_LOG_ERR("param is invalid.");
        return HDF_ERR_INVALID_PARAM;
    }

    data = PlatformDataFromCard(card);
    if (data == NULL) {
        AUDIO_DRIVER_LOG_ERR("PlatformDataFromCard failed.");
        return HDF_FAILURE;
    }

    uint32_t frameSize = data->pcmInfo.frameSize;
    uint32_t totalSize = rxMmapData->totalBufferFrames * frameSize;
    data->captureBufInfo.pointer = 0;
    data->captureBufInfo.curTrafSize = data->captureBufInfo.trafBufSize;
    if (data->captureBufInfo.virtAddr == NULL) {
        AUDIO_DRIVER_LOG_ERR("capture buffer is null.");
        return HDF_FAILURE;
    }

    do {
        if (data->captureBufInfo.runStatus == PCM_PAUSE) {
            OsalMSleep(5);
            continue;
        }

        // 1. get buffer status
        status = AudioDmaBuffStatus(card);
        if (status != ENUM_CIR_BUFF_NORMAL) {
            OsalMSleep(SLEEP_TIME);
            AUDIO_DRIVER_LOG_DEBUG("dma buff status ENUM_CIR_BUFF_FULL.");
            timeout++;
            if (timeout >= TIME_OUT_CONST) {
                AUDIO_DRIVER_LOG_ERR("timeout failed.");
                return HDF_FAILURE;
            }
            continue;
        }
        timeout = 0;

        // 2. read data
        if (MmapReadData(data, rxMmapData, offset) != HDF_SUCCESS) {
            AUDIO_DRIVER_LOG_ERR("MmapReadData fail.");
            return HDF_FAILURE;
        }
        offset += data->captureBufInfo.curTrafSize;

    } while (offset < totalSize && data->captureBufInfo.runStatus != 0);
    return HDF_SUCCESS;
}

int32_t AudioPcmMmapRead(const struct AudioCard *card, const struct AudioMmapData *rxMmapData)
{
    int32_t ret;
    struct PlatformData *data = NULL;
    AUDIO_DRIVER_LOG_DEBUG("entry.");

    ret = (card == NULL || rxMmapData == NULL || rxMmapData->memoryAddress == NULL
        || rxMmapData->totalBufferFrames <= 0);
    if (ret) {
        AUDIO_DRIVER_LOG_ERR("param is invalid.");
        return HDF_ERR_INVALID_PARAM;
    }

    data = PlatformDataFromCard(card);
    if (data == NULL) {
        AUDIO_DRIVER_LOG_ERR("PlatformDataFromCard failed.");
        return HDF_FAILURE;
    }

    ret = AudioMmapReadTransfer(card, rxMmapData);
    if (ret) {
        AUDIO_DRIVER_LOG_ERR("AudioMmapReadTransfer fail.");
        return HDF_ERR_INVALID_PARAM;
    }

    AUDIO_DRIVER_LOG_DEBUG("success.");
    return HDF_SUCCESS;
}

static int32_t AudioRenderBuffInit(struct PlatformData *platformData)
{
    int ret;

    if (platformData == NULL) {
        AUDIO_DRIVER_LOG_ERR("input para is NULL.");
        return HDF_FAILURE;
    }

    if (platformData->renderBufInfo.virtAddr != NULL) {
        return HDF_SUCCESS;
    }

    platformData->renderBufInfo.cirBufMax = MAX_BUFF_SIZE;
    platformData->renderBufInfo.phyAddr = 0;

    ret = AudioDmaBufAlloc(platformData, AUDIO_RENDER_STREAM);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("Dma Buf Alloc fail.");
        return HDF_FAILURE;
    }

    if (platformData->renderBufInfo.virtAddr == NULL) {
        AUDIO_DRIVER_LOG_ERR("mem alloc failed.");
        return HDF_FAILURE;
    }

    (void)memset_s(platformData->renderBufInfo.virtAddr, platformData->renderBufInfo.cirBufMax,
        0, platformData->renderBufInfo.cirBufMax);

    return HDF_SUCCESS;
}

static int32_t AudioRenderBuffFree(struct PlatformData *platformData)
{
    int ret;

    if (platformData == NULL) {
        AUDIO_DRIVER_LOG_ERR("input param is NULL.");
        return HDF_FAILURE;
    }

    ret = AudioDmaBufFree(platformData, AUDIO_RENDER_STREAM);
    if (ret) {
        AUDIO_DRIVER_LOG_ERR("Dma Buf Alloc fail.");
        return HDF_FAILURE;
    }

    platformData->renderBufInfo.virtAddr = NULL;
    platformData->renderBufInfo.phyAddr = 0;
    return HDF_SUCCESS;
}

static int32_t AudioCaptureBuffInit(struct PlatformData *platformData)
{
    int ret;

    if (platformData == NULL) {
        AUDIO_DRIVER_LOG_ERR("input param is NULL.");
        return HDF_FAILURE;
    }
    if (platformData->captureBufInfo.virtAddr != NULL) {
        return HDF_SUCCESS;
    }

    platformData->captureBufInfo.cirBufMax = MAX_BUFF_SIZE;
    platformData->captureBufInfo.phyAddr = 0;

    ret = AudioDmaBufAlloc(platformData, AUDIO_CAPTURE_STREAM);
    if (ret) {
        AUDIO_DRIVER_LOG_ERR("Dma Buf Alloc fail.");
        return HDF_FAILURE;
    }

    if (platformData->captureBufInfo.virtAddr == NULL) {
        AUDIO_DRIVER_LOG_ERR("mem alloc failed.");
        return HDF_FAILURE;
    }

    (void)memset_s(platformData->captureBufInfo.virtAddr, platformData->captureBufInfo.cirBufMax, 0,
                   platformData->captureBufInfo.cirBufMax);

    return HDF_SUCCESS;
}

static int32_t AudioCaptureBuffFree(struct PlatformData *platformData)
{
    int ret;

    if (platformData == NULL) {
        AUDIO_DRIVER_LOG_ERR("input param is NULL.");
        return HDF_FAILURE;
    }

    ret = AudioDmaBufFree(platformData, AUDIO_CAPTURE_STREAM);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("Dma Buf Alloc fail.");
        return HDF_FAILURE;
    }

    platformData->captureBufInfo.virtAddr = NULL;
    platformData->captureBufInfo.phyAddr = 0;
    return HDF_SUCCESS;
}

int32_t AudioRenderOpen(const struct AudioCard *card)
{
    struct PlatformData *data = PlatformDataFromCard(card);
    if (data == NULL) {
        AUDIO_DRIVER_LOG_ERR("PlatformDataFromCard failed.");
        return HDF_FAILURE;
    }

    if (data->renderBufInfo.virtAddr == NULL) {
        if (AudioRenderBuffInit(data) != HDF_SUCCESS) {
            return HDF_FAILURE;
        }
    }
    return HDF_SUCCESS;
}

int32_t AudioRenderClose(const struct AudioCard *card)
{
    struct PlatformData *data = PlatformDataFromCard(card);
    if (data == NULL) {
        AUDIO_DRIVER_LOG_ERR("PlatformDataFromCard failed.");
        return HDF_FAILURE;
    }

    return AudioRenderBuffFree(data);
}

int32_t AudioCaptureOpen(const struct AudioCard *card)
{
    if (card == NULL) {
        AUDIO_DRIVER_LOG_ERR("param card is NULL.");
        return HDF_FAILURE;
    }

    struct PlatformData *platformData = PlatformDataFromCard(card);
    if (platformData == NULL) {
        AUDIO_DRIVER_LOG_ERR("PlatformDataFromCard failed.");
        return HDF_FAILURE;
    }

    if (platformData->captureBufInfo.virtAddr == NULL) {
        if (AudioCaptureBuffInit(platformData) != HDF_SUCCESS) {
            AUDIO_DRIVER_LOG_ERR("AudioCaptureBuffInit: fail.");
            return HDF_FAILURE;
        }
    }
    return HDF_SUCCESS;
}

int32_t AudioCaptureClose(const struct AudioCard *card)
{
    if (card == NULL) {
        AUDIO_DRIVER_LOG_ERR("param card is NULL.");
        return HDF_FAILURE;
    }

    struct PlatformData *platformData = PlatformDataFromCard(card);
    if (platformData == NULL) {
        AUDIO_DRIVER_LOG_ERR("PlatformDataFromCard failed.");
        return HDF_FAILURE;
    }
    return AudioCaptureBuffFree(platformData);
}

static int32_t AudioPcmPending(struct AudioCard *card)
{
    struct PlatformData *data = PlatformDataFromCard(card);
    if (data == NULL) {
        AUDIO_DRIVER_LOG_ERR("PlatformDataFromCard failed.");
        return HDF_FAILURE;
    }

    if (AudioDmaSubmit(data) != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("DmaPending fail.");
        return HDF_FAILURE;
    }

    if (AudioDmaPending(data) != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("DmaPending fail.");
        return HDF_FAILURE;
    }

    if (AudioSampSetPowerMonitor(card, false) != HDF_SUCCESS) {
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

static int32_t AudioPcmPause(struct AudioCard *card)
{
    struct PlatformData *data = PlatformDataFromCard(card);
    if (data == NULL) {
        AUDIO_DRIVER_LOG_ERR("PlatformDataFromCard failed.");
        return HDF_FAILURE;
    }

    if (AudioDmaPause(data) != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("DmaPause fail.");
        return HDF_FAILURE;
    }

    if (AudioSampSetPowerMonitor(card, true) != HDF_SUCCESS) {
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

static int32_t AudioPcmResume(struct AudioCard *card)
{
    struct PlatformData *data = PlatformDataFromCard(card);
    if (data == NULL) {
        AUDIO_DRIVER_LOG_ERR("PlatformDataFromCard failed.");
        return HDF_FAILURE;
    }

    if (AudioDmaResume(data) != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("DmaPause fail.");
        return HDF_FAILURE;
    }

    if (AudioSampPowerUp(card) != HDF_SUCCESS) {
        return HDF_FAILURE;
    }
    if (AudioSampSetPowerMonitor(card, false) != HDF_SUCCESS) {
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

int32_t AudioRenderTrigger(struct AudioCard *card, int cmd)
{
    struct PlatformData *data = PlatformDataFromCard(card);
    if (data == NULL) {
        AUDIO_DRIVER_LOG_ERR("PlatformDataFromCard failed.");
        return HDF_FAILURE;
    }

    switch (cmd) {
        case AUDIO_DRV_PCM_IOCTL_RENDER_START:
            if (AudioPcmPending(card) != HDF_SUCCESS) {
                AUDIO_DRIVER_LOG_ERR("AudioPcmPending fail.");
                return HDF_FAILURE;
            }

            data->renderBufInfo.runStatus = PCM_START;
            break;
        case AUDIO_DRV_PCM_IOCTL_RENDER_STOP:
            if (AudioPcmPause(card) != HDF_SUCCESS) {
                AUDIO_DRIVER_LOG_ERR("AudioPcmPause fail.");
                return HDF_FAILURE;
            }

            data->renderBufInfo.runStatus = PCM_STOP;
            break;
        case AUDIO_DRV_PCM_IOCTL_RENDER_PAUSE:
            if (AudioPcmPause(card) != HDF_SUCCESS) {
                AUDIO_DRIVER_LOG_ERR("AudioPcmPause fail.");
                return HDF_FAILURE;
            }

            data->renderBufInfo.runStatus = PCM_PAUSE;
            break;
        case AUDIO_DRV_PCM_IOCTL_RENDER_RESUME:
            if (AudioPcmResume(card) != HDF_SUCCESS) {
                AUDIO_DRIVER_LOG_ERR("AudioPcmResume fail.");
                return HDF_FAILURE;
            }

            data->renderBufInfo.runStatus = PCM_START;
            break;
        default:
            AUDIO_DRIVER_LOG_ERR("invalude cmd id: %d.", cmd);
            return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AudioCaptureTrigger(struct AudioCard *card, int cmd)
{
    struct PlatformData *data = PlatformDataFromCard(card);
    if (data == NULL) {
        AUDIO_DRIVER_LOG_ERR("PlatformDataFromCard failed.");
        return HDF_FAILURE;
    }

    switch (cmd) {
        case AUDIO_DRV_PCM_IOCTL_CAPTURE_START:
            if (AudioPcmPending(card) != HDF_SUCCESS) {
                AUDIO_DRIVER_LOG_ERR("AudioPcmPending fail.");
                return HDF_FAILURE;
            }

            data->captureBufInfo.runStatus = PCM_START;
            break;
        case AUDIO_DRV_PCM_IOCTL_CAPTURE_STOP:
            if (AudioPcmPause(card) != HDF_SUCCESS) {
                AUDIO_DRIVER_LOG_ERR("AudioPcmPause fail.");
                return HDF_FAILURE;
            }

            data->captureBufInfo.runStatus = PCM_STOP;
            break;
        case AUDIO_DRV_PCM_IOCTL_CAPTURE_PAUSE:
            if (AudioPcmPause(card) != HDF_SUCCESS) {
                AUDIO_DRIVER_LOG_ERR("AudioPcmPause fail.");
                return HDF_FAILURE;
            }

            data->captureBufInfo.runStatus = PCM_PAUSE;
            break;
        case AUDIO_DRV_PCM_IOCTL_CAPTURE_RESUME:
            if (AudioPcmResume(card) != HDF_SUCCESS) {
                AUDIO_DRIVER_LOG_ERR("AudioPcmResume fail.");
                return HDF_FAILURE;
            }

            data->captureBufInfo.runStatus = PCM_START;
            break;
        default:
            AUDIO_DRIVER_LOG_ERR("invalude cmd id: %d.", cmd);
            return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AudioHwParams(const struct AudioCard *card, const struct AudioPcmHwParams *param)
{
    const int chnlCntMin = 1;
    const int chnlCntMax = 2;

    if (card == NULL || param == NULL || param->cardServiceName == NULL) {
        AUDIO_DRIVER_LOG_ERR("input param is NULL.");
        return HDF_FAILURE;
    }

    if (param->channels < chnlCntMin || param->channels > chnlCntMax) {
        AUDIO_DRIVER_LOG_ERR("channels param is invalid.");
        return HDF_FAILURE;
    }

    struct PlatformData *platformData = PlatformDataFromCard(card);
    if (platformData == NULL) {
        AUDIO_DRIVER_LOG_ERR("platformData is null.");
        return HDF_FAILURE;
    }

    if (AudioFramatToBitWidth(param->format, &platformData->pcmInfo.bitWidth) != HDF_SUCCESS) {
        return HDF_FAILURE;
    }
    if (AudioSetPcmInfo(platformData, param) != HDF_SUCCESS) {
        return HDF_FAILURE;
    }

    if (AudioDmaRequestChannel(platformData) != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("Dma Request Channel fail.");
        return HDF_FAILURE;
    }

    if (param->streamType == AUDIO_RENDER_STREAM) {
        if (AudioSetRenderBufInfo(platformData, param) != HDF_SUCCESS) {
            return HDF_FAILURE;
        }
    } else if (param->streamType == AUDIO_CAPTURE_STREAM) {
        if (AudioSetCaptureBufInfo(platformData, param) != HDF_SUCCESS) {
            return HDF_FAILURE;
        }
    } else {
        AUDIO_DRIVER_LOG_ERR("param streamType is invalid.");
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

int32_t AudioRenderPrepare(const struct AudioCard *card)
{
    int ret;
    if (card == NULL) {
        AUDIO_DRIVER_LOG_ERR("param card is NULL.");
        return HDF_FAILURE;
    }

    struct PlatformData *platformData = PlatformDataFromCard(card);
    if (platformData == NULL) {
        AUDIO_DRIVER_LOG_ERR("PlatformDataFromCard failed.");
        return HDF_FAILURE;
    }

    if (platformData->renderBufInfo.virtAddr != NULL) {
        (void)memset_s(platformData->renderBufInfo.virtAddr, platformData->renderBufInfo.cirBufSize, 0,
                       platformData->renderBufInfo.cirBufSize);
    }
    platformData->renderBufInfo.wbufOffSet = 0;
    platformData->renderBufInfo.wptrOffSet = 0;
    platformData->pcmInfo.totalStreamSize = 0;
    platformData->renderBufInfo.framesPosition = 0;
    platformData->renderBufInfo.pointer = 0;

    ret = AudioDmaConfigChannel(platformData);
    if (ret) {
        AUDIO_DRIVER_LOG_ERR("Dma Config Channel fail.");
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

int32_t AudioCapturePrepare(const struct AudioCard *card)
{
    int ret;
    if (card == NULL) {
        AUDIO_DRIVER_LOG_ERR("param card is NULL.");
        return HDF_FAILURE;
    }

    struct PlatformData *platformData = PlatformDataFromCard(card);
    if (platformData == NULL) {
        AUDIO_DRIVER_LOG_ERR("PlatformDataFromCard failed.");
        return HDF_FAILURE;
    }

    if (platformData->captureBufInfo.virtAddr != NULL) {
        (void)memset_s(platformData->captureBufInfo.virtAddr, platformData->captureBufInfo.cirBufSize, 0,
                       platformData->captureBufInfo.cirBufSize);
    }
    platformData->captureBufInfo.rbufOffSet = 0;
    platformData->captureBufInfo.rptrOffSet = 0;
    platformData->captureBufInfo.chnId = 0;
    platformData->pcmInfo.totalStreamSize = 0;
    platformData->captureBufInfo.framesPosition = 0;
    platformData->captureBufInfo.pointer = 0;

    ret = AudioDmaConfigChannel(platformData);
    if (ret) {
        AUDIO_DRIVER_LOG_ERR("Dma Config Channel fail.");
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

int32_t AudioPcmPointer(const struct AudioCard *card, uint32_t *pointer)
{
    int ret;
    if (card == NULL || pointer == NULL) {
        AUDIO_DRIVER_LOG_ERR("param card is NULL.");
        return HDF_FAILURE;
    }

    struct PlatformData *data = PlatformDataFromCard(card);
    if (data == NULL) {
        AUDIO_DRIVER_LOG_ERR("PlatformDataFromCard failed.");
        return HDF_FAILURE;
    }

    ret = AudioDmaPointer(data, pointer);
    if (ret != HDF_SUCCESS) {
        AUDIO_DRIVER_LOG_ERR("Dma Pointer fail.");
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}
