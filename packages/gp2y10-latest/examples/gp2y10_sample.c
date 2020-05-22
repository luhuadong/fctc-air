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

/* cat_gp2y10 by static */
static void cat_gp2y10_static(void)
{
    struct gp2y10_device gp2y10;

    if (RT_EOK == gp2y10_init(&gp2y10, GP2Y10_ILED_PIN, GP2Y10_AOUT_PIN))
    {
        rt_uint32_t dust = gp2y10_get_dust_density(&gp2y10);
        rt_kprintf("Dust: %d ug/m3\n", dust);
    }
    else {
        rt_kprintf("GP2Y10 read error\n");
    }
}

/* cat_gp2y10 by dynamic */
static void cat_gp2y10_dynamic(void)
{
    gp2y10_device_t gp2y10 = gp2y10_create(GP2Y10_ILED_PIN, GP2Y10_AOUT_PIN);

    if (gp2y10) {

        rt_uint32_t dust = gp2y10_get_dust_density(gp2y10);
        rt_kprintf("Dust: %d ug/m3\n", dust);
    }
    else {
        rt_kprintf("GP2Y10 read error\n");
    }

    gp2y10_delete(gp2y10);
}

#ifdef FINSH_USING_MSH
MSH_CMD_EXPORT(cat_gp2y10_static, read gp2y10 dust density);
MSH_CMD_EXPORT(cat_gp2y10_dynamic, read gp2y10 dust density);
#endif