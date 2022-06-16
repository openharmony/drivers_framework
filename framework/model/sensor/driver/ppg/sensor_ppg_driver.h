/*
 * Copyright (c) 2022 Chipsea Technologies (Shenzhen) Corp., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef MODEL_SENSOR_DRIVER_PPG_SENSOR_PPG_DRIVER_H_
#define MODEL_SENSOR_DRIVER_PPG_SENSOR_PPG_DRIVER_H_

#include "hdf_workqueue.h"
#include "osal_time.h"
#include "sensor_ppg_config.h"

#define PPG_MODULE_VER 1

struct PpgOpsCall {
    int32_t (*Enable)(void);
    int32_t (*Disable)(void);
    int32_t (*ReadData)(uint8_t *outBuf, uint16_t outBufMaxLen, uint16_t *outLen);
    int32_t (*SetOption)(uint32_t option);
    int32_t (*SetMode)(uint32_t mode);
};

struct PpgChipData {
    struct PpgCfgData *cfgData;
    struct PpgOpsCall opsCall;
};

struct PpgDrvData {
    struct IDeviceIoService ioService;
    struct HdfDeviceObject *device;
    HdfWorkQueue ppgWorkQueue;
    HdfWork ppgWork;
    bool detectFlag;
    bool enable;
    bool initStatus;
    int64_t interval;
    struct PpgChipData chipData;
};

int32_t RegisterPpgChip(struct PpgChipData *chipData);
#endif  // MODEL_SENSOR_DRIVER_PPG_SENSOR_PPG_DRIVER_H_
