/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef SENSOR_DEVICE_MANAGER_H
#define SENSOR_DEVICE_MANAGER_H

#include "osal_mutex.h"
#include "sensor_device_type.h"
#include "sensor_device_if.h"

enum SensorCmd {
    SENSOR_CMD_GET_INFO_LIST = 0,
    SENSOR_CMD_OPS           = 1,
    SENSOR_CMD_END,
};
enum SensorOpsCmd {
    SENSOR_OPS_CMD_ENABLE        = 1,
    SENSOR_OPS_CMD_DISABLE       = 2,
    SENSOR_OPS_CMD_SET_BATCH     = 3,
    SENSOR_OPS_CMD_SET_MODE      = 4,
    SENSOR_OPS_CMD_SET_OPTION    = 5,
    SENSOR_OPS_CMD_BUTT,
};

struct SensorDevInfoNode {
    struct SensorDeviceInfo devInfo;
    struct DListHead node;
};

typedef int32_t (*SensorCmdHandle)(struct SensorDeviceInfo *info, struct HdfSBuf *data, struct HdfSBuf *reply);

struct SensorCmdHandleList {
    enum SensorOpsCmd cmd;
    SensorCmdHandle func;
};

struct SensorDevMgrData {
    struct IDeviceIoService ioService;
    struct HdfDeviceObject *device;
    struct DListHead sensorDevInfoHead;
    struct OsalMutex mutex;
    struct OsalMutex eventMutex;
};

#endif /* SENSOR_DEVICE_MANAGER_H */
