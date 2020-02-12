/*
 * Copyright (c) 2020, RudyLo <luhuadong@163.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-02-09     luhuadong    the first version
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

#define AT_CLIENT_DEV_NAME        "uart3"
#define AT_DEFAULT_TIMEOUT        5000

#define PRODUCT_KEY               "a1p8Pngb3oY"
#define DEVICE_NAME               "BC28"
#define DEVICE_SECRET             "miYe6iSBGKbYq71nhkd0cddVT2PSlPGs"

#define AT_MQTT_AUTH              "AT+QMTCFG=\"aliauth\",0,\"%s\",\"%s\",\"%s\""
#define AT_MQTT_ALIVE             "AT+QMTCFG=\"keepalive\",0,%u"
#define AT_MQTT_OPEN              "AT+QMTOPEN=0,\"%s.iot-as-mqtt.cn-shanghai.aliyuncs.com\",1883"
#define AT_MQTT_OPEN_SUCC         "+QMTOPEN: 0,0"
#define AT_MQTT_CLOSE             "AT+QMTCLOSE=0"
#define AT_MQTT_CONNECT           "AT+QMTCONN=0,\"%s\""
#define AT_MQTT_CONNECT_SUCC      "+QMTCONN: 0,0,0"
#define AT_MQTT_DISCONNECT        "AT+QMTDISC=0"
#define AT_MQTT_SUB               "AT+QMTSUB=0,1,\"%s\",0"
#define AT_MQTT_SUB_SUCC          "+QMTSUB: 0,1,0,1"
#define AT_MQTT_UNSUB             "AT+QMTUNS=0,1, \"%s\""
#define AT_MQTT_PUB               "AT+QMTPUB=0,0,0,0,\"%s\""
#define AT_MQTT_PUB_SUCC          "+QMTPUB: 0,0,0"
#define JSON_DATA_PACK            "{\"id\":\"125\",\"version\":\"1.0\",\"params\":{\"Temp\":%f,\"Humi\":%f,\"Dust\":%d,\"TVOC\":%d,\"eCO2\":%d},\"method\":\"thing.event.property.post\"}"

#define AT_TEST                   "AT"
#define AT_ECHO_OFF               "ATE0"
#define AT_AUTOCONNECT_DISABLE    "AT+NCONFIG=AUTOCONNECT,FALSE"
#define AT_REBOOT                 "AT+NRB"
#define AT_NBAND_B5               "AT+NBAND=5"
#define AT_CFUN_1                 "AT+CFUN=1"
#define AT_QUERY_IMSI             "AT+CIMI"
#define AT_UE_ATTACH              "AT+CGATT=1"
#define AT_QUERY_STATUS           "AT+NUESTATS"
#define AT_QUERY_REG              "AT+CEREG?"
#define AT_QUERY_IPADDR           "AT+CGPADDR"
#define AT_QUERY_ATTACH           "AT+CGATT?"

#define AT_CLIENT_RECV_BUFF_LEN   512

//int check_send_cmd(int argc, char **argv)
static int check_send_cmd(const char* cmd, const char* resp_expr, unsigned int timeout)
{
	at_response_t resp = RT_NULL;
	int result = 0;

	resp = at_create_resp(256, 0, rt_tick_from_millisecond(timeout));
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
    	LOG_E("# Failed");
        return -RT_ERROR;
    }

    LOG_E("# successed");
    at_delete_resp(resp);
    return RT_EOK;
}


int at_client_attach_auto(void)
{
	return 0;
}

int at_client_test_attach(void)
{
	int result = 0;

    /* close echo */
    check_send_cmd("ATE0", "OK", AT_DEFAULT_TIMEOUT);

    /* 禁用自动连接网络 */
    result = check_send_cmd("AT+NCONFIG=AUTOCONNECT,FALSE", "OK", AT_DEFAULT_TIMEOUT);
    if (result != RT_EOK)
    {
        LOG_E("[AT+NCONFIG=AUTOCONNECT,FALSE] not the expected result");
        return result;
    }

    /* 重启模块 */
    result = check_send_cmd("AT+NRB", "OK", 20000);
    if (result != RT_EOK)
    {
        LOG_E("[AT+NRB] not the expected result");
        //return result;
    }

    while(RT_EOK != check_send_cmd("AT", "OK", AT_DEFAULT_TIMEOUT))
    {
    	rt_thread_delay(1000);
    }

    /* 查询IMEI号 */
    result = check_send_cmd("AT+CGSN=1", "OK", AT_DEFAULT_TIMEOUT);
    if (result != RT_EOK)
    {
        LOG_E("[AT+CGSN=1] not the expected result");
        return result;
    }

    /* 指定要搜索的频段 B8 */
    result = check_send_cmd("AT+NBAND=8", "OK", AT_DEFAULT_TIMEOUT);
    if (result != RT_EOK)
    {
        LOG_E("[AT+NBAND=8] not the expected result");
        return result;
    }

    /* 打开模块的调试灯 */
    result = check_send_cmd("AT+QLEDMODE=1", "OK", AT_DEFAULT_TIMEOUT);
    if (result != RT_EOK)
    {
        LOG_E("[AT+QLEDMODE=1] not the expected result");
        return result;
    }

    /* 将模块设置为全功能模式(开启射频功能) */
    result = check_send_cmd("AT+CFUN=1", "OK", AT_DEFAULT_TIMEOUT);
    if (result != RT_EOK)
    {
        LOG_E("[AT+CFUN=1] not the expected result");
        return result;
    }

    /* 接收到TCP数据时，自动上报 */
    result = check_send_cmd("AT+NSONMI=2", "OK", AT_DEFAULT_TIMEOUT);
    if (result != RT_EOK)
    {
        LOG_E("[AT+NSONMI=2] not the expected result");
        return result;
    }

    /* 关闭eDRX */
    result = check_send_cmd("AT+CEDRXS=0,5", "OK", AT_DEFAULT_TIMEOUT);
    if (result != RT_EOK)
    {
        LOG_E("[AT+CEDRXS=0,5] not the expected result");
        return result;
    }

    /* 关闭PSM */
    result = check_send_cmd("AT+CPSMS=0", "OK", AT_DEFAULT_TIMEOUT);
    if (result != RT_EOK)
    {
        LOG_E("[AT+CPSMS=0] not the expected result");
        return result;
    }

    /* 查询卡的国际识别码(IMSI号)，用于确认SIM卡插入正常 */
    result = check_send_cmd("AT+CIMI", "OK", AT_DEFAULT_TIMEOUT);
    if (result != RT_EOK)
    {
        LOG_E("[AT+CIMI] not the expected result");
        return result;
    }

    /* 触发网络连接 */
    result = check_send_cmd("AT+CGATT=1", "OK", AT_DEFAULT_TIMEOUT);
    if (result != RT_EOK)
    {
        LOG_E("[AT+CGATT=1] not the expected result");
        return result;
    }

    /* 查询模块状态 */
    //at_exec_cmd(resp, "AT+NUESTATS");

    /* 查询网络注册状态 */
    //at_exec_cmd(resp, "AT+CEREG?");

    /* 查看信号强度 */
    //at_exec_cmd(resp, "AT+CSQ");

    /* 查询网络是否被激活，通常需要等待30s */
