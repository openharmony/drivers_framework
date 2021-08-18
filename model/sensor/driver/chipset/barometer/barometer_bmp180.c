/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "securec.h"
#include "barometer_bmp180.h"
#include "osal_time.h"
#include "sensor_barometer_driver.h"
#include "sensor_config_controller.h"
#include "sensor_device_manager.h"
#include "sensor_platform_if.h"
#include "sensor_config_controller.h"
//#include <math.h>

/* IO config for int-pin and I2C-pin */
#define SENSOR_I2C6_DDATA_REG_ADDR 0x114f004c
#define SENSOR_I2C6_CLK_REG_ADDR  0x114f0048
#define SENSOR_I2C_REG_CFG        0x403

static struct EepromData CalibraData={0,0,0,0,0,0,0,0,0,0,0};

static int32_t ReadEepromData(struct SensorCfgData *data, struct EepromData *CalibraData)
{
    int32_t ret;
    uint8_t reg[BAROMETER_EEPROM_SUM];

    (void)memset_s(reg, sizeof(reg), 0, sizeof(reg));

    CHECK_NULL_PTR_RETURN_VALUE(data, HDF_ERR_INVALID_PARAM);

    ret = ReadSensor(&data->busCfg, BMP180_AC1_MSB_ADDR, &reg[BAROMETER_AC1_MSB], sizeof(uint8_t));
    CHECK_PARSER_RESULT_RETURN_VALUE(ret, "read data");

    ret = ReadSensor(&data->busCfg, BMP180_AC1_LSB_ADDR, &reg[BAROMETER_AC1_LSB], sizeof(uint8_t));
    CHECK_PARSER_RESULT_RETURN_VALUE(ret, "read data");

    ret = ReadSensor(&data->busCfg, BMP180_AC2_MSB_ADDR, &reg[BAROMETER_AC2_MSB], sizeof(uint8_t));
    CHECK_PARSER_RESULT_RETURN_VALUE(ret, "read data");

    ret = ReadSensor(&data->busCfg, BMP180_AC2_LSB_ADDR, &reg[BAROMETER_AC2_LSB], sizeof(uint8_t));
    CHECK_PARSER_RESULT_RETURN_VALUE(ret, "read data");

    ret = ReadSensor(&data->busCfg, BMP180_AC3_MSB_ADDR, &reg[BAROMETER_AC3_MSB], sizeof(uint8_t));
    CHECK_PARSER_RESULT_RETURN_VALUE(ret, "read data");

    ret = ReadSensor(&data->busCfg, BMP180_AC3_LSB_ADDR, &reg[BAROMETER_AC3_LSB], sizeof(uint8_t));
    CHECK_PARSER_RESULT_RETURN_VALUE(ret, "read data");

    ret = ReadSensor(&data->busCfg, BMP180_AC4_MSB_ADDR, &reg[BAROMETER_AC4_MSB], sizeof(uint8_t));
    CHECK_PARSER_RESULT_RETURN_VALUE(ret, "read data");
    
    ret = ReadSensor(&data->busCfg, BMP180_AC4_LSB_ADDR, &reg[BAROMETER_AC4_LSB], sizeof(uint8_t));
    CHECK_PARSER_RESULT_RETURN_VALUE(ret, "read data");

    ret = ReadSensor(&data->busCfg, BMP180_AC5_MSB_ADDR, &reg[BAROMETER_AC5_MSB], sizeof(uint8_t));
    CHECK_PARSER_RESULT_RETURN_VALUE(ret, "read data");

    ret = ReadSensor(&data->busCfg, BMP180_AC5_LSB_ADDR, &reg[BAROMETER_AC5_LSB], sizeof(uint8_t));
    CHECK_PARSER_RESULT_RETURN_VALUE(ret, "read data");

    ret = ReadSensor(&data->busCfg, BMP180_AC6_MSB_ADDR, &reg[BAROMETER_AC6_MSB], sizeof(uint8_t));
    CHECK_PARSER_RESULT_RETURN_VALUE(ret, "read data");
    ret = ReadSensor(&data->busCfg, BMP180_AC6_LSB_ADDR, &reg[BAROMETER_AC6_LSB], sizeof(uint8_t));
    CHECK_PARSER_RESULT_RETURN_VALUE(ret, "read data");

    ret = ReadSensor(&data->busCfg, BMP180_B1_MSB_ADDR, &reg[BAROMETER_B1_MSB], sizeof(uint8_t));
    CHECK_PARSER_RESULT_RETURN_VALUE(ret, "read data");

    ret = ReadSensor(&data->busCfg, BMP180_B1_LSB_ADDR, &reg[BAROMETER_B1_LSB], sizeof(uint8_t));
    CHECK_PARSER_RESULT_RETURN_VALUE(ret, "read data");

    ret = ReadSensor(&data->busCfg, BMP180_B2_MSB_ADDR, &reg[BAROMETER_B2_MSB], sizeof(uint8_t));
    CHECK_PARSER_RESULT_RETURN_VALUE(ret, "read data");

    ret = ReadSensor(&data->busCfg, BMP180_B2_LSB_ADDR, &reg[BAROMETER_B2_LSB], sizeof(uint8_t));
    CHECK_PARSER_RESULT_RETURN_VALUE(ret, "read data");

    ret = ReadSensor(&data->busCfg, BMP180_MB_MSB_ADDR, &reg[BAROMETER_MB_MSB], sizeof(uint8_t));
    CHECK_PARSER_RESULT_RETURN_VALUE(ret, "read data");

    ret = ReadSensor(&data->busCfg, BMP180_MB_LSB_ADDR, &reg[BAROMETER_MB_LSB], sizeof(uint8_t));
    CHECK_PARSER_RESULT_RETURN_VALUE(ret, "read data");
    
    ret = ReadSensor(&data->busCfg, BMP180_MC_MSB_ADDR, &reg[BAROMETER_MC_MSB], sizeof(uint8_t));
    CHECK_PARSER_RESULT_RETURN_VALUE(ret, "read data");

    ret = ReadSensor(&data->busCfg, BMP180_MC_LSB_ADDR, &reg[BAROMETER_MC_LSB], sizeof(uint8_t));
    CHECK_PARSER_RESULT_RETURN_VALUE(ret, "read data");

    ret = ReadSensor(&data->busCfg, BMP180_MD_MSB_ADDR, &reg[BAROMETER_MD_MSB], sizeof(uint8_t));
    CHECK_PARSER_RESULT_RETURN_VALUE(ret, "read data");

    ret = ReadSensor(&data->busCfg, BMP180_MD_LSB_ADDR, &reg[BAROMETER_MD_LSB], sizeof(uint8_t));
    CHECK_PARSER_RESULT_RETURN_VALUE(ret, "read data");

    CalibraData->ac1 = (int16_t)(SENSOR_DATA_SHIFT_LEFT(reg[BAROMETER_AC1_MSB], SENSOR_DATA_WIDTH_8_BIT) |
    reg[BAROMETER_AC1_LSB]);
    CalibraData->ac2 = (int16_t)(SENSOR_DATA_SHIFT_LEFT(reg[BAROMETER_AC2_MSB], SENSOR_DATA_WIDTH_8_BIT) |
    reg[BAROMETER_AC2_LSB]);
    CalibraData->ac3 = (int16_t)(SENSOR_DATA_SHIFT_LEFT(reg[BAROMETER_AC3_MSB], SENSOR_DATA_WIDTH_8_BIT) |
    reg[BAROMETER_AC3_LSB]);
    CalibraData->ac4 = (uint16_t)(SENSOR_DATA_SHIFT_LEFT(reg[BAROMETER_AC4_MSB], SENSOR_DATA_WIDTH_8_BIT) |
    reg[BAROMETER_AC4_LSB]);
    CalibraData->ac5 = (uint16_t)(SENSOR_DATA_SHIFT_LEFT(reg[BAROMETER_AC5_MSB], SENSOR_DATA_WIDTH_8_BIT) |
    reg[BAROMETER_AC5_LSB]);
    CalibraData->ac6 = (uint16_t)(SENSOR_DATA_SHIFT_LEFT(reg[BAROMETER_AC6_MSB], SENSOR_DATA_WIDTH_8_BIT) |
    reg[BAROMETER_AC6_LSB]);
    CalibraData->b1 = (int16_t)(SENSOR_DATA_SHIFT_LEFT(reg[BAROMETER_B1_MSB], SENSOR_DATA_WIDTH_8_BIT) |
    reg[BAROMETER_B1_LSB]);
    CalibraData->b2 = (int16_t)(SENSOR_DATA_SHIFT_LEFT(reg[BAROMETER_B2_MSB], SENSOR_DATA_WIDTH_8_BIT) |
    reg[BAROMETER_B2_LSB]);
    CalibraData->mb = (int16_t)(SENSOR_DATA_SHIFT_LEFT(reg[BAROMETER_MB_MSB], SENSOR_DATA_WIDTH_8_BIT) |
    reg[BAROMETER_MB_LSB]);
    CalibraData->mc = (int16_t)(SENSOR_DATA_SHIFT_LEFT(reg[BAROMETER_MC_MSB], SENSOR_DATA_WIDTH_8_BIT) |
    reg[BAROMETER_MC_LSB]);
    CalibraData->md = (int16_t)(SENSOR_DATA_SHIFT_LEFT(reg[BAROMETER_MD_MSB], SENSOR_DATA_WIDTH_8_BIT) |
    reg[BAROMETER_MD_LSB]);

    return ret;
}

