/*
 * mqtt_console.c

 *
 *  Created on: 2016. 2. 1.
 *      Author: Administrator
 */
#include "MQTTConsole.h"

/******************************************************
 *               Variable Definitions
 ******************************************************/

int		g_mqtt_thread_is_running = 0;

int	 	g_mqtt_isconnected = 0;
int 	g_mqtt_reconnection = MQTT_STEP_NONE;

//daniel 160301 gpio control
char  	g_mqtt_gpio_topic[LEN_MAX_TOPIC] = {0, };
int 	g_mqtt_gpio_enable = 0;
char 	g_mqtt_gpio_high_string[] = "high";
char 	g_mqtt_gpio_low_string[] = "low";

//daniel 160301 adc return

char  	g_mqtt_adc_topic[LEN_MAX_TOPIC] = {0, };
int 	g_mqtt_adc_enable = 0;
char 	g_mqtt_adc_return_string[] = "adc";
char 	g_mqtt_adc_value[6] = {0, };
int		g_mqtt_adc_request = 0;




struct TopicList
{
	char topic[LEN_MAX_TOPIC];
	int qos;
}g_topic_list[5];



/******************************************************
 *               Function Declarations
 ******************************************************/
static wiced_result_t mqtt_connection_event_cb( wiced_mqtt_object_t mqtt_object, wiced_mqtt_event_info_t *event );
static wiced_result_t mqtt_wait_for( wiced_mqtt_event_type_t event, uint32_t timeout );
static wiced_result_t mqtt_conn_open( wiced_mqtt_object_t mqtt_obj, wiced_ip_address_t *address, wiced_interface_t interface, wiced_mqtt_callback_t callback, int ssl_enable );
static wiced_result_t mqtt_conn_close( wiced_mqtt_object_t mqtt_object );
static wiced_result_t mqtt_app_subscribe( wiced_mqtt_object_t mqtt_obj, char *topic, uint8_t qos );
static wiced_result_t mqtt_app_unsubscribe( wiced_mqtt_object_t mqtt_obj, char *topic );
static wiced_result_t mqtt_app_publish( wiced_mqtt_object_t mqtt_obj, uint8_t qos, uint8_t *topic, uint8_t *data, uint32_t data_len );

wiced_mqtt_object_t mqtt_object;

wiced_mqtt_callback_t connection_callbacks = mqtt_connection_event_cb;
static wiced_mqtt_event_type_t expected_event;
static wiced_semaphore_t semaphore;

wiced_result_t MQTTStartService()
{

	wiced_result_t ret = WICED_ERROR;
	if( g_mqtt_isconnected == MQTT_SERVICE_CONNECTED )
	{
		W_DBG("[MQTT ALREADY CONNECT]");
		return ret;
	}

	if( g_mqtt_thread_is_running == 0)
	{
		/* Memory allocated for mqtt object*/
		mqtt_object = (wiced_mqtt_object_t) malloc( WICED_MQTT_OBJECT_MEMORY_SIZE_REQUIREMENT );
		if ( mqtt_object == NULL )
		{
			W_DBG("Dont have memory to allocate for mqtt object...\n");
			return ret;
		}

		ret = wiced_mqtt_init( mqtt_object );
		if(ret != WICED_SUCCESS)
			return ret;
		W_DBG("Thread running..");
		g_mqtt_thread_is_running = 1;
	}

	wiced_ip_address_t INITIALISER_IPV4_ADDRESS( mqtt_broker_ip, str_to_ip((char*)(char*)g_wxProfile.mqtt_ip));

	W_DBG("TryMQTTConnect : Connecting to broker %u.%u.%u.%u ...", (uint8_t)(GET_IPV4_ADDRESS(mqtt_broker_ip) >> 24),
					(uint8_t)(GET_IPV4_ADDRESS(mqtt_broker_ip) >> 16),
					(uint8_t)(GET_IPV4_ADDRESS(mqtt_broker_ip) >> 8),
					(uint8_t)(GET_IPV4_ADDRESS(mqtt_broker_ip) >> 0));

	ret = mqtt_conn_open( mqtt_object, &mqtt_broker_ip, WICED_STA_INTERFACE, connection_callbacks, g_wxProfile.mqtt_sslenable );

	if( ret != WICED_SUCCESS )
	{
		W_DBG("TryMQTTConnect : mqtt_conn_open error");
		MQTTClearResource();
		return ret;
	}

	W_RSP("[MQTT CONNECT]\r\n");
	g_mqtt_isconnected = MQTT_SERVICE_CONNECTED;
	//for reconnect
	g_mqtt_reconnection = MQTT_STEP_CONNECTED;

	return ret;
}

