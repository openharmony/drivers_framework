/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef SPI_DISPATCH_SAMPLE_H
#define SPI_DISPATCH_SAMPLE_H

#include "spi_pl022_sample.h"
#include "spi_core.h"

enum {
    SPI_TRANSFER = 1
};

int32_t SampleSpiDispatch(struct HdfDeviceIoClient *client, int cmdId, struct HdfSBuf *data, struct HdfSBuf *reply);

#endif // SPI_DISPATCH_SAMPLE_H
