/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "sensor_als_driver.h"
#include <securec.h>
#include "als_bh1745.h"
#include "osal_math.h"
#include "osal_mem.h"
#include "sensor_config_controller.h"
#include "sensor_device_manager.h"
#include "sensor_platform_if.h"

#define HDF_LOG_TAG    sensor_als_driver_c

#define HDF_ALS_WORK_QUEUE_NAME    "hdf_als_work_queue"

static struct AlsDrvData *g_alsDrvData = NULL;

static struct AlsDrvData *AlsGetDrvData(void)
{
    return g_alsDrvData;
}

static struct SensorRegCfgGroupNode *g_regCfgGroup[SENSOR_GROUP_MAX] = { NULL };

int32_t AlsRegisterChipOps(const struct AlsOpsCall *ops)
{
    struct AlsDrvData *drvData = AlsGetDrvData();

    CHECK_NULL_PTR_RETURN_VALUE(drvData, HDF_ERR_INVALID_PARAM);
    CHECK_NULL_PTR_RETURN_VALUE(ops, HDF_ERR_INVALID_PARAM);

    drvData->ops.Init = ops->Init;
    drvData->ops.ReadData = ops->ReadData;
    return HDF_SUCCESS;
}

static void AlsDataWorkEntry(void *arg)
{
    struct AlsDrvData *drvData = NULL;

    drvData = (struct AlsDrvData *)arg;
    CHECK_NULL_PTR_RETURN(drvData);

    if (drvData->ops.ReadData == NULL) {
        HDF_LOGI("%s: Als ReadData function NULl", __func__);
        return;
    }
    if (drvData->ops.ReadData(drvData->alsCfg) != HDF_SUCCESS) {
        HDF_LOGE("%s: Als read data failed", __func__);
    }
}

static void AlsTimerEntry(uintptr_t arg)
{
    int64_t interval;
    int32_t ret;
    struct AlsDrvData *drvData = (struct AlsDrvData *)arg;
    CHECK_NULL_PTR_RETURN(drvData);

    if (!HdfAddWork(&drvData->alsWorkQueue, &drvData->alsWork)) {
        HDF_LOGE("%s: Als add work queue failed", __func__);
    }

    interval = OsalDivS64(drvData->interval, (SENSOR_CONVERT_UNIT * SENSOR_CONVERT_UNIT));
    interval = (interval < SENSOR_TIMER_MIN_TIME) ? SENSOR_TIMER_MIN_TIME : interval;
    ret = OsalTimerSetTimeout(&drvData->alsTimer, interval);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: Als modify time failed", __func__);
    }
}

static int32_t InitAlsData(struct AlsDrvData *drvData)
{
    if (HdfWorkQueueInit(&drvData->alsWorkQueue, HDF_ALS_WORK_QUEUE_NAME) != HDF_SUCCESS) {
        HDF_LOGE("%s: Als init work queue failed", __func__);
        return HDF_FAILURE;
    }

    if (HdfWorkInit(&drvData->alsWork, AlsDataWorkEntry, drvData) != HDF_SUCCESS) {
        HDF_LOGE("%s: Als create thread failed", __func__);
        return HDF_FAILURE;
    }

    drvData->interval = SENSOR_TIMER_MIN_TIME;
    drvData->enable = false;
    drvData->detectFlag = false;

    return HDF_SUCCESS;
}

static int32_t SetAlsEnable(void)
{
    int32_t ret;
    struct AlsDrvData *drvData = AlsGetDrvData();

    CHECK_NULL_PTR_RETURN_VALUE(drvData, HDF_ERR_INVALID_PARAM);
    CHECK_NULL_PTR_RETURN_VALUE(drvData->alsCfg, HDF_ERR_INVALID_PARAM);

    if (drvData->enable) {
        HDF_LOGE("%s: Als sensor is enabled", __func__);
        return HDF_SUCCESS;
    }

    ret = SetSensorRegCfgArray(&drvData->alsCfg->busCfg, drvData->alsCfg->regCfgGroup[SENSOR_ENABLE_GROUP]);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: Als sensor enable config failed", __func__);
        return ret;
    }

    ret = OsalTimerCreate(&drvData->alsTimer, SENSOR_TIMER_MIN_TIME, AlsTimerEntry, (uintptr_t)drvData);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: Als create timer failed[%d]", __func__, ret);
        return ret;
    }

    ret = OsalTimerStartLoop(&drvData->alsTimer);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: Als start timer failed[%d]", __func__, ret);
        return ret;
    }
    drvData->enable = true;

    return HDF_SUCCESS;
}

