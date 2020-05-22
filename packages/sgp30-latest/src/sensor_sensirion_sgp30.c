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
#include "sgp30.h"

#define DBG_TAG "sensor.sensirion.sgp30"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

/* SGP30 constants */
#define SGP30_FEATURESET               (0x0020)  /* The required set for this library */
#define SGP30_CRC8_POLYNOMIAL          (0x31)    /* Seed for SGP30's CRC polynomial */
#define SGP30_CRC8_INIT                (0xFF)    /* Init value for CRC */
#define SGP30_WORD_LEN                 (2)       /* 2 bytes per word */

/* SGP30 commands */
#define Init_air_quality               (0x2003)
#define Measure_air_quality            (0x2008)
#define Get_baseline                   (0x2015)
#define Set_baseline                   (0x201e)
#define Set_humidity                   (0x2061)
#define Measure_test                   (0x2032)
#define Get_feature_set_version        (0x202f)
#define Measure_raw_signals            (0x2050)
#define Get_Serial_ID                  (0x3682)

/* range */
#define SENSOR_ECO2_RANGE_MIN          (0)
#define SENSOR_ECO2_RANGE_MAX          (8200)
#define SENSOR_TVOC_RANGE_MIN          (0)
#define SENSOR_TVOC_RANGE_MAX          (2000)

/* minial period (ms) */
#define SENSOR_PERIOD_MIN              (0)
#define SENSOR_ECO2_PERIOD_MIN         SENSOR_PERIOD_MIN
#define SENSOR_TVOC_PERIOD_MIN         SENSOR_PERIOD_MIN

/* fifo max length */
#define SENSOR_FIFO_MAX                (1)
#define SENSOR_ECO2_FIFO_MAX           SENSOR_FIFO_MAX
#define SENSOR_TVOC_FIFO_MAX           SENSOR_FIFO_MAX

/*!
 *  @brief  calculates 8-Bit checksum with given polynomial
 */
static rt_uint8_t generate_crc(rt_uint8_t data[], rt_uint8_t datalen)
{
    rt_uint8_t crc = SGP30_CRC8_INIT;

    for (rt_uint8_t i = 0; i < datalen; i++) {
        crc ^= data[i];
        for (rt_uint8_t b = 0; b < 8; b++) {
            if (crc & 0x80)
                crc = (crc << 1) ^ SGP30_CRC8_POLYNOMIAL;
            else
                crc <<= 1;
        }
    }
    return crc;
}

/*!
 *  @brief  I2C low level interfacing
 */
static rt_bool_t 
read_word_from_command(struct rt_i2c_bus_device *bus,
                       rt_uint8_t                cmd[], 
                       rt_uint8_t                cmdlen, 
                       rt_uint16_t               delayms, 
                       rt_uint16_t              *readdata, 
                       rt_uint8_t                readlen)
{
    /* Request */
    rt_i2c_master_send(bus, SGP30_I2CADDR, RT_I2C_WR, cmd, cmdlen);

    rt_thread_mdelay(delayms);

    /* If not need reply */
    if (readlen == 0) return RT_TRUE;

    /* Response */
    rt_uint8_t replylen = readlen * (SGP30_WORD_LEN + 1);
    rt_uint8_t reply[replylen];

    if (rt_i2c_master_recv(bus, SGP30_I2CADDR, RT_I2C_RD, reply, replylen) != replylen)
        return RT_FALSE;

    /* Generate CRC */
    for (rt_uint8_t i = 0; i < readlen; i++) {
        rt_uint8_t crc = generate_crc(reply + i * 3, SGP30_WORD_LEN);

        if (crc != reply[i * 3 + 2])
            return RT_FALSE;
       
        // success! store it
        readdata[i] = reply[i * 3];
        readdata[i] <<= 8;
        readdata[i] |= reply[i * 3 + 1];
    }

    return RT_TRUE;
}

static rt_err_t _sgp30_measure(struct rt_i2c_bus_device *i2c_bus, rt_uint16_t reply[], const rt_size_t len)
{
    RT_ASSERT(reply);

    if (len < 2)
        return -RT_ERROR;

    rt_uint8_t cmd[2] = {0x20, 0x08};  /* Measure_air_quality */
    
    if (!read_word_from_command(i2c_bus, cmd, 2, 12, reply, 2))
        return -RT_ERROR;

    return RT_EOK;
}

