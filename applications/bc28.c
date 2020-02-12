/*
 * Copyright (c) 2020, RudyLo <luhuadong@163.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-02-07     luhuadong    the first version
 */

#include <stdlib.h>
#include <string.h>

#include <rtthread.h>
#include <rtdevice.h>
//#include <board.h>
#include <at.h>

#define LOG_TAG                   "at.bc28"
#define LOG_LVL                   LOG_LVL_DBG
#include <at_log.h>

#include "fctc_air.h"

#define AT_CLIENT_DEV_NAME        "uart3"
#define AT_DEFAULT_TIMEOUT        5000

#define PRODUCT_KEY               "a1p8Pngb3oY"
#define DEVICE_NAME               "BC28"
#define DEVICE_SECRET             "miYe6iSBGKbYq71nhkd0cddVT2PSlPGs"

#define AT_OK                     "OK"
#define AT_ERROR                  "ERROR"

#define AT_TEST                   "AT"
#define AT_ECHO_OFF               "ATE0"
#define AT_AUTOCONNECT_DISABLE    "AT+NCONFIG=AUTOCONNECT,FALSE"
#define AT_REBOOT                 "AT+NRB"
#define AT_NBAND_B8               "AT+NBAND=8"
#define AT_FUN_ON                 "AT+CFUN=1"
#define AT_LED_ON                 "AT+QLEDMODE=1"
#define AT_EDRX_OFF               "AT+CEDRXS=0,5"
#define AT_PSM_OFF                "AT+CPSMS=0"
#define AT_RECV_AUTO              "AT+NSONMI=2"
#define AT_UE_ATTACH              "AT+CGATT=1"
#define AT_QUERY_IMEI             "AT+CGSN=1"
#define AT_QUERY_IMSI             "AT+CIMI"
#define AT_QUERY_STATUS           "AT+NUESTATS"
#define AT_QUERY_REG              "AT+CEREG?"
#define AT_QUERY_IPADDR           "AT+CGPADDR"
#define AT_QUERY_ATTACH           "AT+CGATT?"
#define AT_UE_ATTACH_SUCC         "+CGATT:1"

#define AT_MQTT_AUTH              "AT+QMTCFG=\"aliauth\",0,\"%s\",\"%s\",\"%s\""
#define AT_MQTT_ALIVE             "AT+QMTCFG=\"keepalive\",0,%u"
#define AT_MQTT_OPEN              "AT+QMTOPEN=0,\"%s.iot-as-mqtt.cn-shanghai.aliyuncs.com\",1883"
#define AT_MQTT_OPEN_SUCC         "+QMTOPEN: 0,0"
#define AT_MQTT_CLOSE             "AT+QMTCLOSE=0"
//#define AT_MQTT_CONNECT           "AT+QMTCONN=0,\"%s\""
#define AT_MQTT_CONNECT           "AT+QMTCONN=0,\"867726037265602\""
#define AT_MQTT_CONNECT_SUCC      "+QMTCONN: 0,0,0"
#define AT_MQTT_DISCONNECT        "AT+QMTDISC=0"
#define AT_MQTT_SUB               "AT+QMTSUB=0,1,\"%s\",0"
#define AT_MQTT_SUB_SUCC          "+QMTSUB: 0,1,0,1"
#define AT_MQTT_UNSUB             "AT+QMTUNS=0,1, \"%s\""
#define AT_MQTT_PUB               "AT+QMTPUB=0,0,0,0,\"%s\""
#define AT_MQTT_PUB_SUCC          "+QMTPUB: 0,0,0"

#define AT_QMTSTAT_CLOSED             1
#define AT_QMTSTAT_PINGREQ_TIMEOUT    2
#define AT_QMTSTAT_CONNECT_TIMEOUT    3
#define AT_QMTSTAT_CONNACK_TIMEOUT    4
#define AT_QMTSTAT_DISCONNECT         5
#define AT_QMTSTAT_WORNG_CLOSE        6
#define AT_QMTSTAT_INACTIVATED        7

#define AT_QMTRECV_DATA           "+QMTRECV: %d,%d,\"%s\",\"%s\""

