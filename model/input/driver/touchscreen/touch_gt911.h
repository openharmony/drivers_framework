/*
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#ifndef TOUCH_GT911_H
#define TOUCH_GT911_H

/* the macro defines of GT911 */
#define MAX_SUPPORT_POINT     10
#define ONE_BYTE_MASK         0xFF
#define ONE_BYTE_OFFSET       8
#define GT_EVENT_UP           0x80
#define GT_EVENT_INVALID      0

#define GT_POINT_SIZE         8
#define GT_X_LOW              0
#define GT_X_HIGH             1
#define GT_Y_LOW              2
#define GT_Y_HIGH             3
#define GT_ADDR_LEN           2
#define GT_BUF_STATE_ADDR     0x814E
#define GT_X_LOW_BYTE_BASE    0x8150
#define GT_FINGER_NUM_MASK    0x0F
#define GT_CLEAN_DATA_LEN     3
#define GT_REG_HIGH_POS       0
#define GT_REG_LOW_POS        1
#define GT_CLEAN_POS          2
#define GT_CLEAN_FLAG         0x0
/* Config info macro of GT911 */
#define GT_CFG_INFO_ADDR      0x8140
#define GT_CFG_INFO_LEN       10
#define GT_PROD_ID_1ST        0
#define GT_PROD_ID_2ND        1
#define GT_PROD_ID_3RD        2
#define GT_PROD_ID_4TH        3
#define GT_FW_VER_LOW         4
#define GT_FW_VER_HIGH        5
#define GT_SOLU_X_LOW         6
#define GT_SOLU_X_HIGH        7
#define GT_SOLU_Y_LOW         8
#define GT_SOLU_Y_HIGH        9

#endif