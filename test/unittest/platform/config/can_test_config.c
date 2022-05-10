/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "can_test.h"
#include "device_resource_if.h"
#include "hdf_base.h"
#include "hdf_device_desc.h"
#include "hdf_log.h"

static struct CanTestConfig g_config;

static int32_t CanTestDispatch(struct HdfDeviceIoClient *client, int cmd, struct HdfSBuf *data, struct HdfSBuf *reply)
{
    HDF_LOGD("%s: enter!", __func__);

    if (cmd != 0) {
        return HDF_ERR_NOT_SUPPORT;
    }

    if (reply == NULL) {
        HDF_LOGE("%s: reply is null!", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    if (!HdfSbufWriteBuffer(reply, &g_config, sizeof(g_config))) {
        HDF_LOGE("%s: write reply failed", __func__);
        return HDF_ERR_IO;
    }

    return HDF_SUCCESS;
}

static void CanTestSetDftConfig(struct CanTestConfig *config)
{
    config->busNum = CAN_TEST_BUS_NUM;
    config->bitRate = CAN_TEST_BIT_RATE;
    config->workMode = CAN_TEST_WORK_MODE;
}

static int32_t CanTestReadConfig(struct CanTestConfig *config, const struct DeviceResourceNode *node)
{
    int32_t ret;
    struct DeviceResourceIface *drsOps = NULL;

    drsOps = DeviceResourceGetIfaceInstance(HDF_CONFIG_SOURCE);
    if (drsOps == NULL) {
        HDF_LOGE("%s: invalid drs ops", __func__);
        return HDF_FAILURE;
    }

    ret = drsOps->GetUint16(node, "bus_num", &config->busNum, CAN_TEST_BUS_NUM);
    if (ret != HDF_SUCCESS) {
        HDF_LOGW("%s: read bus num failed, using default ...", __func__);
    }

    ret = drsOps->GetUint32(node, "bit_rate", &config->bitRate, CAN_TEST_BIT_RATE);
    if (ret != HDF_SUCCESS) {
        HDF_LOGW("%s: read bit rate failed, using default ...", __func__);
    }

    ret = drsOps->GetUint8(node, "work_mode", &config->workMode, CAN_TEST_WORK_MODE);
    if (ret != HDF_SUCCESS) {
        HDF_LOGW("%s: read reg len failed, using default ...", __func__);
    }

    return HDF_SUCCESS;
}

static int32_t CanTestBind(struct HdfDeviceObject *device)
{
    int32_t ret;
    static struct IDeviceIoService service;

    if (device == NULL) {
        HDF_LOGE("%s: device or config is null!", __func__);
        return HDF_ERR_IO;
    }

    if (device->property == NULL) {
        HDF_LOGI("%s: property not configed, using default", __func__);
        CanTestSetDftConfig(&g_config);
    } else {
        ret = CanTestReadConfig(&g_config, device->property);
        if (ret != HDF_SUCCESS) {
            HDF_LOGE("%s: read config failed", __func__);
            return ret;
        }
    }

    service.Dispatch = CanTestDispatch;
    device->service = &service;
    return HDF_SUCCESS;
}

static int32_t CanTestInit(struct HdfDeviceObject *device)
{
    (void)device;
    return HDF_SUCCESS;
}

static void CanTestRelease(struct HdfDeviceObject *device)
{
    if (device != NULL) {
        device->service = NULL;
    }
    return;
}

struct HdfDriverEntry g_canTestEntry = {
    .moduleVersion = 1,
    .Bind          = CanTestBind,
    .Init          = CanTestInit,
    .Release       = CanTestRelease,
    .moduleName    = "PLATFORM_CAN_TEST",
};
HDF_INIT(g_canTestEntry);
