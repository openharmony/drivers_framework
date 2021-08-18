/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef BAROMETER_BMP180_H
#define BAROMETER_BMP180_H

#include "sensor_config_parser.h"

#define BMP180_REG_CHIP_ID		   0xD0

#define BMP180_ADDR				   0x77 	//i2c slave address 

//Define calibration register address
#define BMP180_AC1_MSB_ADDR        0xAA
#define BMP180_AC1_LSB_ADDR        0xAB
#define BMP180_AC2_MSB_ADDR        0xAC
#define BMP180_AC2_LSB_ADDR        0xAD
#define BMP180_AC3_MSB_ADDR        0xAE 
#define BMP180_AC3_LSB_ADDR        0xAF 
#define BMP180_AC4_MSB_ADDR        0xB0
#define BMP180_AC4_LSB_ADDR        0xB1
#define BMP180_AC5_MSB_ADDR        0xB2
#define BMP180_AC5_LSB_ADDR        0xB3
#define BMP180_AC6_MSB_ADDR        0xB4
#define BMP180_AC6_LSB_ADDR        0xB5
#define BMP180_B1_MSB_ADDR         0xB6
#define BMP180_B1_LSB_ADDR         0xB7
#define BMP180_B2_MSB_ADDR         0xB8 
#define BMP180_B2_LSB_ADDR         0xB9
#define BMP180_MB_MSB_ADDR         0xBA
#define BMP180_MB_LSB_ADDR         0xBB
#define BMP180_MC_MSB_ADDR         0xBC
#define BMP180_MC_LSB_ADDR         0xBD
#define BMP180_MD_MSB_ADDR         0xBE
#define BMP180_MD_LSB_ADDR         0xBf

//Control register
#define BMP180_CONTROL_REG_ADDR    0xF4    
#define BMP180_COVERT_TEMP         0x2E    //Conversion temperature 4.5ms
#define BMP180_COVERT_PRES_0       0x34    //Conversion atmospheric pressure 4.5ms
#define BMP180_COVERT_PRES_1       0x74    //Conversion atmospheric pressure 7.5ms
#define BMP180_COVERT_PRES_2       0xB4    //Conversion atmospheric pressure 13.5ms 
#define BMP180_COVERT_PRES_3       0xF4    //Convert atmospheric pressure 25.5ms

#define BMP180_OUT_MSB_ADDR		   0xF6    // ADC output high 8 bits
#define BMP180_OUT_LSB_ADDR		   0xF7    // ADC output low 8 bits
#define BMP180_OUT_XLSB_ADDR	   0xF8    // During 19 bit measurement, the ADC output is at least 3 bits

#define BMP180_STATUS_ADDR         0X20

#define SENSOR_DATA_WIDTH_16_BIT   16 // 8 bit
#define OSS_TIME_MS	               26 // Time interval
#define OSSETTING                  1	//Atmospheric pressure conversion time
#define PRESSURE_OF_SEA			   101325.0f	// Reference sea level pressure




int32_t DetectBarometerBmp180Chip(struct SensorCfgData *data);
int32_t ReadBmp180Data(struct SensorCfgData *data);

#endif /*  BAROMETER_BMP180_H */