static int32_t SetAlsDisable(void)
{
    int32_t ret;
    struct AlsDrvData *drvData = AlsGetDrvData();

    CHECK_NULL_PTR_RETURN_VALUE(drvData, HDF_ERR_INVALID_PARAM);
    CHECK_NULL_PTR_RETURN_VALUE(drvData->alsCfg, HDF_ERR_INVALID_PARAM);

    if (!drvData->enable) {
        HDF_LOGE("%s: Als sensor had disable", __func__);
        return HDF_SUCCESS;
    }

    ret = SetSensorRegCfgArray(&drvData->alsCfg->busCfg, drvData->alsCfg->regCfgGroup[SENSOR_DISABLE_GROUP]);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: Als sensor disable config failed", __func__);
        return ret;
    }

    ret = OsalTimerDelete(&drvData->alsTimer);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: Als delete timer failed", __func__);
        return ret;
    }
    drvData->enable = false;

    return HDF_SUCCESS;
}

static int32_t SetAlsBatch(int64_t samplingInterval, int64_t interval)
{
    (void)interval;

    struct AlsDrvData *drvData = NULL;

    drvData = AlsGetDrvData();
    CHECK_NULL_PTR_RETURN_VALUE(drvData, HDF_ERR_INVALID_PARAM);

    drvData->interval = samplingInterval;

    return HDF_SUCCESS;
}

static int32_t SetAlsMode(int32_t mode)
{
    return (mode == SENSOR_WORK_MODE_REALTIME) ? HDF_SUCCESS : HDF_FAILURE;
}

static int32_t SetAlsOption(uint32_t option)
{
    (void)option;
    return HDF_SUCCESS;
}

static int32_t DispatchAls(struct HdfDeviceIoClient *client,
    int cmd, struct HdfSBuf *data, struct HdfSBuf *reply)
{
    (void)client;
    (void)cmd;
    (void)data;
    (void)reply;

    return HDF_SUCCESS;
}

int32_t AlsBindDriver(struct HdfDeviceObject *device)
{
    CHECK_NULL_PTR_RETURN_VALUE(device, HDF_ERR_INVALID_PARAM);

    struct AlsDrvData *drvData = (struct AlsDrvData *)OsalMemCalloc(sizeof(*drvData));
    if (drvData == NULL) {
        HDF_LOGE("%s: Malloc als drv data fail!", __func__);
        return HDF_ERR_MALLOC_FAIL;
    }

    drvData->ioService.Dispatch = DispatchAls;
    drvData->device = device;
    device->service = &drvData->ioService;
    g_alsDrvData = drvData;
    return HDF_SUCCESS;
}

