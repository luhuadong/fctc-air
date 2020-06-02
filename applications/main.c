/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-01-20     luhuadong    first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>

#include <littled.h>
#include <dhtxx.h>
#include <gp2y10.h>
#include <sgp30.h>
#ifdef PKG_USING_BC28_MQTT
#include <bc28_mqtt.h>
#endif

#define JSON_DATA_PACK_STR       "{\"id\":\"125\",\"version\":\"1.0\",\"params\":{\"Temp\":%s,\"Humi\":%s,\"Dust\":%d,\"TVOC\":%d,\"eCO2\":%d},\"method\":\"thing.event.property.post\"}\x1A"

#define MQTT_TOPIC_HELLO         "/a1p8Pngb3oY/BC28/user/hello"
#define MQTT_TOPIC_UPLOAD        "/sys/a1p8Pngb3oY/BC28/thing/event/property/post"

#define LED1_PIN                 GET_PIN(C, 7)   /* defined the LED1 pin: PC7 */
#define LED2_PIN                 GET_PIN(B, 7)   /* defined the LED2 pin: PB7 */
#define LED3_PIN                 GET_PIN(B, 14)  /* defined the LED3 pin: PB14 */

#define USER_BTN_PIN             GET_PIN(C, 13)  /* B1 USER */

#define DHT22_DATA_PIN           GET_PIN(E, 13)  /* D3 */
#define DHT11_DATA_PIN           GET_PIN(E, 9)   /* D6 */

#define GP2Y10_ILED_PIN          GET_PIN(F, 15)  /* D2 */
#define GP2Y10_AOUT_PIN          GET_PIN(C, 3)   /* A2 */

#define SGP30_I2C_BUS_NAME       "i2c1"

#define DELAY_TIME_DEFAULT       3000

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

void user_key_init()
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

