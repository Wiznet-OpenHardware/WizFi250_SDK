#include "wiced_dct.h"
#include "wizfimain/wx_types.h"
#include "wizfimain/wx_commands_m.h"
#if 1	// kaizen 20131018 ID1129 - Add FW_VERSION Check routin
#include "wizfimain/wx_defines.h"
#endif

/******************************************************
 *               Variables Definitions
 ******************************************************/

DEFINE_APP_DCT(WT_PROFILE)
{
#if 1	// kaizen 20131018 ID1129 - Add FW_VERSION Check routin
	.fw_version					= WIZFI250_FW_VERSION      ,
#endif

	.mac.octet[0] = DCTD_MAC_00,
	.mac.octet[1] = DCTD_MAC_01,
	.mac.octet[2] = DCTD_MAC_02,
	.mac.octet[3] = DCTD_MAC_03,
	.mac.octet[4] = DCTD_MAC_04,
	.mac.octet[5] = DCTD_MAC_05,

	.usart_init_structure.USART_BaudRate            = DCTD_USART_BAUDRATE  ,
	.usart_init_structure.USART_WordLength          = DCTD_USART_WORDLENGTH,
	.usart_init_structure.USART_Parity              = DCTD_USART_PARITY    ,
	.usart_init_structure.USART_StopBits            = DCTD_USART_STOPBITS  ,
	.usart_init_structure.USART_HardwareFlowControl = DCTD_USART_HWFC      ,
	.usart_init_structure.USART_Mode                = DCTD_USART_MODE      ,

	.msgLevel                   = DCTD_MSGLEVEL            ,
#if 1 //MikeJ 130410 ID1034 - Add ECHO on/off function
	.echo_mode                  = DCTD_ECHO_MODE           ,
#endif
	// sekim 20131125 Add SPI Interface
	.spi_stdio                  = DCTD_SPI_STDIO           ,
	.spi_mode                   = DCTD_SPI_MODE            ,

	.wifi_ssid                  = DCTD_WIFI_SSID           ,
	.wifi_bssid                 = DCTD_WIFI_BSSID          ,
	.wifi_channel               = DCTD_WIFI_CHANNEL        ,
	.wifi_mode                  = DCTD_WIFI_MODE           ,
	.wifi_authtype              = DCTD_WIFI_AUTHTYPE       ,
	.wifi_keydata               = DCTD_WIFI_KEYDATA        ,
#if 1 //MikeJ 130408 ID1014 - Key buffer overflow
	.wifi_keylen                = DCTD_WIFI_KEYLEN         ,
#endif
	.wifi_dhcp                  = DCTD_WIFI_DHCP           ,
	.wifi_ip                    = DCTD_WIFI_IP             ,
	.wifi_mask                  = DCTD_WIFI_MASK           ,
	.wifi_gateway               = DCTD_WIFI_GATEWAY        ,
	.wifi_countrycode           = DCTD_WIFI_COUNTRYCODE    ,
	.scon_opt1                  = DCTD_SCON_OPT1           ,
	.scon_opt2                  = DCTD_SCON_OPT2           ,
#if 0	// kaizen 20130703 -ID1095 Change the DCTD_SCON_OPT2's initial value to TSN, Because not use DCTD_SCON_OPT3 in source code
	.scon_opt3                  = DCTD_SCON_OPT3           ,
#endif
	.scon_remote_ip             = DCTD_SCON_REMOTE_IP      ,
	.scon_remote_port           = DCTD_SCON_REMOTE_PORT    ,
	.scon_local_port            = DCTD_SCON_LOCAL_PORT     ,
	.scon_datamode              = DCTD_SCON_DATAMODE       ,
	.xrdf_main                  = DCTD_XRDF_MAIN           ,
	.xrdf_data                  = DCTD_XRDF_DATA           ,
	.timer_autoesc              = DCTD_TIMER_AUTOESC       ,

	.ap_mode_ssid               = DCTD_AP_MODE_SSID        ,
	.ap_mode_channel            = DCTD_AP_MODE_CHANNEL     ,
	.ap_mode_authtype           = DCTD_AP_MODE_AUTHTYPE    ,
	.ap_mode_keydata            = DCTD_AP_MODE_KEYDATA     ,
#if 1 //MikeJ 130408 ID1014 - Key buffer overflow
	.ap_mode_keylen             = DCTD_AP_MODE_KEYLEN      ,
#endif
	.web_server_opt             = DCTD_WEB_SERVER_OPT      ,

#if 1 // kaizen 20130814 Modified AT+FGPIO function
	.user_gpio_conifg			= DCTD_GPIO_CONFIG		   ,
#endif

#if 1 // kaizen 20131112 ID1137 Added AT Command of MCWUI(Change Web server User Information)
	.user_id					= DCTD_USER_ID				,
	.user_password				= DCTD_USER_PASSWORD		,
#endif

#if 1 // kaizen 20131118 ID1138 Solved Problem about strange operation when change AP mode using GPIO
	.gpio_wifi_auto_conn		= DCTD_GPIO_WIFI_AUTO_CONN,
#endif
	// sekim 20131212 ID1141 add g_socket_extx_option
	.socket_ext_option1			= DCTD_SOCKET_EXT_OPTION1   ,
	.socket_ext_option2			= DCTD_SOCKET_EXT_OPTION2   ,
	.socket_ext_option3			= DCTD_SOCKET_EXT_OPTION3   ,
	.socket_ext_option4			= DCTD_SOCKET_EXT_OPTION4   ,
	.socket_ext_option5			= DCTD_SOCKET_EXT_OPTION5   ,
	.socket_ext_option6			= DCTD_SOCKET_EXT_OPTION6   ,
	// sekim 20140929 ID1188 Data-Idle-Auto-Reset (Autonix)
	.socket_ext_option7			= DCTD_SOCKET_EXT_OPTION7   ,
	// sekim 20150519 add socket_ext_option8. Reset if not connected in TCP Client/Service
	.socket_ext_option8			= DCTD_SOCKET_EXT_OPTION8   ,
	// sekim XXXX 20160119 Add socket_ext_option9 for TCP Server Multi-Connection
	.socket_ext_option9			= DCTD_SOCKET_EXT_OPTION9   ,

	// sekim 20140710 ID1179 add wcheck_option
	.wcheck_option1		 		= DCTD_WCHECK_OPTION1   ,
	.wcheck_option2		 		= DCTD_WCHECK_OPTION2   ,
	.wcheck_option3		 		= DCTD_WCHECK_OPTION3   ,
	// sekim add wcheck_option4 for Smart-ANT)
	.wcheck_option4		 		= DCTD_WCHECK_OPTION4   ,

	// sekim 20140214 ID1163 Add AT+WANT
    .antenna_type 				= DCTD_ANTENNA_TYPE         ,

    // kaizen 20140408 ID1154, ID1166 Add AT+FWEBSOPT
    .show_hidden_ssid			= DCTD_SHOW_HIDDEN_SSID			,
    .show_only_english_ssid		= DCTD_SHOW_ONLY_ENGLISH_SSID	,
    .show_rssi_range			= DCTD_SHOW_RSSI_RAGE			,

    // kaizen 20140410 ID1168 Customize for ShinHeung
    .custom_idx					= WIZFI250_FW_CUSTOM			,

    // kaizen 20140430 ID1173 - Added Function of Custom GPIO TYPE
    .custom_gpio				= DCTD_CUSTOM_GPIO				,

    // sekim 20140716 ID1182 add s2web_main
    .s2web_main 				= DCTD_S2WEB_MAIN				,

    // kaizen 20140905 flag of printing [LISTEN] message for mbed library
    .enable_listen_msg			= 0,

    // sekim 20150107 added WXCmd_WRFMODE
    .rfmode1                    = 0,

    // sekim 20150416 add AT+MEVTFORM, AT+MEVTDELAY for Coway
    .event_msg_format           = DCTD_EVENT_MSG_FORMAT,
    .event_msg_delay            = DCTD_EVENT_MSG_DELAY,

    // sekim 20150508 Coway ReSend <Connect Event Message>
	.cowaya_check_time          = DCTD_COWAYA_CHECK_TIME,
	.cowaya_check_data          = DCTD_COWAYA_CHECK_DATA,

	// sekim 20150616 add AT+SDNAME
	.domainname_for_scon        = DCTD_DOMAINNAME_FOR_SCON,

	// sekim 20150622 add FFTPSET/FFTPCMD for choyoung
	/*
	.ftpset_ip                  = DCTD_FTPSET_IP,
	.ftpset_port                = DCTD_FTPSET_PORT,
	.ftpset_id                  = DCTD_FTPSET_ID,
	.ftpset_pw                  = DCTD_FTPSET_PW,
	*/
};
