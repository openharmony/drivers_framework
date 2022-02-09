# Copyright (c) 2021 Huawei Device Co., Ltd.
#
# HDF is dual licensed: you can use it either under the terms of
# the GPL, or the BSD license, at your option.
# See the LICENSE file in the root of this repository for complete details.

include $(LITEOSTOPDIR)/config.mk
include $(LITEOSTOPDIR)/../../drivers/adapter/khdf/liteos/lite.mk

MODULE_NAME := hdf_uart_sample

LOCAL_CFLAGS += $(HDF_INCLUDE)

LOCAL_SRCS += src/uart_sample.c \
              src/uart_pl011_sample.c \
              src/buf_fifo.c \
              src/uart_dev_sample.c \
              src/uart_dispatch_sample.c \

LOCAL_INCLUDE := ./include
LOCAL_CFLAGS += -fstack-protector-strong

include $(HDF_DRIVER)

