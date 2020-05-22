/*
 * Copyright (c) 2020, RudyLo <luhuadong@163.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-02-17     luhuadong    the first version
 */

#include <board.h>
#include "dhtxx.h"

#define DBG_TAG "sensor.asair.dhtxx"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

/* timing */
#define DHT1x_BEGIN_TIME         20  /* ms */
#define DHT2x_BEGIN_TIME         1   /* ms */
#define DHTxx_PULL_TIME          30  /* us */
#define DHTxx_REPLY_TIME         100 /* us */
#define MEASURE_TIME             40  /* us */

/* range by ten times */
#define SENSOR_HUMI_RANGE_MIN    0
#define SENSOR_HUMI_RANGE_MAX    1000
#define SENSOR_TEMP_RANGE_MIN    -400
#define SENSOR_TEMP_RANGE_MAX    800

/* minial period (ms) */
#define SENSOR_PERIOD_MIN        1000
#define SENSOR_HUMI_PERIOD_MIN   SENSOR_PERIOD_MIN
#define SENSOR_TEMP_PERIOD_MIN   SENSOR_PERIOD_MIN

/* fifo max length */
#define SENSOR_FIFO_MAX          1
#define SENSOR_HUMI_FIFO_MAX     SENSOR_FIFO_MAX
#define SENSOR_TEMP_FIFO_MAX     SENSOR_FIFO_MAX

static char *const dht_model_table[] = 
{
    "dht11", 
    "dht12", 
    "dht21", 
    "dht22"
};

RT_WEAK void rt_hw_us_delay(rt_uint32_t us)
{
    rt_uint32_t delta;

    us = us * (SysTick->LOAD / (1000000 / RT_TICK_PER_SECOND));
    delta = SysTick->VAL;

    while (delta - SysTick->VAL < us) continue;
}

/**
 * This function will read a bit from sensor.
 *
 * @param pin  the pin of Dout
 *
 * @return the bit value
 */
static uint8_t _dht_read_bit(const rt_base_t pin)
{
    uint8_t retry = 0;

    while(rt_pin_read(pin) && retry < DHTxx_REPLY_TIME)
    {
        retry++;
        rt_hw_us_delay(1);
    }

    retry = 0;
    while(!rt_pin_read(pin) && retry < DHTxx_REPLY_TIME)
    {
        retry++;
        rt_hw_us_delay(1);
    }

    rt_hw_us_delay(MEASURE_TIME);
    
    return rt_pin_read(pin);
}

/**
 * This function will read a byte from sensor.
 *
 * @param pin  the pin of Dout
 *
 * @return the byte
 */
static uint8_t _dht_read_byte(const rt_base_t pin)
{
    uint8_t i, byte = 0;

    for(i=0; i<8; i++)
    {
        byte <<= 1;
        byte |= _dht_read_bit(pin);
    }

    return byte;
}

/**
 * This function will read and update data array.
 *
 * @param sensor   the sensor device to be operated
 * @param data     read data
 *
 * @return RT_TRUE if read successfully, otherwise return RT_FALSE.
 */
