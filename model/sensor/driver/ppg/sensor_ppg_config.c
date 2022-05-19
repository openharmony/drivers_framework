/*
 * Copyright (c) 2022 Chipsea Technologies (Shenzhen) Corp., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include <securec.h>
#include "osal_mem.h"
#include "osal_io.h"
#include "sensor_ppg_config.h"

static struct PpgCfgData *g_ppgCfg = NULL;

#define HDF_LOG_TAG hdf_sensor_ppg_config

static int32_t InitPpgCfgData()
{
    g_ppgCfg = (struct PpgCfgData *)OsalMemCalloc(sizeof(*g_ppgCfg));
    if (g_ppgCfg == NULL) {
        HDF_LOGE("%s: malloc failed!", __func__);
        return HDF_FAILURE;
    }
    (void)memset_s(g_ppgCfg, sizeof(*g_ppgCfg), 0, sizeof(*g_ppgCfg));
    return HDF_SUCCESS;
}

struct PpgCfgData *GetPpgConfig(void)
{
    return g_ppgCfg;
}

static int32_t ParsePpgPinMuxConfig(struct DeviceResourceIface *parser,
    const struct DeviceResourceNode *node, struct PpgPinCfg *pinCfg)
{
    int32_t ret;
    int32_t num;
    struct MultiUsePinSetCfg *mutiPinConfig = NULL;

    num = parser->GetElemNum(node, "multiUseSet");
    if ((num <= 0) || (num > PPG_PIN_CONFIG_MAX_ITEM) || ((num % PPG_PIN_CFG_INDEX_MAX) != 0)) {
        HDF_LOGW("%s: parser multiUseSet size overflow, ignore this config, num = %d", __func__, num);
        return HDF_SUCCESS;
    }

    if (pinCfg->multiUsePin == NULL) {
        pinCfg->multiUsePin = (struct MultiUsePinSetCfg *)OsalMemCalloc(sizeof(uint32_t) * num);
        if (pinCfg->multiUsePin == NULL) {
            HDF_LOGE("%s: malloc ppg multiUsePin config item failed", __func__);
            return HDF_ERR_MALLOC_FAIL;
        }
    }

    ret = parser->GetUint32Array(node, "multiUseSet", (uint32_t *)pinCfg->multiUsePin, num, 0);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: parser multiUseSet reg array failed", __func__);
        return HDF_FAILURE;
    }

    for (uint32_t index = 0; index < (num / PPG_PIN_CFG_INDEX_MAX); ++index) {
        mutiPinConfig = &pinCfg->multiUsePin[index];
        ret = SetSensorPinMux(mutiPinConfig->regAddr, mutiPinConfig->regLen, mutiPinConfig->regValue);
        if (ret != HDF_SUCCESS) {
            HDF_LOGE("%s: SetSensorPinMux failed, index %d, reg=0x%x, len=%d, val=0x%x", __func__, index,
                     mutiPinConfig->regAddr, mutiPinConfig->regLen, mutiPinConfig->regValue);
            return HDF_FAILURE;
        }
    }

    return HDF_SUCCESS;
}

static int32_t ParsePpgGpiosConfig(struct DeviceResourceIface *parser,
                                   const struct DeviceResourceNode *node, struct PpgPinCfg *pinCfg)
{
    int32_t ret;
    int32_t num;
    struct GpiosPinSetCfg *gpioCfg = NULL;

    num = parser->GetElemNum(node, "gipos");
    if ((num <= 0) || (num > PPG_PIN_CONFIG_MAX_ITEM) || ((num % PPG_GPIOS_INDEX_MAX) != 0)) {
        HDF_LOGE("%s: parser gipos size overflow, ignore this config, num = %d", __func__, num);
        return HDF_SUCCESS;
    }

    if (pinCfg->gpios == NULL) {
        pinCfg->gpios = (struct GpiosPinSetCfg *)OsalMemCalloc(sizeof(uint16_t) * num);
        if (pinCfg->gpios == NULL) {
            HDF_LOGE("%s: malloc ppg gipos config item failed", __func__);
            return HDF_ERR_MALLOC_FAIL;
        }
    }

    ret = parser->GetUint16Array(node, "gipos", (uint16_t *)pinCfg->gpios, num, 0);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: parser gipos reg array failed", __func__);
        return HDF_FAILURE;
    }

    pinCfg->gpioNum = 0;

    for (uint32_t index = 0; index < (num / PPG_GPIOS_INDEX_MAX); ++index) {
        gpioCfg = &pinCfg->gpios[index];
        ret = GpioSetDir(gpioCfg->gpioNo, gpioCfg->dirType);
        if (ret != HDF_SUCCESS) {
            HDF_LOGE("%s: GpioSetDir is error, ret = %d!", __func__, ret);
            return HDF_FAILURE;
        }

        if (pinCfg->gpios[index].gpioType == PPG_TYPE_GPIO) {
            ret = GpioWrite(gpioCfg->gpioNo, gpioCfg->gpioValue);
            if (ret != HDF_SUCCESS) {
                HDF_LOGE("%s: GpioWrite is error, ret = %d!", __func__, ret);
                return HDF_FAILURE;
            }
        }
        pinCfg->gpioNum++;
    }

    return HDF_SUCCESS;
}

int32_t ParsePpgPinConfig(struct DeviceResourceIface *parser,
                          const struct DeviceResourceNode *pinCfgNode, struct PpgPinCfg *config)
{
    int32_t ret;

    ret = ParsePpgPinMuxConfig(parser, pinCfgNode, config);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: pin mux config failed!", __func__);
        return HDF_FAILURE;
    }

    ret = ParsePpgGpiosConfig(parser, pinCfgNode, config);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: gpios config failed!", __func__);
        return HDF_FAILURE;
    }

    HDF_LOGI("%s: Parse and Set pin success!", __func__);
    return HDF_SUCCESS;
}

int32_t EnablePpgIrq(uint16_t pinIndex)
{
    struct PpgCfgData *config = GetPpgConfig();
    struct GpiosPinSetCfg *gpio = NULL;
    CHECK_NULL_PTR_RETURN_VALUE(config, HDF_ERR_INVALID_PARAM);
    CHECK_NULL_PTR_RETURN_VALUE(config->pinCfg.gpios, HDF_ERR_INVALID_PARAM);

    for (uint16_t gpioIndex = 0; gpioIndex < config->pinCfg.gpioNum; gpioIndex++) {
        gpio = &config->pinCfg.gpios[gpioIndex];
        CHECK_NULL_PTR_RETURN_VALUE(gpio, HDF_ERR_INVALID_PARAM);
        if ((gpio->gpioType == PPG_TYPE_IRQ) && (pinIndex == gpio->pinIndex)) {
            if (GpioEnableIrq(gpio->gpioNo) != 0) {
                break;
            }
            return HDF_SUCCESS;
        }
    }
    HDF_LOGE("%s: failed!", __func__);
    return HDF_FAILURE;
}

int32_t DisablePpgIrq(uint16_t pinIndex)
{
    struct PpgCfgData *config = GetPpgConfig();
    struct GpiosPinSetCfg *gpio = NULL;
    CHECK_NULL_PTR_RETURN_VALUE(config, HDF_ERR_INVALID_PARAM);
    CHECK_NULL_PTR_RETURN_VALUE(config->pinCfg.gpios, HDF_ERR_INVALID_PARAM);

    for (uint16_t gpioIndex = 0; gpioIndex < config->pinCfg.gpioNum; gpioIndex++) {
        gpio = &config->pinCfg.gpios[gpioIndex];
        CHECK_NULL_PTR_RETURN_VALUE(gpio, HDF_ERR_INVALID_PARAM);
        if ((gpio->gpioType == PPG_TYPE_IRQ) && (pinIndex == gpio->pinIndex)) {
            if (GpioDisableIrq(gpio->gpioNo) != 0) {
                break;
            }
            return HDF_SUCCESS;
        }
    }
    HDF_LOGE("%s: failed!", __func__);
    return HDF_FAILURE;
}

int32_t SetPpgIrq(uint16_t pinIndex, GpioIrqFunc irqFunc)
{
    struct PpgCfgData *config = GetPpgConfig();
    struct GpiosPinSetCfg *gpio = NULL;
    CHECK_NULL_PTR_RETURN_VALUE(config, HDF_ERR_INVALID_PARAM);
    CHECK_NULL_PTR_RETURN_VALUE(irqFunc, HDF_ERR_INVALID_PARAM);
    CHECK_NULL_PTR_RETURN_VALUE(config->pinCfg.gpios, HDF_ERR_INVALID_PARAM);

    for (uint16_t gpioIndex = 0; gpioIndex < config->pinCfg.gpioNum; gpioIndex++) {
        gpio = &config->pinCfg.gpios[gpioIndex];
        CHECK_NULL_PTR_RETURN_VALUE(gpio, HDF_ERR_INVALID_PARAM);
        if ((gpio->pinIndex == pinIndex) && (gpio->gpioType == PPG_TYPE_IRQ)) {
            if (GpioSetIrq(gpio->gpioNo, gpio->triggerMode, irqFunc, NULL) != 0) {
                break;
            }
            return HDF_SUCCESS;
        }
    }

    HDF_LOGE("%s: failed!", __func__);
    return HDF_FAILURE;
}

static int32_t GetPinConfigData(const struct DeviceResourceNode *node, struct PpgPinCfg *pinCfg)
{
    int32_t ret = HDF_FAILURE;
    struct DeviceResourceIface *parser = DeviceResourceGetIfaceInstance(HDF_CONFIG_SOURCE);
    CHECK_NULL_PTR_RETURN_VALUE(parser, HDF_ERR_INVALID_PARAM);

    const struct DeviceResourceNode *pinCfgNode = parser->GetChildNode(node, "ppgPinConfig");
    if (pinCfgNode != NULL) {
        ret = ParsePpgPinConfig(parser, pinCfgNode, pinCfg);
    }

    HDF_LOGI("%s: Parse and Set pin ret = %d!", __func__, ret);
    return ret;
}

void ReleasePpgCfgData(void)
{
    CHECK_NULL_PTR_RETURN(g_ppgCfg);

    if (g_ppgCfg->pinCfg.multiUsePin != NULL) {
        OsalMemFree(g_ppgCfg->pinCfg.multiUsePin);
    }

    if (g_ppgCfg->pinCfg.gpios != NULL) {
        OsalMemFree(g_ppgCfg->pinCfg.gpios);
    }

    OsalMemFree(g_ppgCfg);
    g_ppgCfg = NULL;
}

int32_t ParsePpgCfgData(const struct DeviceResourceNode *node, struct PpgCfgData **cfgData)
{
    int32_t ret;

    if (g_ppgCfg != NULL) {
        HDF_LOGI("%s: ppg config already parsed.", __func__);
        return HDF_SUCCESS;
    }

    ret = InitPpgCfgData();
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: Init ppg config failed.", __func__);
        return HDF_FAILURE;
    }

    ret = GetSensorBaseConfigData(node, &g_ppgCfg->sensorCfg);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: get sensor base config failed.", __func__);
        ReleasePpgCfgData();
        return HDF_FAILURE;
    }

    ret = GetPinConfigData(node, &g_ppgCfg->pinCfg);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: get pin config failed.", __func__);
        ReleasePpgCfgData();
        return HDF_FAILURE;
    }

    *cfgData = g_ppgCfg;
    return HDF_SUCCESS;
}
