/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef RANDOM
#define RANDOM

#include <random>
#include "hdf_base.h"

int32_t randNum(int32_t min, int32_t max)
{
    std::random_device rd;
    std::default_random_engine engine(rd());
    std::uniform_int_distribution<int32_t> randomNum(min, max);
    return randomNum(engine);
}

#endif