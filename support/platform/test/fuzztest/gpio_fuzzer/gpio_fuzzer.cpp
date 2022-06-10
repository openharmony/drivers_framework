/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "gpio_fuzzer.h"
#include <iostream>
#include "random.h"
#include "securec.h"
#include "gpio_if.h"
#include "hdf_base.h"
#include "hdf_log.h"

using namespace std;

namespace {
constexpr int32_t MIN = 0;
constexpr int32_t MAX = 2;
const uint16_t gpioTestNum = 3;
}

struct AllParameters {
    uint16_t descVal;
    uint16_t descDir;
    uint16_t descMode;
};

static int32_t GpioTestIrqHandler(uint16_t gpio, void *data)
{
    (void)gpio;
    (void)data;
    return 0;
}

namespace OHOS {
    bool GpioFuzzTest(const uint8_t *data, size_t size)
    {
        int32_t number;
        struct AllParameters params;

        if (data == nullptr) {
            HDF_LOGE("%{public}s:data is null", __func__);
            return false;
        }

        if (memcpy_s((void *)&params, sizeof(params), data, sizeof(params)) != EOK) {
            HDF_LOGE("%{public}s:memcpy data failed", __func__);
            return false;
        }
        number = randNum(MIN, MAX);
        switch (static_cast<ApiTestCmd>(number)) {
            case ApiTestCmd::GPIO_FUZZ_WRITE:
                GpioWrite(gpioTestNum, params.descVal);
                break;
            case ApiTestCmd::GPIO_FUZZ_SET_DIR:
                GpioSetDir(gpioTestNum, params.descDir);
                break;
            case ApiTestCmd::GPIO_FUZZ_SET_IRQ:
                GpioSetIrq(gpioTestNum, params.descMode, GpioTestIrqHandler, &data);
                break;
            default:
                break;
        }
        return true;
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::GpioFuzzTest(data, size);
    return 0;
}