static rt_err_t _sgp30_measure_raw(struct rt_i2c_bus_device *i2c_bus, rt_uint16_t reply[], const rt_size_t len)
{
    RT_ASSERT(reply);

    if (len < 2)
        return -RT_ERROR;

    rt_uint8_t cmd[2] = {0x20, 0x50};  /* Measure_raw_signals */
    
    if (!read_word_from_command(i2c_bus, cmd, 2, 25, reply, 2))
        return -RT_ERROR;

    return RT_EOK;
}

static rt_err_t _sgp30_get_baseline(struct rt_i2c_bus_device *i2c_bus, void *args)
{
    struct sgp30_baseline *baseline = (struct sgp30_baseline *)args;

    rt_uint8_t cmd[2] = {0x20, 0x15};  /* Get_baseline */
    rt_uint16_t reply[2];
    
    if (!read_word_from_command(i2c_bus, cmd, 2, 10, reply, 2))
        return -RT_ERROR;

    baseline->eco2_base = reply[0];
    baseline->tvoc_base = reply[1];

    return RT_EOK;
}

static rt_err_t _sgp30_set_baseline(struct rt_i2c_bus_device *i2c_bus, void *args)
{
    struct sgp30_baseline *baseline = (struct sgp30_baseline *)args;

    rt_uint8_t cmd[8];
    cmd[0] = 0x20;
    cmd[1] = 0x1e;
    cmd[2] = baseline->tvoc_base >> 8;
    cmd[3] = baseline->tvoc_base & 0xFF;
    cmd[4] = generate_crc(cmd + 2, 2);
    cmd[5] = baseline->eco2_base >> 8;
    cmd[6] = baseline->eco2_base & 0xFF;
    cmd[7] = generate_crc(cmd + 5, 2);

    if (!read_word_from_command(i2c_bus, cmd, 8, 10, RT_NULL, 0))
        return -RT_ERROR;

    return RT_EOK;
}

static rt_err_t _sgp30_set_humidity(struct rt_i2c_bus_device *i2c_bus, void *args)
{
    rt_uint32_t absolute_humidity = (rt_uint32_t)args;

    if (absolute_humidity > 256000)
        return -RT_ERROR;

    rt_uint16_t ah_scaled = (rt_uint16_t)(((rt_uint64_t)absolute_humidity * 256 * 16777) >> 24);
    rt_uint8_t cmd[5];
    cmd[0] = 0x20;
    cmd[1] = 0x61;
    cmd[2] = ah_scaled >> 8;
    cmd[3] = ah_scaled & 0xFF;
    cmd[4] = generate_crc(cmd + 2, 2);

    if (!read_word_from_command(i2c_bus, cmd, 5, 10, RT_NULL, 0))
        return -RT_ERROR;

    return RT_EOK;
}

static rt_size_t _sgp30_polling_get_data(struct rt_sensor_device *sensor, void *buf)
{
    struct rt_sensor_data *sensor_data = buf;
    struct rt_i2c_bus_device *i2c_bus = (struct rt_i2c_bus_device *)sensor->config.intf.user_data;

    rt_uint16_t measure_data[2] = {0};
    if (RT_EOK != _sgp30_measure(i2c_bus, measure_data, sizeof(measure_data)))
    {
        LOG_E("Can not read from %s", sensor->info.model);
        return 0;
    }
    rt_uint32_t timestamp = rt_sensor_get_ts();
    rt_uint32_t eco2 = measure_data[0];
    rt_uint32_t tvoc = measure_data[1];

    if (sensor->info.type == RT_SENSOR_CLASS_ECO2)
    {
        sensor_data->type = RT_SENSOR_CLASS_ECO2;
        sensor_data->data.eco2 = eco2;
        sensor_data->timestamp = timestamp;

        struct rt_sensor_data *partner_data = (struct rt_sensor_data *)sensor->module->sen[1]->data_buf;
        if (partner_data)
        {
            partner_data->type = RT_SENSOR_CLASS_TVOC;
            partner_data->data.tvoc = tvoc;
            partner_data->timestamp = timestamp;
            sensor->module->sen[1]->data_len = sizeof(struct rt_sensor_data);
        }
    }
    else if (sensor->info.type == RT_SENSOR_CLASS_TVOC)
    {
        sensor_data->type = RT_SENSOR_CLASS_TVOC;
        sensor_data->data.tvoc = tvoc;
        sensor_data->timestamp = timestamp;

        struct rt_sensor_data *partner_data = (struct rt_sensor_data *)sensor->module->sen[0]->data_buf;
        if (partner_data)
        {
            partner_data->type = RT_SENSOR_CLASS_ECO2;
            partner_data->data.eco2 = eco2;
            partner_data->timestamp = timestamp;
            sensor->module->sen[0]->data_len = sizeof(struct rt_sensor_data);
        }
    }

    return 1;
}

