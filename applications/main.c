/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2018-11-06     SummerGift   first version
 * 2020-01-20     RudyLo       fctc-air
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>

#include "dhtxx.h"
#include "gp2y10.h"
#include "sgp30.h"


#define LED1_PIN         GET_PIN(C, 7)   /* defined the LED1 pin: PC7 */
#define LED2_PIN         GET_PIN(B, 7)   /* defined the LED2 pin: PB7 */
#define LED3_PIN         GET_PIN(B, 14)  /* defined the LED3 pin: PB14 */
#define LED_RUN_PIN LED3_PIN

#define DHT22_DATA_PIN   GET_PIN(F, 15)  /* D2 */
#define DHT11_DATA_PIN   GET_PIN(E, 9)   /* D6 */

#define GP2Y10_ILED_PIN  GET_PIN(E, 13)  /* D3 */
#define GP2Y10_AOUT_PIN  GET_PIN(C, 3)   /* A2 */

#define SGP30_SCL_PIN    GET_PIN(B, 8)   /* D15 I2C_A_SCL */
#define SGP30_SDA_PIN    GET_PIN(B, 9)   /* D14 I2C_A_SDA */

#define LED_THREAD_PRIORITY      15
#define LED_THREAD_STACK_SIZE    512
#define LED_THREAD_TIMESLICE     15

#define DHT22_THREAD_PRIORITY    5
#define DHT22_THREAD_STACK_SIZE  1024
#define DHT22_THREAD_TIMESLICE   5

#define GP2Y10_THREAD_PRIORITY   10
#define GP2Y10_THREAD_STACK_SIZE 1024
#define GP2Y10_THREAD_TIMESLICE  5

#define SGP30_THREAD_PRIORITY    10
#define SGP30_THREAD_STACK_SIZE  1024
#define SGP30_THREAD_TIMESLICE   5


static void led_thread_entry(void *parameter)
{
    int count = 1;
        
    rt_pin_mode(LED1_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(LED2_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(LED3_PIN, PIN_MODE_OUTPUT);

    while (count++)
    {
        rt_pin_write(LED1_PIN, PIN_HIGH);
        rt_pin_write(LED2_PIN, PIN_LOW);
        rt_pin_write(LED3_PIN, PIN_LOW);
        rt_thread_mdelay(300);
        rt_pin_write(LED1_PIN, PIN_LOW);
        rt_pin_write(LED2_PIN, PIN_HIGH);
        rt_pin_write(LED3_PIN, PIN_LOW);
        rt_thread_mdelay(300);
        rt_pin_write(LED1_PIN, PIN_LOW);
        rt_pin_write(LED2_PIN, PIN_LOW);
        rt_pin_write(LED3_PIN, PIN_HIGH);
        rt_thread_mdelay(300);
    }
}

static void dht22_thread_entry(void *parameter)
{
    static struct dhtxx_device dht22;
    dhtxx_init(&dht22, SENSOR_DHT22, DHT22_DATA_PIN);

    static struct dhtxx_device dht11;
    dhtxx_init(&dht11, SENSOR_DHT11, DHT11_DATA_PIN);

    while(1)
    {
        rt_thread_mdelay(1000);

        if(dhtxx_read(&dht11)) {

            float t = dhtxx_get_temperature(&dht11);
            float h = dhtxx_get_humidity(&dht11);

            rt_kprintf("(DHT11) temperature: %d.%02d'C, humidity: %d.%02d%\n", 
                       (int)t, (int)(t*100) % 100, (int)h, (int)(h*100) % 100);
        }
        else {
            rt_kprintf("(DHT11) error\n");
        }

        if(dhtxx_read(&dht22)) {

            float t = dhtxx_get_temperature(&dht22);
            float h = dhtxx_get_humidity(&dht22);

            rt_kprintf("(DHT22) temperature: %d.%02d'C, humidity: %d.%02d%\n", 
                       (int)t, (int)(t*100) % 100, (int)h, (int)(h*100) % 100);
        }
        else {
            rt_kprintf("(DHT22) error\n");
        }
    }
}
FINSH_FUNCTION_EXPORT(dhtxx, read humidity and temperature)

static void gp2y10_thread_entry(void *parameter)
{
    struct gp2y10_device gp2y10;

    gp2y10_init(&gp2y10, GP2Y10_ILED_PIN, GP2Y10_AOUT_PIN);

    while(1)
    {
        rt_thread_mdelay(1000);
        float dust = gp2y10_get_dust_density(&gp2y10);
        rt_kprintf("(GP2Y10) Dust: %d.%02d ppm\n", (int)dust, (int)(dust*100)%100);
    }
}

static void sgp30_thread_entry(void *parameter)
{
    while(1)
    {
        rt_thread_mdelay(1000);
    }
}

static rt_thread_t led_thread = RT_NULL;

ALIGN(RT_ALIGN_SIZE)
static char dht22_thread_stack[DHT22_THREAD_STACK_SIZE];
static struct rt_thread dht22_thread;

ALIGN(RT_ALIGN_SIZE)
static char gp2y10_thread_stack[GP2Y10_THREAD_STACK_SIZE];
static struct rt_thread gp2y10_thread;

ALIGN(RT_ALIGN_SIZE)
static char sgp30_thread_stack[SGP30_THREAD_STACK_SIZE];
static struct rt_thread sgp30_thread;

int main(void)
{
    /* led thread */

    led_thread = rt_thread_create("led", led_thread_entry, RT_NULL, 
                                  LED_THREAD_STACK_SIZE, 
                                  LED_THREAD_PRIORITY, 
                                  LED_THREAD_TIMESLICE);

    if(led_thread != RT_NULL)
        rt_thread_startup(led_thread);

    /* dht22 thread */

    rt_thread_init(&dht22_thread, "dht22", dht22_thread_entry, RT_NULL, 
                   &dht22_thread_stack[0], sizeof(dht22_thread_stack), 
                   DHT22_THREAD_PRIORITY, DHT22_THREAD_TIMESLICE);

    rt_thread_startup(&dht22_thread);


    return RT_EOK;
}