static rt_bool_t _dht_read(struct rt_sensor_device *sensor, rt_uint8_t data[])
{
    RT_ASSERT(data);

    dht_info_t dht_info = (dht_info_t)sensor->config.intf.user_data;
    if (!dht_info)
    {
        LOG_D("user_data is null");
        return RT_FALSE;
    }
    rt_uint8_t type = dht_info->type;
    rt_base_t  pin  = dht_info->pin;

    uint8_t i, retry = 0, sum = 0;
    rt_base_t level;

    /* Reset data buffer */
    rt_memset(data, 0, DHT_DATA_SIZE);

    /* MCU request sampling */
    rt_pin_mode(pin, PIN_MODE_OUTPUT);
    rt_pin_write(pin, PIN_LOW);

    if (type == DHT11 || type == DHT12) {
        rt_thread_mdelay(DHT1x_BEGIN_TIME);        /* Tbe */
    } else {
        rt_thread_mdelay(DHT2x_BEGIN_TIME);
    }

#ifdef PKG_USING_DHTXX_INTERRUPT_DISABLE
    level = rt_hw_interrupt_disable();
#endif

    rt_pin_mode(pin, PIN_MODE_INPUT_PULLUP);
    rt_hw_us_delay(DHTxx_PULL_TIME);               /* Tgo */

    /* Waiting for sensor reply */
    while (rt_pin_read(pin) && retry < DHTxx_REPLY_TIME)
    {
        retry++;
        rt_hw_us_delay(1);                         /* Trel */
    }
    if(retry >= DHTxx_REPLY_TIME)
    {
        LOG_D("sensor reply timeout on low level");
        return RT_FALSE;
    }

    retry = 0;
    while (!rt_pin_read(pin) && retry < DHTxx_REPLY_TIME)
    {
        retry++;
        rt_hw_us_delay(1);                         /* Treh */
    };
    if(retry >= DHTxx_REPLY_TIME)
    {
        LOG_D("sensor reply timeout on high level");
        return RT_FALSE;
    }

    /* Read data */
    for(i=0; i<DHT_DATA_SIZE; i++)
    {
        data[i] = _dht_read_byte(pin);
    }

#ifdef PKG_USING_DHTXX_INTERRUPT_DISABLE
    rt_hw_interrupt_enable(level);
#endif

    /* Checksum */
    for(i=0; i<DHT_DATA_SIZE-1; i++)
    {
        sum += data[i];
    }
    if(sum != data[4])
    {
        LOG_D("checksum error");
        return RT_FALSE;
    }

    return RT_TRUE;
}

/**
 * This function will get the humidity from dhtxx sensor.
 *
 * @param sensor   the sensor device to be operated
 * @param raw_data raw data from sensor
 *
 * @return the humidity value
 */
static rt_int32_t _dht_get_humidity(struct rt_sensor_device *sensor, rt_uint8_t raw_data[])
{
    RT_ASSERT(raw_data);

    rt_int32_t humi = 0;
    dht_info_t dht_info = (dht_info_t)sensor->config.intf.user_data;

    switch (dht_info->type)
    {
    case DHT11:
    case DHT12:
        humi = raw_data[0] * 10 + raw_data[1];
        break;
    case DHT21:
    case DHT22:
        humi = (raw_data[0] << 8) + raw_data[1];
        break;
    default:
        break;
    }

    return humi;
}

/**
 * This function will get the temperature from dhtxx sensor.
 *
 * @param sensor   the sensor device to be operated
 * @param raw_data raw data from sensor
 *
 * @return the temperature value
 */
static rt_int32_t _dht_get_temperature(struct rt_sensor_device *sensor, rt_uint8_t raw_data[])
{
    RT_ASSERT(raw_data);

    rt_int32_t temp = 0;
    dht_info_t dht_info = (dht_info_t)sensor->config.intf.user_data;

    switch (dht_info->type)
    {
    case DHT11:
    case DHT12:
        temp = raw_data[2] * 10 + raw_data[3];
        break;
    case DHT21:
    case DHT22:
        temp = ((raw_data[2] & 0x7f) << 8) + raw_data[3];
        if(raw_data[2] & 0x80) {
            temp = -temp;
        }
        break;
    default:
        break;
    }

    return temp;
}

