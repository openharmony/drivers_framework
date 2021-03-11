/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "hdf_device_object.h"
#include "hdf_base.h"
#include "hdf_device_node.h"
#include "hdf_log.h"
#include "hdf_observer_record.h"
#include "hdf_service_observer.h"
#include "power_state_token.h"

#define HDF_LOG_TAG device_object

int32_t HdfDeviceSubscribeService(
    struct HdfDeviceObject *deviceObject, const char *serviceName, struct SubscriberCallback callback)
{
    uint32_t matchId;
    struct DevHostService *hostService = NULL;
    const struct HdfDeviceInfo *deviceInfo = NULL;
    if (deviceObject == NULL || serviceName == NULL) {
        HDF_LOGE("%s input param is invalid", __func__);
        return HDF_FAILURE;
    }
    struct HdfDeviceNode *devNode = (struct HdfDeviceNode *)HDF_SLIST_CONTAINER_OF(
        struct HdfDeviceObject, deviceObject, struct HdfDeviceNode, deviceObject);
    hostService = devNode->hostService;
    if (hostService == NULL) {
        HDF_LOGE("Get host service is null");
        return HDF_FAILURE;
    }
    deviceInfo = devNode->deviceInfo;
    if (deviceInfo == NULL) {
        HDF_LOGE("Get device deviceInfo is null");
        return HDF_FAILURE;
    }
    matchId = HdfMakeHardwareId(deviceInfo->hostId, deviceInfo->deviceId);
    return HdfServiceObserverSubscribeService(&hostService->observer, serviceName, matchId, callback);
}

const char *HdfDeviceGetServiceName(const struct HdfDeviceObject *deviceObject)
{
    if (deviceObject == NULL) {
        HDF_LOGE("%s input param is invalid", __func__);
        return NULL;
    }
    struct HdfDeviceNode *devNode = (struct HdfDeviceNode *)HDF_SLIST_CONTAINER_OF(
        struct HdfDeviceObject, deviceObject, struct HdfDeviceNode, deviceObject);
    const struct HdfDeviceInfo *deviceInfo = devNode->deviceInfo;
    if (deviceInfo == NULL) {
        HDF_LOGE("Get device deviceInfo is null");
        return NULL;
    }
    return deviceInfo->svcName;
}

void HdfDeviceRegisterPowerListener(struct HdfDeviceObject *deviceObject, struct IPowerEventListener *listener)
{
    if (deviceObject == NULL) {
        HDF_LOGE("%s input param is invalid", __func__);
        return;
    }
    struct HdfDeviceNode *devNode = (struct HdfDeviceNode *)HDF_SLIST_CONTAINER_OF(
        struct HdfDeviceObject, deviceObject, struct HdfDeviceNode, deviceObject);
    HdfDeviceNodeAddPowerStateListener(devNode, listener);
}

void HdfDeviceAcquireWakeLock(struct HdfDeviceObject *deviceObject)
{
    if (deviceObject == NULL) {
        HDF_LOGE("%s input param is invalid", __func__);
        return;
    }
    struct HdfDeviceNode *devNode = (struct HdfDeviceNode *)HDF_SLIST_CONTAINER_OF(
        struct HdfDeviceObject, deviceObject, struct HdfDeviceNode, deviceObject);
    struct IPowerStateToken *tokenIf = (struct IPowerStateToken *)devNode->powerToken;
    if ((tokenIf != NULL) && (tokenIf->AcquireWakeLock != NULL)) {
        tokenIf->AcquireWakeLock(tokenIf);
    }
}

void HdfDeviceReleaseWakeLock(struct HdfDeviceObject *deviceObject)
{
    if (deviceObject == NULL) {
        HDF_LOGE("%s input param is invalid", __func__);
        return;
    }
    struct HdfDeviceNode *devNode = (struct HdfDeviceNode *)HDF_SLIST_CONTAINER_OF(
        struct HdfDeviceObject, deviceObject, struct HdfDeviceNode, deviceObject);
    struct IPowerStateToken *tokenIf = (struct IPowerStateToken *)devNode->powerToken;
    if ((tokenIf != NULL) && (tokenIf->ReleaseWakeLock != NULL)) {
        tokenIf->ReleaseWakeLock(tokenIf);
    }
}

bool HdfDeviceSetClass(struct HdfDeviceObject *deviceObject, DeviceClass deviceClass)
{
    if ((deviceObject == NULL) || (deviceClass >= DEVICE_CLASS_MAX) ||
        (deviceClass < DEVICE_CLASS_DEFAULT)) {
        return false;
    }
    deviceObject->deviceClass = deviceClass;
    return true;
}

void HdfDeviceObjectConstruct(struct HdfDeviceObject *deviceObject)
{
    if (deviceObject != NULL) {
        deviceObject->property = NULL;
        deviceObject->service = NULL;
        deviceObject->deviceClass = DEVICE_CLASS_DEFAULT;
    }
}
