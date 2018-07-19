#include "wx_defines.h"
#include "wiced_network.h"
#include "wiced_tcpip.h"
#include "wiced.h"
#include "wx_types.h"
#include "http_server.h"
#include "resources.h"
#include "wiced_rtos.h"
#include "wiced_management.h"
#include "wx_commands_f.h"

wiced_ip_setting_t device_config_ip_settings;

#if 1
static const configuration_entry_t app_config[]={};
#else													// kaizen 20130510 Not Used (Reserved Function)
static const configuration_entry_t app_config[] =
{
		{"msgLevel",		DCT_OFFSET(WT_PROFILE, msgLevel), 		1, 					CONFIG_UINT8_DATA 	},
		{"wifi_authtype",	DCT_OFFSET(WT_PROFILE, wifi_authtype),	1,					CONFIG_UINT32_DATA	},
		{"wifi_keydata",	DCT_OFFSET(WT_PROFILE, wifi_keydata),	64,					CONFIG_STRING_DATA	},
		{0,0,0,0}
};
#endif


static wiced_thread_t 	http_thread;
static wiced_thread_t*  p_http_thread = NULL;
UINT8 web_server_status = STOP_WEB_SERVER;

#define PING_TIMEOUT 900

UINT8 WXCmd_FPING(UINT8 *ptr)
{
	UINT8				*p;
	UINT8				status;
	UINT8				dummy_ip[WX_IPv4_ADDR_SIZE];
	UINT32				loop_cnt;
	wiced_ip_address_t 	ping_target_ip;


	p = WXParse_NextParamGet(&ptr);
	if (!p)		return WXCODE_EINVAL;
	status = WXParse_Int(p, &loop_cnt);

#if 1	// kaizen 20131010 limited ping count ( max : 50 )
	if( loop_cnt > 50 )
		return WXCODE_EINVAL;
#endif

	p = WXParse_NextParamGet(&ptr);
	if (!p)		return WXCODE_EINVAL;

	// For verify IP address
	status = WXParse_Ip(p, (UINT8*)&dummy_ip);
	if (status != WXCODE_SUCCESS)	return WXCODE_EINVAL;

	SET_IPV4_ADDRESS( ping_target_ip, str_to_ip((char*)p) );
	Send_Ping( loop_cnt, &ping_target_ip, (wiced_interface_t)g_wxProfile.wifi_mode );

	return WXCODE_SUCCESS;
}

void Send_Ping(UINT32 loop_cnt, wiced_ip_address_t *ping_target_ip, wiced_interface_t interface )
{
	UINT8				i;
	uint32_t			elapsed_ms;
	wiced_result_t		wiced_status;


	for(i=0; i<loop_cnt; i++)
	{
		wiced_status = wiced_ping( interface, ping_target_ip, PING_TIMEOUT, &elapsed_ms );

		if ( wiced_status == WICED_SUCCESS )
			W_RSP("Ping Reply : %3lu ms\r\n", (uint32_t)elapsed_ms );

		else if ( wiced_status == WICED_TIMEOUT )
			W_RSP("Ping timeout\r\n");

		else
			W_RSP("Ping error\r\n");
	}
}


UINT8 WXCmd_FDNS(UINT8 *ptr)
{
	UINT8 	*p;
	UINT8 	status;
	UINT8 	hostname[256];
	UINT32 	timeout_ms;
	wiced_ip_address_t ip_address;

	p = WXParse_NextParamGet(&ptr);
	if (!p)		return WXCODE_EINVAL;
	strncpy((char*)hostname,(char*)p,256);

	p = WXParse_NextParamGet(&ptr);
	if (!p)		return WXCODE_EINVAL;
	status = WXParse_Int(p, &timeout_ms);
	if (status != WXCODE_SUCCESS)	return WXCODE_EINVAL;

#if 1 //MikeJ 130408 ID1025 - When DNS failed, return error
	if(wiced_hostname_lookup((char*)hostname, &ip_address, timeout_ms ) != WICED_SUCCESS)
		return WXCODE_FAILURE;
#else
	wiced_hostname_lookup((char*)hostname, &ip_address, timeout_ms );
#endif

    W_RSP("%u.%u.%u.%u\r\n",
            (unsigned char) ( ( ( GET_IPV4_ADDRESS( ip_address ) ) >> 24 ) & 0xff ),
            (unsigned char) ( ( ( GET_IPV4_ADDRESS( ip_address ) ) >> 16 ) & 0xff ),
            (unsigned char) ( ( ( GET_IPV4_ADDRESS( ip_address ) ) >> 8 ) & 0xff ),
            (unsigned char) ( ( ( GET_IPV4_ADDRESS( ip_address ) ) >> 0 ) & 0xff ));

	return WXCODE_SUCCESS;
}


void ap_sta_mode_webserver_thread()
{
	if( g_wxProfile.wifi_mode == STATION_MODE )
	{
		W_DBG("Start Web Server in Station Mode");
		if( g_wxProfile.wifi_dhcp == 1 )
		{
			wiced_configure_device_start(app_config,WICED_STA_INTERFACE,WICED_USE_EXTERNAL_DHCP_SERVER, NULL);
		}
		else
		{
			SET_IPV4_ADDRESS(device_config_ip_settings.ip_address, str_to_ip((char*)g_wxProfile.wifi_ip));
			SET_IPV4_ADDRESS(device_config_ip_settings.netmask, str_to_ip((char*)g_wxProfile.wifi_mask));
			SET_IPV4_ADDRESS(device_config_ip_settings.gateway, str_to_ip((char*)g_wxProfile.wifi_gateway));

			wiced_configure_device_start(app_config,WICED_STA_INTERFACE, WICED_USE_STATIC_IP, &device_config_ip_settings);
		}
	}
	else if( g_wxProfile.wifi_mode == AP_MODE )
	{
		SET_IPV4_ADDRESS(device_config_ip_settings.ip_address, str_to_ip((char*)g_wxProfile.wifi_ip));
		SET_IPV4_ADDRESS(device_config_ip_settings.netmask, str_to_ip((char*)g_wxProfile.wifi_mask));
		SET_IPV4_ADDRESS(device_config_ip_settings.gateway, str_to_ip((char*)g_wxProfile.wifi_gateway));

		W_DBG("Start Web Server in AP Mode");
		wiced_configure_device_start(app_config,WICED_AP_INTERFACE,WICED_USE_INTERNAL_DHCP_SERVER,&device_config_ip_settings);
	}

	W_DBG("Stop Web Server");
	web_server_status = STOP_WEB_SERVER;
	p_http_thread = NULL;
}

