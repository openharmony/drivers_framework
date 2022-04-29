/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef CAN_CLIENT_H
#define CAN_CLIENT_H

#include "can_core.h"
#include "can_mail.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct CanClient;

int32_t CanClientCreateByNumber(int32_t busNum, struct CanClient **client);

void CanClientDestroy(struct CanClient *client);

int32_t CanClientWriteMsg(struct CanClient *client, const struct CanMsg *msg);

int32_t CanClientReadMsg(struct CanClient *client, struct CanMsg *msg, uint32_t tms);

int32_t CanClientAddFilter(struct CanClient *client, const struct CanFilter *filter);

int32_t CanClientDelFilter(struct CanClient *client, const struct CanFilter *filter);

int32_t CanClientSetCfg(struct CanClient *client, const struct CanConfig *cfg);

int32_t CanClientGetCfg(struct CanClient *client, struct CanConfig *cfg);

int32_t CanClientGetState(struct CanClient *client);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
