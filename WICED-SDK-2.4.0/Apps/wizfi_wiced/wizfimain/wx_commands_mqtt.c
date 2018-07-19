#include "wx_defines.h"
#include "wx_commands_mqtt.h"
#include "wiced_MQTT/MQTTConsole.h"
#include "wiced_platform.h"
#include "platform_common_config.h"


UINT8 WXCmd_MQTTSET(UINT8 *ptr)
{
	UINT8	*p;
	UINT8 	status;
	UINT8 	buff_mqtt_user[50] = {0,};
	UINT8 	buff_mqtt_password[50] = {0,};
	UINT8 	buff_mqtt_clientid[50] = {0,};
	UINT32 	buff_mqtt_alive;

	if ( strcmp((char*)ptr, "?")==0 )
	{
		W_RSP("%s,%s,%s,%d\r\n", g_wxProfile.mqtt_user, g_wxProfile.mqtt_password, g_wxProfile.mqtt_clientid, g_wxProfile.mqtt_alive);
		return WXCODE_SUCCESS;
	}

	p = WXParse_NextParamGet(&ptr);
	if (p)
	{
		if ( strlen((char*)p)==0 || strlen((char*)p) > sizeof(g_wxProfile.mqtt_user) ) return WXCODE_EINVAL;
		strcpy((char*)buff_mqtt_user, (char*)p);
	}

	p = WXParse_NextParamGet(&ptr);
	if (p)
	{
		if ( strlen((char*)p)==0 || strlen((char*)p) > sizeof(g_wxProfile.mqtt_password) ) return WXCODE_EINVAL;
		strcpy((char*)buff_mqtt_password, (char*)p);
	}

	p = WXParse_NextParamGet(&ptr);
	if (p)
	{
		if ( strlen((char*)p)==0 || strlen((char*)p) > sizeof(g_wxProfile.mqtt_clientid) ) return WXCODE_EINVAL;
		strcpy((char*)buff_mqtt_clientid, (char*)p);
	}

	p = WXParse_NextParamGet(&ptr);
	if (p)
	{
		status = WXParse_Int(p, &buff_mqtt_alive);
		if (status != WXCODE_SUCCESS )	return WXCODE_EINVAL;
		if (buff_mqtt_alive < 30 || buff_mqtt_alive > 300) return WXCODE_EINVAL;
	}

	//connection check
	if (g_mqtt_isconnected == 1)
		return WXCODE_FAILURE;

	if ( buff_mqtt_user[0] )		strcpy((char*)g_wxProfile.mqtt_user, (char*)buff_mqtt_user);
	if ( buff_mqtt_password[0] )	strcpy((char*)g_wxProfile.mqtt_password,(char*) buff_mqtt_password);
	if ( buff_mqtt_clientid[0] )	strcpy((char*)g_wxProfile.mqtt_clientid, (char*)buff_mqtt_clientid);
	g_wxProfile.mqtt_alive = buff_mqtt_alive;

	return WXCODE_SUCCESS;


	return WXCODE_SUCCESS;
}

