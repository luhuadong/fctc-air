/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-11-06     luhuadong   first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>
#include <arpa/inet.h>
#include <netdev.h>

#include <littled.h>
#include <dhtxx.h>
#include <gp2y10.h>
#include <sgp30.h>
#include <ccs811.h>
#include "ali_mqtt.h"

#define DBG_TAG                  "main"
#define DBG_LVL                  DBG_LOG
#include <rtdbg.h>

#define JSON_DATA_PACK_STR       "{\"id\":\"125\",\"version\":\"1.0\",\"params\":{\"Temp\":%s,\"Humi\":%s,\"Dust\":%d,\"TVOC\":%d,\"eCO2\":%d},\"method\":\"thing.event.property.post\"}"

/* User Modified Part */
#define LED1_PIN                 GET_PIN(B, 0)   /* defined the LD1 (green) pin: PB0 <- PC7 */
#define LED2_PIN                 GET_PIN(B, 7)   /* defined the LD2 (blue)  pin: PB7 */
#define LED3_PIN                 GET_PIN(B, 14)  /* defined the LD3 (red)   pin: PB14 */

#define USER_BTN_PIN             GET_PIN(C, 13)  /* B1 USER */

#define DHT22_DATA_PIN           GET_PIN(E, 13)  /* D3 */
#define DHT11_DATA_PIN           GET_PIN(E, 9)   /* D6 */

#define GP2Y10_ILED_PIN          GET_PIN(F, 15)  /* D2 */
#define GP2Y10_AOUT_PIN          GET_PIN(C, 3)   /* A2 : ADC1_IN13 */

#define SGP30_I2C_BUS_NAME       "i2c1"          /* SCL: PB8(24), SDA: PB9(25) */
#define CCS811_I2C_BUS_NAME      "i2c1"
#define BC28_AT_CLIENT_NAME      "uart3"         /* No BC28 */
#define NET_DEVICE_NAME          "e0"


#define DELAY_TIME_DEFAULT       5000

#define SENSOR_TEMP              (0)
#define SENSOR_HUMI              (1)
#define SENSOR_DUST              (2)
#define SENSOR_TVOC              (3)
#define SENSOR_ECO2              (4)

#define EVENT_FLAG_TEMP          (1 << SENSOR_TEMP)
#define EVENT_FLAG_HUMI          (1 << SENSOR_HUMI)
#define EVENT_FLAG_DUST          (1 << SENSOR_DUST)
#define EVENT_FLAG_TVOC          (1 << SENSOR_TVOC)
#define EVENT_FLAG_ECO2          (1 << SENSOR_ECO2)
//#define EVENT_FLAG_UPLOAD        (1 << 28)
//#define EVENT_FLAG_PAUSE         (1 << 30)

/* event */
static struct rt_event event;

/* message queue */
static rt_mq_t      sync_mq    = RT_NULL;

/* mailbox */
static rt_mailbox_t upload_mb  = RT_NULL;

/* memory pool */
//static char json_data[512];

/* semaphore */
//static rt_sem_t key_sem = RT_NULL;

/* LED indicator */
static int led_normal;
static int led_upload;
static int led_warning;

static rt_thread_t temp_thread = RT_NULL;
static rt_thread_t humi_thread = RT_NULL;
static rt_thread_t dust_thread = RT_NULL;
static rt_thread_t tvoc_thread = RT_NULL;
static rt_thread_t eco2_thread = RT_NULL;

static rt_thread_t sync_thread = RT_NULL;
static rt_thread_t upload_thread = RT_NULL;

struct sensor_msg
{
    rt_uint8_t tag;
    rt_int32_t data;
};

static rt_bool_t is_paused = RT_FALSE;

static void user_key_cb(void *args)
{
    if (!is_paused) {
        rt_kprintf("(BUTTON) paused\n");
        is_paused = RT_TRUE;
        LED_ON(led_upload);

        //rt_event_send(&event, EVENT_FLAG_PAUSE);
    }
    else {
        rt_kprintf("(BUTTON) resume\n");
        is_paused = RT_FALSE;
        LED_OFF(led_upload);

        //rt_event_recv(&event, EVENT_FLAG_PAUSE, RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR, 0, NULL);
    }
}

static void user_key_init()
{
    rt_pin_mode(USER_BTN_PIN, PIN_MODE_INPUT_PULLUP);
    /* Why can not use PIN_IRQ_MODE_FALLING ??? */
    rt_pin_attach_irq(USER_BTN_PIN, PIN_IRQ_MODE_RISING, user_key_cb, RT_NULL);
    rt_pin_irq_enable(USER_BTN_PIN, PIN_IRQ_ENABLE);
}

static char *int10_to_str(const rt_int32_t num, char *str)
{
    RT_ASSERT(str);

    int anum = num < 0 ? -num : num;
    int integer = anum / 10;
    int decimal = anum % 10;

    rt_sprintf(str, "%s%d.%d", num < 0 ? "-" : "", integer, decimal);
    return str;
}

