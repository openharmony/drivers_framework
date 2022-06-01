/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */
#include "platform_listener_u.h"
#include "hdf_io_service_if.h"
#include "hdf_log.h"
#include "ioservstat_listener.h"
#include "osal_mem.h"
#include "securec.h"

struct ModuleOnDevEventReceived {
    enum PlatformModuleType moudle;
    OnEventReceived callback;
};

static int GpioOnDevEventReceive(void *priv, uint32_t id, struct HdfSBuf *data)
{
    struct PlatformUserListenerGpioParam *gpio = NULL;
    struct PlatformUserListener *userListener = NULL;
    uint16_t gpioId;
    if (priv == NULL || data == NULL) {
        HDF_LOGE("GpioOnDevEventReceive param error");
        return HDF_FAILURE;
    }

    userListener = (struct PlatformUserListener *)priv;
    gpio = (struct PlatformUserListenerGpioParam *)userListener->data;
    if (gpio == NULL || gpio->data == NULL || gpio->func == NULL) {
        HDF_LOGE("GpioOnDevEventReceive gpio error");
        return HDF_FAILURE;
    }

    if (!HdfSbufReadUint16(data, &gpioId)) {
        HDF_LOGE("GpioOnDevEventReceive read sbuf fail");
        return HDF_ERR_IO;
    }

    HDF_LOGD("GpioOnDevEventReceive event %d gpioId:%d == gpio:%d", id, gpioId, gpio->gpio);
    if ((id == PLATFORM_LISTENER_EVENT_GPIO_INT_NOTIFY) && (gpioId == gpio->gpio)) {
        gpio->func(gpioId, gpio->data);
    }
    return HDF_SUCCESS;
}

static struct ModuleOnDevEventReceived g_Receives[] = {
    {PLATFORM_MODULE_GPIO, GpioOnDevEventReceive},
};

static OnEventReceived PlatformUserListenerCbGet(enum PlatformModuleType moudle)
{
    unsigned int i;
    for (i = 0; i < sizeof(g_Receives) / sizeof(g_Receives[0]); i++) {
        if (moudle == g_Receives[i].moudle) {
            return g_Receives[i].callback;
        }
    }
    HDF_LOGD("PlatformUserListenerCbGet module callback[%d]not find", moudle);
    return NULL;
}

static struct PlatformUserListener *PlatformUserListenerInit(
    struct PlatformUserListenerManager *manager, uint32_t num, void *data)
{
    struct PlatformUserListener *userListener = NULL;
    struct HdfDevEventlistener *listener = NULL;

    userListener = OsalMemCalloc(sizeof(struct PlatformUserListener));
    if (userListener == NULL) {
        HDF_LOGE("PlatformUserListenerInit userListener get failed");
        return NULL;
    }

    listener = OsalMemCalloc(sizeof(struct HdfDevEventlistener));
    if (listener == NULL) {
        HDF_LOGE("PlatformUserListenerInit hdf listener get failed");
        OsalMemFree(userListener);
        return NULL;
    }

    userListener->listener = listener;
    userListener->moudle = manager->moudle;
    userListener->num = num;
    userListener->data = data;

    listener->callBack = PlatformUserListenerCbGet(manager->moudle);
    if (listener->callBack == NULL) {
        HDF_LOGE("PlatformUserListenerInit hdf listener cb get failed");
        OsalMemFree(userListener);
        OsalMemFree(listener);
        return NULL;
    }
    listener->priv = userListener;
    if (HdfDeviceRegisterEventListener(manager->service, listener) != HDF_SUCCESS) {
        HDF_LOGE("PlatformUserListenerInit HdfDeviceRegisterEventListener failed");
        OsalMemFree(userListener);
        OsalMemFree(listener);
        return NULL;
    }

    HDF_LOGD("PlatformUserListenerInit get listener for %d %d success", manager->moudle, num);
    return userListener;
}

int32_t PlatformUserListenerReg(struct PlatformUserListenerManager *manager, uint32_t num, void *data)
{
    struct PlatformUserListener *pos = NULL;
    struct PlatformUserListener *node = NULL;
    if (manager == NULL) {
        HDF_LOGE("PlatformUserListenerReg manager null");
        return HDF_FAILURE;
    }

    if (OsalMutexLock(&manager->lock) != HDF_SUCCESS) {
        HDF_LOGE("PlatformUserListenerReg: OsalMutexLock failed");
        return HDF_FAILURE;
    };
    DLIST_FOR_EACH_ENTRY(pos, &manager->listeners, struct PlatformUserListener, node) {
        if (pos->num == num) {
            (void)OsalMutexUnlock(&manager->lock);
            return HDF_SUCCESS;
        }
    }

    node = PlatformUserListenerInit(manager, num, data);
    if (node == NULL) {
        HDF_LOGE("PlatformUserListenerReg PlatformUserListenerInit fail");
        (void)OsalMutexUnlock(&manager->lock);
        return HDF_FAILURE;
    }

    DListInsertTail(&node->node, &manager->listeners);
    (void)OsalMutexUnlock(&manager->lock);
    return HDF_SUCCESS;
}

void PlatformUserListenerDestory(struct PlatformUserListenerManager *manager, uint32_t num)
{
    struct PlatformUserListener *pos = NULL;
    struct PlatformUserListener *tmp = NULL;
    if (manager == NULL) {
        HDF_LOGE("PlatformUserListenerDestory manager null");
        return;
    }
    if (OsalMutexLock(&manager->lock) != HDF_SUCCESS) {
        HDF_LOGE("%s: OsalMutexLock failed", __func__);
        return;
    }
    DLIST_FOR_EACH_ENTRY_SAFE(pos, tmp, &manager->listeners, struct PlatformUserListener, node) {
        if (pos->num == num) {
            HDF_LOGD("PlatformUserListenerDestory: node [%d][%d] find, then del", manager->moudle, num);
            if (HdfDeviceUnregisterEventListener(manager->service, pos->listener) != HDF_SUCCESS) {
                HDF_LOGE("PlatformUserListenerDestory unregister fail");
                (void)OsalMutexUnlock(&manager->lock);
                return;
            }
            OsalMemFree(pos->listener);
            OsalMemFree(pos->data);
            DListRemove(&pos->node);
            OsalMemFree(pos);
            (void)OsalMutexUnlock(&manager->lock);
            return;
        }
    }
    (void)OsalMutexUnlock(&manager->lock);
}

struct PlatformUserListenerManager *PlatformUserListenerManagerGet(enum PlatformModuleType moudle)
{
    struct PlatformUserListenerManager *manager = OsalMemCalloc(sizeof(struct PlatformUserListenerManager));
    if (manager == NULL) {
        HDF_LOGE("PlatformUserListenerManagerGet manager get failed");
        return NULL;
    }

    manager->moudle = moudle;
    DListHeadInit(&manager->listeners);
    if (OsalMutexInit(&manager->lock) != HDF_SUCCESS) {
        HDF_LOGE("PlatformUserListenerManagerGet moudle %d OsalSpinInit fail", moudle);
        OsalMemFree(manager);
        return NULL;
    }

    HDF_LOGD("PlatformUserListenerManagerGet moudle %d success", moudle);
    return manager;
}
