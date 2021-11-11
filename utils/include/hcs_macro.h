/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef HCS_MACRO_H
#define HCS_MACRO_H

// define utils macro
#define HCS_CAT(part1, part2) part1 ## part2
#define HCS_ROOT root

// define macro for module
#define HCS_NODE_EXISTS(node) HCS_CAT(node, _exists)
#define HCS_NODE(parent, node) HCS_CAT(parent, _##node)
#define HCS_NODE_HAS_PROP(node, prop) HCS_CAT(node, _##prop##_exists)
#define HCS_PROP(node, prop) HCS_CAT(node, _##prop)

#define HCS_FOREACH_CHILD(node, func) \
    HCS_CAT(node, _foreach_child)(func)

#define HCS_FOREACH_CHILD_VARGS(node, func, ...) \
    HCS_CAT(node, _foreach_child_vargs)(func, __VA_ARGS__)

#define HCS_ARRAYS(arrays_node) HCS_CAT(arrays_node, _data)

#define HCS_ARRAYS_SIZE(arrays_node) HCS_CAT(arrays_node, _array_size)

#endif // HCS_MACRO_H