#define AT_CLIENT_RECV_BUFF_LEN   256

#if 0
#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>

#include "fctc_air.h"

#define LED_THREAD_PRIORITY      20
#define LED_THREAD_STACK_SIZE    512
#define LED_THREAD_TIMESLICE     15

#define SAMPLE_UART_NAME       "uart3"    /* 串口设备名称 */
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
static void cat_bc28(int argc, char **argv)
{
    char cmd[64] = {0};

    if (argc < 2) {
        rt_sprintf(cmd, "AT\r\n");
    }
    else {
        rt_sprintf(cmd, "%s\r\n", argv[1]);
    }

    rt_kprintf("hello Quectel BC28 ==> %s\n", cmd);

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
    rt_device_write(serial, 0, cmd, rt_strlen(cmd));

    rt_thread_mdelay(5000);

    rt_thread_delete(bc28_rx_thread);
    rt_sem_detach(&rx_sem);
    rt_device_close(serial);
}
#ifdef FINSH_USING_MSH
MSH_CMD_EXPORT(cat_bc28, Usage: cat_bc28 <AT command>);
#endif

#endif

static void urc_stat_func(struct at_client *client, const char *data, rt_size_t size)
{
    /* WIFI 连接成功信息 */
    LOG_D("AT Server device WIFI connect success!");
}

static void urc_recv_func(struct at_client *client, const char *data, rt_size_t size)
{
    /* 接收到服务器发送数据 */
    LOG_D("AT Client receive AT Server data!");
}

static struct at_urc urc_table[] = {
    { "+QMTSTAT", ":", urc_stat_func },
    { "+QMTRECV", ":", urc_recv_func },
};

int at_client_port_init(void)
{
    /* 添加多种 URC 数据至 URC 列表中，当接收到同时匹配 URC 前缀和后缀的数据，执行 URC 函数  */
    at_set_urc_table(urc_table, sizeof(urc_table) / sizeof(urc_table[0]));
    return RT_EOK;
}

//int check_send_cmd(int argc, char **argv)
static int check_send_cmd(const char* cmd, const char* resp_expr, const rt_size_t lines, const rt_int32_t timeout)
{
    at_response_t resp = RT_NULL;
    int result = 0;

    resp = at_create_resp(AT_CLIENT_RECV_BUFF_LEN, lines, rt_tick_from_millisecond(timeout));
    if (resp == RT_NULL)
    {
        rt_kprintf("No memory for response structure!");
        return -RT_ENOMEM;
    }

    result = at_exec_cmd(resp, cmd);
    if (result != RT_EOK)
    {
        LOG_E("AT client send commands failed or return response error!");
        at_delete_resp(resp);
        return result;
    }

    /* Print response line buffer */
    {
        const char *line_buffer = RT_NULL;

        rt_kprintf("Response buffer");

        for(rt_size_t line_num = 1; line_num <= resp->line_counts; line_num++)
        {
            if((line_buffer = at_resp_get_line(resp, line_num)) != RT_NULL)
            {
                LOG_D("line %d buffer : %s", line_num, line_buffer);
            }
            else
            {
                LOG_E("Parse line buffer error!");
            }
        }
    }

    char resp_arg[AT_CMD_MAX_LEN] = { 0 };
    if (at_resp_parse_line_args_by_kw(resp, resp_expr, "%s", resp_arg) <= 0)
    {
        at_delete_resp(resp);
        LOG_E("# >_< Failed\n");
        return -RT_ERROR;
    }

    LOG_E("# ^_^ successed\n");
    at_delete_resp(resp);
    return RT_EOK;
}

int bc28_mqtt_auth(void)
{
    char cmd[AT_CMD_MAX_LEN] = {0};
    rt_sprintf(cmd, AT_MQTT_AUTH, PRODUCT_KEY, DEVICE_NAME, DEVICE_SECRET);

    return check_send_cmd(cmd, AT_OK, 0, AT_DEFAULT_TIMEOUT);
}

