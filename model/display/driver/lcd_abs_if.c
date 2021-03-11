/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include <asm/io.h>
#include "hdf_device_desc.h"
#include "osal.h"
#include "lcd_abs_if.h"

/* support max panel number */
#define PANEL_MAX 2
static struct PanelData *g_panelData[PANEL_MAX];
static int32_t numRegisteredPanel;

int32_t PanelDataRegister(struct PanelData *data)
{
    int32_t i;

    if (data == NULL) {
        HDF_LOGE("%s: panel data is null", __func__);
        return HDF_ERR_INVALID_PARAM;
    }
    if (numRegisteredPanel == PANEL_MAX) {
        return HDF_FAILURE;
    }
    numRegisteredPanel++;
    for (i = 0; i < PANEL_MAX; i++) {
        if (g_panelData[i] == NULL) {
            break;
        }
    }
    if (i >= PANEL_MAX) {
        return HDF_FAILURE;
    }
    g_panelData[i] = data;
    HDF_LOGI("%s: panel data register success", __func__);
    return HDF_SUCCESS;
}

struct PanelData *GetPanelData(int32_t index)
{
    if ((index >= PANEL_MAX) || index < 0) {
        return NULL;
    }
    return g_panelData[index];
}

struct PanelInfo *GetPanelInfo(int32_t index)
{
    if ((index >= PANEL_MAX) || index < 0) {
        return NULL;
    }
    if (g_panelData[index] == NULL) {
        return NULL;
    }
    return g_panelData[index]->info;
}

