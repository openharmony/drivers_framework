/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "audio_platform_base.h"
#include "audio_core.h"

#define HDF_LOG_TAG audio_platform_base
struct PlatformData *PlatformDataFromDevice(const struct AudioCard *card)
{
    if (card == NULL || card->rtd == NULL || card->rtd->platform == NULL) {
        AUDIO_DRIVER_LOG_ERR("param is null.");
        return NULL;
    }
    return card->rtd->platform->devData;
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
    AUDIO_DRIVER_LOG_DEBUG("audioLen = %d\n", audioLen);
    return HDF_SUCCESS;
}
