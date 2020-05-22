/*
 * Copyright (c) 2020, RudyLo <luhuadong@163.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-02-23     luhuadong    the first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "sgp30.h"

static void cat_sgp30_tvoc(void)
{
    rt_device_t tvoc_dev = RT_NULL;
    struct rt_sensor_data sensor_data;

    tvoc_dev = rt_device_find("tvoc_sg3");
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

    if (1 != rt_device_read(tvoc_dev, 0, &sensor_data, 1)) 
    {
        rt_kprintf("Read TVOC data failed.\n");
    }
    rt_kprintf("[%d] TVOC: %d\n", sensor_data.timestamp, sensor_data.data.tvoc);

    rt_device_close(tvoc_dev);
}

static void cat_sgp30_eco2(void)
{
    rt_device_t eco2_dev = RT_NULL;
    struct rt_sensor_data sensor_data;

    eco2_dev = rt_device_find("eco2_sg3");
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

    if (1 != rt_device_read(eco2_dev, 0, &sensor_data, 1)) 
    {
        rt_kprintf("Read eCO2 data failed.\n");
    }
    rt_kprintf("[%d] eCO2: %d\n", sensor_data.timestamp, sensor_data.data.eco2);

    rt_device_close(eco2_dev);
}

static void cat_sgp30_baseline(void)
{
    rt_device_t sgp30_dev = RT_NULL;
    struct sgp30_baseline baseline;

    sgp30_dev = rt_device_find("tvoc_sg3");
    if (!sgp30_dev) 
    {
        rt_kprintf("Can't find eCO2 device.\n");
        return;
    }

    if (rt_device_open(sgp30_dev, RT_DEVICE_FLAG_RDWR)) 
    {
        rt_kprintf("Open eCO2 device failed.\n");
        return;
    }

    if (RT_EOK != rt_device_control(sgp30_dev, RT_SENSOR_CTRL_GET_BASELINE, &baseline)) 
    {
        rt_kprintf("Get baseline failed.\n");
    }
    rt_kprintf("eCO2 baseline: %d, TVOC baseline: %d\n", baseline.eco2_base, baseline.tvoc_base);

    rt_device_close(sgp30_dev);
}
#ifdef FINSH_USING_MSH
MSH_CMD_EXPORT(cat_sgp30_tvoc, read sgp30 TVOC data);
MSH_CMD_EXPORT(cat_sgp30_eco2, read sgp30 eCO2 data);
MSH_CMD_EXPORT(cat_sgp30_baseline, read sgp30 TVOC and eCO2 baseline);
#endif