/*
 * param data using float because the sensor data include int and float
*/
static void sync(const rt_uint8_t tag, const rt_int32_t data)
{
    struct sensor_msg msg;

    msg.tag  = tag;
    msg.data = data;
  
    rt_mq_send(sync_mq, (void *)&msg, sizeof(msg));  /* send sensor data */
    rt_event_send(&event, (1 << tag));               /* send sensor event */
}

static void upload_thread_entry(void *parameter)
{
    void *pclient = NULL;
    //int   res = 0;
    struct netdev *dev;

    dev = netdev_get_by_name(NET_DEVICE_NAME);
    if (dev == RT_NULL)
    {
        LOG_E("(upload) Can't find %s device.\n", NET_DEVICE_NAME);
        return;
    }

    while (!netdev_is_internet_up(dev))
    {
        rt_thread_mdelay(1000);
    }
    LOG_I("(upload) %s is connected to internet.\n", NET_DEVICE_NAME);

    pclient = ali_mqtt_create();
    if (pclient == RT_NULL)
    {
        LED_BLINK_FAST(led_warning);
        LOG_E("(upload) init mqtt network failed.\n");
        return;
    }
    LOG_I("(upload) init mqtt network ok.\n");

#if 0
    res = example_subscribe(pclient);
    if (res < 0) 
    {
        LED_BLINK_FAST(led_warning);
        rt_kprintf("(upload) mqtt topic subscribe failed.\n");
        IOT_MQTT_Destroy(&pclient);
        return;
    }
#endif
    LED_OFF(led_warning);
    LED_BLINK(led_normal);

    char *buf;

    while (1)
    {
        if (RT_EOK == rt_mb_recv(upload_mb, (rt_ubase_t *)&buf, RT_WAITING_FOREVER))
        {
            LED_BEEP_FAST(led_upload);

            ali_mqtt_publish(pclient, buf);
        }
        IOT_MQTT_Yield(pclient, 200);
    }
}

static void sync_thread_entry(void *parameter)
{
    struct sensor_msg msg;
    rt_int32_t air[5] = {0};
    char temp_str[8] = {0};
    char humi_str[8] = {0};
    char buf[512] = {0};

    int count = 0;
    rt_uint32_t recved;
    rt_uint32_t sensor_event = EVENT_FLAG_TEMP | EVENT_FLAG_HUMI | /* EVENT_FLAG_DUST | */
                               EVENT_FLAG_TVOC | EVENT_FLAG_ECO2;

    /* clear sensor event */
    rt_event_recv(&event, sensor_event, RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, 0, &recved);

    while(1)
    {
        if (RT_EOK == rt_mq_recv(sync_mq, (void *)&msg, sizeof(msg), RT_WAITING_FOREVER))
        {
            air[msg.tag] = msg.data;
        }

        if (RT_EOK == rt_event_recv(&event, sensor_event, RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR, 0, &recved))
        {
            /*
            if (RT_EOK == rt_event_recv(&event, EVENT_FLAG_PAUSE, RT_EVENT_FLAG_AND, 0, &recved))
            {
                continue;
            }
            */

            if (is_paused)
                continue;

            int10_to_str(air[0], temp_str);
            int10_to_str(air[1], humi_str);

            rt_kprintf("[%03d] Temp: %s C, Humi: %s%, Dust:%4d ug/m3, TVOC:%4d ppb, eCO2:%4d ppm\n", 
                        ++count, temp_str, humi_str, air[2], air[3], air[4]);

            if (count%10 == 0)
            {
                rt_sprintf(buf, JSON_DATA_PACK_STR, temp_str, humi_str, air[2], air[3], air[4]);
                rt_mb_send(upload_mb, (rt_ubase_t)buf);
            }
        }
    }
}

