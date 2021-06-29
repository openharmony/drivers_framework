/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "securec.h"
#include "accel_bmi160.h"
#include "hdf_base.h"
#include "hdf_device_desc.h"
#include "osal_math.h"
#include "osal_mem.h"
#include "sensor_accel_driver.h"
#include "sensor_config_controller.h"
#include "sensor_device_manager.h"
#include "sensor_platform_if.h"

#define HDF_LOG_TAG    sensor_accel_driver_c

#define HDF_ACCEL_WORK_QUEUE_NAME    "hdf_accel_work_queue"

static struct AccelDetectIfList g_accelDetectIfList[] = {
    {ACCEL_CHIP_NAME_BMI160, DetectAccelBim160Chip},
};

static struct AccelDrvData *g_accelDrvData = NULL;

static struct AccelDrvData *AccelGetDrvData(void)
{
    return g_accelDrvData;
}

static struct SensorRegCfgGroupNode *g_regCfgGroup[SENSOR_GROUP_MAX] = { NULL };

int32_t RegisterAccelChipOps(const struct AccelOpsCall *ops)
{
    struct AccelDrvData *drvData = NULL;

    CHECK_NULL_PTR_RETURN_VALUE(ops, HDF_ERR_INVALID_PARAM);

    drvData = AccelGetDrvData();
    CHECK_NULL_PTR_RETURN_VALUE(drvData, HDF_ERR_INVALID_PARAM);
    drvData->ops.Init = ops->Init;
    drvData->ops.ReadData = ops->ReadData;
    return HDF_SUCCESS;
}

static void AccelDataWorkEntry(void *arg)
{
    int32_t ret;
    struct AccelDrvData *drvData = (struct AccelDrvData *)arg;
    CHECK_NULL_PTR_RETURN(drvData);
    CHECK_NULL_PTR_RETURN(drvData->ops.ReadData);

    ret = drvData->ops.ReadData(drvData->accelCfg);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: accel read data failed", __func__);
        return;
    }
}

static void AccelTimerEntry(uintptr_t arg)
{
    int64_t interval;
    int32_t ret;
    struct AccelDrvData *drvData = (struct AccelDrvData *)arg;
    CHECK_NULL_PTR_RETURN(drvData);

    if (!HdfAddWork(&drvData->accelWorkQueue, &drvData->accelWork)) {
        HDF_LOGE("%s: accel add work queue failed", __func__);
    }

    interval = OsalDivS64(drvData->interval, (SENSOR_CONVERT_UNIT * SENSOR_CONVERT_UNIT));
    interval = (interval < SENSOR_TIMER_MIN_TIME) ? SENSOR_TIMER_MIN_TIME : interval;
    ret = OsalTimerSetTimeout(&drvData->accelTimer, interval);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: accel modify time failed", __func__);
    }
}

static int32_t InitAccelData(void)
{
    struct AccelDrvData *drvData = AccelGetDrvData();
    int32_t ret;
    CHECK_NULL_PTR_RETURN_VALUE(drvData, HDF_ERR_INVALID_PARAM);

    if (drvData->initStatus) {
        return HDF_SUCCESS;
    }

    if (HdfWorkQueueInit(&drvData->accelWorkQueue, HDF_ACCEL_WORK_QUEUE_NAME) != HDF_SUCCESS) {
        HDF_LOGE("%s: accel init work queue failed", __func__);
        return HDF_FAILURE;
    }

    if (HdfWorkInit(&drvData->accelWork, AccelDataWorkEntry, drvData) != HDF_SUCCESS) {
        HDF_LOGE("%s: accel create thread failed", __func__);
        return HDF_FAILURE;
    }

    CHECK_NULL_PTR_RETURN_VALUE(drvData->ops.Init, HDF_ERR_INVALID_PARAM);

    ret = drvData->ops.Init(drvData->accelCfg);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: accel create thread failed", __func__);
        return HDF_FAILURE;
    }

    drvData->interval = SENSOR_TIMER_MIN_TIME;
    drvData->initStatus = true;
    drvData->enable = false;

    return HDF_SUCCESS;
}

static int32_t SetAccelEnable(void)
{
    int32_t ret;
    struct AccelDrvData *drvData = AccelGetDrvData();

    CHECK_NULL_PTR_RETURN_VALUE(drvData, HDF_ERR_INVALID_PARAM);
    CHECK_NULL_PTR_RETURN_VALUE(drvData->accelCfg, HDF_ERR_INVALID_PARAM);

    if (drvData->enable) {
        HDF_LOGE("%s: accel sensor is enabled", __func__);
        return HDF_SUCCESS;
    }

    ret = SetSensorRegCfgArray(&drvData->accelCfg->busCfg, drvData->accelCfg->regCfgGroup[SENSOR_ENABLE_GROUP]);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: accel sensor enable config failed", __func__);
        return ret;
    }

    ret = OsalTimerCreate(&drvData->accelTimer, SENSOR_TIMER_MIN_TIME, AccelTimerEntry, (uintptr_t)drvData);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: accel create timer failed[%d]", __func__, ret);
        return ret;
    }

    ret = OsalTimerStartLoop(&drvData->accelTimer);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: accel start timer failed[%d]", __func__, ret);
        return ret;
    }
    drvData->enable = true;

    return HDF_SUCCESS;
}

