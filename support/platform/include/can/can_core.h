/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef CAN_CORE_H
#define CAN_CORE_H

#include "can_if.h"
#include "can_manager.h"
#include "can_msg.h"
#include "can_service.h"
#include "hdf_base.h"
#include "hdf_dlist.h"
#include "hdf_sref.h"
#include "osal_mutex.h"
#include "platform_core.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct CanCntlr;
struct CanRxBox;

struct CanCntlrMethod {
    int32_t (*open)(struct CanCntlr *cntlr);
    int32_t (*close)(struct CanCntlr *cntlr);
    int32_t (*lock)(struct CanCntlr *cntlr);
    int32_t (*unlock)(struct CanCntlr *cntlr);
    int32_t (*sendMsg)(struct CanCntlr *cntlr, const struct CanMsg *msg);
    int32_t (*setCfg)(struct CanCntlr *cntlr, const struct CanConfig *cfg);
    int32_t (*getCfg)(struct CanCntlr *cntlr, struct CanConfig *cfg);
    int32_t (*getState)(struct CanCntlr *cntlr);
    int32_t (*addFilter)(struct CanCntlr *cntlr, struct CanFilter *filter);
    int32_t (*delFilter)(struct CanCntlr *cntlr, struct CanFilter *filter);
};

struct CanCntlr {
    struct PlatformDevice device; // common father
    struct OsalMutex lock;        // running lock
    int32_t number;               // bus number
    int32_t speed;                // bit rate
    int32_t state;                // bus status
    int32_t mode;                 // work mode
    struct CanCntlrMethod *ops;
    struct DListHead filters;
    struct DListHead rxBoxList;
    struct OsalMutex rboxListLock;
};

struct CanFilterNode {
    const struct CanFilter *filter;
    struct DListHead node;
    bool active;
};

enum CanErrState {
    CAN_ERR_ACTIVE,  // error active CAN_ERR_PASSIVE,                 // error passive
    CAN_ERR_BUS_OFF, // error bus off
};

struct CanTiming {
    uint32_t brp;  // prescaler
    uint32_t sjw;  // sync jump width
    uint32_t seg1; // phase segment 1
    uint32_t seg2; // phase segment 2
};

// HDF dev relationship manage
static inline int32_t CanCntlrSetHdfDev(struct CanCntlr *cntlr, struct HdfDeviceObject *device)
{
    return PlatformDeviceSetHdfDev(&cntlr->device, device);
}

static inline struct CanCntlr *CanCntlrFromHdfDev(struct HdfDeviceObject *device)
{
    struct PlatformDevice *pdevice = PlatformDeviceFromHdfDev(device);
    return (pdevice == NULL) ? NULL : CONTAINER_OF(pdevice, struct CanCntlr, device);
}

// CAN BUS operations
int32_t CanCntlrWriteMsg(struct CanCntlr *cntlr, const struct CanMsg *msg);

int32_t CanCntlrAddFilter(struct CanCntlr *cntlr, struct CanFilter *filter);

int32_t CanCntlrDelFilter(struct CanCntlr *cntlr, struct CanFilter *filter);

int32_t CanCntlrSetCfg(struct CanCntlr *cntlr, const struct CanConfig *cfg);

int32_t CanCntlrGetCfg(struct CanCntlr *cntlr, struct CanConfig *cfg);

int32_t CanCntlrGetState(struct CanCntlr *cntlr);

int32_t CanCntlrOnNewMsg(struct CanCntlr *cntlr, struct CanMsg *msg);

int32_t CanCntlrAddRxBox(struct CanCntlr *cntlr, struct CanRxBox *rxBox);

int32_t CanCntlrDelRxBox(struct CanCntlr *cntlr, struct CanRxBox *rxBox);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
