/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "lcd_abs_if.h"
#include <asm/io.h>
#include "hdf_device_desc.h"
#include "osal.h"

static struct PanelManager g_panelManager;
int32_t RegisterPanel(struct PanelData *data)
{
    int32_t panelNum;

    if (data == NULL) {
        HDF_LOGE("%s: panel data is null", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    panelNum = g_panelManager.panelNum;
    if (panelNum >= PANEL_MAX) {
        HDF_LOGE("%s registered panel up PANEL_MAX", __func__);
        return HDF_FAILURE;
    }
    g_panelManager.panel[panelNum] = data;
    g_panelManager.panelNum++;
    HDF_LOGI("%s: register success", __func__);
    return HDF_SUCCESS;
}

struct PanelManager *GetPanelManager(void)
{
    if (g_panelManager.panelNum == 0) {
        return NULL;
    } else {
        return &g_panelManager;
    }
}

struct PanelData *GetPanel(int32_t index)
{
    struct PanelManager *panelManager = NULL;

    panelManager = GetPanelManager();
    if (panelManager == NULL) {
        HDF_LOGE("%s panelManager is null", __func__);
        return NULL;
    }
    if (index >= g_panelManager.panelNum) {
        HDF_LOGE("%s index is greater than g_panelManager.panelNum", __func__);
        return NULL;
    }
    return panelManager->panel[index];
}
