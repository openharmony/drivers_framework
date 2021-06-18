/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "hi35xx_disp.h"
#include <securec.h>
#include "hdf_base.h"
#include "hdf_device_desc.h"
#include "hdf_disp.h"
#include "hdf_log.h"
#include "lcd_abs_if.h"
#include "osal_io.h"
#include "osal_mem.h"
#include "pwm_if.h"

#define TRANSFORM_KILO 1000
#define TRANSFORM_MILL 1000000

struct Hi35xxDispCtrl {
    struct DispControl ctrl;
    unsigned long mipiCfgBase;
    unsigned long pwmCfgBase;
};

static void MipiMuxCfg(unsigned long ioCfgBase)
{
    /* config dsi data lane0 */
    OSAL_WRITEL(0x670, ioCfgBase + 0x0088);
    OSAL_WRITEL(0x670, ioCfgBase + 0x0084);
    /* config dsi data lane1 */
    OSAL_WRITEL(0x470, ioCfgBase + 0x007C);
    OSAL_WRITEL(0x470, ioCfgBase + 0x0080);
    /* config dsi data lane2 */
    OSAL_WRITEL(0x470, ioCfgBase + 0x006C);
    OSAL_WRITEL(0x470, ioCfgBase + 0x0070);
    /* config dsi data lane3 */
    OSAL_WRITEL(0x670, ioCfgBase + 0x0068);
    OSAL_WRITEL(0x670, ioCfgBase + 0x0064);
    /* config dsi clock lane */
    OSAL_WRITEL(0x460, ioCfgBase + 0x0074);
    OSAL_WRITEL(0x460, ioCfgBase + 0x0078);
}

static void Lcd6BitMuxCfg(unsigned long ioCfgBase)
{
    OSAL_WRITEL(0x4f4, ioCfgBase + 0x0068);
    OSAL_WRITEL(0x454, ioCfgBase + 0x0084);
    OSAL_WRITEL(0x474, ioCfgBase + 0x007c);
    OSAL_WRITEL(0x674, ioCfgBase + 0x0088);
    OSAL_WRITEL(0x474, ioCfgBase + 0x0080);
    OSAL_WRITEL(0x474, ioCfgBase + 0x0074);
    OSAL_WRITEL(0x474, ioCfgBase + 0x0078);
    OSAL_WRITEL(0x474, ioCfgBase + 0x006C);
    OSAL_WRITEL(0x474, ioCfgBase + 0x0070);
    OSAL_WRITEL(0x674, ioCfgBase + 0x0064);
}

static void Lcd8BitMuxCfg(unsigned long ioCfgBase)
{
    OSAL_WRITEL(0x422, ioCfgBase + 0x0034);
    OSAL_WRITEL(0x462, ioCfgBase + 0x0058);
    OSAL_WRITEL(0x462, ioCfgBase + 0x004c);
    OSAL_WRITEL(0x462, ioCfgBase + 0x0054);
    OSAL_WRITEL(0x422, ioCfgBase + 0x0048);
    OSAL_WRITEL(0x622, ioCfgBase + 0x0040);
    OSAL_WRITEL(0x622, ioCfgBase + 0x0044);
    OSAL_WRITEL(0x622, ioCfgBase + 0x005C);
    OSAL_WRITEL(0x622, ioCfgBase + 0x003c);
    OSAL_WRITEL(0x422, ioCfgBase + 0x0038);
    OSAL_WRITEL(0x622, ioCfgBase + 0x0050);
    OSAL_WRITEL(0x462, ioCfgBase + 0x0060);
}

