/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef SPI_FUZZER
#define SPI_FUZZER

#define FUZZ_PROJECT_NAME "spi_fuzzer"

enum class ApiNumber {
    SPI_FUZZ_TRANSFER = 0,
    SPI_FUZZ_WRITE,
    SPI_FUZZ_SETCFG,
};

#endif
