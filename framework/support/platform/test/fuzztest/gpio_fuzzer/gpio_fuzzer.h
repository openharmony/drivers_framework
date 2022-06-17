/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef GPIO_FUZZER
#define GPIO_FUZZER

#define FUZZ_PROJECT_NAME "gpio_fuzzer"

enum class ApiTestCmd {
    GPIO_FUZZ_WRITE = 0,
    GPIO_FUZZ_SET_DIR,
    GPIO_FUZZ_SET_IRQ,
};

#endif