static int32_t InitAlsOps(struct SensorCfgData *config, struct SensorDeviceInfo *deviceInfo)
{
    CHECK_NULL_PTR_RETURN_VALUE(config, HDF_ERR_INVALID_PARAM);

    deviceInfo->ops.Enable = SetAlsEnable;
    deviceInfo->ops.Disable = SetAlsDisable;
    deviceInfo->ops.SetBatch = SetAlsBatch;
    deviceInfo->ops.SetMode = SetAlsMode;
    deviceInfo->ops.SetOption = SetAlsOption;

    if (memcpy_s(&deviceInfo->sensorInfo, sizeof(deviceInfo->sensorInfo),
        &config->sensorInfo, sizeof(config->sensorInfo)) != EOK) {
        HDF_LOGE("%s: Copy sensor info failed", __func__);
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

static int32_t InitAlsAfterDetected(struct SensorCfgData *config)
{
    struct SensorDeviceInfo deviceInfo;
    CHECK_NULL_PTR_RETURN_VALUE(config, HDF_ERR_INVALID_PARAM);

    if (InitAlsOps(config, &deviceInfo) != HDF_SUCCESS) {
        HDF_LOGE("%s: Init als ops failed", __func__);
        return HDF_FAILURE;
    }

    if (AddSensorDevice(&deviceInfo) != HDF_SUCCESS) {
        HDF_LOGE("%s: Add als device failed", __func__);
        return HDF_FAILURE;
    }

    if (ParseSensorRegConfig(config) != HDF_SUCCESS) {
        HDF_LOGE("%s: Parse sensor register failed", __func__);
        (void)DeleteSensorDevice(&config->sensorInfo);
        ReleaseSensorAllRegConfig(config);
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

struct SensorCfgData *AlsCreateCfgData(const struct DeviceResourceNode *node)
{
    struct AlsDrvData *drvData = AlsGetDrvData();

    if (drvData == NULL || node == NULL) {
        HDF_LOGE("%s: Als node pointer NULL", __func__);
        return NULL;
    }

    if (drvData->detectFlag) {
        HDF_LOGE("%s: Als sensor have detected", __func__);
        return NULL;
    }

    if (drvData->alsCfg == NULL) {
        HDF_LOGE("%s: Als alsCfg pointer NULL", __func__);
        return NULL;
    }

    if (GetSensorBaseConfigData(node, drvData->alsCfg) != HDF_SUCCESS) {
        HDF_LOGE("%s: Get sensor base config failed", __func__);
        goto BASE_CONFIG_EXIT;
    }

    if (DetectSensorDevice(drvData->alsCfg) != HDF_SUCCESS) {
        HDF_LOGI("%s: Als sensor detect device no exist", __func__);
        drvData->detectFlag = false;
        goto BASE_CONFIG_EXIT;
    }

    drvData->detectFlag = true;
    if (InitAlsAfterDetected(drvData->alsCfg) != HDF_SUCCESS) {
        HDF_LOGE("%s: Als sensor detect device no exist", __func__);
        goto INIT_EXIT;
    }

    return drvData->alsCfg;

INIT_EXIT:
    (void)ReleaseSensorBusHandle(&drvData->alsCfg->busCfg);
BASE_CONFIG_EXIT:
    drvData->alsCfg->root = NULL;
    (void)memset_s(&drvData->alsCfg->sensorInfo, sizeof(struct SensorBasicInfo), 0, sizeof(struct SensorBasicInfo));
    (void)memset_s(&drvData->alsCfg->busCfg, sizeof(struct SensorBusCfg), 0, sizeof(struct SensorBusCfg));
    (void)memset_s(&drvData->alsCfg->sensorAttr, sizeof(struct SensorAttr), 0, sizeof(struct SensorAttr));
    return NULL;
}

void AlsReleaseCfgData(struct SensorCfgData *alsCfg)
{
    CHECK_NULL_PTR_RETURN(alsCfg);

    (void)DeleteSensorDevice(&alsCfg->sensorInfo);
    ReleaseSensorAllRegConfig(alsCfg);
    (void)ReleaseSensorBusHandle(&alsCfg->busCfg);

    alsCfg->root = NULL;
    (void)memset_s(&alsCfg->sensorInfo, sizeof(struct SensorBasicInfo), 0, sizeof(struct SensorBasicInfo));
    (void)memset_s(&alsCfg->busCfg, sizeof(struct SensorBusCfg), 0, sizeof(struct SensorBusCfg));
    (void)memset_s(&alsCfg->sensorAttr, sizeof(struct SensorAttr), 0, sizeof(struct SensorAttr));
}

int32_t AlsInitDriver(struct HdfDeviceObject *device)
{
    CHECK_NULL_PTR_RETURN_VALUE(device, HDF_ERR_INVALID_PARAM);
    struct AlsDrvData *drvData = (struct AlsDrvData *)device->service;
    CHECK_NULL_PTR_RETURN_VALUE(drvData, HDF_ERR_INVALID_PARAM);

    if (InitAlsData(drvData) != HDF_SUCCESS) {
        HDF_LOGE("%s: Init als config failed", __func__);
        return HDF_FAILURE;
    }

    drvData->alsCfg = (struct SensorCfgData *)OsalMemCalloc(sizeof(*drvData->alsCfg));
    if (drvData->alsCfg == NULL) {
        HDF_LOGE("%s: Malloc als config data failed", __func__);
        return HDF_FAILURE;
    }

    drvData->alsCfg->regCfgGroup = &g_regCfgGroup[0];

    return HDF_SUCCESS;
}

void AlsReleaseDriver(struct HdfDeviceObject *device)
{
    CHECK_NULL_PTR_RETURN(device);

    struct AlsDrvData *drvData = (struct AlsDrvData *)device->service;
    CHECK_NULL_PTR_RETURN(drvData);

    if (drvData->detectFlag) {
        AlsReleaseCfgData(drvData->alsCfg);
    }

    OsalMemFree(drvData->alsCfg);
    drvData->alsCfg = NULL;

    HdfWorkDestroy(&drvData->alsWork);
    HdfWorkQueueDestroy(&drvData->alsWorkQueue);
    OsalMemFree(drvData);
}

struct HdfDriverEntry g_sensorAlsDevEntry = {
    .moduleVersion = 1,
    .moduleName = "HDF_SENSOR_ALS",
    .Bind = AlsBindDriver,
    .Init = AlsInitDriver,
    .Release = AlsReleaseDriver,
};

HDF_INIT(g_sensorAlsDevEntry);
