/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef HDF_DISP_H
#define HDF_DISP_H
#include "hdf_base.h"
#include "hdf_device_desc.h"
#include "hdf_sbuf.h"
#include "hdf_workqueue.h"
#include "lcd_abs_if.h"
#include "osal_mem.h"
#include "osal_mutex.h"
#include "osal_timer.h"

#ifdef HDF_LOG_TAG
#undef HDF_LOG_TAG
#endif
#define HDF_LOG_TAG HDF_DISP
#define ESD_DEFAULT_INTERVAL   5000
#define ESD_MAX_RECOVERY       10

typedef int32_t (*DispCmdHandle)(struct HdfDeviceObject *device, struct HdfSBuf *reqData, struct HdfSBuf *rspData);

struct DispInfo {
    uint32_t width;
    uint32_t hbp;
    uint32_t hfp;
    uint32_t hsw;
    uint32_t height;
    uint32_t vbp;
    uint32_t vfp;
    uint32_t vsw;
    uint32_t frameRate;
    uint32_t intfType;
    enum IntfSync intfSync;
    uint32_t minLevel;
    uint32_t maxLevel;
    uint32_t defLevel;
};

struct DispOperations {
    int32_t (*init)(uint32_t devId);
    int32_t (*on)(uint32_t devId);
    int32_t (*off)(uint32_t devId);
    int32_t (*setBacklight)(uint32_t devId, uint32_t level);
    int32_t (*getDispInfo)(uint32_t devId, struct DispInfo *info);
};

enum EsdState {
    ESD_READY = 1,
    ESD_RUNNING,
};

struct DispEsd {
    struct PanelEsd **panelEsd;
    HdfWork **work;
    bool *workInit;
    OsalTimer **timer;
    int32_t panelNum;
};

struct DispControl;
struct DispControlOps {
    int32_t (*on)(struct DispControl *dispCtrl, uint32_t devId);
    int32_t (*off)(struct DispControl *dispCtrl, uint32_t devId);
    int32_t (*setBacklight)(struct DispControl *dispCtrl, uint32_t devId, uint32_t level);
    int32_t (*getDispInfo)(struct DispControl *dispCtrl, uint32_t devId);
};

struct DispControl {
    struct HdfDeviceObject *object;
    struct PanelManager *panelManager;
    struct DispInfo *info;
    struct DispControlOps ops;
};

struct DispManager {
    struct DispControl *dispCtrl;
    struct PanelManager *panelManager;
    struct OsalMutex dispMutex;
    HdfWorkQueue dispWorkQueue;
    bool initialzed;
    struct DispEsd *esd;
};

int32_t RegisterDispCtrl(struct DispControl *dispCtrl);
#endif /* HDF_DISP_H */