/*
wiced_result_t MQTTSUBSCRIBE_TOPICLIST()
{

	wiced_result_t ret = WICED_ERROR;
	int i, listed_topic_cnt = 0;

	W_RSP("SUBSCRIBE ALL TOPIC LIST....\r\n");
	for(i=0; i<5; i++)
	{
		W_DBG("(%d), %s", i,  g_topic_list[i]);

		if(strcmp(g_topic_list[i].topic, ""))
		{
			W_DBG("TEST 101");
			listed_topic_cnt++;
			ret = mqtt_app_subscribe( mqtt_object, g_topic_list[i].topic, g_topic_list[i].qos);
			if(ret != WICED_SUCCESS)
			{
				MQTTClearResource();
				wiced_rtos_delay_milliseconds(500);
				return WICED_ERROR;
			}
		}
	}
	W_RSP("( %d ) TOPIC SUBSCRIBED...\r\n", listed_topic_cnt);

	return ret;
}

*/
wiced_result_t MQTTStopService()
{
	wiced_result_t ret = WICED_ERROR;

	if( g_mqtt_isconnected != MQTT_SERVICE_CONNECTED )
	{
		W_DBG("[MQTT ALREADY DISCONNECTED]");
		return ret;
	}

	//MQTTTopicList(MQTT_TOPIC_DELETE_ALL, NULL, 0);

	ret = mqtt_conn_close( mqtt_object );

	if( ret == WICED_SUCCESS )
	{
		if( MQTTClearResource() == WICED_SUCCESS )
			W_RSP("[MQTT DISCONNECT]\r\n");
	}
	else
		g_mqtt_isconnected = MQTT_SERVICE_DISCONNECTED;

	g_mqtt_reconnection = MQTT_STEP_NONE;

	return ret;
}


wiced_result_t MQTTPublish(const char* Topic, const char* args, int qos)
{
	wiced_result_t ret = WICED_ERROR;

	if( g_mqtt_isconnected != MQTT_SERVICE_CONNECTED )
		return ret;


	ret =  mqtt_app_publish( mqtt_object, qos, (uint8_t*)Topic, (uint8_t*)args ,strlen(args) );
	if(ret != WICED_SUCCESS)
	{
		MQTTClearResource();
		wiced_rtos_delay_milliseconds(500);
		return WICED_ERROR;
	}

//	W_RSP("[MQTT Published: %s, \"%s\"]\r\n", (char*)Topic, (char*)args);
	return ret;

}

