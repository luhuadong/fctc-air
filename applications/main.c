/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
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

#include "dht.h"
#include "gp2y10.h"
#include "sgp30.h"


#define LED1_PIN                 GET_PIN(C, 7)   /* defined the LED1 pin: PC7 */
#define LED2_PIN                 GET_PIN(B, 7)   /* defined the LED2 pin: PB7 */
#define LED3_PIN                 GET_PIN(B, 14)  /* defined the LED3 pin: PB14 */
#define LED_RUN_PIN              LED3_PIN

#define DHT22_DATA_PIN           GET_PIN(E, 13)  /* D3 */
#define DHT11_DATA_PIN           GET_PIN(E, 9)   /* D6 */

#define GP2Y10_ILED_PIN          GET_PIN(F, 15)  /* D2 */
#define GP2Y10_AOUT_PIN          GET_PIN(C, 3)   /* A2 */

#define SGP30_SCL_PIN            GET_PIN(B, 8)   /* D15 I2C_A_SCL */
#define SGP30_SDA_PIN            GET_PIN(B, 9)   /* D14 I2C_A_SDA */

#define ADC_DEV_NAME             "adc1"      /* ADC device name */
#define ADC_DEV_CHANNEL          4           /* ADC channel */
#define ADC_CONVERT_BITS         12          /* 转换位数为12位 */
#define SGP30_I2C_BUS_NAME       "i2c1"

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
        rt_thread_mdelay(3000);
        rt_pin_write(LED1_PIN, PIN_LOW);
        rt_pin_write(LED2_PIN, PIN_HIGH);
        //rt_pin_write(LED3_PIN, PIN_LOW);
        rt_thread_mdelay(3000);
        //rt_pin_write(LED1_PIN, PIN_LOW);
        rt_pin_write(LED2_PIN, PIN_LOW);
        rt_pin_write(LED3_PIN, PIN_HIGH);
        rt_thread_mdelay(3000);
    }
}

static void dht22_thread_entry(void *parameter)
{
    struct dht_device dht22;
    dht_init(&dht22, SENSOR_DHT22, DHT22_DATA_PIN);

    while(1)
    {
        if(dht_read(&dht22)) {

            float t = dht_get_temperature(&dht22);
            float h = dht_get_humidity(&dht22);

            rt_kprintf("(DHT22) temperature: %d.%02d'C, humidity: %d.%02d%\n", 
                       (int)t, (int)(t*100) % 100, (int)h, (int)(h*100) % 100);
        }
        else {
            rt_kprintf("(DHT22) error\n");
        }

        rt_thread_mdelay(1000);
    }
}

static void gp2y10_thread_entry(void *parameter)
{
    struct gp2y10_device gp2y10;

    gp2y10_init(&gp2y10, GP2Y10_ILED_PIN, GP2Y10_AOUT_PIN);

    while(1)
    {
        float dust = gp2y10_get_dust_density(&gp2y10);
        rt_kprintf("(GP2Y10) Dust: %d.%02d ug/m3\n", (int)dust, (int)(dust*100)%100);

        rt_thread_mdelay(1000);
    }
}

