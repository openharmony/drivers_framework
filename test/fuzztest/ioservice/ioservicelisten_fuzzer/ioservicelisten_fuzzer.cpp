/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "ioservicelisten_fuzzer.h"
#include <cinttypes>
#include <cstddef>
#include <cstdint>
#include "hdf_base.h"
#include "hdf_log.h"
#include "hdf_io_service.h"
#include "osal_time.h"

namespace OHOS {
struct Eventlistener {
    struct HdfDevEventlistener listener;
    int32_t eventCount;
};
static int OnDevEventReceived(
    struct HdfDevEventlistener *listener, struct HdfIoService *service, uint32_t id, struct HdfSBuf *data);
static int eventCount = 0;

static struct Eventlistener listener0 = {
    .listener.onReceive = OnDevEventReceived,
    .listener.priv = (void *)"listener0",
    .eventCount = 0,
};

static int OnDevEventReceived(
    struct HdfDevEventlistener *listener, struct HdfIoService *service, uint32_t id, struct HdfSBuf *data)
{
    OsalTimespec time;
    OsalGetTime(&time);
    HDF_LOGI("%{public}s: received event[%{public}d] from %{public}s at %" PRIu64 ".%" PRIu64 "",
        (char *)listener->priv, eventCount++, (char *)service->priv, time.sec, time.usec);

    const char *string = HdfSbufReadString(data);
    if (string == nullptr) {
        HDF_LOGE("failed to read string in event data");
        return HDF_SUCCESS;
    }
    struct Eventlistener *listenercount = CONTAINER_OF(listener, struct Eventlistener, listener);
    listenercount->eventCount++;
    HDF_LOGI("%{public}s: dev event received: %{public}u %{public}s", (char *)service->priv, id, string);
    return HDF_SUCCESS;
}

bool IoserviceListenFuzzTest(const uint8_t *data, size_t size)
{
    bool result = false;
    struct HdfIoService *serv = HdfIoServiceBind((const char *)data);
    if (serv == nullptr) {
        return false;
    }
    if (HdfDeviceRegisterEventListener(serv, &listener0.listener) == HDF_SUCCESS) {
        int ret = HdfDeviceUnregisterEventListener(serv, &listener0.listener);
        if (ret != HDF_SUCCESS) {
            HdfIoServiceRecycle(serv);
            return false;
        }
        result = true;
    }
    HdfIoServiceRecycle(serv);
    return result;
}
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::IoserviceListenFuzzTest(data, size);
    return HDF_SUCCESS;
}