static rt_size_t _dht_polling_get_data(struct rt_sensor_device *sensor, void *buf)
{
    struct rt_sensor_data *sensor_data = buf;

    rt_uint8_t raw_data[DHT_DATA_SIZE] = {0};
    if (RT_TRUE != _dht_read(sensor, raw_data))
    {
        LOG_D("Can not read from %s", sensor->info.model);
        return 0;
    }
    rt_uint32_t timestamp = rt_sensor_get_ts();
    rt_int32_t temp = _dht_get_temperature(sensor, raw_data);
    rt_int32_t humi = _dht_get_humidity(sensor, raw_data);

    if (sensor->info.type == RT_SENSOR_CLASS_HUMI)
    {
        sensor_data->type = RT_SENSOR_CLASS_HUMI;
        sensor_data->data.humi = humi;
        sensor_data->timestamp = timestamp;

        struct rt_sensor_data *partner_data = (struct rt_sensor_data *)sensor->module->sen[1]->data_buf;
        if (partner_data)
        {
            partner_data->type = RT_SENSOR_CLASS_TEMP;
            partner_data->data.temp = temp;
            partner_data->timestamp = timestamp;
            sensor->module->sen[1]->data_len = sizeof(struct rt_sensor_data);
        }
    }
    else if (sensor->info.type == RT_SENSOR_CLASS_TEMP)
    {
        sensor_data->type = RT_SENSOR_CLASS_TEMP;
        sensor_data->data.temp = temp;
        sensor_data->timestamp = timestamp;

        struct rt_sensor_data *partner_data = (struct rt_sensor_data *)sensor->module->sen[0]->data_buf;
        if (partner_data)
        {
            partner_data->type = RT_SENSOR_CLASS_HUMI;
            partner_data->data.humi = humi;
            partner_data->timestamp = timestamp;
            sensor->module->sen[0]->data_len = sizeof(struct rt_sensor_data);
        }
    }

    return 1;
}

static rt_size_t dht_fetch_data(struct rt_sensor_device *sensor, void *buf, rt_size_t len)
{
    if (sensor->config.mode == RT_SENSOR_MODE_POLLING)
    {
        return _dht_polling_get_data(sensor, buf);
    }
    else
        return 0;
}

static rt_err_t dht_control(struct rt_sensor_device *sensor, int cmd, void *args)
{
    rt_err_t result = RT_EOK;

    switch (cmd)
    {
    case RT_SENSOR_CTRL_GET_ID:
        break;
    case RT_SENSOR_CTRL_SET_MODE:
        sensor->config.mode = (rt_uint32_t)args & 0xFF;
        break;
    case RT_SENSOR_CTRL_SET_RANGE:
        break;
    case RT_SENSOR_CTRL_SET_ODR:
        break;
    case RT_SENSOR_CTRL_SET_POWER:
        break;
    case RT_SENSOR_CTRL_SELF_TEST:
        break;
    default:
        break;
    }

    return result;
}

static struct rt_sensor_ops sensor_ops =
{
    dht_fetch_data,
    dht_control
};

/**
 * This function will init dhtxx sensor device.
 *
 * @param intf  interface 
 *
 * @return RT_EOK
 */
static int _dht_init(struct dht_info *info)
{
    rt_pin_mode(info->pin, PIN_MODE_INPUT_PULLUP);

    return RT_EOK;
}

/**
 * Call function rt_hw_dht_init for initial and register a dhtxx sensor.
 *
 * @param name  the name will be register into device framework
 * @param cfg   sensor config
 *
 * @return the result
 */
