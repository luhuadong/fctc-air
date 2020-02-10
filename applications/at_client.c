/*
 * Copyright (c) 2020, RudyLo <luhuadong@163.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-02-09     luhuadong    the first version
 */

//#include <stdlib.h>
#include <string.h>

#include <rtthread.h>
#include <rtdevice.h>
//#include <board.h>
#include <at.h>

#define LOG_TAG              "at.sample"
#include <at_log.h>

/* AT+CIFSR            Query local IP address and MAC */
int at_client_test(int argc, char **argv)
{
    at_response_t resp = RT_NULL;
    int result = 0;

    if (argc != 1)
    {
        LOG_E("at_client_test  - AT client send commands to AT server.");
        return -1;
    }

    resp = at_create_resp(256, 0, rt_tick_from_millisecond(5000));
    if (resp == RT_NULL)
    {
        LOG_E("No memory for response structure!");
        return -2;
    }

    /* close echo */
    at_exec_cmd(resp, "ATE0");

    result = at_exec_cmd(resp, "AT+CIFSR");
    if (result != RT_EOK)
    {
        LOG_E("AT client send commands failed or return response error!");
        goto __exit;
    }

    /* Print response line buffer */
    {
        const char *line_buffer = RT_NULL;

        LOG_D("Response buffer");

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

    {
        char resp_arg[AT_CMD_MAX_LEN] = { 0 };
        const char * resp_expr = "%*[^\"]\"%[^\"]\"";

        LOG_D(" Parse arguments");
        if (at_resp_parse_line_args(resp, 1, resp_expr, resp_arg) == 1)
        {
            LOG_D("Station IP  : %s", resp_arg);
            memset(resp_arg, 0x00, AT_CMD_MAX_LEN);
        }
        else
        {
            LOG_E("Parse error, current line buff : %s", at_resp_get_line(resp, 4));
        }

        if (at_resp_parse_line_args(resp, 2, resp_expr, resp_arg) == 1)
        {
            LOG_D("Station MAC : %s", resp_arg);
        }
        else
        {
            LOG_E("Parse error, current line buff : %s", at_resp_get_line(resp, 5));
            goto __exit;
        }
    }
__exit:
    if(resp)
    {
        at_delete_resp(resp);
    }

    return result;
}

int at_client_test_init(int argc, char **argv)
{
#define AT_CLIENT_RECV_BUFF_LEN         512

    if (argc > 2)
    {
        rt_kprintf("-- AT client initialize.\nUsage: at_client_init <dev_name>\n");
        return -RT_ERROR;
    }

    /* setup uart */
    rt_device_t serial;

    if (argc == 2)
    	serial = rt_device_find(argv[1]);
    else
    	serial = rt_device_find("uart3");

    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;

    config.baud_rate = BAUD_RATE_9600;        //修改波特率为 9600
    config.data_bits = DATA_BITS_8;           //数据位 8
    config.stop_bits = STOP_BITS_1;           //停止位 1
    config.bufsz     = 128;                   //修改缓冲区 buff size 为 128
    config.parity    = PARITY_NONE;           //无奇偶校验位

    rt_device_control(serial, RT_DEVICE_CTRL_CONFIG, &config);
    rt_device_close(serial);

    if (argc == 2)
    	at_client_init(argv[1], AT_CLIENT_RECV_BUFF_LEN);
    else
    	at_client_init("uart3", AT_CLIENT_RECV_BUFF_LEN);

    return RT_EOK;
}

#ifdef FINSH_USING_MSH
//#include <finsh.h>
MSH_CMD_EXPORT(at_client_test, AT client send cmd and get response);
MSH_CMD_EXPORT_ALIAS(at_client_test_init, at_client_init, initialize AT client);
#endif