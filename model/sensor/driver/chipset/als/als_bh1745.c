/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "als_bh1745.h"
#include <securec.h>
#include "osal_mem.h"
#include "osal_time.h"
#include "sensor_als_driver.h"
#include "sensor_config_controller.h"
#include "sensor_device_manager.h"

static struct Bh1745DrvData *g_bh1745DrvData = NULL;

struct Bh1745DrvData *Bh1745GetDrvData(void)
{
    return g_bh1745DrvData;
}

/* IO config for int-pin and I2C-pin */
#define SENSOR_I2C6_DATA_REG_ADDR 0x114f004c
#define SENSOR_I2C6_CLK_REG_ADDR  0x114f0048
#define SENSOR_I2C_REG_CFG        0x403

static int32_t ReadBh1745RawData(struct SensorCfgData *data, struct AlsData *rawData, int64_t *timestamp)
{
    uint8_t status = 0;
    uint8_t reg[ALS_LIGHT_BUTT];
    OsalTimespec time;

    (void)memset_s(&time, sizeof(time), 0, sizeof(time));
    (void)memset_s(reg, sizeof(reg), 0, sizeof(reg));

    CHECK_NULL_PTR_RETURN_VALUE(data, HDF_ERR_INVALID_PARAM);

    if (OsalGetTime(&time) != HDF_SUCCESS) {
        HDF_LOGE("%s: Get time failed", __func__);
        return HDF_FAILURE;
    }
    *timestamp = time.sec * SENSOR_SECOND_CONVERT_NANOSECOND + time.usec * SENSOR_CONVERT_UNIT; /* unit nanosecond */

    int32_t ret = ReadSensor(&data->busCfg, BH1745_MODECONTROL3_ADDR, &status, sizeof(uint8_t));
    if (!(status & BH1745_ALS_DATA_READY_MASK) || (ret != HDF_SUCCESS)) {
        HDF_LOGE("%s: data status [%u] ret [%d]", __func__, status, ret);
        return HDF_FAILURE;
    }

    ret = ReadSensor(&data->busCfg, BH1745_ALS_R_LSB_ADDR, &reg[ALS_R_LSB], sizeof(uint8_t));
    CHECK_PARSER_RESULT_RETURN_VALUE(ret, "read data");

    ret = ReadSensor(&data->busCfg, BH1745_ALS_R_MSB_ADDR, &reg[ALS_R_MSB], sizeof(uint8_t));
    CHECK_PARSER_RESULT_RETURN_VALUE(ret, "read data");

    ret = ReadSensor(&data->busCfg, BH1745_ALS_G_LSB_ADDR, &reg[ALS_G_LSB], sizeof(uint8_t));
    CHECK_PARSER_RESULT_RETURN_VALUE(ret, "read data");

    ret = ReadSensor(&data->busCfg, BH1745_ALS_G_MSB_ADDR, &reg[ALS_G_MSB], sizeof(uint8_t));
    CHECK_PARSER_RESULT_RETURN_VALUE(ret, "read data");

    ret = ReadSensor(&data->busCfg, BH1745_ALS_B_LSB_ADDR, &reg[ALS_B_LSB], sizeof(uint8_t));
    CHECK_PARSER_RESULT_RETURN_VALUE(ret, "read data");

    ret = ReadSensor(&data->busCfg, BH1745_ALS_B_MSB_ADDR, &reg[ALS_B_MSB], sizeof(uint8_t));
    CHECK_PARSER_RESULT_RETURN_VALUE(ret, "read data");

    ret = ReadSensor(&data->busCfg, BH1745_ALS_C_LSB_ADDR, &reg[ALS_C_LSB], sizeof(uint8_t));
    CHECK_PARSER_RESULT_RETURN_VALUE(ret, "read data");

    ret = ReadSensor(&data->busCfg, BH1745_ALS_C_MSB_ADDR, &reg[ALS_C_MSB], sizeof(uint8_t));
    CHECK_PARSER_RESULT_RETURN_VALUE(ret, "read data");

    rawData->red = (int16_t)(SENSOR_DATA_SHIFT_LEFT(reg[ALS_R_MSB], SENSOR_DATA_WIDTH_8_BIT) |
        reg[ALS_R_LSB]);
    rawData->green = (int16_t)(SENSOR_DATA_SHIFT_LEFT(reg[ALS_G_MSB], SENSOR_DATA_WIDTH_8_BIT) |
        reg[ALS_G_LSB]);
    rawData->blue = (int16_t)(SENSOR_DATA_SHIFT_LEFT(reg[ALS_B_MSB], SENSOR_DATA_WIDTH_8_BIT) |
        reg[ALS_B_LSB]);
    rawData->clear = (int16_t)(SENSOR_DATA_SHIFT_LEFT(reg[ALS_C_MSB], SENSOR_DATA_WIDTH_8_BIT) |
        reg[ALS_C_LSB]);

    return HDF_SUCCESS;
}

