/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "sensor_hall_driver.h"
#include <securec.h>
#include "gpio_if.h"
#include "hall_ak8789.h"
#include "hdf_base.h"
#include "hdf_device_desc.h"
#include "osal_irq.h"
#include "osal_math.h"
#include "osal_mem.h"
#include "sensor_config_controller.h"
#include "sensor_device_manager.h"
#include "sensor_platform_if.h"

#define HDF_LOG_TAG    sensor_hall_driver_c
#define HDF_HALL_WORK_QUEUE_NAME    "hdf_hall_work_queue"

static struct HallDetectIfList g_hallDetectIfList[] = {
    {HALL_CHIP_NAME_AK8789, DetectHallAk8789Chip},
};   

static struct HallDrvData *g_hallDrvData = NULL;

static struct HallDrvData *HallGetDrvData(void)
{
    return g_hallDrvData;
}

int32_t RegisterHallChipOps(const struct HallOpsCall *ops)
{
    struct HallDrvData *drvData = NULL;
    CHECK_NULL_PTR_RETURN_VALUE(ops, HDF_ERR_INVALID_PARAM);
    drvData = HallGetDrvData();
    CHECK_NULL_PTR_RETURN_VALUE(drvData, HDF_ERR_INVALID_PARAM);
    drvData->ops.Init = ops->Init;                                
    drvData->ops.ReadData = ops->ReadData;
    return HDF_SUCCESS;
}

static void HallDataWorkEntry(void *arg)
{
    int32_t ret;
    struct HallDrvData *drvData = (struct HallDrvData *)arg;
    CHECK_NULL_PTR_RETURN(drvData);
    CHECK_NULL_PTR_RETURN(drvData->ops.ReadData);

    ret = drvData->ops.ReadData(drvData->hallCfg);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: hall read data failed", __func__);
        return;
    }
}
                                            
static int32_t HallNorthPolarityIrqFunc(uint16_t gpio, void *data)
{
    (void)gpio;

    HDF_LOGE("%s: trigger north polarity irq rasing", __func__);

    struct HallDrvData *drvData = (struct HallDrvData *)data;
    CHECK_NULL_PTR_RETURN_VALUE(drvData, HDF_ERR_INVALID_PARAM);

    if (!HdfAddWork(&drvData->hallWorkQueue, &drvData->hallWork)) {
        HDF_LOGE("%s: hall add work queue failed", __func__);
    }

    return HDF_SUCCESS;
}

static int32_t HallSouthPolarityIrqFunc(uint16_t gpio, void *data)
{
    (void)gpio;

    HDF_LOGE("%s: trigger south polarity irq rasing", __func__);

    struct HallDrvData *drvData = (struct HallDrvData *)data;
    CHECK_NULL_PTR_RETURN_VALUE(drvData, HDF_ERR_INVALID_PARAM);

    if (!HdfAddWork(&drvData->hallWorkQueue, &drvData->hallWork)) {
        HDF_LOGE("%s: hall add work queue failed", __func__);
    }

    return HDF_SUCCESS;
}

static int32_t InitHallData(void)
{
    struct HallDrvData *drvData = HallGetDrvData();
    CHECK_NULL_PTR_RETURN_VALUE(drvData, HDF_ERR_INVALID_PARAM);

    if (drvData->initStatus) {
        return HDF_SUCCESS;
    }

    if (HdfWorkQueueInit(&drvData->hallWorkQueue, HDF_HALL_WORK_QUEUE_NAME) != HDF_SUCCESS) {
        HDF_LOGE("%s: hall init work queue failed", __func__);
        return HDF_FAILURE;
    }

    if (HdfWorkInit(&drvData->hallWork, HallDataWorkEntry, drvData) != HDF_SUCCESS) {
        HDF_LOGE("%s: hall create thread failed", __func__);
        return HDF_FAILURE;
    }

    drvData->interval = SENSOR_TIMER_MIN_TIME;
    drvData->initStatus = true;
    drvData->enable = false;

    return HDF_SUCCESS;
}

