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

/* defined the LED1 pin: PC7 */
#define LED1_PIN    GET_PIN(C, 7)
/* defined the LED2 pin: PB7 */
#define LED2_PIN    GET_PIN(B, 7)
/* defined the LED3 pin: PB14 */
#define LED3_PIN    GET_PIN(B, 14)

#define LED_RUN_PIN LED1_PIN
#define LIGHT_PIN   LED2_PIN

static rt_thread_t bc28_thread = RT_NULL;

static void light_on(rt_base_t pin)
{
    rt_pin_write(pin, PIN_HIGH);
}

static void light_off(rt_base_t pin)
{
    rt_pin_write(pin, PIN_LOW);
}

static void mqtt_recv_cb(char *json)
{

}

static void bc28_thread_entry(void *parameter)
{
    if(RT_EOK != bc28_init())
    {
        rt_kprintf("(BC28) init failed\n");
        return;
    }
    rt_kprintf("(BC28) attach ok\n");

    while (RT_EOK != build_mqtt_network())
    {
        bc28_mqtt_close();
        rt_kprintf("(BC28) rebuild mqtt network\n");
    }
    rt_kprintf("(BC28) MQTT connect ok\n");

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
