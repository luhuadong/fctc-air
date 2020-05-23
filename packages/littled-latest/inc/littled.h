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

#ifndef PKG_USING_LITTLED_PERIOD
#define DEFAULT_PERIOD           (1000)
#else
#define DEFAULT_PERIOD           PKG_USING_LITTLED_PERIOD
#endif

#ifndef PKG_USING_LITTLED_PULSE
#define DEFAULT_PULSE            (DEFAULT_PERIOD/2)
#else
#define DEFAULT_PULSE            PKG_USING_LITTLED_PULSE
#endif

#ifndef PKG_USING_LITTLED_BELL_TIME
#define BELL_TIME                (50000)
#else
#define BELL_TIME                PKG_USING_LITTLED_BELL_TIME
#endif

#ifndef PKG_USING_LITTLED_BEEP_COUNT
#define BEEP_COUNT               (3)
#else
#define BEEP_COUNT               PKG_USING_LITTLED_BEEP_COUNT
#endif

#define LED_ON(ld)               led_mode(ld, 1, 1, 0, 0)
#define LED_OFF(ld)              led_mode(ld, 1, 0, 0, 0)
#define LED_TOGGLE(ld)           led_mode(ld, 0, 0, 0, 1)

#define LED_BEEP(ld)             led_mode(ld, DEFAULT_PERIOD,   DEFAULT_PULSE,   0, BEEP_COUNT)
#define LED_BEEP_FAST(ld)        led_mode(ld, DEFAULT_PERIOD/2, DEFAULT_PULSE/2, 0, BEEP_COUNT)
#define LED_BEEP_SLOW(ld)        led_mode(ld, DEFAULT_PERIOD*2, DEFAULT_PULSE*2, 0, BEEP_COUNT)

#define LED_BELL(ld)             led_mode(ld, DEFAULT_PERIOD,   DEFAULT_PULSE,   BELL_TIME, 0)
#define LED_BELL_FAST(ld)        led_mode(ld, DEFAULT_PERIOD/2, DEFAULT_PULSE/2, BELL_TIME, 0)
#define LED_BELL_SLOW(ld)        led_mode(ld, DEFAULT_PERIOD*2, DEFAULT_PULSE*2, BELL_TIME, 0)

#define LED_BLINK(ld)            led_mode(ld, DEFAULT_PERIOD,   DEFAULT_PULSE,   0, 0)
#define LED_BLINK_FAST(ld)       led_mode(ld, DEFAULT_PERIOD/2, DEFAULT_PULSE/2, 0, 0)
#define LED_BLINK_SLOW(ld)       led_mode(ld, DEFAULT_PERIOD*2, DEFAULT_PULSE*2, 0, 0)

int  led_register(rt_base_t pin, rt_base_t active_logic);
void led_unregister(int ld);

int  led_mode(int ld, rt_uint32_t period, rt_uint32_t pulse, rt_uint32_t time, rt_uint32_t count);

#endif /* __PKG_LITTLED_H__ */
