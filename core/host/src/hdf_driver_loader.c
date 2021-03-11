/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "hdf_driver_loader.h"
#include "devsvc_manager_clnt.h"
#include "hcs_tree_if.h"
#include "hdf_device_desc.h"
#include "hdf_device_node.h"
#include "hdf_log.h"
#include "hdf_object_manager.h"
#include "hdf_attribute_manager.h"

#define HDF_LOG_TAG driver_loader

struct HdfDeviceNode *HdfDriverLoaderLoadNode(
    struct IDriverLoader *loader, const struct HdfDeviceInfo *deviceInfo)
{
    struct HdfDriverEntry *driverEntry = NULL;
    struct HdfDeviceNode *devNode = NULL;
    if ((loader == NULL) || (loader->GetDriverEntry == NULL)) {
        HDF_LOGE("loader or loader->GetDriverEntry is null");
        return NULL;
    }

    driverEntry = loader->GetDriverEntry(deviceInfo);
    if (driverEntry == NULL) {
        HDF_LOGE("Load service failed, deviceEntry is null");
        return NULL;
    }

    devNode = HdfDeviceNodeNewInstance();
    if (devNode == NULL) {
        HDF_LOGE("Load service failed, device node is null");
        return NULL;
    }

    devNode->driverEntry = driverEntry;
    devNode->deviceInfo = deviceInfo;
    devNode->deviceObject.property = HcsGetNodeByMatchAttr(HdfGetRootNode(), deviceInfo->deviceMatchAttr);
    if (devNode->deviceObject.property == NULL) {
        HDF_LOGW("Get property is null, match attr is: %s", deviceInfo->deviceMatchAttr);
    }

    if ((deviceInfo->policy == SERVICE_POLICY_PUBLIC) || (deviceInfo->policy == SERVICE_POLICY_CAPACITY)) {
        if (driverEntry->Bind == NULL) {
            HDF_LOGE("driver bind method is null");
            HdfDeviceNodeFreeInstance(devNode);
            return NULL;
        }
        if (driverEntry->Bind(&devNode->deviceObject) != 0) {
            HDF_LOGE("bind driver failed");
            HdfDeviceNodeFreeInstance(devNode);
            return NULL;
        }
    }
    return devNode;
}

static void HdfDriverLoaderUnLoadNode(struct IDriverLoader *loader, const struct HdfDeviceInfo *deviceInfo)
{
    struct HdfDriverEntry *driverEntry = NULL;
    struct HdfDeviceObject *deviceObject = NULL;
    if ((loader == NULL) || (loader->GetDriverEntry == NULL)) {
        HDF_LOGE("loader or loader->GetDriverEntry is null");
        return;
    }

    driverEntry = loader->GetDriverEntry(deviceInfo);
    if (driverEntry == NULL) {
        HDF_LOGE("Load service failed, driverEntry is null");
        return;
    }
    if (driverEntry->Release == NULL) {
        HDF_LOGI("Device Release func is null");
        return;
    }
    deviceObject = DevSvcManagerClntGetDeviceObject(deviceInfo->svcName);
    driverEntry->Release(deviceObject);
}

static void HdfDriverLoaderConstruct(struct HdfDriverLoader *inst)
{
    struct IDriverLoader *driverLoaderIf = (struct IDriverLoader *)inst;
    driverLoaderIf->LoadNode = HdfDriverLoaderLoadNode;
    driverLoaderIf->UnLoadNode = HdfDriverLoaderUnLoadNode;
    driverLoaderIf->GetDriverEntry = HdfDriverLoaderGetDriverEntry;
}

struct HdfObject *HdfDriverLoaderCreate()
{
    static bool isDriverLoaderInit = false;
    static struct HdfDriverLoader driverLoader;
    if (!isDriverLoaderInit) {
        HdfDriverLoaderConstruct(&driverLoader);
        isDriverLoaderInit = true;
    }
    return (struct HdfObject *)&driverLoader;
}

struct IDriverLoader *HdfDriverLoaderGetInstance()
{
    static struct IDriverLoader *instance = NULL;
    if (instance == NULL) {
        instance = (struct IDriverLoader *)HdfObjectManagerGetObject(HDF_OBJECT_ID_DRIVER_LOADER);
    }
    return instance;
}

