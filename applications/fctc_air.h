/*
 * Copyright (c) 2020, RudyLo <luhuadong@163.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-02-07     RudyLo       the first version
 */

#ifndef __FCTC_AIR_H__
#define __FCTC_AIR_H__

#define LED1_PIN                 GET_PIN(C, 7)   /* defined the LED1 pin: PC7 */
#define LED2_PIN                 GET_PIN(B, 7)   /* defined the LED2 pin: PB7 */
#define LED3_PIN                 GET_PIN(B, 14)  /* defined the LED3 pin: PB14 */
#define LED_RUNNING              LED1_PIN
#define LED_PAUSE                LED2_PIN
#define LED_WARNING              LED3_PIN

extern rt_bool_t is_paused;

void user_btn_init(void);


#endif /* __FCTC_AIR_H__ */
