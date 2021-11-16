/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "pwm_test.h"
#include "device_resource_if.h"
#include "hdf_base.h"
#include "hdf_log.h"
#include "osal_time.h"

#define HDF_LOG_TAG           pwm_test
#define SEQ_OUTPUT_DELAY      100 /* Delay time of sequential output, unit: ms */
#define OUTPUT_WAVES_DELAY    1 /* Delay time of waves output, unit: second */
#define TEST_WAVES_NUMBER     10 /* The number of waves for test. */

struct PwmTestFunc {
    enum PwmTestCmd type;
    int32_t (*Func)(struct PwmTest *test);
};

static DevHandle PwmTestGetHandle(struct PwmTest *test)
{
    return PwmOpen(test->num);
}

static void PwmTestReleaseHandle(DevHandle handle)
{
    if (handle == NULL) {
        HDF_LOGE("%s: pwm handle is null.", __func__);
        return;
    }
    PwmClose(handle);
}

static int32_t PwmSetConfigTest(struct PwmTest *test)
{
    int32_t ret;
    struct PwmConfig cfg = {0};
    uint32_t number;

    HDF_LOGI("%s: enter. Test [PwmSetConfig].", __func__);
    number = test->cfg.number;
    test->cfg.number = ((number > 0) ? 0 : TEST_WAVES_NUMBER);
    HDF_LOGI("%s: Set number %u.", __func__, test->cfg.number);
    ret = PwmSetConfig(test->handle, &(test->cfg));
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: [PwmSetConfig] failed, ret %d.", __func__, ret);
        return HDF_FAILURE;
    }
    OsalSleep(OUTPUT_WAVES_DELAY);
    test->cfg.number = number;
    HDF_LOGI("%s: Set number %u.", __func__, test->cfg.number);
    ret = PwmSetConfig(test->handle, &(test->cfg));
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: [PwmSetConfig] failed, ret %d.", __func__, ret);
        return HDF_FAILURE;
    }
    ret = PwmGetConfig(test->handle, &cfg);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: [PwmGetConfig] failed, ret %d.", __func__, ret);
        return HDF_FAILURE;
    }
    if (memcmp(&cfg, &(test->cfg), sizeof(cfg)) != 0) {
        HDF_LOGE("%s: [memcmp] failed.", __func__);
        return HDF_FAILURE;
    }

    HDF_LOGI("%s: success.", __func__);
    return ret;
}

static int32_t PwmGetConfigTest(struct PwmTest *test)
{
    int32_t ret;
    struct PwmConfig cfg = {0};

    HDF_LOGI("%s: enter. Test [PwmGetConfig].", __func__);
    ret = PwmGetConfig(test->handle, &cfg);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: [PwmGetConfig] failed, ret %d.", __func__, ret);
        return HDF_FAILURE;
    }
    HDF_LOGI("%s: success.", __func__);
    return ret;
}