static void read_sensor_entry(void *parameter)
{
    rt_device_t sensor = RT_NULL;
    struct rt_sensor_data sensor_data;

    sensor = rt_device_find(parameter);
    if (!sensor) 
    {
        LOG_E("Can not find %s device.", parameter);
        return;
    }

    if (rt_device_open(sensor, RT_DEVICE_FLAG_RDWR)) 
    {
        LOG_E("Open %s device failed.", parameter);
        return;
    }

    rt_thread_mdelay(2000);  /* 越过2s不稳定期 */

    while(1)
    {
        if (1 != rt_device_read(sensor, 0, &sensor_data, 1)) 
        {
            LOG_E("(Temp) Read %s data failed.", parameter);
            rt_thread_mdelay(DELAY_TIME_DEFAULT);
        }

        if (sensor_data.type == RT_SENSOR_CLASS_TEMP)
        {
            LOG_D("[%d] Temp: %d'C", sensor_data.timestamp, sensor_data.data.temp/10);
            sync(SENSOR_TEMP, sensor_data.data.temp);
        }
        else if (sensor_data.type == RT_SENSOR_CLASS_HUMI)
        {
            LOG_D("[%d] Humi: %d%%", sensor_data.timestamp, sensor_data.data.humi/10);
            sync(SENSOR_HUMI, sensor_data.data.humi);
        }
        else if (sensor_data.type == RT_SENSOR_CLASS_DUST)
        {
            LOG_D("[%d] Dust: %d", sensor_data.timestamp, sensor_data.data.dust);
            sync(SENSOR_DUST, sensor_data.data.dust);
        }
        else if (sensor_data.type == RT_SENSOR_CLASS_TVOC)
        {
            LOG_D("[%d] TVOC: %d ppb", sensor_data.timestamp, sensor_data.data.tvoc);
            sync(SENSOR_TVOC, sensor_data.data.tvoc);
        }
        else if (sensor_data.type == RT_SENSOR_CLASS_ECO2)
        {
            LOG_D("[%d] eCO2: %d ppm", sensor_data.timestamp, sensor_data.data.eco2);
            sync(SENSOR_ECO2, sensor_data.data.eco2);
        }

        rt_thread_mdelay(DELAY_TIME_DEFAULT);
    }

    rt_device_close(sensor);
}

static void read_dust_entry(void *parameter)
{
    rt_device_t dust_dev = RT_NULL;
    struct rt_sensor_data sensor_data;

    dust_dev = rt_device_find(parameter);
    if (dust_dev == RT_NULL)
    {
        LOG_E("(Dust) Can't find %s device.\n", parameter);
        return;
    }

    if (rt_device_open(dust_dev, RT_DEVICE_FLAG_RDWR)) 
    {
        LOG_E("(Dust) Open %s device failed.\n", parameter);
        return;
    }

    while(1)
    {
        if (1 != rt_device_read(dust_dev, 0, &sensor_data, 1))
        {
            LOG_E("(Dust) Read %s data failed.\n", parameter);
        } else
        {
            LOG_D("[%d] Dust: %d\n", sensor_data.timestamp, sensor_data.data.dust);
            sync(SENSOR_DUST, sensor_data.data.dust);
        }
        rt_thread_mdelay(DELAY_TIME_DEFAULT);
    }
    rt_device_close(dust_dev);
}

static void read_tvoc_entry(void *parameter)
{
    rt_device_t tvoc_dev = RT_NULL;
    struct rt_sensor_data sensor_data;
    int count = 0;

    tvoc_dev = rt_device_find(parameter);
    if (!tvoc_dev) 
    {
        LOG_E("(TVOC) Can't find %s device.\n", parameter);
        return;
    }

    if (rt_device_open(tvoc_dev, RT_DEVICE_FLAG_RDWR)) 
    {
        LOG_E("(TVOC) Open %s device failed.\n", parameter);
        return;
    }

    while (1)
    {
        if (1 != rt_device_read(tvoc_dev, 0, &sensor_data, 1)) 
        {
            LOG_E("(TVOC) Read %s data failed.\n", parameter);
        } else
        {
            LOG_D("[%d] TVOC: %d\n", sensor_data.timestamp, sensor_data.data.tvoc);
            sync(SENSOR_TVOC, sensor_data.data.tvoc);
        }

#if 1
        count++;
        if (count == 15)
        {
            struct sgp30_baseline baseline;
            rt_device_control(tvoc_dev, RT_SENSOR_CTRL_GET_BASELINE, &baseline);
            rt_kprintf("baseline: tvoc = %d, eco2 = %d\n", baseline.tvoc_base, baseline.eco2_base);
            count = 0;
        }
        
#endif

        rt_thread_mdelay(1000);
    }
    rt_device_close(tvoc_dev);
}

static void read_eco2_entry(void *parameter)
{
    rt_device_t eco2_dev = RT_NULL;
    struct rt_sensor_data sensor_data;

    eco2_dev = rt_device_find(parameter);
    if (!eco2_dev) 
    {
        LOG_E("(eCO2) Can't find %s device.\n", parameter);
        return;
    }

    if (rt_device_open(eco2_dev, RT_DEVICE_FLAG_RDWR)) 
    {
        LOG_E("(eCO2) Open %s device failed.\n", parameter);
        return;
    }

    while(1)
    {
        if (1 != rt_device_read(eco2_dev, 0, &sensor_data, 1)) 
        {
            LOG_E("(eCO2) Read %s data failed.\n", parameter);
        } else
        {
            LOG_D("[%d] eCO2: %d\n", sensor_data.timestamp, sensor_data.data.eco2);
            sync(SENSOR_ECO2, sensor_data.data.eco2);
        }
        rt_thread_mdelay(1000);
    }
    rt_device_close(eco2_dev);
}