void Lcd24BitMuxCfg(unsigned long ioCfgBase)
{
    OSAL_WRITEL(0x462, ioCfgBase + 0x0034);
    OSAL_WRITEL(0x432, ioCfgBase + 0x0058);
    OSAL_WRITEL(0x462, ioCfgBase + 0x004C);
    OSAL_WRITEL(0x432, ioCfgBase + 0x0054);
    OSAL_WRITEL(0x432, ioCfgBase + 0x0048);
    OSAL_WRITEL(0x632, ioCfgBase + 0x0040);
    OSAL_WRITEL(0x632, ioCfgBase + 0x0044);
    OSAL_WRITEL(0x632, ioCfgBase + 0x005C);
    OSAL_WRITEL(0x632, ioCfgBase + 0x003C);
    OSAL_WRITEL(0x432, ioCfgBase + 0x0038);
    OSAL_WRITEL(0x632, ioCfgBase + 0x0050);
    OSAL_WRITEL(0x462, ioCfgBase + 0x0060);
    OSAL_WRITEL(0x672, ioCfgBase + 0x0084);
    OSAL_WRITEL(0x672, ioCfgBase + 0x0088);
    OSAL_WRITEL(0x472, ioCfgBase + 0x007C);
    OSAL_WRITEL(0x472, ioCfgBase + 0x0080);
    OSAL_WRITEL(0x472, ioCfgBase + 0x0074);
    OSAL_WRITEL(0x472, ioCfgBase + 0x0078);
    OSAL_WRITEL(0x472, ioCfgBase + 0x006C);
    OSAL_WRITEL(0x462, ioCfgBase + 0x0070);
    OSAL_WRITEL(0x672, ioCfgBase + 0x0064);
    OSAL_WRITEL(0x672, ioCfgBase + 0x0068);
    OSAL_WRITEL(0x532, ioCfgBase + 0x0094);
    OSAL_WRITEL(0x532, ioCfgBase + 0x0090);
    OSAL_WRITEL(0x532, ioCfgBase + 0x008C);
    OSAL_WRITEL(0x632, ioCfgBase + 0x0098);
    OSAL_WRITEL(0x632, ioCfgBase + 0x009C);
    OSAL_WRITEL(0x532, ioCfgBase + 0x0030);
}

static void LcdPinMuxCfg(const struct Hi35xxDispCtrl *hi35xxCtrl, uint32_t intf)
{
    unsigned long ioCfgBase;

    ioCfgBase = hi35xxCtrl->mipiCfgBase;
    if (intf == MIPI_DSI) {
        MipiMuxCfg(ioCfgBase);
    } else if (intf == LCD_6BIT) {
        Lcd6BitMuxCfg(ioCfgBase);
    } else if (intf == LCD_8BIT) {
        Lcd8BitMuxCfg(ioCfgBase);
    } else if (intf == LCD_24BIT) {
        Lcd24BitMuxCfg(ioCfgBase);
    } else {
        HDF_LOGE("%s: not support intf: %d", __func__, intf);
    }
}

static void PwmPinMuxCfg(const struct Hi35xxDispCtrl *hi35xxCtrl, uint32_t dev)
{
    /* pwm pin config */
    unsigned long ioCfgBase;

    ioCfgBase = hi35xxCtrl->pwmCfgBase;
    switch (dev) {
        case PWM_DEV0:
            OSAL_WRITEL(0x601, ioCfgBase + 0x0024);
            break;
        case PWM_DEV1:
            OSAL_WRITEL(0x601, ioCfgBase + 0x0028);
            break;
        default:
            HDF_LOGE("%s: not support pwm dev: %d", __func__, dev);
            break;
    }
}

static int32_t GetBitsPerPixel(enum DsiOutFormat format)
{
    int32_t bpp;

    switch (format) {
        case FORMAT_RGB_16_BIT:
            bpp = 16; // 16 bits per pixel
            break;
        case FORMAT_RGB_18_BIT:
            bpp = 18; // 18 bits per pixel
            break;
        case FORMAT_RGB_24_BIT:
            bpp = 24; // 24 bits per pixel
            break;
        default:
            bpp = 24; // 24 bits per pixel
            break;
    }
    return bpp;
}

static uint32_t CalcPixelClk(struct PanelInfo *info)
{
    uint16_t hpixel;
    uint16_t vline;

    hpixel = info->width + info->hbp + info->hfp + info->hsw;
    vline = info->height + info->vbp + info->vfp + info->vsw;
    uint32_t pixNum = hpixel * vline * info->frameRate;
    if ((pixNum % TRANSFORM_KILO) == 0) {
        return pixNum / TRANSFORM_KILO;
    }
    return (pixNum / TRANSFORM_KILO + 1);
}

static uint32_t CalcDataRate(struct PanelInfo *info)
{
    uint16_t hpixel;
    uint16_t vline;
    uint32_t bitClk;

    hpixel = info->width + info->hbp + info->hfp + info->hsw;
    vline = info->height + info->vbp + info->vfp + info->vsw;
    int32_t bpp = GetBitsPerPixel(info->mipi.format);
    uint32_t bitNum = hpixel * vline * info->frameRate * bpp;
    if ((bitNum % TRANSFORM_MILL) == 0) {
        bitClk = bitNum / TRANSFORM_MILL;
    } else {
        bitClk = bitNum / TRANSFORM_MILL + 1;
    }
    if (!info->mipi.lane) {
        return HDF_FAILURE;
    }
    if ((bitClk % info->mipi.lane) == 0) {
        return bitClk / info->mipi.lane;
    }
    return (bitClk / info->mipi.lane) + 1;
}

