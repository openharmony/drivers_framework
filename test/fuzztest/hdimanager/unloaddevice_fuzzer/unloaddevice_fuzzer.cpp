/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "unloaddevice_fuzzer.h"
#include <cstddef>
#include <cstdint>
#include <hdf_base.h>
#include <hdi_base.h>
#include "idevmgr_hdi.h"

using namespace OHOS::HDI::DeviceManager::V1_0;

static constexpr const char *TEST_SERVICE_NAME = "sample_driver_service";

namespace OHOS {
sptr<IDeviceManager> g_devicemanager = IDeviceManager::Get();

bool UnloadDeviceFuzzTest(const uint8_t *data, size_t size)
{
    bool result = false;
    std::string serviceName((const char *)data);
    if (g_devicemanager != nullptr && g_devicemanager->LoadDevice(TEST_SERVICE_NAME) == HDF_SUCCESS) {
        result = true;
    }

    if (g_devicemanager != nullptr && g_devicemanager->UnloadDevice(serviceName) == HDF_SUCCESS) {
        result = true;
    }
    return result;
}
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::UnloadDeviceFuzzTest(data, size);
    return HDF_SUCCESS;
}
