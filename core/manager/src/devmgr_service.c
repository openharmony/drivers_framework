/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "devmgr_service.h"
#include "devhost_service_clnt.h"
#include "device_token_clnt.h"
#include "hdf_attribute_manager.h"
#include "hdf_base.h"
#include "hdf_driver_installer.h"
#include "hdf_host_info.h"
#include "hdf_log.h"
#include "hdf_object_manager.h"
#include "power_state_manager.h"


#define HDF_LOG_TAG devmgr_service

static int DevmgrServiceActiveDevice(struct DevHostServiceClnt *hostClnt, struct HdfDeviceInfo *deviceInfo, bool isLoad)
{
    struct IDevHostService *devHostSvcIf = (struct IDevHostService *)hostClnt->hostService;
    if (isLoad && (deviceInfo->preload != DEVICE_PRELOAD_ENABLE)) {
        int ret = devHostSvcIf->AddDevice(devHostSvcIf, deviceInfo);
        if (ret == HDF_SUCCESS) {
            deviceInfo->preload = DEVICE_PRELOAD_ENABLE;
        }
        return ret;
    } else if (!isLoad && (deviceInfo->preload != DEVICE_PRELOAD_DISABLE)) {
        devHostSvcIf->DelDevice(devHostSvcIf, deviceInfo);
        deviceInfo->preload = DEVICE_PRELOAD_DISABLE;
        return HDF_SUCCESS;
    } else {
        return HDF_FAILURE;
    }
}

static int DevmgrServiceFindAndActiveDevice(const char *svcName, bool isLoad)
{
    struct HdfSListIterator itHost;
    struct HdfSListIterator itDeviceInfo;
    struct HdfDeviceInfo *deviceInfo = NULL;
    struct DevHostServiceClnt *hostClnt = NULL;
    struct DevmgrService *devMgrSvc = (struct DevmgrService *)DevmgrServiceGetInstance();
    if (devMgrSvc == NULL || svcName == NULL) {
        return HDF_ERR_INVALID_PARAM;
    }

    HdfSListIteratorInit(&itHost, &devMgrSvc->hosts);
    while (HdfSListIteratorHasNext(&itHost)) {
        hostClnt = (struct DevHostServiceClnt *)HdfSListIteratorNext(&itHost);
        HdfSListIteratorInit(&itDeviceInfo, hostClnt->deviceInfos);
        while (HdfSListIteratorHasNext(&itDeviceInfo)) {
            deviceInfo = (struct HdfDeviceInfo *)HdfSListIteratorNext(&itDeviceInfo);
            if (strcmp(deviceInfo->svcName, svcName) == 0) {
                return DevmgrServiceActiveDevice(hostClnt, deviceInfo, isLoad);
            }
        }
    }
    return HDF_FAILURE;
}

int32_t DevmgrServiceLoadLeftDriver(struct DevmgrService *devMgrSvc)
{
    int32_t ret;
    struct HdfSListIterator itHost;
    struct HdfSListIterator itDeviceInfo;
    struct HdfDeviceInfo *deviceInfo = NULL;
    struct DevHostServiceClnt *hostClnt = NULL;
    if (devMgrSvc == NULL) {
        return HDF_FAILURE;
    }

    HdfSListIteratorInit(&itHost, &devMgrSvc->hosts);
    while (HdfSListIteratorHasNext(&itHost)) {
        hostClnt = (struct DevHostServiceClnt *)HdfSListIteratorNext(&itHost);
        HdfSListIteratorInit(&itDeviceInfo, hostClnt->deviceInfos);
        while (HdfSListIteratorHasNext(&itDeviceInfo)) {
            deviceInfo = (struct HdfDeviceInfo *)HdfSListIteratorNext(&itDeviceInfo);
            if (deviceInfo->preload == DEVICE_PRELOAD_ENABLE_STEP2) {
                ret = DevmgrServiceActiveDevice(hostClnt, deviceInfo, true);
                if (ret != HDF_SUCCESS) {
                    HDF_LOGE("%s load driver %s failed!", __func__, deviceInfo->moduleName);
                }
            }
        }
    }
    return HDF_SUCCESS;
}