int bc28_mqtt_open(void)
{
    char cmd[AT_CMD_MAX_LEN] = {0};
    rt_sprintf(cmd, AT_MQTT_OPEN, PRODUCT_KEY);

    return check_send_cmd(cmd, AT_MQTT_OPEN_SUCC, 4, 75000);
}

int bc28_mqtt_close(void)
{
    return check_send_cmd(AT_MQTT_CLOSE, AT_OK, 0, AT_DEFAULT_TIMEOUT);
}

int bc28_mqtt_connect(void)
{
    return check_send_cmd(AT_MQTT_CONNECT, AT_MQTT_CONNECT_SUCC, 4, 10000);
}

int bc28_mqtt_disconnect(void)
{
    return check_send_cmd(AT_MQTT_DISCONNECT, AT_OK, 0, AT_DEFAULT_TIMEOUT);
}

int bc28_mqtt_subscribe(const char *topic)
{
    char cmd[AT_CMD_MAX_LEN] = {0};
    rt_sprintf(cmd, AT_MQTT_SUB, topic);

    return check_send_cmd(cmd, AT_MQTT_SUB_SUCC, 4, AT_DEFAULT_TIMEOUT);
}

int bc28_mqtt_unsubscribe(const char *topic)
{
    char cmd[AT_CMD_MAX_LEN] = {0};
    rt_sprintf(cmd, AT_MQTT_UNSUB, topic);

    return check_send_cmd(cmd, AT_OK, 0, AT_DEFAULT_TIMEOUT);
}

int bc28_mqtt_publish(const char *topic, const char *msg)
{
    char cmd[AT_CMD_MAX_LEN] = {0};
    rt_sprintf(cmd, AT_MQTT_PUB, topic);

    check_send_cmd(cmd, ">", 0, AT_DEFAULT_TIMEOUT);
    LOG_D("go...");

    return check_send_cmd(msg, AT_MQTT_PUB_SUCC, 0, AT_DEFAULT_TIMEOUT);
}

int bc28_mqtt_upload(int argc, char **argv)
{
    if(argc != 6) {
        LOG_E("Usage: bc28_mqtt_upload <temp> <humi> <dust> <tvoc> <eco2>");
        return -RT_ERROR;
    }

    int result = 0;
    char json_pack[256] = {0};

    int temp = atoi(argv[1]);
    int humi = atoi(argv[2]);
    int dust = atoi(argv[3]);
    int tvoc = atoi(argv[4]);
    int eco2 = atoi(argv[5]);

    rt_sprintf(json_pack, JSON_DATA_PACK, temp, humi, dust, tvoc, eco2);

    result = bc28_mqtt_publish(MQTT_TOPIC_UPLOAD, json_pack);

    if(result == RT_EOK)
        LOG_D("Upload OK");
    else
        LOG_D("Upload Error");

    return result;
}