static int32_t MipiDsiInit(struct PanelInfo *info)
{
    int32_t ret;
    struct DevHandle *mipiHandle = NULL;
    struct MipiCfg cfg;

    mipiHandle = MipiDsiOpen(0);
    if (mipiHandle == NULL) {
        HDF_LOGE("%s: MipiDsiOpen failed", __func__);
        return HDF_FAILURE;
    }
    cfg.lane = info->mipi.lane;
    cfg.mode = info->mipi.mode;
    cfg.format = info->mipi.format;
    cfg.burstMode = info->mipi.burstMode;
    cfg.timing.xPixels = info->width;
    cfg.timing.hsaPixels = info->hsw;
    cfg.timing.hbpPixels = info->hbp;
    cfg.timing.hlinePixels = info->width + info->hbp + info->hfp + info->hsw;
    cfg.timing.vsaLines = info->vsw;
    cfg.timing.vbpLines = info->vbp;
    cfg.timing.vfpLines = info->vfp;
    cfg.timing.ylines = info->height;
    /* 0 : no care */
    cfg.timing.edpiCmdSize = 0;
    cfg.pixelClk = CalcPixelClk(info);
    cfg.phyDataRate = CalcDataRate(info);
    /* config mipi device */
    ret = MipiDsiSetCfg(mipiHandle, &cfg);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s:MipiDsiSetCfg failed", __func__);
    }
    MipiDsiClose(mipiHandle);
    HDF_LOGI("%s:pixelClk = %d, phyDataRate = %u", __func__, cfg.pixelClk, cfg.phyDataRate);
    return ret;
}

static int32_t PwmInit(struct Hi35xxDispCtrl *hi35xxCtrl, struct PanelInfo *info)
{
    int32_t ret;

    /* pwm pin config */
    PwmPinMuxCfg(hi35xxCtrl, info->pwm.dev);
    /* pwm config */
    struct DevHandle *pwmHandle = PwmOpen(info->pwm.dev);
    if (pwmHandle == NULL) {
        HDF_LOGE("%s: PwmOpen failed", __func__);
        return HDF_FAILURE;
    }
    struct PwmConfig config;
    (void)memset_s(&config, sizeof(struct PwmConfig), 0, sizeof(struct PwmConfig));
    config.duty = 1;
    config.period = info->pwm.period;
    config.status = 0;
    ret = PwmSetConfig(pwmHandle, &config);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: PwmSetConfig err, ret %d", __func__, ret);
        PwmClose(pwmHandle);
        return HDF_FAILURE;
    }
    PwmClose(pwmHandle);
    return HDF_SUCCESS;
}

static int32_t Hi35xxHardwareInit(struct Hi35xxDispCtrl *hi35xxCtrl)
{
    int32_t i;
    int32_t panelNum;
    int32_t ret = HDF_FAILURE;
    struct PanelData **panel = NULL;
    struct PanelInfo *info = NULL;

    if (hi35xxCtrl->ctrl.panelManager == NULL) {
        HDF_LOGE("%s: panelManager is null", __func__);
        return HDF_FAILURE;
    }
    if (hi35xxCtrl->ctrl.panelManager->panelNum <= 0) {
        HDF_LOGE("%s: none of panels registered", __func__);
        return HDF_FAILURE;
    }
    panelNum = hi35xxCtrl->ctrl.panelManager->panelNum;
    panel = hi35xxCtrl->ctrl.panelManager->panel;
    for (i = 0; i < panelNum; i++) {
        info = panel[i]->info;
        if (info == NULL) {
            HDF_LOGE("%s:get info failed", __func__);
            return HDF_FAILURE;
        }
        if (info->blk.type == BLK_PWM) {
            ret = PwmInit(hi35xxCtrl, info);
            if (ret) {
                HDF_LOGE("%s:PwmInit failed", __func__);
                return HDF_FAILURE;
            }
        }
        /* lcd pin mux config */
        LcdPinMuxCfg(hi35xxCtrl, info->intfType);
        if (info->intfType == MIPI_DSI) {
            /* mipi dsi init */
            ret = MipiDsiInit(info);
            if (ret) {
                HDF_LOGE("%s:MipiDsiInit failed", __func__);
                return HDF_FAILURE;
            }
        }
        if (panel[i]->init != NULL) {
            /* panel driver init */
            ret = panel[i]->init(panel[i]);
            if (ret != HDF_SUCCESS) {
                HDF_LOGE("%s: panelData->init failed", __func__);
                return HDF_FAILURE;
            }
        }
    }
    return ret;
}