static void sgp30_thread_entry(void *parameter)
{
    sgp30_device_t sgp30 = RT_NULL;
    int counter = 0;

    sgp30 = sgp30_init(SGP30_I2C_BUS_NAME);
    if(!sgp30) {
        sgp30_deinit(sgp30);
        rt_kprintf("(SGP30) Init failed\n");
        return;
    }

    while(1)
    {
        /* Read TVOC and eCO2 */
        if(!sgp30_measure(sgp30)) {
            rt_kprintf("(SGP30) Measurement failed\n");
            continue;
        }
        rt_kprintf("(SGP30) TVOC: %d ppb, eCO2: %d ppm\n", sgp30->TVOC, sgp30->eCO2);

        /* Read rawH2 and rawEthanol */
        if(!sgp30_measure_raw(sgp30)) {
            rt_kprintf("(SGP30) Raw Measurement failed\n");
            continue;
        }
        rt_kprintf("(SGP30) Raw H2: %d, Raw Ethanol: %d\n", sgp30->rawH2, sgp30->rawEthanol);

        rt_thread_mdelay(1000);

        counter++;
        if(counter == 30) {
            counter = 0;

            rt_uint16_t TVOC_base, eCO2_base;
            if(!sgp30_get_baseline(sgp30, &eCO2_base, &TVOC_base)) {
                rt_kprintf("(SGP30) Failed to get baseline readings\n");
                return;
            }
            rt_kprintf("(SGP30) ****Baseline values: eCO2: 0x%x & TVOC: 0x%x", eCO2_base, TVOC_base);
        }
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

    //rt_thread_startup(&dht22_thread);

    /* gp2y10 thread */

    rt_thread_init(&gp2y10_thread, "gp2y10", gp2y10_thread_entry, RT_NULL, 
                   &gp2y10_thread_stack[0], sizeof(gp2y10_thread_stack), 
                   GP2Y10_THREAD_PRIORITY, GP2Y10_THREAD_TIMESLICE);

    //rt_thread_startup(&gp2y10_thread);

    /* sgp30 thread */

    rt_thread_init(&sgp30_thread, "sgp30", sgp30_thread_entry, RT_NULL, 
                   &sgp30_thread_stack[0], sizeof(sgp30_thread_stack), 
                   SGP30_THREAD_PRIORITY, SGP30_THREAD_TIMESLICE);

    //rt_thread_startup(&gp2y10_thread);


    return RT_EOK;
}


/*
 * EXPORT FUNCTIONS
 */

/* cat_dht11 */
static void cat_dht11(void)
{
    struct dht_device dht11;
    dht_init(&dht11, SENSOR_DHT11, DHT11_DATA_PIN);

    if(dht_read(&dht11)) {

        float t = dht_get_temperature(&dht11);
        float h = dht_get_humidity(&dht11);

        rt_kprintf("(DHT11) temperature: %d.%02d'C, humidity: %d.%02d%\n", 
                    (int)t, (int)(t*100) % 100, (int)h, (int)(h*100) % 100);
    }
    else {
        rt_kprintf("(DHT11) error\n");
    }
}
#ifdef FINSH_USING_MSH
MSH_CMD_EXPORT(cat_dht11, read dht11 humidity and temperature);
#endif

/* cat_dht22 */
static void cat_dht22(void)
{
    struct dht_device dht22;
    dht_init(&dht22, SENSOR_DHT22, DHT22_DATA_PIN);

    if(dht_read(&dht22)) {

        float t = dht_get_temperature(&dht22);
        float h = dht_get_humidity(&dht22);

        rt_kprintf("(DHT22) temperature: %d.%02d'C, humidity: %d.%02d%\n", 
                    (int)t, (int)(t*100) % 100, (int)h, (int)(h*100) % 100);
    }
    else {
        rt_kprintf("(DHT22) error\n");
    }
}
#ifdef FINSH_USING_MSH
MSH_CMD_EXPORT(cat_dht22, read dht22 humidity and temperature);
#endif

/* cat_gp2y10 */
static void cat_gp2y10(void)
{
    struct gp2y10_device gp2y10;

    gp2y10_init(&gp2y10, GP2Y10_ILED_PIN, GP2Y10_AOUT_PIN);
    float dust = gp2y10_get_dust_density(&gp2y10);
    rt_kprintf("(GP2Y10) Dust: %d.%02d ug/m3\n", (int)dust, (int)(dust*100)%100);
}
#ifdef FINSH_USING_MSH
MSH_CMD_EXPORT(cat_gp2y10, read gp2y10 dust density);
#endif

/* cat_sgp30 */
void cat_sgp30(void)
{
    rt_kprintf("hello SGP30\n");

    sgp30_device_t sgp30 = RT_NULL;

    sgp30 = sgp30_init(SGP30_I2C_BUS_NAME);
    if(!sgp30) {
        rt_kprintf("(SGP30) Init failed\n");
        sgp30_deinit(sgp30);
        return;
    }

    rt_kprintf("(SGP30) Serial number: %x.%x.%x\n", sgp30->serialnumber[0], 
               sgp30->serialnumber[1], sgp30->serialnumber[2]);

    rt_uint16_t loop = 20;

    while(loop--)
    {
        /* Read TVOC and eCO2 */
        if(!sgp30_measure(sgp30)) {
            rt_kprintf("(SGP30) Measurement failed\n");
            sgp30_deinit(sgp30);
            break;
        }

        /* Read rawH2 and rawEthanol */
        if(!sgp30_measure_raw(sgp30)) {
            rt_kprintf("(SGP30) Raw Measurement failed\n");
            sgp30_deinit(sgp30);
            break;
        }

        rt_kprintf("[%2u] TVOC: %d ppb, eCO2: %d ppm; Raw H2: %d, Raw Ethanol: %d\n", 
                   loop, sgp30->TVOC, sgp30->eCO2, sgp30->rawH2, sgp30->rawEthanol);

        rt_thread_mdelay(1500);
    }
    
    sgp30_deinit(sgp30);
}
#ifdef FINSH_USING_MSH
MSH_CMD_EXPORT(cat_sgp30, read sgp30 TVOC and eCO2);
#endif

/* cat_hello */
void cat_hello(void)
{
    rt_kprintf("hello RT-Thread\n");
}
#ifdef FINSH_USING_MSH
MSH_CMD_EXPORT(cat_hello, say hello to RT-Thread);
#endif
