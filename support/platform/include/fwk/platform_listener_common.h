/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef PLATFORM_LISTENER_COMMON_H
#define PLATFORM_LISTENER_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

enum PlatformListenerEventID {
    PLATFORM_LISTENER_EVENT_GPIO_INT_NOTIFY,
    PLATFORM_LISTENER_EVENT_TIMER_NOTIFY,
    PLATFORM_LISTENER_EVENT_RTC_ALARM_NOTIFY,
};

#define LISTENER_MATCH_INFO_LEN 256
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* PLATFORM_LISTENER_COMMON_H */