/*
//process, 0=delete all topic, 1=delete topic, 2=add topic  3=print topic list
wiced_result_t MQTTTopicList(int process, const char* Topic, int qos)
{

	W_DBG("process: %d, Topic: %s, qos: %d", process, (char*)Topic, qos);
	return WICED_ERROR;

	if( process == MQTT_TOPIC_DELETE_ALL)
	{
		int i;
		for(i=0; i<5; i++)
		{
			memset(g_topic_list[i].topic, 0, sizeof(g_topic_list[i].topic));
			g_topic_list[i].qos = 0;
		}
		W_DBG("Clear Topic list");
		return WICED_SUCCESS;
	}

	else if( process == MQTT_TOPIC_DELETE )
	{
		int i;
		for(i=0; i<5; i++)
		{
			if(!strcmp(g_topic_list[i].topic,(char*)Topic))
			{
				memset(g_topic_list[i].topic, 0, sizeof(g_topic_list[i].topic));
				g_topic_list[i].qos = qos;
				break;
			}
		}
		W_DBG("DELETE Topic(%d), %s", i, g_topic_list[i].topic);
		return WICED_SUCCESS;
	}

	else if( process == MQTT_TOPIC_ADD )
	{
		int i;
		//check
		for(i=0; i<5; i++)
		{
			if(!strcmp(g_topic_list[i].topic, (char*)Topic))
			{
				W_DBG("ALEADY added Topic(%d), %s", i, g_topic_list[i].topic);
				return WICED_SUCCESS;
			}
		}

		for(i=0; i<5; i++)
		{
			if(!strcmp(g_topic_list[i].topic, ""))
			{
				strcpy(g_topic_list[i].topic,(char*)Topic);
				g_topic_list[i].qos = qos;
				break;
			}
		}
		W_DBG("ADD Topic(%d), %s", i, g_topic_list[i].topic);

		return WICED_SUCCESS;

	}

	else if( process == MQTT_TOPIC_PRINT )
	{
		int i;
		W_RSP("MQTT TOPIC LIST\r\n");
		W_RSP("num / qos / Topic\r\n");
		for(i=0; i<5; i++)
		{
			if(strcmp(g_topic_list[i].topic, ""))
				W_RSP(" %d  /  %d  /  %s\r\n", i, g_topic_list[i].qos, g_topic_list[i].topic);
		}
		W_DBG("=========================");
		return WICED_SUCCESS;
	}


	return WICED_ERROR;
}
*/

wiced_result_t MQTTSubscribe(const char* Topic, int qos)
{
	wiced_result_t ret = WICED_ERROR;

	if( g_mqtt_isconnected != MQTT_SERVICE_CONNECTED )
		return ret;

	ret = mqtt_app_subscribe( mqtt_object, (char*)Topic , qos );
	if(ret != WICED_SUCCESS)
	{
		MQTTClearResource();
		wiced_rtos_delay_milliseconds(500);
		//MQTTStartService((char*)g_mqtt_broker_ip, (int)g_mqtt_broker_port, (int)g_mqtt_broker_ssl_enable);
		return WICED_ERROR;
	}

	//MQTTTopicList(MQTT_TOPIC_ADD, Topic, qos);
	//W_RSP("[MQTT Subscribe: %s]\r\n", (char*)Topic);
	return ret;
}

wiced_result_t MQTTUnsubscribe(const char* Topic)
{
	wiced_result_t ret = WICED_ERROR;

	if( g_mqtt_isconnected != MQTT_SERVICE_CONNECTED )
		return ret;

	ret =  mqtt_app_unsubscribe( mqtt_object, (char*)Topic );
	if(ret != WICED_SUCCESS)
	{

		MQTTClearResource();
		wiced_rtos_delay_milliseconds(500);
		return WICED_ERROR;
	}

	//MQTTTopicList(MQTT_TOPIC_DELETE, Topic, 0);
	//W_RSP("[MQTT UnSubscribe: %s]\r\n", (char*)Topic);
	return ret;
}


wiced_result_t MQTTShowTopic()
{
	wiced_result_t ret = WICED_ERROR;

	//ret = MQTTTopicList(MQTT_TOPIC_PRINT, NULL, 0);

	return ret;

}


wiced_result_t MQTTClearResource()
{

	wiced_result_t ret = WICED_ERROR;
	ret = wiced_mqtt_deinit( mqtt_object );
    g_mqtt_thread_is_running = 0;
    g_mqtt_isconnected = MQTT_SERVICE_DISCONNECTED;
    free( mqtt_object );
    mqtt_object = NULL;

    return ret;
}
/*
wiced_result_t MQTTForcedTerminate()
{

	wiced_result_t ret = WICED_ERROR;
	//MQTTClearSocket();
	ret = wiced_mqtt_deinit( mqtt_object );
	W_DBG("mqtt deinit %d", ret);
    g_mqtt_thread_is_running = 0;
    g_mqtt_isconnected = MQTT_SERVICE_DISCONNECTED;
    extern mqtt_connection_t *g_p_mqtt_connection;
    ret = wiced_mqtt_deinit( (wiced_mqtt_object_t)g_p_mqtt_connection );
    W_DBG("wiced_mqtt_deinit %d", ret);
//    mqtt_core_deinit((mqtt_connection_t*)mqtt_object);
//    free(g_p_mqtt_connection);
   // g_p_mqtt_connection = NULL;
    free( mqtt_object );

    W_DBG("Clear MQTT terminate -2");
    WDUMP(mqtt_object, 16);

    mqtt_object = NULL;
    return ret;
}
*/



