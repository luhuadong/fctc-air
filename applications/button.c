/*
 * Copyright (c) 2020, RudyLo <luhuadong@163.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-02-08     luhuadong    the first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>

#include "fctc_air.h"

#define USER_BTN_PIN    GET_PIN(C, 13)
//#define USER_BTN_PIN    GET_PIN(A, 0)

rt_bool_t is_paused = RT_FALSE;

static void key_cb(void *args)
{
	if (!is_paused) {
		rt_kprintf("(BUTTON) paused\n");
		is_paused = RT_TRUE;
		rt_pin_write(LED_PAUSE, PIN_HIGH);
		/* pase sync thread print or upload data to cloud */
	}
	else {
		rt_kprintf("(BUTTON) resume\n");
		is_paused = RT_FALSE;
		rt_pin_write(LED_PAUSE, PIN_LOW);
		/* resume */
	}
}

void user_btn_init()
{
	rt_pin_mode(USER_BTN_PIN, PIN_MODE_INPUT_PULLUP);
	/* Why can not use PIN_IRQ_MODE_FALLING ??? */
	rt_pin_attach_irq(USER_BTN_PIN, PIN_IRQ_MODE_RISING, key_cb, RT_NULL);
	rt_pin_irq_enable(USER_BTN_PIN, PIN_IRQ_ENABLE);
}