#if 0
    result = check_send_cmd("AT+CGATT?", "+CGATT:1", 60000);
    if (result != RT_EOK)
    {
        LOG_E("[AT+CGATT?] not the expected result");
        return result;
    }
#else
    while(RT_EOK != check_send_cmd("AT+CGATT?", "+CGATT:1", 60000))
    {
    	rt_thread_delay(1000);
    }
#endif

    /* 查询模块的 IP 地址 */
    //at_exec_cmd(resp, "AT+CGPADDR");

    LOG_D("AT attach successed");
    return RT_EOK;
}

int at_client_mqtt_config(void)
{
    char cmd[256] = {0};
    rt_sprintf(cmd, "AT+QMTCFG=\"aliauth\",0,\"%s\",\"%s\",\"%s\"", PRODUCT_KEY, DEVICE_NAME, DEVICE_SECRET);

    return check_send_cmd(cmd, "OK", AT_DEFAULT_TIMEOUT);
}

int at_client_mqtt_open(void)
{
    char cmd[128] = {0};
    rt_sprintf(cmd, "AT+QMTOPEN=0,\"%s.iot-as-mqtt.cn-shanghai.aliyuncs.com\",1883", PRODUCT_KEY);

    return check_send_cmd(cmd, "+QMTOPEN: 0,0", 75000);
}

int at_client_mqtt_close(void)
{
    return check_send_cmd("AT+QMTCLOSE=0", "OK", AT_DEFAULT_TIMEOUT);
}

int at_client_mqtt_connect(void)
{
    return check_send_cmd("AT+QMTCONN=0,\"867726037265602\"", "+QMTCONN: 0,0,0", 60000);
}

int at_client_mqtt_disconnect(void)
{
    return check_send_cmd("AT+QMTDISC=0", "OK", AT_DEFAULT_TIMEOUT);
}

//int at_client_mqtt_subscribe(const char *topic)
int at_client_mqtt_subscribe(void)
{
    //char cmd[128] = {0};
    //rt_sprintf(cmd, "AT+QMTSUB=0,1,\"%s\",0", topic);

    return check_send_cmd("AT+QMTSUB=0,1,\"/a1p8Pngb3oY/BC28/user/hello\",0", "+QMTSUB: 0,1,0,1", AT_DEFAULT_TIMEOUT);
}

