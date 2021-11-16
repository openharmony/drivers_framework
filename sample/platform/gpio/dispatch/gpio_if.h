/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef GPIO_IF_H
#define GPIO_IF_H

#include <stdint.h>

#define GPIO_SERVICE_NAME "GPIO_SAMPLE"

enum GpioValue {
    GPIO_VAL_LOW = 0,
    GPIO_VAL_HIGH = 1,
    GPIO_VAL_ERR,
};

enum GpioDirType {
    GPIO_DIR_IN = 0,
    GPIO_DIR_OUT = 1,
    GPIO_DIR_ERR,
};

enum GpioOps {
    GPIO_OPS_SET_DIR = 1,
    GPIO_OPS_GET_DIR,
    GPIO_OPS_WRITE,
    GPIO_OPS_READ
};

int32_t GpioOpen();
int32_t GpioClose();
int32_t GpioSetDir(uint16_t gpio, uint16_t dir);
int32_t GpioGetDir(uint16_t gpio, uint16_t *dir);
int32_t GpioWrite(uint16_t gpio, uint16_t val);
int32_t GpioRead(uint16_t gpio, uint16_t *val);

#endif // GPIO_IF_H