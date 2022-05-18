/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include <iostream>
#include "random.h"
#include "securec.h"
#include "hdf_base.h"
#include "spi_if.h"
#include "spi_fuzzer.h"

using namespace std;

namespace {
constexpr int32_t MIN = 0;
constexpr int32_t MAX = 2;
const int32_t busTestNum = 0;
const int32_t csTestNum = 0;
const uint32_t SPI_BUF_SIZE = 8;
}

struct AllParameters {
    uint32_t descSpeed;
    uint16_t descDelay;
    uint8_t descKeep;
    uint32_t desMaxSpeedHz;
    uint16_t desMode;
    uint8_t desTransferMode;
    uint8_t desBitsPerWord;
    uint8_t buf[SPI_BUF_SIZE];
};

namespace OHOS {
    bool SpiFuzzTest(const uint8_t *data, size_t size)
    {
        int32_t number;
        DevHandle handle = nullptr;
        struct SpiMsg msg;
        struct SpiCfg cfg;
        struct SpiDevInfo info;
        struct AllParameters params;

        if (data == nullptr) {
            return false;
        }
        if (memcpy_s ((void *)&params, sizeof(params), data, sizeof(params)) != EOK) {
            return false;
        }

        info.busNum = busTestNum;
        info.csNum = csTestNum;
        msg.speed = params.descSpeed;
        msg.delayUs = params.descDelay;
        msg.keepCs = params.descKeep;
        msg.len = SPI_BUF_SIZE;
        msg.rbuf = (uint8_t *)malloc(SPI_BUF_SIZE);
        if (msg.rbuf == nullptr) {
            return false;
        }
        msg.wbuf = (uint8_t *)malloc(SPI_BUF_SIZE);
        if (msg.wbuf == nullptr) {
            free(msg.rbuf);
            return false;
        }
        if (memcpy_s((void *)msg.wbuf, SPI_BUF_SIZE, params.buf, SPI_BUF_SIZE) != EOK) {
            free(msg.rbuf);
            free(msg.wbuf);
            return false;
        }
        cfg.maxSpeedHz = params.desMaxSpeedHz;
        cfg.mode = params.desMode;
        cfg.transferMode = params.desTransferMode;
        cfg.bitsPerWord = params.desBitsPerWord;

        handle = SpiOpen(&info);
        number = randNum(MIN, MAX);
        switch (static_cast<ApiNumber>(number)) {
            case ApiNumber::SPI_FUZZ_TRANSFER:
                SpiTransfer(handle, &msg, SPI_BUF_SIZE);
                break;
            case ApiNumber::SPI_FUZZ_WRITE:
                SpiWrite(handle, msg.wbuf, SPI_BUF_SIZE);
                break;
            case ApiNumber::SPI_FUZZ_SETCFG:
                SpiSetCfg(handle, &cfg);
                break;
            default:
                break;
        }
        free(msg.rbuf);
        free(msg.wbuf);
        SpiClose(handle);
        return true;
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    OHOS::SpiFuzzTest(data, size);
    return 0;
}
