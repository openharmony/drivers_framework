/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "audio_accessory_base_test.h"
#include "audio_accessory_base.h"
#include "audio_driver_log.h"

#define HDF_LOG_TAG audio_accessory_base_test

static struct AudioMixerControl g_TestReg = {
    .min = 0,
    .max = 0,
    .platformMax = 0,
    .mask = 0,
    .reg = 0,
    .rreg = 0,
    .shift= 0,
    .rshift = 0,
    .invert = 0,
    .value = 0,
};

int32_t AccessoryI2cReadWriteTest(void)
{
    struct AudioAddrConfig addrConfig;

    if (AccessoryI2cReadWrite(NULL, 0) == HDF_SUCCESS) {
        return HDF_FAILURE;
    }

    if (AccessoryI2cReadWrite(&addrConfig, 0) == HDF_SUCCESS) {
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AccessoryRegBitsReadTest(void)
{
    uint32_t regValue = 0;

    if (AccessoryRegBitsRead(NULL, NULL) == HDF_SUCCESS) {
        return HDF_FAILURE;
    }

    if (AccessoryRegBitsRead(&g_TestReg, &regValue) == HDF_SUCCESS) {
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

int32_t AccessoryRegBitsUpdateTest(void)
{
    if (AccessoryRegBitsUpdate(g_TestReg) == HDF_SUCCESS) {
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}


int32_t AcessoryDeviceFrequencyParseTest(void)
{
    uint16_t freq = 0;
    if (AcessoryDeviceFrequencyParse(0, NULL) == HDF_SUCCESS) {
        return HDF_FAILURE;
    }

    if (AcessoryDeviceFrequencyParse(0, &freq) == HDF_SUCCESS) {
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

int32_t AccessoryDaiParamsUpdateTest(void)
{
    struct DaiParamsVal value;

    value.channelVal = 1; // 1 is dma channel
    if (AccessoryDaiParamsUpdate(value) == HDF_SUCCESS) {
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

int32_t AccessoryDeviceCfgGetTest(void)
{

    struct AccessoryData accessoryData;
    struct AccessoryTransferData accessoryTransferData;
    if (AccessoryDeviceCfgGet(NULL, NULL) == HDF_SUCCESS) {
        return HDF_FAILURE;
    }

    memset(&accessoryData, 0, sizeof(struct AccessoryData));
    memset(&accessoryTransferData, 0, sizeof(struct AccessoryTransferData));
    if (AccessoryDeviceCfgGet(&accessoryData, &accessoryTransferData) == HDF_SUCCESS) {
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AccessoryDeviceCtrlRegInitTest(void)
{
    if (AccessoryDeviceCtrlRegInit() == HDF_SUCCESS) {
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

int32_t AccessoryDeviceRegReadTest(void)
{
    uint32_t val;
    struct AccessoryDevice codec;
    if (AccessoryDeviceRegRead(NULL, 0, NULL) == HDF_SUCCESS) {
        return HDF_FAILURE;
    }

    if (AccessoryDeviceRegRead(&codec, 0, &val) == HDF_SUCCESS) {
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AccessoryDeviceRegWriteTest(void)
{
    struct AccessoryDevice codec;
    if (AccessoryDeviceRegWrite(NULL, 0, 0) == HDF_SUCCESS) {
        return HDF_FAILURE;
    }

    if (AccessoryDeviceRegWrite(&codec, 0, 0) == HDF_SUCCESS) {
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t AccessoryGetConfigInfoTest(void)
{
    struct HdfDeviceObject device;
    struct AccessoryData codecData;

    if (AccessoryGetConfigInfo(NULL, NULL) == HDF_SUCCESS) {
        return HDF_FAILURE;
    }

    if (AccessoryGetConfigInfo(&device, &codecData) == HDF_SUCCESS) {
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

