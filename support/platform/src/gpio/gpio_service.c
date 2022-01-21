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
#include "platform_core.h"

#define HDF_LOG_TAG gpio_service

static int32_t GpioServiceIoRead(struct HdfSBuf *data, struct HdfSBuf *reply)
{
    int32_t ret;
    uint16_t gpio;
    uint16_t value;

    if (data == NULL || reply == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }

    if (!HdfSbufReadUint16(data, &gpio)) {
        PLAT_LOGE("%s: read gpio number failed", __func__);
        return HDF_ERR_IO;
    }

    ret = GpioRead(gpio, &value);
    if (ret != HDF_SUCCESS) {
        PLAT_LOGE("%s: read gpio failed:%d", __func__, ret);
        return ret;
    }

    if (!HdfSbufWriteUint16(reply, value)) {
        PLAT_LOGE("%s: write subf failed:%d", __func__, ret);
        return ret;
    }

    return ret;
}

static int32_t GpioServiceIoWrite(struct HdfSBuf *data, struct HdfSBuf *reply)
{
    int32_t ret;
    uint16_t gpio;
    uint16_t value;

    if (data == NULL || reply == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }

    if (!HdfSbufReadUint16(data, &gpio)) {
        PLAT_LOGE("%s: read gpio number failed", __func__);
        return HDF_ERR_IO;
    }

    if (!HdfSbufReadUint16(data, &value)) {
        PLAT_LOGE("%s: read gpio value failed", __func__);
        return HDF_ERR_IO;
    }

    ret = GpioWrite(gpio, value);
    if (ret != HDF_SUCCESS) {
        PLAT_LOGE("%s: write gpio failed:%d", __func__, ret);
        return ret;
    }

    return ret;
}

static int32_t GpioServiceIoGetDir(struct HdfSBuf *data, struct HdfSBuf *reply)
{
    int32_t ret;
    uint16_t gpio;
    uint16_t dir;

    if (data == NULL || reply == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }

    if (!HdfSbufReadUint16(data, &gpio)) {
        PLAT_LOGE("%s: read gpio number failed", __func__);
        return HDF_ERR_IO;
    }

    ret = GpioGetDir(gpio, &dir);
    if (ret != HDF_SUCCESS) {
        PLAT_LOGE("%s: get gpio dir failed:%d", __func__, ret);
        return ret;
    }

    if (!HdfSbufWriteUint16(reply, dir)) {
        PLAT_LOGE("%s: write subf failed:%d", __func__, ret);
        return ret;
    }

    return ret;
}

static int32_t GpioServiceIoSetDir(struct HdfSBuf *data, struct HdfSBuf *reply)
{
    int32_t ret;
    uint16_t gpio;
    uint16_t dir;

    if (data == NULL || reply == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }

    if (!HdfSbufReadUint16(data, &gpio)) {
        PLAT_LOGE("%s: read gpio number failed", __func__);
        return HDF_ERR_IO;
    }

    if (!HdfSbufReadUint16(data, &dir)) {
        PLAT_LOGE("%s: read gpio dir failed", __func__);
        return HDF_ERR_IO;
    }

    ret = GpioSetDir(gpio, dir);
    if (ret != HDF_SUCCESS) {
        PLAT_LOGE("%s: set gpio dir failed:%d", __func__, ret);
        return ret;
    }

    return ret;
}

static int32_t GpioServiceDispatch(struct HdfDeviceIoClient *client, int cmd,
    struct HdfSBuf *data, struct HdfSBuf *reply)
{
    int32_t ret;

    switch (cmd) {
        case GPIO_IO_READ:
            return GpioServiceIoRead(data, reply);
        case GPIO_IO_WRITE:
            return GpioServiceIoWrite(data, reply);
        case GPIO_IO_GETDIR:
            return GpioServiceIoGetDir(data, reply);
        case GPIO_IO_SETDIR:
            return GpioServiceIoSetDir(data, reply);
        default:
            ret = HDF_ERR_NOT_SUPPORT;
            break;
    }
    return ret;
}

static int32_t GpioServiceBind(struct HdfDeviceObject *device)
{   
    int32_t ret;
    struct PlatformManager *gpioMgr = NULL;

    PLAT_LOGI("GpioServiceBind: enter");
    if (device == NULL) {
        PLAT_LOGE("GpioServiceBind: device is NULL");
        return HDF_ERR_INVALID_OBJECT;
    }

    gpioMgr = GpioManagerGet();
    if (gpioMgr == NULL) {
        PLAT_LOGE("GpioServiceBind: get gpio manager failed");
        return HDF_PLT_ERR_DEV_GET;
    }

    ret = PlatformDeviceCreateService(&gpioMgr->device, GpioServiceDispatch);
    if (ret != HDF_SUCCESS) {
        PLAT_LOGE("GpioServiceBind: create gpio service failed:%d", ret);
        return ret;
    }

    ret = PlatformDeviceBind(&gpioMgr->device, device);
    if (ret != HDF_SUCCESS) {
        PLAT_LOGE("GpioServiceBind: bind gpio device failed:%d", ret);
        (void)PlatformDeviceDestroyService(&gpioMgr->device);
        return ret;
    }

    PLAT_LOGI("GpioServiceBind: success");
    return HDF_SUCCESS;
}

static int32_t GpioServiceInit(struct HdfDeviceObject *device)
{
    (void)device;
    return HDF_SUCCESS;
}

static void GpioServiceRelease(struct HdfDeviceObject *device)
{
    struct PlatformManager *gpioMgr = NULL;

    PLAT_LOGI("GpioServiceRelease: enter");
    if (device == NULL) {
        PLAT_LOGI("GpioServiceRelease: device is null");
        return;
    }

    gpioMgr = GpioManagerGet();
    if (gpioMgr == NULL) {
        PLAT_LOGE("GpioServiceBind: get gpio manager failed");
        return;
    }

    (void)PlatformDeviceUnbind(&gpioMgr->device, device);
    (void)PlatformDeviceDestroyService(&gpioMgr->device);
    PLAT_LOGI("GpioServiceRelease: done");
}

struct HdfDriverEntry g_gpioServiceEntry = {
    .moduleVersion = 1,
    .Bind = GpioServiceBind,
    .Init = GpioServiceInit,
    .Release = GpioServiceRelease,
    .moduleName = "HDF_PLATFORM_GPIO_MANAGER",
};
HDF_INIT(g_gpioServiceEntry);
