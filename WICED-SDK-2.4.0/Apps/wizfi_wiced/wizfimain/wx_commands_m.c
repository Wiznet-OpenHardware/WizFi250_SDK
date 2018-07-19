// kaizen works
#include "wx_defines.h"
#include "platform_dct.h"
#include "wiced_dct.h"

#if 1	// kaizen 20130514 ID1048 - PS(Power Save) Function for MCU and Wi-Fi.
#include "wiced_platform.h"
static wiced_timed_event_t powersave_timeout_event;
wiced_result_t powersave_timeout( void* arg );
#endif

extern UINT8 web_server_status;	// kaizen 20130731 ID1107 This variable is web server's running status

UINT8 WXCmd_MPROF(UINT8 *ptr)
{
	if ( strcmp((char*)ptr, "VD") == 0 || strcmp((char*)ptr, "vd") == 0 )
	{
		DisplayWTProfile(((WT_PROFILE*)wiced_dct_get_app_section()));
		return WXCODE_SUCCESS;
	}

	if ( strcmp((char*)ptr, "VG") == 0 || strcmp((char*)ptr, "vg") == 0 )
	{
		DisplayWTProfile(&g_wxProfile);
		return WXCODE_SUCCESS;
	}

	if ( strcmp((char*)ptr, "L") == 0 || strcmp((char*)ptr, "l") == 0 )
	{
		Load_Profile();
		return WXCODE_SUCCESS;
	}

	if ( strcmp((char*)ptr, "S") == 0 || strcmp((char*)ptr, "s") == 0 )
	{
		Save_Profile();
		// kaizen 20130404
		Apply_To_Wifi_Dct();
		return WXCODE_SUCCESS;
	}

	return WXCODE_EINVAL;
}

UINT8 WXCmd_MFDEF(UINT8 *ptr)
{
	if ( strcmp((char*)ptr, "FR") == 0 || strcmp((char*)ptr,"fr") == 0 )
	{
		Default_Profile();
		Save_Profile();

		WXS2w_StatusNotify(WXCODE_SUCCESS, 0);
#if 1	// kaizen 20130520 ID1039 - When Module Reset, Socket close & Disassociation
		WXS2w_SystemReset();
#else
		NVIC_SystemReset();
#endif
	}
#if 1 //MikeJ 130410 ID1022 - Reset Response
	return WXCODE_EINVAL;
#else
	return WXCODE_SUCCESS;
#endif
}

UINT8 WXCmd_MRESET(UINT8 *ptr)
{
	WXS2w_StatusNotify(WXCODE_SUCCESS, 0);
#if 1	// kaizen 20130520 ID1039 - When Module Reset, Socket close & Disassociation
	WXS2w_SystemReset();
#else
	NVIC_SystemReset();
#endif
	return WXCODE_SUCCESS;
}

UINT8 WXCmd_MMSG(UINT8 *ptr)
{
	UINT8 *p;
	UINT8 status;
	UINT32 param;

	if ( strcmp((char*)ptr, "?") == 0 )
	{
		// Message Level(1:Response 2:Event 3:Debug)
		W_RSP("%d\r\n", g_wxProfile.msgLevel );
		return WXCODE_SUCCESS;
	}

	p = WXParse_NextParamGet(&ptr);
	if (!p)		return WXCODE_EINVAL;
	status = WXParse_Int(p, &param);
	if (status != WXCODE_SUCCESS )					return WXCODE_EINVAL;
	if( param != 1 && param != 2 && param != 3 )	return WXCODE_EINVAL;

	g_wxProfile.msgLevel = param;

	return WXCODE_SUCCESS;
}

UINT8 WXCmd_MMAC(UINT8 *ptr)
{
	UINT8 *p;
	UINT8 status;
	wiced_mac_t mac;

	wiced_wifi_get_mac_address( &mac );

	if ( strcmp((char*)ptr, "?") == 0 )
	{
		W_RSP("%02X:%02X:%02X:%02X:%02X:%02X\r\n", mac.octet[0],mac.octet[1],mac.octet[2],mac.octet[3],mac.octet[4],mac.octet[5]);
		return WXCODE_SUCCESS;
	}

	p = WXParse_NextParamGet(&ptr);
	if (p)
	{
		status = WXParse_Mac(p, mac.octet);
		if ( status != WXCODE_SUCCESS )		return WXCODE_EINVAL;
	}

	g_wxProfile.mac = mac;
	wiced_wifi_set_mac_address(mac);

	// kaizen 20130404
	Save_Profile();
	Apply_To_Wifi_Dct();

#if 1	// kaizen 20130611 ID1076 - When execute command to need system reset, WizFi250 should print [OK] message.
	WXS2w_StatusNotify(WXCODE_SUCCESS, 0);
#endif

	wiced_rtos_delay_milliseconds(100);

#if 1	// kaizen 20130520 ID1039 - When Module Reset, Socket close & Disassociation
	WXS2w_SystemReset();
#else
	NVIC_SystemReset();
#endif

	return WXCODE_SUCCESS;
}

// sekim 20130412 WizFi250 Version
UINT8 WXCmd_MINFO(UINT8 *ptr)
{
#if 0 //MikeJ 130702 ID1093 - Adjust Response Format
	W_RSP("Firmware : %s (SDK %s)\r\n", WIZFI250_FW_VERSION, WICED_VERSION);
	W_RSP("Platform : %s\r\n", WIZFI250_HW_VERSION);
#else
	// sekim 20130924 remove space in AT+MINFO
	W_RSP("FW version/HW version\r\n");
	W_RSP("%s/%s\r\n", WIZFI250_FW_VERSION, WIZFI250_HW_VERSION);
#endif

	return WXCODE_SUCCESS;
}

#if 1	// kaizen 20130514 ID1048 - PS(Power Save) Function for MCU and Wi-Fi. The STM32 power save mode used by WICED is Stop Mode.
UINT8 WXCmd_MMCUPS(UINT8 *ptr)
{
	UINT8 *p;
	UINT8 status;
	UINT32 mode, timeout;

	p = WXParse_NextParamGet(&ptr);
	if (!p)		return WXCODE_EINVAL;
	status = WXParse_Int(p, &mode);
	if (status != WXCODE_SUCCESS )						return WXCODE_EINVAL;
	if( mode != PS_DISABLE && mode != PS_ENABLE )		return WXCODE_EINVAL;

	if( mode == PS_ENABLE )
	{
		p = WXParse_NextParamGet(&ptr);
		if (!p)		return WXCODE_EINVAL;
		status = WXParse_Int(p, &timeout);
		if (status != WXCODE_SUCCESS )								return WXCODE_EINVAL;
		if (timeout < 100 || timeout > 60 * 60 * 1000)				return WXCODE_EINVAL;

		W_DBG("Enable Power Save Mode");
		wiced_platform_mcu_enable_powersave();
	    wiced_rtos_register_timed_event( &powersave_timeout_event, WICED_HARDWARE_IO_WORKER_THREAD, &powersave_timeout, timeout, 0 );
	}

	return WXCODE_SUCCESS;
}

