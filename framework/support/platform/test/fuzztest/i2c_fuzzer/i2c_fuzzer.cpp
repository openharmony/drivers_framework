/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "i2c_fuzzer.h"

using namespace std;

namespace {
const int32_t BUS_NUM = 6;
const uint16_t BUF_LEN = 7;
}

struct AllParameters {
    uint16_t addr;
    uint16_t flags;
    uint8_t buf[BUF_LEN];
};

namespace OHOS {
    bool I2cFuzzTest(const uint8_t *data, size_t size)
    {
        DevHandle handle = nullptr;
        struct AllParameters params;
        struct I2cMsg msg;

        if (data == nullptr) {
            HDF_LOGE("%{public}s:data is null", __func__);
            return false;
        }
        if (memcpy_s((void *)&params, sizeof(params), data, sizeof(params)) != EOK) {
            HDF_LOGE("%{public}s:memcpy data failed", __func__);
            return false;
        }

        handle = I2cOpen(BUS_NUM);
        if (handle == nullptr) {
            HDF_LOGE("%{public}s:handle is null", __func__);
            return false;
        }
        msg.addr = params.addr;
        msg.flags = params.flags;
        msg.len = BUF_LEN;
        msg.buf = (uint8_t *)malloc(BUF_LEN);
        if (memcpy_s(msg.buf, BUF_LEN, params.buf, BUF_LEN) != EOK) {
            HDF_LOGE("%{public}s:memcpy buf failed", __func__);
            free(msg.buf);
            return false;
        }
        I2cTransfer(handle, &msg, 1);
        free(msg.buf);
        I2cClose(handle);

        return true;
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::I2cFuzzTest(data, size);
    return 0;
}
