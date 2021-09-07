/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */
#ifndef HDF_ENCODER_H
#define HDF_ENCODER_H

#include <securec.h>
#include "osal_time.h"
#include "osal_timer.h"
#include "input_config.h"
#include "hdf_input_device_manager.h"


typedef struct EncoderDriverInfo {
    struct HdfDeviceObject *hdfEncoderDev;
    uint8_t devType;
    EncoderCfg *encoderCfg;
    InputDevice *inputDev;
    OsalTimer timer;
    uint16_t encoderClkPreSta;
    uint16_t encoderClkNowSta;
    uint16_t encoderDataPreSta;
    uint16_t encoderDataNowSta;
} EncoderDriver;

#endif