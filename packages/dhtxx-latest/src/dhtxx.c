/*
 * Copyright (c) 2020, RudyLo <luhuadong@163.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-01-20     luhuadong    the first version
 */

#include <board.h>
#include "dhtxx.h"

#define DBG_TAG "sensor.asair.dhtxx"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

/* timing */
#define DHT1x_BEGIN_TIME         20  /* ms */
#define DHT2x_BEGIN_TIME         1   /* ms */
#define DHTxx_PULL_TIME          30  /* us */
#define DHTxx_REPLY_TIME         100 /* us */
#define MEASURE_TIME             40  /* us */

RT_WEAK void rt_hw_us_delay(rt_uint32_t us)
{
    rt_uint32_t delta;

    us = us * (SysTick->LOAD / (1000000 / RT_TICK_PER_SECOND));
    delta = SysTick->VAL;

    while (delta - SysTick->VAL < us) continue;
}

/**
 * This function will split a number into two part according to times.
 *
 * @param num      the number will be split
 * @param integer  the integer part
 * @param decimal  the decimal part
 * @param times    how many times of the real number (you should use 10 in this case)
 *
 * @return 0 if num is positive, 1 if num is negative
 */
int split_int(const int num, int *integer, int *decimal, const rt_uint32_t times)
{
    int flag = 0;
    if (num < 0) flag = 1;

    int anum = num<0 ? -num : num;
    *integer = anum / times;
    *decimal = anum % times;

    return flag;
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

/**
 * This function will read a bit from sensor.
 *
 * @param pin  the pin of Dout
 *
 * @return the bit value
 */
static uint8_t dht_read_bit(const rt_base_t pin)
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
static uint8_t dht_read_byte(const rt_base_t pin)
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
 * This function will read and update data array.
 *
 * @param dev  the device to be operated
 *
 * @return RT_TRUE if read successfully, otherwise return RT_FALSE.
 */
rt_bool_t dht_read(dht_device_t dev)
{
    RT_ASSERT(dev);

    uint8_t i, retry = 0, sum = 0;
    rt_base_t level;

    /* Reset data buffer */
    rt_memset(dev->data, 0, DHT_DATA_SIZE);

    /* MCU request sampling */
    rt_pin_mode(dev->pin, PIN_MODE_OUTPUT);
    rt_pin_write(dev->pin, PIN_LOW);

    if (dev->type == DHT11 || dev->type == DHT12) {
        rt_thread_mdelay(DHT1x_BEGIN_TIME);        /* Tbe */
    } else {
        rt_thread_mdelay(DHT2x_BEGIN_TIME);
    }

#ifdef PKG_USING_DHTXX_INTERRUPT_DISABLE
    level = rt_hw_interrupt_disable();
#endif

    rt_pin_mode(dev->pin, PIN_MODE_INPUT_PULLUP);
    rt_hw_us_delay(DHTxx_PULL_TIME);               /* Tgo */

    /* Waiting for sensor reply */
    while (rt_pin_read(dev->pin) && retry < DHTxx_REPLY_TIME)
    {
        retry++;
        rt_hw_us_delay(1);                         /* Trel */
    }
    if(retry >= DHTxx_REPLY_TIME) return RT_FALSE;

    retry = 0;
    while (!rt_pin_read(dev->pin) && retry < DHTxx_REPLY_TIME)
    {
        retry++;
        rt_hw_us_delay(1);                         /* Treh */
    };
    if(retry >= DHTxx_REPLY_TIME) return RT_FALSE;

    /* Read data */
    for(i=0; i<DHT_DATA_SIZE; i++)
    {
        dev->data[i] = dht_read_byte(dev->pin);
    }

#ifdef PKG_USING_DHTXX_INTERRUPT_DISABLE
    rt_hw_interrupt_enable(level);
#endif

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
rt_int32_t dht_get_humidity(dht_device_t const dev)
{
    RT_ASSERT(dev);

    rt_int32_t humi = 0;

    switch(dev->type)
    {
    case DHT11:
    case DHT12:
        humi = dev->data[0] * 10 + dev->data[1];
        break;
    case DHT21:
    case DHT22:
        humi = (dev->data[0] << 8) + dev->data[1];
        break;
    default:
        break;
    }

    return humi;
}

/**
 * This function will get the temperature from dhtxx sensor.
 *
 * @param dev  the device to be operated
 *
 * @return the temperature value
 */
rt_int32_t dht_get_temperature(dht_device_t const dev)
{
    RT_ASSERT(dev);

    rt_int32_t temp = 0;

    switch(dev->type)
    {
    case DHT11:
    case DHT12:
        temp = dev->data[2] * 10 + dev->data[3];
        break;
    case DHT21:
    case DHT22:
        temp = ((dev->data[2] & 0x7f) << 8) + dev->data[3];
        if(dev->data[2] & 0x80) {
            temp = -temp;
        }
        break;
    default:
        break;
    }

    return temp;
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
rt_err_t dht_init(struct dht_device *dev, const rt_uint8_t type, const rt_base_t pin)
{
    if(dev == NULL)
        return -RT_ERROR;

    dev->type = type;
    dev->pin  = pin;

    rt_memset(dev->data, 0, DHT_DATA_SIZE);
    rt_pin_mode(dev->pin, PIN_MODE_INPUT_PULLUP);
    
    return RT_EOK;
}

dht_device_t dht_create(const rt_uint8_t type, const rt_base_t pin)
{
    dht_device_t dev;

    dev = rt_calloc(1, sizeof(struct dht_device));
    if (dev == RT_NULL)
    {
        LOG_E("Can't allocate memory for dhtxx device");
        return RT_NULL;
    }

    dev->type = type;
    dev->pin  = pin;

    rt_memset(dev->data, 0, DHT_DATA_SIZE);
    rt_pin_mode(dev->pin, PIN_MODE_INPUT_PULLUP);

    return dev;
}

void dht_delete(dht_device_t dev)
{
    if (dev)
        rt_free(dev);
}