static int32_t GetLcdIntfType(enum LcdIntfType type, uint32_t *out)
{
    int32_t ret = HDF_SUCCESS;

    switch (type) {
        case MIPI_DSI:
            *out = INTF_MIPI;
            break;
        case LCD_6BIT:
            *out = INTF_LCD_6BIT;
            break;
        case LCD_8BIT:
            *out = INTF_LCD_8BIT;
            break;
        case LCD_16BIT:
            *out = INTF_LCD_16BIT;
            break;
        case LCD_18BIT:
            *out = INTF_LCD_18BIT;
            break;
        case LCD_24BIT:
            *out = INTF_LCD_24BIT;
            break;
        default:
            HDF_LOGE("%s: not support intf: %d", __func__, type);
            ret = HDF_FAILURE;
            break;
    }
    return ret;
}

static int32_t Hi35xxGetDispInfo(struct DispControl *dispCtrl, uint32_t devId)
{
    struct PanelInfo *panelInfo = NULL;
    struct DispInfo *info = NULL;
    struct PanelData *panel = NULL;

    if (dispCtrl == NULL) {
        HDF_LOGE("%s:dispCtrl is null", __func__);
        return HDF_FAILURE;
    }
    if (dispCtrl->panelManager == NULL || (devId >= dispCtrl->panelManager->panelNum)) {
        HDF_LOGE("%s: get panel fail", __func__);
        return HDF_FAILURE;
    }
    panel = dispCtrl->panelManager->panel[devId];
    if (panel == NULL) {
        HDF_LOGE("%s:panel is null", __func__);
        return HDF_FAILURE;
    }
    panelInfo = panel->info;
    if (panelInfo == NULL) {
        HDF_LOGE("%s:get info failed", __func__);
        return HDF_FAILURE;
    }
    info = dispCtrl->info;
    if (info == NULL) {
        HDF_LOGE("%s:info is null", __func__);
        return HDF_FAILURE;
    }
    info->width = panelInfo->width;
    info->height = panelInfo->height;
    info->hbp = panelInfo->hbp;
    info->hfp = panelInfo->hfp;
    info->hsw = panelInfo->hsw;
    info->vbp = panelInfo->vbp;
    info->vfp = panelInfo->vfp;
    info->vsw = panelInfo->vsw;
    if (GetLcdIntfType(panelInfo->intfType, &info->intfType) != HDF_SUCCESS) {
        HDF_LOGE("%s:GetLcdIntfType failed", __func__);
        return HDF_FAILURE;
    }
    info->intfSync = panelInfo->intfSync;
    info->frameRate = panelInfo->frameRate;
    info->minLevel = panelInfo->blk.minLevel;
    info->maxLevel = panelInfo->blk.maxLevel;
    info->defLevel = panelInfo->blk.defLevel;
    HDF_LOGI("info->width = %d, info->height = %d", info->width, info->height);
    HDF_LOGI("info->hbp = %d, info->hfp = %d", info->hbp, info->hfp);
    HDF_LOGI("info->frameRate = %d, info->intfSync = %d", info->frameRate, info->intfSync);
    return HDF_SUCCESS;
}

static int32_t Hi35xxOn(struct DispControl *dispCtrl, uint32_t devId)
{
    int32_t ret = HDF_FAILURE;
    struct PanelData *panel = NULL;

    if (dispCtrl == NULL) {
        HDF_LOGE("%s: dispCtrl is null", __func__);
        return HDF_FAILURE;
    }
    if (dispCtrl->panelManager == NULL || (devId >= dispCtrl->panelManager->panelNum)) {
        HDF_LOGE("%s: get panel fail", __func__);
        return HDF_FAILURE;
    }
    panel = dispCtrl->panelManager->panel[devId];
    if (panel == NULL) {
        HDF_LOGE("%s: panel is null", __func__);
        return HDF_FAILURE;
    }
    if (panel->on != NULL) {
        /* panel driver on */
        ret = panel->on(panel);
        if (ret != HDF_SUCCESS) {
            HDF_LOGE("%s: panel->on failed", __func__);
            return HDF_FAILURE;
        }
    }
    return ret;
}

