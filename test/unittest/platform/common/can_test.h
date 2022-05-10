/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef CAN_TEST_H
#define CAN_TEST_H

#include "can_if.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CAN_TEST_BUS_NUM   31
#define CAN_TEST_BIT_RATE  (1000 * 10) // 1K
#define CAN_TEST_WORK_MODE CAN_BUS_LOOPBACK

int32_t CanTestExecute(int cmd);

enum CanTestCmd {
    CAN_TEST_SEND_AND_READ = 0,
    CAN_TEST_NO_BLOCK_READ,
    CAN_TEST_BLOCK_READ,
    CAN_TEST_ADD_DEL_FILTER,
    CAN_TEST_ADD_MULTI_FILTER,
    CAN_TEST_GET_BUS_STATE,
    CAN_TEST_MULTI_THREAD_READ_SAME_HANDLE,
    CAN_TEST_MULTI_THREAD_READ_MULTI_HANDLE,
    CAN_TEST_MULTI_THREAD_SEND_SAME_HANDLE,
    CAN_TEST_MULTI_THREAD_SEND_MULTI_HANDLE,
    CAN_TEST_RELIABILITY,
    CAN_TEST_CMD_MAX,
};

struct CanTestConfig {
    uint16_t busNum;
    uint32_t bitRate;
    uint8_t workMode;
};

struct CanTester {
    struct CanTestConfig config;
    DevHandle handle;
    uint16_t total;
    uint16_t fails;
    bool readerFlag;
    bool pollerFlag;
    struct CanMsg msg;
};

#ifdef __cplusplus
}
#endif
#endif /* CAN_TEST_H */
