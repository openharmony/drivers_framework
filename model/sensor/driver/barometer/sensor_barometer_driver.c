/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "securec.h"
#include "barometer_bmp180.h"
#include "hdf_base.h"
#include "hdf_device_desc.h"
#include "osal_math.h"
#include "osal_mem.h"
#include "sensor_barometer_driver.h"
#include "sensor_config_controller.h"
#include "sensor_device_manager.h"
#include "sensor_platform_if.h"

#define HDF_LOG_TAG    sensor_barometer_driver_c

#define HDF_BAROMETER_WORK_QUEUE_NAME    "hdf_barometer_work_queue"

static struct BarometerDetectIfList g_barometerDetectIfList[] = {
    {BAROMETER_CHIP_NAME_BMP180, DetectBarometerBmp180Chip},
};

static struct BarometerDrvData *g_barometerDrvData = NULL;

static struct BarometerDrvData *BarometerGetDrvData(void)
{
    return g_barometerDrvData;
}

static struct SensorRegCfgGroupNode *g_regCfgGroup[SENSOR_GROUP_MAX] = { NULL };

int32_t RegisterBarometerChipOps(const struct BarometerOpsCall *ops)
{
    struct BarometerDrvData *drvData = NULL;

    CHECK_NULL_PTR_RETURN_VALUE(ops, HDF_ERR_INVALID_PARAM);

    drvData = BarometerGetDrvData();
    CHECK_NULL_PTR_RETURN_VALUE(drvData, HDF_ERR_INVALID_PARAM);
    drvData->ops.Init = ops->Init;
    drvData->ops.ReadData = ops->ReadData;
    return HDF_SUCCESS;
}

static void BarometerDataWorkEntry(void *arg)
{
    int32_t ret;
    struct BarometerDrvData *drvData = (struct BarometerDrvData *)arg;
    CHECK_NULL_PTR_RETURN(drvData);
    CHECK_NULL_PTR_RETURN(drvData->ops.ReadData);

    ret = drvData->ops.ReadData(drvData->barometerCfg);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: barometer read data failed", __func__);
        return;
    }
}

static void BarometerTimerEntry(uintptr_t arg)
{
    int64_t interval;
    int32_t ret;
    struct BarometerDrvData *drvData = (struct BarometerDrvData *)arg;
    CHECK_NULL_PTR_RETURN(drvData);

    if (!HdfAddWork(&drvData->barometerWorkQueue, &drvData->barometerWork)) {
        HDF_LOGE("%s: barometer add work queue failed", __func__);
    }

    interval = OsalDivS64(drvData->interval, (SENSOR_CONVERT_UNIT * SENSOR_CONVERT_UNIT));
    interval = (interval < SENSOR_TIMER_MIN_TIME) ? SENSOR_TIMER_MIN_TIME : interval;
    ret = OsalTimerSetTimeout(&drvData->barometerTimer, interval);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: barometer modify time failed", __func__);
    }
}

static int32_t InitBarometerData(void)
{
    struct BarometerDrvData *drvData = BarometerGetDrvData();
    int32_t ret;
    CHECK_NULL_PTR_RETURN_VALUE(drvData, HDF_ERR_INVALID_PARAM);

    if (drvData->initStatus) {
        return HDF_SUCCESS;
    }

    if (HdfWorkQueueInit(&drvData->barometerWorkQueue, HDF_BAROMETER_WORK_QUEUE_NAME) != HDF_SUCCESS) {
        HDF_LOGE("%s: barometer init work queue failed", __func__);
        return HDF_FAILURE;
    }

    if (HdfWorkInit(&drvData->barometerWork, BarometerDataWorkEntry, drvData) != HDF_SUCCESS) {
        HDF_LOGE("%s: barometer create thread failed", __func__);
        return HDF_FAILURE;
    }

    CHECK_NULL_PTR_RETURN_VALUE(drvData->ops.Init, HDF_ERR_INVALID_PARAM);

    ret = drvData->ops.Init(drvData->barometerCfg);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: barometer create thread failed", __func__);
        return HDF_FAILURE;
    }

    drvData->interval = SENSOR_TIMER_MIN_TIME;
    drvData->initStatus = true;
    drvData->enable = false;

    return HDF_SUCCESS;
}

static int32_t SetBarometerEnable(void)
{
    int32_t ret;
    struct BarometerDrvData *drvData = BarometerGetDrvData();

    CHECK_NULL_PTR_RETURN_VALUE(drvData, HDF_ERR_INVALID_PARAM);
    CHECK_NULL_PTR_RETURN_VALUE(drvData->barometerCfg, HDF_ERR_INVALID_PARAM);

    if (drvData->enable) {
        HDF_LOGE("%s: barometer sensor is enabled", __func__);
        return HDF_SUCCESS;
    }

    ret = SetSensorRegCfgArray(&drvData->barometerCfg->busCfg, drvData->barometerCfg->regCfgGroup[SENSOR_ENABLE_GROUP]);
   if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: barometer sensor enable config failed", __func__);
        return ret;
   }

    ret = OsalTimerCreate(&drvData->barometerTimer, SENSOR_TIMER_MIN_TIME, BarometerTimerEntry, (uintptr_t)drvData);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: barometer create timer failed[%d]", __func__, ret);
        return ret;
    }

    ret = OsalTimerStartLoop(&drvData->barometerTimer);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: barometer start timer failed[%d]", __func__, ret);
        return ret;
    }
    drvData->enable = true;

    return HDF_SUCCESS;
}