UINT8 WXCmd_MQTTCON(UINT8 *ptr)
{
	UINT8 *p;
	UINT8 status;
	wiced_result_t 	ret = WXCODE_SUCCESS;
	UINT32 buff_mqtt_connect = 0;
	WT_IPADDR buff_mqtt_ip;
	UINT32 buff_mqtt_port = 0;
	UINT32 buff_mqtt_sslenable = 0;

	if ( strcmp((char*)ptr, "?")==0 )
	{
		if ( g_mqtt_isconnected == 1 && WXLink_IsWiFiLinked() )	W_RSP("Connected(%s)\r\n", (char*)g_wxProfile.mqtt_ip);
		else 		{	g_mqtt_isconnected = 0;		W_RSP("Disconnected\r\n"); }

		return WXCODE_SUCCESS;
	}

	if( !WXLink_IsWiFiLinked() )
		return WXCODE_WIFISTATUS_ERROR;

	p = WXParse_NextParamGet(&ptr);
	if (!p)		return WXCODE_EINVAL;
	status = WXParse_Int(p, &buff_mqtt_connect);
	if ( status != WXCODE_SUCCESS )	return WXCODE_EINVAL;
	if ( buff_mqtt_connect!=0 && buff_mqtt_connect!=1 ) return WXCODE_EINVAL;

	if ( buff_mqtt_connect==0 )
	{
		ret = MQTTStopService();
		if(ret != WICED_SUCCESS)
		{
			return WXCODE_FAILURE;
		}
		return WXCODE_SUCCESS;
	}

	p = WXParse_NextParamGet(&ptr);
	if (!p)		return WXCODE_EINVAL;
	status = WXParse_Ip(p, (uint8_t*)&buff_mqtt_ip);
	if (status != WXCODE_SUCCESS )	return WXCODE_EINVAL;

	p = WXParse_NextParamGet(&ptr);
	if (!p)		return WXCODE_EINVAL;
	status = WXParse_Int(p, &buff_mqtt_port);
	if ( status != WXCODE_SUCCESS )	return WXCODE_EINVAL;
	if ( !is_valid_port(buff_mqtt_port) ) return WXCODE_EINVAL;

	p = WXParse_NextParamGet(&ptr);
	if (!p)		return WXCODE_EINVAL;
	status = WXParse_Int(p, &buff_mqtt_sslenable);
	if ( status != WXCODE_SUCCESS )	return WXCODE_EINVAL;

	//daniel 160630 MQTTS NOT Support
	if ( buff_mqtt_sslenable!=0 )
			return WXCODE_EINVAL;
	/*
	if ( buff_mqtt_sslenable!=0 && buff_mqtt_sslenable!=1 )
		return WXCODE_EINVAL;
	*/
	if ( buff_mqtt_connect==1 )
	{
		int scid=0;
		for(scid=0; scid<WX_MAX_SCID_RANGE; scid++)
		{
			if ( g_scList[scid].tlsMode == 'S') return WXCODE_FAILURE;
		}

		sprintf((char*)g_wxProfile.mqtt_ip, "%d.%d.%d.%d", ((uint8_t*)&buff_mqtt_ip)[0], ((uint8_t*)&buff_mqtt_ip)[1], ((uint8_t*)&buff_mqtt_ip)[2], ((uint8_t*)&buff_mqtt_ip)[3]);
		g_wxProfile.mqtt_port = buff_mqtt_port;
		g_wxProfile.mqtt_sslenable = buff_mqtt_sslenable;

		ret = MQTTStartService();
		if(ret != WICED_SUCCESS)
		{
			return WXCODE_FAILURE;
		}

		g_mqtt_isconnected = 1;
	}

	return WXCODE_SUCCESS;
}


UINT8 WXCmd_MQTTPUB(UINT8 *ptr)
{
	UINT8 *p;
	UINT8 status;
	UINT32 buff_length = 0;
	UINT8 buff_pub_topic[LEN_MAX_TOPIC];
	UINT32 wait_time = 3000, left_time, start_time, cur_time;
	UINT8 retry_cnt;
	UINT32 tmp_length;
	wiced_result_t 	ret = WXCODE_SUCCESS;

	if (g_mqtt_isconnected != 1)
	{
		W_DBG("MQTT Disconnected : %d", g_mqtt_isconnected);
		return WXCODE_FAILURE;
	}

	p = WXParse_NextParamGet(&ptr);
	if (!p)		return WXCODE_EINVAL;
	if ( strlen((char*)p)==0 || strlen((char*)p) > sizeof(buff_pub_topic) ) return WXCODE_EINVAL;
	strcpy((char*)buff_pub_topic, (char*)p);

	p = WXParse_NextParamGet(&ptr);
	if ( !p )							return WXCODE_EINVAL;
	status = WXParse_Int(p, &buff_length);
	if ( buff_length<=0 || buff_length>sizeof(g_scTxBuffer) ) return WXCODE_EINVAL;
	if ( status != WXCODE_SUCCESS )		return WXCODE_EINVAL;

	W_RSP("[%d]\r\n", buff_length);

	wizfi_task_monitor_stop = 1;
	start_time = host_rtos_get_time();
	//daniel 160630
	memset(g_scTxBuffer,0,sizeof(g_scTxBuffer));

	while(buff_length > 0) {
		cur_time = host_rtos_get_time();
		if(wait_time > cur_time - start_time)
			left_time = wait_time - (cur_time - start_time);
		else left_time = 0;

		tmp_length = MIN(buff_length, 1400);
		ret = wiced_uart_receive_bytes(STDIO_UART, g_scTxBuffer, tmp_length, left_time);	//WICED_NEVER_TIMEOUT);
		if(ret == WICED_TIMEOUT) {
			W_DBG("WXCmd_MQTTPUB: Timeout\r\n");
			status = WXCODE_FAILURE;
			goto SSEND_EXIT;
		}

		g_scTxIndex = tmp_length;
		retry_cnt = 3;
		while(retry_cnt--) {
			ret = MQTTPublish((char*)buff_pub_topic, (char*)g_scTxBuffer, 0);

			cur_time = host_rtos_get_time();
			if(wait_time <= cur_time - start_time) {
				if(buff_length > tmp_length) ret = WXCODE_FAILURE;
				goto SSEND_EXIT;
			}
			if(ret == WXCODE_SUCCESS) break;
			W_DBG("WXCmd_MQTTPUB: Publish Error, try again\r\n");
			g_scTxIndex = tmp_length;
		}
		if(retry_cnt == 0) {
			W_DBG("WXCmd_MQTTPUB: Publish Error, fail\r\n");
			break;
		}

		buff_length -= tmp_length;
	}

SSEND_EXIT:
	wizfi_task_monitor_stop = 0;
	wiced_update_system_monitor(&wizfi_task_monitor_item, MAXIMUM_ALLOWED_INTERVAL_BETWEEN_WIZFIMAINTASK);

	return ret;
}

