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


int led_register(rt_base_t pin, rt_base_t active_logic);


/**
 * This function will read a bit from sensor.
 *
 * @param ld  led descriptor
 *
 * @return the bit value
 */
int led_mode(ld, freq, duty, time, count);


#endif /* __PKG_LITTLED_H__ */