UINT8 WXCmd_FWEBS(UINT8 *ptr)
{
	UINT8				*p;
	UINT8				status;
	UINT32				web_server_cmd;
	wiced_interface_t	interface = WICED_STA_INTERFACE;

	if ( strcmp((char*)ptr, "?") == 0 )
	{
		W_RSP("%d,%s\r\n",web_server_status, g_wxProfile.web_server_opt);	// kaizen 20130731 ID1107 Changed option( A(Auto):Start Web server when link up event, M(Manual): Start Web Server when enter command )
		return WXCODE_SUCCESS;
	}

#if 0 //MikeJ 130823 - On/Off parameter should be 'Required'
	p = WXParse_NextParamGet(&ptr);
	if (p)
	{
		status = WXParse_Int( p, &web_server_cmd );
		if ( status != WXCODE_SUCCESS )		return WXCODE_EINVAL;
		if ( web_server_cmd != STOP_WEB_SERVER && web_server_cmd != START_WEB_SERVER )		return WXCODE_EINVAL;
	}
#else
	p = WXParse_NextParamGet(&ptr);
	if ( !p )	return WXCODE_EINVAL;
	status = WXParse_Int( p, &web_server_cmd );
	if ( status != WXCODE_SUCCESS )		return WXCODE_EINVAL;
	if ( web_server_cmd != STOP_WEB_SERVER && web_server_cmd != START_WEB_SERVER )		return WXCODE_EINVAL;
#endif

#if 1	// kaizen 20130731 ID1107 Changed option( A(Auto):Start Web server when link up event, M(Manual): Start Web Server when enter command )
	p = WXParse_NextParamGet(&ptr);
	if ( p )
	{
		// sekim 20150212 Like Function Button by Coway
		if ( strcmp(upstr((char*)p), "F") == 0 )
		{
			send_maincommand_queue(105, 1);
			return WXCODE_SUCCESS;
		}

		if ( strcmp(upstr((char*)p), "A") != 0 && strcmp(upstr((char*)p),"M") != 0 )
			return WXCODE_EINVAL;

		strcpy((char*)g_wxProfile.web_server_opt, (char*)p);
		g_wxProfile.web_server_opt[1] = '\0';
	}

	if( web_server_cmd == START_WEB_SERVER )
	{
		if( web_server_status == START_WEB_SERVER && p_http_thread != NULL )
		{
			W_DBG("WXCmd_FWEBS : Configuration Web Server is already running");
			return WXCODE_SUCCESS;
		}

		if		( g_wxProfile.wifi_mode == STATION_MODE )		interface = WICED_STA_INTERFACE;
		else if	( g_wxProfile.wifi_mode == AP_MODE )			interface = WICED_AP_INTERFACE;

		if( wiced_wifi_is_ready_to_transceive(interface) != WICED_SUCCESS )
		{
			W_DBG("WXCmd_FWEBS : Wizfi250 will start web server after WJOIN command ");
			return WXCODE_SUCCESS;
		}

		p_http_thread = &http_thread;
		web_server_status = START_WEB_SERVER;
		if( wiced_rtos_create_thread((wiced_thread_t*)p_http_thread, WICED_APPLICATION_PRIORITY+3,"AP_Sta_Mode_Webserver",ap_sta_mode_webserver_thread, 1024 * 4, NULL) != WICED_SUCCESS )
		{
			W_DBG("WXCmd_FWEBS : Config_Webserver thread create error");
			return WXCODE_FAILURE;
		}
	}

	else if( web_server_cmd == STOP_WEB_SERVER )
	{
		if(web_server_status == STOP_WEB_SERVER )
		{
			W_DBG("WXCmd_FWEBS : Configuration web server is not running");
			return WXCODE_SUCCESS;
		}

		if( wiced_configure_device_stop() != WICED_SUCCESS )
		{
			W_DBG("WXCmd_FWEBS : error of wiced_configure_device_stop");
			return WXCODE_FAILURE;
		}

		web_server_status = STOP_WEB_SERVER;
	}


	return WXCODE_SUCCESS;
}

#else
	if( web_server_cmd == START_WEB_SERVER )
	{
		if( web_server_status == WEB_SERVER_IS_RUN )
		{
			W_DBG("Configuration Web Server is already running");
			return WXCODE_FAILURE;
		}

		if		( g_wxProfile.wifi_mode == STATION_MODE )		interface = WICED_STA_INTERFACE;
		else if	( g_wxProfile.wifi_mode == AP_MODE )			interface = WICED_AP_INTERFACE;

		if( wiced_wifi_is_ready_to_transceive(interface) != WICED_SUCCESS )
		{
			W_DBG("WizFi250 must run to Station Mode or AP Mode");
			return WXCODE_FAILURE;
		}

#if 0	// kaizen 20130513 For Web Server Service Option
		p = WXParse_NextParamGet(&ptr);
		if ( !p )							return WXCODE_EINVAL;
		if ( strlen((char*)p) >= 3 )		return WXCODE_EINVAL;
		strcpy((char*)buf_web_server_opt, (char*)p);

		if ( strcmp(upstr((char*)buf_web_server_opt), "SO") != 0 && strcmp(upstr((char*)buf_web_server_opt),"O") != 0 )
			return WXCODE_EINVAL;

		strcpy((char*)g_wxProfile.web_server_opt, (char*)buf_web_server_opt);

		if( strcmp(upstr((char*)buf_web_server_opt),"SO") == 0 || strcmp(upstr((char*)buf_web_server_opt),"O") == 0 )
		{
			p_http_thread = &http_thread;
			web_server_status = WEB_SERVER_IS_RUN;
			if( wiced_rtos_create_thread((wiced_thread_t*)p_http_thread, WICED_APPLICATION_PRIORITY+3,"AP_Sta_Mode_Webserver",ap_sta_mode_webserver_thread, 1024 * 4, NULL) != WICED_SUCCESS )
			{
				W_DBG("WXCmd_FWEBS : Config_Webserver thread create error");
				return WXCODE_FAILURE;
			}
		}
#else
		p_http_thread = &http_thread;
		web_server_status = WEB_SERVER_IS_RUN;
		if( wiced_rtos_create_thread((wiced_thread_t*)p_http_thread, WICED_APPLICATION_PRIORITY+3,"AP_Sta_Mode_Webserver",ap_sta_mode_webserver_thread, 1024 * 4, NULL) != WICED_SUCCESS )
		{
			W_DBG("WXCmd_FWEBS : Config_Webserver thread create error");
			return WXCODE_FAILURE;
		}
#endif
	}
	else if( web_server_cmd == STOP_WEB_SERVER )
	{
		if( web_server_status == WEB_SERVER_IS_STOP )
		{
			W_DBG("Configuration web server is not running");
			return WXCODE_FAILURE;
		}

		if( wiced_configure_device_stop() != WICED_SUCCESS )
		{
			W_DBG("error : wiced_configure_device_stop");
			return WXCODE_FAILURE;
		}

		web_server_status = WEB_SERVER_IS_STOP;
	}

	return WXCODE_SUCCESS;
}
#endif