UINT8 WXCmd_MQTTSUB(UINT8 *ptr)
{
	UINT8 *p;
	wiced_result_t ret;
	UINT32 	mqtt_sub_state;
	UINT8 	mqtt_Sub_topic[LEN_MAX_TOPIC] = {0,};
	UINT32 	mqtt_sub_qos = 0;

	UINT8* 	t_Sub_topic;

	if( !WXLink_IsWiFiLinked() )
		return WXCODE_WIFISTATUS_ERROR;


	//subscribe & unsubscribe Topic
	p = WXParse_NextParamGet(&ptr);
	if (!p)		return WXCODE_EINVAL;
	ret = WXParse_Int(p, &mqtt_sub_state);
	if (ret != WXCODE_SUCCESS)	return WXCODE_EINVAL;

	//Get Topic
	p = WXParse_NextParamGet(&ptr);
	if (!p)		return WXCODE_EINVAL;
//	strcpy((char*)t_Sub_topic, (char*)p);
	t_Sub_topic = p;

	//Get Subscribe QOS
	p = WXParse_NextParamGet(&ptr);
	if ( p )
	{
		ret = WXParse_Int(p, &mqtt_sub_qos);
		if ( ret != WXCODE_SUCCESS )
			return WXCODE_EINVAL;
		if( mqtt_sub_qos < 0 && mqtt_sub_qos > 2 )
			return WXCODE_EINVAL;
	}

	if (mqtt_sub_state != 0 && mqtt_sub_state != 1 )		return WXCODE_EINVAL;

	strcpy((char*)mqtt_Sub_topic, (char*)t_Sub_topic);

	if ( mqtt_sub_state == 0 )
	{
		ret = MQTTUnsubscribe((char*)mqtt_Sub_topic);

		if (ret == WICED_SUCCESS)					return WXCODE_SUCCESS;
		else if (ret == WICED_ERROR)					return WXCODE_FAILURE;
	}

	else if( mqtt_sub_state == 1 )
	{
		ret = MQTTSubscribe((char*)mqtt_Sub_topic, mqtt_sub_qos);

		if (ret == WICED_SUCCESS)					return WXCODE_SUCCESS;
		else if (ret == WICED_ERROR)					return WXCODE_FAILURE;
	}

	return WXCODE_FAILURE;
}


