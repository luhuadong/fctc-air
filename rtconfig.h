#ifndef RT_CONFIG_H__
#define RT_CONFIG_H__

/* Automatically generated file; DO NOT EDIT. */
/* RT-Thread Configuration */

/* RT-Thread Kernel */

#define RT_NAME_MAX 8
#define RT_ALIGN_SIZE 4
#define RT_THREAD_PRIORITY_32
#define RT_THREAD_PRIORITY_MAX 32
#define RT_TICK_PER_SECOND 1000
#define RT_USING_OVERFLOW_CHECK
#define RT_USING_HOOK
#define RT_USING_IDLE_HOOK
#define RT_IDLE_HOOK_LIST_SIZE 4
#define IDLE_THREAD_STACK_SIZE 256
#define RT_DEBUG
#define RT_DEBUG_COLOR

/* Inter-Thread communication */

#define RT_USING_SEMAPHORE
#define RT_USING_MUTEX
#define RT_USING_EVENT
#define RT_USING_MAILBOX
#define RT_USING_MESSAGEQUEUE

/* Memory Management */

#define RT_USING_MEMHEAP
#define RT_USING_SMALL_MEM
#define RT_USING_HEAP

/* Kernel Device Object */

#define RT_USING_DEVICE
#define RT_USING_DEVICE_OPS
#define RT_USING_CONSOLE
#define RT_CONSOLEBUF_SIZE 256
#define RT_CONSOLE_DEVICE_NAME "lpuart1"
#define RT_VER_NUM 0x40002
#define ARCH_ARM
#define RT_USING_CPU_FFS
#define ARCH_ARM_CORTEX_M
#define ARCH_ARM_CORTEX_M4

/* RT-Thread Components */

#define RT_USING_COMPONENTS_INIT
#define RT_USING_USER_MAIN
#define RT_MAIN_THREAD_STACK_SIZE 2048
#define RT_MAIN_THREAD_PRIORITY 10

/* C++ features */


/* Command shell */

#define RT_USING_FINSH
#define FINSH_THREAD_NAME "tshell"
#define FINSH_USING_HISTORY
#define FINSH_HISTORY_LINES 5
#define FINSH_USING_SYMTAB
#define FINSH_USING_DESCRIPTION
#define FINSH_THREAD_PRIORITY 20
#define FINSH_THREAD_STACK_SIZE 4096
#define FINSH_CMD_SIZE 80
#define FINSH_USING_MSH
#define FINSH_USING_MSH_DEFAULT
#define FINSH_USING_MSH_ONLY
#define FINSH_ARG_MAX 10

/* Device virtual file system */


/* Device Drivers */

#define RT_USING_DEVICE_IPC
#define RT_PIPE_BUFSZ 512
#define RT_USING_SYSTEM_WORKQUEUE
#define RT_SYSTEM_WORKQUEUE_STACKSIZE 2048
#define RT_SYSTEM_WORKQUEUE_PRIORITY 23
#define RT_USING_SERIAL
#define RT_SERIAL_RB_BUFSZ 64
#define RT_USING_I2C
#define RT_USING_I2C_BITOPS
#define RT_USING_PIN
#define RT_USING_ADC
#define RT_USING_SENSOR
#define RT_USING_SENSOR_CMD

/* Using USB */


/* POSIX layer and C standard library */

#define RT_USING_LIBC

/* Network */

/* Socket abstraction layer */

#define RT_USING_SAL

/* protocol stack implement */

#define SAL_USING_AT
#define SAL_SOCKETS_NUM 16

/* Network interface device */

#define RT_USING_NETDEV
#define NETDEV_USING_IFCONFIG
#define NETDEV_USING_PING
#define NETDEV_USING_NETSTAT
#define NETDEV_USING_AUTO_DEFAULT
#define NETDEV_IPV4 1
#define NETDEV_IPV6 0

/* light weight TCP/IP stack */


/* AT commands */