wiced_result_t powersave_timeout( void* arg )
{
	wiced_result_t result;

	W_DBG("Disable Power Save Mode");
    wiced_platform_mcu_disable_powersave();

	result = wiced_rtos_deregister_timed_event( &powersave_timeout_event );

    return result;
}
#endif

#if 1	// kaizen 20130731 ID1108	Added exception routine at WXCmd_MWIFIPS
UINT32 wifi_ps_current_status = PS_DISABLE;
UINT32 wifi_ps_delay = 0;
UINT8 WXCmd_MWIFIPS(UINT8 *ptr)
{
	UINT8  *p;
	UINT8  status;
	UINT32 mode, return_to_sleep_delay;


	if ( strcmp((char*)ptr, "?") == 0 )
	{
		W_RSP("%d,%d\r\n",wifi_ps_current_status,wifi_ps_delay);
		return WXCODE_SUCCESS;
	}

	if (wiced_wifi_is_ready_to_transceive(WICED_STA_INTERFACE) != WICED_SUCCESS)
	{
		W_DBG("For using Wi-Fi Power Save Mode, WizFi250 have to connect to Access Point");
		return WXCODE_FAILURE;
	}

	p = WXParse_NextParamGet(&ptr);
	if (!p)		return WXCODE_EINVAL;
	status = WXParse_Int(p, &mode);
	if (status != WXCODE_SUCCESS )		return WXCODE_EINVAL;
	if( mode != PS_DISABLE && mode != PS_ENABLE && mode != PS_THR_ENABLE )		return WXCODE_EINVAL;

	if( mode == PS_ENABLE )
	{
		if ( wifi_ps_current_status == PS_ENABLE )
		{
			W_DBG("WizFi250 already enable Wi-Fi Power Save Mode");
			return WXCODE_SUCCESS;
		}

		W_DBG("Enable Wi-Fi Power Save Mode");
		wifi_ps_current_status = PS_ENABLE;
		wifi_ps_delay = 0;

		if ( wiced_wifi_enable_powersave() != WICED_SUCCESS )
		{
			W_DBG("Error : wiced_wifi_enable_powersave()");
			return WXCODE_FAILURE;
		}
		if ( wiced_network_suspend() != WICED_SUCCESS )
		{
			W_DBG("Error : wiced_network_suspend()");
			return WXCODE_FAILURE;
		}
	}
	else if ( mode == PS_THR_ENABLE )
	{
		p = WXParse_NextParamGet(&ptr);
		if (!p)		return WXCODE_EINVAL;
		status = WXParse_Int(p, &return_to_sleep_delay);
		if (status != WXCODE_SUCCESS )											return WXCODE_EINVAL;
		if ( return_to_sleep_delay % 10 != 0 || return_to_sleep_delay > 100 )	return WXCODE_EINVAL;

		if ( wifi_ps_current_status == PS_THR_ENABLE )
		{
			W_DBG("WizFi250 already enable Wi-Fi Power Save Mode while attempting to maximize throughtput");
			return WXCODE_SUCCESS;
		}

		W_DBG("Enable Wi-Fi Power Save Mode with throughput");
		wifi_ps_current_status = PS_THR_ENABLE;
		wifi_ps_delay = return_to_sleep_delay;

		if ( wiced_wifi_enable_powersave_with_throughput( (UINT8)return_to_sleep_delay ) != WICED_SUCCESS )
		{
			W_DBG("Error : wiced_wifi_enable_powersave_with_throughput");
			return WXCODE_FAILURE;
		}
		if ( wiced_network_suspend() != WICED_SUCCESS )
		{
			W_DBG("Error : wiced_network_suspend()");
			return WXCODE_FAILURE;
		}
	}
	else if( mode == PS_DISABLE )
	{
		if ( wifi_ps_current_status == PS_DISABLE )
		{
			W_DBG("WizFi250 already disable Wi-Fi Power Save Mode");
			return WXCODE_SUCCESS;
		}

		W_DBG("Disable Wi-Fi Power Save Mode");
		wifi_ps_current_status = PS_DISABLE;

		if ( wiced_wifi_disable_powersave() != WICED_SUCCESS )
		{
			W_DBG("Error : wiced_wifi_disable_powersave()");
			return WXCODE_FAILURE;
		}
		if ( wiced_network_resume() != WICED_SUCCESS )
		{
			W_DBG("Error : wiced_network_suspend()");
			return WXCODE_FAILURE;
		}
	}

	return WXCODE_SUCCESS;
}
#else
UINT8 WXCmd_MWIFIPS(UINT8 *ptr)
{
	UINT8  *p;
	UINT8  status;
	UINT32 mode, return_to_sleep_delay;

	p = WXParse_NextParamGet(&ptr);
	if (!p)		return WXCODE_EINVAL;
	status = WXParse_Int(p, &mode);
	if (status != WXCODE_SUCCESS )		return WXCODE_EINVAL;
	if( mode != PS_DISABLE && mode != PS_ENABLE && mode != PS_THR_ENABLE )		return WXCODE_EINVAL;

	if( mode == PS_ENABLE )
	{
		W_DBG("Enable Wi-Fi Power Save Mode");
		if ( wiced_wifi_enable_powersave() != WICED_SUCCESS )
		{
			W_DBG("Error : wiced_wifi_enable_powersave()");
			return WXCODE_FAILURE;
		}
		if ( wiced_network_suspend() != WICED_SUCCESS )
		{
			W_DBG("Error : wiced_network_suspend()");
			return WXCODE_FAILURE;
		}
	}
	else if ( mode == PS_THR_ENABLE )
	{
		p = WXParse_NextParamGet(&ptr);
		if (!p)		return WXCODE_EINVAL;
		status = WXParse_Int(p, &return_to_sleep_delay);
		if (status != WXCODE_SUCCESS )											return WXCODE_EINVAL;
		if ( return_to_sleep_delay % 10 != 0 || return_to_sleep_delay > 100 )	return WXCODE_EINVAL;

		W_DBG("Enable Wi-Fi Power Save Mode with throughput");
		if ( wiced_wifi_enable_powersave_with_throughput( (UINT8)return_to_sleep_delay ) != WICED_SUCCESS )
		{
			W_DBG("Error : wiced_wifi_enable_powersave_with_throughput");
			return WXCODE_FAILURE;
		}
		if ( wiced_network_suspend() != WICED_SUCCESS )
		{
			W_DBG("Error : wiced_network_suspend()");
			return WXCODE_FAILURE;
		}
	}
	else if( mode == PS_DISABLE )
	{
		W_DBG("Disable Wi-Fi Power Save Mode");
		if ( wiced_wifi_disable_powersave() != WICED_SUCCESS )
		{
			W_DBG("Error : wiced_wifi_disable_powersave()");
			return WXCODE_FAILURE;
		}
		if ( wiced_network_resume() != WICED_SUCCESS )
		{
			W_DBG("Error : wiced_network_suspend()");
			return WXCODE_FAILURE;
		}
	}

	return WXCODE_SUCCESS;
}
#endif

