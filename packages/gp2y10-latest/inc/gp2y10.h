/*
 * Copyright (c) 2020, RudyLo <luhuadong@163.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-01-21     luhuadong    the first version
 */

#ifndef __GP2Y10_H__
#define __GP2Y10_H__

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <sensor.h>

#define GP2Y10LIB_VERSION   "0.8.1"

#ifndef PKG_USING_GP2Y10_ADC_DEV_NAME
#define ADC_DEV_NAME        "adc1"
#else
#define ADC_DEV_NAME        PKG_USING_GP2Y10_ADC_DEV_NAME
#endif

#ifndef PKG_USING_GP2Y10_ADC_DEV_CHANNEL
#define ADC_DEV_CHANNEL     4
#else
#define ADC_DEV_CHANNEL     PKG_USING_GP2Y10_ADC_DEV_CHANNEL
#endif

#ifndef PKG_USING_GP2Y10_CONVERT_BITS
#define CONVERT_BITS        (1 << 12)
#else
#define CONVERT_BITS        (1 << PKG_USING_GP2Y10_CONVERT_BITS)
#endif

#ifndef PKG_USING_GP2Y10_VOLTAGE_RATIO
#define VOLTAGE_RATIO       11
#else
#define VOLTAGE_RATIO       PKG_USING_GP2Y10_VOLTAGE_RATIO
#endif

// PKG_USING_GP2Y10_SOFT_FILTER

struct gp2y10_device
{
	rt_adc_device_t    adc_dev;
	rt_base_t          iled_pin;
	rt_base_t          aout_pin;
	rt_mutex_t         lock;
};
typedef struct gp2y10_device *gp2y10_device_t;

rt_err_t        gp2y10_init(struct gp2y10_device *dev, const rt_base_t iled_pin, const rt_base_t aout_pin);
gp2y10_device_t gp2y10_create(const rt_base_t iled_pin, const rt_base_t aout_pin);
void            gp2y10_delete(gp2y10_device_t dev);
rt_uint32_t     gp2y10_get_dust_density(gp2y10_device_t dev);

rt_err_t rt_hw_gp2y10_init(const char *name, struct rt_sensor_config *cfg);

#endif /* __GP2Y10_H__ */