int DevmgrServiceLoadDevice(const char *svcName)
{
    return DevmgrServiceFindAndActiveDevice(svcName, true);
}

int DevmgrServiceUnLoadDevice(const char *svcName)
{
    return DevmgrServiceFindAndActiveDevice(svcName, false);
}

static struct DevHostServiceClnt *DevmgrServiceFindDeviceHost(struct IDevmgrService *inst, uint16_t hostId)
{
    struct HdfSListIterator it;
    struct DevHostServiceClnt *hostClnt = NULL;
    struct DevmgrService *dmService = (struct DevmgrService *)inst;
    if (dmService == NULL) {
        HDF_LOGE("Find device host failed, dmService is null");
        return NULL;
    }
    HdfSListIteratorInit(&it, &dmService->hosts);
    while (HdfSListIteratorHasNext(&it)) {
        hostClnt = (struct DevHostServiceClnt *)HdfSListIteratorNext(&it);
        if (hostClnt->hostId == hostId) {
            return hostClnt;
        }
    }
    HDF_LOGE("Find host failed, host id is %u", hostId);
    return NULL;
}

static int DevmgrServiceAttachDevice(
    struct IDevmgrService *inst, const struct HdfDeviceInfo *deviceInfo, struct IHdfDeviceToken *token)
{
    if (deviceInfo == NULL) {
        HDF_LOGE("deviceInfo is null");
        return HDF_FAILURE;
    }
    struct DevHostServiceClnt *hostClnt = DevmgrServiceFindDeviceHost(inst, deviceInfo->hostId);
    if (hostClnt == NULL) {
        HDF_LOGE("hostClnt is null");
        return HDF_FAILURE;
    }
    struct DeviceTokenClnt *tokenClnt = DeviceTokenClntNewInstance(token);
    if (tokenClnt == NULL) {
        HDF_LOGE("tokenClnt is null");
        return HDF_FAILURE;
    }

    tokenClnt->deviceInfo = deviceInfo;
    HdfSListAdd(&hostClnt->devices, &tokenClnt->node);
    return HDF_SUCCESS;
}

static int DevmgrServiceAttachDeviceHost(
    struct IDevmgrService *inst, uint16_t hostId, struct IDevHostService *hostService)
{
    struct DevHostServiceClnt *hostClnt = DevmgrServiceFindDeviceHost(inst, hostId);
    if (hostClnt == NULL) {
        HDF_LOGE("Attach device host failed, hostClnt is null");
        return HDF_FAILURE;
    }
    if (hostService == NULL) {
        HDF_LOGE("Attach device host failed, hostService is null");
        return HDF_FAILURE;
    }
    hostClnt->deviceInfos = HdfAttributeManagerGetDeviceList(hostClnt->hostId, hostClnt->hostName);
    if (hostClnt->deviceInfos == NULL) {
        HDF_LOGE("Get device list failed");
        return HDF_FAILURE;
    }
    hostClnt->devCount = HdfSListCount(hostClnt->deviceInfos);
    hostClnt->hostService = hostService;
    return DevHostServiceClntInstallDriver(hostClnt);
}

static int DevmgrServiceStartDeviceHosts(struct DevmgrService *inst)
{
    struct HdfSList hostList;
    struct HdfSListIterator it;
    struct HdfHostInfo *hostAttr = NULL;
    struct DevHostServiceClnt *hostClnt = NULL;
    struct IDriverInstaller *installer = NULL;
    installer = DriverInstallerGetInstance();
    if ((installer == NULL) || (installer->StartDeviceHost == NULL)) {
        HDF_LOGE("installer or installer->StartDeviceHost is null");
        return HDF_FAILURE;
    }
    HdfSListInit(&hostList);
    if (!HdfAttributeManagerGetHostList(&hostList)) {
        HDF_LOGW("%s get host list is null", __func__);
        return HDF_SUCCESS;
    }
    HdfSListIteratorInit(&it, &hostList);
    while (HdfSListIteratorHasNext(&it)) {
        hostAttr = (struct HdfHostInfo *)HdfSListIteratorNext(&it);
        hostClnt = DevHostServiceClntNewInstance(hostAttr->hostId, hostAttr->hostName);
        if (hostClnt == NULL) {
            HDF_LOGW("Create new device host client failed");
            continue;
        }
        HdfSListAdd(&inst->hosts, &hostClnt->node);
        hostClnt->hostPid = installer->StartDeviceHost(hostAttr->hostId, hostAttr->hostName);
        if (hostClnt->hostPid == HDF_FAILURE) {
            HDF_LOGW("Start device host failed, host id is %u", hostAttr->hostId);
            HdfSListRemove(&inst->hosts, &hostClnt->node);
            DevHostServiceClntFreeInstance(hostClnt);
        }
    }
    HdfSListFlush(&hostList, HdfHostInfoDelete);
    return HDF_SUCCESS;
}

