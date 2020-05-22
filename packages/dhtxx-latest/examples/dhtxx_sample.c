/*
 * Copyright (c) 2020, RudyLo <luhuadong@163.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-01-20     luhuadong    the first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>

#include "dhtxx.h"

#define DHT11_DATA_PIN           GET_PIN(E, 9)
#define DHT22_DATA_PIN           GET_PIN(E, 13)

/* cat_dht11 by static */
static void cat_dht11(void)
{
    struct dht_device dht11;
    dht_init(&dht11, DHT11, DHT11_DATA_PIN);

    if(dht_read(&dht11)) {

        rt_int32_t temp = dht_get_temperature(&dht11);
        rt_int32_t humi = dht_get_humidity(&dht11);

        rt_kprintf("Temp: %d, Humi: %d\n", temp, humi);
    }
    else {
        rt_kprintf("DHT11 read error\n");
    }
}

/* cat_dht22 by dynamic */
static void cat_dht22(void)
{
    dht_device_t dht22 = dht_create(DHT22, DHT22_DATA_PIN);

    if(dht_read(dht22)) {

        rt_int32_t temp = dht_get_temperature(dht22);
        rt_int32_t humi = dht_get_humidity(dht22);

        rt_kprintf("Temp: %d, Humi: %d\n", temp, humi);
    }
    else {
        rt_kprintf("DHT22 read error\n");
    }

    dht_delete(dht22);
}

#ifdef FINSH_USING_MSH
MSH_CMD_EXPORT(cat_dht11, read dht11 humidity and temperature);
MSH_CMD_EXPORT(cat_dht22, read dht22 humidity and temperature);
#endif