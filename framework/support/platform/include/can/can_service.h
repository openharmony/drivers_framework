/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef CAN_SERVICE_H
#define CAN_SERVICE_H

#include "hdf_device_desc.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

enum CanIoCmd {
    CAN_IO_READ = 0,
    CAN_IO_WRITE = 1,
    CAN_IO_SETCFG = 2,
    CAN_IO_GETCFG = 3,
};

int32_t CanServiceBind(struct HdfDeviceObject *device);

void CanServiceRelease(struct HdfDeviceObject *device);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CAN_SERVICE_H */