// sekim 20140716 ID1182 add s2web_main
UINT8 WXCmd_FWEBM(UINT8 *ptr)
{
	UINT8 *p;
	UINT8 i;

	if ( strcmp((char*)ptr, "?") == 0 )
	{
		for (i=0; i<7; i++)
		{
			W_RSP("%c", g_wxProfile.s2web_main[i]);
		}
		W_RSP("\r\n");
		return WXCODE_SUCCESS;
	}

	p = WXParse_NextParamGet(&ptr);
	if (!p)						return WXCODE_EINVAL;
	if ( strlen((char*)p)!=7 )	return WXCODE_EINVAL;

	for (i=0; i<7; i++)		if ( p[i]<'0' || p[i]>'9' )  return WXCODE_EINVAL;
	for (i=0; i<7; i++)		g_wxProfile.s2web_main[i] = p[i];

	return WXCODE_SUCCESS;
}

//  kaizen 20130704 ID1081 Added OTA Command
UINT8 WXCmd_FOTA(UINT8 *ptr)
{
	WXS2w_StatusNotify(WXCODE_SUCCESS, 0);

	wiced_start_ota_upgrade();

	return WXCODE_SUCCESS;
}

#if 1 // kaizen 20130814 Modified AT+FGPIO function
UINT8 WXCmd_FGPIO(UINT8 *ptr)
{
	UINT8 *p;
	UINT8 i, status, selected_gpio=0;
	UINT32 mode=1;					// 0 : input , 1 : output, 2 : initialize
	UINT32 gpio_num, gpio_config_value, gpio_set_value;


	if ( strcmp((char*)ptr, "?") == 0 )
	{
		for(i=0; i<USER_GPIO_MAX_COUNT; i++)
		{
			// kaizen 20131125 Bug fixed about GPIO Input Mode
			if( g_wxProfile.user_gpio_conifg[i].mode == GPIO_INPUT_MODE ) // input_mode
				g_wxProfile.user_gpio_conifg[i].gpio_set_value = wiced_gpio_input_get(g_wxProfile.user_gpio_conifg[i].gpio_num - 1);

			if( i == USER_GPIO_MAX_COUNT-1 )
			{
				W_RSP("{%d,%d,%d,%d}\r\n",g_wxProfile.user_gpio_conifg[i].mode,g_wxProfile.user_gpio_conifg[i].gpio_num,g_wxProfile.user_gpio_conifg[i].gpio_config_value,g_wxProfile.user_gpio_conifg[i].gpio_set_value);
				break;
			}

			W_RSP("{%d,%d,%d,%d},",g_wxProfile.user_gpio_conifg[i].mode,g_wxProfile.user_gpio_conifg[i].gpio_num,g_wxProfile.user_gpio_conifg[i].gpio_config_value,g_wxProfile.user_gpio_conifg[i].gpio_set_value);
		}

		return WXCODE_SUCCESS;
	}

	p = WXParse_NextParamGet(&ptr);
	if (!p)		return WXCODE_EINVAL;
	status = WXParse_Int(p, &mode);
	if ( status != WXCODE_SUCCESS )				return WXCODE_EINVAL;
	if ( mode != GPIO_INPUT_MODE && mode != GPIO_OUTPUT_MODE && mode != GPIO_INIT_MODE )	return WXCODE_EINVAL;


	p = WXParse_NextParamGet(&ptr);
	if (!p)		return WXCODE_EINVAL;
	status = WXParse_Int(p, &gpio_num);
	if ( status != WXCODE_SUCCESS)				return WXCODE_EINVAL;
	for ( selected_gpio=0; selected_gpio< USER_GPIO_MAX_COUNT; selected_gpio++ )
	{
		if( g_wxProfile.user_gpio_conifg[selected_gpio].gpio_num == gpio_num )
			break;

		if( selected_gpio >= USER_GPIO_MAX_COUNT - 1 )
			return WXCODE_EINVAL;
	}
	gpio_num = gpio_num - 1;		// WICED_GPIO is defined enum type. But enum type start 0 value. ( ex, WICED_GPIO6 = 5(enum value) )


	if ( mode == GPIO_INPUT_MODE )				// 0 is input mode
	{
		if ( g_wxProfile.user_gpio_conifg[selected_gpio].mode != GPIO_INPUT_MODE )
		{
			W_DBG("GPIO initialize is wrong. it must be set <input mode");
			return WXCODE_EINVAL;
		}

#if 1	// kaizen 20131125 Bug fixed about GPIO Input Mode
		g_wxProfile.user_gpio_conifg[selected_gpio].gpio_set_value = wiced_gpio_input_get(gpio_num);
		W_RSP("%d\r\n", g_wxProfile.user_gpio_conifg[selected_gpio].gpio_set_value);
#else
		gpio_set_value = wiced_gpio_input_get(gpio_num);
		W_RSP("%d\r\n", gpio_set_value);
#endif
	}
	else if ( mode == GPIO_OUTPUT_MODE )			// 1 is output mode
	{
		if ( g_wxProfile.user_gpio_conifg[selected_gpio].mode != GPIO_OUTPUT_MODE )
		{
			W_DBG("GPIO initialize is wrong. it must be set <output mode>");
			return WXCODE_EINVAL;
		}

		p = WXParse_NextParamGet(&ptr);
		if (!p)		return WXCODE_EINVAL;
		status = WXParse_Int(p, &gpio_set_value);
		if ( status != WXCODE_SUCCESS )												return WXCODE_EINVAL;
		if ( gpio_set_value != GPIO_SET_LOW && gpio_set_value != GPIO_SET_HIGH )	return WXCODE_EINVAL;

		if		( gpio_set_value == GPIO_SET_LOW )		wiced_gpio_output_low  (gpio_num);
		else if ( gpio_set_value == GPIO_SET_HIGH )		wiced_gpio_output_high (gpio_num);

		g_wxProfile.user_gpio_conifg[selected_gpio].gpio_set_value = gpio_set_value;
	}
	else if ( mode == GPIO_INIT_MODE )
	{
		p =  WXParse_NextParamGet(&ptr);
		if (!p)		return WXCODE_EINVAL;
		status = WXParse_Int(p, &gpio_config_value);
		if ( status != WXCODE_SUCCESS )
		if ( gpio_config_value < 0 || gpio_config_value > 5 )	return WXCODE_EINVAL;


		if ( gpio_config_value == 0 || gpio_config_value == 1 || gpio_config_value == 2 )
			g_wxProfile.user_gpio_conifg[selected_gpio].mode = 0;	// input mode
		else if ( gpio_config_value == 3 || gpio_config_value == 4 || gpio_config_value ==5 )
			g_wxProfile.user_gpio_conifg[selected_gpio].mode = 1;	// output mode

		g_wxProfile.user_gpio_conifg[selected_gpio].gpio_config_value = gpio_config_value;
		wiced_gpio_init(gpio_num, gpio_config_value );
	}

	return WXCODE_SUCCESS;
}
#else
UINT8 WXCmd_FGPIO(UINT8 *ptr)
{
	UINT8 *p;
	UINT8 status;
	UINT32 mode=0;
	UINT32 gpio_num=0;
	UINT32 gpio_value=0;


	p = WXParse_NextParamGet(&ptr);
	if (!p)		return WXCODE_EINVAL;
	status = WXParse_Int(p, &mode);

	if ( status != WXCODE_SUCCESS )				return WXCODE_EINVAL;
	if ( mode != 0 && mode != 1 )				return WXCODE_EINVAL;


	p = WXParse_NextParamGet(&ptr);
	if (!p)		return WXCODE_EINVAL;
	status = WXParse_Int(p, &gpio_num);
	gpio_num = gpio_num - 1;	//	WICED_GPIO defined in WICED are set -1 ( ex, WICED_GPIO6 = 5 )

	if ( status != WXCODE_SUCCESS )				return WXCODE_EINVAL;
	if ( gpio_num != WICED_GPIO_1 && gpio_num != WICED_GPIO_5 && gpio_num != WICED_GPIO_6 && gpio_num != WICED_GPIO_7 && gpio_num != WICED_GPIO_8 )	return WXCODE_EINVAL;

	if ( mode == 0 )		// Input GPIO
	{
		wiced_gpio_init(gpio_num, INPUT_HIGH_IMPEDANCE); // pull up 할지 pull down 할지 impedance 할지 고민
		gpio_value = wiced_gpio_input_get(gpio_num);
		W_RSP("GPIO%d value : %d\r\n",gpio_num+1, gpio_value);
	}
	else if ( mode == 1 )	// Output GPIO
	{
		p = WXParse_NextParamGet(&ptr);
		if (!p)		return WXCODE_EINVAL;
		status = WXParse_Int(p, &gpio_value);
		if ( status != WXCODE_SUCCESS )										return WXCODE_EINVAL;
		if ( gpio_value != GPIO_SET_LOW && gpio_value != GPIO_SET_HIGH )	return WXCODE_EINVAL;

		wiced_gpio_init(gpio_num, OUTPUT_PUSH_PULL);

		if( gpio_value == GPIO_SET_LOW )
		{
			wiced_gpio_output_low  (gpio_num);
		}
		else if ( gpio_value == GPIO_SET_HIGH )
		{
			wiced_gpio_output_high (gpio_num);
		}
	}
	return WXCODE_SUCCESS;
}
#endif