void adc_publish(uint32_t arg)
{

	wiced_rtos_delay_milliseconds(500);

    wiced_mqtt_msgid_t pktid;

    pktid = wiced_mqtt_publish( mqtt_object, (uint8_t*)g_mqtt_adc_topic, (uint8_t*)g_mqtt_adc_value, strlen(g_mqtt_adc_value), 0 );

    if ( pktid == 0 )
    {
        W_DBG("WICED_ERROR");
    }

}





/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * Copyright 2015, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */
/*
 * Call back function to handle connection events.
 */
wiced_result_t mqtt_connection_event_cb( wiced_mqtt_object_t mqtt_object, wiced_mqtt_event_info_t *event )
{
	//daniel 160301 gpio control
	//UINT8 t_gpio[6] = {0,};
	UINT16 adc_value = 0;
	//wiced_result_t ret = WICED_ERROR;
	sprintf(g_mqtt_adc_value,"%d",adc_value);		//clear

	switch ( event->type )
    {

        case WICED_MQTT_EVENT_TYPE_CONNECT_REQ_STATUS:
        case WICED_MQTT_EVENT_TYPE_DISCONNECTED:
        case WICED_MQTT_EVENT_TYPE_PUBLISHED:
        case WICED_MQTT_EVENT_TYPE_SUBCRIBED:
        case WICED_MQTT_EVENT_TYPE_UNSUBSCRIBED:
        {
            expected_event = event->type;
            wiced_rtos_set_semaphore( &semaphore );
        }
            break;
        case WICED_MQTT_EVENT_TYPE_PUBLISH_MSG_RECEIVED:
        {
            wiced_mqtt_topic_msg_t msg = event->data.pub_recvd;

            //daniel 160630 modify original source for Message format
            W_RSP("{Q,%.*s,,%d}%.*s\r\n",(int)msg.topic_len, msg.topic, (int)msg.data_len, (int)msg.data_len, msg.data);

//            memset(&msg,0,sizeof(msg));

//            W_RSP("{%.*s,%d} %.*s\r\n",(int)msg.topic_len, msg.topic, (int)msg.data_len, (int)msg.data_len, msg.data);
//            WPRINT_APP_INFO(( "[MQTT] Received %.*s  for TOPIC : %.*s\n\n", (int) msg.data_len, msg.data, (int) msg.topic_len, msg.topic ));
/*
            W_DBG("END CB-1");
            //daniel 160301 gpio control
            if(g_mqtt_gpio_enable == 0 && g_mqtt_adc_enable == 0)
            	break;

            if(g_mqtt_gpio_enable == 1)
            {
            	UINT8 WXCmd_FGPIO(UINT8 *ptr);
            	//check topic
            	if(strncmp((char*)g_mqtt_gpio_topic, (char*)msg.topic,  (int)msg.topic_len) == 0)
            	{
            		//check data
            		if(strncmp(g_mqtt_gpio_high_string, (char*)msg.data, strlen(g_mqtt_gpio_high_string)) == 0)
            		{
            			sprintf((char*)t_gpio,"1,1,1");
            			ret = WXCmd_FGPIO(t_gpio);
            			if( ret == WXCODE_SUCCESS )			W_DBG("GPIO HIGH");
            			else								W_DBG("GPIO control error");
            		}
            		else if(strncmp(g_mqtt_gpio_low_string, (char*)msg.data, strlen(g_mqtt_gpio_low_string)) == 0)
            		{
            			sprintf((char*)t_gpio,"1,1,0");
						ret = WXCmd_FGPIO(t_gpio);
						if( ret == WXCODE_SUCCESS )			W_DBG("GPIO LOW");
						else								W_DBG("GPIO control error");
            		}
            	}
            }
            W_DBG("END CB");
            ///////////////////////////////////////

            //daniel 160301 adc return
            if( g_mqtt_adc_enable == 1)
            {
            	//check topic
            	if(strncmp((char*)g_mqtt_adc_topic, (char*)msg.topic,  (int)msg.topic_len) == 0)
            	{
            		//check data
            		if(strncmp(g_mqtt_adc_return_string, (char*)msg.data, strlen(g_mqtt_adc_return_string)) == 0)
            		{
            			ret = wiced_adc_take_sample(WICED_ADC_3, &adc_value);
            			if( ret != WXCODE_SUCCESS )			W_DBG("ADC get error");
            			if( ret == WXCODE_SUCCESS )
            			{
            				sprintf(g_mqtt_adc_value,"%d",adc_value);
            				W_DBG("%s", g_mqtt_adc_value);

            				g_mqtt_adc_request = 1;


            			}
            		}
            	}
            }
            */
            ////////////////////////////////////////////////
        }
            break;
        default:
            break;
    }
    return WICED_SUCCESS;
}

