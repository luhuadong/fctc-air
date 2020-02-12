/*
 * Copyright (c) 2020, RudyLo <luhuadong@163.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-02-07     luhuadong    the first version
 */

#ifndef __FCTC_AIR_H__
#define __FCTC_AIR_H__

#define LED1_PIN                 GET_PIN(C, 7)   /* defined the LED1 pin: PC7 */
#define LED2_PIN                 GET_PIN(B, 7)   /* defined the LED2 pin: PB7 */
#define LED3_PIN                 GET_PIN(B, 14)  /* defined the LED3 pin: PB14 */
#define LED_RUNNING              LED1_PIN
#define LED_PAUSE                LED2_PIN
#define LED_WARNING              LED3_PIN

#define JSON_DATA_PACK           "{\"id\":\"125\",\"version\":\"1.0\",\"params\":{\"Temp\":%f,\"Humi\":%f,\"Dust\":%d,\"TVOC\":%d,\"eCO2\":%d},\"method\":\"thing.event.property.post\"}\x1A"

#define MQTT_TOPIC_HELLO         "/a1p8Pngb3oY/BC28/user/hello"
#define MQTT_TOPIC_UPLOAD        "/sys/a1p8Pngb3oY/BC28/thing/event/property/post"

extern rt_bool_t is_paused;

void user_btn_init(void);


/* NB-IoT */
int at_client_dev_init(void);
int at_client_attach(void);

int bc28_mqtt_auth(void);
int bc28_mqtt_open(void);
int bc28_mqtt_close(void);
int bc28_mqtt_connect(void);
int bc28_mqtt_disconnect(void);
int bc28_mqtt_subscribe(const char *topic);
int bc28_mqtt_unsubscribe(const char *topic);
int bc28_mqtt_publish(const char *topic, const char *msg);

#endif /* __FCTC_AIR_H__ */