static int32_t SetAccelDisable(void)
{
    int32_t ret;
    struct AccelDrvData *drvData = AccelGetDrvData();

    CHECK_NULL_PTR_RETURN_VALUE(drvData, HDF_ERR_INVALID_PARAM);
    CHECK_NULL_PTR_RETURN_VALUE(drvData->accelCfg, HDF_ERR_INVALID_PARAM);

    if (!drvData->enable) {
        HDF_LOGE("%s: accel sensor had disable", __func__);
        return HDF_SUCCESS;
    }

    ret = SetSensorRegCfgArray(&drvData->accelCfg->busCfg, drvData->accelCfg->regCfgGroup[SENSOR_DISABLE_GROUP]);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: accel sensor disable config failed", __func__);
        return ret;
    }

    ret = OsalTimerDelete(&drvData->accelTimer);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: accel delete timer failed", __func__);
        return ret;
    }
    drvData->enable = false;
    return HDF_SUCCESS;
}

static int32_t SetAccelBatch(int64_t samplingInterval, int64_t interval)
{
    (void)interval;

    struct AccelDrvData *drvData = NULL;

    drvData = AccelGetDrvData();
    CHECK_NULL_PTR_RETURN_VALUE(drvData, HDF_ERR_INVALID_PARAM);

    drvData->interval = samplingInterval;

    return HDF_SUCCESS;
}

static int32_t SetAccelMode(int32_t mode)
{
    return (mode == SENSOR_WORK_MODE_REALTIME) ? HDF_SUCCESS : HDF_FAILURE;
}

static int32_t SetAccelOption(uint32_t option)
{
    (void)option;
    return HDF_SUCCESS;
}

static int32_t DispatchAccel(struct HdfDeviceIoClient *client,
    int cmd, struct HdfSBuf *data, struct HdfSBuf *reply)
{
    (void)client;
    (void)cmd;
    (void)data;
    (void)reply;

    return HDF_SUCCESS;
}

int32_t BindAccelDriver(struct HdfDeviceObject *device)
{
    CHECK_NULL_PTR_RETURN_VALUE(device, HDF_ERR_INVALID_PARAM);

    struct AccelDrvData *drvData = (struct AccelDrvData *)OsalMemCalloc(sizeof(*drvData));
    if (drvData == NULL) {
        HDF_LOGE("%s: malloc accel drv data fail!", __func__);
        return HDF_ERR_MALLOC_FAIL;
    }

    drvData->ioService.Dispatch = DispatchAccel;
    drvData->device = device;
    device->service = &drvData->ioService;
    g_accelDrvData = drvData;
    return HDF_SUCCESS;
}

