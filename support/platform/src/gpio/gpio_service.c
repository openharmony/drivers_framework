/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "gpio_if.h"
#include "gpio/gpio_core.h"
#include "gpio/gpio_service.h"
#include "hdf_device_desc.h"
#include "hdf_log.h"
#include "platform_core.h"

#define HDF_LOG_TAG gpio_service

static int32_t GpioManagerIoRead(struct HdfSBuf *data, struct HdfSBuf *reply)
{
    int32_t ret;
    uint16_t gpio;
    uint16_t value;

    if (data == NULL || reply == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }

    if (!HdfSbufReadUint16(data, &gpio)) {
        HDF_LOGE("%s: read gpio number failed", __func__);
        return HDF_ERR_IO;
    }

    ret = GpioRead(gpio, &value);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: read gpio failed:%d", __func__, ret);
        return ret;
    }

    if (!HdfSbufWriteUint16(reply, value)) {
        HDF_LOGE("%s: write subf failed:%d", __func__, ret);
        return ret;
    }

    return ret;
}

static int32_t GpioManagerIoWrite(struct HdfSBuf *data, struct HdfSBuf *reply)
{
    int32_t ret;
    uint16_t gpio;
    uint16_t value;

    if (data == NULL || reply == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }

    if (!HdfSbufReadUint16(data, &gpio)) {
        HDF_LOGE("%s: read gpio number failed", __func__);
        return HDF_ERR_IO;
    }

    if (!HdfSbufReadUint16(data, &value)) {
        HDF_LOGE("%s: read gpio value failed", __func__);
        return HDF_ERR_IO;
    }

    ret = GpioWrite(gpio, value);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: write gpio failed:%d", __func__, ret);
        return ret;
    }

    return ret;
}

static int32_t GpioManagerIoGetDir(struct HdfSBuf *data, struct HdfSBuf *reply)
{
    int32_t ret;
    uint16_t gpio;
    uint16_t dir;

    if (data == NULL || reply == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }

    if (!HdfSbufReadUint16(data, &gpio)) {
        HDF_LOGE("%s: read gpio number failed", __func__);
        return HDF_ERR_IO;
    }

    ret = GpioGetDir(gpio, &dir);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: get gpio dir failed:%d", __func__, ret);
        return ret;
    }

    if (!HdfSbufWriteUint16(reply, dir)) {
        HDF_LOGE("%s: write subf failed:%d", __func__, ret);
        return ret;
    }

    return ret;
}

static int32_t GpioManagerIoSetDir(struct HdfSBuf *data, struct HdfSBuf *reply)
{
    int32_t ret;
    uint16_t gpio;
    uint16_t dir;

    if (data == NULL || reply == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }

    if (!HdfSbufReadUint16(data, &gpio)) {
        HDF_LOGE("%s: read gpio number failed", __func__);
        return HDF_ERR_IO;
    }

    if (!HdfSbufReadUint16(data, &dir)) {
        HDF_LOGE("%s: read gpio dir failed", __func__);
        return HDF_ERR_IO;
    }

    ret = GpioSetDir(gpio, dir);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: set gpio dir failed:%d", __func__, ret);
        return ret;
    }

    return ret;
}

static int32_t GpioManagerDispatch(struct HdfDeviceIoClient *client, int cmd,
    struct HdfSBuf *data, struct HdfSBuf *reply)
{
    int32_t ret;

    switch (cmd) {
        case GPIO_IO_READ:
            return GpioManagerIoRead(data, reply);
        case GPIO_IO_WRITE:
            return GpioManagerIoWrite(data, reply);
        case GPIO_IO_GETDIR:
            return GpioManagerIoGetDir(data, reply);
        case GPIO_IO_SETDIR:
            return GpioManagerIoSetDir(data, reply);
        default:
            ret = HDF_ERR_NOT_SUPPORT;
            break;
    }
    return ret;
}

static int32_t GpioManagerBind(struct HdfDeviceObject *device)
{   
    struct GpioManager *gpioMgr = NULL;

    HDF_LOGI("GpioManagerBind: enter");
    if (device == NULL) {
        HDF_LOGE("GpioManagerBind: device is NULL");
        return HDF_ERR_INVALID_OBJECT;
    }

    gpioMgr = GpioManagerGet();
    if (gpioMgr == NULL) {
        HDF_LOGE("GpioManagerBind: get gpio manager failed");
        return HDF_PLT_ERR_DEV_GET;
    }

    gpioMgr->device.hdfDev = device;
    device->service = &gpioMgr->service;
    device->service->Dispatch = GpioManagerDispatch;
    HDF_LOGI("GpioManagerBind: success");
    return HDF_SUCCESS;
}

static int32_t GpioManagerInit(struct HdfDeviceObject *device)
{
    (void)device;
    return HDF_SUCCESS;
}

static void GpioManagerRelease(struct HdfDeviceObject *device)
{
    struct GpioManager *gpioMgr = NULL;

    HDF_LOGI("GpioManagerRelease: enter");
    if (device == NULL) {
        HDF_LOGI("GpioManagerRelease: device is null");
        return;
    }

    gpioMgr = GpioManagerGet();
    if (gpioMgr == NULL) {
        HDF_LOGE("GpioManagerBind: get gpio manager failed");
        return;
    }

    gpioMgr->device.hdfDev = NULL;
    device->service = NULL;
    device->service->Dispatch = NULL;
    HDF_LOGI("GpioManagerRelease: done");
}

struct HdfDriverEntry g_gpioManagerEntry = {
    .moduleVersion = 1,
    .Bind = GpioManagerBind,
    .Init = GpioManagerInit,
    .Release = GpioManagerRelease,
    .moduleName = "HDF_PLATFORM_GPIO_MANAGER",
};
HDF_INIT(g_gpioManagerEntry);
