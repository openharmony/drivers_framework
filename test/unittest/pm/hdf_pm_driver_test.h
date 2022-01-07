/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef HDF_PM_DRIVER_TEST_H
#define HDF_PM_DRIVER_TEST_H

#include "hdf_object.h"

#define SAMPLE_SERVICE "pm_service"

enum {
    HDF_PM_TEST_BEGEN = 0,
    HDF_PM_TEST_ONE_DRIVER_ONCE,
    HDF_PM_TEST_ONE_DRIVER_TWICE,
    HDF_PM_TEST_ONE_DRIVER_TEN,
    HDF_PM_TEST_ONE_DRIVER_HUNDRED,
    HDF_PM_TEST_ONE_DRIVER_THOUSAND,
    HDF_PM_TEST_TWO_DRIVER_ONCE,
    HDF_PM_TEST_TWO_DRIVER_TWICE,
    HDF_PM_TEST_TWO_DRIVER_TEN,
    HDF_PM_TEST_TWO_DRIVER_HUNDRED,
    HDF_PM_TEST_TWO_DRIVER_THOUSAND,
    HDF_PM_TEST_THREE_DRIVER_ONCE,
    HDF_PM_TEST_THREE_DRIVER_TWICE,
    HDF_PM_TEST_THREE_DRIVER_TEN,
    HDF_PM_TEST_THREE_DRIVER_HUNDRED,
    HDF_PM_TEST_THREE_DRIVER_THOUSAND,
    HDF_PM_TEST_THREE_DRIVER_SEQ_HUNDRED,
    HDF_PM_TEST_THREE_DRIVER_HUNDRED_WITH_SYNC,
    HDF_PM_TEST_END,
};

#endif // HDF_PM_DRIVER_TEST_H
