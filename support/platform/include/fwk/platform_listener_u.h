/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef PLATFORM_LISTENER_U_H
#define PLATFORM_LISTENER_U_H

#include "gpio_if.h"
#include "hdf_base.h"
#include "hdf_dlist.h"
#include "hdf_io_service_if.h"
#include "hdf_service_status.h"
#include "osal_mutex.h"
#include "platform_core.h"
#include "platform_listener_common.h"
#include "rtc_if.h"
#include "svcmgr_ioservice.h"
#include "timer_if.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct PlatformUserListener {
    enum PlatformModuleType moudle;
    uint32_t num;
    struct HdfDevEventlistener *listener;
    void *data;
    struct DListHead node;
};

struct PlatformUserListenerManager {
    struct HdfIoService *service;
    enum PlatformModuleType moudle;
    struct DListHead listeners;
    struct OsalMutex lock;
};

struct PlatformUserListenerGpioParam {
    uint16_t gpio;
    void *data;
    GpioIrqFunc func;
};

struct PlatformUserListenerRtcParam {
    enum RtcAlarmIndex index;
    RtcAlarmCallback func;
};

struct PlatformUserListenerTimerParam {
    uint32_t handle;
    bool isOnce;
    TimerHandleCb func;
    struct PlatformUserListenerManager *manager;
};

struct PlatformUserListenerManager *PlatformUserListenerManagerGet(enum PlatformModuleType moudle);
int32_t PlatformUserListenerReg(
    struct PlatformUserListenerManager *manager, uint32_t num, void *data, OnEventReceived callback);
void PlatformUserListenerDestory(struct PlatformUserListenerManager *manager, uint32_t num);

int GpioOnDevEventReceive(void *priv, uint32_t id, struct HdfSBuf *data);
int TimerOnDevEventReceive(void *priv, uint32_t id, struct HdfSBuf *data);
int RtcOnDevEventReceive(void *priv, uint32_t id, struct HdfSBuf *data);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* PLATFORM_LISTENER_U_H */