#define RT_USING_AT
#define AT_USING_CLIENT
#define AT_CLIENT_NUM_MAX 1
#define AT_USING_SOCKET
#define AT_USING_CLI
#define AT_CMD_MAX_LEN 256
#define AT_SW_VERSION_NUM 0x10300

/* VBUS(Virtual Software BUS) */


/* Utilities */

#define RT_USING_ULOG
#define ULOG_OUTPUT_LVL_D
#define ULOG_OUTPUT_LVL 7
#define ULOG_ASSERT_ENABLE
#define ULOG_LINE_BUF_SIZE 128

/* log format */

#define ULOG_USING_COLOR
#define ULOG_OUTPUT_TIME
#define ULOG_OUTPUT_LEVEL
#define ULOG_OUTPUT_TAG
#define ULOG_BACKEND_USING_CONSOLE

/* RT-Thread online packages */

/* IoT - internet of things */

#define PKG_USING_BC28_MQTT
#define PKG_USING_BC28_AT_CLIENT_DEV_NAME "uart3"
#define PKG_USING_BC28_MQTT_BAUD_RATE_9600
#define PKG_USING_BC28_MQTT_BAUD_RATE 9600
#define PKG_USING_BC28_RESET_PIN 5
#define PKG_USING_BC28_ADC0_PIN -1
#define PKG_USING_BC28_MQTT_PRODUCT_KEY "a1p8Pngb3oY"
#define PKG_USING_BC28_MQTT_DEVICE_NAME "BC28"
#define PKG_USING_BC28_MQTT_DEVICE_SECRET "miYe6iSBGKbYq71nhkd0cddVT2PSlPGs"
#define PKG_USING_BC28_MQTT_KEEP_ALIVE 300
#define PKG_USING_BC28_MQTT_LATEST_VERSION

/* Wi-Fi */

/* Marvell WiFi */


/* Wiced WiFi */

#define PKG_USING_AT_DEVICE
#define PKG_USING_AT_DEVICE_LATEST_VERSION
#define PKG_AT_DEVICE_VER_NUM 0x99999

/* IoT Cloud */


/* security packages */


/* language packages */


/* multimedia packages */


/* tools packages */


/* system packages */


/* peripheral libraries and drivers */

#define PKG_USING_SENSORS_DRIVERS
#define PKG_USING_DHTXX
#define PKG_USING_DHTXX_LATEST_VERSION
#define PKG_USING_GP2Y10
#define PKG_USING_GP2Y10_ADC_DEV_NAME "adc1"
#define PKG_USING_GP2Y10_ADC_DEV_CHANNEL 4
#define PKG_USING_GP2Y10_CONVERT_BITS 12
#define PKG_USING_GP2Y10_VOLTAGE_RATIO 11
#define PKG_USING_GP2Y10_SOFT_FILTER
#define PKG_USING_GP2Y10_LATEST_VERSION
#define PKG_USING_SGP30
#define PKG_USING_SGP30_LATEST_VERSION
#define PKG_USING_LITTLED
#define PKG_USING_LITTLED_PERIOD 1000
#define PKG_USING_LITTLED_PULSE 500
#define PKG_USING_LITTLED_BELL_TIME 50000
#define PKG_USING_LITTLED_BEEP_COUNT 3
#define PKG_USING_LITTLED_LATEST_VERSION

/* miscellaneous packages */


/* samples: kernel and components samples */

#define SOC_FAMILY_STM32
#define SOC_SERIES_STM32L4

/* Hardware Drivers Config */

#define SOC_STM32L4R5ZI

/* Onboard Peripheral Drivers */

#define BSP_USING_STLINK_TO_USART

/* On-chip Peripheral Drivers */

#define BSP_USING_GPIO
#define BSP_USING_UART
#define BSP_USING_LPUART1
#define BSP_USING_UART3
#define BSP_USING_ADC
#define BSP_USING_ADC1
#define BSP_USING_I2C
#define BSP_USING_I2C1
#define BSP_I2C1_SCL_PIN 24
#define BSP_I2C1_SDA_PIN 25

/* Board extended module Drivers */


#endif
