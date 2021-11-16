/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "spi_dispatch_sample.h"
#include "spi_sample.h"
#include "hdf_log.h"
#include "hdf_sbuf.h"

#define HDF_LOG_TAG spi_dispatch_sample

static int32_t SampleSpiTransfer(struct SpiCntlr *cntlr, struct HdfSBuf *txBuf)
{
    HDF_LOGD("%s: Enter", __func__);
    uint32_t readSize = sizeof(struct SpiMsg);
    struct SpiMsg *msg = NULL;

    if (cntlr == NULL || cntlr->priv == NULL || txBuf == NULL) {
        HDF_LOGE("%s: invalid parameter", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    if (!HdfSbufReadBuffer(txBuf, (const void **)&msg, &readSize)) {
        HDF_LOGE("%s: Failed to read sbuf", __func__);
        return HDF_DEV_ERR_NO_MEMORY;
    }

    if (SampleSpiCntlrTransfer(cntlr, msg, msg->len) != HDF_SUCCESS) {
        HDF_LOGE("%s: SampleSpiCntlrTransfer error", __func__);
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t SampleSpiDispatch(struct HdfDeviceIoClient *client, int cmdId, struct HdfSBuf *data, struct HdfSBuf *reply)
{
    if (client == NULL || client->device == NULL) {
        HDF_LOGE("%s: client or client->device is NULL", __func__);
        return HDF_FAILURE;
    }

    struct SpiCntlr *cntlr = (struct SpiCntlr *)client->device->service;
    if (cntlr == NULL || cntlr->method == NULL) {
        HDF_LOGE("%s: cntlr or cntlr->method is NULL", __func__);
        return HDF_FAILURE;
    }

    switch (cmdId) {
        case SPI_TRANSFER:
            return SampleSpiTransfer(cntlr, data);
        default:
            HDF_LOGE("%s: invalid cmdId %d", __func__, cmdId);
            return HDF_FAILURE;
    }
}