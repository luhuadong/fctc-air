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

RT_WEAK void rt_hw_us_delay(rt_uint32_t us)
{
    rt_uint32_t delta;

    us = us * (SysTick->LOAD / (1000000 / RT_TICK_PER_SECOND));
    delta = SysTick->VAL;

    while (delta - SysTick->VAL < us) continue;
}

gp2y10_device_t gp2y10_init(gp2y10_device_t dev, rt_base_t iled_pin, rt_base_t aout_pin)
{
	if(dev == NULL) return RT_NULL;

    /* 查找设备 */
    dev->adc_dev = (rt_adc_device_t)rt_device_find(ADC_DEV_NAME);

    if (dev->adc_dev == RT_NULL)
    {
        rt_kprintf("(GP2Y10) Can't find %s device!\n", ADC_DEV_NAME);
        //return RT_ERROR;
    }
	
	dev->iled_pin = iled_pin;
	dev->aout_pin = aout_pin;

	rt_pin_mode(dev->iled_pin, PIN_MODE_OUTPUT);
    rt_pin_write(dev->iled_pin, PIN_LOW);

	return dev;
}

void gp2y10_deinit(gp2y10_device_t dev)
{
    RT_ASSERT(dev);

    rt_mutex_delete(dev->lock);

    rt_free(dev);
}

#ifdef GP2Y10_USING_SOFT_FILTER
/*!
 *  @brief  Read value from sensor and update data array.
 *  @param  usec
 *          Optionally pass pull-up time (in microseconds) before DHT reading
 *          starts. Default is 55 (see function declaration in dht.h).
 *  @return If read data successfully return DHTLIB_OK,
 *          if the data are not complete return DHTLIB_ERROR_CHECKSUM,
 *          if read timeout return DHTLIB_ERROR_TIMEOUT.
 */
rt_uint32_t gp2y10_filter(rt_uint32_t m)
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

rt_uint32_t gp2y10_get_adc_value(gp2y10_device_t dev)
{
    rt_uint32_t adc_value;

    /* 使能设备 */
    rt_adc_enable(dev->adc_dev, ADC_DEV_CHANNEL);

    /* Get ADC value */
    rt_pin_write(dev->iled_pin, PIN_HIGH);
    rt_hw_us_delay(PULSE_TIME);

    /* 读取采样值 */
    adc_value = rt_adc_read(dev->adc_dev, ADC_DEV_CHANNEL);
    
    rt_pin_write(dev->iled_pin, PIN_LOW);

    //rt_kprintf("(GP2Y10) ADC:%d\n", adc_value);

    /* 关闭通道 */
    rt_adc_disable(dev->adc_dev, ADC_DEV_CHANNEL);

#ifdef GP2Y10_USING_SOFT_FILTER
    return gp2y10_filter(adc_value);
#else
    return adc_value;
#endif
}

float gp2y10_get_voltage(gp2y10_device_t dev)
{
    rt_uint32_t adc_value;
    float voltage;

    adc_value = gp2y10_get_adc_value(dev);

    /* convert */
    voltage = adc_value * VOLTAGE_RATIO * SYS_VOLTAGE / CONVERT_BITS;

    //rt_kprintf("(GP2Y10) ADC:%d, Voltage:%dmv\n", adc_value, (int)voltage);

    return voltage;
}

float gp2y10_get_dust_density(gp2y10_device_t dev)
{
	float voltage, density;

    voltage = gp2y10_get_voltage(dev);

    if(voltage >= NO_DUST_VOLTAGE)
    {
    	voltage -= NO_DUST_VOLTAGE;
    	density  = voltage * COV_RATIO;
    }
    else density = 0.0;

    //rt_kprintf("(GP2Y10) Voltage:%dmv, Dust:%dppm\n", (int)voltage, (int)density);

    return density;
}