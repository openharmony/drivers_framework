/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef TIMER_TEST_H
#define TIMER_TEST_H

#include "hdf_device_desc.h"
#include "platform_if.h"

enum TimerTestCmd {
    TIMER_START_TEST = 1,
    TIMER_TEST_SET,
    TIMER_TEST_SETONCE,
    TIMER_TEST_GET,
    TIMER_TEST_START,
    TIMER_TEST_STOP,
    TIMER_MULTI_THREAD_TEST,
    TIMER_RELIABILITY_TEST,
};

#define TIMER_TEST_STACK_SIZE    (1024 * 100)
#define TIMER_TEST_WAIT_TIMES    20
#define TIMER_TEST_WAIT_TIMEOUT  5
#define TIMER_TEST_PERIOD_TIMES  2

#define TIMER_TEST_TIME_ID_THREAD1  3
#define TIMER_TEST_TIME_ID_THREAD2  5
#define TIMER_TEST_TIME_USECONDS    5000

struct TimerTest {
    struct IDeviceIoService service;
    struct HdfDeviceObject *device;
    int32_t (*TestEntry)(struct TimerTest *test, int32_t cmd);
    DevHandle handle;
    uint32_t number;
    uint32_t uSecond;
    uint32_t isPeriod;
};

static inline struct TimerTest *GetTimerTest(void)
{
    return (struct TimerTest *)DevSvcManagerClntGetService("TIMER_TEST");
}

#endif /* TIMER_TEST_H */
