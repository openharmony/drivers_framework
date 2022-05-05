/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "gpio_fuzzer.h"
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <random>
#include <cstddef>
#include "gpio_if.h"
#include "hdf_base.h"
#include "securec.h"

using namespace std;

namespace {
constexpr int32_t MIN = 0;
constexpr int32_t MAX = 2;
const uint16_t gpioTestNum = 3;
const int32_t MAX_LEN = 10;
}

struct AllParameters {
    uint16_t descVal;
    uint16_t descDir;
    uint16_t descMode;
};

union ConvertToParam {
    uint8_t data[MAX_LEN];
    struct AllParameters params;
};

static int32_t GpioTestIrqHandler(uint16_t gpio, void *data)
{
    (void)gpio;
    (void)data;
    return 0;
}

static int32_t randNum()
{
    std::random_device rd;
    std::default_random_engine engine(rd());
    std::uniform_int_distribution<int32_t> randomNum(MIN, MAX);
    return randomNum(engine);
}

namespace OHOS {
    bool GpioFuzzTest(const uint8_t* data, size_t size)
    {
        union ConvertToParam convertor;

        if (data == nullptr) {
            return false;
        }

        if (memcpy_s (convertor.data, MAX_LEN, data, MAX_LEN) != EOK) {
            return false;
        }

        int32_t number = randNum();
        switch (static_cast<ApiNumber>(number)) {
            case ApiNumber::NUM_ZERO:
                GpioWrite(gpioTestNum, convertor.params.descVal);
                break;
            case ApiNumber::NUM_ONE:
                GpioSetDir(gpioTestNum, convertor.params.descDir);
                break;
            case ApiNumber::NUM_TWO:
                GpioSetIrq(gpioTestNum, convertor.params.descMode, GpioTestIrqHandler, &data);
                break;
            default:
                break;
        }
        return true;
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::GpioFuzzTest(data, size);
    return 0;
}
