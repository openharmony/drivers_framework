/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include <securec.h>
#include "hdf_base.h"
#include "hdf_log.h"
#include "hdf_disp.h"

#define OFFSET_TWO_BYTE    16

struct DispOperations *g_dispOps = NULL;

int32_t DispRegister(struct DispOperations *ops)
{
    if (g_dispOps == NULL) {
        g_dispOps = ops;
        HDF_LOGI("%s: disp ops register success", __func__);
        return HDF_SUCCESS;
    }
    HDF_LOGD("%s: disp ops registered", __func__);
    return HDF_FAILURE;
}

struct DispOperations *GetDispOps(void)
{
    return g_dispOps;
}

static int32_t DispOn(uint32_t devId)
{
    int32_t ret;
    struct DispOperations *ops = NULL;

    ops = GetDispOps();
    if (ops == NULL) {
        HDF_LOGE("%s: ops is null", __func__);
        return HDF_FAILURE;
    }
    if (ops->on != NULL) {
        ret = ops->on(devId);
        if (ret != HDF_SUCCESS) {
            HDF_LOGE("%s: ops on failed", __func__);
            return HDF_FAILURE;
        }
    }
    return HDF_SUCCESS;
}

static int32_t DispOff(uint32_t devId)
{
    int32_t ret;
    struct DispOperations *ops = NULL;

    ops = GetDispOps();
    if (ops == NULL) {
        HDF_LOGE("%s: ops is null", __func__);
        return HDF_FAILURE;
    }
    if (ops->off != NULL) {
        ret = ops->off(devId);
        if (ret != HDF_SUCCESS) {
            HDF_LOGE("%s: ops off failed", __func__);
            return HDF_FAILURE;
        }
    }
    return HDF_SUCCESS;
}

static int32_t DispInit(struct HdfDeviceObject *device, struct HdfSBuf *reqData, struct HdfSBuf *rspData)
{
    (void)device;
    (void)rspData;
    int32_t ret;
    uint32_t devId = 0;

    if (reqData == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }
    if (!HdfSbufReadUint32(reqData, &devId)) {
        HDF_LOGE("%s: HdfSbufReadBuffer failed", __func__);
        return HDF_FAILURE;
    }

    struct DispOperations *ops = NULL;
    ops = GetDispOps();
    if (ops == NULL) {
        HDF_LOGE("%s: ops is null", __func__);
        return HDF_FAILURE;
    }
    if (ops->init != NULL) {
        ret = ops->init(devId);
        if (ret) {
            HDF_LOGE("%s: init failed", __func__);
            return HDF_FAILURE;
        }
    }
    return HDF_SUCCESS;
}

