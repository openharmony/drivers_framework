/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef UART_FUZZER
#define UART_FUZZER

#define FUZZ_PROJECT_NAME "uart_fuzzer"

enum class ApiTestCmd {
    UART_FUZZ_SET_BAUD = 0,
    UART_FUZZ_SET_ATTRIBUTE,
    UART_FUZZ_SET_TRANSMODE,
};

#endif