/*
 * Call back function to handle channel events.
 *
 * For each event:
 *  - The call back will set the expected_event global to the received event.
 *  - The call back will set the event semaphore to run any blocking thread functions waiting on this event
 *  - Some events will also log other global variables required for extra processing.
 *
 * A thread function will probably be waiting for the received event. Once the event is received and the
 * semaphore is set, the thread function will check for the received event and make sure it matches what
 * it is expecting.
 *
 * Note:  This mechanism is not thread safe as we are using a non protected global variable for read/write.
 * However as this snip is a single controlled thread, there is no risc of racing conditions. It is
 * however not recommended for multi-threaded applications.
 */

/*
 * A blocking call to an expected event.
 */
static wiced_result_t mqtt_wait_for( wiced_mqtt_event_type_t event, uint32_t timeout )
{
    if ( wiced_rtos_get_semaphore( &semaphore, timeout ) != WICED_SUCCESS )
    {
        return WICED_ERROR;
    }
    else
    {
        if ( event != expected_event )
        {
            return WICED_ERROR;
        }
    }
    return WICED_SUCCESS;
}

/*
 * Open a connection and wait for TIMEOUT_DEFAULT period to receive a connection open OK event
 */
//static wiced_result_t mqtt_conn_open( wiced_mqtt_object_t mqtt_obj, wiced_ip_address_t *address, wiced_interface_t interface, wiced_mqtt_callback_t callback, wiced_mqtt_security_t *security )
static wiced_result_t mqtt_conn_open( wiced_mqtt_object_t mqtt_obj, wiced_ip_address_t *address, wiced_interface_t interface, wiced_mqtt_callback_t callback, int ssl_enable )
{
    wiced_mqtt_pkt_connect_t conninfo;
    wiced_result_t ret = WICED_SUCCESS;
    //daniel 160228 for security
    wiced_mqtt_security_t *security = NULL;

    memset( &conninfo, 0, sizeof( conninfo ) );

    conninfo.port_number = (uint16_t)g_wxProfile.mqtt_port;                   /* set to 0 indicates library to use default settings */
    conninfo.mqtt_version = WICED_MQTT_PROTOCOL_VER4;
    conninfo.clean_session = 1;
    conninfo.client_id = (uint8_t*)g_wxProfile.mqtt_clientid;
    conninfo.keep_alive = (uint16_t)g_wxProfile.mqtt_alive;
    conninfo.password = (uint8_t*)g_wxProfile.mqtt_password;
    conninfo.username = (uint8_t*)g_wxProfile.mqtt_user;

    W_DBG("[Client ID: %s]\r\n[User Name: %s]\r\n[Password: %s]", conninfo.client_id, conninfo.username, conninfo.password);

    //// for security.
    if(ssl_enable == 1)
    {
//    	security.ca_cert =
//    	security.cert =
//    	security.key =
    	W_DBG("[MQTT SSL Enabled]");
    }


    ret = wiced_mqtt_connect( mqtt_obj, address, interface, callback, security, &conninfo );
//    ret = wiced_mqtt_connect( mqtt_obj, address, interface, callback, security, &conninfo );
   //////////////////////////////////////////////////////

    if ( ret != WICED_SUCCESS )
    {
        return WICED_ERROR;
    }


    if ( mqtt_wait_for( WICED_MQTT_EVENT_TYPE_CONNECT_REQ_STATUS, TIMEOUT_DEFAULT ) != WICED_SUCCESS )
    {
        return WICED_ERROR;
    }

    return WICED_SUCCESS;
}

