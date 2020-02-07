/*
 * Copyright (c) 2020, RudyLo <luhuadong@163.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-02-07     RudyLo       the first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>

#include "fctc_air.h"

#define LED_THREAD_PRIORITY      20
#define LED_THREAD_STACK_SIZE    512
#define LED_THREAD_TIMESLICE     15

#define SAMPLE_UART_NAME       "uart1"    /* 串口设备名称 */
static rt_device_t serial;                /* 串口设备句柄 */
static struct rt_semaphore rx_sem;    /* 用于接收消息的信号量 */

static rt_thread_t bc28_rx_thread = RT_NULL;

/* 接收数据回调函数 */
static rt_err_t uart_input(rt_device_t dev, rt_size_t size)
{
    /* 串口接收到数据后产生中断，调用此回调函数，然后发送接收信号量 */
    rt_sem_release(&rx_sem);

    return RT_EOK;
}

/* 接收数据的线程 */
static void serial_thread_entry(void *parameter)
{
    char ch;

    while (1)
    {
        /* 从串口读取一个字节的数据，没有读取到则等待接收信号量 */
        while (rt_device_read(serial, -1, &ch, 1) != 1)
        {
            /* 阻塞等待接收信号量，等到信号量后再次读取数据 */
            rt_sem_take(&rx_sem, RT_WAITING_FOREVER);
        }
        /* 输出 */
        rt_kprintf("%c", ch);
    }
}


/* 设置发送完成回调函数 */
//rt_err_t rt_device_set_tx_complete(rt_device_t dev, rt_err_t (*tx_done)(rt_device_t dev,void *buffer));

/* cat_bc28 */
static void cat_bc28(void)
{
    rt_kprintf("hello Quectel BC28\n");

    char cmd[] = "AT\r\n";

    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;

    /* step1：查找串口设备 */
    serial = rt_device_find(SAMPLE_UART_NAME);

    /* step2：修改串口配置参数 */
    config.baud_rate = BAUD_RATE_9600;        //修改波特率为 9600
    config.data_bits = DATA_BITS_8;           //数据位 8
    config.stop_bits = STOP_BITS_1;           //停止位 1
    config.bufsz     = 128;                   //修改缓冲区 buff size 为 128
    config.parity    = PARITY_NONE;           //无奇偶校验位

    /* step3：控制串口设备。通过控制接口传入命令控制字，与控制参数 */
    rt_device_control(serial, RT_DEVICE_CTRL_CONFIG, &config);

    /* step4：打开串口设备。以中断接收及轮询发送模式打开串口设备 */
    rt_device_open(serial, RT_DEVICE_FLAG_INT_RX);

    /* 初始化信号量 */
    rt_sem_init(&rx_sem, "rx_sem", 0, RT_IPC_FLAG_FIFO);

    /* 设置接收回调函数 */
    rt_device_set_rx_indicate(serial, uart_input);

    bc28_rx_thread = rt_thread_create("bc28_rx", serial_thread_entry, RT_NULL, 
                                  LED_THREAD_STACK_SIZE, 
                                  LED_THREAD_PRIORITY, 
                                  LED_THREAD_TIMESLICE);

    if(bc28_rx_thread) rt_thread_startup(bc28_rx_thread);

    /* 发送字符串 */
    rt_device_write(serial, 0, cmd, (sizeof(cmd) - 1));

    rt_thread_mdelay(5000);

    rt_thread_delete(bc28_rx_thread);
    rt_sem_detach(&rx_sem);
    rt_device_close(serial);
}
#ifdef FINSH_USING_MSH
MSH_CMD_EXPORT(cat_bc28, debug Quectel BC28);
#endif