static int32_t Hi35xxOff(struct DispControl *dispCtrl, uint32_t devId)
{
    int32_t ret = HDF_FAILURE;
    struct PanelData *panel = NULL;

    if (dispCtrl == NULL) {
        HDF_LOGE("%s: dispCtrl is null", __func__);
        return HDF_FAILURE;
    }
    if (dispCtrl->panelManager == NULL || (devId >= dispCtrl->panelManager->panelNum)) {
        HDF_LOGE("%s: get panel fail", __func__);
        return HDF_FAILURE;
    }
    panel = dispCtrl->panelManager->panel[devId];
    if (panel == NULL) {
        HDF_LOGE("%s: panel is null", __func__);
        return HDF_FAILURE;
    }
    if (panel->off != NULL) {
        /* panel driver off */
        ret = panel->off(panel);
        if (ret != HDF_SUCCESS) {
            HDF_LOGE("%s: panel->off failed", __func__);
            return HDF_FAILURE;
        }
    }
    return ret;
}

static int32_t Hi35xxSetBacklight(struct DispControl *dispCtrl, uint32_t devId, uint32_t level)
{
    int32_t ret = HDF_FAILURE;
    struct PanelData *panel = NULL;

    if (dispCtrl == NULL) {
        HDF_LOGE("%s: dispCtrl is null", __func__);
        return HDF_FAILURE;
    }
    if (dispCtrl->panelManager == NULL || (devId >= dispCtrl->panelManager->panelNum)) {
        HDF_LOGE("%s: get panel fail", __func__);
        return HDF_FAILURE;
    }
    panel = dispCtrl->panelManager->panel[devId];
    if (panel == NULL) {
        HDF_LOGE("%s: panel is null", __func__);
        return HDF_FAILURE;
    }
    if (panel->setBacklight != NULL) {
        /* panel driver set backlight */
        ret = panel->setBacklight(panel, level);
        if (ret != HDF_SUCCESS) {
            HDF_LOGE("%s: setBacklight failed", __func__);
            return HDF_FAILURE;
        }
    }
    return ret;
}

static int32_t Hi35xxResInit(struct Hi35xxDispCtrl *hi35xxCtrl, struct PanelManager *panelManager)
{
    hi35xxCtrl->ctrl.ops.on = Hi35xxOn;
    hi35xxCtrl->ctrl.ops.off = Hi35xxOff;
    hi35xxCtrl->ctrl.ops.setBacklight = Hi35xxSetBacklight;
    hi35xxCtrl->ctrl.ops.getDispInfo = Hi35xxGetDispInfo;
    hi35xxCtrl->ctrl.panelManager = panelManager;
    hi35xxCtrl->mipiCfgBase = (unsigned long)OsalIoRemap(IO_CFG2_BASE, IO_CFG_SIZE);
    hi35xxCtrl->pwmCfgBase = (unsigned long)OsalIoRemap(IO_CFG1_BASE, IO_CFG_SIZE);
    return HDF_SUCCESS;
}

static int32_t Hi35xxEntryInit(struct HdfDeviceObject *object)
{
    struct PanelManager *panelManager = NULL;
    struct Hi35xxDispCtrl *hi35xxCtrl = NULL;

    if (object == NULL) {
        HDF_LOGE("%s: object is null!", __func__);
        return HDF_FAILURE;
    }
    hi35xxCtrl = (struct Hi35xxDispCtrl *)OsalMemCalloc(sizeof(struct Hi35xxDispCtrl));
    if (hi35xxCtrl == NULL) {
        HDF_LOGE("%s hi35xxCtrl malloc fail", __func__);
        return HDF_FAILURE;
    }
    panelManager = GetPanelManager();
    if (panelManager == NULL) {
        HDF_LOGE("%s: panelManager is null", __func__);
        return HDF_FAILURE;
    }
    if (Hi35xxResInit(hi35xxCtrl, panelManager) == HDF_FAILURE) {
        HDF_LOGE("%s Hi35xxResInit fail", __func__);
        return HDF_FAILURE;
    }
    if (Hi35xxHardwareInit(hi35xxCtrl) == HDF_FAILURE) {
        HDF_LOGE("%s Hi35xxHardwareInit fail", __func__);
        return HDF_FAILURE;
    }
    hi35xxCtrl->ctrl.object = object;
    hi35xxCtrl->ctrl.object->priv = hi35xxCtrl;
    return RegisterDispCtrl(&hi35xxCtrl->ctrl);
}

struct HdfDriverEntry g_hi35xxDevEntry = {
    .moduleVersion = 1,
    .moduleName = "HI351XX_DISP",
    .Init = Hi35xxEntryInit,
};

HDF_INIT(g_hi35xxDevEntry);
