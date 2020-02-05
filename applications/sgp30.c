/*
 * Copyright (c) 2020, RudyLo <luhuadong@163.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-02-01     RudyLo       the first version
 */

#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <string.h>

#define DBG_ENABLE
#define DBG_SECTION_NAME "SGP30"
#define DBG_LEVEL DBG_LOG
#define DBG_COLOR
#include <rtdbg.h>

#include "sgp30.h"

//#ifdef PKG_USING_SGP30

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
static rt_err_t sensor_init(sgp30_device_t dev)
{
    rt_uint8_t cmd[2] = {0, 0};
    rt_uint16_t featureset;

    /* Soft Reset: Reset Command using the General Call address */
    cmd[0] = 0x00;
    cmd[1] = 0x06;
    if (!read_word_from_command(dev->i2c, cmd, 2, 10, RT_NULL, 0))
        return -RT_ERROR;

    /* Get_Serial_ID */
    cmd[0] = 0x36;
    cmd[1] = 0x82;
    if (!read_word_from_command(dev->i2c, cmd, 2, 10, dev->serialnumber, 3))
    	return -RT_ERROR;

    /* Get_feature_set_version */
    cmd[0] = 0x20;
    cmd[1] = 0x2F;
    if (!read_word_from_command(dev->i2c, cmd, 2, 10, &featureset, 1))
    	return -RT_ERROR;

    rt_kprintf("(SGP30) Featureset 0x%x\n", featureset);
    if ((featureset & 0xF0) != SGP30_FEATURESET)
    	return -RT_ERROR;

    /* Init_air_quality */
    cmd[0] = 0x20;
    cmd[1] = 0x03;
    if(!read_word_from_command(dev->i2c, cmd, 2, 10, RT_NULL, 0))
    	return -RT_ERROR;

    return RT_EOK;
}

/**
 * This function initializes sgp30 registered device driver
 *
 * @param dev the name of sgp30 device
 *
 * @return the sgp30 device.
 */
sgp30_device_t sgp30_init(const char *i2c_bus_name)
{
	sgp30_device_t dev;

	RT_ASSERT(i2c_bus_name);

	dev = rt_calloc(1, sizeof(struct sgp30_device));
    if (dev == RT_NULL)
    {
        LOG_E("Can't allocate memory for sgp30 device on '%s' ", i2c_bus_name);
        return RT_NULL;
    }

    dev->is_ready = RT_FALSE;

	dev->i2c = rt_i2c_bus_device_find(i2c_bus_name);
    if (dev->i2c == RT_NULL)
    {
        LOG_E("Can't find sgp30 device on '%s' ", i2c_bus_name);
        rt_free(dev);
        return RT_NULL;
    }

    dev->lock = rt_mutex_create("mutex_sgp30", RT_IPC_FLAG_FIFO);
    if (dev->lock == RT_NULL)
    {
        LOG_E("Can't create mutex for sgp30 device on '%s' ", i2c_bus_name);
        rt_free(dev);
        return RT_NULL;
    }

    if (sensor_init(dev) != RT_EOK)
        return RT_NULL;
    else
        return dev;
}

/**
 * This function releases memory and deletes mutex lock
 *
 * @param dev the pointer of device driver structure
 */
void sgp30_deinit(sgp30_device_t dev)
{
    RT_ASSERT(dev);

    rt_mutex_delete(dev->lock);

    rt_free(dev);
}

/*!
 *  @brief  Commands the sensor to take a single eCO2/VOC measurement. Places
 *          results in {@link TVOC} and {@link eCO2}
 *  @return True if command completed successfully, false if something went
 *          wrong!
 */
rt_bool_t sgp30_measure(sgp30_device_t dev)
{
	RT_ASSERT(dev);

    rt_uint8_t  cmd[2];
    rt_uint16_t reply[2];

    cmd[0] = 0x20;  /* Measure_air_quality */
    cmd[1] = 0x08;
    
    if (!read_word_from_command(dev->i2c, cmd, 2, 12, reply, 2))
        return RT_FALSE;

    dev->eCO2 = reply[0];
    dev->TVOC = reply[1];

    return RT_TRUE;
}

 /*!
  *  @brief  Commands the sensor to take a single H2/ethanol raw measurement. Places results in {@link rawH2} and {@link rawEthanol}
  *  @returns True if command completed successfully, false if something went wrong!
  */
