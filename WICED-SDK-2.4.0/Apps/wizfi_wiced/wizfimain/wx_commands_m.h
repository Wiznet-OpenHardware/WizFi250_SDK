// kaizen works
#ifndef WX_COMMANDS_M_H
#define WX_COMMANDS_M_H

UINT8 WXCmd_MPROF(UINT8 *ptr);
UINT8 WXCmd_MFDEF(UINT8 *ptr);
UINT8 WXCmd_MRESET(UINT8 *ptr);
UINT8 WXCmd_MMSG(UINT8 *ptr);
UINT8 WXCmd_MMAC(UINT8 *ptr);
UINT8 WXCmd_MINFO(UINT8 *ptr);

#if 1	// kaizen 20130514 ID1048 - PS(Power Save) Function for MCU and Wi-Fi.
UINT8 WXCmd_MMCUPS	(UINT8 *ptr);
UINT8 WXCmd_MWIFIPS	(UINT8 *ptr);
#endif
#if 1	// kaizen 20130514 ID1047 - WiFi Tx Power Function
UINT8 WXCmd_MTXP	(UINT8 *ptr);
#endif

// sekim 20131023 add MEXT1
UINT8 WXCmd_MEXT1(UINT8 *ptr);


UINT8 WXCmd_MCWUI	(UINT8 *ptr);	// kaizen 20131112 ID1137 Added AT Command of MCWUI(Change Web server User Information)
UINT8 WXCmd_MCUSTOM	(UINT8 *ptr);	// kaizen 20140410

UINT8 WXCmd_MCSTGPIO(UINT8 *ptr);	// kaizen 20140430 ID1173 - Added Function of Custom GPIO TYPE
UINT8 WXCmd_MEVTMSG	(UINT8 *ptr);	// kaizen 20140529
UINT8 WXCmd_MEVTFORM(UINT8 *ptr);
UINT8 WXCmd_MEVTDELAY(UINT8 *ptr);

UINT8 WXCmd_COWAYA(UINT8 *ptr);


UINT8 WXCmd_GMMPSET(UINT8 *ptr);
UINT8 WXCmd_GMMPDATA(UINT8 *ptr);
UINT8 WXCmd_GMMPOPT(UINT8 *ptr);

void Default_Profile();
void Save_Profile();
void Load_Profile();
#if 0	// kaizen 20130712 ID1099 For Setting TCP/UDP Connection using web server, Do not need this function
void Apply_To_APP_Dct();
#endif
void Apply_To_Wifi_Dct();
void DisplayWTProfile(WT_PROFILE* pProfile);

#define DCTD_MSGLEVEL               2
#if 1 //MikeJ 130410 ID1034 - Add ECHO on/off function
#define DCTD_ECHO_MODE              TRUE
#endif

//////////////////////////////////////////////////////////////////////
// sekim 20131125 Add SPI Interface
#define DCTD_SPI_STDIO              0
#define DCTD_SPI_MODE               9
//////////////////////////////////////////////////////////////////////

#define DCTD_WIFI_SSID              "WizFiAP"
#define DCTD_WIFI_BSSID             ""
#define DCTD_WIFI_CHANNEL           6
#define DCTD_WIFI_MODE              0
#define DCTD_WIFI_AUTHTYPE          WICED_SECURITY_WPA2_MIXED_PSK
#define DCTD_WIFI_KEYDATA           "12345678"
#if 1 //MikeJ 130408 ID1014 - Key buffer overflow
#define DCTD_WIFI_KEYLEN            8
#endif
#define DCTD_WIFI_DHCP              1
#define DCTD_WIFI_IP                "192.168.12.101"
#define DCTD_WIFI_MASK              "255.255.255.0"
#define DCTD_WIFI_GATEWAY           "192.168.12.1"
#define DCTD_WIFI_COUNTRYCODE       "AU"		// kaizen 20131127 ID1143 Change the default country code to Australia
#define DCTD_SCON_OPT1              "O"

#if 0	// kaizen 20130703 -ID1095 Change the DCTD_SCON_OPT2's initial value to TSN, Because not use DCTD_SCON_OPT3 in
#define DCTD_SCON_OPT2              "TS"
#else
#define DCTD_SCON_OPT2				"TSN"
#endif

#if 0	// kaizen 20130703 -ID1095 Change the DCTD_SCON_OPT2's initial value to TSN, Because not use DCTD_SCON_OPT3 in
#define DCTD_SCON_OPT3              "N"
#endif

#define DCTD_SCON_REMOTE_IP         ""
#define DCTD_SCON_REMOTE_PORT       0
#define DCTD_SCON_LOCAL_PORT        5000
#define DCTD_SCON_DATAMODE          1
#define DCTD_XRDF_MAIN              "111111111"
#define DCTD_XRDF_DATA              "{,}\r\n"
#define DCTD_TIMER_AUTOESC          1000

#define DCTD_MAC_00                 0x00
#define DCTD_MAC_01                 0x08
#define DCTD_MAC_02                 0xDC
#define DCTD_MAC_03                 0x00
#define DCTD_MAC_04                 0x00
#define DCTD_MAC_05                 0x00

// 20131015 sekim Rinnai Baud Rate to 9600
#define DCTD_USART_BAUDRATE         115200
//#define DCTD_USART_BAUDRATE         9600
#define DCTD_USART_WORDLENGTH       USART_WordLength_8b
#define DCTD_USART_PARITY           USART_Parity_No
#define DCTD_USART_STOPBITS         USART_StopBits_1
#define DCTD_USART_HWFC             USART_HardwareFlowControl_None
#define DCTD_USART_MODE             USART_Mode_Rx | USART_Mode_Tx

