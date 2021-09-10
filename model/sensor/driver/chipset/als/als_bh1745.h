/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef ALS_BH1745_H
#define ALS_BH1745_H

#include "sensor_als_driver.h"
#include "sensor_config_parser.h"

/* ALS DATA REGISTERS ADDR */
#define BH1745_ALS_R_LSB_ADDR              0X50
#define BH1745_ALS_R_MSB_ADDR              0X51
#define BH1745_ALS_G_LSB_ADDR              0X52
#define BH1745_ALS_G_MSB_ADDR              0X53
#define BH1745_ALS_B_LSB_ADDR              0X54
#define BH1745_ALS_B_MSB_ADDR              0X55
#define BH1745_ALS_C_LSB_ADDR              0X56
#define BH1745_ALS_C_MSB_ADDR              0X57
#define BH1745_MODECONTROL3_ADDR           0X44

/* ALS DATA READY */
#define BH1745_ALS_DATA_READY_MASK         0X02

int32_t DetectAlsBim160Chip(struct SensorCfgData *data);
int32_t ReadBh1745Data(struct SensorCfgData *data);

struct Bh1745DrvData {
    struct IDeviceIoService ioService;
    struct HdfDeviceObject *device;
    struct SensorCfgData *sensorCfg;
};

#endif /* ALS_BH1745_H */