static int32_t GetDispInfo(struct HdfDeviceObject *device, struct HdfSBuf *reqData, struct HdfSBuf *rspData)
{
    (void)device;
    (void)rspData;
    uint32_t devId = 0;
    struct DispInfo info;
    struct DispOperations *ops = NULL;

    if (reqData == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }
    if (!HdfSbufReadUint32(reqData, &devId)) {
        HDF_LOGE("%s: HdfSbufReadBuffer failed", __func__);
        return HDF_FAILURE;
    }

    ops = GetDispOps();
    if (ops == NULL) {
        HDF_LOGE("%s: ops is null", __func__);
        return HDF_FAILURE;
    }
    (void)memset_s(&info, sizeof(struct DispInfo), 0, sizeof(struct DispInfo));
    if (ops->getDispInfo != NULL) {
        if (ops->getDispInfo(devId, &info)) {
            HDF_LOGE("%s: getDispInfo failed", __func__);
            return HDF_FAILURE;
        }
    }
    if (!HdfSbufWriteBuffer(rspData, &info, sizeof(struct DispInfo)) != 0) {
        HDF_LOGE("%s: copy info failed", __func__);
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

static int32_t SetPowerMode(struct HdfDeviceObject *device, struct HdfSBuf *reqData, struct HdfSBuf *rspData)
{
    uint32_t para = 0;

    (void)device;
    (void)rspData;
    if (reqData == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }
    if (!HdfSbufReadUint32(reqData, &para)) {
        HDF_LOGE("%s: HdfSbufReadBuffer failed", __func__);
        return HDF_FAILURE;
    }
    uint32_t devId = (para >> OFFSET_TWO_BYTE) & 0xffff;
    uint32_t mode = para & 0xffff;
    if (mode == DISP_ON) {
        return DispOn(devId);
    }
    if (mode == DISP_OFF) {
        return DispOff(devId);
    }
    HDF_LOGE("not support mode:%u", mode);
    return HDF_FAILURE;
}

static int32_t SetBacklight(struct HdfDeviceObject *device, struct HdfSBuf *reqData, struct HdfSBuf *rspData)
{
    int32_t ret;
    (void)device;
    (void)rspData;
    if (reqData == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }
    uint32_t para = 0;
    if (!HdfSbufReadUint32(reqData, &para)) {
        HDF_LOGE("%s: HdfSbufReadBuffer failed", __func__);
        return HDF_FAILURE;
    }
    uint32_t devId = (para >> OFFSET_TWO_BYTE) & 0xffff;
    uint32_t level = para & 0xffff;
    struct PanelInfo *info = GetPanelInfo(devId);
    if (info == NULL) {
        HDF_LOGE("%s:GetPanelInfo failed", __func__);
        return HDF_FAILURE;
    }
    if (level > info->blk.maxLevel) {
        level = info->blk.maxLevel;
    } else if (level < info->blk.minLevel && level != 0) {
        level = info->blk.minLevel;
    }
    struct DispOperations *ops = GetDispOps();
    if (ops == NULL) {
        HDF_LOGE("%s: ops is null", __func__);
        return HDF_FAILURE;
    }
    if (ops->setBacklight != NULL) {
        ret = ops->setBacklight(devId, level);
        if (ret != HDF_SUCCESS) {
            HDF_LOGE("%s: setBacklight failed", __func__);
            return HDF_FAILURE;
        }
    }
    HDF_LOGI("%s:level = %u", __func__, level);
    return HDF_SUCCESS;
}

DispCmdHandle g_dispCmdHandle[] = {
    DispInit,
    GetDispInfo,
    SetPowerMode,
    SetBacklight,
};

static int32_t DispCmdProcess(struct HdfDeviceObject *device, int32_t cmd, struct HdfSBuf *reqData,
    struct HdfSBuf *rspData)
{
    int32_t cmdNum = sizeof(g_dispCmdHandle) / sizeof(g_dispCmdHandle[0]);
    if (device == NULL || reqData == NULL || rspData == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }
    if (cmd >= cmdNum || cmd < 0) {
        HDF_LOGE("%s: invalid cmd = %d", __func__, cmd);
        return HDF_FAILURE;
    }
    HDF_LOGD("%s: cmd = %d", __func__, cmd);
    if (g_dispCmdHandle[cmd] == NULL) {
        return HDF_FAILURE;
    }

    return g_dispCmdHandle[cmd](device, reqData, rspData);
}

static int32_t HdfDispDispatch(struct HdfDeviceIoClient *client, int id, struct HdfSBuf *data,
    struct HdfSBuf *reply)
{
    if (client == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }
    return DispCmdProcess(client->device, id, data, reply);
}

static int HdfDispBind(struct HdfDeviceObject *dev)
{
    if (dev == NULL) {
        return HDF_FAILURE;
    }
    static struct IDeviceIoService dispService = {
        .object.objectId = 1,
        .Dispatch = HdfDispDispatch,
    };
    dev->service = &dispService;
    return HDF_SUCCESS;
}

static int32_t HdfDispEntryInit(struct HdfDeviceObject *object)
{
    if (object == NULL) {
        HDF_LOGE("%s: object is null!", __func__);
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

struct HdfDriverEntry g_dispDevEntry = {
    .moduleVersion = 1,
    .moduleName = "HDF_DISP",
    .Init = HdfDispEntryInit,
    .Bind = HdfDispBind,
};

HDF_INIT(g_dispDevEntry);
