/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef AUDIO_PLATFORM_BASE_H
#define AUDIO_PLATFORM_BASE_H

#include "audio_platform_if.h"
#include "audio_host.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define MIN_PERIOD_SIZE 4096
#define BITSTOBYTE 8
#define MAX_BUFF_SIZE (64 * 1024)

enum DataBitWidth {
    DATA_BIT_WIDTH8  =  8,      /* 8 bit witdth */
    DATA_BIT_WIDTH16 =  16,     /* 16 bit witdth */
    DATA_BIT_WIDTH18 =  18,     /* 18 bit witdth */
    DATA_BIT_WIDTH20 =  20,     /* 20 bit witdth */
    DATA_BIT_WIDTH24 =  24,     /* 24 bit witdth */
    DATA_BIT_WIDTH32 =  32,     /* 32 bit witdth */
};

unsigned int SysReadl(unsigned long addr);
void SysWritel(unsigned long addr, unsigned int value);

struct PlatformData *PlatformDataFromCard(const struct AudioCard *card);
uint32_t AudioBytesToFrames(uint32_t frameBits, uint32_t size);
int32_t AudioDataBigEndianChange(char *srcData, uint32_t audioLen, enum DataBitWidth bitWidth);
int32_t AudioFramatToBitWidth(enum AudioFormat format, unsigned int *bitWidth);
int32_t AudioSetPcmInfo(struct PlatformData *platformData, const struct AudioPcmHwParams *param);
int32_t AudioSetRenderBufInfo(struct PlatformData *data, const struct AudioPcmHwParams *param);
int32_t AudioSetCaptureBufInfo(struct PlatformData *data, const struct AudioPcmHwParams *param);
int32_t AudioPcmWrite(const struct AudioCard *card, struct AudioTxData *txData);
int32_t AudioPcmRead(const struct AudioCard *card, struct AudioRxData *rxData);
int32_t AudioPcmMmapWrite(const struct AudioCard *card, const struct AudioMmapData *txMmapData);
int32_t AudioPcmMmapRead(const struct AudioCard *card, const struct AudioMmapData *rxMmapData);
int32_t AudioRenderOpen(const struct AudioCard *card);
int32_t AudioCaptureOpen(const struct AudioCard *card);
int32_t AudioRenderClose(const struct AudioCard *card);
int32_t AudioPcmPointer(const struct AudioCard *card, uint32_t *pointer);
int32_t AudioCaptureClose(const struct AudioCard *card);
int32_t AudioHwParams(const struct AudioCard *card, const struct AudioPcmHwParams *param);
int32_t AudioRenderPrepare(const struct AudioCard *card);
int32_t AudioCapturePrepare(const struct AudioCard *card);
int32_t AudioRenderTrigger(struct AudioCard *card, int cmd);
int32_t AudioCaptureTrigger(struct AudioCard *card, int cmd);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* CODEC_CORE_H */
