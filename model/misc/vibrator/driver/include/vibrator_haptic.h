/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef VIBRATOR_HAPTIC_H
#define VIBRATOR_HAPTIC_H

#include "hdf_device_desc.h"
#include "hdf_dlist.h"
#include "osal_mutex.h"
#include "osal_sem.h"
#include "osal_thread.h"
#include "vibrator_driver_type.h"

enum VibratorEffectType {
    VIBRATOR_TYPE_EFFECT    = 0, // Preset effect in device
    VIBRATOR_TYPE_TIME      = 1, // Preset effect by time
};

enum VibratorTimeSeqIndex {
    VIBRATOR_TIME_DELAY_INDEX    = 0,
    VIBRATOR_TIME_DURATION_INDEX = 1,
    VIBRATOR_TIME_INDEX_BUTT,
};

struct VibratorEffectNode {
    const char *effect;
    int32_t num;
    uint32_t *seq; // The first element of seq is preset type referring to enum VibratorEffectType
    struct DListHead node;
};

struct VibratorEffectCfg {
    enum VibratorConfigMode cfgMode; // References enum VibratorConfigMode
    uint32_t duration;
    const char *effect;
};

struct VibratorHapticData {
    bool supportPreset;
    struct DListHead effectSeqHead;
    struct OsalMutex mutex;
    struct OsalSem hapticSem;
    struct OsalThread hapticThread;
    bool threadExitFlag;
    uint32_t duration[VIBRATOR_TIME_INDEX_BUTT];
    int32_t effectType;
    int32_t seqCount;
    uint32_t *currentEffectSeq;
};

int32_t InitVibratorHaptic(struct HdfDeviceObject *device);
int32_t StartHaptic(struct VibratorEffectCfg *effectCfg);
int32_t StopHaptic(void);
int32_t DestroyHaptic(void);

#endif /* VIBRATOR_HAPTIC_H */
