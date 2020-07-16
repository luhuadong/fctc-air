#ifndef __ALI_MQTT_H__
#define __ALI_MQTT_H__

#include <dev_sign_api.h>
#include <mqtt_api.h>

void *ali_mqtt_create(void);
int   example_subscribe(void *handle);
int   example_publish(void *handle, char *payload);

#endif /* __ALI_MQTT_H__ */