static int32_t PwmSetPeriodTest(struct PwmTest *test)
{
    int32_t ret;
    struct PwmConfig cfg = {0};
    uint32_t period;

    period = test->cfg.period + test->originCfg.period;
    HDF_LOGI("%s: enter. Test [PwmSetPeriod] period %u.", __func__, period);
    ret = PwmSetPeriod(test->handle, period);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: [PwmSetPeriod] failed, ret %d.", __func__, ret);
        return HDF_FAILURE;
    }
    ret = PwmGetConfig(test->handle, &cfg);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: [PwmGetConfig] failed, ret %d.", __func__, ret);
        return HDF_FAILURE;
    }
    if (cfg.period != period) {
        HDF_LOGE("%s: failed.", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGI("%s: success.", __func__);
    return ret;
}

static int32_t PwmSetDutyTest(struct PwmTest *test)
{
    int32_t ret;
    struct PwmConfig cfg = {0};
    uint32_t duty;

    duty = test->cfg.duty + test->originCfg.duty;
    HDF_LOGI("%s: enter. Test [PwmSetDuty] duty %u.", __func__, duty);
    ret = PwmSetDuty(test->handle, duty);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: [PwmSetDuty] failed, ret %d.", __func__, ret);
        return HDF_FAILURE;
    }
    ret = PwmGetConfig(test->handle, &cfg);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: [PwmGetConfig] failed, ret %d.", __func__, ret);
        return HDF_FAILURE;
    }
    if (cfg.duty != duty) {
        HDF_LOGE("%s: failed.", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGI("%s: success.", __func__);
    return ret;
}

static int32_t PwmSetPolarityTest(struct PwmTest *test)
{
    int32_t ret;
    struct PwmConfig cfg = {0};

    HDF_LOGI("%s: enter.", __func__);
    test->cfg.polarity = PWM_NORMAL_POLARITY;
    HDF_LOGI("%s: Test [PwmSetPolarity] polarity %u.", __func__, test->cfg.polarity);
    ret = PwmSetPolarity(test->handle, test->cfg.polarity);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: [PwmSetPolarity] failed, ret %d.", __func__, ret);
        return HDF_FAILURE;
    }
    test->cfg.polarity = PWM_INVERTED_POLARITY;
    HDF_LOGI("%s: Test [PwmSetPolarity] polarity %u.", __func__, test->cfg.polarity);
    ret = PwmSetPolarity(test->handle, test->cfg.polarity);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: [PwmSetPolarity] failed, ret %d.", __func__, ret);
        return HDF_FAILURE;
    }
    ret = PwmGetConfig(test->handle, &cfg);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: [PwmGetConfig] failed, ret %d.", __func__, ret);
        return HDF_FAILURE;
    }
    if (cfg.polarity != test->cfg.polarity) {
        HDF_LOGE("%s: failed.", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGI("%s: success.", __func__);
    return ret;
}

static int32_t PwmEnableTest(struct PwmTest *test)
{
    int32_t ret;
    struct PwmConfig cfg = {0};

    HDF_LOGI("%s: enter.", __func__);
    ret = PwmDisable(test->handle);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: [PwmDisable] failed, ret %d.", __func__, ret);
        return HDF_FAILURE;
    }
    HDF_LOGI("%s: Test [PwmEnable] enable.", __func__);
    ret = PwmEnable(test->handle);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: [PwmEnable] failed, ret %d.", __func__, ret);
        return HDF_FAILURE;
    }
    ret = PwmGetConfig(test->handle, &cfg);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: [PwmGetConfig] failed, ret %d.", __func__, ret);
        return HDF_FAILURE;
    }
    if (cfg.status == PWM_DISABLE_STATUS) {
        HDF_LOGE("%s: failed", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGI("%s: success.", __func__);
    return ret;
}

static int32_t PwmDisableTest(struct PwmTest *test)
{
    int32_t ret;
    struct PwmConfig cfg = {0};

    HDF_LOGI("%s: enter.", __func__);
    ret = PwmEnable(test->handle);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: [PwmEnable] failed, ret %d.", __func__, ret);
        return HDF_FAILURE;
    }
    HDF_LOGI("%s: Test [PwmDisable] disable.", __func__);
    ret = PwmDisable(test->handle);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: [PwmDisable] failed, ret %d.", __func__, ret);
        return HDF_FAILURE;
    }
    ret = PwmGetConfig(test->handle, &cfg);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: [PwmGetConfig] failed, ret %d.", __func__, ret);
        return HDF_FAILURE;
    }
    if (cfg.status == PWM_ENABLE_STATUS) {
        HDF_LOGE("%s: failed.", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGI("%s: success.", __func__);
    return ret;
}

#define TEST_PERIOD 2147483647
#define TEST_DUTY 2147483647
#define TEST_POLARITY 10
static int32_t PwmReliabilityTest(struct PwmTest *test)
{
    struct PwmConfig cfg = {0};

    (void)PwmSetConfig(test->handle, &(test->cfg));
    (void)PwmSetConfig(test->handle, NULL);
    (void)PwmGetConfig(test->handle, &cfg);
    (void)PwmGetConfig(test->handle, NULL);

    (void)PwmSetPeriod(test->handle, 0);
    (void)PwmSetPeriod(test->handle, TEST_PERIOD);

    (void)PwmSetDuty(test->handle, 0);
    (void)PwmSetDuty(test->handle, TEST_DUTY);

    (void)PwmSetPolarity(test->handle, 0);
    (void)PwmSetPolarity(test->handle, TEST_POLARITY);

    (void)PwmEnable(test->handle);
    (void)PwmEnable(test->handle);

    (void)PwmDisable(test->handle);
    (void)PwmDisable(test->handle);
    HDF_LOGI("%s: success.", __func__);
    return HDF_SUCCESS;
}

static int32_t PwmTestAll(struct PwmTest *test)
{
    int32_t total = 0;
    int32_t error = 0;

    if (PwmSetPeriodTest(test) != HDF_SUCCESS) {
        error++;
    }
    total++;
    if (PwmSetDutyTest(test) != HDF_SUCCESS) {
        error++;
    }
    total++;
    if (PwmSetPolarityTest(test) != HDF_SUCCESS) {
        error++;
    }
    total++;
    if (PwmGetConfigTest(test) != HDF_SUCCESS) {
        error++;
    }
    total++;
    if (PwmEnableTest(test) != HDF_SUCCESS) {
        error++;
    }
    total++;
    if (PwmSetConfigTest(test) != HDF_SUCCESS) {
        error++;
    }
    total++;
    if (PwmDisableTest(test) != HDF_SUCCESS) {
        error++;
    }
    total++;
    if (PwmReliabilityTest(test) != HDF_SUCCESS) {
        error++;
    }
    total++;

    HDF_LOGI("%s: Pwm Test Total %d Error %d.", __func__, total, error);
    return HDF_SUCCESS;
}

static struct PwmTestFunc g_pwmTestFunc[] = {
    { PWM_SET_PERIOD_TEST, PwmSetPeriodTest },
    { PWM_SET_DUTY_TEST, PwmSetDutyTest },
    { PWM_SET_POLARITY_TEST, PwmSetPolarityTest },
    { PWM_ENABLE_TEST, PwmEnableTest },
    { PWM_DISABLE_TEST, PwmDisableTest },
    { PWM_SET_CONFIG_TEST, PwmSetConfigTest },
    { PWM_GET_CONFIG_TEST, PwmGetConfigTest },
    { PWM_RELIABILITY_TEST, PwmReliabilityTest },
    { PWM_TEST_ALL, PwmTestAll },
};

static int32_t PwmTestEntry(struct PwmTest *test, int32_t cmd)
{
    int32_t i;
    int32_t ret = HDF_ERR_NOT_SUPPORT;

    if (test == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    OsalMSleep(SEQ_OUTPUT_DELAY);
    test->handle = PwmTestGetHandle(test);
    if (test->handle == NULL) {
        HDF_LOGE("%s: pwm test get handle fail.", __func__);
        return HDF_FAILURE;
    }
    // At first test case.
    if (cmd == PWM_SET_PERIOD_TEST) {
        ret = PwmGetConfig(test->handle, &(test->originCfg));
        if (ret != HDF_SUCCESS) {
            HDF_LOGE("%s: [PwmGetConfig] failed, ret %d.", __func__, ret);
            return HDF_FAILURE;
        }
    }
    for (i = 0; i < sizeof(g_pwmTestFunc) / sizeof(g_pwmTestFunc[0]); i++) {
        if (cmd == g_pwmTestFunc[i].type && g_pwmTestFunc[i].Func != NULL) {
            ret = g_pwmTestFunc[i].Func(test);
            HDF_LOGI("%s: cmd %d ret %d.", __func__, cmd, ret);
            break;
        }
    }
    // At last test case.
    if (cmd == PWM_DISABLE_TEST) {
        PwmSetConfig(test->handle, &(test->originCfg));
    }
    PwmTestReleaseHandle(test->handle);
    OsalMSleep(SEQ_OUTPUT_DELAY);

    return ret;
}

static int32_t PwmTestBind(struct HdfDeviceObject *device)
{
    static struct PwmTest test;

    if (device != NULL) {
        device->service = &test.service;
    } else {
        HDF_LOGE("%s: device is NULL.", __func__);
    }
    HDF_LOGE("%s: success.", __func__);
    return HDF_SUCCESS;
}

static int32_t PwmTestInitFromHcs(struct PwmTest *test, const struct DeviceResourceNode *node)
{
    int32_t ret;
    uint32_t tmp;

    struct DeviceResourceIface *face = NULL;

    face = DeviceResourceGetIfaceInstance(HDF_CONFIG_SOURCE);
    if (face == NULL) {
        HDF_LOGE("%s: face is null.", __func__);
        return HDF_FAILURE;
    }
    if (face->GetUint32 == NULL) {
        HDF_LOGE("%s: GetUint32 not support.", __func__);
        return HDF_ERR_NOT_SUPPORT;
    }
    ret = face->GetUint32(node, "num", &(test->num), 0);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: read num fail.", __func__);
        return HDF_FAILURE;
    }
    ret = face->GetUint32(node, "period", &(test->cfg.period), 0);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: read period fail.", __func__);
        return HDF_FAILURE;
    }
    ret = face->GetUint32(node, "duty", &(test->cfg.duty), 0);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: read duty fail.", __func__);
        return HDF_FAILURE;
    }
    ret = face->GetUint32(node, "polarity", &tmp, 0);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: read polarity fail.", __func__);
        return HDF_FAILURE;
    }
    test->cfg.polarity = tmp;
    ret = face->GetUint32(node, "status", &tmp, 0);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: read status fail", __func__);
        return HDF_FAILURE;
    }
    test->cfg.status = tmp;
    return HDF_SUCCESS;
}

static int32_t PwmTestInit(struct HdfDeviceObject *device)
{
    struct PwmTest *test = NULL;

    if (device == NULL || device->service == NULL || device->property == NULL) {
        HDF_LOGE("%s: invalid parameter.", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    test = (struct PwmTest *)device->service;
    if (PwmTestInitFromHcs(test, device->property) != HDF_SUCCESS) {
        return HDF_FAILURE;
    }
    HDF_LOGE("%s: success.", __func__);
    test->TestEntry = PwmTestEntry;
    return HDF_SUCCESS;
}

static void PwmTestRelease(struct HdfDeviceObject *device)
{
    (void)device;
    HDF_LOGE("%s: success.", __func__);
}

struct HdfDriverEntry g_pwmTestEntry = {
    .moduleVersion = 1,
    .Bind = PwmTestBind,
    .Init = PwmTestInit,
    .Release = PwmTestRelease,
    .moduleName = "PLATFORM_PWM_TEST",
};
HDF_INIT(g_pwmTestEntry);