int DevmgrServiceStartService(struct IDevmgrService *inst)
{
    struct DevmgrService *dmService = (struct DevmgrService *)inst;
    if (dmService == NULL) {
        HDF_LOGE("Start device manager service failed, dmService is null");
        return HDF_FAILURE;
    }
    return DevmgrServiceStartDeviceHosts(dmService);
}

bool DevmgrServiceConstruct(struct DevmgrService *inst)
{
    if (OsalMutexInit(&inst->devMgrMutex) != HDF_SUCCESS) {
        HDF_LOGE("%s mutex init failed", __func__);
        return false;
    }
    struct IDevmgrService *devMgrSvcIf = (struct IDevmgrService *)inst;
    if (devMgrSvcIf != NULL) {
        devMgrSvcIf->AttachDevice = DevmgrServiceAttachDevice;
        devMgrSvcIf->AttachDeviceHost = DevmgrServiceAttachDeviceHost;
        devMgrSvcIf->StartService = DevmgrServiceStartService;
        devMgrSvcIf->AcquireWakeLock = DevmgrServiceAcquireWakeLock;
        devMgrSvcIf->ReleaseWakeLock = DevmgrServiceReleaseWakeLock;
        HdfSListInit(&inst->hosts);
    }
    return true;
}


struct HdfObject *DevmgrServiceCreate()
{
    static bool isDevMgrServiceInit = false;
    static struct DevmgrService devmgrServiceInstance;
    if (!isDevMgrServiceInit) {
        if (!DevmgrServiceConstruct(&devmgrServiceInstance)) {
            return NULL;
        }
        isDevMgrServiceInit = true;
    }
    return (struct HdfObject *)&devmgrServiceInstance;
}

struct IDevmgrService *DevmgrServiceGetInstance()
{
    static struct IDevmgrService *instance = NULL;
    if (instance == NULL) {
        instance = (struct IDevmgrService *)HdfObjectManagerGetObject(HDF_OBJECT_ID_DEVMGR_SERVICE);
    }
    return instance;
}

void DevmgrServiceRelease(struct HdfObject *object)
{
    struct DevmgrService *devmgrService = (struct DevmgrService *)object;
    if (devmgrService == NULL) {
        return;
    }
    HdfSListFlush(&devmgrService->hosts, DevHostServiceClntDelete);
    OsalMutexDestroy(&devmgrService->devMgrMutex);
}

void DevmgrServiceAcquireWakeLock(struct IDevmgrService *inst, struct IPowerStateToken *tokenIf)
{
    (void)inst;
    struct PowerStateManager *stateManager = PowerStateManagerGetInstance();
    if ((stateManager != NULL) && (stateManager->AcquireWakeLock != NULL)) {
        stateManager->AcquireWakeLock(stateManager, tokenIf);
    }
}

void DevmgrServiceReleaseWakeLock(struct IDevmgrService *inst, struct IPowerStateToken *tokenIf)
{
    (void)inst;
    struct PowerStateManager *stateManager = PowerStateManagerGetInstance();
    if ((stateManager != NULL) && (stateManager->ReleaseWakeLock != NULL)) {
        stateManager->ReleaseWakeLock(stateManager, tokenIf);
    }
}
