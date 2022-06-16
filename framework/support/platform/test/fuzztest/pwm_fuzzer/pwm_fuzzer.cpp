/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "pwm_fuzzer.h"
#include <iostream>
#include "random.h"
#include "securec.h"
#include "hdf_base.h"
#include "hdf_log.h"
#include "pwm_if.h"

using namespace std;

namespace {
constexpr int32_t MIN = 0;
constexpr int32_t MAX = 2;
const int32_t PWM_FUZZ_NUM = 1;
}

struct AllParameters {
    uint32_t descPer;
    uint32_t descDuty;
    uint8_t descPolar;
};

namespace OHOS {
    bool PwmFuzzTest(const uint8_t *data, size_t size)
    {
        int32_t number;
        DevHandle handle = nullptr;
        struct AllParameters params;

        if (data == nullptr) {
            HDF_LOGE("%{public}s:data is null", __func__);
            return false;
        }

        if (memcpy_s((void *)&params, sizeof(params), data, sizeof(params)) != EOK) {
            HDF_LOGE("%{public}s:data is null", __func__);
            return false;
        }

        number = randNum(MIN, MAX);
        handle = PwmOpen(PWM_FUZZ_NUM);
        switch (static_cast<ApiTestCmd>(number)) {
            case ApiTestCmd::PWM_FUZZ_SET_PERIOD:
                PwmSetPeriod(handle, params.descPer);
                break;
            case ApiTestCmd::PWM_FUZZ_SET_DUTY:
                PwmSetDuty(handle, params.descDuty);
                break;
            case ApiTestCmd::PWM_FUZZ_SET_POLARITY:
                PwmSetPolarity(handle, params.descPolar);
                break;
            default:
                break;
        }
        PwmClose(handle);
        return true;
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::PwmFuzzTest(data, size);
    return 0;
}