static int32_t SetHallEnable(void)
{
    int32_t ret;
    struct HallDrvData *drvData = HallGetDrvData();
    CHECK_NULL_PTR_RETURN_VALUE(drvData, HDF_ERR_INVALID_PARAM);

    if (drvData->enable) {
        HDF_LOGE("%s: hall sensor is enabled", __func__);
        return HDF_SUCCESS;
    }
    
    if (drvData->GpioIrq[HALL_NORTH_POLARITY_GPIO] >= 0) {
        ret = GpioSetIrq(drvData->GpioIrq[HALL_NORTH_POLARITY_GPIO], OSAL_IRQF_TRIGGER_FALLING, 
            HallNorthPolarityIrqFunc, drvData);
        if (ret != 0) {
            HDF_LOGE("gpio set north irq failed: %d", ret);
        }
        ret = GpioEnableIrq(drvData->GpioIrq[HALL_NORTH_POLARITY_GPIO]);
        if (ret != 0) {
            HDF_LOGE("gpio enable north irq failed: %d", ret);
        }
    }
    
    if (drvData->GpioIrq[HALL_SOUTH_POLARITY_GPIO] >= 0) {
        ret = GpioSetIrq(drvData->GpioIrq[HALL_SOUTH_POLARITY_GPIO], OSAL_IRQF_TRIGGER_FALLING, 
            HallSouthPolarityIrqFunc, drvData);
        if (ret != 0) {
            HDF_LOGE("%s: gpio set south irq failed", __func__);
        }
        ret = GpioEnableIrq(drvData->GpioIrq[HALL_SOUTH_POLARITY_GPIO]);
        if (ret != 0) {
            HDF_LOGE("%s: gpio enable south irq failed", __func__);
        }
    }

    drvData->enable = true;
    return HDF_SUCCESS;
}

static int32_t SetHallDisable(void)
{
    int32_t ret;
    struct HallDrvData *drvData = HallGetDrvData();
    CHECK_NULL_PTR_RETURN_VALUE(drvData, HDF_ERR_INVALID_PARAM);

    if (!drvData->enable) {
        HDF_LOGE("%s: hall sensor had disable", __func__);
        return HDF_SUCCESS;
    }

    if (drvData->GpioIrq[HALL_NORTH_POLARITY_GPIO] >= 0) {
        ret = GpioUnSetIrq(drvData->GpioIrq[HALL_NORTH_POLARITY_GPIO]);
        if (ret != 0) {
            HDF_LOGE("%s: gpio unset north irq failed", __func__);
        }
        ret = GpioDisableIrq(drvData->GpioIrq[HALL_NORTH_POLARITY_GPIO]);
        if (ret != 0) {
            HDF_LOGE("%s: gpio disable north irq failed", __func__);
        }
    }
    
    if (drvData->GpioIrq[HALL_SOUTH_POLARITY_GPIO] >= 0) {
        ret = GpioUnSetIrq(drvData->GpioIrq[HALL_SOUTH_POLARITY_GPIO]);
        if (ret != 0) {
            HDF_LOGE("%s: gpio unset south irq failed", __func__);
        }
        ret = GpioDisableIrq(drvData->GpioIrq[HALL_SOUTH_POLARITY_GPIO]);
        if (ret != 0) {
            HDF_LOGE("%s: gpio disable south irq failed", __func__);
        }
    }

    drvData->enable = false;
    return HDF_SUCCESS;
}

static int32_t SetHallBatch(int64_t samplingInterval, int64_t interval)
{
    return HDF_SUCCESS;
}

static int32_t SetHallMode(int32_t mode)
{
    return (mode == SENSOR_WORK_MODE_ON_CHANGE) ? HDF_SUCCESS : HDF_FAILURE;
}

static int32_t SetHallOption(uint32_t option)
{
    (void)option;
    return HDF_SUCCESS;
}

static int32_t DispatchHall(struct HdfDeviceIoClient *client,
    int cmd, struct HdfSBuf *data, struct HdfSBuf *reply)
{
    (void)client;
    (void)cmd;
    (void)data;
    (void)reply;

    return HDF_SUCCESS;
}

int32_t BindHallDriver(struct HdfDeviceObject *device)
{
    CHECK_NULL_PTR_RETURN_VALUE(device, HDF_ERR_INVALID_PARAM);
    HDF_LOGI("%s: hall sensor bind success", __func__);

    struct HallDrvData *drvData = (struct HallDrvData *)OsalMemCalloc(sizeof(*drvData));
    if (drvData == NULL) {
        HDF_LOGE("%s: malloc hall drv data fail!", __func__);
        return HDF_ERR_MALLOC_FAIL;
    }

    drvData->ioService.Dispatch = DispatchHall;
    drvData->device = device;
    device->service = &drvData->ioService;
    g_hallDrvData = drvData;
    return HDF_SUCCESS;
}