static int32_t SetBarometerDisable(void)
{
    int32_t ret;
    struct BarometerDrvData *drvData = BarometerGetDrvData();

    CHECK_NULL_PTR_RETURN_VALUE(drvData, HDF_ERR_INVALID_PARAM);
    CHECK_NULL_PTR_RETURN_VALUE(drvData->barometerCfg, HDF_ERR_INVALID_PARAM);

    if (!drvData->enable) {
        HDF_LOGE("%s: barometer sensor had disable", __func__);
        return HDF_SUCCESS;
    }

    ret = SetSensorRegCfgArray(&drvData->barometerCfg->busCfg, drvData->barometerCfg->regCfgGroup[SENSOR_DISABLE_GROUP]);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: barometer sensor disable config failed", __func__);
        return ret;
    }

    ret = OsalTimerDelete(&drvData->barometerTimer);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: barometer delete timer failed", __func__);
        return ret;
    }
    drvData->enable = false;
    return HDF_SUCCESS;
}

static int32_t SetBarometerBatch(int64_t samplingInterval, int64_t interval)
{
    (void)interval;

    struct BarometerDrvData *drvData = NULL;

    drvData = BarometerGetDrvData();
    CHECK_NULL_PTR_RETURN_VALUE(drvData, HDF_ERR_INVALID_PARAM);

    drvData->interval = samplingInterval;

    return HDF_SUCCESS;
}

static int32_t SetBarometerMode(int32_t mode)
{
    return (mode == SENSOR_WORK_MODE_REALTIME) ? HDF_SUCCESS : HDF_FAILURE;
}

static int32_t SetBarometerOption(uint32_t option)
{
    (void)option;
    return HDF_SUCCESS;
}

static int32_t DispatchBarometer(struct HdfDeviceIoClient *client,
    int cmd, struct HdfSBuf *data, struct HdfSBuf *reply)
{
    (void)client;
    (void)cmd;
    (void)data;
    (void)reply;

    return HDF_SUCCESS;
}

int32_t BindBarometerDriver(struct HdfDeviceObject *device)
{
    CHECK_NULL_PTR_RETURN_VALUE(device, HDF_ERR_INVALID_PARAM);

    struct BarometerDrvData *drvData = (struct BarometerDrvData *)OsalMemCalloc(sizeof(*drvData));
    if (drvData == NULL) {
        HDF_LOGE("%s: malloc barometer drv data fail!", __func__);
        return HDF_ERR_MALLOC_FAIL;
    }

    drvData->ioService.Dispatch = DispatchBarometer;
    drvData->device = device;
    device->service = &drvData->ioService;
    g_barometerDrvData = drvData;
    return HDF_SUCCESS;
}

