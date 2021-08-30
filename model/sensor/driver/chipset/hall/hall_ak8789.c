/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "hall_ak8789.h"
#include <securec.h>
#include "osal_irq.h"
#include "osal_time.h"
#include "sensor_config_controller.h"
#include "sensor_device_manager.h"
#include "sensor_hall_driver.h"

/* IO config for int-pin and Gpio-pin */
#define SENSOR_HALL_DATA_REG_ADDR 0x114f0040
#define SENSOR_HALL_CLK_REG_ADDR  0x114f0044
#define SENSOR_HALL_REG_CFG       0x400

int32_t ReadAk8789Data(struct SensorCfgData *data)
{
    int32_t ret;  
    uint8_t tmp = 1;
    OsalTimespec time;
    struct SensorReportEvent event;

    CHECK_NULL_PTR_RETURN_VALUE(data, HDF_ERR_INVALID_PARAM);

    (void)memset_s(&event, sizeof(event), 0, sizeof(event));
    (void)memset_s(&time, sizeof(time), 0, sizeof(time));
    if (OsalGetTime(&time) != HDF_SUCCESS) {      
        HDF_LOGE("%s: Get time failed", __func__);
        return HDF_FAILURE;
    }
    
    event.timestamp = time.sec * SENSOR_SECOND_CONVERT_NANOSECOND + time.usec *
        SENSOR_CONVERT_UNIT; /* unit nanosecond */
    event.sensorId = SENSOR_TAG_HALL;
    event.version = 0;
    event.option = 0;                     
    event.mode = SENSOR_WORK_MODE_ON_CHANGE;
    event.dataLen = sizeof(tmp);       
    event.data = (uint8_t *)&tmp;    
    ret = ReportSensorEvent(&event);  
    return ret;
}

static int32_t InitHallPreConfig(void)
{
    if (SetSensorPinMux(SENSOR_HALL_DATA_REG_ADDR, SENSOR_ADDR_WIDTH_4_BYTE, SENSOR_HALL_REG_CFG) != HDF_SUCCESS) {
        HDF_LOGE("%s: Data write mux pin failed", __func__);
        return HDF_FAILURE;
    }
    if (SetSensorPinMux(SENSOR_HALL_CLK_REG_ADDR, SENSOR_ADDR_WIDTH_4_BYTE, SENSOR_HALL_REG_CFG) != HDF_SUCCESS) {
        HDF_LOGE("%s: Clk write mux pin failed", __func__);
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t DetectHallAk8789Chip(struct SensorCfgData *data)
{
    int32_t ret;
    struct HallOpsCall ops;

    CHECK_NULL_PTR_RETURN_VALUE(data, HDF_ERR_INVALID_PARAM);

    if (strcmp(HALL_CHIP_NAME_AK8789, data->sensorAttr.chipName) != 0) {
        return HDF_SUCCESS;
    }
    ret = InitHallPreConfig();
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: init  AK8789 bus mux config", __func__);
        return HDF_FAILURE;
    }
    ops.Init = NULL;
    ops.ReadData = ReadAk8789Data;
    ret = RegisterHallChipOps(&ops);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: register AK8789 hall failed", __func__);
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}