int main(void)
{
    rt_kprintf("  ___ ___ _____ ___     _   _     \n");
    rt_kprintf(" | __/ __|_   _/ __|   /_\\ (_)_ _ \n");
    rt_kprintf(" | _| (__  | || (__   / _ \\| | '_|\n");
    rt_kprintf(" |_| \\___| |_| \\___| /_/ \\_\\_|_| \n\n");

    rt_err_t result;

    /* initialization */

    user_key_init();

    led_normal = led_register(LED1_PIN, PIN_HIGH);
    led_upload = led_register(LED2_PIN, PIN_HIGH);
    led_warning = led_register(LED3_PIN, PIN_HIGH);

    LED_ON(led_normal);
    LED_OFF(led_upload);
    LED_OFF(led_warning);

    /* create message queue */
    sync_mq = rt_mq_create("sync_mq", sizeof(struct sensor_msg), 10, RT_IPC_FLAG_FIFO);
    if (sync_mq == RT_NULL)
    {
        rt_kprintf("create message queue failed.\n");
        return -1;
    }

    /* create mailbox */
    upload_mb = rt_mb_create("upload_mb", 1, RT_IPC_FLAG_FIFO);
    if (upload_mb == RT_NULL)
    {
        rt_kprintf("create mailbox failed.\n");
        return -1;
    }

    /* create event */
    result = rt_event_init(&event, "event", RT_IPC_FLAG_FIFO);
    if (result != RT_EOK)
    {
        rt_kprintf("init event failed.\n");
        return -1;
    }

    temp_thread = rt_thread_create("temp_th", read_sensor_entry, "temp_dht", 1024, 10, 5);
    humi_thread = rt_thread_create("humi_th", read_sensor_entry, "humi_dht", 1024, 10, 5);
    dust_thread = rt_thread_create("dust_th", read_sensor_entry, "dust_gp2", 1024, 15, 5);
    tvoc_thread = rt_thread_create("tvoc_th", read_sensor_entry, "tvoc_cs8", 1024, 16, 5);
    eco2_thread = rt_thread_create("eco2_th", read_sensor_entry, "eco2_cs8", 1024, 16, 5);

    sync_thread = rt_thread_create("sync", sync_thread_entry, RT_NULL, 1024, 15, 5);
    upload_thread = rt_thread_create("upload", upload_thread_entry, RT_NULL, 4096, 5, 5);

    /* start up all user thread */
    if(temp_thread) rt_thread_startup(temp_thread);
    if(humi_thread) rt_thread_startup(humi_thread);
    //if(dust_thread) rt_thread_startup(dust_thread);
    if(tvoc_thread) rt_thread_startup(tvoc_thread);
    if(eco2_thread) rt_thread_startup(eco2_thread);

    if(sync_thread) rt_thread_startup(sync_thread);
    //if(upload_thread) rt_thread_startup(upload_thread);

    return RT_EOK;
}

static int rt_hw_dht_port(void)
{
    struct rt_sensor_config cfg;
    
    cfg.intf.type = RT_SENSOR_INTF_ONEWIRE;
    cfg.intf.user_data = (void *)DHT11_DATA_PIN;
    rt_hw_dht_init("dht", &cfg);
    
    return RT_EOK;
}
INIT_COMPONENT_EXPORT(rt_hw_dht_port);

static int rt_hw_gp2y10_port(void)
{
    struct gp2y10_device gp2y10_dev;
    struct rt_sensor_config cfg;

    gp2y10_dev.iled_pin = GP2Y10_ILED_PIN;
    gp2y10_dev.aout_pin = GP2Y10_AOUT_PIN;
    
    //cfg.intf.type = RT_SENSOR_INTF_ADC;
    cfg.intf.user_data = (void *)&gp2y10_dev;
    rt_hw_gp2y10_init("gp2", &cfg);

    return RT_EOK;
}
//INIT_COMPONENT_EXPORT(rt_hw_gp2y10_port);

#if 0
static int rt_hw_sgp30_port(void)
{
    struct rt_sensor_config cfg;
    
    cfg.intf.type = RT_SENSOR_INTF_I2C;
    cfg.intf.dev_name = SGP30_I2C_BUS_NAME;
    rt_hw_sgp30_init("sg3", &cfg);
    
    return RT_EOK;
}
INIT_COMPONENT_EXPORT(rt_hw_sgp30_port);
#else
static int rt_hw_ccs811_port(void)
{
    struct rt_sensor_config cfg;
    
    cfg.intf.type = RT_SENSOR_INTF_I2C;
    cfg.intf.dev_name = CCS811_I2C_BUS_NAME;
    rt_hw_ccs811_init("cs8", &cfg);
    
    return RT_EOK;
}
INIT_COMPONENT_EXPORT(rt_hw_ccs811_port);
#endif