static rt_size_t sgp30_fetch_data(struct rt_sensor_device *sensor, void *buf, rt_size_t len)
{
    if (sensor->config.mode == RT_SENSOR_MODE_POLLING)
    {
        return _sgp30_polling_get_data(sensor, buf);
    }
    else
        return 0;
}

static rt_err_t sgp30_control(struct rt_sensor_device *sensor, int cmd, void *args)
{
    rt_err_t result = RT_EOK;
    struct rt_i2c_bus_device *i2c_bus = (struct rt_i2c_bus_device *)sensor->config.intf.user_data;

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
    case RT_SENSOR_CTRL_GET_BASELINE:  /* Custom command : Get baseline */
        LOG_D("Custom command : Get baseline");
        if (args)
        {
            result = _sgp30_get_baseline(i2c_bus, args);
        }
        break;
    case RT_SENSOR_CTRL_SET_BASELINE:  /* Custom command : Set baseline */
        LOG_D("Custom command : Set baseline");
        if (args)
        {
            result = _sgp30_set_baseline(i2c_bus, args);
        }
        break;
    case RT_SENSOR_CTRL_SET_HUMIDITY:  /* Custom command : Set humidity */
        LOG_D("Custom command : Set humidity");
        if (args)
        {
            result = _sgp30_set_humidity(i2c_bus, args);
        }
        break;
    default:
        return -RT_ERROR;
        break;
    }

    return result;
}

static struct rt_sensor_ops sensor_ops =
{
    sgp30_fetch_data,
    sgp30_control
};

/*!
 *  @brief  Setups the hardware and detects a valid SGP30. Initializes I2C
 *          then reads the serialnumber and checks that we are talking to an
 *          SGP30. Commands the sensor to begin the IAQ algorithm. Must be 
 *          called after startup.
 *  @param  dev
 *          The pointer to I2C device
 *  @return RT_EOK if SGP30 found on I2C and command completed successfully, 
 *          -RT_ERROR if something went wrong!
 */
static rt_err_t _sensor_init(struct rt_i2c_bus_device *i2c_bus)
{
    rt_uint8_t cmd[2] = {0, 0};
    rt_uint16_t serialnumber[3] = {0, 0, 0};
    rt_uint16_t featureset;

    /* Soft Reset: Reset Command using the General Call address */
    cmd[0] = 0x00;
    cmd[1] = 0x06;
    if (!read_word_from_command(i2c_bus, cmd, 2, 10, RT_NULL, 0))
    {
        return -RT_ERROR;
    }

    /* Get_Serial_ID */
    cmd[0] = 0x36;
    cmd[1] = 0x82;
    if (!read_word_from_command(i2c_bus, cmd, 2, 10, serialnumber, 3))
    {
        return -RT_ERROR;
    }
    LOG_D("Serial number: %02d.%02d.%02d", serialnumber[0], serialnumber[1], serialnumber[2]);

    /* Get_feature_set_version */
    cmd[0] = 0x20;
    cmd[1] = 0x2F;
    if (!read_word_from_command(i2c_bus, cmd, 2, 10, &featureset, 1))
    {
        return -RT_ERROR;
    }
    LOG_D("Featureset 0x%x", featureset);

    if ((featureset & 0xF0) != SGP30_FEATURESET)
    {
        return -RT_ERROR;
    }

    /* Init_air_quality */
    cmd[0] = 0x20;
    cmd[1] = 0x03;
    if(!read_word_from_command(i2c_bus, cmd, 2, 10, RT_NULL, 0))
    {
        return -RT_ERROR;
    }

    return RT_EOK;
}

/**
 * This function will init dhtxx sensor device.
 *
 * @param intf  interface 
 *
 * @return RT_EOK
 */
static rt_err_t _sgp30_init(struct rt_sensor_intf *intf)
{
    if (intf->type == RT_SENSOR_INTF_I2C)
    {
        intf->user_data = (void *)rt_i2c_bus_device_find(intf->dev_name);
        if (intf->user_data == RT_NULL)
        {
            LOG_E("Can't find sgp30 device on '%s' ", intf->dev_name);
            return -RT_ERROR;
        }
    }
    return _sensor_init((struct rt_i2c_bus_device *)intf->user_data);
}

