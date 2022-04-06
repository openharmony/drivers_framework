/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "ili9881_st_5p5.h"
#include "gpio_if.h"
#include "hdf_bl.h"
#include "hdf_disp.h"
#include "osal.h"

struct panel_ili9881_dev *g_panel_dev = NULL;

static struct panel_ili9881_dev *ToPanelSimpleDev(const struct PanelData *panel)
{
    return (struct panel_ili9881_dev *)panel->object->priv;
}

static int32_t panel_simple_regulator_enable(void)
{
    int32_t err;
    if (g_panel_dev == NULL) {
        return -1;
    }
    err = regulator_enable(g_panel_dev->supply);
    if (err < 0) {
        HDF_LOGE("regulator_enable failed");
        return err;
    }
    return 0;
}
static int32_t panel_simple_regulator_disable(void)
{
    int32_t err;
    if (g_panel_dev == NULL) {
        return -1;
    }
    regulator_disable(g_panel_dev->supply);
    return 0;
}

int panel_simple_loader_protect(struct drm_panel *panel)
{
    int err;
    (void)panel;
    err = panel_simple_regulator_enable();
    if (err < 0) {
        HDF_LOGE("failed to enable supply: %d\n", err);
        return err;
    }
    return 0;
}
EXPORT_SYMBOL(panel_simple_loader_protect);

static int32_t PanelSendCmds(struct mipi_dsi_device *dsi,
    const struct DsiCmdDesc *cmds, int size)
{
    int32_t i = 0;

    HDF_LOGI("%s enter", __func__);
    if (dsi == NULL) {
        HDF_LOGE("dsi is NULL");
        return -EINVAL;
    }
    for (i = 0; i < size; i++) {
        mipi_dsi_dcs_write_buffer(dsi, cmds[i].payload, cmds[i].dataLen);
        if (cmds[i].delay) {
            OsalMSleep(cmds[i].delay);
        }
    }
    return HDF_SUCCESS;
}

static int32_t PanelOn(struct PanelData *panel)
{
    struct panel_ili9881_dev *panel_dev = NULL;

    panel_dev = ToPanelSimpleDev(panel);
    if (panel_dev->hw_delay.enable_delay) {
        OsalMSleep(panel_dev->hw_delay.enable_delay);
    }
    return HDF_SUCCESS;
}

static int32_t PanelOff(struct PanelData *panel)
{
    struct panel_ili9881_dev *panel_dev = NULL;

    panel_dev = ToPanelSimpleDev(panel);
    if (panel_dev->hw_delay.disable_delay) {
        OsalMSleep(panel_dev->hw_delay.disable_delay);
    }
    return HDF_SUCCESS;
}

static int32_t PanelPrepare(struct PanelData *panel)
{
    int32_t ret;
    struct panel_ili9881_dev *panel_dev = NULL;

    panel_dev = ToPanelSimpleDev(panel);
    ret = regulator_enable(panel_dev->supply);
    if (ret < 0) {
        HDF_LOGE("failed to enable supply: %d\n", ret);
        return ret;
    }
    gpiod_set_value_cansleep(panel_dev->enable_gpio, 1);

    if (panel_dev->hw_delay.reset_delay > 0) {
        OsalMSleep(panel_dev->hw_delay.reset_delay);
    }

    gpiod_set_value_cansleep(panel_dev->reset_gpio, 1);

    if (panel_dev->hw_delay.reset_delay > 0) {
        OsalMSleep(panel_dev->hw_delay.reset_delay);
    }

    gpiod_set_value_cansleep(panel_dev->reset_gpio, 0);

    if (panel_dev->hw_delay.prepare_delay > 0) {
        OsalMSleep(panel_dev->hw_delay.prepare_delay);
    }

    ret = PanelSendCmds(panel_dev->dsiDev, g_panelOnCode, \
        sizeof(g_panelOnCode) / sizeof(g_panelOnCode[0]));
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s PanelSendCmds failed", __func__);
        return HDF_FAILURE;
    }
    if (panel_dev->hw_delay.init_delay > 0) {
        OsalMSleep(panel_dev->hw_delay.init_delay);
    }

    return HDF_SUCCESS;
}

static int32_t PanelUnprepare(struct PanelData *panel)
{
    int32_t ret;
    struct panel_ili9881_dev *panel_dev = NULL;

    panel_dev = ToPanelSimpleDev(panel);

    ret = PanelSendCmds(panel_dev->dsiDev, g_panelOffCode, \
        sizeof(g_panelOffCode) / sizeof(g_panelOffCode[0]));
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s PanelSendCmds failed", __func__);
        return HDF_FAILURE;
    }
    gpiod_set_value_cansleep(panel_dev->reset_gpio, 1);
    gpiod_set_value_cansleep(panel_dev->enable_gpio, 1);
    regulator_disable(panel_dev->supply);

    if (panel_dev->hw_delay.unprepare_delay) {
        OsalMSleep(panel_dev->hw_delay.unprepare_delay);
    }

    return HDF_SUCCESS;
}

static int32_t PanelInit(struct PanelData *panel)
{
    return 0;
}

#define BLK_PWM_INDEX             2
#define PWM_MAX_PERIOD            40000
/* backlight setting */
#define MIN_LEVEL                 0
#define MAX_LEVEL                 255
#define DEFAULT_LEVEL             127