#if 1	// kaizen 20130514 ID1047 - WiFi Tx Power Function
UINT8 WXCmd_MTXP(UINT8 *ptr)
{
	UINT8 *p;
	UINT8 status;
	UINT32 dbm = 0;

	if ( strcmp((char*)ptr, "?") == 0 )
	{
		wiced_wifi_get_tx_power((UINT8*)&dbm);
		W_RSP("%d dBm", dbm);

		return WXCODE_SUCCESS;
	}

	p = WXParse_NextParamGet(&ptr);
	if (!p)		return WXCODE_EINVAL;
	status = WXParse_Int(p, &dbm);
	if (status != WXCODE_SUCCESS )			return WXCODE_EINVAL;
	if ( dbm < 0 || dbm > 31 )				return WXCODE_EINVAL;

	if( wiced_wifi_set_tx_power((UINT8)dbm) != WICED_SUCCESS )
	{
		W_DBG("Error : wiced_wifi_set_tx_power()");
		return WXCODE_FAILURE;
	}

	return WXCODE_SUCCESS;
}
#endif

void Default_Profile()
{
#if 1	// kaizen 20131018 ID1129 - Add FW_VERSION Check routin
	strcpy((char*)g_wxProfile.fw_version			, WIZFI250_FW_VERSION	   );
#endif
	g_wxProfile.msgLevel                            = DCTD_MSGLEVEL             ;
#if 1 //MikeJ 130410 ID1034 - Add ECHO on/off function
    g_wxProfile.echo_mode                           = DCTD_ECHO_MODE            ;
#endif

    /////////////////////////////////////////////////////////////////////////////////
    // sekim 20140714 Encored Customizing
    //g_wxProfile.usart_init_structure.USART_BaudRate = DCTD_USART_BAUDRATE;
	if( CHKCUSTOM("ENCORED") )	g_wxProfile.usart_init_structure.USART_BaudRate = 38400;
	// sekim 20140929 ID1187 URIEL Customizing 2 : 9600, AT+WSTAT, W_AP_xxxxxxxx
	else if( CHKCUSTOM("URIEL") )	g_wxProfile.usart_init_structure.USART_BaudRate = 9600;
	////////////////////////////////////////////////////////////////////////////////////
	// sekim 20141218 POSBANK Customizing
	else if( CHKCUSTOM("POSBANK") )
	{
		g_wxProfile.spi_mode = 1;
		g_wxProfile.usart_init_structure.USART_BaudRate = 1843200;
		g_wxProfile.usart_init_structure.USART_HardwareFlowControl = USART_HardwareFlowControl_RTS_CTS;
	}
	////////////////////////////////////////////////////////////////////////////////////
	else						g_wxProfile.usart_init_structure.USART_BaudRate = DCTD_USART_BAUDRATE;

	if( CHKCUSTOM("ENCORED") )	memcpy(g_wxProfile.s2web_main, "1000000", 7);
	else						memcpy(g_wxProfile.s2web_main, "1111111", 7);
	/////////////////////////////////////////////////////////////////////////////////

    g_wxProfile.usart_init_structure.USART_WordLength          = DCTD_USART_WORDLENGTH;
    g_wxProfile.usart_init_structure.USART_Parity              = DCTD_USART_PARITY    ;
    g_wxProfile.usart_init_structure.USART_StopBits            = DCTD_USART_STOPBITS  ;
    g_wxProfile.usart_init_structure.USART_HardwareFlowControl = DCTD_USART_HWFC      ;
    g_wxProfile.usart_init_structure.USART_Mode                = DCTD_USART_MODE      ;

	////////////////////////////////////////////////////////////////////////////////////
	// sekim 20150106 POSBANK Customizing
	if( CHKCUSTOM("POSBANK") )
	{
		g_wxProfile.usart_init_structure.USART_HardwareFlowControl = USART_HardwareFlowControl_RTS_CTS;
	}
	////////////////////////////////////////////////////////////////////////////////////

    // sekim 20131125 Add SPI Interface
    g_wxProfile.spi_stdio                           = DCTD_SPI_STDIO            ;
    g_wxProfile.spi_mode                            = DCTD_SPI_MODE             ;

    strcpy((char*)g_wxProfile.wifi_ssid             , DCTD_WIFI_SSID           );
    strcpy((char*)g_wxProfile.wifi_bssid            , DCTD_WIFI_BSSID          );
    g_wxProfile.wifi_channel                        = DCTD_WIFI_CHANNEL         ;
    g_wxProfile.wifi_mode                           = DCTD_WIFI_MODE            ;
    g_wxProfile.wifi_authtype                       = DCTD_WIFI_AUTHTYPE        ;
    strcpy((char*)g_wxProfile.wifi_keydata          , DCTD_WIFI_KEYDATA        );
#if 1 //MikeJ 130408 ID1014 - Key buffer overflow
	g_wxProfile.wifi_keylen                         = DCTD_WIFI_KEYLEN          ;
#endif
    g_wxProfile.wifi_dhcp                           = DCTD_WIFI_DHCP            ;
    strcpy((char*)g_wxProfile.wifi_ip               , DCTD_WIFI_IP             );
    strcpy((char*)g_wxProfile.wifi_mask             , DCTD_WIFI_MASK           );
    strcpy((char*)g_wxProfile.wifi_gateway          , DCTD_WIFI_GATEWAY        );
    strcpy((char*)g_wxProfile.wifi_countrycode      , DCTD_WIFI_COUNTRYCODE    );
    strcpy((char*)g_wxProfile.scon_opt1             , DCTD_SCON_OPT1           );
    strcpy((char*)g_wxProfile.scon_opt2             , DCTD_SCON_OPT2           );
#if 0	// kaizen 20130703 -ID1095 Change the DCTD_SCON_OPT2's initial value to TSN, Because not use DCTD_SCON_OPT3 in source code
    strcpy((char*)g_wxProfile.scon_opt3             , DCTD_SCON_OPT3           );
#endif
    strcpy((char*)g_wxProfile.scon_remote_ip        , DCTD_SCON_REMOTE_IP      );
    g_wxProfile.scon_remote_port                    = DCTD_SCON_REMOTE_PORT     ;
    g_wxProfile.scon_local_port                     = DCTD_SCON_LOCAL_PORT      ;
    g_wxProfile.scon_datamode                       = DCTD_SCON_DATAMODE        ;
    strcpy((char*)g_wxProfile.xrdf_main             , DCTD_XRDF_MAIN           );
    strcpy((char*)g_wxProfile.xrdf_data             , DCTD_XRDF_DATA           );
    g_wxProfile.timer_autoesc                       = DCTD_TIMER_AUTOESC        ;

#if 0	// kaizen 20130924 initialize mac address problem when set factory default command
    g_wxProfile.mac.octet[0] = DCTD_MAC_00;
    g_wxProfile.mac.octet[1] = DCTD_MAC_01;
    g_wxProfile.mac.octet[2] = DCTD_MAC_02;
    g_wxProfile.mac.octet[3] = DCTD_MAC_03;
    g_wxProfile.mac.octet[4] = DCTD_MAC_04;
    g_wxProfile.mac.octet[5] = DCTD_MAC_05;
#endif

    strcpy((char*)g_wxProfile.ap_mode_ssid          , DCTD_AP_MODE_SSID        );
    g_wxProfile.ap_mode_channel                     = DCTD_AP_MODE_CHANNEL      ;
    g_wxProfile.ap_mode_authtype                    = DCTD_AP_MODE_AUTHTYPE     ;
    strcpy((char*)g_wxProfile.ap_mode_keydata       , DCTD_AP_MODE_KEYDATA     );
#if 1 //MikeJ 130408 ID1014 - Key buffer overflow
	g_wxProfile.ap_mode_keylen                      = DCTD_AP_MODE_KEYLEN       ;
#endif
    strcpy((char*)g_wxProfile.web_server_opt		, DCTD_WEB_SERVER_OPT		);

#if 1 // kaizen 20130814 Modified AT+FGPIO function
    int i;
    UINT8 gpio_num[USER_GPIO_MAX_COUNT] = DCTD_GPIO_NUM;
    for(i=0; i<USER_GPIO_MAX_COUNT; i++)
    {
    	g_wxProfile.user_gpio_conifg[i].mode				= DCTD_GPIO_MODE;
    	g_wxProfile.user_gpio_conifg[i].gpio_num 			= gpio_num[i];
    	g_wxProfile.user_gpio_conifg[i].gpio_config_value 	= DCTD_GPIO_CONFIG_VALUE;
    	g_wxProfile.user_gpio_conifg[i].gpio_set_value 		= DCTD_GPIO_SET_VALUE;
    }
#endif

#if 1	// kaizen 20131112 ID1137 Added AT Command of MCWUI(Change Web server User Information)
    strcpy((char*)g_wxProfile.user_id, (char*)DCTD_USER_ID);
    strcpy((char*)g_wxProfile.user_password, (char*)DCTD_USER_PASSWORD);

    g_wxProfile.gpio_wifi_auto_conn = 0;
#endif

    // sekim 20131212 ID1141 add g_socket_extx_option
    g_wxProfile.socket_ext_option1                     = DCTD_SOCKET_EXT_OPTION1    ;
    g_wxProfile.socket_ext_option2                     = DCTD_SOCKET_EXT_OPTION2    ;
    g_wxProfile.socket_ext_option3                     = DCTD_SOCKET_EXT_OPTION3    ;
    g_wxProfile.socket_ext_option4                     = DCTD_SOCKET_EXT_OPTION4    ;
    g_wxProfile.socket_ext_option5                     = DCTD_SOCKET_EXT_OPTION5    ;
    g_wxProfile.socket_ext_option6                     = DCTD_SOCKET_EXT_OPTION6    ;
    // sekim 20140929 ID1188 Data-Idle-Auto-Reset (Autonix)
    g_wxProfile.socket_ext_option7                     = DCTD_SOCKET_EXT_OPTION7    ;
    // sekim 20150519 add socket_ext_option8. Reset if not connected in TCP Client/Service
    g_wxProfile.socket_ext_option8                     = DCTD_SOCKET_EXT_OPTION8    ;
    // sekim XXXX 20160119 Add socket_ext_option9 for TCP Server Multi-Connection
    g_wxProfile.socket_ext_option9                     = DCTD_SOCKET_EXT_OPTION9    ;


    // sekim 20140710 ID1179 add wcheck_option
    g_wxProfile.wcheck_option1                         = DCTD_WCHECK_OPTION1;
    g_wxProfile.wcheck_option2                         = DCTD_WCHECK_OPTION2;
    g_wxProfile.wcheck_option3                         = DCTD_WCHECK_OPTION3;
    // sekim add wcheck_option4 for Smart-ANT)
    g_wxProfile.wcheck_option4                         = DCTD_WCHECK_OPTION4;

    // sekim 20140214 ID1163 Add AT+WANT (execpt Antenna)
    //g_wxProfile.antenna_type = DCTD_ANTENNA_TYPE;
    // sekim 20150209 URIEL Antenna Default
    if( CHKCUSTOM("URIEL") )	g_wxProfile.antenna_type = 1;

    // kaizen 20140408 ID1154, ID1166 Add AT+FWEBSOPT
    g_wxProfile.show_hidden_ssid		= DCTD_SHOW_HIDDEN_SSID;
    g_wxProfile.show_only_english_ssid	= DCTD_SHOW_ONLY_ENGLISH_SSID;
    g_wxProfile.show_rssi_range			= DCTD_SHOW_RSSI_RAGE;

    // sekim 20150107 added WXCmd_WRFMODE
    g_wxProfile.rfmode1 = 0;

    // sekim 20150416 add AT+MEVTFORM, AT+MEVTDELAY for Coway
    memcpy(g_wxProfile.event_msg_format, DCTD_EVENT_MSG_FORMAT, sizeof(g_wxProfile.event_msg_format) );
    g_wxProfile.event_msg_delay			= DCTD_EVENT_MSG_DELAY;

    // sekim 20150508 Coway ReSend <Connect Event Message>
    g_wxProfile.cowaya_check_time		= DCTD_COWAYA_CHECK_TIME;
    strcpy((char*)g_wxProfile.cowaya_check_data, (char*)DCTD_COWAYA_CHECK_DATA);

    // sekim 20150616 add AT+SDNAME
    memset(g_wxProfile.domainname_for_scon, 0, sizeof(g_wxProfile.domainname_for_scon));

	// sekim 20150622 add FFTPSET/FFTPCMD for choyoung
    /*
    memset(g_wxProfile.ftpset_ip, 0, sizeof(g_wxProfile.ftpset_ip));
    g_wxProfile.ftpset_port = 0;
    memset(g_wxProfile.ftpset_id, 0, sizeof(g_wxProfile.ftpset_id));
    memset(g_wxProfile.ftpset_pw, 0, sizeof(g_wxProfile.ftpset_pw));
    */

    if( CHKCUSTOM("COWAYA") )
    {
    	g_wxProfile.socket_ext_option6 = 30;
    	g_wxProfile.event_msg_delay = 100;
        g_wxProfile.cowaya_check_time = 20;
        strcpy((char*)g_wxProfile.cowaya_check_data, "A1001");

        // sekim 20150902 Add recovery DCT function
        g_wxProfile.mext1_1 = 11;
        g_wxProfile.mext1_2 = 240;
    }

    // sekim 20150616 Coway Mega Default Setting
    if( CHKCUSTOM("COWAYB") )
    {
    	g_wxProfile.socket_ext_option6 = 30;
    	g_wxProfile.socket_ext_option8 = 600;
    	//g_wxProfile.event_msg_delay = 100;
        //g_wxProfile.cowaya_check_time = 20;
        //strcpy((char*)g_wxProfile.cowaya_check_data, "A1001");
        strcpy((char*)g_wxProfile.domainname_for_scon, "iocare.coway.co.kr");
    }
}