/**
 * Call function rt_hw_sgp30_init for initial and register a dhtxx sensor.
 *
 * @param name  the name will be register into device framework
 * @param cfg   sensor config
 *
 * @return the result
 */
rt_err_t rt_hw_sgp30_init(const char *name, struct rt_sensor_config *cfg)
{
    int result;
    rt_sensor_t sensor_tvoc = RT_NULL;
    rt_sensor_t sensor_eco2 = RT_NULL;
    struct rt_sensor_module *module = RT_NULL;

    if (_sgp30_init(&cfg->intf) != RT_EOK)
    {
        return -RT_ERROR;
    }
    
    module = rt_calloc(1, sizeof(struct rt_sensor_module));
    if (module == RT_NULL)
    {
        return -RT_ENOMEM;
    }

    /* eCO2 sensor register */
    {
        sensor_eco2 = rt_calloc(1, sizeof(struct rt_sensor_device));
        if (sensor_eco2 == RT_NULL)
            goto __exit;

        sensor_eco2->info.type       = RT_SENSOR_CLASS_ECO2;
        sensor_eco2->info.vendor     = RT_SENSOR_VENDOR_SENSIRION;
        sensor_eco2->info.model      = "sgp30";
        sensor_eco2->info.unit       = RT_SENSOR_UNIT_ONE;
        sensor_eco2->info.intf_type  = RT_SENSOR_INTF_I2C;
        sensor_eco2->info.range_max  = SENSOR_ECO2_RANGE_MAX;
        sensor_eco2->info.range_min  = SENSOR_ECO2_RANGE_MIN;
        sensor_eco2->info.period_min = SENSOR_ECO2_PERIOD_MIN;
        sensor_eco2->info.fifo_max   = SENSOR_ECO2_FIFO_MAX;
        sensor_eco2->data_len        = 0;

        rt_memcpy(&sensor_eco2->config, cfg, sizeof(struct rt_sensor_config));
        sensor_eco2->ops = &sensor_ops;
        sensor_eco2->module = module;

        result = rt_hw_sensor_register(sensor_eco2, name, RT_DEVICE_FLAG_RDWR, RT_NULL);
        if (result != RT_EOK)
        {
            LOG_E("device register err code: %d", result);
            goto __exit;
        }
    }

    /* TVOC sensor register */
    {
        sensor_tvoc = rt_calloc(1, sizeof(struct rt_sensor_device));
        if (sensor_tvoc == RT_NULL)
            goto __exit;

        sensor_tvoc->info.type       = RT_SENSOR_CLASS_TVOC;
        sensor_tvoc->info.vendor     = RT_SENSOR_VENDOR_SENSIRION;
        sensor_tvoc->info.model      = "sgp30";
        sensor_tvoc->info.unit       = RT_SENSOR_UNIT_ONE;
        sensor_tvoc->info.intf_type  = RT_SENSOR_INTF_I2C;
        sensor_tvoc->info.range_max  = SENSOR_TVOC_RANGE_MAX;
        sensor_tvoc->info.range_min  = SENSOR_TVOC_RANGE_MIN;
        sensor_tvoc->info.period_min = SENSOR_TVOC_PERIOD_MIN;
        sensor_tvoc->info.fifo_max   = SENSOR_TVOC_FIFO_MAX;
        sensor_tvoc->data_len        = 0;

        rt_memcpy(&sensor_tvoc->config, cfg, sizeof(struct rt_sensor_config));
        sensor_tvoc->ops = &sensor_ops;
        sensor_tvoc->module = module;
        
        result = rt_hw_sensor_register(sensor_tvoc, name, RT_DEVICE_FLAG_RDWR, RT_NULL);
        if (result != RT_EOK)
        {
            LOG_E("device register err code: %d", result);
            goto __exit;
        }
    }

    module->sen[0] = sensor_eco2;
    module->sen[1] = sensor_tvoc;
    module->sen_num = 2;
    
    LOG_I("sensor init success");
    
    return RT_EOK;
    
__exit:
    if(sensor_tvoc) 
    {
        if(sensor_tvoc->data_buf)
            rt_free(sensor_tvoc->data_buf);

        rt_free(sensor_tvoc);
    }
    if(sensor_eco2) 
    {
        if(sensor_eco2->data_buf)
            rt_free(sensor_eco2->data_buf);

        rt_free(sensor_eco2);
    }
    if (module)
        rt_free(module);

    return -RT_ERROR;
}