rt_bool_t sgp30_measure_raw(sgp30_device_t dev)
{
	RT_ASSERT(dev);

    rt_uint8_t  cmd[2];
    rt_uint16_t reply[2];

    cmd[0] = 0x20;  /* Measure_raw_signals */
    cmd[1] = 0x50;
    
    if (!read_word_from_command(dev->i2c, cmd, 2, 25, reply, 2))
        return RT_FALSE;

    dev->rawH2      = reply[0];
    dev->rawEthanol = reply[1];

    return RT_TRUE;
}

/*!
 *   @brief  Request baseline calibration values for both CO2 and TVOC IAQ
 *           calculations. Places results in parameter memory locaitons.
 *   @param  eco2_base 
 *           A pointer to a uint16_t which we will save the calibration
 *           value to
 *   @param  tvoc_base 
 *           A pointer to a uint16_t which we will save the calibration value to
 *   @return True if command completed successfully, false if something went
 *           wrong!
 */
rt_bool_t 
sgp30_get_baseline(sgp30_device_t dev, rt_uint16_t *eco2_base, rt_uint16_t *tvoc_base)
{
	RT_ASSERT(dev);

    rt_uint8_t  cmd[2];
    rt_uint16_t reply[2];

    cmd[0] = 0x20;  /* Get_baseline */
    cmd[1] = 0x15;
    
    if (!read_word_from_command(dev->i2c, cmd, 2, 10, reply, 2))
        return RT_FALSE;

    *eco2_base = reply[0];
    *tvoc_base = reply[1];

    return RT_TRUE;
}

/*!
 *  @brief  Assign baseline calibration values for both CO2 and TVOC IAQ
 *          calculations.
 *  @param  eco2_base 
 *          A uint16_t which we will save the calibration value from
 *  @param  tvoc_base 
 *          A uint16_t which we will save the calibration value from
 *  @return True if command completed successfully, false if something went
 *          wrong!
 */
rt_bool_t 
sgp30_set_baseline(sgp30_device_t dev, rt_uint16_t eco2_base, rt_uint16_t tvoc_base)
{
	RT_ASSERT(dev);

    rt_uint8_t cmd[8];
    cmd[0] = 0x20;
    cmd[1] = 0x1e;
    cmd[2] = tvoc_base >> 8;
    cmd[3] = tvoc_base & 0xFF;
    cmd[4] = generate_crc(cmd + 2, 2);
    cmd[5] = eco2_base >> 8;
    cmd[6] = eco2_base & 0xFF;
    cmd[7] = generate_crc(cmd + 5, 2);

    return read_word_from_command(dev->i2c, cmd, 8, 10, RT_NULL, 0);
}

/*!
 *  @brief  Set the absolute humidity value [mg/m^3] for compensation to increase
 *          precision of TVOC and eCO2.
 *  @param  absolute_humidity 
 *          A uint32_t [mg/m^3] which we will be used for compensation.
 *          If the absolute humidity is set to zero, humidity compensation
 *          will be disabled.
 *  @return True if command completed successfully, false if something went
 *          wrong!
 */
rt_bool_t 
sgp30_set_humidity(sgp30_device_t dev, rt_uint32_t absolute_humidity)
{
	RT_ASSERT(dev);

    if (absolute_humidity > 256000) {
        return RT_FALSE;
    }

    rt_uint16_t ah_scaled = (rt_uint16_t)(((rt_uint64_t)absolute_humidity * 256 * 16777) >> 24);
    rt_uint8_t cmd[5];
    cmd[0] = 0x20;
    cmd[1] = 0x61;
    cmd[2] = ah_scaled >> 8;
    cmd[3] = ah_scaled & 0xFF;
    cmd[4] = generate_crc(cmd + 2, 2);

  return read_word_from_command(dev->i2c, cmd, 5, 10, RT_NULL, 0);

}

//#endif /* PKG_USING_SGP30 */
