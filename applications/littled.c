/*
 * Copyright (c) 2020, RudyLo <luhuadong@163.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-02-24     luhuadong    the first version
 */

#include <rtthread.h>
#include <rtdevice.h>

#define DBG_TAG "pkg.littled"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>


struct led_node
{
    int ld;
    rt_base_t pin;
    rt_base_t active_logic;

    int freq;
    int duty;
    int time;
    int count;
    int cur_count;

    struct rt_thread *tid;

    int status; // idle, running
    rt_mutex_t  lock;
    rt_slist_t slist;
};

struct led_msg
{
    int ld;
    int freq;
    int duty;
    int time;
    int count;
};

static rt_slist_t littled_list = RT_SLIST_OBJECT_INIT(littled_list);

//static int is_singleton = 0;
static rt_bool_t has_instance = RT_FALSE;

int led_register(rt_base_t pin, rt_base_t active_logic)
{
    // create node

    // insert list
}


/**
 * This function will read a bit from sensor.
 *
 * @param ld  led descriptor
 *
 * @return the bit value
 */
int led_mode(ld, freq, duty, time, count)
{
    // pack msg
    
    // send mailbox
}


static void led_short_entry(void *args)
{
    //
}

static void led_long_entry(void *args)
{
    while(1)
    {
        //
    }
}

static void littled_entry(void *args)
{
    while(1)
    {
        // wait mailbox
    }
}

static int littled_init(void)
{
    rt_thread_t littled_thread;

    littled_thread = rt_thread_create("littled", littled_entry, 
                                      "RT_NULL", 1024, 
                                       RT_THREAD_PRIORITY_MAX / 2, 20);
    
    if (littled_thread) 
        rt_thread_startup(littled_thread);
}
INIT_APP_EXPORT(littled_init);