int at_client_attach(void)
{
    int result = 0;

    /* close echo */
    check_send_cmd(AT_ECHO_OFF, AT_OK, 0, AT_DEFAULT_TIMEOUT);

    /* 禁用自动连接网络 */
    result = check_send_cmd(AT_AUTOCONNECT_DISABLE, AT_OK, 0, AT_DEFAULT_TIMEOUT);
    if (result != RT_EOK) return result;

    /* 重启模块 */
    check_send_cmd(AT_REBOOT, AT_OK, 0, 10000);

    while(RT_EOK != check_send_cmd(AT_TEST, AT_OK, 0, AT_DEFAULT_TIMEOUT))
    {
        rt_thread_delay(1000);
    }

    /* 查询IMEI号 */
    result = check_send_cmd(AT_QUERY_IMEI, AT_OK, 0, AT_DEFAULT_TIMEOUT);
    if (result != RT_EOK) return result;

    /* 指定要搜索的频段 B8 */
    result = check_send_cmd(AT_NBAND_B8, AT_OK, 0, AT_DEFAULT_TIMEOUT);
    if (result != RT_EOK) return result;

    /* 打开模块的调试灯 */
    result = check_send_cmd(AT_LED_ON, AT_OK, 0, AT_DEFAULT_TIMEOUT);
    if (result != RT_EOK) return result;

    /* 将模块设置为全功能模式(开启射频功能) */
    result = check_send_cmd(AT_FUN_ON, AT_OK, 0, AT_DEFAULT_TIMEOUT);
    if (result != RT_EOK) return result;

    /* 接收到TCP数据时，自动上报 */
    result = check_send_cmd(AT_RECV_AUTO, AT_OK, 0, AT_DEFAULT_TIMEOUT);
    if (result != RT_EOK) return result;

    /* 关闭eDRX */
    result = check_send_cmd(AT_EDRX_OFF, AT_OK, 0, AT_DEFAULT_TIMEOUT);
    if (result != RT_EOK) return result;

    /* 关闭PSM */
    result = check_send_cmd(AT_PSM_OFF, AT_OK, 0, AT_DEFAULT_TIMEOUT);
    if (result != RT_EOK) return result;

    /* 查询卡的国际识别码(IMSI号)，用于确认SIM卡插入正常 */
    result = check_send_cmd(AT_QUERY_IMSI, AT_OK, 0, AT_DEFAULT_TIMEOUT);
    if (result != RT_EOK) return result;

    /* 触发网络连接 */
    result = check_send_cmd(AT_UE_ATTACH, AT_OK, 0, AT_DEFAULT_TIMEOUT);
    if (result != RT_EOK) return result;

    /* 查询模块状态 */
    //at_exec_cmd(resp, "AT+NUESTATS");

    /* 查询网络注册状态 */
    //at_exec_cmd(resp, "AT+CEREG?");

    /* 查看信号强度 */
    //at_exec_cmd(resp, "AT+CSQ");

    /* 查询网络是否被激活，通常需要等待30s */
    int count = 60;
    while(count > 0 && RT_EOK != check_send_cmd(AT_QUERY_ATTACH, AT_UE_ATTACH_SUCC, 0, AT_DEFAULT_TIMEOUT))
    {
        rt_thread_delay(1000);
        count--;
    }

    /* 查询模块的 IP 地址 */
    //at_exec_cmd(resp, "AT+CGPADDR");

    if (count > 0) 
        return RT_EOK;
    else 
        return -RT_ETIMEOUT;
}

/**
 * AT client initialize.
 *
 * @return 0 : initialize success
 *        -1 : initialize failed
 *        -5 : no memory
 */
int at_client_dev_init(void)
{
    rt_device_t serial = rt_device_find(AT_CLIENT_DEV_NAME);
    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;

    config.baud_rate = BAUD_RATE_9600;
    config.data_bits = DATA_BITS_8;
    config.stop_bits = STOP_BITS_1;
    config.bufsz     = AT_CLIENT_RECV_BUFF_LEN;
    config.parity    = PARITY_NONE;

    rt_device_control(serial, RT_DEVICE_CTRL_CONFIG, &config);
    rt_device_close(serial);

    return at_client_init(AT_CLIENT_DEV_NAME, AT_CLIENT_RECV_BUFF_LEN);
}

#ifdef FINSH_USING_MSH
MSH_CMD_EXPORT(bc28_mqtt_auth,        AT client MQTT auth);
MSH_CMD_EXPORT(bc28_mqtt_open,        AT client MQTT open);
MSH_CMD_EXPORT(bc28_mqtt_close,       AT client MQTT close);
MSH_CMD_EXPORT(bc28_mqtt_connect,     AT client MQTT connect);
MSH_CMD_EXPORT(bc28_mqtt_disconnect,  AT client MQTT disconnect);
MSH_CMD_EXPORT(bc28_mqtt_subscribe,   AT client MQTT subscribe);
MSH_CMD_EXPORT(bc28_mqtt_unsubscribe, AT client MQTT unsubscribe);
MSH_CMD_EXPORT(bc28_mqtt_publish,     AT client MQTT publish);
MSH_CMD_EXPORT(bc28_mqtt_upload,      AT client MQTT upload);

MSH_CMD_EXPORT(at_client_attach, AT client attach to access network);
MSH_CMD_EXPORT_ALIAS(at_client_dev_init, at_client_init, initialize AT client);
#endif
