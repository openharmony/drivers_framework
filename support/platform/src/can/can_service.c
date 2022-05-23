/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "can/can_service.h"
#include "can/can_core.h"
#include "hdf_device_desc.h"
#include "osal_mem.h"
#include "platform_core.h"

#define HDF_LOG_TAG can_service

static int32_t CanServiceDispatch(
    struct HdfDeviceIoClient *client, int cmd, struct HdfSBuf *data, struct HdfSBuf *reply)
{
    struct CanCntlr *cntlr = NULL;
    (void)data;
    (void)reply;

    if (client == NULL || client->device == NULL) {
        HDF_LOGE("CanServiceDispatch: invalid client object!");
        return HDF_ERR_INVALID_OBJECT;
    }

    cntlr = CanCntlrFromHdfDev(client->device);
    if (cntlr == NULL) {
        HDF_LOGE("CanServiceDispatch: no controller binded!");
        return HDF_ERR_INVALID_OBJECT;
    }

    HDF_LOGD("CanServiceDispatch: cntlr number=%d, cmd = %d", cntlr->number, cmd);
    return HDF_SUCCESS;
}

int32_t CanServiceBind(struct HdfDeviceObject *device)
{
    struct IDeviceIoService *service = NULL;

    if (device == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }

    service = (struct IDeviceIoService *)OsalMemCalloc(sizeof(*service));
    if (service == NULL) {
        HDF_LOGE("CanServiceBind: alloc service failed!");
        return HDF_ERR_MALLOC_FAIL;
    }

    service->Dispatch = CanServiceDispatch;
    device->service = service;
    return HDF_SUCCESS;
}

void CanServiceRelease(struct HdfDeviceObject *device)
{
    if (device != NULL && device->service != NULL) {
        OsalMemFree(device->service);
    }
}
