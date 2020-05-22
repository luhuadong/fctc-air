/*
 * Copyright (c) 2020, RudyLo <luhuadong@163.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-02-20     luhuadong    the first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>

#include "gp2y10.h"

#define GP2Y10_ILED_PIN          GET_PIN(F, 15)  /* D2 */
#define GP2Y10_AOUT_PIN          GET_PIN(C, 3)   /* A2 */

static void read_dust_entry(void *args)
{
    rt_device_t dust_dev = RT_NULL;
    struct rt_sensor_data sensor_data;

    dust_dev = rt_device_find(args);
    if (dust_dev == RT_NULL)
    {
        rt_kprintf("Can't find dust device.\n");
        return;
    }

    if (rt_device_open(dust_dev, RT_DEVICE_FLAG_RDWR)) 
    {
        rt_kprintf("Open dust device failed.\n");
        return;
    }

    while(1)
    {
        if (1 != rt_device_read(dust_dev, 0, &sensor_data, 1))
        {
            rt_kprintf("Read dust data failed.\n");
            continue;
        }
        rt_kprintf("[%d] Dust: %d\n", sensor_data.timestamp, sensor_data.data.dust);

        rt_thread_mdelay(2000);
    }

    rt_device_close(dust_dev);
}

static int gp2y10_read_sample(void)
{
    rt_thread_t dust_thread;

    dust_thread = rt_thread_create("dust_th", read_dust_entry, 
                                   "dust_gp2", 1024, 
                                    RT_THREAD_PRIORITY_MAX / 2, 20);
    
    if (dust_thread) 
        rt_thread_startup(dust_thread);
}
INIT_APP_EXPORT(gp2y10_read_sample);

static int rt_hw_gp2y10_port(void)
{
    struct gp2y10_device gp2y10_dev;
    struct rt_sensor_config cfg;

    gp2y10_dev.iled_pin = GP2Y10_ILED_PIN;
    gp2y10_dev.aout_pin = GP2Y10_AOUT_PIN;
    
    //cfg.intf.type = RT_SENSOR_INTF_ADC;
    cfg.intf.user_data = (void *)&gp2y10_dev;
    rt_hw_gp2y10_init("gp2", &cfg);

    return RT_EOK;
}
INIT_COMPONENT_EXPORT(rt_hw_gp2y10_port);