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

/* cat_sgp30 */
static void cat_sgp30(void)
{
    sgp30_device_t sgp30 = sgp30_create(SGP30_I2C_BUS_NAME);

    if(!sgp30) 
    {
        rt_kprintf("(SGP30) Init failed\n");
        return;
    }

    rt_kprintf("(SGP30) Serial number: %x.%x.%x\n", sgp30->serialnumber[0], 
               sgp30->serialnumber[1], sgp30->serialnumber[2]);

    rt_uint16_t loop = 20;

    while(loop--)
    {
        /* Read TVOC and eCO2 */
        if(!sgp30_measure(sgp30)) 
        {
            rt_kprintf("(SGP30) Measurement failed\n");
            sgp30_delete(sgp30);
            break;
        }

        /* Read rawH2 and rawEthanol */
        if(!sgp30_measure_raw(sgp30)) 
        {
            rt_kprintf("(SGP30) Raw Measurement failed\n");
            sgp30_delete(sgp30);
            break;
        }

        rt_kprintf("[%2u] TVOC: %d ppb, eCO2: %d ppm; Raw H2: %d, Raw Ethanol: %d\n", 
                   loop, sgp30->TVOC, sgp30->eCO2, sgp30->rawH2, sgp30->rawEthanol);

        rt_thread_mdelay(1500);
    }
    
    sgp30_delete(sgp30);
}
#ifdef FINSH_USING_MSH
MSH_CMD_EXPORT(cat_sgp30, read sgp30 TVOC and eCO2);
#endif
