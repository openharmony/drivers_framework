/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef SPI_SAMPLE_H
#define SPI_SAMPLE_H

#include "spi_core.h"

int32_t SampleSpiCntlrTransfer(struct SpiCntlr *cntlr, struct SpiMsg *msg, uint32_t count);
int32_t SampleSpiCntlrSetCfg(struct SpiCntlr *cntlr, struct SpiCfg *cfg);
int32_t SampleSpiCntlrGetCfg(struct SpiCntlr *cntlr, struct SpiCfg *cfg);

#endif // SPI_SAMPLE_H
