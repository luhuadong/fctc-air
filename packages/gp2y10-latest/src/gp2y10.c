/*
 * Copyright (c) 2020, RudyLo <luhuadong@163.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-01-21     luhuadong    the first version
 */

#include "gp2y10.h"

#define DBG_TAG "sensor.sharp.gp2y10"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

#define ILED_PULSE_TIME          280    /* us */
#define COV_RATIO                0.17   /* (ug/m3)/mV */
#define NO_DUST_VOLTAGE          600    /* mV */
#define REFER_VOLTAGE            5000   /* mV */

RT_WEAK void rt_hw_us_delay(rt_uint32_t us)
{
    rt_uint32_t delta;

    us = us * (SysTick->LOAD / (1000000 / RT_TICK_PER_SECOND));
    delta = SysTick->VAL;

    while (delta - SysTick->VAL < us) continue;
}

#ifdef PKG_USING_GP2Y10_SOFT_FILTER
static rt_uint32_t gp2y10_filter(rt_uint32_t m)
{
    static int flag_first = 0, _buff[10], sum;
    const int _buff_max = 10;
    rt_uint32_t i;

    if(flag_first == 0)
    {
        flag_first = 1;

        for(i = 0, sum = 0; i < _buff_max; i++)
        {
            _buff[i] = m;
            sum += _buff[i];
        }
        return m;
    }
    else
    {
        sum -= _buff[0];
        for(i = 0; i < (_buff_max - 1); i++)
        {
            _buff[i] = _buff[i + 1];
        }
        _buff[9] = m;
        sum += _buff[9];
    
        i = sum / 10.0;
        return i;
    }
}
#endif

static rt_uint32_t gp2y10_get_adc_value(gp2y10_device_t dev)
{
    rt_uint32_t adc_value;

    rt_adc_enable(dev->adc_dev, ADC_DEV_CHANNEL);    /* 使能设备 */
    
    rt_pin_write(dev->iled_pin, PIN_HIGH);           /* Get ADC value */
    rt_hw_us_delay(ILED_PULSE_TIME);

    adc_value = rt_adc_read(dev->adc_dev, ADC_DEV_CHANNEL);   /* 读取采样值 */
    
    rt_pin_write(dev->iled_pin, PIN_LOW);
    rt_adc_disable(dev->adc_dev, ADC_DEV_CHANNEL);   /* 关闭通道 */

    LOG_D("ADC: %d", adc_value);

#ifdef PKG_USING_GP2Y10_SOFT_FILTER
    return gp2y10_filter(adc_value);
#else
    return adc_value;
#endif
}

static float gp2y10_get_voltage(gp2y10_device_t dev)
{
    rt_uint32_t adc_value;
    float voltage;

    adc_value = gp2y10_get_adc_value(dev);

    /* convert */
    voltage = adc_value * VOLTAGE_RATIO * REFER_VOLTAGE / CONVERT_BITS;

    LOG_D("Voltage: %dmv", (int)voltage);

    return voltage;
}

rt_uint32_t gp2y10_get_dust_density(gp2y10_device_t dev)
{
	float       voltage;
    rt_uint32_t density;

    voltage = gp2y10_get_voltage(dev);

    if(voltage >= NO_DUST_VOLTAGE)
    {
    	voltage -= NO_DUST_VOLTAGE;
    	density  = voltage * COV_RATIO;
    }
    else density = 0;

    LOG_D("Dust density: %d ug/m3", density);

    return density;
}

rt_err_t gp2y10_init(struct gp2y10_device *dev, const rt_base_t iled_pin, const rt_base_t aout_pin)
{
    RT_ASSERT(dev);

    /* 查找设备 */
    dev->adc_dev = (rt_adc_device_t)rt_device_find(ADC_DEV_NAME);
    if (dev->adc_dev == RT_NULL)
    {
        LOG_E("Can't find %s device!", ADC_DEV_NAME);
        return -RT_ERROR;
    }
    
    dev->iled_pin = iled_pin;
    dev->aout_pin = aout_pin;

    rt_pin_mode(dev->iled_pin, PIN_MODE_OUTPUT);
    rt_pin_write(dev->iled_pin, PIN_LOW);

    return RT_EOK;
}

gp2y10_device_t gp2y10_create(const rt_base_t iled_pin, const rt_base_t aout_pin)
{
    gp2y10_device_t dev;

    dev = rt_calloc(1, sizeof(struct gp2y10_device));
    if (dev == RT_NULL)
    {
        LOG_E("Can't allocate memory for gp2y10 device");
        return RT_NULL;
    }

    dev->adc_dev = (rt_adc_device_t)rt_device_find(ADC_DEV_NAME);

    if (dev->adc_dev == RT_NULL)
    {
        LOG_E("Can't find %s device!", ADC_DEV_NAME);
        rt_free(dev);
        return RT_NULL;
    }

    dev->iled_pin = iled_pin;
    dev->aout_pin = aout_pin;

    rt_pin_mode(dev->iled_pin, PIN_MODE_OUTPUT);
    rt_pin_write(dev->iled_pin, PIN_LOW);

    return dev;
}

void gp2y10_delete(gp2y10_device_t dev)
{
    if (dev)
    {
        //rt_mutex_delete(dev->lock);
        rt_free(dev);
    }
}