static int32_t ReadTempData(struct SensorCfgData *data,  struct TempData *Temp)
{
    int32_t ret;
    uint8_t status = 0;
    uint8_t reg[BAROMETER_TEM_SUM];
    uint8_t value[SENSOR_VALUE_BUTT];
    value[SENSOR_ADDR_INDEX] = 0xF4;
    value[SENSOR_VALUE_INDEX] = 0x2E;

    (void)memset_s(reg, sizeof(reg), 0, sizeof(reg));

    CHECK_NULL_PTR_RETURN_VALUE(data, HDF_ERR_INVALID_PARAM);

    ret = ReadSensor(&data->busCfg, BMP180_COVERT_PRES_3, &status, sizeof(uint8_t));
    if ((status & BMP180_STATUS_ADDR) == 0x00) {
        WriteSensor(&data->busCfg, value,sizeof(value));
        OsalMDelay(5);
        ret = ReadSensor(&data->busCfg, BMP180_OUT_MSB_ADDR, &reg[BAROMETER_TEM_MSB], sizeof(uint8_t));
        CHECK_PARSER_RESULT_RETURN_VALUE(ret, "read data");

        ret = ReadSensor(&data->busCfg, BMP180_OUT_LSB_ADDR, &reg[BAROMETER_TEM_LSB], sizeof(uint8_t));
        CHECK_PARSER_RESULT_RETURN_VALUE(ret, "read data");

        Temp-> U_Temperature = (int32_t)(SENSOR_DATA_SHIFT_LEFT(reg[BAROMETER_TEM_MSB], SENSOR_DATA_WIDTH_8_BIT) |
        reg[BAROMETER_TEM_LSB]);
    }
    return ret;
}