static int32_t InitBarometerOps(struct SensorDeviceInfo *deviceInfo)
{
    struct BarometerDrvData *drvData = BarometerGetDrvData();

    CHECK_NULL_PTR_RETURN_VALUE(drvData, HDF_ERR_INVALID_PARAM);

    deviceInfo->ops.Enable = SetBarometerEnable;
    deviceInfo->ops.Disable = SetBarometerDisable;
    deviceInfo->ops.SetBatch = SetBarometerBatch;
    deviceInfo->ops.SetMode = SetBarometerMode;
    deviceInfo->ops.SetOption = SetBarometerOption;

    if (memcpy_s(&deviceInfo->sensorInfo, sizeof(deviceInfo->sensorInfo),
        &drvData->barometerCfg->sensorInfo, sizeof(drvData->barometerCfg->sensorInfo)) != EOK) {
        HDF_LOGE("%s: copy sensor info failed", __func__);
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

static int32_t InitBarometerAfterConfig(void)
{
    struct SensorDeviceInfo deviceInfo;

    if (InitBarometerData() != HDF_SUCCESS) {
        HDF_LOGE("%s: init barometer config failed", __func__);
        return HDF_FAILURE;
    }

    if (InitBarometerOps(&deviceInfo) != HDF_SUCCESS) {
        HDF_LOGE("%s: init barometer ops failed", __func__);
        return HDF_FAILURE;
    }

    if (AddSensorDevice(&deviceInfo) != HDF_SUCCESS) {
        HDF_LOGE("%s: add barometer device failed", __func__);
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

static int32_t DetectBarometerChip(void)
{
    int32_t num;
    int32_t ret;
    int32_t loop;
    struct BarometerDrvData *drvData = BarometerGetDrvData();

    CHECK_NULL_PTR_RETURN_VALUE(drvData, HDF_ERR_INVALID_PARAM);
    CHECK_NULL_PTR_RETURN_VALUE(drvData->barometerCfg, HDF_ERR_INVALID_PARAM);

    num = sizeof(g_barometerDetectIfList) / sizeof(g_barometerDetectIfList[0]);
    for (loop = 0; loop < num; ++loop) {
        if (g_barometerDetectIfList[loop].DetectChip != NULL) {
            ret = g_barometerDetectIfList[loop].DetectChip(drvData->barometerCfg);
            if (ret == HDF_SUCCESS) {
                drvData->detectFlag = true;
                return HDF_SUCCESS;
            }
        }
    }

    HDF_LOGE("%s: detect barometer device failed", __func__);
    drvData->detectFlag = false;
    return HDF_FAILURE;
}

int32_t InitBarometerDriver(struct HdfDeviceObject *device)
{

    CHECK_NULL_PTR_RETURN_VALUE(device, HDF_ERR_INVALID_PARAM);
    struct BarometerDrvData *drvData = (struct BarometerDrvData *)device->service;
    CHECK_NULL_PTR_RETURN_VALUE(drvData, HDF_ERR_INVALID_PARAM);

    if (drvData->detectFlag) {
        HDF_LOGE("%s: barometer sensor have detected", __func__);
        return HDF_SUCCESS;
    }

    drvData->barometerCfg = (struct SensorCfgData *)OsalMemCalloc(sizeof(*drvData->barometerCfg));
    if (drvData->barometerCfg == NULL) {
        HDF_LOGE("%s: malloc sensor config data failed", __func__);
        return HDF_FAILURE;
    }

    drvData->barometerCfg->regCfgGroup = &g_regCfgGroup[0];

    if (GetSensorBaseConfigData(device->property, drvData->barometerCfg) != HDF_SUCCESS) {
        HDF_LOGE("%s: get sensor base config failed", __func__);
        goto BASE_CONFIG_EXIT;
    }
   
    // if return failure, hdf framework go to next detect sensor
    if (DetectBarometerChip() != HDF_SUCCESS) {
        HDF_LOGE("%s: barometer sensor detect device no exist", __func__);
        goto DETECT_CHIP_EXIT;
    }
    drvData->detectFlag = true;

    if (ParseSensorRegConfig(drvData->barometerCfg) != HDF_SUCCESS) {
        HDF_LOGE("%s: detect sensor device failed", __func__);
        goto REG_CONFIG_EXIT;
    }
     HDF_LOGE("%d", drvData->barometerCfg->regCfgGroup);
    if (InitBarometerAfterConfig() != HDF_SUCCESS) {
        HDF_LOGE("%s: init barometer after config failed", __func__);
        goto INIT_EXIT;
    }

    HDF_LOGI("%s: init barometer driver success", __func__);
    return HDF_SUCCESS;

INIT_EXIT:
    (void)DeleteSensorDevice(&drvData->barometerCfg->sensorInfo);
REG_CONFIG_EXIT:
    ReleaseSensorAllRegConfig(drvData->barometerCfg);
    (void)ReleaseSensorBusHandle(&drvData->barometerCfg->busCfg);
DETECT_CHIP_EXIT:
    drvData->detectFlag = false;
BASE_CONFIG_EXIT:
    drvData->barometerCfg->root = NULL;
    drvData->barometerCfg->regCfgGroup = NULL;
    OsalMemFree(drvData->barometerCfg);
    drvData->barometerCfg = NULL;
    return HDF_FAILURE;
}

void ReleaseBarometerDriver(struct HdfDeviceObject *device)
{
    CHECK_NULL_PTR_RETURN(device);

    struct BarometerDrvData *drvData = (struct BarometerDrvData *)device->service;
    CHECK_NULL_PTR_RETURN(drvData);

    (void)DeleteSensorDevice(&drvData->barometerCfg->sensorInfo);
    drvData->detectFlag = false;

    if (drvData->barometerCfg != NULL) {
        drvData->barometerCfg->root = NULL;
        drvData->barometerCfg->regCfgGroup = NULL;
        ReleaseSensorAllRegConfig(drvData->barometerCfg);
        (void)ReleaseSensorBusHandle(&drvData->barometerCfg->busCfg);
        OsalMemFree(drvData->barometerCfg);
        drvData->barometerCfg = NULL;
    }

    drvData->initStatus = false;
}

struct HdfDriverEntry g_sensorBarometerDevEntry = {
    .moduleVersion = 1,
    .moduleName = "HDF_SENSOR_BAROMETER",
    .Bind = BindBarometerDriver,
    .Init = InitBarometerDriver,
    .Release = ReleaseBarometerDriver,
};

HDF_INIT(g_sensorBarometerDevEntry);