/*
UINT8 WXCmd_MQTTStatus(UINT8 *ptr)
{
	UINT8 *p;
	UINT32 buff1 = 0;
	wiced_result_t ret;
	UINT8 *t_mqtt_ip;

	if ( strcmp((char*)ptr, "?") == 0 )
	{
		W_RSP("%s,%s,%s %d\r\n", g_mqtt_ClientID,g_mqtt_UserName,g_mqtt_Password,g_mqtt_AliveTime);
		return WXCODE_SUCCESS;
//		if (ret == MQTT_CONSOLE_FAILURE)					return WXCODE_FAILURE;
//		else if (ret == MQTT_CONSOLE_WIFI_DISCONNECTED)		return WXCODE_WIFISTATUS_ERROR;
	}




	p = WXParse_NextParamGet(&ptr);
	if ( !p )							return WXCODE_EINVAL;
	ret = WXParse_Int(p, &buff1);
	if ( ret != WXCODE_SUCCESS )		return WXCODE_EINVAL;

	p = WXParse_NextParamGet(&ptr);
	//if (!p)		return WXCODE_EINVAL;
//		strcpy((char*)t_mqtt_ip, (char*)p);
	t_mqtt_ip = p;

	strcpy((char*)g_mqtt_broker_ip, (char*)t_mqtt_ip);





	if(buff1 == 1)
	{
		if (g_mqtt_isconnected != 1)
			return WXCODE_FAILURE;

		MQTTShowTopic();
		return WXCODE_SUCCESS;
	}




//daniel 160301 gpio control
UINT8 WXCmd_MQTTGPIO(UINT8 *ptr)
{
	UINT8 *p;
	UINT32 t_gpio_enable = 0;
	UINT8*	t_gpio_topic;
	wiced_result_t ret;

	if ( strcmp((char*)ptr, "?") == 0 )
	{
		W_RSP("%d,%s\r\n", g_mqtt_gpio_enable, g_mqtt_gpio_topic);
		return WXCODE_SUCCESS;
	}


	p = WXParse_NextParamGet(&ptr);
	if ( !p )							return WXCODE_EINVAL;
	ret = WXParse_Int(p, &t_gpio_enable);
	if ( ret != WXCODE_SUCCESS )		return WXCODE_EINVAL;

	// 0 -> off, 1 -> on
	if( t_gpio_enable < 0 || t_gpio_enable > 1 )
		return WXCODE_EINVAL;

	g_mqtt_gpio_enable = t_gpio_enable;

	p = WXParse_NextParamGet(&ptr);
	if (p && !t_gpio_enable)			return WXCODE_EINVAL;
	t_gpio_topic = p;

	strncpy((char*)g_mqtt_gpio_topic, (char*)t_gpio_topic, LEN_MAX_TOPIC);

	return WXCODE_SUCCESS;

}


//daniel 160301 adc value return
UINT8 WXCmd_MQTTADC(UINT8 *ptr)
{
	UINT8 *p;
	UINT32 t_adc_enable = 0;
	UINT8*	t_adc_topic;
	wiced_result_t ret;

	if ( strcmp((char*)ptr, "?") == 0 )
	{
		W_RSP("%d,%s\r\n", g_mqtt_adc_enable, g_mqtt_adc_topic);
		return WXCODE_SUCCESS;
	}

	p = WXParse_NextParamGet(&ptr);
	if ( !p )							return WXCODE_EINVAL;
	ret = WXParse_Int(p, &t_adc_enable);
	if ( ret != WXCODE_SUCCESS )		return WXCODE_EINVAL;

	// 0 -> off, 1 -> on
	if( t_adc_enable < 0 || t_adc_enable > 1 )
		return WXCODE_EINVAL;

	g_mqtt_adc_enable = t_adc_enable;

	if(t_adc_enable == 0)
	{
		ret =  wiced_adc_deinit( WICED_ADC_3 );
		if(ret == WICED_SUCCESS) 		return WXCODE_SUCCESS;
		else							return WXCODE_FAILURE;
	}

	p = WXParse_NextParamGet(&ptr);
	if (p && !t_adc_enable)			return WXCODE_EINVAL;
	t_adc_topic = p;

	strncpy((char*)g_mqtt_adc_topic, (char*)t_adc_topic, LEN_MAX_TOPIC);

	if(t_adc_enable == 1)
	{
		ret =  wiced_adc_init( WICED_ADC_3, 5 );
		if(ret == WICED_SUCCESS) 		return WXCODE_SUCCESS;
		else							return WXCODE_FAILURE;
	}

	return WXCODE_FAILURE;

}

*/