static int32_t ReadBarometerData(struct SensorCfgData *data, struct BarData *Barom)
{

    int32_t ret;
    uint8_t status = 0;
    uint8_t reg[BAROMETER_BAR_SUM];
    uint8_t value[SENSOR_VALUE_BUTT];
    value[SENSOR_ADDR_INDEX] = 0xF4;
    value[SENSOR_VALUE_INDEX] = 0x74;

    (void)memset_s(reg, sizeof(reg), 0, sizeof(reg));

    CHECK_NULL_PTR_RETURN_VALUE(data, HDF_ERR_INVALID_PARAM);

    ret = ReadSensor(&data->busCfg, BMP180_COVERT_PRES_3, &status, sizeof(uint8_t));
    if ((status & BMP180_STATUS_ADDR) == 0x00) {
    WriteSensor(&data->busCfg,value,sizeof(value));
    OsalMDelay(8);
    ret = ReadSensor(&data->busCfg, BMP180_OUT_MSB_ADDR, &reg[BAROMETER_BAR_MSB], sizeof(uint8_t));
    CHECK_PARSER_RESULT_RETURN_VALUE(ret, "read data");

    ret = ReadSensor(&data->busCfg, BMP180_OUT_LSB_ADDR, &reg[BAROMETER_BAR_LSB], sizeof(uint8_t));
    CHECK_PARSER_RESULT_RETURN_VALUE(ret, "read data");

    ret = ReadSensor(&data->busCfg, BMP180_OUT_XLSB_ADDR, &reg[BAROMETER_BAR_XLSB], sizeof(uint8_t));
    CHECK_PARSER_RESULT_RETURN_VALUE(ret, "read data");

    Barom->U_Pressure = (int32_t)(SENSOR_DATA_SHIFT_RIGHT((SENSOR_DATA_SHIFT_LEFT(reg[BAROMETER_BAR_MSB], SENSOR_DATA_WIDTH_16_BIT) |
    SENSOR_DATA_SHIFT_LEFT(reg[BAROMETER_BAR_LSB] , SENSOR_DATA_WIDTH_8_BIT) |
    reg[BAROMETER_BAR_XLSB]),(8-OSSETTING)));
    }

    return ret;
}

