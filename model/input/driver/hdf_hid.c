/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include <securec.h>
#include "hdf_device_desc.h"
#include "hdf_log.h"

static int32_t HdfHIDDriverInit(struct HdfDeviceObject *device)
{
    (void)device;
    return HDF_SUCCESS;
}

static int32_t HdfHIDDispatch(struct HdfDeviceIoClient *client, int cmd, struct HdfSBuf *data, struct HdfSBuf *reply)
{
    (void)cmd;
    if (client == NULL || data == NULL || reply == NULL) {
        HDF_LOGE("%s: param is null", __func__);
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

static int32_t HdfHIDDriverBind(struct HdfDeviceObject *device)
{
    if (device == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }
    static struct IDeviceIoService hidService = {
        .Dispatch = HdfHIDDispatch,
    };
    device->service = &hidService;
    return HDF_SUCCESS;
}

struct HdfDriverEntry g_hdfHIDEntry = {
    .moduleVersion = 1,
    .moduleName = "HDF_HID",
    .Bind = HdfHIDDriverBind,
    .Init = HdfHIDDriverInit,
};

HDF_INIT(g_hdfHIDEntry);