#include "../wizfimain/wx_defines.h"
#include "wiced_mqtt/mqtt_api.h"

#ifndef APPS_WIZFI_WICED_MQTT_MQTTCONSOLE_H_
#define APPS_WIZFI_WICED_MQTT_MQTTCONSOLE_H_

wiced_result_t MQTTStartService();
wiced_result_t MQTTStopService();
wiced_result_t MQTTPublish(const char* Topic, const char* argv, int qos);
wiced_result_t MQTTSubscribe(const char* Topic, int qos);
wiced_result_t MQTTUnsubscribe(const char* Topic);
wiced_result_t MQTTShowTopic();
wiced_result_t MQTTClearResource();

wiced_result_t MQTTSUBSCRIBE_TOPICLIST();

enum g_MQTTService {MQTT_SERVICE_DISCONNECTED = 0, MQTT_SERVICE_CONNECTED};
enum g_reconnection_step { MQTT_STEP_NONE = 0, MQTT_STEP_CONNECTED, MQTT_STEP_LINKDOWN, MQTT_STEP_LINKUP_RECONNECT };


#define TIMEOUT_CONNECT 10  //connectnetwork½Ã Å¸ÀÓ¾Æ¿ô(s)
#define TIMEOUT_DEFAULT 5000 // ms
#define TIME_KEEPALIVE 30

#define LEN_MAX_TOPIC 128
#define LEN_MAX_MSG 128

#define LEN_MAX_IP 30
#define LEN_MAX_CID 50
#define LEN_MAX_USERNAME 50
#define LEN_MAX_PASSWORD 70

#define SIZE_MAX_BUF 256
extern int		g_mqtt_isconnected;

//daniel 160301 gpio control
extern char  	g_mqtt_gpio_topic[LEN_MAX_TOPIC];
extern int 		g_mqtt_gpio_enable;

extern char  	g_mqtt_adc_topic[LEN_MAX_TOPIC];
extern int 		g_mqtt_adc_enable;


#endif /* APPS_WIZFI_WICED_MQTT_MQTTCONSOLE_H_ */
