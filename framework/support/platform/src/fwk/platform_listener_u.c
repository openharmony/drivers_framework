/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */
#include "platform_listener_u.h"
#include "hdf_log.h"
#include "ioservstat_listener.h"
#include "osal_mem.h"
#include "securec.h"

int RtcOnDevEventReceive(void *priv, uint32_t id, struct HdfSBuf *data)
{
    struct PlatformUserListenerRtcParam *rtc = NULL;
    struct PlatformUserListener *userListener = NULL;
    uint8_t index;
    if (priv == NULL || data == NULL) {
        HDF_LOGE("RtcOnDevEventReceive id %d param error", id);
        return HDF_FAILURE;
    }

    userListener = (struct PlatformUserListener *)priv;
    rtc = (struct PlatformUserListenerRtcParam *)userListener->data;
    if (rtc == NULL || rtc->func == NULL) {
        HDF_LOGE("RtcOnDevEventReceive rtc id %d error", id);
        return HDF_FAILURE;
    }

    if (!HdfSbufReadUint8(data, &index)) {
        HDF_LOGE("RtcOnDevEventReceive id %d read sbuf fail", id);
        return HDF_ERR_IO;
    }

    HDF_LOGD("RtcOnDevEventReceive event %d index:%d == index:%d", id, index, rtc->index);
    if ((id == PLATFORM_LISTENER_EVENT_RTC_ALARM_NOTIFY) && (index == rtc->index)) {
        rtc->func(index);
    }
    return HDF_SUCCESS;
}

int TimerOnDevEventReceive(void *priv, uint32_t id, struct HdfSBuf *data)
{
    struct PlatformUserListenerTimerParam *timer = NULL;
    struct PlatformUserListener *userListener = NULL;
    uint32_t handle;
    if (priv == NULL || data == NULL) {
        HDF_LOGE("TimerOnDevEventReceive id %d param error", id);
        return HDF_FAILURE;
    }

    userListener = (struct PlatformUserListener *)priv;
    timer = (struct PlatformUserListenerTimerParam *)userListener->data;
    if (timer == NULL || timer->func == NULL) {
        HDF_LOGE("TimerOnDevEventReceive timer id %d error", id);
        return HDF_FAILURE;
    }

    if (!HdfSbufReadUint32(data, &handle)) {
        HDF_LOGE("TimerOnDevEventReceive id %d read sbuf fail", id);
        return HDF_ERR_IO;
    }

    HDF_LOGD("TimerOnDevEventReceive event %d handle:%d == handle:%d", id, handle, timer->handle);
    if ((id == PLATFORM_LISTENER_EVENT_TIMER_NOTIFY) && (handle == timer->handle)) {
        timer->func(handle);
    }
    return HDF_SUCCESS;
}

int GpioOnDevEventReceive(void *priv, uint32_t id, struct HdfSBuf *data)
{
    struct PlatformUserListenerGpioParam *gpio = NULL;
    struct PlatformUserListener *userListener = NULL;
    uint16_t gpioId;
    if (priv == NULL || data == NULL) {
        HDF_LOGE("GpioOnDevEventReceive id %d param error", id);
        return HDF_FAILURE;
    }

    userListener = (struct PlatformUserListener *)priv;
    gpio = (struct PlatformUserListenerGpioParam *)userListener->data;
    if (gpio == NULL || gpio->data == NULL || gpio->func == NULL) {
        HDF_LOGE("GpioOnDevEventReceive id %d gpio error", id);
        return HDF_FAILURE;
    }

    if (!HdfSbufReadUint16(data, &gpioId)) {
        HDF_LOGE("GpioOnDevEventReceive id %d read sbuf fail", id);
        return HDF_ERR_IO;
    }

    HDF_LOGD("GpioOnDevEventReceive event %d gpioId:%d == gpio:%d", id, gpioId, gpio->gpio);
    if ((id == PLATFORM_LISTENER_EVENT_GPIO_INT_NOTIFY) && (gpioId == gpio->gpio)) {
        gpio->func(gpioId, gpio->data);
    }
    return HDF_SUCCESS;
}

static struct PlatformUserListener *PlatformUserListenerInit(
    const struct PlatformUserListenerManager *manager, uint32_t num, void *data, OnEventReceived callback)
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

    listener->callBack = callback;
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

int32_t PlatformUserListenerReg(
    struct PlatformUserListenerManager *manager, uint32_t num, void *data, OnEventReceived callback)
{
    struct PlatformUserListener *pos = NULL;
    struct PlatformUserListener *node = NULL;
    if (manager == NULL || callback == NULL) {
        HDF_LOGE("PlatformUserListenerReg param null");
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

    node = PlatformUserListenerInit(manager, num, data, callback);
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