rt_err_t rt_hw_dht_init(const char *name, struct rt_sensor_config *cfg)
{
    int result;
    rt_sensor_t sensor_temp = RT_NULL, sensor_humi = RT_NULL;
    struct rt_sensor_module *module = RT_NULL;

    dht_info_t dht_info = rt_calloc(1, sizeof(struct dht_info));
    if (dht_info == RT_NULL)
    {
        result = -RT_ENOMEM;
        goto __exit;
    }
    rt_memcpy(dht_info, cfg->intf.user_data, sizeof(struct dht_info));

    if (_dht_init(dht_info) != RT_EOK)
    {
        LOG_E("dhtxx sensor init failed");
        result = -RT_ERROR;
        goto __exit;
    }
    
    module = rt_calloc(1, sizeof(struct rt_sensor_module));
    if (module == RT_NULL)
    {
        result = -RT_ENOMEM;
        goto __exit;
    }

    /* humidity sensor register */
    {
        sensor_humi = rt_calloc(1, sizeof(struct rt_sensor_device));
        if (sensor_humi == RT_NULL)
        {
            result = -RT_ENOMEM;
            goto __exit;
        }

        sensor_humi->info.type       = RT_SENSOR_CLASS_HUMI;
        sensor_humi->info.vendor     = RT_SENSOR_VENDOR_ASAIR;
        sensor_humi->info.model      = dht_model_table[dht_info->type];
        sensor_humi->info.unit       = RT_SENSOR_UNIT_PERMILLAGE;
        sensor_humi->info.intf_type  = RT_SENSOR_INTF_ONEWIRE;
        sensor_humi->info.range_max  = SENSOR_HUMI_RANGE_MAX;
        sensor_humi->info.range_min  = SENSOR_HUMI_RANGE_MIN;
        sensor_humi->info.period_min = SENSOR_HUMI_PERIOD_MIN;
        sensor_humi->info.fifo_max   = SENSOR_HUMI_FIFO_MAX;
        sensor_humi->data_len        = 0;

        rt_memcpy(&sensor_humi->config, cfg, sizeof(struct rt_sensor_config));
        sensor_humi->ops = &sensor_ops;
        sensor_humi->module = module;
        sensor_humi->config.intf.user_data = (void *)dht_info;
        
        result = rt_hw_sensor_register(sensor_humi, name, RT_DEVICE_FLAG_RDWR, RT_NULL);
        if (result != RT_EOK)
        {
            LOG_E("device register err code: %d", result);
            result = -RT_ERROR;
            goto __exit;
        }
    }

    /* temperature sensor register */
    {
        sensor_temp = rt_calloc(1, sizeof(struct rt_sensor_device));
        if (sensor_temp == RT_NULL)
        {
            result = -RT_ENOMEM;
            goto __exit;
        }

        sensor_temp->info.type       = RT_SENSOR_CLASS_TEMP;
        sensor_temp->info.vendor     = RT_SENSOR_VENDOR_ASAIR;
        sensor_temp->info.model      = dht_model_table[dht_info->type];
        sensor_temp->info.unit       = RT_SENSOR_UNIT_DCELSIUS;
        sensor_temp->info.intf_type  = RT_SENSOR_INTF_ONEWIRE;
        sensor_temp->info.range_max  = SENSOR_TEMP_RANGE_MAX;
        sensor_temp->info.range_min  = SENSOR_TEMP_RANGE_MIN;
        sensor_temp->info.period_min = SENSOR_TEMP_PERIOD_MIN;
        sensor_temp->info.fifo_max   = SENSOR_TEMP_FIFO_MAX;
        sensor_temp->data_len        = 0;

        rt_memcpy(&sensor_temp->config, cfg, sizeof(struct rt_sensor_config));
        sensor_temp->ops = &sensor_ops;
        sensor_temp->module = module;
        sensor_temp->config.intf.user_data = (void *)dht_info;

        result = rt_hw_sensor_register(sensor_temp, name, RT_DEVICE_FLAG_RDWR, RT_NULL);
        if (result != RT_EOK)
        {
            LOG_E("device register err code: %d", result);
            result = -RT_ERROR;
            goto __exit;
        }
    }

    module->sen[0] = sensor_humi;
    module->sen[1] = sensor_temp;
    module->sen_num = 2;
    
    LOG_I("sensor init success");
    
    return RT_EOK;
    
__exit:
    if(sensor_humi) 
    {
        if(sensor_humi->data_buf)
            rt_free(sensor_humi->data_buf);

        rt_free(sensor_humi);
    }
    if(sensor_temp) 
    {
        if(sensor_temp->data_buf)
            rt_free(sensor_temp->data_buf);

        rt_free(sensor_temp);
    }
    if (module)
        rt_free(module);
    if (dht_info)
        rt_free(dht_info);

    return result;
}
