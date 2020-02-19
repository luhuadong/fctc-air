/*
 * Copyright (c) 2020, RudyLo <luhuadong@163.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-01-20     luhuadong    the first version
 */

#include "dht.h"
#if 0
RT_WEAK void rt_hw_us_delay(rt_uint32_t us)
{
    rt_uint32_t delta;

    us = us * (SysTick->LOAD / (1000000 / RT_TICK_PER_SECOND));
    delta = SysTick->VAL;

    while (delta - SysTick->VAL < us) continue;
}

/**
 * This function will read a bit from sensor.
 *
 * @param pin  the pin of Dout
 *
 * @return the bit value
 */
static uint8_t dht_read_bit(rt_base_t pin)
{
	uint8_t retry = 0;

	while(rt_pin_read(pin) && retry < DHTxx_REPLY_TIME)
    {
        retry++;
        rt_hw_us_delay(1);
    }

	retry = 0;
	while(!rt_pin_read(pin) && retry < DHTxx_REPLY_TIME)
    {
        retry++;
        rt_hw_us_delay(1);
    }

	rt_hw_us_delay(MEASURE_TIME);
	
	return rt_pin_read(pin);
}

/**
 * This function will read a byte from sensor.
 *
 * @param pin  the pin of Dout
 *
 * @return the byte
 */
static uint8_t dht_read_byte(rt_base_t pin)
{
	uint8_t i, byte = 0;

    for(i=0; i<8; i++)
    {
        byte <<= 1;
		byte |= dht_read_bit(pin);
    }

    return byte;
}

/**
 * This function will init dhtxx sensor device.
 *
 * @param dev  the device to init
 * @param type the type of sensor
 * @param pin  the pin of Dout
 *
 * @return the device handler
 */
dht_device_t dht_init(dht_device_t dev, dht_type type, rt_base_t pin)
{
	if(dev == NULL) return RT_NULL;
	
	dev->type = type;
	dev->pin  = pin;

	dev->begin_time = DHT2x_BEGIN_TIME;
	if(type == SENSOR_DHT11) dev->begin_time = DHT11_BEGIN_TIME;                  

	rt_memset(dev->data, 0, DHT_DATA_SIZE);

	return dev;
}

/**
 * This function will read and update data array.
 *
 * @param dev  the device to be operated
 *
 * @return RT_TRUE if read successfully, otherwise return RT_FALSE.
 */
rt_bool_t dht_read(dht_device_t dev)
{
	uint8_t i, retry = 0, sum = 0;

	/* Reset data buffer */
	rt_memset(dev->data, 0, DHT_DATA_SIZE);

	/* MCU request sampling */
	rt_pin_mode(dev->pin, PIN_MODE_OUTPUT);
    rt_pin_write(dev->pin, PIN_LOW);
    rt_thread_mdelay(dev->begin_time);             /* Tbe */

#if 0
    rt_pin_write(dev->pin, PIN_HIGH);
    rt_hw_us_delay(DHTxx_PULL_TIME);
    rt_pin_mode(dev->pin, PIN_MODE_INPUT);
#else
    rt_pin_mode(dev->pin, PIN_MODE_INPUT_PULLUP);
    rt_hw_us_delay(DHTxx_PULL_TIME);               /* Tgo */
#endif

    //rt_kprintf("(I) here 1\n");

    /* Waiting for sensor reply */
    while (rt_pin_read(dev->pin) && retry < DHTxx_REPLY_TIME)
    {
        retry++;
        rt_hw_us_delay(1);                         /* Trel */
    }
    if(retry >= DHTxx_REPLY_TIME) return RT_FALSE;

    //rt_kprintf("(I) here 2\n");

    retry = 0;
    while (!rt_pin_read(dev->pin) && retry < DHTxx_REPLY_TIME)
    {
        retry++;
        rt_hw_us_delay(1);                         /* Treh */
    };
    if(retry >= DHTxx_REPLY_TIME) return RT_FALSE;

    //rt_kprintf("(I) here 3\n");

    /* Read data */
    for(i=0; i<DHT_DATA_SIZE; i++)
	{
		dev->data[i] = dht_read_byte(dev->pin);
	}

	//rt_kprintf("(I) here 4\n");

	/* Checksum */
    for(i=0; i<DHT_DATA_SIZE-1; i++)
	{
		sum += dev->data[i];
	}
	if(sum != dev->data[4]) return RT_FALSE;

	return RT_TRUE;
}

/**
 * This function will get the humidity from dhtxx sensor.
 *
 * @param dev  the device to be operated
 *
 * @return the humidity value
 */
float dht_get_humidity(dht_device_t dev)
{
	float h = 0.0;

	switch(dev->type)
	{
	case SENSOR_DHT11:
		h = dev->data[0] + dev->data[1] * 0.1;
		break;
	case SENSOR_DHT21:
	case SENSOR_DHT22:
		h = ((dev->data[0] << 8) + dev->data[1]) * 0.1;
		break;
	default:
		break;
	}

	return h;
}

/**
 * This function will get the temperature from dhtxx sensor.
 *
 * @param dev  the device to be operated
 *
 * @return the temperature value
 */
float dht_get_temperature(dht_device_t dev)
{
	float t = 0.0;

	switch(dev->type)
	{
	case SENSOR_DHT11:
		t = dev->data[2] + dev->data[3] * 0.1;
		break;
	case SENSOR_DHT21:
	case SENSOR_DHT22:
		t = (((dev->data[2] & 0x7f) << 8) + dev->data[3]) * 0.1;
		if(dev->data[2] & 0x80) {
			t = -t;
		}
		break;
	default:
		break;
	}

	return t;
}

/**
 * This function will convert temperature in degree Celsius to Kelvin.
 *
 * @param c  the temperature indicated by degree Celsius
 *
 * @return the result
 */
float convert_c2k(float c)
{
	return c + 273.15;
}

/**
 * This function will convert temperature in degree Celsius to Fahrenheit.
 *
 * @param c  the temperature indicated by degree Celsius
 *
 * @return the result
 */
float convert_c2f(float c)
{
	return c * 1.8 + 32;
}

/**
 * This function will convert temperature in degree Fahrenheit to Celsius.
 *
 * @param f  the temperature indicated by degree Fahrenheit
 *
 * @return the result
 */
float convert_f2c(float f)
{
	return (f - 32) * 0.55555;
}
#endif