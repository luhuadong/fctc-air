/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-06     SummerGift   first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>

#include <bc28_mqtt.h>
#include <cJSON.h>

#define LOG_TAG                   "main"
#define LOG_LVL                   LOG_LVL_DBG
#include <ulog.h>

/* defined the LED1 pin: PC7 */
#define LED1_PIN    GET_PIN(C, 7)
/* defined the LED2 pin: PB7 */
#define LED2_PIN    GET_PIN(B, 7)
/* defined the LED3 pin: PB14 */
#define LED3_PIN    GET_PIN(B, 14)
/* D3 */
#define LIGHT_PIN   GET_PIN(E, 13)

#define LED_RUN_PIN LED1_PIN

static rt_thread_t bc28_thread = RT_NULL;

//void light_on(rt_base_t pin)
void light_on(void)
{
    rt_pin_write(LIGHT_PIN, PIN_HIGH);
}

void light_off(void)
{
    rt_pin_write(LIGHT_PIN, PIN_LOW);
}

static void mqtt_recv_cb(const char *json)
{
    cJSON *obj = cJSON_Parse(json);
    char  *str = cJSON_Print(obj);
    rt_kprintf("%s\n", str);
    rt_free(str);

    cJSON *powerstate = cJSON_GetObjectItem(cJSON_GetObjectItem(obj, "params"), "powerstate");
    LOG_D("power state: %d", powerstate->valueint);
    if (powerstate->valueint == 1)
    {
        light_on();
        LOG_D("switch on");
    }
    else if (powerstate->valueint == 0)
    {
        light_off();
        LOG_D("switch off");
    }
}

static void bc28_thread_entry(void *parameter)
{
    if (bc28_init() < 0)
    {
        rt_kprintf("(BC28) init failed\n");
        return;
    }

    if (bc28_client_attach() < 0)
    {
        rt_kprintf("(BC28) attach failed\n");
        return;
    }

    rt_kprintf("(BC28) attach ok\n");

    while (bc28_build_mqtt_network() < 0)
    {
        bc28_mqtt_close();
        rt_kprintf("(BC28) rebuild mqtt network\n");
    }
    rt_kprintf("(BC28) MQTT connect ok\n");

    bc28_bind_parser(mqtt_recv_cb);

    while (1)
    {
        rt_thread_mdelay(1000);
    }
}

int main(void)
{
    int count = 1;

    rt_pin_mode(LED_RUN_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(LIGHT_PIN, PIN_MODE_OUTPUT);

    bc28_thread = rt_thread_create("bc28", bc28_thread_entry, RT_NULL, 2048, 5, 5);
    if(bc28_thread) rt_thread_startup(bc28_thread);

    while (count++)
    {
        rt_pin_write(LED_RUN_PIN, PIN_HIGH);
        rt_thread_mdelay(500);
        rt_pin_write(LED_RUN_PIN, PIN_LOW);
        rt_thread_mdelay(500);
    }

    return RT_EOK;
}
