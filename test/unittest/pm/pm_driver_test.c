/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "devmgr_service.h"
#include "devsvc_manager_clnt.h"
#include "hdf_device_desc.h"
#include "hdf_log.h"
#include "hdf_pm.h"
#include "osal_time.h"

#define HDF_LOG_TAG pm_driver_test
#define PM_TEST_COUNT 4
#define PM_WAIT_TIME 500

static int resumeCnt = 0;
static int suspendCnt = 0;

int32_t HdfPmTest()
{
    struct HdfDeviceObject *pm_test_service_obj = NULL;
    struct HdfDeviceObject *sample_service_obj = NULL;

    HDF_LOGI("%s enter!", __func__);
    resumeCnt = 0;
    suspendCnt = 0;
    sample_service_obj = DevSvcManagerClntGetDeviceObject("sample_service");
    if (sample_service_obj == NULL) {
         HDF_LOGE("%s sample_service DeviceObject is null!", __func__);
         return HDF_FAILURE;
    }

    pm_test_service_obj = DevSvcManagerClntGetDeviceObject("pm_test_service");
    if (pm_test_service_obj == NULL) {
         HDF_LOGE("%s pm_test_service DeviceObject is null!", __func__);
         return HDF_FAILURE;
    }

    HdfPmSetMode(sample_service_obj, HDF_POWER_DYNAMIC_CTRL);
    HdfPmSetMode(pm_test_service_obj, HDF_POWER_DYNAMIC_CTRL);

    HdfPmAcquireDevice(pm_test_service_obj);
    HdfPmReleaseDevice(pm_test_service_obj);
    HdfPmAcquireDevice(pm_test_service_obj);
    HdfPmReleaseDevice(pm_test_service_obj);
    HdfPmAcquireDevice(pm_test_service_obj);
    HdfPmAcquireDevice(pm_test_service_obj);
    HdfPmReleaseDevice(pm_test_service_obj);
    HdfPmReleaseDevice(pm_test_service_obj);

    HdfPmAcquireDevice(pm_test_service_obj);
    HdfPmAcquireDevice(sample_service_obj);
    HdfPmReleaseDevice(pm_test_service_obj);
    HdfPmReleaseDevice(sample_service_obj);
    OsalMSleep(PM_WAIT_TIME);
    HdfPmSetMode(sample_service_obj, HDF_POWER_SYS_CTRL);
    HdfPmSetMode(pm_test_service_obj, HDF_POWER_SYS_CTRL);
    HDF_LOGI("%s count:%d %d!", __func__, resumeCnt, suspendCnt);
    HDF_LOGI("%s exit!", __func__);
    return ((resumeCnt == PM_TEST_COUNT) && (suspendCnt == PM_TEST_COUNT)) ? HDF_SUCCESS : HDF_FAILURE;
}

void HdfPmDriverRelease(struct HdfDeviceObject *deviceObject)
{
    (void)deviceObject;
    return;
}

int32_t PmDriverDispatch(struct HdfDeviceIoClient *client, int cmdId, struct HdfSBuf *data, struct HdfSBuf *reply)
{
    HDF_LOGI("%s enter!", __func__);

    return HdfPmTest();
}

int HdfPmDriverBind(struct HdfDeviceObject *deviceObject)
{
    HDF_LOGI("%s enter", __func__);
    if (deviceObject == NULL) {
        return HDF_FAILURE;
    }
    static struct IDeviceIoService testService = {
        .Dispatch = PmDriverDispatch,
        .Open = NULL,
        .Release = NULL,
    };
    deviceObject->service = &testService;
    return HDF_SUCCESS;
}

int HdfPmDozeResume(struct HdfDeviceObject *deviceObject)
{
    HDF_LOGI("%s called", __func__);
    return HDF_SUCCESS;
}

int HdfPmDozeSuspend(struct HdfDeviceObject *deviceObject)
{
    HDF_LOGI("%s called", __func__);
    return HDF_SUCCESS;
}

int HdfPmResume(struct HdfDeviceObject *deviceObject)
{
    HDF_LOGI("%s called", __func__);
    resumeCnt++;
    return HDF_SUCCESS;
}

int HdfPmSuspend(struct HdfDeviceObject *deviceObject)
{
    HDF_LOGI("%s called", __func__);
    suspendCnt++;
    return HDF_SUCCESS;
}

struct PmDriverPmListener {
    struct IPowerEventListener powerListener;
    void *p;
};

int HdfPmDriverInit(struct HdfDeviceObject *deviceObject)
{
    HDF_LOGI("%s enter!", __func__);
    if (deviceObject == NULL) {
        HDF_LOGE("%s ptr is null!", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGD("%s Init success", __func__);

    static struct PmDriverPmListener pmListener = {0};
    pmListener.powerListener.DozeResume = HdfPmDozeResume;
    pmListener.powerListener.DozeSuspend = HdfPmDozeSuspend;
    pmListener.powerListener.Resume = HdfPmResume;
    pmListener.powerListener.Suspend = HdfPmSuspend;

    int ret = HdfPmRegisterPowerListener(deviceObject, &pmListener.powerListener);
    HDF_LOGI("%s register power listener, ret = %d", __func__, ret);

    return HDF_SUCCESS;
}

struct HdfDriverEntry g_pmDriverEntry = {
    .moduleVersion = 1,
    .moduleName = "pm_test_driver",
    .Bind = HdfPmDriverBind,
    .Init = HdfPmDriverInit,
    .Release = HdfPmDriverRelease,
};

HDF_INIT(g_pmDriverEntry);