void Save_Profile()
{
	// sekim 20133081 USART_Cmd(USART1, DISABLE) in wiced_dct_write_xxxx
	USART_Cmd(USART1, DISABLE);
	wiced_dct_write_app_section( &g_wxProfile, sizeof(WT_PROFILE) );
	USART_Cmd(USART1, ENABLE);
}

void Load_Profile()
{
	wiced_dct_read_app_section( &g_wxProfile, sizeof(WT_PROFILE) );
}

#if 0 // kaizen 20130712 ID1099 For Setting TCP/UDP Connection using web server, Do not need this function
void Apply_To_APP_Dct()
{
	platform_dct_wifi_config_t	wifi_config;
	wiced_config_ap_entry_t *	station_entry;
	wiced_config_soft_ap_t *	ap_entry;

	wiced_dct_read_wifi_config_section( &wifi_config );
	station_entry	= &wifi_config.stored_ap_list[0];
	ap_entry		= &wifi_config.soft_ap_settings;

	// Station Mode로 동작하기 위한 정보를 APP_DCT에 반영
#if 1 //MikeJ 130408 ID1012 - Allow 32 characters SSID
	strncpy( (char*)g_wxProfile.wifi_ssid, (char*)station_entry->details.SSID.val, WX_MAX_SSID_LEN);
#else
	strcpy( (char*)g_wxProfile.wifi_ssid, (char*)station_entry->details.SSID.val);
#endif
	sprintf( (char*)g_wxProfile.wifi_bssid, "%02x:%02x:%02x:%02x:%02x:%02x",
		station_entry->details.BSSID.octet[0], station_entry->details.BSSID.octet[1], 
		station_entry->details.BSSID.octet[2], station_entry->details.BSSID.octet[3], 
		station_entry->details.BSSID.octet[4], station_entry->details.BSSID.octet[5] );
	g_wxProfile.wifi_channel = (UINT8)station_entry->details.channel;
	g_wxProfile.wifi_authtype = (UINT32)station_entry->details.security;
#if 1 //MikeJ 130408 ID1014 - Key buffer overflow
	memcpy ( (char*)g_wxProfile.wifi_keydata, station_entry->security_key, station_entry->security_key_length );
	g_wxProfile.wifi_keylen = station_entry->security_key_length;
#else
	strcpy ( (char*)g_wxProfile.wifi_keydata, (char*)station_entry->security_key);
#endif

	// AP Mode로 동작하기 위한 정보를 APP DCT에 반영
#if 1 //MikeJ 130408 ID1012 - Allow 32 characters SSID
	strncpy( (char*)g_wxProfile.ap_mode_ssid, (char*)ap_entry->SSID.val, WX_MAX_SSID_LEN);
#else
	strcpy( (char*)g_wxProfile.ap_mode_ssid, (char*)ap_entry->SSID.val );
#endif
	g_wxProfile.ap_mode_channel = ap_entry->channel;
	g_wxProfile.wifi_authtype = ap_entry->security;
#if 1 //MikeJ 130408 ID1014 - Key buffer overflow
	memcpy ( (char*)g_wxProfile.ap_mode_keydata, ap_entry->security_key, ap_entry->security_key_length );
	g_wxProfile.ap_mode_keylen = ap_entry->security_key_length;
#else
	strcpy( (char*)g_wxProfile.ap_mode_keydata, (char*)ap_entry->security_key);
#endif

	Save_Profile();
}
#endif