#define DCTD_AP_MODE_SSID           "WizFiSoftAP"
#define DCTD_AP_MODE_CHANNEL        1
#define DCTD_AP_MODE_AUTHTYPE       WICED_SECURITY_WPA2_MIXED_PSK
#define DCTD_AP_MODE_KEYDATA        "12345678"
#if 1 //MikeJ 130408 ID1014 - Key buffer overflow
#define DCTD_AP_MODE_KEYLEN         8
#endif

#if 1	// kaizen 20130726 ID1105 Modified code in order to launch web server when link up event --> 20130731 ID1107 Changed option( A(Auto):Start Web server when link up event, M(Manual): Start Web Server when enter command )
// sekim 20140214 ID1161 Default FWEBS => "M" for some memory issue
//#define DCTD_WEB_SERVER_OPT        "A"
#define DCTD_WEB_SERVER_OPT        "M"
#endif

#if 1 // kaizen 20130814 Modified AT+FGPIO function
#define USER_GPIO_MAX_COUNT 		5
#define DCTD_GPIO_MODE				1	// OUTPUT
#define DCTD_GPIO_NUM  				{ 1, 5, 6, 7, 8 }
#define DCTD_GPIO_CONFIG_VALUE		3	// OUTPUT PUSH PULL
#define DCTD_GPIO_SET_VALUE			0

#define DCTD_GPIO_CONFIG			{ 	{DCTD_GPIO_MODE, 1, DCTD_GPIO_CONFIG_VALUE, DCTD_GPIO_SET_VALUE},	\
										{DCTD_GPIO_MODE, 5, DCTD_GPIO_CONFIG_VALUE, DCTD_GPIO_SET_VALUE}, 	\
										{DCTD_GPIO_MODE, 6, DCTD_GPIO_CONFIG_VALUE, DCTD_GPIO_SET_VALUE}, 	\
										{DCTD_GPIO_MODE, 7, DCTD_GPIO_CONFIG_VALUE, DCTD_GPIO_SET_VALUE}, 	\
										{DCTD_GPIO_MODE, 8, DCTD_GPIO_CONFIG_VALUE, DCTD_GPIO_SET_VALUE}  	\
									}	// GPIO_MODE, GPIO_NUM, GPIO_CONFIG_VALUE, GPIO_SET_VALUE
#endif

#if 1 // kaizen 20131112 ID1137 Added AT Command of MCWUI(Change Web server User Information)
#define DCTD_USER_ID				"admin"
#define DCTD_USER_PASSWORD			"admin"
#endif

#if 1 // kaizen 20131118 ID1138 Solved Problem about strange operation when change AP mode using GPIO
#define DCTD_GPIO_WIFI_AUTO_CONN	0
#endif

// sekim 20131212 ID1141 add g_socket_extx_option
#define DCTD_SOCKET_EXT_OPTION1        5000
#define DCTD_SOCKET_EXT_OPTION2        5000
#define DCTD_SOCKET_EXT_OPTION3        5
#define DCTD_SOCKET_EXT_OPTION4        100
#define DCTD_SOCKET_EXT_OPTION5        0
#define DCTD_SOCKET_EXT_OPTION6        0
// sekim 20140929 ID1188 Data-Idle-Auto-Reset (Autonix)
#define DCTD_SOCKET_EXT_OPTION7        0
// sekim 20150519 add socket_ext_option8. Reset if not connected in TCP Client/Service
#define DCTD_SOCKET_EXT_OPTION8        0
// sekim XXXX 20160119 Add socket_ext_option9 for TCP Server Multi-Connection
#define DCTD_SOCKET_EXT_OPTION9        0

// sekim 20140710 ID1179 add wcheck_option
#define DCTD_WCHECK_OPTION1         0
#define DCTD_WCHECK_OPTION2         0
#define DCTD_WCHECK_OPTION3         0
// sekim add wcheck_option4 for Smart-ANT)
#define DCTD_WCHECK_OPTION4         0

// sekim 20140214 ID1163 Add AT+WANT
#define DCTD_ANTENNA_TYPE		3

// kaizen 20140408 ID1154, ID1166 Add AT+FWEBSOPT
#define DCTD_SHOW_HIDDEN_SSID		1
#define DCTD_SHOW_ONLY_ENGLISH_SSID	0
#define DCTD_SHOW_RSSI_RAGE			0

// kaizen 20140430 ID1173 - Added Function of Custom GPIO TYPE
#define DCTD_CUSTOM_GPIO			0

// sekim 20140716 ID1182 add s2web_main
#define DCTD_S2WEB_MAIN "1111111"

// sekim 20150107 added WXCmd_WRFMODE
#define DCTD_RFMODE1				0

// sekim 20150416 add AT+MEVTFORM, AT+MEVTDELAY for Coway
#define DCTD_EVENT_MSG_FORMAT "1111111111"
#define DCTD_EVENT_MSG_DELAY 0

// sekim 20150508 Coway ReSend <Connect Event Message>
#define DCTD_COWAYA_CHECK_TIME 0
#define DCTD_COWAYA_CHECK_DATA ""

// sekim 20150616 add AT+SDNAME
#define DCTD_DOMAINNAME_FOR_SCON ""

// sekim 20150622 add FFTPSET/FFTPCMD for choyoung
/*
#define DCTD_FTPSET_IP ""
#define DCTD_FTPSET_PORT 0
#define DCTD_FTPSET_ID ""
#define DCTD_FTPSET_PW ""
*/

#endif

