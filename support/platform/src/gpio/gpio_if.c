/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "gpio_if.h"

#ifdef __USER__
#include "gpio/gpio_service.h"
#include "hdf_io_service_if.h"
#include "platform_errno.h"
#else
#include "gpio/gpio_core.h"
#endif

#include "hdf_log.h"

#define HDF_LOG_TAG gpio_if

#ifdef __USER__

static void *GpioManagerServiceGet(void)
{
    static void *manager = NULL;

    if (manager != NULL) {
        return manager;
    }
    manager = (void *)HdfIoServiceBind("HDF_PLATFORM_GPIO_MANAGER");
    if (manager == NULL) {
        HDF_LOGE("%s: failed to get gpio manager service!", __func__);
    }
    return manager;
}

int32_t GpioRead(uint16_t gpio, uint16_t *val)
{
    int32_t ret;
    struct HdfIoService *service = NULL;
    struct HdfSBuf *data = NULL;
    struct HdfSBuf *reply = NULL;

    service = (struct HdfIoService *)GpioManagerServiceGet();
    if (service == NULL) {
        return HDF_PLT_ERR_DEV_GET;
    }

    data = HdfSBufObtainDefaultSize();
    if (data == NULL) {
        return HDF_ERR_MALLOC_FAIL;
    }

    reply = HdfSBufObtainDefaultSize();
    if (reply == NULL) {
        HdfSBufRecycle(data);
        return HDF_ERR_MALLOC_FAIL;
    }

    if (!HdfSbufWriteUint16(data, (uint16_t)gpio)) {
        HDF_LOGE("%s: write gpio number failed!", __func__);
        HdfSBufRecycle(data);
        HdfSBufRecycle(reply);
        return HDF_ERR_IO;
    }

    ret = service->dispatcher->Dispatch(&service->object, GPIO_IO_READ, data, reply);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: service call failed:%d", __func__, ret);
        HdfSBufRecycle(data);
        HdfSBufRecycle(reply);
        return ret;
    }

    if (!HdfSbufReadUint16(reply, val)) {
        HDF_LOGE("%s: read sbuf failed", __func__);
        HdfSBufRecycle(data);
        HdfSBufRecycle(reply);
        return HDF_ERR_IO;
    }

    HdfSBufRecycle(data);
    HdfSBufRecycle(reply);
    return HDF_SUCCESS;
}

int32_t GpioWrite(uint16_t gpio, uint16_t val)
{
    int32_t ret;
    struct HdfIoService *service = NULL;
    struct HdfSBuf *data = NULL;

    service = (struct HdfIoService *)GpioManagerServiceGet();
    if (service == NULL) {
        return HDF_PLT_ERR_DEV_GET;
    }

    data = HdfSBufObtainDefaultSize();
    if (data == NULL) {
        return HDF_ERR_MALLOC_FAIL;
    }

    if (!HdfSbufWriteUint16(data, (uint16_t)gpio)) {
        HDF_LOGE("%s: write gpio number failed!", __func__);
        HdfSBufRecycle(data);
        return HDF_ERR_IO;
    }

    if (!HdfSbufWriteUint16(data, (uint16_t)val)) {
        HDF_LOGE("%s: write gpio value failed!", __func__);
        HdfSBufRecycle(data);
        return HDF_ERR_IO;
    }

    ret = service->dispatcher->Dispatch(&service->object, GPIO_IO_WRITE, data, NULL);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: service call failed:%d", __func__, ret);
        HdfSBufRecycle(data);
        return ret;
    }

    HdfSBufRecycle(data);
    return HDF_SUCCESS;
}