void Apply_To_Wifi_Dct()
{
	platform_dct_wifi_config_t	wifi_config;
	wiced_config_ap_entry_t *	station_entry;
	wiced_config_soft_ap_t *	ap_entry;


	wiced_dct_read_wifi_config_section( &wifi_config );
	station_entry	= &wifi_config.stored_ap_list[0];
	ap_entry		= &wifi_config.soft_ap_settings;

	// Station Mode로 동작하기 위한 정보를 wifi_config_dct에 반영
#if 1 //MikeJ 130408 ID1012 - Allow 32 characters SSID
#if 0 //MikeJ 130806 ID1116 - Couldn't display 32 characters SSID
{
	UINT8 i;
	for(i=0; i<WX_MAX_SSID_LEN; i++) {
		if(g_wxProfile.wifi_ssid[i] == '\0') break;
	}
	station_entry->details.SSID.len = i;
	memcpy( (char*)station_entry->details.SSID.val, (char*)g_wxProfile.wifi_ssid, i );
}
#else
	station_entry->details.SSID.len = strlen((char*)g_wxProfile.wifi_ssid);
	memcpy( (char*)station_entry->details.SSID.val, (char*)g_wxProfile.wifi_ssid, station_entry->details.SSID.len );
#endif
#else
	station_entry->details.SSID.len = strlen( (char*)g_wxProfile.wifi_ssid );
	strcpy( (char*)station_entry->details.SSID.val, (char*)g_wxProfile.wifi_ssid );
#endif
	station_entry->details.BSSID = str_to_mac( (char*)g_wxProfile.wifi_bssid );
	station_entry->details.channel = (UINT8)g_wxProfile.wifi_channel;
	station_entry->details.security = (wiced_security_t)g_wxProfile.wifi_authtype;
#if 1 //MikeJ 130408 ID1014 - Key buffer overflow
	memcpy ( station_entry->security_key, (char*)g_wxProfile.wifi_keydata, g_wxProfile.wifi_keylen );
	station_entry->security_key_length = g_wxProfile.wifi_keylen;
#else
	strcpy( station_entry->security_key, (char*)g_wxProfile.wifi_keydata );
#endif

	// AP Mode로 동작하기 위한 정보를 wifi_config_dct에 반영
#if 1 //MikeJ 130408 ID1012 - Allow 32 characters SSID
#if 0 //MikeJ 130806 ID1116 - Couldn't display 32 characters SSID
{
	UINT8 i;
	for(i=0; i<WX_MAX_SSID_LEN; i++) {
		if(g_wxProfile.ap_mode_ssid[i] == '\0') break;
	}
	ap_entry->SSID.len = i;
	memcpy( (char*)ap_entry->SSID.val, (char*)g_wxProfile.ap_mode_ssid, i );
}
#else
	ap_entry->SSID.len = strlen((char*)g_wxProfile.ap_mode_ssid);
	memcpy( (char*)ap_entry->SSID.val, (char*)g_wxProfile.ap_mode_ssid, ap_entry->SSID.len );
#endif
#else
	ap_entry->SSID.len = strlen( (char*)g_wxProfile.ap_mode_ssid );
	strcpy( (char*)ap_entry->SSID.val, (char*)g_wxProfile.ap_mode_ssid );
#endif
	ap_entry->channel = (UINT8)g_wxProfile.ap_mode_channel;
#if 0 //MikeJ 130806 ID1121 - WSEC profile load problem at AP mode
	ap_entry->security = (wiced_security_t)g_wxProfile.wifi_authtype;
#else
	ap_entry->security = (wiced_security_t)g_wxProfile.ap_mode_authtype;
#endif
#if 1 //MikeJ 130408 ID1014 - Key buffer overflow
	memcpy ( ap_entry->security_key, (char*)g_wxProfile.ap_mode_keydata, g_wxProfile.ap_mode_keylen );
	ap_entry->security_key_length = g_wxProfile.ap_mode_keylen;
#else
	strcpy( (char*)ap_entry->security_key, (char*)g_wxProfile.ap_mode_keydata );
#endif


	// kaizen 20130404
	wifi_config.country_code = MK_CNTRY(g_wxProfile.wifi_countrycode[0],g_wxProfile.wifi_countrycode[1],0);
	wifi_config.mac_address  = g_wxProfile.mac;


	// sekim 20133081 USART_Cmd(USART1, DISABLE) in wiced_dct_write_xxxx
	USART_Cmd(USART1, DISABLE);
	wiced_dct_write_wifi_config_section( &wifi_config );
	USART_Cmd(USART1, ENABLE);
}

