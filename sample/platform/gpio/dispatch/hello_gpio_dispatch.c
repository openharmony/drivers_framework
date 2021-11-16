/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "gpio_if.h"
#include "hdf_log.h"
#include "hdf_base.h"

#define HDF_LOG_TAG hello_gpio_dispatch
#define GPIO_PIN 11

int main()
{
    uint16_t dir;
    uint16_t val;
    if (GpioOpen() != HDF_SUCCESS) {
        HDF_LOGE("%s: GpioOpen failed", __func__);
        return HDF_FAILURE;
    }
    if (GpioSetDir(GPIO_PIN, GPIO_DIR_IN) != HDF_SUCCESS) {
        HDF_LOGE("%s: GpioSetDir failed, gpio %u, dir %u", __func__, GPIO_PIN, GPIO_DIR_IN);
        return HDF_FAILURE;
    }
    if (GpioWrite(GPIO_PIN, GPIO_VAL_HIGH) != HDF_SUCCESS) {
        HDF_LOGE("%s: GpioWrite failed, gpio %u, val %u", __func__, GPIO_PIN, GPIO_VAL_HIGH);
        return HDF_FAILURE;
    }
    if (GpioGetDir(GPIO_PIN, &dir) != HDF_SUCCESS) {
        HDF_LOGE("%s: GpioGetDir failed, gpio %u", __func__, GPIO_PIN);
        return HDF_FAILURE;
    }
    if (GpioRead(GPIO_PIN, &val) != HDF_SUCCESS) {
        HDF_LOGE("%s: GpioRead failed, gpio %u", __func__, GPIO_PIN);
        return HDF_FAILURE;
    }
    if (GpioClose() != HDF_SUCCESS) {
        HDF_LOGE("%s: GpioClose failed", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGD("GPIO %u direction is set to %u, value is set to %u", GPIO_PIN, dir, val);
    return HDF_SUCCESS;
}