int32_t ReadBmp180Data(struct SensorCfgData *data)
{
    int32_t ret;
    int32_t tmp[BAROMETER_SUM];
    struct TempData TemperatureData={ 0 };
    struct BarData BarometerData= { 0 };
    struct Coefficient CoefficientData = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    OsalTimespec time;
    struct SensorReportEvent event;

    (void)memset_s(&time, sizeof(time), 0, sizeof(time));
    (void)memset_s(&event, sizeof(event), 0, sizeof(event));

    if (OsalGetTime(&time) != HDF_SUCCESS) {
        HDF_LOGE("%s: Get time failed", __func__);
        return HDF_FAILURE;
    }
    event.timestamp = time.sec * SENSOR_SECOND_CONVERT_NANOSECOND + time.usec * SENSOR_CONVERT_UNIT; /* unit nanosecond */

    ret = ReadTempData(data, &TemperatureData);
    if (ret != HDF_SUCCESS) {
    return HDF_FAILURE;
    }

    ret = ReadBarometerData(data, &BarometerData);
    if (ret != HDF_SUCCESS) {
    return HDF_FAILURE;
    }

    event.sensorId = SENSOR_TAG_BAROMETER;
    event.option = 0;
    event.mode = SENSOR_WORK_MODE_REALTIME;

    // Calculated temperature 
	CoefficientData.x1 = ((TemperatureData.U_Temperature - CalibraData.ac6)*(CalibraData.ac5)) >> 15;
  	CoefficientData.x2 = (CalibraData.mc << 11) / (CoefficientData.x1 + CalibraData.md);
  	CoefficientData.b5 = CoefficientData.x1 + CoefficientData.x2;
    tmp[BAROMETER_TEMPERATURE] = (CoefficientData.b5 + 8) >> 4;
	//Calculated pressure
	CoefficientData.b6 = CoefficientData.b5 - 4000;
	CoefficientData.x1 = (CalibraData.b2 * ((CoefficientData.b6 * CoefficientData.b6)>>12))>>11;
	CoefficientData.x2 = (CalibraData.ac2 * CoefficientData.b6)>>11;
	CoefficientData.x3 = CoefficientData.x1 + CoefficientData.x2;
	CoefficientData.b3 = (((((int32_t)CalibraData.ac1)*4 + CoefficientData.x3)<<OSSETTING) + 2)>>2;
	CoefficientData.x1 = (CalibraData.ac3 * CoefficientData.b6)>>13;
	CoefficientData.x2 = (CalibraData.b1 * ((CoefficientData.b6 * CoefficientData.b6)>>12))>>16;
	CoefficientData.x3 = ((CoefficientData.x1 + CoefficientData.x2) + 2)>>2;
	CoefficientData.b4 = (CalibraData.ac4 * (uint32_t)(CoefficientData.x3 + 32768))>>15;
	CoefficientData.b7 = ((uint32_t)BarometerData.U_Pressure - (uint32_t)CoefficientData.b3) * (50000>>OSSETTING);
	if (CoefficientData.b7 < 0x80000000)
	{
		CoefficientData.p = (CoefficientData.b7<<1)/CoefficientData.b4;
	}
	else
	{
		CoefficientData.p = (CoefficientData.b7/CoefficientData.b4)<<1;
	}
	CoefficientData.x1 = (CoefficientData.p>>8) * (CoefficientData.p>>8);
	CoefficientData.x1 = (CoefficientData.x1 * 3038)>>16;
	CoefficientData.x2 = (-7357 * CoefficientData.p)>>16;
	tmp[BAROMETER_BAROMETER] = CoefficientData.p+((CoefficientData.x1 + CoefficientData.x2 + 3791)>>4);	
 
    event.dataLen = sizeof(tmp);
    event.data = (uint8_t *)&tmp;
    ret = ReportSensorEvent(&event);
    return ret;
}

