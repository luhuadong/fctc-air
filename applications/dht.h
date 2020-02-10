/*
 * Copyright (c) 2020, RudyLo <luhuadong@163.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-01-20     luhuadong    the first version
 */

#ifndef __DHT_H__
#define __DHT_H__

#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>
#include "board.h"

#define DHTLIB_VERSION       "0.0.1"

/* timing */
#define DHT11_BEGIN_TIME     20  /* ms */
#define DHT2x_BEGIN_TIME     1   /* ms */
#define DHTxx_PULL_TIME      30  /* us */
#define DHTxx_REPLY_TIME     100 /* us */
#define MEASURE_TIME         40  /* us */

#define DHT_DATA_SIZE        5

typedef enum dht_type
{
	SENSOR_DHT11 = 11,
	SENSOR_DHT21 = 21,
	SENSOR_DHT22 = 22
}dht_type;

struct dht_device
{
	rt_uint8_t  type;
	rt_base_t   pin;
	rt_uint32_t begin_time;
	rt_uint8_t  data[DHT_DATA_SIZE];
	rt_mutex_t  lock;
};
typedef struct dht_device *dht_device_t;

dht_device_t dht_init(dht_device_t dev, dht_type type, rt_base_t pin);
rt_bool_t dht_read(dht_device_t dev);
float dht_get_humidity(dht_device_t dev);
float dht_get_temperature(dht_device_t dev);
float convert_c2k(float c);
float convert_c2f(float c);
float convert_f2c(float f);

//int rt_hw_dht_init(const char *name, struct rt_sensor_config *cfg);

#endif /* __DHT_H__ */