static int32_t InitHallOps(struct SensorDeviceInfo *deviceInfo)
{
    struct HallDrvData *drvData = HallGetDrvData();
    CHECK_NULL_PTR_RETURN_VALUE(drvData, HDF_ERR_INVALID_PARAM);

    deviceInfo->ops.Enable = SetHallEnable;
    deviceInfo->ops.Disable = SetHallDisable;
    deviceInfo->ops.SetBatch = SetHallBatch;
    deviceInfo->ops.SetMode = SetHallMode;
    deviceInfo->ops.SetOption = SetHallOption;

    if (memcpy_s(&deviceInfo->sensorInfo, sizeof(deviceInfo->sensorInfo),
        &drvData->hallCfg->sensorInfo, sizeof(drvData->hallCfg->sensorInfo)) != EOK) {
        HDF_LOGE("%s: copy sensor info failed", __func__);
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

static int32_t InitHallAfterConfig(void)
{
    struct SensorDeviceInfo deviceInfo;

    if (InitHallData() != HDF_SUCCESS) {
        HDF_LOGE("%s: init hall config failed", __func__);
        return HDF_FAILURE;
    } 

    if (InitHallOps(&deviceInfo) != HDF_SUCCESS) {
        HDF_LOGE("%s: init hall ops failed", __func__);
        return HDF_FAILURE;
    }

    if (AddSensorDevice(&deviceInfo) != HDF_SUCCESS) {
        HDF_LOGE("%s: add hall device failed", __func__);
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

static int32_t DetectHallChip(void)
{
    int32_t num;
    int32_t ret;
    int32_t loop;
    struct HallDrvData *drvData = HallGetDrvData();

    CHECK_NULL_PTR_RETURN_VALUE(drvData, HDF_ERR_INVALID_PARAM);
    CHECK_NULL_PTR_RETURN_VALUE(drvData->hallCfg, HDF_ERR_INVALID_PARAM);

    num = sizeof(g_hallDetectIfList) / sizeof(g_hallDetectIfList[0]);
    for (loop = 0; loop < num; ++loop) {
        if (g_hallDetectIfList[loop].DetectChip != NULL) {
            ret = g_hallDetectIfList[loop].DetectChip(drvData->hallCfg);
            if (ret == HDF_SUCCESS) {
                drvData->detectFlag = true;
                return HDF_SUCCESS;
            }
        }
    }

    HDF_LOGE("%s: detect hall device failed", __func__);
    drvData->detectFlag = false;
    return HDF_FAILURE;
}

static int32_t ParserHallPinConfigData(const struct DeviceResourceNode *node, struct HallDrvData *drvData)
{
    int32_t ret;
    struct DeviceResourceIface *parser = NULL;

    CHECK_NULL_PTR_RETURN_VALUE(node, HDF_ERR_INVALID_PARAM);
    CHECK_NULL_PTR_RETURN_VALUE(drvData, HDF_ERR_INVALID_PARAM);

    parser = DeviceResourceGetIfaceInstance(HDF_CONFIG_SOURCE);
    CHECK_NULL_PTR_RETURN_VALUE(parser, HDF_ERR_INVALID_PARAM);

    CHECK_NULL_PTR_RETURN_VALUE(parser->GetChildNode, HDF_ERR_INVALID_PARAM);
    
    const struct DeviceResourceNode *pinNode = parser->GetChildNode(node, "hallPinConfig");
    CHECK_NULL_PTR_RETURN_VALUE(pinNode, HDF_ERR_INVALID_PARAM);

    ret = parser->GetUint32(pinNode, "NorthPolarityGpio", (uint32_t *)&drvData->GpioIrq[HALL_NORTH_POLARITY_GPIO], 0);
    CHECK_PARSER_RESULT_RETURN_VALUE(ret, "NorthPolarityGpio");
    ret = parser->GetUint32(pinNode, "SouthPolarityGpio", (uint32_t *)&drvData->GpioIrq[HALL_SOUTH_POLARITY_GPIO], 0);
    CHECK_PARSER_RESULT_RETURN_VALUE(ret, "SouthPolarityGpio");

    return HDF_SUCCESS;
}

static int32_t SetHallGpioPin(const struct DeviceResourceNode *node, struct HallDrvData *drvData) 
{
    if (ParserHallPinConfigData(node, drvData) != HDF_SUCCESS) {
        HDF_LOGE("%s: get hall pin config failed", __func__);
        return HDF_FAILURE;
    }
    
    if (drvData->GpioIrq[HALL_NORTH_POLARITY_GPIO] >= 0) {
        int ret = GpioSetDir(drvData->GpioIrq[HALL_NORTH_POLARITY_GPIO], GPIO_DIR_IN);
        if (ret != 0) {
            HDF_LOGE("%s:%d set north gpio dir failed", __func__, __LINE__);
            return HDF_FAILURE;
        }
    }
    
    if (drvData->GpioIrq[HALL_SOUTH_POLARITY_GPIO] >= 0) {
        int ret = GpioSetDir(drvData->GpioIrq[HALL_SOUTH_POLARITY_GPIO], GPIO_DIR_IN);
        if (ret != 0) {
            HDF_LOGE("%s:%d south south gpio dir failed", __func__, __LINE__);
            return HDF_FAILURE;
        }
    }

    return HDF_SUCCESS;
}

int32_t InitHallDriver(struct HdfDeviceObject *device)
{
    struct HallDrvData *drvData = NULL;
    CHECK_NULL_PTR_RETURN_VALUE(device, HDF_ERR_INVALID_PARAM);
    HDF_LOGI("%s: hall sensor init success", __func__);
    drvData = (struct HallDrvData *)device->service;
    CHECK_NULL_PTR_RETURN_VALUE(drvData, HDF_ERR_INVALID_PARAM);

    if (drvData->detectFlag) {
        HDF_LOGE("%s: hall sensor have detected", __func__);
        return HDF_SUCCESS;
    }

    drvData->hallCfg = (struct SensorCfgData *)OsalMemCalloc(sizeof(*drvData->hallCfg));
    if (drvData->hallCfg == NULL) {
        HDF_LOGE("%s: malloc sensor config data failed", __func__);
        return HDF_FAILURE;
    }

    if (GetSensorBaseConfigData(device->property, drvData->hallCfg) != HDF_SUCCESS) {
        HDF_LOGE("%s: get sensor base config failed", __func__);
        goto BASE_CONFIG_EXIT;
    }

    if (SetHallGpioPin(device->property, drvData) != HDF_SUCCESS) {
        HDF_LOGE("%s: set hall gpio pin  failed", __func__);
        goto BASE_CONFIG_EXIT;
    }

    if (DetectHallChip() != HDF_SUCCESS) {
        HDF_LOGE("%s: hall sensor detect device no exist", __func__);
        goto DETECT_CHIP_EXIT;
    }
    drvData->detectFlag = true;

    if (InitHallAfterConfig() != HDF_SUCCESS) {
        HDF_LOGE("%s: init hall after config failed", __func__);
        goto INIT_EXIT;
    }

    HDF_LOGI("%s: init hall driver success", __func__);
    return HDF_SUCCESS;

INIT_EXIT:
    (void)DeleteSensorDevice(&drvData->hallCfg->sensorInfo);
DETECT_CHIP_EXIT:
    drvData->detectFlag = false;
BASE_CONFIG_EXIT:
    drvData->hallCfg->root = NULL;
    OsalMemFree(drvData->hallCfg);
    drvData->hallCfg = NULL;
    return HDF_FAILURE;
}

void ReleaseHallDriver(struct HdfDeviceObject *device)
{
    struct HallDrvData *drvData = NULL;
    CHECK_NULL_PTR_RETURN(device);
    HDF_LOGI("%s: hall sensor release success", __func__);

    drvData = (struct HallDrvData *)device->service;
    CHECK_NULL_PTR_RETURN(drvData);

    (void)DeleteSensorDevice(&drvData->hallCfg->sensorInfo);
    drvData->detectFlag = false;

    if (drvData->hallCfg != NULL) {
        drvData->hallCfg->root = NULL;
        OsalMemFree(drvData->hallCfg);
        drvData->hallCfg = NULL;
    }

    drvData->initStatus = false;
}

struct HdfDriverEntry g_sensorHallDevEntry = {
    .moduleVersion = 1,
    .moduleName = "HDF_SENSOR_HALL",
    .Bind = BindHallDriver,
    .Init = InitHallDriver,
    .Release = ReleaseHallDriver,
};

HDF_INIT(g_sensorHallDevEntry);