// sekim 20131212 ID1141 add g_socket_extx_option
extern uint32_t g_socket_extx_option1;
extern uint32_t g_socket_extx_option2;
extern uint32_t g_socket_extx_option3;
extern uint32_t g_socket_critical_error;
UINT8 WXCmd_FSOCK(UINT8 *ptr)
{
	UINT8 *p;
	UINT32 buff1 = 0, buff2 = 0;
	UINT8 status;

	if ( strcmp((char*)ptr, "?") == 0 )
	{
		// sekim 20140929 ID1188 Data-Idle-Auto-Reset (Autonix)
		/*
		W_RSP("%d,%d,%d,%d,%d,%d\r\n", g_wxProfile.socket_ext_option1, g_wxProfile.socket_ext_option2, g_wxProfile.socket_ext_option3,
				g_wxProfile.socket_ext_option4, g_wxProfile.socket_ext_option5, g_wxProfile.socket_ext_option6);
				*/
		W_RSP("%d,%d,%d,%d,%d,%d,%d,%d\r\n", g_wxProfile.socket_ext_option1, g_wxProfile.socket_ext_option2, g_wxProfile.socket_ext_option3,
				g_wxProfile.socket_ext_option4, g_wxProfile.socket_ext_option5, g_wxProfile.socket_ext_option6, g_wxProfile.socket_ext_option7, g_wxProfile.socket_ext_option8);

		return WXCODE_SUCCESS;
	}

	p = WXParse_NextParamGet(&ptr);
	if ( !p )							return WXCODE_EINVAL;
	status = WXParse_Int(p, &buff1);
	if ( status != WXCODE_SUCCESS )		return WXCODE_EINVAL;

	p = WXParse_NextParamGet(&ptr);
	if ( !p )							return WXCODE_EINVAL;
	status = WXParse_Int(p, &buff2);
	if ( status != WXCODE_SUCCESS )		return WXCODE_EINVAL;

	if ( buff1==1 )
	{
		if ( (buff2==0) || (buff2>=2000 && buff2<=20000) )
		{
			g_wxProfile.socket_ext_option1 = buff2;
			g_socket_extx_option1 = g_wxProfile.socket_ext_option1;
			return WXCODE_SUCCESS;
		}
	}
	else if ( buff1==2 )
	{
		if ( (buff2==0) || (buff2>=2000 && buff2<=20000) )
		{
			g_wxProfile.socket_ext_option2 = buff2;
			g_socket_extx_option2 = g_wxProfile.socket_ext_option2;
			return WXCODE_SUCCESS;
		}
	}
	else if ( buff1==3 )
	{
		g_wxProfile.socket_ext_option3 = buff2;
		g_socket_extx_option3 = g_wxProfile.socket_ext_option3;
		return WXCODE_SUCCESS;
	}
	else if ( buff1==4 )
	{
		if ( buff2>=10 )
		{
			g_wxProfile.socket_ext_option4 = buff2;
			return WXCODE_SUCCESS;
		}
	}
	// sekim 20140625 ID1176 add option to clear tcp-idle-connection
	else if ( buff1==5 )
	{
		if ( (buff2==0) || (buff2>=5 && buff2<=3600) )
		{
			g_wxProfile.socket_ext_option5 = buff2;
			return WXCODE_SUCCESS;
		}
	}
	// sekim 20140430 ID1152 add scon_retry
	else if ( buff1==6 )
	{
		if (  buff2==0 || ((buff2>=10) && (buff2<=1000)) )
		{
			g_wxProfile.socket_ext_option6 = buff2;
			return WXCODE_SUCCESS;
		}
	}
	// sekim 20140929 ID1188 Data-Idle-Auto-Reset (Autonix)
	else if ( buff1==7 )
	{
		if (  buff2==0 || ((buff2>=5) && (buff2<=1000)) )
		{
			g_wxProfile.socket_ext_option7 = buff2;
			return WXCODE_SUCCESS;
		}
	}
	// sekim 20150519 add socket_ext_option8. Reset if not connected in TCP Client/Service
	else if ( buff1==8 )
	{
		if (  buff2==0 || ((buff2>=5) && (buff2<=1000)) )
		{
			g_wxProfile.socket_ext_option8 = buff2;
			return WXCODE_SUCCESS;
		}
	}
	// sekim XXXX 20160119 Add socket_ext_option9 for TCP Server Multi-Connection
	else if ( buff1==9 )
	{
		g_wxProfile.socket_ext_option9 = buff2;
		return WXCODE_SUCCESS;
	}


	return WXCODE_EINVAL;
}

