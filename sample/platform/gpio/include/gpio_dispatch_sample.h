/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef GPIO_DISPATCH_SAMPLE_H
#define GPIO_DISPATCH_SAMPLE_H

#include "gpio_pl061_sample.h"

enum GpioOps {
    GPIO_OPS_SET_DIR = 1,
    GPIO_OPS_GET_DIR,
    GPIO_OPS_WRITE,
    GPIO_OPS_READ
};

int32_t SampleGpioDispatch(struct HdfDeviceIoClient *client, int cmdId, struct HdfSBuf *data, struct HdfSBuf *reply);

#endif // GPIO_DISPATCH_SAMPLE_H