/*
 * Close a connection and wait for 5 seconds to receive a connection close OK event
 */
static wiced_result_t mqtt_conn_close( wiced_mqtt_object_t mqtt_obj )
{
    if ( wiced_mqtt_disconnect( mqtt_obj ) != WICED_SUCCESS )
    {
        return WICED_ERROR;
    }
    if ( mqtt_wait_for( WICED_MQTT_EVENT_TYPE_DISCONNECTED, TIMEOUT_DEFAULT ) != WICED_SUCCESS )
    {
        return WICED_ERROR;
    }
    return WICED_SUCCESS;
}

/*
 * Subscribe to WICED_TOPIC and wait for 5 seconds to receive an ACM.
 */
static wiced_result_t mqtt_app_subscribe( wiced_mqtt_object_t mqtt_obj, char *topic, uint8_t qos )
{
    wiced_mqtt_msgid_t pktid;
    pktid = wiced_mqtt_subscribe( mqtt_obj, topic, qos );
    if ( pktid == 0 )
    {
        return WICED_ERROR;
    }
    if ( mqtt_wait_for( WICED_MQTT_EVENT_TYPE_SUBCRIBED, TIMEOUT_DEFAULT ) != WICED_SUCCESS )
    {
        return WICED_ERROR;
    }
    return WICED_SUCCESS;
}

/*
 * Unsubscribe from WICED_TOPIC and wait for 10 seconds to receive an ACM.
 */
static wiced_result_t mqtt_app_unsubscribe( wiced_mqtt_object_t mqtt_obj, char *topic )
{
    wiced_mqtt_msgid_t pktid;
    pktid = wiced_mqtt_unsubscribe( mqtt_obj, topic );

    if ( pktid == 0 )
    {
        return WICED_ERROR;
    }
    if ( mqtt_wait_for( WICED_MQTT_EVENT_TYPE_UNSUBSCRIBED, TIMEOUT_DEFAULT*2 ) != WICED_SUCCESS )
    {
        return WICED_ERROR;
    }
    return WICED_SUCCESS;
}

/*
 * Publish (send) WICED_MESSAGE_STR to WICED_TOPIC and wait for 5 seconds to receive a PUBCOMP (as it is QoS=2).
 */
static wiced_result_t mqtt_app_publish( wiced_mqtt_object_t mqtt_obj, uint8_t qos, uint8_t *topic, uint8_t *data, uint32_t data_len )
{
    wiced_mqtt_msgid_t pktid;
    //W_DBG("%d, %s, %s, %d", qos, topic, data, data_len);
    pktid = wiced_mqtt_publish( mqtt_obj, topic, data, data_len, qos );
    if ( pktid == 0 )
    {
        return WICED_ERROR;
    }
    if ( mqtt_wait_for( WICED_MQTT_EVENT_TYPE_PUBLISHED, 20000 ) != WICED_SUCCESS )
//    if ( mqtt_wait_for( WICED_MQTT_EVENT_TYPE_PUBLISHED, TIMEOUT_DEFAULT ) != WICED_SUCCESS )
    {
        return WICED_ERROR;
    }
    return WICED_SUCCESS;
}


