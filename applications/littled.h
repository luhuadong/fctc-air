/*
 * Copyright (c) 2020, RudyLo <luhuadong@163.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-02-23     luhuadong    the first version
 */

#ifndef __PKG_LITTLED_H__
#define __PKG_LITTLED_H__

#ifndef PKG_USING_LITTLED_MAX
#define LITTLED_MAX         100
#else
#define LITTLED_MAX         PKG_USING_LITTLED_MAX
#endif

#define LED_ON(ld)          led_mode(ld, 0, LITTLED_MAX, 0, 0)
#define LED_OFF(ld)         led_mode(ld, 0, 0, 0, 0)
#define LED_BEEP(ld)        led_mode(ld, 1, LITTLED_MAX/2, 0, 3)
#define LED_BELL(ld)        led_mode(ld, 1, LITTLED_MAX/2, 50, 0)
#define LED_BLINK(ld)       led_mode(ld, 1, LITTLED_MAX/2, 0, 0)

int led_register(rt_base_t pin, rt_base_t active_logic);

//void led_mode(int ld, float freq, rt_uint32_t duty, rt_uint32_t time, rt_uint32_t count);
int led_mode(int ld, rt_uint32_t period, rt_uint32_t pulse, rt_uint32_t time, rt_uint32_t count);
void led_toggle(int ld);

#endif /* __PKG_LITTLED_H__ */
