/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "devsvc_manager_clnt.h"
#include "devmgr_service.h"
#include "devsvc_manager.h"
#include "hdf_attribute_manager.h"
#include "hdf_base.h"
#include "hdf_log.h"
#include "hdf_object_manager.h"

#define HDF_LOG_TAG devsvc_manager_clnt

int DevSvcManagerClntAddService(const char *svcName, struct HdfDeviceObject *service)
{
    struct DevSvcManagerClnt *devSvcMgrClnt = DevSvcManagerClntGetInstance();
    if (devSvcMgrClnt == NULL) {
        HDF_LOGE("Get device manager client is null");
        return HDF_FAILURE;
    }

    struct IDevSvcManager *serviceManager = devSvcMgrClnt->devSvcMgrIf;
    if (serviceManager == NULL || serviceManager->AddService == NULL) {
        HDF_LOGE("AddService function is not assigned");
        return HDF_FAILURE;
    }
    return serviceManager->AddService(serviceManager, svcName, service);
}

const struct HdfObject *DevSvcManagerClntGetService(const char *svcName)
{
    struct DevSvcManagerClnt *devSvcMgrClnt = DevSvcManagerClntGetInstance();
    if (devSvcMgrClnt == NULL) {
        HDF_LOGE("Get device manager client is null");
        return NULL;
    }

    struct IDevSvcManager *serviceManager = devSvcMgrClnt->devSvcMgrIf;
    if (serviceManager == NULL || serviceManager->GetService == NULL) {
        HDF_LOGE("GetService function is not assigned");
        return NULL;
    }
    return serviceManager->GetService(serviceManager, svcName);
}

struct HdfDeviceObject *DevSvcManagerClntGetDeviceObject(const char *svcName)
{
    struct DevSvcManagerClnt *devSvcMgrClnt = DevSvcManagerClntGetInstance();
    if (devSvcMgrClnt == NULL) {
        HDF_LOGE("Get device manager client is null");
        return NULL;
    }

    struct IDevSvcManager *serviceManager = devSvcMgrClnt->devSvcMgrIf;
    if (serviceManager == NULL || serviceManager->GetObject == NULL) {
        HDF_LOGE("GetObject function is not assigned");
        return NULL;
    }
    return serviceManager->GetObject(serviceManager, svcName);
}

struct HdfDeviceObject *HdfRegisterDevice(const char *moduleName, const char *serviceName)
{
    int ret;
    if (!HdfDeviceListAdd(moduleName, serviceName)) {
        HDF_LOGE("%s device info add failed!", __func__);
        return NULL;
    }
    ret = DevmgrServiceLoadDevice(serviceName);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s load device %s failed!", __func__, serviceName);
        HdfDeviceListDel(moduleName, serviceName);
        return NULL;
    }
    return DevSvcManagerClntGetDeviceObject(serviceName);
}

void HdfUnregisterDevice(const char *moduleName, const char *serviceName)
{
    int ret;
    ret = DevmgrServiceUnLoadDevice(serviceName);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s unload device %s failed!", __func__, serviceName);
    }
    HdfDeviceListDel(moduleName, serviceName);
}

int DevSvcManagerClntSubscribeService(const char *svcName, struct SubscriberCallback callback)
{
    struct DevSvcManagerClnt *devSvcMgrClnt = DevSvcManagerClntGetInstance();
    if (devSvcMgrClnt == NULL) {
        HDF_LOGE("Get device manager client is null");
        return HDF_FAILURE;
    }

    struct IDevSvcManager *serviceManager = devSvcMgrClnt->devSvcMgrIf;
    if (serviceManager == NULL || serviceManager->SubscribeService == NULL) {
        HDF_LOGE("SubscribeService function is not assigned");
        return HDF_FAILURE;
    }
    return serviceManager->SubscribeService(serviceManager, svcName, callback);
}

int DevSvcManagerClntUnsubscribeService(const char *svcName)
{
    struct DevSvcManagerClnt *devSvcMgrClnt = DevSvcManagerClntGetInstance();
    if (devSvcMgrClnt == NULL) {
        HDF_LOGE("Get device manager client is null");
        return HDF_FAILURE;
    }

    struct IDevSvcManager *serviceManager = devSvcMgrClnt->devSvcMgrIf;
    if (serviceManager == NULL || serviceManager->UnsubscribeService == NULL) {
        HDF_LOGE("UnsubService function is not assigned");
        return HDF_FAILURE;
    }
    return serviceManager->UnsubscribeService(serviceManager, svcName);
}

void DevSvcManagerClntRemoveService(const char *svcName)
{
    struct DevSvcManagerClnt *devSvcMgrClnt = DevSvcManagerClntGetInstance();
    if (devSvcMgrClnt == NULL) {
        HDF_LOGE("Get device manager client is null");
        return;
    }

    struct IDevSvcManager *serviceManager = devSvcMgrClnt->devSvcMgrIf;
    if (serviceManager == NULL || serviceManager->RemoveService == NULL) {
        HDF_LOGE("Remove service function is not assigned");
        return;
    }
    serviceManager->RemoveService(serviceManager, svcName);
}

static void DevSvcManagerClntConstruct(struct DevSvcManagerClnt *inst)
{
    inst->devSvcMgrIf = (struct IDevSvcManager *)HdfObjectManagerGetObject(HDF_OBJECT_ID_DEVSVC_MANAGER);
}

struct DevSvcManagerClnt *DevSvcManagerClntGetInstance()
{
    static struct DevSvcManagerClnt *instance = NULL;
    if (instance == NULL) {
        static struct DevSvcManagerClnt singletonInstance;
        DevSvcManagerClntConstruct(&singletonInstance);
        instance = &singletonInstance;
    }
    return instance;
}

void DevSvcManagerClntFreeInstance(struct DevSvcManagerClnt *instance)
{
    if (instance != NULL) {
        if (instance->devSvcMgrIf != NULL) {
            HdfObjectManagerFreeObject((struct HdfObject *)instance->devSvcMgrIf);
        }
    }
}

