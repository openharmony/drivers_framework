/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 *
 * HDF is dual licensed: you can use it either under the terms of
 * the GPL, or the BSD license, at your option.
 * See the LICENSE file in the root of this repository for complete details.
 */

#include <linux/device.h>
#include <linux/init.h>
#include <linux/iio/iio.h>
#include <linux/iio/sysfs.h>
#include <linux/iio/buffer.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include "hdf_log.h"
#include "hdf_base.h"

#define ADC_SCALE 32680000
#define ADC_SCALE_DATA 100000
#define ADC_VIRTUAL_RAW_DATA 119
#define REAL_BITS 16
#define STORAGE_BITS 16
#define SHIFT_BITS 0
#define SCAN_MASKS 0xF
#define BUS_NUM 2

enum AdcScanIndex {
    ADC_SCAN_VOLTAGE_1,
    ADC_SCAN_VOLTAGE_2,
};

struct VirtualAdcDev {
    char *name;
    int BusNum;
    struct mutex lock;
};

static int AdcReadChannelData(struct iio_dev *indioDev, struct iio_chan_spec const *chan, int *val)
{
    if (val == NULL) {
        HDF_LOGE("%s: val is NULL", __func__);
        return -EINVAL;
    }
    (void)indioDev;
    (void)chan;
    *val = ADC_VIRTUAL_RAW_DATA;
    return IIO_VAL_INT;
}

/*
 * val : The value written by the application, val is the integer part if it is a fractional value.
 * val2 : The value written by the application, if it is a fractional value, val2 is the fractional part.
*/
static int AdcReadRaw(struct iio_dev *indioDev, struct iio_chan_spec const *chan, int *val, int *val2, long mask)
{
    struct VirtualAdcDev *dev = NULL;
    int ret;
 
    if (indioDev == NULL || chan == NULL || val == NULL || val2 == NULL) {
        HDF_LOGE("%s: Illegal parameter", __func__);
        return -EINVAL;
    }
    if (ADC_SCALE_DATA == 0) {
        HDF_LOGE("%s: ADC_SCALE_DATA is Illegal", __func__);
        return -EINVAL;
    }
 
    dev = iio_priv(indioDev);
    switch (mask) {
        case IIO_CHAN_INFO_RAW:
            iio_device_claim_direct_mode(indioDev);
            mutex_lock(&dev->lock);
            ret = AdcReadChannelData(indioDev, chan, val);
            mutex_unlock(&dev->lock);
            iio_device_release_direct_mode(indioDev);
            break;
        case IIO_CHAN_INFO_SCALE:
            *val = ADC_SCALE / ADC_SCALE_DATA;
            *val2 = ADC_SCALE % ADC_SCALE_DATA;
            ret = IIO_VAL_INT_PLUS_MICRO;
            break;
        default:
            HDF_LOGE("%s: mask is Illegal", __func__);
            ret = -EINVAL;
            break;
        }

    return ret;
}

static const struct iio_chan_spec AdcChannels[] = {
    {
        .type = IIO_VOLTAGE,
        .differential = 1,
        .indexed = 1,
        .channel = ADC_SCAN_VOLTAGE_1,
        .info_mask_separate = BIT(IIO_CHAN_INFO_RAW) | BIT(IIO_CHAN_INFO_OFFSET) | BIT(IIO_CHAN_INFO_SCALE),
        .scan_index = ADC_SCAN_VOLTAGE_1,
        .scan_type = {
            .sign = 's',
            .realbits = REAL_BITS,
            .storagebits = STORAGE_BITS,
            .shift = SHIFT_BITS,
            .endianness = IIO_BE,
        },
    },
    {
        .type = IIO_VOLTAGE,
        .differential = 1,
        .indexed = 1,
        .channel = ADC_SCAN_VOLTAGE_2,
        .info_mask_separate = BIT(IIO_CHAN_INFO_RAW) | BIT(IIO_CHAN_INFO_OFFSET) | BIT(IIO_CHAN_INFO_SCALE),
        .scan_index = ADC_SCAN_VOLTAGE_2,
        .scan_type = {
            .sign = 's',
            .realbits = REAL_BITS,
            .storagebits = STORAGE_BITS,
            .shift = SHIFT_BITS,
            .endianness = IIO_BE,
        },
    },
};

static const struct iio_info AdcIioInfo = {
    .read_raw = AdcReadRaw,
};

static void VirtualAdcDevRelease(struct device *dev)
{
    (void)dev;
}

static struct platform_device g_virtualAdcPlatformDevice = {
    .name = "virtual_adc_dev",
    .id = -1,
    .dev = {
        .release = VirtualAdcDevRelease,
    }
};

static int VirtualAdcPlatformProbe(struct platform_device *pdev)
{
    struct VirtualAdcDev *adcInfo = NULL;
    struct iio_dev *indioDev = NULL;
    int ret;

    indioDev = devm_iio_device_alloc(&pdev->dev, sizeof(*adcInfo));
    if (!indioDev) {
        HDF_LOGE("%s: malloc iio_dev fail", __func__);
        return -ENOMEM;
    }
    adcInfo = iio_priv(indioDev);
    adcInfo->name = "virtual-adc-3516";
    adcInfo->BusNum = BUS_NUM;
    
    indioDev->name = KBUILD_MODNAME;
    indioDev->channels = AdcChannels;
    indioDev->num_channels = ARRAY_SIZE(AdcChannels);
    indioDev->available_scan_masks = SCAN_MASKS;
    indioDev->info = &AdcIioInfo;
    indioDev->modes = INDIO_DIRECT_MODE;

    platform_set_drvdata(pdev, indioDev);
	
    ret = iio_device_register(indioDev);
    if (ret < 0) {
        HDF_LOGE("%s: iio device register fail", __func__);
        return ret;
    }
	
    return HDF_SUCCESS;
}

static int VirtualAdcPlatformRemove(struct platform_device *pdev)
{
    struct iio_dev *indioDev = NULL;

    indioDev = platform_get_drvdata(pdev);
    if (indioDev == NULL) {
        HDF_LOGE("%s: iio device is NULL", __func__);
        return -EINVAL;
    }
    iio_device_unregister(indioDev);
    platform_set_drvdata(pdev, NULL);

    return HDF_SUCCESS;
}

static struct platform_driver g_virtualAdcPlatformDriver = {
    .driver = {
        .name = "virtual_adc_dev",
        .owner = THIS_MODULE,
    },
    .probe = VirtualAdcPlatformProbe,
    .remove = VirtualAdcPlatformRemove,
};

static int __init VirtualAdcInit(void)
{
    int ret;

    ret = platform_device_register(&g_virtualAdcPlatformDevice);
    if (ret == 0) {
        ret = platform_driver_register(&g_virtualAdcPlatformDriver);
    }

    return ret;
}

static void __exit VirtualAdcExit(void)
{
    platform_device_unregister(&g_virtualAdcPlatformDevice);
    platform_driver_unregister(&g_virtualAdcPlatformDriver);
}
module_init(VirtualAdcInit);
module_exit(VirtualAdcExit);
