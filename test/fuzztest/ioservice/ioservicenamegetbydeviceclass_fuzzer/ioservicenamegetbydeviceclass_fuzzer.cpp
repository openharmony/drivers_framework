/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "ioservicenamegetbydeviceclass_fuzzer.h"
#include <cstddef>
#include <cstdint>
#include <hdf_sbuf.h>
#include "hdf_base.h"
#include "hdf_log.h"
#include "hdf_io_service_if.h"

namespace OHOS {
bool IoserviceNameGetByDeviceClassFuzzTest(const uint8_t *data, size_t size)
{
    bool result = false;
    if (data == nullptr) {
        HDF_LOGE("%{public}s: data is nullptr!", __func__);
        return false;
    }

    struct HdfSBuf *reply = HdfSbufObtainDefaultSize();
    int32_t ret = HdfGetServiceNameByDeviceClass(static_cast<DeviceClass>(*data), reply);
    if (ret == HDF_SUCCESS) {
        result = true;
    }
    HdfSbufRecycle(reply);
    return result;
}
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::IoserviceNameGetByDeviceClassFuzzTest(data, size);
    return HDF_SUCCESS;
}