static void sync_thread_entry(void *parameter)
{
    struct sensor_msg msg;
    rt_int32_t air[5] = {0};
    char temp_str[8] = {0};
    char humi_str[8] = {0};
    char buf[512] = {0};

    int count = 0;
    rt_uint32_t recved;
    rt_uint32_t sensor_event = EVENT_FLAG_TEMP | EVENT_FLAG_HUMI | EVENT_FLAG_DUST | 
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

static void upload_thread_entry(void *parameter)
{
#ifdef PKG_USING_BC28_MQTT
    if(RT_EOK != bc28_init())
    {
        rt_kprintf("(BC28) init failed\n");
        LED_BLINK_FAST(led_warning);
        return;
    }
    rt_kprintf("(BC28) attach ok\n");

    while (RT_EOK != build_mqtt_network())
    {
        LED_BLINK_FAST(led_warning);
        bc28_mqtt_close();
        rt_kprintf("(BC28) rebuild mqtt network\n");
    }
    rt_kprintf("(BC28) MQTT connect ok\n");
#else
    /*  */
#endif

    LED_OFF(led_warning);
    LED_BLINK(led_normal);

    char *buf;

    while (1)
    {
        if (RT_EOK == rt_mb_recv(upload_mb, (rt_ubase_t *)&buf, RT_WAITING_FOREVER))
        {
            LED_BEEP_FAST(led_upload);
#ifdef PKG_USING_BC28_MQTT
            bc28_mqtt_publish(MQTT_TOPIC_UPLOAD, buf);
#else
    /*  */
#endif
        }
    }
}

static void read_temp_entry(void *parameter)
{
    rt_device_t temp_dev = RT_NULL;
    struct rt_sensor_data sensor_data;

    temp_dev = rt_device_find(parameter);
    if (!temp_dev) 
    {
        rt_kprintf("(Temp) Can't find %s device.\n", parameter);
        return;
    }

    if (rt_device_open(temp_dev, RT_DEVICE_FLAG_RDWR)) 
    {
        rt_kprintf("(Temp) Open %s device failed.\n", parameter);
        return;
    }

    rt_thread_mdelay(2000);  /* 越过2s不稳定期 */

    while(1)
    {
        if (1 != rt_device_read(temp_dev, 0, &sensor_data, 1)) 
        {
            //LED_BEEP(led_warning);
            rt_kprintf("(Temp) Read %s data failed.\n", parameter);
        } else 
        {
            //rt_kprintf("[%d] Temp: %d\n", sensor_data.timestamp, sensor_data.data.temp);
            sync(SENSOR_TEMP, sensor_data.data.temp);
        }
        rt_thread_mdelay(DELAY_TIME_DEFAULT);
    }
    rt_device_close(temp_dev);
}

static void read_humi_entry(void *parameter)
{
    rt_device_t humi_dev = RT_NULL;
    struct rt_sensor_data sensor_data;

    humi_dev = rt_device_find(parameter);
    if (!humi_dev) 
    {
        rt_kprintf("(Humi) Can't find %s device.\n", parameter);
        return;
    }

    if (rt_device_open(humi_dev, RT_DEVICE_FLAG_RDWR)) 
    {
        rt_kprintf("(Humi) Open %s device failed.\n", parameter);
        return;
    }

    rt_thread_mdelay(2000);  /* 越过2s不稳定期 */

    while (1)
    {
        if (1 != rt_device_read(humi_dev, 0, &sensor_data, 1)) 
        {
            //LED_BEEP(led_warning);
            rt_kprintf("(Humi) Read %s data failed.\n", parameter);
        } else
        {
            //rt_kprintf("[%d] Humi: %d\n", sensor_data.timestamp, sensor_data.data.humi);
            sync(SENSOR_HUMI, sensor_data.data.humi);
        }
        rt_thread_mdelay(DELAY_TIME_DEFAULT);
    }
    rt_device_close(humi_dev);
}

static void read_dust_entry(void *parameter)
{
    rt_device_t dust_dev = RT_NULL;
    struct rt_sensor_data sensor_data;

    dust_dev = rt_device_find(parameter);
    if (dust_dev == RT_NULL)
    {
        rt_kprintf("(Dust) Can't find %s device.\n", parameter);
        return;
    }

    if (rt_device_open(dust_dev, RT_DEVICE_FLAG_RDWR)) 
    {
        rt_kprintf("(Dust) Open %s device failed.\n", parameter);
        return;
    }

    while(1)
    {
        if (1 != rt_device_read(dust_dev, 0, &sensor_data, 1))
        {
            rt_kprintf("(Dust) Read %s data failed.\n", parameter);
        } else
        {
            //rt_kprintf("[%d] Dust: %d\n", sensor_data.timestamp, sensor_data.data.dust);
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
        rt_kprintf("(TVOC) Can't find %s device.\n", parameter);
        return;
    }

    if (rt_device_open(tvoc_dev, RT_DEVICE_FLAG_RDWR)) 
    {
        rt_kprintf("(TVOC) Open %s device failed.\n", parameter);
        return;
    }

    while (1)
    {
        if (1 != rt_device_read(tvoc_dev, 0, &sensor_data, 1)) 
        {
            rt_kprintf("(TVOC) Read %s data failed.\n", parameter);
        } else
        {
            //rt_kprintf("[%d] TVOC: %d\n", sensor_data.timestamp, sensor_data.data.tvoc);
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
        rt_kprintf("(eCO2) Can't find %s device.\n", parameter);
        return;
    }

    if (rt_device_open(eco2_dev, RT_DEVICE_FLAG_RDWR)) 
    {
        rt_kprintf("(eCO2) Open %s device failed.\n", parameter);
        return;
    }

    while(1)
    {
        if (1 != rt_device_read(eco2_dev, 0, &sensor_data, 1)) 
        {
            rt_kprintf("(eCO2) Read %s data failed.\n", parameter);
        } else
        {
            //rt_kprintf("[%d] eCO2: %d\n", sensor_data.timestamp, sensor_data.data.eco2);
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

    temp_thread = rt_thread_create("temp_th", read_temp_entry, "temp_dh2", 1024, 10, 5);
    humi_thread = rt_thread_create("humi_th", read_humi_entry, "humi_dh2", 1024, 10, 5);
    dust_thread = rt_thread_create("dust_th", read_dust_entry, "dust_gp2", 1024, 15, 5);
    tvoc_thread = rt_thread_create("tvoc_th", read_tvoc_entry, "tvoc_sg3", 1024, 16, 5);
    eco2_thread = rt_thread_create("eco2_th", read_eco2_entry, "eco2_sg3", 1024, 16, 5);

    sync_thread = rt_thread_create("sync", sync_thread_entry, RT_NULL, 1024, 15, 5);
    upload_thread = rt_thread_create("upload", upload_thread_entry, RT_NULL, 2048, 5, 5);

    /* start up all user thread */
    if(temp_thread) rt_thread_startup(temp_thread);
    if(humi_thread) rt_thread_startup(humi_thread);
    if(dust_thread) rt_thread_startup(dust_thread);
    if(tvoc_thread) rt_thread_startup(tvoc_thread);
    if(eco2_thread) rt_thread_startup(eco2_thread);

    if(sync_thread) rt_thread_startup(sync_thread);
    if(upload_thread) rt_thread_startup(upload_thread);

    return RT_EOK;
}

static int rt_hw_dht22_port(void)
{
    struct dht_info info;
    struct rt_sensor_config cfg;

    info.type = DHT22;
    info.pin  = DHT22_DATA_PIN;
    
    cfg.intf.type = RT_SENSOR_INTF_ONEWIRE;
    cfg.intf.user_data = (void *)&info;
    rt_hw_dht_init("dh2", &cfg);
    
    return RT_EOK;
}
INIT_COMPONENT_EXPORT(rt_hw_dht22_port);

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
INIT_COMPONENT_EXPORT(rt_hw_gp2y10_port);

static int rt_hw_sgp30_port(void)
{
    struct rt_sensor_config cfg;
    
    cfg.intf.type = RT_SENSOR_INTF_I2C;
    cfg.intf.dev_name = SGP30_I2C_BUS_NAME;
    rt_hw_sgp30_init("sg3", &cfg);
    
    return RT_EOK;
}
INIT_COMPONENT_EXPORT(rt_hw_sgp30_port);