UINT8 WXCmd_S2WEB(UINT8 *ptr)
{
	UINT8 	*p;

	extern char g_s2web_data[256];

	p = WXParse_NextParamGet(&ptr);
	if (!p)		return WXCODE_EINVAL;
	strncpy((char*)g_s2web_data, (char*)p, sizeof(g_s2web_data));

	return WXCODE_SUCCESS;
}

// sekim 20130510 Service Mode(Requested Functions for a specific customer)
UINT8 WXCmd_SMODE(UINT8 *ptr)
{
	UINT8 *p;
	UINT32 buff1 = 0;

	p = WXParse_NextParamGet(&ptr);
	if ( !p )							return WXCODE_EINVAL;
	WXParse_Int(p, &buff1);

	g_wxProfile.smode = buff1;

	return WXCODE_SUCCESS;
}

// kaizen 20140408 ID1154, ID1166 Add AT+FWEBSOPT
UINT8 WXCmd_FWEBSOPT(UINT8 *ptr)
{
	UINT8 *p;
	UINT32 buff1 = 0, buff2 = 0, buff3 = 0, buff4 = 0;
	UINT8 status;

	if ( strcmp((char*)ptr, "?,1") == 0 )
	{
		W_RSP("%d,%d,%d\r\n", g_wxProfile.show_hidden_ssid, g_wxProfile.show_only_english_ssid, g_wxProfile.show_rssi_range);
		return WXCODE_SUCCESS;
	}

	p = WXParse_NextParamGet(&ptr);
	if ( !p )							return WXCODE_EINVAL;
	status = WXParse_Int(p, &buff1);
	if ( status != WXCODE_SUCCESS )		return WXCODE_EINVAL;

	if ( buff1==1 )		// scan option
	{
		p = WXParse_NextParamGet(&ptr);
		if ( !p )							return WXCODE_EINVAL;
		status = WXParse_Int(p, &buff2);
		if ( status != WXCODE_SUCCESS )		return WXCODE_EINVAL;
		if ( (buff2 != 0) && (buff2 != 1) )
			return WXCODE_EINVAL;


		p = WXParse_NextParamGet(&ptr);
		if ( !p )							return WXCODE_EINVAL;
		status = WXParse_Int(p, &buff3);
		if ( status != WXCODE_SUCCESS )		return WXCODE_EINVAL;
		if ( (buff3 != 0) && (buff3 != 1) )
			return WXCODE_EINVAL;


		p = WXParse_NextParamGet(&ptr);
		if ( !p )							return WXCODE_EINVAL;
		status = WXParse_Int(p, &buff4);
		if ( status != WXCODE_SUCCESS )		return WXCODE_EINVAL;
		if ( (buff4 < 0) && (buff4 > 3) )
			return WXCODE_EINVAL;


		g_wxProfile.show_hidden_ssid 		= (UINT8)buff2;
		g_wxProfile.show_only_english_ssid	= (UINT8)buff3;
		g_wxProfile.show_rssi_range			= (UINT8)buff4;
	}

	return WXCODE_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////
// sekim 20150622 add FFTPSET/FFTPCMD for choyoung
/*
UINT8 WXCmd_FFTPSET(UINT8 *ptr)
{
	UINT8 *p;
	UINT8 status;

	UINT8 ftpset_buff_ip[16];
	UINT32 ftpset_buff_port;
	UINT8 ftpset_buff_id[20];
	UINT8 ftpset_buff_pw[20];

	memset(ftpset_buff_ip, 0, sizeof(ftpset_buff_ip));
	ftpset_buff_port = 0;
	memset(ftpset_buff_id, 0, sizeof(ftpset_buff_id));
	memset(ftpset_buff_pw, 0, sizeof(ftpset_buff_pw));

	if ( strcmp((char*)ptr, "?") == 0 )
	{
		W_RSP("%s,%d,%s,%s\r\n", g_wxProfile.ftpset_ip, g_wxProfile.ftpset_port, g_wxProfile.ftpset_id, g_wxProfile.ftpset_pw);
		return WXCODE_SUCCESS;
	}

	p = WXParse_NextParamGet(&ptr);
	if ( p )
	{
		strcpy((char*)ftpset_buff_ip, (char*)p);
	}

	p = WXParse_NextParamGet(&ptr);
	if ( !p )							return WXCODE_EINVAL;
	status = WXParse_Int(p, &ftpset_buff_port);
	if ( status != WXCODE_SUCCESS )		return WXCODE_EINVAL;

	p = WXParse_NextParamGet(&ptr);
	if ( p )
	{
		strcpy((char*)ftpset_buff_id, (char*)p);
	}

	p = WXParse_NextParamGet(&ptr);
	if ( p )
	{
		strcpy((char*)ftpset_buff_pw, (char*)p);
	}

	strcpy((char*)g_wxProfile.ftpset_ip, (char*)ftpset_buff_ip);
	g_wxProfile.ftpset_port = ftpset_buff_port;
	strcpy((char*)g_wxProfile.ftpset_id, (char*)ftpset_buff_id);
	strcpy((char*)g_wxProfile.ftpset_pw, (char*)ftpset_buff_pw);

	return WXCODE_SUCCESS;
}

// LIST/DELE/RETR/STOR
UINT8 WXCmd_FFTPCMD(UINT8 *ptr)
{
	UINT8 *p;
	char szFTPCommand[100];
	char szFTPBuff[100];

	p = WXParse_NextParamGet(&ptr);
	if ( p )
	{
		strcpy(szFTPCommand, (char*)p);
	}

	if ( !WXLink_IsWiFiLinked() )		return WXCODE_FAILURE;
	if ( memcmp(szFTPCommand, "LIST", 4)!=0 && memcmp(szFTPCommand, "DELE", 4)!=0 && memcmp(szFTPCommand, "RETR", 4)!=0 && memcmp(szFTPCommand, "STOR", 4)!=0 )
	{
		return WXCODE_EINVAL;
	}

    wiced_result_t result;
	wiced_packet_t* rx_packet;
	wiced_tcp_socket_t socket_ftpc;
	wiced_tcp_socket_t socket_ftpd;

    char* rx_data;
	uint16_t rx_data_length;
	uint16_t available_data_length;
	wiced_interface_t interface = (g_wxProfile.wifi_mode==AP_MODE)?WICED_AP_INTERFACE:WICED_STA_INTERFACE;

	// Connect TCP
	result = wiced_tcp_create_socket(&socket_ftpc, interface);
	if ( result!=WICED_SUCCESS )
	{
		W_DBG("wiced_tcp_create_socket error : %d", result);
		return WXCODE_FAILURE;
	}
	wiced_ip_address_t INITIALISER_IPV4_ADDRESS(remote_ip, str_to_ip((char*)g_wxProfile.ftpset_ip));
	#define TCP_CLIENT_CONNECT_TIMEOUT        5000
	result = wiced_tcp_connect(&socket_ftpc, &remote_ip, g_wxProfile.ftpset_port, TCP_CLIENT_CONNECT_TIMEOUT);
	if ( result!=WICED_SUCCESS )
	{
		W_DBG("wiced_tcp_connect : connect error %s, %d", g_wxProfile.ftpset_ip, g_wxProfile.ftpset_port);
		wiced_tcp_delete_socket(&socket_ftpc);
		return WXCODE_FAILURE;
	}

	// Create Session
	UINT32 ftp_pasv_buff_ip[4], ftp_pasv_buff_port[2];
	UINT8 ftp_state = 0;
	while(1)
	{
		result = wiced_tcp_receive(&socket_ftpc, &rx_packet, WICED_WAIT_FOREVER);
		if ( result!=WICED_SUCCESS )
		{
			if ( result==WICED_PENDING )
			{
				if ( socket_ftpc.socket.nx_tcp_socket_state==NX_TCP_ESTABLISHED )
					continue;
			}

			ftp_state = -1;
			break;
		}
		wiced_packet_get_data(rx_packet, 0, (uint8_t**)&rx_data, &rx_data_length, &available_data_length);

		if ( memcmp(rx_data, "220 ", 4 )==0 )		{ sprintf(szFTPBuff, "USER %s\n", g_wxProfile.ftpset_id); result = wiced_tcp_send_buffer(&socket_ftpc, szFTPBuff, strlen(szFTPBuff)); }
		else if ( memcmp(rx_data, "331 ", 4 )==0 )	{ sprintf(szFTPBuff, "PASS %s\n", g_wxProfile.ftpset_pw); result = wiced_tcp_send_buffer(&socket_ftpc, szFTPBuff, strlen(szFTPBuff)); }
		else if ( memcmp(rx_data, "230 ", 4 )==0 )	{ sprintf(szFTPBuff, "TYPE I\n");                         result = wiced_tcp_send_buffer(&socket_ftpc, szFTPBuff, strlen(szFTPBuff)); }
		else if ( memcmp(rx_data, "200 ", 4 )==0 )	{ sprintf(szFTPBuff, "PASV\n");                           result = wiced_tcp_send_buffer(&socket_ftpc, szFTPBuff, strlen(szFTPBuff)); }
		else if ( memcmp(rx_data, "227 ", 4 )==0 )
		{
			char* ftp_pasv_data = strstr(rx_data, "(");
			printf("XXX 200 : (%s)\r\n", ftp_pasv_data);
			sscanf(ftp_pasv_data, "(%d,%d,%d,%d,%d,%d)", &ftp_pasv_buff_ip[0], &ftp_pasv_buff_ip[1], &ftp_pasv_buff_ip[2], &ftp_pasv_buff_ip[3], &ftp_pasv_buff_port[0], &ftp_pasv_buff_port[1]);
			printf("XXX 204 : (%d, %d, %d, %d, %d, %d)\r\n", ftp_pasv_buff_ip[0], ftp_pasv_buff_ip[1], ftp_pasv_buff_ip[2], ftp_pasv_buff_ip[3], ftp_pasv_buff_port[0], ftp_pasv_buff_port[1]);
			#define	pack_port(var, off) (((var[(off) + 0] & 0xff) << 8) | ((var[(off) + 1] & 0xff) << 0))
			printf("XXX 210 : (%d)\r\n", pack_port(ftp_pasv_buff_port, 0));
			break;
		}
		else     									{ ftp_state = -3; break; }

		if ( result!=WICED_SUCCESS )
		{
			ftp_state = -2;
			break;
		}

		wiced_packet_delete(rx_packet);
	}

	wiced_packet_delete(rx_packet);
	if ( ftp_state!=0 )
	{
		wiced_tcp_disconnect(&socket_ftpc);
		wiced_tcp_delete_socket(&socket_ftpc);
		W_DBG("WXCmd_FFTPCMD error : ftp_state(%d)", ftp_state);
		return WXCODE_FAILURE;
	}

	// Connect TCP
	char ftpd_string_ip[20];
	UINT16 ftpd_port = pack_port(ftp_pasv_buff_port, 0);
	sprintf(ftpd_string_ip, "%d.%d.%d.%d", ftp_pasv_buff_ip[0], ftp_pasv_buff_ip[1], ftp_pasv_buff_ip[2], ftp_pasv_buff_ip[3]);

	W_DBG("FTP Data : ftpd_string_ip(%s), ftpd_port(%d)\r\n", ftpd_string_ip, ftpd_port);

	result = wiced_tcp_create_socket(&socket_ftpd, interface);
	if ( result!=WICED_SUCCESS )
	{
		W_DBG("wiced_tcp_create_socket error : %d", result);
		return WXCODE_FAILURE;
	}
	wiced_ip_address_t INITIALISER_IPV4_ADDRESS(ftpd_ip, str_to_ip((char*)ftpd_string_ip));
	result = wiced_tcp_connect(&socket_ftpd, &ftpd_ip, ftpd_port, TCP_CLIENT_CONNECT_TIMEOUT);
	if ( result!=WICED_SUCCESS )
	{
		W_DBG("wiced_tcp_connect : connect error %s, %d", ftpd_string_ip, ftpd_port);
		wiced_tcp_delete_socket(&socket_ftpd);
		return WXCODE_FAILURE;
	}


	// Do FTP Command (LIST)
	if ( memcmp(szFTPCommand, "LIST", 4)==0 )
	{
		sprintf(szFTPBuff, "%s\n", szFTPCommand);
		result = wiced_tcp_send_buffer(&socket_ftpc, szFTPBuff, strlen(szFTPBuff));
		if ( result!=WICED_SUCCESS )
		{
			wiced_tcp_disconnect(&socket_ftpc);
			wiced_tcp_delete_socket(&socket_ftpc);
			W_DBG("WXCmd_FFTPCMD error : ftp_state(%d)", ftp_state);
			return WXCODE_FAILURE;
		}
		result = wiced_tcp_receive(&socket_ftpc, &rx_packet, WICED_WAIT_FOREVER);
		if ( result!=WICED_SUCCESS )
		{
			wiced_tcp_disconnect(&socket_ftpc);
			wiced_tcp_delete_socket(&socket_ftpc);
			W_DBG("WXCmd_FFTPCMD error : ftp_state(%d)", ftp_state);
			return WXCODE_FAILURE;
		}
		wiced_packet_get_data(rx_packet, 0, (uint8_t**)&rx_data, &rx_data_length, &available_data_length);
		if ( rx_data[0]=='3' || rx_data[0]=='4' || rx_data[0]=='5' )
		{
			wiced_tcp_disconnect(&socket_ftpc);
			wiced_tcp_delete_socket(&socket_ftpc);
			W_DBG("WXCmd_FFTPCMD error : ftp_state(%d)", ftp_state);
			return WXCODE_FAILURE;
		}

		while(1)
		{
			result = wiced_tcp_receive(&socket_ftpd, &rx_packet, WICED_WAIT_FOREVER);

			if ( result!=WICED_SUCCESS )
			{
				if ( result==WICED_PENDING )
				{
					if ( socket_ftpd.socket.nx_tcp_socket_state==NX_TCP_ESTABLISHED )
						continue;
				}

				ftp_state = -1;
				break;
			}
			wiced_packet_get_data(rx_packet, 0, (uint8_t**)&rx_data, &rx_data_length, &available_data_length);

			extern wiced_mutex_t g_upart_type1_wizmutex;
			wiced_rtos_lock_mutex(&g_upart_type1_wizmutex);
			WXHal_CharNPut(rx_data, rx_data_length);
			wiced_rtos_unlock_mutex(&g_upart_type1_wizmutex);

			wiced_packet_delete(rx_packet);
		}

		wiced_packet_delete(rx_packet);
		wiced_tcp_disconnect(&socket_ftpd);
		wiced_tcp_delete_socket(&socket_ftpd);
	}





	// Close Session
	sprintf(szFTPBuff, "QUIT\n");
	result = wiced_tcp_send_buffer(&socket_ftpc, szFTPBuff, strlen(szFTPBuff));
	if ( result!=WICED_SUCCESS )
	{
		W_DBG("WXCmd_FFTPCMD QUIT error : ");
		wiced_tcp_disconnect(&socket_ftpc);
		wiced_tcp_delete_socket(&socket_ftpc);
		return WXCODE_FAILURE;
	}

	// Disconnect TCP
	wiced_tcp_disconnect(&socket_ftpc);
	wiced_tcp_delete_socket(&socket_ftpc);

	return WXCODE_SUCCESS;
}
*/
////////////////////////////////////////////////////////////////////////////////////////////










// sekim 20141125 add WXCmd_FGETADC
UINT8 g_initADC = 0;
UINT8 WXCmd_FGETADC(UINT8 *ptr)
{
	wiced_result_t result;
	if ( g_initADC==0 )
	{
		result = wiced_adc_init( WICED_ADC_3, 5 );
		if ( result!=WICED_SUCCESS )
		{
			W_DBG("wiced_adc_init error : %d", result);
			return WXCODE_EINVAL;
		}
	}

	UINT16 sample_value;
	result = wiced_adc_take_sample(WICED_ADC_3, &sample_value);
	if ( result!=WICED_SUCCESS )
	{
		W_DBG("wiced_adc_take_sample error : %d", result);
		return WXCODE_EINVAL;
	}
	W_RSP("%d\r\n", sample_value);

	return WXCODE_SUCCESS;
}

// sekim 20150508 Coway ReSend <Connect Event Message>
UINT8 g_cowaya_recv_checkdata = 0;
UINT8 WXCmd_COWAYA(UINT8 *ptr)
{
	UINT8 *p;
	UINT8 status;

	UINT32 buff_cowaya_check_time;
	char buff_cowaya_check_data[10];

	if ( strcmp((char*)ptr, "?") == 0 )
	{
		W_RSP("%d,%s\r\n", g_wxProfile.cowaya_check_time, g_wxProfile.cowaya_check_data);
		return WXCODE_SUCCESS;
	}

	p = WXParse_NextParamGet(&ptr);
	if ( !p )	return WXCODE_EINVAL;
	status = WXParse_Int(p, &buff_cowaya_check_time);
	if ( status != WXCODE_SUCCESS )		return WXCODE_EINVAL;

	p = WXParse_NextParamGet(&ptr);
	if ( p )
	{
		if ( strlen((char*)p)> (sizeof(buff_cowaya_check_data)-1) )	return WXCODE_EINVAL;

		strcpy((char*)buff_cowaya_check_data, (char*)p);
	}

	g_wxProfile.cowaya_check_time = buff_cowaya_check_time;
	strcpy(g_wxProfile.cowaya_check_data, buff_cowaya_check_data);

	return WXCODE_SUCCESS;
}

// sekim 20151222 Add GMMP Command
#include "gmmp_lib/define/define.h"
UINT8 WXCmd_GMMPSET(UINT8 *ptr)
{
	UINT8 	*p;
	UINT8 	status;
	UINT32 	gmmp_mode;
	UINT8 	gmmp_ip[20];
	UINT32	gmmp_port;
	UINT8 	gmmp_DomainCode[LEN_DOMAIN_CODE];
	UINT8 	gmmp_GWAuthID[LEN_AUTH_ID];
	UINT8 	gmmp_GWMFID[LEN_AUTH_KEY];
	UINT8 	gmmp_DeviceID[LEN_GW_ID];

	p = WXParse_NextParamGet(&ptr);
	if (!p)		return WXCODE_EINVAL;
	status = WXParse_Int(p, &gmmp_mode);
	if (status != WXCODE_SUCCESS)	return WXCODE_EINVAL;

	if ( gmmp_mode==0 )
	{
		int GMMPStopService();
		GMMPStopService();
		return WXCODE_SUCCESS;
	}

	p = WXParse_NextParamGet(&ptr);
	if (!p)		return WXCODE_EINVAL;
	strcpy((char*)gmmp_ip, (char*)p);

	p = WXParse_NextParamGet(&ptr);
	if (!p)		return WXCODE_EINVAL;
	status = WXParse_Int(p, &gmmp_port);
	if ( status!=WXCODE_SUCCESS )	return WXCODE_EINVAL;

	p = WXParse_NextParamGet(&ptr);
	if (!p)		return WXCODE_EINVAL;
	strcpy((char*)gmmp_DomainCode, (char*)p);

	p = WXParse_NextParamGet(&ptr);
	if (!p)		return WXCODE_EINVAL;
	strcpy((char*)gmmp_GWAuthID, (char*)p);

	p = WXParse_NextParamGet(&ptr);
	if (!p)		return WXCODE_EINVAL;
	strcpy((char*)gmmp_GWMFID, (char*)p);

	p = WXParse_NextParamGet(&ptr);
	if (!p)		return WXCODE_EINVAL;
	strcpy((char*)gmmp_DeviceID, (char*)p);

	if ( gmmp_mode==1 )
	{
		int GMMPStartService(char* szServerIP, int nPort, char* szDomainCode, char* szGWAuthID, char* szGWMFID, char* szDeviceID, const int nNetwrokType);
		int nRet = GMMPStartService((char*)gmmp_ip, gmmp_port, (char*)gmmp_DomainCode, (char*)gmmp_GWAuthID, (char*)gmmp_GWMFID, (char*)gmmp_DeviceID, 1);
		if ( nRet!=0 )
		{
			return WXCODE_FAILURE;
		}
	}

	return WXCODE_SUCCESS;
}

UINT8 WXCmd_GMMPDATA(UINT8 *ptr)
{
	UINT8 *p;
	UINT32 buff1 = 0;
	UINT8 status;
	extern char g_pszMessage[256];

	if ( strcmp((char*)ptr, "?") == 0 )
	{
		W_RSP("%s\r\n", g_pszMessage);
		return WXCODE_SUCCESS;
	}

	p = WXParse_NextParamGet(&ptr);
	if ( !p )							return WXCODE_EINVAL;
	status = WXParse_Int(p, &buff1);
	if ( status != WXCODE_SUCCESS )		return WXCODE_EINVAL;
	if ( buff1!=0 && buff1!=1 )			return WXCODE_EINVAL;

	p = ptr;
	if (!p)		return WXCODE_EINVAL;
	strcpy(g_pszMessage, (char*)p);

	if ( buff1==1 )
	{
		int GW_Delivery();
		GW_Delivery();
	}

	return WXCODE_SUCCESS;
}

UINT8 WXCmd_GMMPOPT(UINT8 *ptr)
{
	UINT8 *p;
	UINT32 buff1 = 0, buff2 = 0, buff3 = 0, buff4 = 0;
	UINT8 status;

	p = WXParse_NextParamGet(&ptr);
	if ( !p )							return WXCODE_EINVAL;
	status = WXParse_Int(p, &buff1);
	if ( status != WXCODE_SUCCESS )		return WXCODE_EINVAL;

	p = WXParse_NextParamGet(&ptr);
	if ( !p )							return WXCODE_EINVAL;
	status = WXParse_Int(p, &buff2);
	if ( status != WXCODE_SUCCESS )		return WXCODE_EINVAL;

	p = WXParse_NextParamGet(&ptr);
	if ( p )	status = WXParse_Hex(p, &buff3);

	p = WXParse_NextParamGet(&ptr);
	if ( p )	status = WXParse_Hex(p, &buff4);

	if ( buff1==10 )
	{
		extern char g_bPrintfDebug;
		extern char g_bPrintfLog;
		if ( buff2==1 )
		{
			g_bPrintfDebug = buff3;
			g_bPrintfLog = buff4;
		}
		else
		{
			W_RSP("%d,%d\r\n", g_bPrintfDebug, g_bPrintfLog);
		}
	}
	else if ( buff1==11 )
	{
		extern int g_nTimerHB;
		if ( buff2==1 )
		{
			g_nTimerHB = buff2;
		}
		else
		{
			W_RSP("%d\r\n", g_nTimerHB);
		}
	}
	else if ( buff1==12 )
	{
		extern int g_nTimerDelivery;
		if ( buff2==1 )
		{
			g_nTimerDelivery = buff2;
		}
		else
		{
			W_RSP("%d\r\n", g_nTimerDelivery);
		}
	}
	else
	{
		return WXCODE_EINVAL;
	}

	return WXCODE_SUCCESS;
}

