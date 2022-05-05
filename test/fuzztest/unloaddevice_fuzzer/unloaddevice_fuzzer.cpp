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
#include "idevmgr_hdi.h"

using namespace OHOS::HDI::DeviceManager::V1_0;

static constexpr const char *TEST_SERVICE_NAME = "sample_driver_service";

namespace OHOS {
bool UnloadDeviceFuzzTest(const uint8_t *data, size_t size)
{
    bool result = false;
    std::string serviceName((const char *)data);
    sptr<IDeviceManager> devicemanager = IDeviceManager::Get();
    if (!devicemanager->LoadDevice(TEST_SERVICE_NAME)) {
        result = true;
    }

    if (!devicemanager->UnloadDevice(serviceName)) {
        result = true;
    }
    return result;
}
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::UnloadDeviceFuzzTest(data, size);
    return 0;
}