static int32_t InitBmp180(struct SensorCfgData *data)
{
    int32_t ret;
    CHECK_NULL_PTR_RETURN_VALUE(data, HDF_ERR_INVALID_PARAM);
    struct SensorReportEvent event;

    (void)memset_s(&event, sizeof(event), 0, sizeof(event));

    ret = ReadEepromData(data, &CalibraData);
    if (ret != HDF_SUCCESS) {
        return HDF_FAILURE;
    }
    ret = SetSensorRegCfgArray(&data->busCfg, data->regCfgGroup[SENSOR_INIT_GROUP]);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: BMP180 sensor init config failed", __func__);
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

static int32_t InitBarometerPreConfig(void)
{
    if (SetSensorPinMux(SENSOR_I2C6_DDATA_REG_ADDR, SENSOR_ADDR_WIDTH_4_BYTE, SENSOR_I2C_REG_CFG) != HDF_SUCCESS) {
        HDF_LOGE("%s: Data write mux pin failed", __func__);
        return HDF_FAILURE;
    }
    if (SetSensorPinMux(SENSOR_I2C6_CLK_REG_ADDR, SENSOR_ADDR_WIDTH_4_BYTE, SENSOR_I2C_REG_CFG) != HDF_SUCCESS) {
        HDF_LOGE("%s: Clk write mux pin failed", __func__);
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t DetectBarometerBmp180Chip(struct SensorCfgData *data)
{ 
    int32_t ret;
    struct BarometerOpsCall ops;
    CHECK_NULL_PTR_RETURN_VALUE(data, HDF_ERR_INVALID_PARAM);

    if (strcmp(BAROMETER_CHIP_NAME_BMP180, data->sensorAttr.chipName) != 0) {
        return HDF_SUCCESS;
    }
    ret = InitBarometerPreConfig();
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: init  bmp180 bus mux config", __func__);
        return HDF_FAILURE;
    }
    if (DetectSensorDevice(data) != HDF_SUCCESS) {
        return HDF_FAILURE;
    }
    ops.Init = InitBmp180;
    ops.ReadData = ReadBmp180Data;
    ret = RegisterBarometerChipOps(&ops);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: register bmp180 barometer failed", __func__);
        (void)ReleaseSensorBusHandle(&data->busCfg);
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}