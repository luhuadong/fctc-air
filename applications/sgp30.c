/*
 * Copyright (c) 2020, RudyLo <luhuadong@163.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-02-01     RudyLo       the first version
 */

#define DBG_ENABLE
#define DBG_SECTION_NAME "SGP30"
#define DBG_LEVEL DBG_LOG
#define DBG_COLOR
#include <rtdbg.h>

#include "sgp30.h"

#ifdef PKG_USING_SGP30

static void write(rt_uint8_t address, rt_uint8_t *data, rt_uint8_t n)
{

}

static void read(rt_uint8_t address, rt_uint8_t *data, rt_uint8_t n)
{

}

static rt_bool_t read_word_from_command(rt_uint8_t command[], rt_uint8_t commandLength,
                          rt_uint16_t delay, rt_uint16_t *readdata, rt_uint8_t readlen)
{

}

static rt_uint8_t generateCRC(uint8_t data[], uint8_t datalen)
{

}

/**
 * This function initializes sgp30 registered device driver
 *
 * @param dev the name of sgp30 device
 *
 * @return the sgp30 device.
 */
sgp30_device_t sgp30_init(const char *i2c_bus_name)
{
	sgp30_device_t dev;

	RT_ASSERT(i2c_bus_name);

	dev = rt_calloc(1, sizeof(struct sgp30_device));
    if (dev == RT_NULL)
    {
        LOG_E("Can't allocate memory for sgp30 device on '%s' ", i2c_bus_name);
        return RT_NULL;
    }

	dev->i2c = rt_i2c_bus_device_find(i2c_bus_name);
    if (dev->i2c == RT_NULL)
    {
        LOG_E("Can't find sgp30 device on '%s' ", i2c_bus_name);
        rt_free(dev);
        return RT_NULL;
    }

    dev->lock = rt_mutex_create("mutex_sgp30", RT_IPC_FLAG_FIFO);
    if (dev->lock == RT_NULL)
    {
        LOG_E("Can't create mutex for sgp30 device on '%s' ", i2c_bus_name);
        rt_free(dev);
        return RT_NULL;
    }

    sensor_init(dev);
    return dev;
}

/**
 * This function releases memory and deletes mutex lock
 *
 * @param dev the pointer of device driver structure
 */
void sgp30_deinit(sgp30_device_t dev)
{
    RT_ASSERT(dev);

    rt_mutex_delete(dev->lock);

    rt_free(dev);
}

rt_bool_t sgp30_measure(sgp30_device_t dev)
{

}

rt_bool_t sgp30_measure_raw(sgp30_device_t dev)
{

}

rt_bool_t sgp30_get_baseline(sgp30_device_t dev, rt_uint16_t *eco2_base, rt_uint16_t *tvoc_base)
{

}

rt_bool_t sgp30_set_baseline(sgp30_device_t dev, rt_uint16_t eco2_base, rt_uint16_t tvoc_base)
{

}

rt_bool_t sgp30_set_humidity(sgp30_device_t dev, rt_uint32_t absolute_humidity)
{

}

#endif /* PKG_USING_SGP30 */
