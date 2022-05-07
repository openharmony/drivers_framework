/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef REGISTER_SERVICE_STATUS_LISTENER_FUZZER_H
#define REGISTER_SERVICE_STATUS_LISTENER_FUZZER_H

#include "iservstat_listener_hdi.h"

#define FUZZ_PROJECT_NAME "registerservicestatuslistener_fuzzer"

namespace OHOS {
namespace HDI {
namespace ServiceManager {
namespace V1_0 {
class RegisterServStatListenerFuzzer : public ServStatListenerStub {
public:
    virtual ~RegisterServStatListenerFuzzer() {}

    void OnReceive(const ServiceStatus &status) override;
};
} // namespace V1_0
} // namespace ServiceManager
} // namespace HDI
} // namespace OHOS

#endif