/*
 * Copyright (c) 2020, RudyLo <luhuadong@163.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-01-21     RudyLo       the first version
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
	
	dev->iled_pin = iled_pin;
	dev->aout_pin = aout_pin;

	rt_pin_mode(dev->iled_pin, PIN_MODE_OUTPUT);
    rt_pin_write(dev->iled_pin, PIN_LOW);

	return dev;
}

rt_uint32_t gp2y10_get_adc_value(gp2y10_device_t dev)
{
    rt_uint32_t adc_value;
    rt_adc_device_t adc_dev;            /* ADC 设备句柄 */ 

    /* 查找设备 */
    adc_dev = (rt_adc_device_t)rt_device_find(ADC_DEV_NAME);

    if (adc_dev == RT_NULL)
    {
        rt_kprintf("(GP2Y10) Can't find %s device!\n", ADC_DEV_NAME);
        //return RT_ERROR;
    }

    /* 使能设备 */
    rt_adc_enable(adc_dev, ADC_DEV_CHANNEL);

    /* Get ADC value */
    rt_pin_write(dev->iled_pin, PIN_HIGH);
    rt_hw_us_delay(PULSE_TIME);

    /* 读取采样值 */
    adc_value = rt_adc_read(adc_dev, ADC_DEV_CHANNEL);
    
    rt_pin_write(dev->iled_pin, PIN_LOW);

    rt_kprintf("(GP2Y10) ADC:%d\n", adc_value);

    /* 关闭通道 */
    rt_adc_disable(adc_dev, ADC_DEV_CHANNEL);

    return adc_value;
}

float gp2y10_get_voltage(gp2y10_device_t dev)
{
    rt_uint32_t adc_value;
    float voltage;

    adc_value = gp2y10_get_adc_value(dev);

    /* convert */
    voltage = adc_value * VOLTAGE_RATIO * SYS_VOLTAGE / CONVERT_BITS;

    rt_kprintf("(GP2Y10) ADC:%d, Voltage:%dmv\n", adc_value, (int)voltage);

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

    rt_kprintf("(GP2Y10) Voltage:%dmv, Dust:%dppm\n", (int)voltage, (int)density);

    return density;
}