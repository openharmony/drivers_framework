/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include "gpio_pl061_sample.h"
#include "osal_irq.h"

#define HDF_LOG_TAG gpio_pl061_sample

int32_t Pl061GetGroupByGpioNum(struct GpioCntlr *cntlr, uint16_t gpio, struct GpioGroup **group)
{
    struct Pl061GpioCntlr *pl061 = NULL;
    uint16_t groupIndex = Pl061ToGroupNum(gpio);

    if (cntlr == NULL) {
        HDF_LOGE("%s: cntlr is NULL", __func__);
        return HDF_ERR_INVALID_OBJECT;
    }
    pl061 = ToPl061GpioCntlr(cntlr);
    if (groupIndex >= pl061->groupNum) {
        HDF_LOGE("%s: err group index:%u", __func__, groupIndex);
        return HDF_ERR_INVALID_PARAM;
    }
    *group = &pl061->groups[groupIndex];
    return HDF_SUCCESS;
}