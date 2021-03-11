/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "power_state_token.h"
#include "devmgr_service_clnt.h"
#include "hdf_device_desc.h"
#include "hdf_slist.h"
#include "osal_mem.h"

static void PowerStateTokenAcqureWakeLock(struct IPowerStateToken *token)
{
    struct HdfSRef *sref = NULL;
    struct PowerStateToken *stateToken = (struct PowerStateToken *)token;
    if (stateToken == NULL) {
        return;
    }
    sref = (struct HdfSRef *)&stateToken->wakeRef;
    if ((sref != NULL) && (sref->Acquire != NULL)) {
        sref->Acquire(sref);
    }
}

static void PowerStateTokenReleaseWakeLock(struct IPowerStateToken *token)
{
    struct HdfSRef *sref = NULL;
    struct PowerStateToken *stateToken = (struct PowerStateToken *)token;
    if (stateToken == NULL) {
        return;
    }
    sref = (struct HdfSRef *)&stateToken->wakeRef;
    if ((sref != NULL) && (sref->Release != NULL)) {
        sref->Release(sref);
    }
}

static void PowerStateTokenOnFirstAcquire(struct HdfSRef *sref)
{
    if (sref == NULL) {
        return;
    }
    struct PowerStateToken *stateToken = (struct PowerStateToken *)HDF_SLIST_CONTAINER_OF(
        struct HdfSRef, sref, struct PowerStateToken, wakeRef);
    if (stateToken->state != POWER_STATE_ACTIVE) {
        struct IDevmgrService *devMgrSvcIf = NULL;
        struct IPowerEventListener* listener = stateToken->listener;
        struct DevmgrServiceClnt *inst = DevmgrServiceClntGetInstance();
        if (inst == NULL) {
            return;
        }
        devMgrSvcIf = (struct IDevmgrService *)inst->devMgrSvcIf;
        if (devMgrSvcIf == NULL || devMgrSvcIf->AcquireWakeLock == NULL) {
            return;
        }
        devMgrSvcIf->AcquireWakeLock(devMgrSvcIf, &stateToken->super);
        if (stateToken->state == POWER_STATE_INACTIVE) {
            if ((listener != NULL) && (listener->Resume != NULL)) {
                listener->Resume(stateToken->deviceObject);
            }
        }
        stateToken->state = POWER_STATE_ACTIVE;
    }
}

static void PowerStateTokenOnLastRelease(struct HdfSRef *sref)
{
    if (sref == NULL) {
        return;
    }
    struct PowerStateToken *stateToken = (struct PowerStateToken *)HDF_SLIST_CONTAINER_OF(
        struct HdfSRef, sref, struct PowerStateToken, wakeRef);
    if (stateToken->state == POWER_STATE_ACTIVE) {
        struct IDevmgrService *devMgrSvcIf = NULL;
        struct IPowerEventListener *listener = stateToken->listener;
        struct DevmgrServiceClnt *inst = DevmgrServiceClntGetInstance();
        if (inst == NULL) {
            return;
        }
        devMgrSvcIf = (struct IDevmgrService *)inst->devMgrSvcIf;
        if ((devMgrSvcIf == NULL) || (devMgrSvcIf->AcquireWakeLock == NULL)) {
            return;
        }
        devMgrSvcIf->ReleaseWakeLock(devMgrSvcIf, &stateToken->super);
        if ((listener != NULL) && (listener->Suspend != NULL)) {
            listener->Suspend(stateToken->deviceObject);
        }
        stateToken->state = POWER_STATE_INACTIVE;
    }
}

static void PowerStateTokenConstruct(
    struct PowerStateToken *powerStateToken, struct HdfDeviceObject *deviceObject, struct IPowerEventListener *listener)
{
    struct IPowerStateToken *tokenIf = &powerStateToken->super;
    struct IHdfSRefListener *srefListener = (struct IHdfSRefListener *)OsalMemCalloc(sizeof(struct IHdfSRefListener));
    if (srefListener == NULL) {
        return;
    }

    tokenIf->AcquireWakeLock = PowerStateTokenAcqureWakeLock;
    tokenIf->ReleaseWakeLock = PowerStateTokenReleaseWakeLock;

    srefListener->OnFirstAcquire = PowerStateTokenOnFirstAcquire;
    srefListener->OnLastRelease = PowerStateTokenOnLastRelease;

    powerStateToken->state = POWER_STATE_IDLE;
    powerStateToken->listener = listener;
    powerStateToken->deviceObject = deviceObject;
    HdfSRefConstruct(&powerStateToken->wakeRef, srefListener);
}

struct PowerStateToken *PowerStateTokenNewInstance(
    struct HdfDeviceObject *deviceObject, struct IPowerEventListener *listener)
{
    struct PowerStateToken *stateToken =
        (struct PowerStateToken *)OsalMemCalloc(sizeof(struct PowerStateToken));
    if (stateToken != NULL) {
        PowerStateTokenConstruct(stateToken, deviceObject, listener);
    }
    return stateToken;
}

void PowerStateTokenFreeInstance(struct PowerStateToken *stateToken)
{
    if (stateToken != NULL) {
        if (stateToken->wakeRef.listener != NULL) {
            OsalMemFree(stateToken->wakeRef.listener);
            stateToken->wakeRef.listener = NULL;
        }
        OsalMemFree(stateToken);
    }
}
