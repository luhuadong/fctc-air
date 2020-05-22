/*
 * Copyright (c) 2020, RudyLo <luhuadong@163.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-02-25     luhuadong    the first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include "littled.h"

#define LED1_PIN        GET_PIN(C, 7)

static int littled_sample(void)
{
	int led_test;

	led_test = led_register(LED1_PIN, PIN_HIGH);

	/* Call it anywhere like that */

    LED_ON(led_test);

    rt_thread_mdelay(1000);

    LED_OFF(led_test);

    LED_BEEP(led_test);

    rt_thread_mdelay(5000);

    LED_BLINK(led_test);

    led_unregister(led_test);
}
#ifdef FINSH_USING_MSH
MSH_CMD_EXPORT(littled_sample, Driver LED based on littled);
#endif