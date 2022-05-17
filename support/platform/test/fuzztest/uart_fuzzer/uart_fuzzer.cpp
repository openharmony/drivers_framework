/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "uart_fuzzer.h"
#include <iostream>
#include "random.h"
#include "securec.h"
#include "hdf_base.h"
#include "uart_if.h"

using namespace std;

namespace {
constexpr int32_t MIN = 0;
constexpr int32_t MAX = 2;
const int32_t UART_FUZZ_PORT = 1;
}

struct AllParameters {
    uint32_t desBaudRate;
    struct UartAttribute paraAttribute;
    uint32_t paraMode;
};

namespace OHOS {
    bool UartFuzzTest(const uint8_t *data, size_t size)
    {
        int32_t number;
        DevHandle handle = nullptr;
        struct AllParameters params;

        if (data == nullptr) {
            return false;
        }

        if (memcpy_s((void *)&params, sizeof(params), data, sizeof(params)) != EOK) {
            return false;
        }

        number = randNum(MIN, MAX);
        handle = UartOpen(UART_FUZZ_PORT);
        switch (static_cast<ApiTestCmd>(number)) {
            case ApiTestCmd::UART_FUZZ_SET_BAUD:
                UartSetBaud(handle, params.desBaudRate);
                break;
            case ApiTestCmd::UART_FUZZ_SET_ATTRIBUTE:
                UartSetAttribute(handle, &params.paraAttribute);
                break;
            case ApiTestCmd::UART_FUZZ_SET_TRANSMODE:
                UartSetTransMode(handle, (enum UartTransMode)params.paraMode);
                break;
            default:
                break;
        }
        UartClose(handle);
        return true;
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::UartFuzzTest(data, size);
    return 0;
}
