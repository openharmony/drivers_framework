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
#include "platform_core.h"
#else
#include "devsvc_manager_clnt.h"
#include "gpio/gpio_core.h"
#endif

#include "hdf_base.h"

#define PLAT_LOG_TAG gpio_if

#ifdef __USER__

static void *GpioManagerServiceGet(void)
{
    static void *manager = NULL;

    if (manager != NULL) {
        return manager;
    }
    manager = (void *)HdfIoServiceBind("HDF_PLATFORM_GPIO_MANAGER");
    if (manager == NULL) {
        PLAT_LOGE("%s: failed to get gpio manager service!", __func__);
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
        PLAT_LOGE("%s: write gpio number failed!", __func__);
        HdfSBufRecycle(data);
        HdfSBufRecycle(reply);
        return HDF_ERR_IO;
    }

    ret = service->dispatcher->Dispatch(&service->object, GPIO_IO_READ, data, reply);
    if (ret != HDF_SUCCESS) {
        PLAT_LOGE("%s: service call failed:%d", __func__, ret);
        HdfSBufRecycle(data);
        HdfSBufRecycle(reply);
        return ret;
    }

    if (!HdfSbufReadUint16(reply, val)) {
        PLAT_LOGE("%s: read sbuf failed", __func__);
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
        PLAT_LOGE("%s: write gpio number failed!", __func__);
        HdfSBufRecycle(data);
        return HDF_ERR_IO;
    }

    if (!HdfSbufWriteUint16(data, (uint16_t)val)) {
        PLAT_LOGE("%s: write gpio value failed!", __func__);
        HdfSBufRecycle(data);
        return HDF_ERR_IO;
    }

    ret = service->dispatcher->Dispatch(&service->object, GPIO_IO_WRITE, data, NULL);
    if (ret != HDF_SUCCESS) {
        PLAT_LOGE("%s: service call failed:%d", __func__, ret);
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
        PLAT_LOGE("%s: write gpio number failed!", __func__);
        HdfSBufRecycle(data);
        HdfSBufRecycle(reply);
        return HDF_ERR_IO;
    }

    ret = service->dispatcher->Dispatch(&service->object, GPIO_IO_GETDIR, data, reply);
    if (ret != HDF_SUCCESS) {
        PLAT_LOGE("%s: service call failed:%d", __func__, ret);
        HdfSBufRecycle(data);
        HdfSBufRecycle(reply);
        return ret;
    }

    if (!HdfSbufReadUint16(reply, dir)) {
        PLAT_LOGE("%s: read sbuf failed", __func__);
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
        PLAT_LOGE("%s: write gpio number failed!", __func__);
        HdfSBufRecycle(data);
        return HDF_ERR_IO;
    }

    if (!HdfSbufWriteUint16(data, (uint16_t)dir)) {
        PLAT_LOGE("%s: write gpio value failed!", __func__);
        HdfSBufRecycle(data);
        return HDF_ERR_IO;
    }

    ret = service->dispatcher->Dispatch(&service->object, GPIO_IO_SETDIR, data, NULL);
    if (ret != HDF_SUCCESS) {
        PLAT_LOGE("%s: service call failed:%d", __func__, ret);
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

int32_t GpioUnsetIrq(uint16_t gpio, void *arg)
{
    (void)gpio;
    (void)arg;
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
    int32_t ret;
    struct GpioCntlr *cntlr = GpioCntlrGet(gpio);

    ret = GpioCntlrRead(cntlr, GpioCntlrGetLocal(cntlr, gpio), val);

    GpioCntlrPut(cntlr);
    return ret;
}

int32_t GpioWrite(uint16_t gpio, uint16_t val)
{
    int32_t ret;
    struct GpioCntlr *cntlr = GpioCntlrGet(gpio);

    ret = GpioCntlrWrite(cntlr, GpioCntlrGetLocal(cntlr, gpio), val);

    GpioCntlrPut(cntlr);
    return ret;
}

int32_t GpioSetDir(uint16_t gpio, uint16_t dir)
{
    int32_t ret;
    struct GpioCntlr *cntlr = GpioCntlrGet(gpio);

    ret = GpioCntlrSetDir(cntlr, GpioCntlrGetLocal(cntlr, gpio), dir);

    GpioCntlrPut(cntlr);
    return ret;
}

int32_t GpioGetDir(uint16_t gpio, uint16_t *dir)
{
    int32_t ret;
    struct GpioCntlr *cntlr = GpioCntlrGet(gpio);

    ret = GpioCntlrGetDir(cntlr, GpioCntlrGetLocal(cntlr, gpio), dir);

    GpioCntlrPut(cntlr);
    return ret;
}

int32_t GpioSetIrq(uint16_t gpio, uint16_t mode, GpioIrqFunc func, void *arg)
{
    int32_t ret;
    struct GpioCntlr *cntlr = GpioCntlrGet(gpio);

    ret = GpioCntlrSetIrq(cntlr, GpioCntlrGetLocal(cntlr, gpio), mode, func, arg);

    GpioCntlrPut(cntlr);
    return ret;
}

int32_t GpioUnsetIrq(uint16_t gpio, void *arg)
{
    int32_t ret;
    struct GpioCntlr *cntlr = GpioCntlrGet(gpio);

    ret = GpioCntlrUnsetIrq(cntlr, GpioCntlrGetLocal(cntlr, gpio), arg);

    GpioCntlrPut(cntlr);
    return ret;
}

int32_t GpioEnableIrq(uint16_t gpio)
{
    int32_t ret;
    struct GpioCntlr *cntlr = GpioCntlrGet(gpio);

    ret = GpioCntlrEnableIrq(cntlr, GpioCntlrGetLocal(cntlr, gpio));

    GpioCntlrPut(cntlr);
    return ret;
}

int32_t GpioDisableIrq(uint16_t gpio)
{
    int32_t ret;
    struct GpioCntlr *cntlr = GpioCntlrGet(gpio);

    ret = GpioCntlrDisableIrq(cntlr, GpioCntlrGetLocal(cntlr, gpio));

    GpioCntlrPut(cntlr);
    return ret;
}
#endif