//int at_client_mqtt_publish(const char *topic, const char *msg)
int at_client_mqtt_publish(void)
{
    //char cmd[128] = {0};
    //rt_sprintf(cmd, "AT+QMTPUB=0,0,0,0,\"%s\"", topic);
    char *msg = "RudyLo\x1A";
#if 0
    if (RT_EOK == check_send_cmd("AT+QMTPUB=0,0,0,0,\"/a1p8Pngb3oY/BC28/user/hello\"", ">", AT_DEFAULT_TIMEOUT))
    {
    	LOG_D("go...");
    	return check_send_cmd(msg, "+QMTPUB: 0,0,0", AT_DEFAULT_TIMEOUT);
    }
#else
    check_send_cmd("AT+QMTPUB=0,0,0,0,\"/a1p8Pngb3oY/BC28/user/hello\"", ">", AT_DEFAULT_TIMEOUT);
    LOG_D("go...");
    return check_send_cmd(msg, "+QMTPUB: 0,0,0", AT_DEFAULT_TIMEOUT);
#endif
    //return -RT_ERROR;
}

int at_client_mqtt_upload(void)
{
    //char cmd[128] = {0};
    //rt_sprintf(cmd, "AT+QMTPUB=0,0,0,0,\"%s\"", topic);
    char *msg = "{\"id\":\"125\",\"version\":\"1.0\",\"params\":{\"Temp\":{\"value\":99.9,},\"Humi\":{\"value\":99.5,},\"Dust\":{\"value\":21,},\"TVOC\":{\"value\":43,},\"eCO2\":{\"value\": 476,},},\"method\":\"thing.event.property.post\"}\x1A";
#if 0
    if (RT_EOK == check_send_cmd("AT+QMTPUB=0,0,0,0,\"/sys/a1p8Pngb3oY/BC28/thing/event/property/post\"", ">", AT_DEFAULT_TIMEOUT))
    {
    	LOG_D("go...");
    	return check_send_cmd(msg, "+QMTPUB: 0,0,0", AT_DEFAULT_TIMEOUT);
    }
#else
    check_send_cmd("AT+QMTPUB=0,0,0,0,\"/sys/a1p8Pngb3oY/BC28/thing/event/property/post\"", ">", AT_DEFAULT_TIMEOUT);
    LOG_D("go...");
    return check_send_cmd(msg, "+QMTPUB: 0,0,0", AT_DEFAULT_TIMEOUT);
#endif
    //return -RT_ERROR;
}

int at_client_test_init(int argc, char **argv)
{
    if (argc > 2)
    {
        rt_kprintf("-- AT client initialize.\nUsage: at_client_init <dev_name>\n");
        return -RT_ERROR;
    }

    char *at_dev = RT_NULL;
    if (argc == 2)
    	at_dev = argv[1];
    else
    	at_dev = AT_CLIENT_DEV_NAME;

    /* setup uart */
    rt_device_t serial = rt_device_find(at_dev);

    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;

    config.baud_rate = BAUD_RATE_9600;
    config.data_bits = DATA_BITS_8;
    config.stop_bits = STOP_BITS_1;
    config.bufsz     = 128;
    config.parity    = PARITY_NONE;

    rt_device_control(serial, RT_DEVICE_CTRL_CONFIG, &config);
    rt_device_close(serial);

    at_client_init(at_dev, AT_CLIENT_RECV_BUFF_LEN);

    return RT_EOK;
}

#if 0
//#ifdef FINSH_USING_MSH
MSH_CMD_EXPORT(at_client_mqtt_config, AT client MQTT);
MSH_CMD_EXPORT(at_client_mqtt_open, AT client MQTT);
MSH_CMD_EXPORT(at_client_mqtt_close, AT client MQTT);
MSH_CMD_EXPORT(at_client_mqtt_connect, AT client MQTT);
MSH_CMD_EXPORT(at_client_mqtt_disconnect, AT client MQTT);
MSH_CMD_EXPORT(at_client_mqtt_subscribe, AT client MQTT);
MSH_CMD_EXPORT(at_client_mqtt_publish, AT client MQTT);
MSH_CMD_EXPORT(at_client_mqtt_upload, AT client MQTT);

MSH_CMD_EXPORT(check_send_cmd, AT client send cmd and check response);
MSH_CMD_EXPORT(at_client_test_attach, AT client attach to access network);
MSH_CMD_EXPORT_ALIAS(at_client_test_init, at_client_init, initialize AT client);
#endif