static int32_t InitAccelOps(struct SensorDeviceInfo *deviceInfo)
{
    struct AccelDrvData *drvData = AccelGetDrvData();

    CHECK_NULL_PTR_RETURN_VALUE(drvData, HDF_ERR_INVALID_PARAM);

    deviceInfo->ops.Enable = SetAccelEnable;
    deviceInfo->ops.Disable = SetAccelDisable;
    deviceInfo->ops.SetBatch = SetAccelBatch;
    deviceInfo->ops.SetMode = SetAccelMode;
    deviceInfo->ops.SetOption = SetAccelOption;

    if (memcpy_s(&deviceInfo->sensorInfo, sizeof(deviceInfo->sensorInfo),
        &drvData->accelCfg->sensorInfo, sizeof(drvData->accelCfg->sensorInfo)) != EOK) {
        HDF_LOGE("%s: copy sensor info failed", __func__);
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

static int32_t InitAccelAfterConfig(void)
{
    struct SensorDeviceInfo deviceInfo;

    if (InitAccelData() != HDF_SUCCESS) {
        HDF_LOGE("%s: init accel config failed", __func__);
        return HDF_FAILURE;
    }

    if (InitAccelOps(&deviceInfo) != HDF_SUCCESS) {
        HDF_LOGE("%s: init accel ops failed", __func__);
        return HDF_FAILURE;
    }

    if (AddSensorDevice(&deviceInfo) != HDF_SUCCESS) {
        HDF_LOGE("%s: add accel device failed", __func__);
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

static int32_t DetectAccelChip(void)
{
    int32_t num;
    int32_t ret;
    int32_t loop;
    struct AccelDrvData *drvData = AccelGetDrvData();

    CHECK_NULL_PTR_RETURN_VALUE(drvData, HDF_ERR_INVALID_PARAM);
    CHECK_NULL_PTR_RETURN_VALUE(drvData->accelCfg, HDF_ERR_INVALID_PARAM);

    num = sizeof(g_accelDetectIfList) / sizeof(g_accelDetectIfList[0]);
    for (loop = 0; loop < num; ++loop) {
        if (g_accelDetectIfList[loop].DetectChip != NULL) {
            ret = g_accelDetectIfList[loop].DetectChip(drvData->accelCfg);
            if (ret == HDF_SUCCESS) {
                drvData->detectFlag = true;
                return HDF_SUCCESS;
            }
        }
    }

    HDF_LOGE("%s: detect accel device failed", __func__);
    drvData->detectFlag = false;
    return HDF_FAILURE;
}

int32_t InitAccelDriver(struct HdfDeviceObject *device)
{
    CHECK_NULL_PTR_RETURN_VALUE(device, HDF_ERR_INVALID_PARAM);
    struct AccelDrvData *drvData = (struct AccelDrvData *)device->service;
    CHECK_NULL_PTR_RETURN_VALUE(drvData, HDF_ERR_INVALID_PARAM);

    if (drvData->detectFlag) {
        HDF_LOGE("%s: accel sensor have detected", __func__);
        return HDF_SUCCESS;
    }

    drvData->accelCfg = (struct SensorCfgData *)OsalMemCalloc(sizeof(*drvData->accelCfg));
    if (drvData->accelCfg == NULL) {
        HDF_LOGE("%s: malloc sensor config data failed", __func__);
        return HDF_FAILURE;
    }

    drvData->accelCfg->regCfgGroup = &g_regCfgGroup[0];

    if (GetSensorBaseConfigData(device->property, drvData->accelCfg) != HDF_SUCCESS) {
        HDF_LOGE("%s: get sensor base config failed", __func__);
        goto BASE_CONFIG_EXIT;
    }

    // if return failure, hdf framework go to next detect sensor
    if (DetectAccelChip() != HDF_SUCCESS) {
        HDF_LOGE("%s: accel sensor detect device no exist", __func__);
        goto DETECT_CHIP_EXIT;
    }
    drvData->detectFlag = true;

    if (ParseSensorRegConfig(drvData->accelCfg) != HDF_SUCCESS) {
        HDF_LOGE("%s: detect sensor device failed", __func__);
        goto REG_CONFIG_EXIT;
    }

    if (InitAccelAfterConfig() != HDF_SUCCESS) {
        HDF_LOGE("%s: init accel after config failed", __func__);
        goto INIT_EXIT;
    }

    HDF_LOGI("%s: init accel driver success", __func__);
    return HDF_SUCCESS;

INIT_EXIT:
    (void)DeleteSensorDevice(&drvData->accelCfg->sensorInfo);
REG_CONFIG_EXIT:
    ReleaseSensorAllRegConfig(drvData->accelCfg);
    (void)ReleaseSensorBusHandle(&drvData->accelCfg->busCfg);
DETECT_CHIP_EXIT:
    drvData->detectFlag = false;
BASE_CONFIG_EXIT:
    drvData->accelCfg->root = NULL;
    drvData->accelCfg->regCfgGroup = NULL;
    OsalMemFree(drvData->accelCfg);
    drvData->accelCfg = NULL;
    return HDF_FAILURE;
}

void ReleaseAccelDriver(struct HdfDeviceObject *device)
{
    CHECK_NULL_PTR_RETURN(device);

    struct AccelDrvData *drvData = (struct AccelDrvData *)device->service;
    CHECK_NULL_PTR_RETURN(drvData);

    (void)DeleteSensorDevice(&drvData->accelCfg->sensorInfo);
    drvData->detectFlag = false;

    if (drvData->accelCfg != NULL) {
        drvData->accelCfg->root = NULL;
        drvData->accelCfg->regCfgGroup = NULL;
        ReleaseSensorAllRegConfig(drvData->accelCfg);
        (void)ReleaseSensorBusHandle(&drvData->accelCfg->busCfg);
        OsalMemFree(drvData->accelCfg);
        drvData->accelCfg = NULL;
    }

    drvData->initStatus = false;
}

struct HdfDriverEntry g_sensorAccelDevEntry = {
    .moduleVersion = 1,
    .moduleName = "HDF_SENSOR_ACCEL",
    .Bind = BindAccelDriver,
    .Init = InitAccelDriver,
    .Release = ReleaseAccelDriver,
};

HDF_INIT(g_sensorAccelDevEntry);