// kaizen 20130424 1046 Modified Display Format
void DisplayWTProfile(WT_PROFILE* pProfile)
{
	UINT8 i;
	wiced_mac_t mac;
	char buffDisplay[100];

	if(pProfile->wifi_mode == STATION_MODE)
	{
		W_RSP("+WSET=%d,%s,%s,%d\r\n",pProfile->wifi_mode,pProfile->wifi_ssid,pProfile->wifi_bssid,pProfile->wifi_channel);
		W_RSP("+WSEC=%d,%s,",pProfile->wifi_mode,authtype_to_str(buffDisplay,pProfile->wifi_authtype));

		if(pProfile->wifi_authtype == WICED_SECURITY_WEP_PSK) {							// WEP Security
#if 0 //MikeJ 1300806 ID1117 - WEP Security problem
			for(i=0; i<pProfile->wifi_keylen; i++) {									// Check if key can be able to print
				if(!isprint(pProfile->wifi_keydata[i])) break;
			}
			if(i == pProfile->wifi_keylen) {											// If it can print, just print it
				memcpy( buffDisplay, pProfile->wifi_keydata, pProfile->wifi_keylen );
				buffDisplay[pProfile->wifi_keylen] = '\0';								// Not necessary but Just in case
				W_RSP( "%s\r\n", buffDisplay );
			} else {																	// If not, print as HEX
				for(i=0; i<pProfile->wifi_keylen; i++)
					W_RSP( "%02x ", pProfile->wifi_keydata[i] );
				W_RSP( "\r\n" );
			}
#else
			for(i=0; i<pProfile->wifi_keydata[1]; i++) {									// Check if key can be able to print
				if(!isprint(pProfile->wifi_keydata[i+2])) break;
			}
			if(i == pProfile->wifi_keydata[1]) {											// If it can print, just print it
				memcpy( buffDisplay, pProfile->wifi_keydata+2, pProfile->wifi_keydata[1] );
				buffDisplay[pProfile->wifi_keydata[1]] = '\0';								// Not necessary but Just in case
				W_RSP( "%s\r\n", buffDisplay );
			} else {																	// If not, print as HEX
				for(i=0; i<pProfile->wifi_keydata[1]; i++)
					W_RSP( "%02x ", pProfile->wifi_keydata[i+2] );
				W_RSP( "\r\n" );
			}
#endif
		} else if(pProfile->wifi_authtype & (WPA_SECURITY | WPA2_SECURITY)) {			// WPA Security
			if(pProfile->wifi_keylen < WSEC_MAX_PSK_LEN) {								// 8 ~ 63 ASCII
				pProfile->wifi_keydata[pProfile->wifi_keylen] = '\0';					// Not necessary but Just in case
				W_RSP( "%s\r\n", pProfile->wifi_keydata );
			} else {																	// 64 HEX
				for(i=0; i<pProfile->wifi_keylen; i++)
					W_RSP( "%02x ", pProfile->wifi_keydata[i] );
				W_RSP( "\r\n" );
			}
		} else		W_RSP( "\r\n" );													// OPEN Security
	}
	else if(pProfile->wifi_mode == AP_MODE)
	{
		W_RSP("+WSET=%d,%s,%s,%d\r\n",pProfile->wifi_mode,pProfile->ap_mode_ssid,"",pProfile->ap_mode_channel);
		W_RSP("+WSEC=%d,%s,",pProfile->wifi_mode,authtype_to_str(buffDisplay,pProfile->ap_mode_authtype));

		if(pProfile->ap_mode_authtype == WICED_SECURITY_WEP_PSK)	W_DBG("!! Wrong Security Config !!\r\n");		// WEP Security
		else if(pProfile->ap_mode_authtype & (WPA_SECURITY | WPA2_SECURITY))			// WPA Security
		{
			if(pProfile->ap_mode_keylen < WSEC_MAX_PSK_LEN)
			{
				pProfile->ap_mode_keydata[pProfile->ap_mode_keylen] = '\0';				// Not necessary but Just in case
				W_RSP("%s\r\n",pProfile->ap_mode_keydata);
			}
			else																		// 64 HEX
			{
				for(i=0; i<pProfile->ap_mode_keylen; i++)
					W_RSP("%02x ",pProfile->ap_mode_keydata[i]);

				W_RSP("\r\n");
			}
		}
		else	W_RSP("\r\n");															// OPEN Security
	}

	W_RSP("+WNET=%d,%s,%s,%s\r\n",pProfile->wifi_dhcp, pProfile->wifi_ip, pProfile->wifi_mask, pProfile->wifi_gateway);
	W_RSP("+WREG=%s\r\n",pProfile->wifi_countrycode);									// need check
	W_RSP("+SCON=%s,%s,%s,%d,%d,%d\r\n",pProfile->scon_opt1, pProfile->scon_opt2, pProfile->scon_remote_ip, pProfile->scon_remote_port, pProfile->scon_local_port, pProfile->scon_datamode);
	W_RSP("+SFORM=%c%c%c%c%c%c%c%c%c,%02x,%02x,%02x,%02x,%02x\r\n", pProfile->xrdf_main[0], pProfile->xrdf_main[1],
			pProfile->xrdf_main[2], pProfile->xrdf_main[3], pProfile->xrdf_main[4], pProfile->xrdf_main[5],
			pProfile->xrdf_main[6], pProfile->xrdf_main[7], pProfile->xrdf_main[8], pProfile->xrdf_data[0],
			pProfile->xrdf_data[1], pProfile->xrdf_data[2], pProfile->xrdf_data[3], pProfile->xrdf_data[4]);
	W_RSP("+MMSG=%d\r\n", pProfile->msgLevel );

	wiced_wifi_get_mac_address( &mac );
	W_RSP("+MMAC=%02X:%02X:%02X:%02X:%02X:%02X\r\n", mac.octet[0],mac.octet[1],mac.octet[2],mac.octet[3],mac.octet[4],mac.octet[5]);

	uartinfo_to_str(buffDisplay, &pProfile->usart_init_structure);
	W_RSP("+USET=%s\r\n",buffDisplay);
#if 1	// kaizen 20130731 ID1107 Changed option( A(Auto):Start Web server when link up event, M(Manual): Start Web Server when enter command )
#if 0 //MikeJ 1300822 - Wrong command name
	W_RSP("+ECHO=%d\r\n",pProfile->echo_mode);
#else
	W_RSP("+MECHO=%d\r\n",pProfile->echo_mode);
#endif
	W_RSP("+FWEBS=%d,%s\r\n", web_server_status, pProfile->web_server_opt );
#else
	W_RSP("+ECHO=%d\r\n", g_wxProfile.echo_mode);
	W_RSP("+FWEBS=%s\r\n", g_wxProfile.web_server_opt);									// kaizen 20130513 For Web Server Service Option
#endif

	W_RSP("+FGPIO=");
	for(i=0; i<USER_GPIO_MAX_COUNT; i++)
	{
		if( i == USER_GPIO_MAX_COUNT-1 )
		{
			W_RSP("{%d,%d,%d,%d}\r\n",g_wxProfile.user_gpio_conifg[i].mode,g_wxProfile.user_gpio_conifg[i].gpio_num,g_wxProfile.user_gpio_conifg[i].gpio_config_value,g_wxProfile.user_gpio_conifg[i].gpio_set_value);
			break;
		}

		W_RSP("{%d,%d,%d,%d},",g_wxProfile.user_gpio_conifg[i].mode,g_wxProfile.user_gpio_conifg[i].gpio_num,g_wxProfile.user_gpio_conifg[i].gpio_config_value,g_wxProfile.user_gpio_conifg[i].gpio_set_value);
	}
}