static struct PanelInfo g_panelInfo = {
    .width = 720,          /* width */
    .height = 1280,          /* height */
    .hbp = 40,             /* horizontal back porch */
    .hfp = 40,         /* horizontal front porch */
    .hsw = 10,              /* horizontal sync width */
    .vbp = 15,              /* vertical back porch */
    .vfp = 10,              /* vertical front porch */
    .vsw = 36,               /* vertical sync width */
    .clockFreq = 75000000,  /* clock */
    .pWidth = 68,           /* physical width */
    .pHeight = 121,         /* physical height */
    .connectorType = DRM_MODE_CONNECTOR_DPI,   /* DRM_MODE_CONNECTOR_DPI=17 */
    .blk = { BLK_PWM, MIN_LEVEL, MAX_LEVEL, DEFAULT_LEVEL },
};

static void PanelResInit(struct panel_ili9881_dev *panel_dev)
{
    panel_dev->dsiDev->lanes = 4;  /* 4: dsi,lanes ,number of active data lanes */
    panel_dev->dsiDev->format = MIPI_DSI_FMT_RGB888; // dsi,format pixel format for video mode MIPI_DSI_FMT_RGB888
    panel_dev->dsiDev->mode_flags = (MIPI_DSI_MODE_VIDEO | MIPI_DSI_MODE_VIDEO_BURST | MIPI_DSI_MODE_LPM \
                                          | MIPI_DSI_MODE_EOT_PACKET);
    panel_dev->panel.info = &g_panelInfo;
    panel_dev->panel.init = PanelInit;
    panel_dev->panel.on = PanelOn;
    panel_dev->panel.off = PanelOff;
    panel_dev->panel.prepare = PanelPrepare;
    panel_dev->panel.unprepare = PanelUnprepare;
    panel_dev->panel.priv = panel_dev->dsiDev;
    panel_dev->hw_delay.disable_delay = 50; /* 50:disable_delay time */
    panel_dev->hw_delay.enable_delay = 120; /* 120:enable_delay */
    panel_dev->hw_delay.init_delay = 20; /* 20:init_delay */
    panel_dev->hw_delay.prepare_delay = 2; /* 2:prepare_delay */
    panel_dev->hw_delay.reset_delay = 100;  /* 100:reset_delay */
    panel_dev->hw_delay.unprepare_delay = 20;  /* 20:unprepare_delay */
}

int32_t PanelEntryInit(struct HdfDeviceObject *object)
{
    struct device_node *panelNode = NULL;
    struct panel_ili9881_dev *panel_dev = NULL;

    panel_dev = (struct panel_ili9881_dev *)OsalMemCalloc(sizeof(struct panel_ili9881_dev));
    if (panel_dev == NULL) {
        HDF_LOGE("%s panel_dev malloc fail", __func__);
        return HDF_FAILURE;
    }
    g_panel_dev = panel_dev;
    panelNode = of_find_compatible_node(NULL, NULL, "simple-panel-dsi");
    if (panelNode == NULL) {
        HDF_LOGE("%s of_find_compatible_node fail", __func__);
        goto FAIL;
    }
    panel_dev->dsiDev = of_find_mipi_dsi_device_by_node(panelNode);
    if (panel_dev->dsiDev == NULL) {
        HDF_LOGE("%s of_find_mipi_dsi_device_by_node fail", __func__);
        goto FAIL;
    }
    panel_dev->supply = devm_regulator_get(&panel_dev->dsiDev->dev, "power");
    if (panel_dev->supply == NULL) {
        HDF_LOGE("Get regulator fail");
        goto FAIL;
    }

    panel_dev->enable_gpio = devm_gpiod_get_optional(&panel_dev->dsiDev->dev, "enable", GPIOD_ASIS);
    if (IS_ERR(panel_dev->enable_gpio)) {
        HDF_LOGE("get enable_gpio fail");
        goto FAIL;
    }
    panel_dev->hpd_gpio = devm_gpiod_get_optional(&panel_dev->dsiDev->dev, "hpd", GPIOD_IN);
    if (IS_ERR(panel_dev->hpd_gpio)) {
        HDF_LOGE("get hpd_gpio fail");
        goto FAIL;
    }
    panel_dev->reset_gpio = devm_gpiod_get_optional(&panel_dev->dsiDev->dev, "reset", GPIOD_ASIS);
    if (IS_ERR(panel_dev->reset_gpio)) {
        HDF_LOGE("get reset_gpio fail");
        goto FAIL;
    }

    PanelResInit(panel_dev);
    panel_dev->panel.object = object;
    object->priv = panel_dev;

    if (RegisterPanel(&panel_dev->panel) != HDF_SUCCESS) {
        HDF_LOGE("RegisterPanel fail");
        goto FAIL;
    }
    HDF_LOGI("%s success", __func__);
    return HDF_SUCCESS;
FAIL:
    OsalMemFree(panel_dev);
    return HDF_FAILURE;
}

struct HdfDriverEntry PanelDevEntry = {
    .moduleVersion = 1,
    .moduleName = "LCD_ILI9881_ST_5P5",
    .Init = PanelEntryInit,
};

HDF_INIT(PanelDevEntry);