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

#define LED_THREAD_PRIORITY         (RT_THREAD_PRIORITY_MAX - 1)
#define LED_THREAD_STACK_SIZE       (512)
#define LED_THREAD_TIMESLICE        (10)

struct led_node
{
    int ld;
    rt_base_t pin;
    rt_base_t active_logic;

    rt_uint32_t period;
    rt_uint32_t pulse;
    rt_uint32_t time;
    rt_uint32_t count;

    struct rt_thread *tid;

    int status; // idle, running
    rt_mutex_t  lock;
    rt_slist_t  list;
};

struct led_msg
{
    int ld;
    rt_uint32_t period;
    rt_uint32_t pulse;
    rt_uint32_t time;
    rt_uint32_t count;
};

// 链表本身也要加互斥锁，保证原子操作
struct littled_head
{
    rt_uint32_t ld_max;
    rt_mutex_t  lock;
    rt_slist_t  head;

};
static struct littled_head littled_list;



static rt_thread_t  littled_thread = RT_NULL;
static rt_mailbox_t littled_mb = RT_NULL;

int led_register(rt_base_t pin, rt_base_t active_logic)
{
    // create node
    struct led_node *new_led = (struct led_node *)rt_calloc(1, sizeof(struct led_node));
    if (new_led == RT_NULL)
    {
        return -RT_ENOMEM;
    }

    //rt_mutex_take(littled_list.lock, RT_WAITING_FOREVER);

    new_led->pin = pin;
    new_led->active_logic = active_logic;

    // insert list
    rt_slist_append(&littled_list.head, &new_led->list);

    int ld = littled_list.ld_max++;

    //rt_mutex_release(littled_list.lock);

    // set mode
    rt_pin_mode(new_led->pin, PIN_MODE_OUTPUT);
    rt_pin_write(new_led->pin, !new_led->active_logic);

    rt_kprintf("littled [%d] register\n", ld);

    return ld;
}

/**
 * This function will read a bit from sensor.
 *
 * @param ld    Led descriptor
 * @param freq  Frequency
 * @param duty  Threshold value (0: off, max: on)
 * @param time
 * @param count 
 *
 * @return the bit value
 */
int led_mode(int ld, rt_uint32_t period, rt_uint32_t pulse, rt_uint32_t time, rt_uint32_t count)
{
    // pack msg
    struct led_msg *msg = (struct led_msg*)rt_malloc(sizeof(struct led_msg));

    msg->ld     = ld;
    msg->period = period;
    msg->pulse  = pulse;
    msg->time   = time;
    msg->count  = count;

    // send mailbox
    rt_mb_send(littled_mb, (rt_ubase_t)msg);
}

/*
void led_toggle(int ld)
{
    rt_base_t pin = list(ld)->pin;

    rt_base_t value = rt_pin_read(pin);
    rt_pin_write(pin, !value);
}
*/

static void led_task_entry(void *args)
{
    struct led_node *led = (struct led_node *)args;

    rt_base_t   pin          = led->pin;
    rt_base_t   active_logic = led->active_logic;
    rt_uint32_t period       = led->period;
    rt_uint32_t pulse        = led->pulse;
    rt_uint32_t time         = led->time;
    rt_uint32_t count        = led->count;

    rt_uint32_t cur_time   = 0;
    rt_uint32_t cur_count  = 0;

    rt_kprintf("[%d] led task running\n", led->ld);

    if (pulse == 0)
    {
        rt_pin_write(pin, !active_logic);
        return;
    }

    if (pulse == period)
    {
        rt_pin_write(pin, active_logic);
        return;
    }

    while(cur_count <= count && cur_time <= time)
    {
        rt_pin_write(pin, active_logic);
        rt_thread_mdelay(pulse);
        rt_pin_write(pin, !active_logic);
        rt_thread_mdelay(period - pulse);

        if (count)
            cur_count++;

        if (time)
            cur_time += period;
    }

    led->tid = RT_NULL;
}

static void littled_daemon_entry(void *args)
{
    struct led_msg *msg;
    char led_thread_name[8];

    while(1)
    {
        // wait mailbox
        if (RT_EOK == rt_mb_recv(littled_mb, (rt_ubase_t *)&msg, RT_WAITING_FOREVER))
        {
            rt_kprintf("littled recv mailbox\n");

            rt_slist_t *node = RT_NULL;
            struct led_node *led = RT_NULL;

            // find led node
            rt_slist_for_each(node, &littled_list.head)
            {
                led = rt_slist_entry(node, struct led_node, list);
                if (msg->ld == led->ld)
                {
                    break;
                }
            }

            if (node == RT_NULL)
            {
                rt_kpintf("#8#\n");
                continue;
            }

            led->period = msg->period;
            led->pulse  = msg->pulse;
            led->time   = msg->time;
            led->count  = msg->count;

            if (led->tid)
            {
                rt_thread_delete(led->tid);
                led->tid = RT_NULL;
            }
            
            rt_free(msg);

            {
                rt_sprintf(led_thread_name, "led_%d", led->ld);
                rt_thread_t tid = rt_thread_create(led_thread_name, led_task_entry, led,
                                                    512, RT_THREAD_PRIORITY_MAX - 1, 5);
                
                
                rt_thread_startup(tid);
                led->tid = tid;
            }
        }
    }
}

static int littled_init(void)
{
    if (littled_thread)
    {
        LOG_W("littled thread has been created");
        return -RT_ERROR;
    }

    littled_list.ld_max = 0;
    littled_list.lock   = RT_NULL;
    littled_list.head.next   = RT_NULL;
    //littled_list.head   = RT_SLIST_OBJECT_INIT(littled_list.head);

    /* Create a mutex lock for list */

    littled_list.lock = rt_mutex_create("littled", RT_IPC_FLAG_FIFO);
    if (littled_list.lock == RT_NULL)
        goto __exit;

    /* Create a mailbox for thread */

    littled_mb = rt_mb_create("littled", 32, RT_IPC_FLAG_FIFO);
    if (littled_mb == RT_NULL)
        goto __exit;

    /* create the littled thread */

    littled_thread = rt_thread_create("littled", littled_daemon_entry, RT_NULL, 
                                      1024, RT_THREAD_PRIORITY_MAX / 2, 20);
    if (littled_thread == RT_NULL)
        goto __exit;

    rt_thread_startup(littled_thread);
    return RT_EOK;

__exit:
    if (littled_thread)
        rt_thread_delete(littled_thread);

    if (littled_mb)
        rt_mb_delete(littled_mb);

    if (littled_list.lock)
        rt_mutex_delete(littled_list.lock);

    return -RT_ERROR;
}
INIT_APP_EXPORT(littled_init);
