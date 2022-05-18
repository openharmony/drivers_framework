/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef PWM_FUZZER
#define PWM_FUZZER

#define FUZZ_PROJECT_NAME "pwm_fuzzer"

enum class ApiTestCmd {
    PWM_FUZZ_SET_PERIOD = 0,
    PWM_FUZZ_SET_DUTY,
    PWM_FUZZ_SET_POLARITY,
};

#endif
