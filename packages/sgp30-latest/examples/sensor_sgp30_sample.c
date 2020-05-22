/*
 * Copyright (c) 2020, RudyLo <luhuadong@163.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-02-22     luhuadong    the first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "sgp30.h"

#define SGP30_I2C_BUS_NAME       "i2c1"

static void read_tvoc_entry(void *args)
{
    rt_device_t tvoc_dev = RT_NULL;
    struct rt_sensor_data sensor_data;

    tvoc_dev = rt_device_find(args);
    if (!tvoc_dev) 
    {
        rt_kprintf("Can't find TVOC device.\n");
        return;
    }

    if (rt_device_open(tvoc_dev, RT_DEVICE_FLAG_RDWR)) 
    {
        rt_kprintf("Open TVOC device failed.\n");
        return;
    }

    while (1)
    {
        if (1 != rt_device_read(tvoc_dev, 0, &sensor_data, 1)) 
        {
            rt_kprintf("Read TVOC data failed.\n");
            continue;
        }
        rt_kprintf("[%d] TVOC: %d\n", sensor_data.timestamp, sensor_data.data.tvoc);

        rt_thread_mdelay(2000);
    }

    rt_device_close(tvoc_dev);
}

static void read_eco2_entry(void *args)
{
    rt_device_t eco2_dev = RT_NULL;
    struct rt_sensor_data sensor_data;

    eco2_dev = rt_device_find(args);
    if (!eco2_dev) 
    {
        rt_kprintf("Can't find eCO2 device.\n");
        return;
    }

    if (rt_device_open(eco2_dev, RT_DEVICE_FLAG_RDWR)) 
    {
        rt_kprintf("Open eCO2 device failed.\n");
        return;
    }

    while(1)
    {
        if (1 != rt_device_read(eco2_dev, 0, &sensor_data, 1)) 
        {
            rt_kprintf("Read eCO2 data failed.\n");
            continue;
        }
        rt_kprintf("[%d] eCO2: %d\n", sensor_data.timestamp, sensor_data.data.eco2);

        rt_thread_mdelay(2000);
    }

    rt_device_close(eco2_dev);
}

static int sgp30_read_sample(void)
{
    rt_thread_t tvoc_thread, eco2_thread;

    tvoc_thread = rt_thread_create("tvoc_th", read_tvoc_entry, 
                                   "tvoc_sg3", 1024, 
                                    RT_THREAD_PRIORITY_MAX / 2, 20);
    
    
    eco2_thread = rt_thread_create("eco2_th", read_eco2_entry, 
                                   "eco2_sg3", 1024, 
                                    RT_THREAD_PRIORITY_MAX / 2, 20);
    
    if (tvoc_thread) 
        rt_thread_startup(tvoc_thread);

    if (eco2_thread) 
        rt_thread_startup(eco2_thread);

}
INIT_APP_EXPORT(sgp30_read_sample);

static int rt_hw_sgp30_port(void)
{
    struct rt_sensor_config cfg;
    
    cfg.intf.type = RT_SENSOR_INTF_I2C;
    cfg.intf.dev_name = SGP30_I2C_BUS_NAME;
    rt_hw_sgp30_init("sg3", &cfg);
    
    return RT_EOK;
}
INIT_COMPONENT_EXPORT(rt_hw_sgp30_port);