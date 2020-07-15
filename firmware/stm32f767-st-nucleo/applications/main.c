/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-06     SummerGift   first version
 * 2019-1-10      e31207077    add stm32f767-st-nucleo bsp
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>


/* User Modified Part */
#define LED1_PIN                 GET_PIN(B, 0)   /* defined the LD1 (green) pin: PB0 <- PC7 */
#define LED2_PIN                 GET_PIN(B, 7)   /* defined the LD2 (blue)  pin: PB7 */
#define LED3_PIN                 GET_PIN(B, 14)  /* defined the LD3 (red)   pin: PB14 */

#define USER_BTN_PIN             GET_PIN(C, 13)  /* B1 USER */

#define DHT22_DATA_PIN           GET_PIN(E, 13)  /* D3 */
#define DHT11_DATA_PIN           GET_PIN(E, 9)   /* D6 */

#define GP2Y10_ILED_PIN          GET_PIN(F, 15)  /* D2 */
#define GP2Y10_AOUT_PIN          GET_PIN(C, 3)   /* A2 */

#define SGP30_I2C_BUS_NAME       "i2c1"          /* SCL: PB8(24), SDA: PB9(25) */
#define BC28_AT_CLIENT_NAME      "uart3"         /* No BC28 */

static void user_key_cb(void *args)
{
    rt_kprintf("(BUTTON) pressed\n");
}

static void user_key_init()
{
    rt_pin_mode(USER_BTN_PIN, PIN_MODE_INPUT_PULLUP);
    /* Why can not use PIN_IRQ_MODE_FALLING ??? */
    rt_pin_attach_irq(USER_BTN_PIN, PIN_IRQ_MODE_RISING, user_key_cb, RT_NULL);
    rt_pin_irq_enable(USER_BTN_PIN, PIN_IRQ_ENABLE);
}

int main(void)
{
    int count = 1;
    /* set LED1 pin mode to output */
    rt_pin_mode(LED1_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(LED2_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(LED3_PIN, PIN_MODE_OUTPUT);

    user_key_init();

    while (count++)
    {
        rt_pin_write(LED1_PIN, PIN_HIGH);
        rt_pin_write(LED2_PIN, PIN_HIGH);
        rt_pin_write(LED3_PIN, PIN_HIGH);
        rt_thread_mdelay(500);
        rt_pin_write(LED1_PIN, PIN_LOW);
        rt_pin_write(LED2_PIN, PIN_LOW);
        rt_pin_write(LED3_PIN, PIN_LOW);
        rt_thread_mdelay(500);
    }

    return RT_EOK;		
}
