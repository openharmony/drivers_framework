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

#include "hdf_thread_ex.h"
#include "osal_mem.h"
#include "osal_thread.h"

void HdfThreadStart(struct HdfThread *thread)
{
    if (thread == NULL) {
        return;
    }
    struct OsalThreadParam param = {
        .priority = OSAL_THREAD_PRI_DEFAULT,
        .stackSize = 0,
    };
    OsalThreadStart(&thread->adapter, &param);
    thread->status = true;
}

void HdfThreadStop(struct HdfThread *thread)
{
    if (thread == NULL) {
        return;
    }
    OsalThreadDestroy(&thread->adapter);
    thread->status = false;
}

bool HdfThreadIsRunning(struct HdfThread *thread)
{
    if (thread == NULL) {
        return false;
    }
    return thread->status;
}

void HdfThreadMain(void *argv)
{
    struct HdfThread *thread = (struct HdfThread *)argv;
    if (thread == NULL) {
        return;
    }
    if (thread->ThreadEntry != NULL) {
        thread->ThreadEntry(argv);
    } else {
        OsalThreadDestroy(&thread->adapter);
    }
}

void HdfThreadConstruct(struct HdfThread *thread)
{
    if (thread == NULL) {
        return;
    }
    thread->Start = HdfThreadStart;
    thread->Stop = HdfThreadStop;
    thread->IsRunning = HdfThreadIsRunning;
    thread->status = false;
    OsalThreadCreate(&thread->adapter, (OsalThreadEntry)HdfThreadMain, thread);
}

void HdfThreadDestruct(struct HdfThread *thread)
{
    if (thread != NULL && thread->IsRunning()) {
        thread->Stop(thread);
    }
}

struct HdfThread *HdfThreadNewInstance()
{
    struct HdfThread *thread =
        (struct HdfThread *)OsalMemCalloc(sizeof(struct HdfThread));
    if (thread != NULL) {
        HdfThreadConstruct(thread);
    }
    return thread;
}

void HdfThreadFreeInstance(struct HdfThread *thread)
{
    if (thread != NULL) {
        HdfThreadDestruct(thread);
        OsalMemFree(thread);
    }
}

