/*
 * Copyright (c) 2020, RudyLo <luhuadong@163.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-02-01     RudyLo       the first version
 */

#ifndef __SGP30_H__
#define __SGP30_H__

#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>
#include "board.h"

#define SGP30LIB_VERSION           "0.0.1"

// the i2c address
#define SGP30_I2CADDR              0x58    /* SGP30 has only one I2C address */

// commands and constants
#define SGP30_FEATURESET           0x0020  /* The required set for this library */
#define SGP30_CRC8_POLYNOMIAL      0x31    /* Seed for SGP30's CRC polynomial */
#define SGP30_CRC8_INIT            0xFF    /* Init value for CRC */
#define SGP30_WORD_LEN             2       /* 2 bytes per word */

/* Command */
#define Init_air_quality           0x2003
#define Measure_air_quality        0x2008
#define Get_baseline               0x2015
#define Set_baseline               0x201e
#define Set_humidity               0x2061
#define Measure_test               0x2032
#define Get_feature_set_version    0x202f
#define Measure_raw_signals        0x2050

#define Get_Serial_ID              0x3682

struct sgp30_device
{
	struct rt_i2c_bus_device *i2c;

	rt_uint16_t TVOC;
	rt_uint16_t eCO2;
	rt_uint16_t rawH2;
	rt_uint16_t rawEthanol;
	rt_uint16_t serialnumber[3];

	rt_bool_t   is_ready;
	rt_mutex_t  lock;
};
typedef struct sgp30_device *sgp30_device_t;

//rt_bool_t sgp30_begin(TwoWire *theWire = &Wire);
//rt_bool_t sgp30_init(sgp30_device_t dev, const char *i2c_bus_name);
sgp30_device_t sgp30_init(const char *i2c_bus_name);
void sgp30_deinit(sgp30_device_t dev);
rt_bool_t sgp30_measure(sgp30_device_t dev);
rt_bool_t sgp30_measure_raw(sgp30_device_t dev);

rt_bool_t sgp30_get_baseline(sgp30_device_t dev, rt_uint16_t *eco2_base, rt_uint16_t *tvoc_base);
rt_bool_t sgp30_set_baseline(sgp30_device_t dev, rt_uint16_t eco2_base, rt_uint16_t tvoc_base);
rt_bool_t sgp30_set_humidity(sgp30_device_t dev, rt_uint32_t absolute_humidity);

#endif /* __SGP30_H__ */