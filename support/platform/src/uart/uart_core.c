/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "uart_core.h"
#include "hdf_log.h"
#include "osal_mem.h"
#include "uart_if.h"

#define HDF_LOG_TAG uart_core_c

int32_t UartHostInit(struct UartHost *host)
{
    int32_t ret;

    if (host == NULL || host->method == NULL) {
        HDF_LOGE("%s: host or method is NULL", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    if (OsalAtomicRead(&host->atom) == 1) {
        HDF_LOGE("%s: device is busy", __func__);
        return HDF_ERR_DEVICE_BUSY;
    }
    OsalAtomicInc(&host->atom);
    if (host->method->Init != NULL) {
        ret = host->method->Init(host);
        if (ret != HDF_SUCCESS) {
            OsalAtomicDec(&host->atom);
            HDF_LOGE("%s: host init failed", __func__);
            return ret;
        }
    }
    return HDF_SUCCESS;
}

int32_t UartHostDeinit(struct UartHost *host)
{
    int32_t ret;
    if (host == NULL || host->method == NULL) {
        HDF_LOGE("%s: host or method is NULL", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    if (host->method->Deinit != NULL) {
        ret = host->method->Deinit(host);
        if (ret != HDF_SUCCESS) {
            HDF_LOGE("%s: host deinit failed", __func__);
            return ret;
        }
    }
    OsalAtomicDec(&host->atom);
    return HDF_SUCCESS;
}

void UartHostDestroy(struct UartHost *host)
{
    if (host == NULL) {
        return;
    }
    OsalMemFree(host);
}

struct UartHost *UartHostCreate(struct HdfDeviceObject *device)
{
    struct UartHost *host = NULL;

    if (device == NULL) {
        HDF_LOGE("%s: invalid parameter", __func__);
        return NULL;
    }
    host = (struct UartHost *)OsalMemCalloc(sizeof(*host));
    if (host == NULL) {
        HDF_LOGE("%s: OsalMemCalloc error", __func__);
        return NULL;
    }
    host->device = device;
    device->service = &(host->service);
    host->device->service->Dispatch = UartIoDispatch;
    OsalAtomicSet(&host->atom, 0);
    host->priv = NULL;
    host->method = NULL;
    return host;
}
