/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef CAN_MAIL_H
#define CAN_MAIL_H

#include "can_core.h"
#include "can_msg.h"
#include "platform_queue.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct CanRxBox {
    struct PlatformQueue *queue;
    struct DListHead node;
    struct DListHead filters;
    OsalSpinlock spin;
};

struct CanRxBox *CanRxBoxCreate(void);

void CanRxBoxDestroy(struct CanRxBox *rbox);

static inline void CanRxBoxLock(struct CanRxBox *rbox)
{
    (void)OsalSpinLock(&rbox->spin);
}

static inline void CanRxBoxUnlock(struct CanRxBox *rbox)
{
    (void)OsalSpinUnlock(&rbox->spin);
}

int32_t CanRxBoxAddMsg(struct CanRxBox *rbox, struct CanMsg *cmsg);

int32_t CanRxBoxGetMsg(struct CanRxBox *rbox, struct CanMsg **cmsg, uint32_t tms);

int32_t CanRxBoxAddFilter(struct CanRxBox *rbox, const struct CanFilter *filter);

int32_t CanRxBoxDelFilter(struct CanRxBox *rbox, const struct CanFilter *filter);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
