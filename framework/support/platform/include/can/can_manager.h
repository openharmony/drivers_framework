/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef CAN_MANAGER_H
#define CAN_MANAGER_H

#include "hdf_base.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct CanCntlr;

// CAN BUS controller manage
int32_t CanCntlrAdd(struct CanCntlr *cntlr);

int32_t CanCntlrDel(struct CanCntlr *cntlr);

void CanCntlrPut(struct CanCntlr *cntlr);

struct CanCntlr *CanCntlrGetByName(const char *name);

struct CanCntlr *CanCntlrGetByNumber(int32_t number);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
