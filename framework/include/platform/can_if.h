/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef CAN_IF_H
#define CAN_IF_H

#include "platform_if.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define CAN_DATA_LEN 8

enum CanBusState {
    CAN_BUS_RESET,
    CAN_BUS_READY,
    CAN_BUS_BUSY,
    CAN_BUS_STOP,
    CAN_BUS_SLEEP,
    CAN_BUS_ERROR,
    CAN_BUS_INVALID,
};

enum CanBusMode {
    CAN_BUS_NORMAL,
    CAN_BUS_LOOPBACK,
};

struct CanMsg {
    union {
        uint32_t id11 : 11;
        uint32_t id29 : 29;
        uint32_t id : 29;
    };
    uint32_t ide : 1;
    uint32_t rtr : 1;
    uint32_t padding : 1;
    uint8_t dlc;
    uint8_t data[CAN_DATA_LEN];
    int32_t error;
    const struct CanFilter *filter;
};

enum CanEvent {
    CAN_EVENT_MSG_RECEIVED,
    CAN_EVENT_MSG_SENT,
    CAN_EVENT_ERROR,
};

enum CanFilterType {
    CAN_FILTER_HW = 1,
};

struct CanFilter {
    uint32_t rtr : 1;
    uint32_t ide : 1;
    uint32_t id : 29;
    uint32_t rtrMask : 1;
    uint32_t ideMask : 1;
    uint32_t idMask : 29;
    uint32_t type : 2;
};

struct CanConfig {
    uint32_t speed;
    uint8_t mode;
};

int32_t CanBusOpen(int32_t number, DevHandle *handle);

void CanBusClose(DevHandle handle);

int32_t CanBusSendMsg(DevHandle handle, const struct CanMsg *msg);

int32_t CanBusReadMsg(DevHandle handle, struct CanMsg *msg, uint32_t tms);

int32_t CanBusAddFilter(DevHandle handle, const struct CanFilter *filter);

int32_t CanBusDelFilter(DevHandle handle, const struct CanFilter *filter);

int32_t CanBusSetCfg(DevHandle handle, const struct CanConfig *cfg);

int32_t CanBusGetCfg(DevHandle handle, struct CanConfig *cfg);

int32_t CanBusGetState(DevHandle handle);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CAN_IF_H */
