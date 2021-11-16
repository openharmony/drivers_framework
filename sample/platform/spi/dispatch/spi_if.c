/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "spi_if.h"
#include "osal_mem.h"
#include "hdf_io_service_if.h"
#include "hdf_log.h"
#include "securec.h"

#define HDF_LOG_TAG spi_if

DevHandle SpiOpen(const struct SpiDevInfo *info)
{
    int32_t ret;
    void *devHandle = NULL;
    uint32_t port;
    char *serviceName = NULL;

    if (info == NULL) {
        HDF_LOGW("info is NULL");
        return NULL;
    }
    port = info->busNum;

    serviceName = (char *)OsalMemCalloc(MAX_DEV_NAME_SIZE + 1);
    if (serviceName == NULL) {
        HDF_LOGE("Failed to OsalMemCalloc serviceName");
        return NULL;
    }
    ret = snprintf_s(serviceName, MAX_DEV_NAME_SIZE + 1, MAX_DEV_NAME_SIZE, SPI_DEV_SERVICE_NAME_PREFIX, port);
    if (ret < 0) {
        HDF_LOGE("Failed to snprintf_s");
        OsalMemFree(serviceName);
        return NULL;
    }
    devHandle = (void *)HdfIoServiceBind(serviceName);
    OsalMemFree(serviceName);
    return devHandle;
}

void SpiClose(DevHandle handle)
{
    if (handle == NULL) {
        HDF_LOGW("handle is NULL");
        return;
    }
    struct HdfIoService *service = (struct HdfIoService *)handle;
    HdfIoServiceRecycle(service);
    OsalMemFree(handle);
}

int32_t SpiTransfer(DevHandle handle, struct SpiMsg *msgs)
{
    int32_t ret;
    if (handle == NULL || msgs == NULL) {
        HDF_LOGW("handle or msgs is NULL");
        return HDF_ERR_INVALID_PARAM;
    }
    struct HdfIoService *service = (struct HdfIoService *)handle;
    if (service->dispatcher == NULL || service->dispatcher->Dispatch == NULL) {
        HDF_LOGE("service->dispatcher or service->dispatcher->Dispatch is NULL");
        return HDF_FAILURE;
    }

    struct HdfSBuf *sBuf = HdfSBufObtainDefaultSize();
    if (sBuf == NULL) {
        HDF_LOGE("Failed to obtain sBuf");
        return HDF_FAILURE;
    }

    if (!HdfSbufWriteBuffer(sBuf, msgs, sizeof(struct SpiMsg))) {
        HDF_LOGE("Failed to write sbuf");
        HdfSBufRecycle(sBuf);
        return HDF_FAILURE;
    }
    ret = service->dispatcher->Dispatch(&service->object, SPI_TRANSFER, sBuf, NULL);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("Failed to send service call");
    }
    HdfSBufRecycle(sBuf);
    return ret;
}
