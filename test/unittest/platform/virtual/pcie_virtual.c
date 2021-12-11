/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "device_resource_if.h"
#include "hdf_log.h"
#include "osal_mem.h"
#include "pcie_core.h"
#include "pcie_if.h"

#define HDF_LOG_TAG pcie_virtual_c

#define PCIE_ADAPTER_ONE_BYTE 1
#define PCIE_ADAPTER_TWO_BYTE 2
#define PCIE_ADAPTER_FOUR_BYTE 4
#define PCIE_READ_DATA_1 0x95
#define PCIE_READ_DATA_2 0x27
#define PCIE_READ_DATA_3 0x89

struct PcieAdapterHost {
    struct PcieCntlr cntlr;
};

static int32_t pcieAdapterInit(struct PcieCntlr *cntlr, uint16_t busNum)
{
    if (cntlr == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    if (busNum != cntlr->devInfo.busNum) {
        HDF_LOGE("pci bus num not match.");
        return HDF_ERR_NOT_SUPPORT;
    }

    HDF_LOGE("pcieAdapterInit:success! vendorId is %d, devId is %d", cntlr->devInfo.vendorId, cntlr->devInfo.devId);
    return HDF_SUCCESS;
}

static int32_t pcieAdapterRead(struct PcieCntlr *cntlr, uint32_t pos, uint8_t *data, uint32_t len)
{
    if (cntlr == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }

    HDF_LOGE("pcieAdapterRead: pos = 0x%x, data = 0x%x, data val = %u, len = %d", pos, (uint32_t)data,
        *data, len);
    if (len == 1) {
        *data = PCIE_READ_DATA_1;
    } else if (len == 2) {
        *data = PCIE_READ_DATA_1;
        data++;
        *data = PCIE_READ_DATA_2;
    } else {
        *data = PCIE_READ_DATA_1;
        data++;
        *data = PCIE_READ_DATA_2;
        data++;
        *data = PCIE_READ_DATA_2;
        data++;
        *data = PCIE_READ_DATA_3;
    }
    return HDF_SUCCESS;
}

static int32_t pcieAdapterWrite(struct PcieCntlr *cntlr, uint32_t pos, uint8_t *data, uint32_t len)
{
    if (cntlr == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }

    HDF_LOGE("pcieAdapterWrite: pos = 0x%x, data = 0x%x, data val = %u, len = %d", pos, (uint32_t)data,
        *data, len);
    return HDF_SUCCESS;
}

static int32_t pcieAdapterDeinit(struct PcieCntlr *cntlr)
{
    if (cntlr == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    cntlr->priv = NULL;
    return HDF_SUCCESS;
}

static struct PcieCntlrOps g_pcieAdapterHostOps = {
    .init = pcieAdapterInit,
    .read = pcieAdapterRead,
    .write = pcieAdapterWrite,
    .deinit = pcieAdapterDeinit,
};

static int32_t PcieAdapterBind(struct HdfDeviceObject *obj)
{
    struct PcieAdapterHost *host = NULL;
    int32_t ret;

    if (obj == NULL) {
        HDF_LOGE("PcieAdapterBind: Fail, device is NULL.");
        return HDF_ERR_INVALID_OBJECT;
    }

    host = (struct PcieAdapterHost *)OsalMemCalloc(sizeof(struct PcieAdapterHost));
    if (host == NULL) {
        HDF_LOGE("PcieAdapterBind: no mem for PcieAdapterHost.");
        return HDF_ERR_MALLOC_FAIL;
    }
    host->cntlr.ops = &g_pcieAdapterHostOps;
    host->cntlr.hdfDevObj = obj;
    obj->service = &(host->cntlr.service);

    ret = PcieCntlrParse(&(host->cntlr), obj);
    if (ret != HDF_SUCCESS) {
        goto _ERR;
    }

    ret = PcieCntlrAdd(&(host->cntlr));
    if (ret != HDF_SUCCESS) {
        goto _ERR;
    }

    HDF_LOGE("PcieAdapterBind: success.");
    return HDF_SUCCESS;
_ERR:
    PcieCntlrRemove(&(host->cntlr));
    OsalMemFree(host);
    HDF_LOGD("PcieAdapterBind: fail, err = %d.", ret);
    return ret;
}

static int32_t PcieAdapterInit(struct HdfDeviceObject *obj)
{
    (void)obj;

    HDF_LOGE("PcieAdapterInit: success.");
    return HDF_SUCCESS;
}

static void PcieAdapterRelease(struct HdfDeviceObject *obj)
{
    struct PcieCntlr *cntlr = NULL;
    struct PcieAdapterHost *host = NULL;

    if (obj == NULL) {
        return;
    }

    cntlr = (struct PcieCntlr *)obj->service;
    if (cntlr == NULL) {
        return;
    }
    PcieCntlrRemove(cntlr);
    host = (struct PcieAdapterHost *)cntlr;
    OsalMemFree(host);
    HDF_LOGD("PcieAdapterRelease: success.");
}

struct HdfDriverEntry g_pcieVirtualDriverEntry = {
    .moduleVersion = 1,
    .Bind = PcieAdapterBind,
    .Init = PcieAdapterInit,
    .Release = PcieAdapterRelease,
    .moduleName = "PLATFORM_PCIE_VIRTUAL",
};
HDF_INIT(g_pcieVirtualDriverEntry);