int32_t GpioGetDir(uint16_t gpio, uint16_t *dir)
{
    int32_t ret;
    struct HdfIoService *service = NULL;
    struct HdfSBuf *data = NULL;
    struct HdfSBuf *reply = NULL;

    service = (struct HdfIoService *)GpioManagerServiceGet();
    if (service == NULL) {
        return HDF_PLT_ERR_DEV_GET;
    }

    data = HdfSBufObtainDefaultSize();
    if (data == NULL) {
        return HDF_ERR_MALLOC_FAIL;
    }

    reply = HdfSBufObtainDefaultSize();
    if (reply == NULL) {
        HdfSBufRecycle(data);
        return HDF_ERR_MALLOC_FAIL;
    }

    if (!HdfSbufWriteUint16(data, (uint16_t)gpio)) {
        HDF_LOGE("%s: write gpio number failed!", __func__);
        HdfSBufRecycle(data);
        HdfSBufRecycle(reply);
        return HDF_ERR_IO;
    }

    ret = service->dispatcher->Dispatch(&service->object, GPIO_IO_GETDIR, data, reply);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: service call failed:%d", __func__, ret);
        HdfSBufRecycle(data);
        HdfSBufRecycle(reply);
        return ret;
    }

    if (!HdfSbufReadUint16(reply, dir)) {
        HDF_LOGE("%s: read sbuf failed", __func__);
        HdfSBufRecycle(data);
        HdfSBufRecycle(reply);
        return HDF_ERR_IO;
    }

    HdfSBufRecycle(data);
    HdfSBufRecycle(reply);
    return HDF_SUCCESS;
}

int32_t GpioSetDir(uint16_t gpio, uint16_t dir)
{
    int32_t ret;
    struct HdfIoService *service = NULL;
    struct HdfSBuf *data = NULL;

    service = (struct HdfIoService *)GpioManagerServiceGet();
    if (service == NULL) {
        return HDF_PLT_ERR_DEV_GET;
    }

    data = HdfSBufObtainDefaultSize();
    if (data == NULL) {
        return HDF_ERR_MALLOC_FAIL;
    }

    if (!HdfSbufWriteUint16(data, (uint16_t)gpio)) {
        HDF_LOGE("%s: write gpio number failed!", __func__);
        HdfSBufRecycle(data);
        return HDF_ERR_IO;
    }

    if (!HdfSbufWriteUint16(data, (uint16_t)dir)) {
        HDF_LOGE("%s: write gpio value failed!", __func__);
        HdfSBufRecycle(data);
        return HDF_ERR_IO;
    }

    ret = service->dispatcher->Dispatch(&service->object, GPIO_IO_SETDIR, data, NULL);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: service call failed:%d", __func__, ret);
        HdfSBufRecycle(data);
        return ret;
    }

    HdfSBufRecycle(data);
    return HDF_SUCCESS;
}

int32_t GpioSetIrq(uint16_t gpio, uint16_t mode, GpioIrqFunc func, void *arg)
{
    (void)gpio;
    (void)mode;
    (void)func;
    (void)arg;
    return HDF_SUCCESS;
}

int32_t GpioUnSetIrq(uint16_t gpio)
{
    (void)gpio;
    return HDF_SUCCESS;
}

int32_t GpioEnableIrq(uint16_t gpio)
{
    (void)gpio;
    return HDF_SUCCESS;
}

int32_t GpioDisableIrq(uint16_t gpio)
{
    (void)gpio;
    return HDF_SUCCESS;
}

#else
int32_t GpioRead(uint16_t gpio, uint16_t *val)
{
    return GpioCntlrRead(GpioGetCntlr(gpio), GpioToLocal(gpio), val);
}

int32_t GpioWrite(uint16_t gpio, uint16_t val)
{
    return GpioCntlrWrite(GpioGetCntlr(gpio), GpioToLocal(gpio), val);
}

int32_t GpioSetDir(uint16_t gpio, uint16_t dir)
{
    return GpioCntlrSetDir(GpioGetCntlr(gpio), GpioToLocal(gpio), dir);
}

int32_t GpioGetDir(uint16_t gpio, uint16_t *dir)
{
    return GpioCntlrGetDir(GpioGetCntlr(gpio), GpioToLocal(gpio), dir);
}

int32_t GpioSetIrq(uint16_t gpio, uint16_t mode, GpioIrqFunc func, void *arg)
{
    return GpioCntlrSetIrq(GpioGetCntlr(gpio), GpioToLocal(gpio), mode, func, arg);
}

int32_t GpioUnSetIrq(uint16_t gpio)
{
    return GpioCntlrUnsetIrq(GpioGetCntlr(gpio), GpioToLocal(gpio));
}

int32_t GpioEnableIrq(uint16_t gpio)
{
    return GpioCntlrEnableIrq(GpioGetCntlr(gpio), GpioToLocal(gpio));
}

int32_t GpioDisableIrq(uint16_t gpio)
{
    return GpioCntlrDisableIrq(GpioGetCntlr(gpio), GpioToLocal(gpio));
}
#endif