int32_t ReadBh1745Data(struct SensorCfgData *data)
{
    int32_t ret;
    struct AlsData rawData = { 0, 0, 0, 0 };
    int32_t tmp[ALS_LIGHT_NUM];
    struct SensorReportEvent event;

    (void)memset_s(&event, sizeof(event), 0, sizeof(event));
    ret = ReadBh1745RawData(data, &rawData, &event.timestamp);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: BH1745 read raw data failed", __func__);
        return HDF_FAILURE;
    }

    event.sensorId = SENSOR_TAG_AMBIENT_LIGHT;
    event.option = 0;
    event.mode = SENSOR_WORK_MODE_REALTIME;

    tmp[ALS_R] = rawData.red;
    tmp[ALS_G] = rawData.green;
    tmp[ALS_B] = rawData.blue;
    tmp[ALS_C] = rawData.clear;

    event.dataLen = sizeof(tmp);
    event.data = (uint8_t *)&tmp;
    ret = ReportSensorEvent(&event);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: BH1745 report data failed", __func__);
    }
    return ret;
}

static int32_t InitBh1745(struct SensorCfgData *data)
{
    int32_t ret;

    CHECK_NULL_PTR_RETURN_VALUE(data, HDF_ERR_INVALID_PARAM);
    ret = SetSensorRegCfgArray(&data->busCfg, data->regCfgGroup[SENSOR_INIT_GROUP]);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: BH1745 sensor init config failed", __func__);
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

static int32_t InitAlsPreConfig(void)
{
    if (SetSensorPinMux(SENSOR_I2C6_DATA_REG_ADDR, SENSOR_ADDR_WIDTH_4_BYTE, SENSOR_I2C_REG_CFG) != HDF_SUCCESS) {
        HDF_LOGE("%s: Data write mux pin failed", __func__);
        return HDF_FAILURE;
    }
    if (SetSensorPinMux(SENSOR_I2C6_CLK_REG_ADDR, SENSOR_ADDR_WIDTH_4_BYTE, SENSOR_I2C_REG_CFG) != HDF_SUCCESS) {
        HDF_LOGE("%s: Clk write mux pin failed", __func__);
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

static int32_t DispatchBH1745(struct HdfDeviceIoClient *client,
    int cmd, struct HdfSBuf *data, struct HdfSBuf *reply)
{
    (void)client;
    (void)cmd;
    (void)data;
    (void)reply;

    return HDF_SUCCESS;
}

int32_t Bh1745BindDriver(struct HdfDeviceObject *device)
{
    CHECK_NULL_PTR_RETURN_VALUE(device, HDF_ERR_INVALID_PARAM);

    struct Bh1745DrvData *drvData = (struct Bh1745DrvData *)OsalMemCalloc(sizeof(*drvData));
    if (drvData == NULL) {
        HDF_LOGE("%s: Malloc Bh1745 drv data fail", __func__);
        return HDF_ERR_MALLOC_FAIL;
    }

    drvData->ioService.Dispatch = DispatchBH1745;
    drvData->device = device;
    device->service = &drvData->ioService;
    g_bh1745DrvData = drvData;

    return HDF_SUCCESS;
}

int32_t Bh1745InitDriver(struct HdfDeviceObject *device)
{
    int32_t ret;
    struct AlsOpsCall ops;

    CHECK_NULL_PTR_RETURN_VALUE(device, HDF_ERR_INVALID_PARAM);
    struct Bh1745DrvData *drvData = (struct Bh1745DrvData *)device->service;
    CHECK_NULL_PTR_RETURN_VALUE(drvData, HDF_ERR_INVALID_PARAM);

    ret = InitAlsPreConfig();
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: Init  BH1745 bus mux config", __func__);
        return HDF_FAILURE;
    }

    drvData->sensorCfg = AlsCreateCfgData(device->property);
    if (drvData->sensorCfg == NULL) {
        return HDF_ERR_NOT_SUPPORT;
    }

    ops.Init = NULL;
    ops.ReadData = ReadBh1745Data;
    ret = AlsRegisterChipOps(&ops);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: Register BH1745 als failed", __func__);
        return HDF_FAILURE;
    }

    ret = InitBh1745(drvData->sensorCfg);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: Init BH1745 als failed", __func__);
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

void Bh1745ReleaseDriver(struct HdfDeviceObject *device)
{
    CHECK_NULL_PTR_RETURN(device);

    struct Bh1745DrvData *drvData = (struct Bh1745DrvData *)device->service;
    CHECK_NULL_PTR_RETURN(drvData);

    AlsReleaseCfgData(drvData->sensorCfg);
    drvData->sensorCfg = NULL;
    OsalMemFree(drvData);
}

struct HdfDriverEntry g_alsBh1745DevEntry = {
    .moduleVersion = 1,
    .moduleName = "HDF_SENSOR_ALS_BH1745",
    .Bind = Bh1745BindDriver,
    .Init = Bh1745InitDriver,
    .Release = Bh1745ReleaseDriver,
};

HDF_INIT(g_alsBh1745DevEntry);
