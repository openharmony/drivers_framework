/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef SPI_IF_H
#define SPI_IF_H

#include "hdf_platform.h"

#define SPI_DEV_SERVICE_NAME_PREFIX "HDF_PLATFORM_SPI_%u"
#define MAX_DEV_NAME_SIZE 32

struct SpiDevInfo {
    uint32_t busNum;
    uint32_t csNum;
};

struct SpiMsg {
    uint8_t *wbuf;
    uint8_t *rbuf;
    uint32_t len;
    uint32_t speed;
    uint16_t delayUs;
    uint8_t csChange;
};

enum {
    SPI_TRANSFER = 1
};

DevHandle SpiOpen(const struct SpiDevInfo *info);
void SpiClose(DevHandle handle);
int32_t SpiTransfer(DevHandle handle, struct SpiMsg *msgs);

#endif // SPI_IF_H
