/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "osal_spinlock.h"
#include <pthread.h>
#include "hdf_log.h"
#include "osal_mem.h"

#define HDF_LOG_TAG osal_spinlock

int32_t OsalSpinInit(OsalSpinlock *spinlock)
{
    pthread_spinlock_t *spinTmp = NULL;
    int ret;

    if (spinlock == NULL) {
        HDF_LOGE("%s invalid param", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    spinlock->realSpinlock = NULL;

    spinTmp = (pthread_spinlock_t *)OsalMemCalloc(sizeof(pthread_spinlock_t));
    if (spinTmp == NULL) {
        HDF_LOGE("malloc fail");
        return HDF_ERR_MALLOC_FAIL;
    }

    ret = pthread_spin_init(spinTmp, PTHREAD_PROCESS_PRIVATE);
    if (ret != 0) {
        HDF_LOGE("pthread_spin_init fail %d %d", ret, __LINE__);
        OsalMemFree(spinTmp);
        return HDF_FAILURE;
    }
    spinlock->realSpinlock = (void *)spinTmp;

    return HDF_SUCCESS;
}

int32_t OsalSpinDestroy(OsalSpinlock *spinlock)
{
    int ret;

    if (spinlock == NULL || spinlock->realSpinlock == NULL) {
        HDF_LOGE("%s invalid param", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    ret = pthread_spin_destroy((pthread_spinlock_t *)spinlock->realSpinlock);
    if (ret != 0) {
        HDF_LOGE("pthread_spin_destroy fail %d %d", ret, __LINE__);
        return HDF_FAILURE;
    }

    OsalMemFree(spinlock->realSpinlock);
    spinlock->realSpinlock = NULL;

    return HDF_SUCCESS;
}

int32_t OsalSpinLock(OsalSpinlock *spinlock)
{
    int ret;

    if (spinlock == NULL || spinlock->realSpinlock == NULL) {
        HDF_LOGE("%s invalid param", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    ret = pthread_spin_lock((pthread_spinlock_t *)spinlock->realSpinlock);
    if (ret != 0) {
        HDF_LOGE("pthread_spin_lock fail %d %d", ret, __LINE__);
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

int32_t OsalSpinUnlock(OsalSpinlock *spinlock)
{
    int ret;

    if (spinlock == NULL || spinlock->realSpinlock == NULL) {
        HDF_LOGE("%s invalid param", __func__);
        return HDF_ERR_INVALID_PARAM;
    }

    ret = pthread_spin_unlock((pthread_spinlock_t *)spinlock->realSpinlock);
    if (ret != 0) {
        HDF_LOGE("pthread_spin_unlock fail %d %d", ret, __LINE__);
        return HDF_FAILURE;
    }

    return HDF_SUCCESS;
}