UINT8 WXCmd_MEXT1(UINT8 *ptr)
{
	UINT8 *p;
	UINT32 buff1 = 0, buff2 = 0;
	UINT8 status;

	// sekim 20150114 Just added <WXCmd_MEXT1=?>
	if ( strcmp((char*)ptr, "?") == 0 )
	{
		W_RSP("%d,%d\r\n", g_wxProfile.mext1_1, g_wxProfile.mext1_2);

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

	g_wxProfile.mext1_1 = buff1;
	g_wxProfile.mext1_2 = buff2;

	return WXCODE_SUCCESS;
}

// kaizen 20131112 ID1137 Added AT Command of MCWUI(Change Web server User Information)
UINT8 WXCmd_MCWUI(UINT8 *ptr)
{
	UINT8 *p;
	UINT8	change_user_id		[MAX_USER_ID_SIZE];
	UINT8	change_user_password[MAX_USER_PASSWORD_SIZE];


	if ( strcmp((char*)ptr, "?") == 0 )
	{
		W_RSP("%s,%s\r\n", g_wxProfile.user_id, g_wxProfile.user_password);

		return WXCODE_SUCCESS;
	}

	// verify userid
	p = WXParse_NextParamGet(&ptr);
	if ( !p )									return WXCODE_EINVAL;
	if ( strlen((char*)p) >= MAX_USER_ID_SIZE )	return WXCODE_EINVAL;
	if ( strcmp((char*)g_wxProfile.user_id, (char*)p) != 0 )
		return WXCODE_EINVAL;

	// verify user password
	p = WXParse_NextParamGet(&ptr);
	if ( !p )											return WXCODE_EINVAL;
	if ( strlen((char*)p) >= MAX_USER_PASSWORD_SIZE )	return WXCODE_EINVAL;
	if ( strcmp((char*)g_wxProfile.user_password, (char*)p) != 0 )
		return WXCODE_EINVAL;

	// change userid
	p = WXParse_NextParamGet(&ptr);
	if ( !p )									return WXCODE_EINVAL;
	if ( strlen((char*)p) >= MAX_USER_ID_SIZE )	return WXCODE_EINVAL;
	strcpy((char*)change_user_id, (char*)p);

	// change user password
	p = WXParse_NextParamGet(&ptr);
	if ( !p )											return WXCODE_EINVAL;
	if ( strlen((char*)p) >= MAX_USER_PASSWORD_SIZE )	return WXCODE_EINVAL;
	strcpy((char*)change_user_password, (char*)p);


	strcpy((char*)g_wxProfile.user_id,(char*)change_user_id);
	strcpy((char*)g_wxProfile.user_password,(char*)change_user_password);


	return WXCODE_SUCCESS;
}


// kaizen 20140410 ID1168 Customize for ShinHeung
UINT8 WXCmd_MCUSTOM(UINT8 *ptr)
{
	UINT8 *p;

	if ( strcmp((char*)ptr, "?") == 0 )
	{
		W_RSP("%s\r\n",g_wxProfile.custom_idx);

		return WXCODE_SUCCESS;
	}

	p = WXParse_NextParamGet(&ptr);
	if ( !p )							return WXCODE_EINVAL;
	if ( strlen((char*)p) > sizeof(g_wxProfile.custom_idx))		return WXCODE_EINVAL;

	if( strncmp((char*)p, "WIZNET", sizeof("WIZNET")) == 0 ||
		strncmp((char*)p, "SHINHEUNG", sizeof("SHINHEUNG")) == 0 ||
		strncmp((char*)p, "URIEL", sizeof("URIEL")) == 0 ||
		strncmp((char*)p, "POSBANK", sizeof("POSBANK")) == 0 ||
		strncmp((char*)p, "COWAYA", sizeof("COWAYA")) == 0 ||
		strncmp((char*)p, "COWAYB", sizeof("COWAYB")) == 0 ||
		strncmp((char*)p, "ENCORED", sizeof("ENCORED")) == 0
		)
	{
		memset(g_wxProfile.custom_idx, 0, sizeof(g_wxProfile.custom_idx));
		strcpy((char*)g_wxProfile.custom_idx, (char*) p );
	}
	else
	{
		return WXCODE_EINVAL;
	}

	return WXCODE_SUCCESS;
}

// kaizen 20140430 ID1173 - Added Function of Custom GPIO TYPE
extern uint32_t g_custom_gpio;

UINT8 WXCmd_MCSTGPIO(UINT8 *ptr)
{
	UINT8 *p, status;
	UINT32 buff1 = 0;


	if ( strcmp((char*)ptr, "?") == 0 )
	{
		W_RSP("%d\r\n",g_wxProfile.custom_gpio);

		return WXCODE_SUCCESS;
	}

	p = WXParse_NextParamGet(&ptr);
	if ( !p )		return WXCODE_EINVAL;

	status = WXParse_Int(p, &buff1);
	if ( status != WXCODE_SUCCESS )							return WXCODE_EINVAL;
	if ( buff1 != CST_NONE && buff1 != CST_OPEN_DRAIN )		return WXCODE_EINVAL;		// 0: Normal Operation, 1: OpenDrain(USART TX, SPI(MISO, DataReady))

	g_wxProfile.custom_gpio = buff1;
	g_custom_gpio = g_wxProfile.custom_gpio;

	return WXCODE_SUCCESS;
}


// kaizen 20140905 flag of printing [LISTEN] message for mbed library
UINT8 WXCmd_MEVTMSG(UINT8 *ptr)
{
	UINT8 *p;
	UINT32 buff1 = 0;
	UINT8 status;

	p = WXParse_NextParamGet(&ptr);
	if ( !p )							return WXCODE_EINVAL;
	status = WXParse_Int(p, &buff1);
	if ( status != WXCODE_SUCCESS )		return WXCODE_EINVAL;
	if ( buff1 != 0 && buff1 != 1)		return WXCODE_EINVAL;

	g_wxProfile.enable_listen_msg = buff1;			// Set for printing [Listen Event Message]

	return WXCODE_SUCCESS;
}


//////////////////////////////////////////////////////////////////////////////////////////
// sekim 20150416 add AT+MEVTFORM, AT+MEVTDELAY for Coway
UINT8 WXCmd_MEVTFORM(UINT8 *ptr)
{
	UINT8 *p;
	UINT8 status = WXCODE_SUCCESS;
	UINT32 i;
	UINT8 buff_msg_format[10];

	if ( strcmp((char*)ptr, "?") == 0 )
	{
		W_RSP("%c%c%c%c%c%c%c%c%c%c\r\n", g_wxProfile.event_msg_format[0], g_wxProfile.event_msg_format[1],
				g_wxProfile.event_msg_format[2], g_wxProfile.event_msg_format[3], g_wxProfile.event_msg_format[4], g_wxProfile.event_msg_format[5],
				g_wxProfile.event_msg_format[6], g_wxProfile.event_msg_format[7], g_wxProfile.event_msg_format[8], g_wxProfile.event_msg_format[9]);
		return WXCODE_SUCCESS;
	}

	p = WXParse_NextParamGet(&ptr);
	if ( !p )								return WXCODE_EINVAL;
	for(i = 0; i < 10; i++)
	{
		if ( p[i] == '0' || p[i] == '1' || p[i] == '2' )	buff_msg_format[i] = p[i];
		else												status = WXCODE_EINVAL;
	}
	if ( status != WXCODE_SUCCESS )			return status;
	if( p[i] != '\0' ) 						return WXCODE_EINVAL;

	memcpy(g_wxProfile.event_msg_format, buff_msg_format, sizeof(buff_msg_format));

	return WXCODE_SUCCESS;
}
UINT8 WXCmd_MEVTDELAY(UINT8 *ptr)
{
	UINT8 *p;
	UINT32 buff1 = 0;
	UINT8 status;

	if ( strcmp((char*)ptr, "?") == 0 )
	{
		W_RSP("%d\r\n", g_wxProfile.event_msg_delay);
		return WXCODE_SUCCESS;
	}

	p = WXParse_NextParamGet(&ptr);
	if ( !p )							return WXCODE_EINVAL;
	status = WXParse_Int(p, &buff1);
	if ( status != WXCODE_SUCCESS )		return WXCODE_EINVAL;

	if ( buff1 < 1 )		return WXCODE_EINVAL;
	if ( buff1 > 1000 )		return WXCODE_EINVAL;

	g_wxProfile.event_msg_delay = buff1;

	return WXCODE_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////////////////////
