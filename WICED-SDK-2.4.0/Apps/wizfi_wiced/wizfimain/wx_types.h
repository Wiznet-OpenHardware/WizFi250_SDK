#ifndef WX_TYPES_H_
#define WX_TYPES_H_

#include "wiced_tcpip.h"
#include "wiced_wifi.h"

#ifndef UINT8
typedef unsigned char UINT8;
#endif
    
#ifndef INT8
typedef char INT8;
#endif

#ifndef INT16
typedef short INT16;
#endif
    
	
#ifndef UINT16
typedef unsigned short UINT16;
#endif

#ifndef INT32
typedef int INT32;
#endif
    
#ifndef UINT32
typedef unsigned int UINT32;
#endif

#ifndef BOOL
	#ifdef WIN32
	typedef int BOOL;
	#else
	typedef UINT8 BOOL;
	#endif
#endif

#ifndef LONG64
typedef long long LONG64;
#endif
    
#ifndef ULONG64
typedef unsigned long long ULONG64;
#endif

#ifndef WX_IPADDR_T
typedef UINT8 WT_IPADDR[4];
#endif

#ifndef VOID
#define VOID void
#endif

#ifndef TRUE
#define TRUE  1
#endif
 
#ifndef FALSE
#define FALSE 0
#endif

#define BIT(n) (1 << (n))
#define is_valid_port(x) ((x) > 0 && (x) <= 65535)
#if 1 //MikeJ 130624 ID1084 - Modified local-port checking part
#define is_valid_port_include_zero(x) ((x) >= 0 && (x) <= 65535)
#endif

#define WX_MAX_SSID_LEN    32

typedef struct WT_NETWORK
{
    UINT8 dhcp;
    UINT8 unused1;
    UINT16 unused2;
    WT_IPADDR ipAddr;
    WT_IPADDR netMask;
    WT_IPADDR gateway;
} WT_NETWORK;

typedef struct WT_SCCONFIG
{
    UINT8 mode;
    UINT16 port;
    WT_IPADDR ipAddr;
} WT_SCCONFIG;


typedef struct WT_SCLIST
{
	UINT8 conType;
	UINT8 conMode;
	UINT16 remotePort;
	wiced_ip_address_t remoteIp;
	UINT16 localPort;
	UINT8 dataMode;
	UINT8 tlsMode;
	UINT8 bAccepted;
	void* pSocket;
	// sekim 20130311 2.2.1 Migration, about TLS
	//wiced_tls_basic_context_t* pTLSContext;
	void* pTLSContext;
	// sekim 20130709 ID1089 if Socket Close by AT Command, don't restart <TCP Server>
	UINT8 notRestartTCPServer;
	// sekim 20140625 ID1176 add option to clear tcp-idle-connection
	UINT32 tcp_time_lastdata;
	UINT8 bConnected;
} WT_SCLIST;


// sekim 20121112 add WizFiQueue
typedef struct WizFiQueue
{
	UINT16 queue_id;
	UINT16 queue_opt;
} WizFiQueue;

#if 1 // kaizen 20130814 Modified AT+FGPIO function
typedef struct WizFi_Gpio_Config
{
	UINT32 mode;						// input, output
	UINT32 gpio_num;
	UINT32 gpio_config_value;
	UINT32 gpio_set_value;
}WizFi_Gpio_Config;
#endif

// kaizen
#include "stm32f2xx_usart.h"
#include "wwd_wifi.h"

// kaizen 20131104
#define MAX_USER_ID_SIZE			20 + 1
#define MAX_USER_PASSWORD_SIZE		20 + 1


typedef struct WT_PROFILE
{
#if 1	// kaizen 20131018 -ID1129 Add FW_VERSION Check Routine
	UINT8				fw_version[8];
#endif
	wiced_mac_t 		mac;
	USART_InitTypeDef	usart_init_structure;
	UINT8               msgLevel;

#if 1 //MikeJ 130410 ID1034 - Add ECHO on/off function
	UINT8               echo_mode;
#endif
	// sekim 20131125 Add SPI Interface
	UINT8               spi_stdio; // 0:Auto, 1:RX(UART)TX(UART) 2:RX(SPI)TX(SPI)
	UINT8               spi_mode;

#if 0 //MikeJ 130806 ID1116 - Couldn't display 32 characters SSID
	UINT8               wifi_ssid[WX_MAX_SSID_LEN];
#else
	UINT8               wifi_ssid[WX_MAX_SSID_LEN+1];
#endif
	UINT8               wifi_bssid[18];
	UINT8               wifi_channel;
	UINT8               wifi_mode;
	wiced_security_t    wifi_authtype;
	UINT8               wifi_keydata[64];
#if 1 //MikeJ 130408 ID1014 - Key buffer overflow
	UINT8               wifi_keylen;
#endif
	UINT8               wifi_dhcp;
	UINT8               wifi_ip[16];
	UINT8               wifi_mask[16];
	UINT8               wifi_gateway[16];
	UINT8               wifi_countrycode[3];

	UINT8               wifi_dns1[16];
	UINT8               wifi_dns2[16];

	UINT8               scon_opt1[3];
	UINT8               scon_opt2[4];
#if 0	// kaizen 20130703 -ID1095 Change the DCTD_SCON_OPT2's initial value to TSN, Because not use DCTD_SCON_OPT3 in source code
	UINT8               scon_opt3[2];
#endif

	UINT8               scon_remote_ip[16];
	UINT16              scon_remote_port;
	UINT16              scon_local_port;
	UINT8               scon_datamode;
	UINT8               xrdf_main[9];
	UINT8               xrdf_data[5];
	UINT16              timer_autoesc;

#if 0 //MikeJ 130806 ID1116 - Couldn't display 32 characters SSID
	UINT8            	ap_mode_ssid[WX_MAX_SSID_LEN];
#else
	UINT8               ap_mode_ssid[WX_MAX_SSID_LEN+1];
#endif
	UINT8            	ap_mode_channel;
	wiced_security_t 	ap_mode_authtype;
	UINT8            	ap_mode_keydata[64];
#if 1 //MikeJ 130408 ID1014 - Key buffer overflow
	UINT8               ap_mode_keylen;
#endif
#if 1 // kaizen 20130513 For Web Server Service Option
	UINT8				web_server_opt[2];
#endif
	UINT16				smode;
#if 1 // kaizen 20130814 Modified AT+FGPIO function
	WizFi_Gpio_Config	user_gpio_conifg[5];
#endif

#if 1 // kaizen 20131104
	UINT8				user_id[MAX_USER_ID_SIZE];
	UINT8				user_password[MAX_USER_PASSWORD_SIZE];
#endif

#if 1 // kaizen 20131118 ID1138 Solved Problem about strange operation when change AP mode using GPIO
	UINT8				gpio_wifi_auto_conn;
#endif
	// sekim 20131023 add MEXT1
	UINT16				mext1_1;
	UINT16				mext1_2;

	// sekim 20131212 ID1141 add g_socket_extx_option
	UINT32 socket_ext_option1;
	UINT32 socket_ext_option2;
	UINT32 socket_ext_option3;
	UINT32 socket_ext_option4;
	// sekim 20140625 ID1176 add option to clear tcp-idle-connection
	UINT32 socket_ext_option5;
	// sekim 20140430 ID1152 add scon_retry
	UINT32 socket_ext_option6;
	// sekim 20140929 ID1188 Data-Idle-Auto-Reset (Autonix)
	UINT32 socket_ext_option7;
	// sekim 20150519 add socket_ext_option8. Reset if not connected in TCP Client/Service
	UINT32 socket_ext_option8;
	// sekim XXXX 20160119 Add socket_ext_option9 for TCP Server Multi-Connection
	UINT32 socket_ext_option9;

	// sekim 20140214 ID1163 Add AT+WANT
	UINT8 antenna_type;

	// kaizen 20140408 ID1154, ID1166 Add AT+FWEBSOPT
	UINT8 show_hidden_ssid;
	UINT8 show_only_english_ssid;
	UINT8 show_rssi_range;

	// kaizen 20140410 ID1168 Customize for ShinHeung
	UINT8 custom_idx[20];

	// kaizen 20140430 ID1173 - Added Function of Custom GPIO TYPE
	UINT32 custom_gpio;

	// sekim 20140710 ID1180 add air command
	UINT8 aircmd_opentype[3];
	UINT8 aircmd_mode;
	UINT16 aircmd_port;
	UINT8 aircmd_opt;

	// sekim 20140710 ID1179 add wcheck_option
	UINT32 wcheck_option1;
	UINT8 wcheck_option2;
	UINT8 wcheck_option3;
	// sekim add wcheck_option4 for Smart-ANT)
	UINT8 wcheck_option4;


	// kaizen
	UINT8 enable_listen_msg;

	// sekim 20140716 ID1182 add s2web_main
	UINT8	s2web_main[8];

	// sekim 20150107 added WXCmd_WRFMODE
	UINT32 rfmode1;

	// sekim 20150416 add AT+MEVTFORM, AT+MEVTDELAY for Coway
	UINT8 event_msg_format[10]; // Boot, CON_SUCCESS, ECIDCLOSE, LINKUP, LINKDOWN, RESET, TCPSENDERROR, P2PFAIL, LISTEN, MISC.....
	UINT8 event_msg_delay;

	// sekim 20150508 Coway ReSend <Connect Event Message>
	UINT32 cowaya_check_time;
	char cowaya_check_data[10];

	// sekim 20150616 add AT+SDNAME
	char domainname_for_scon[50];

	// sekim 20150622 add FFTPSET/FFTPCMD for choyoung
	/*
	UINT8 ftpset_ip[16];
	UINT16 ftpset_port;
	UINT8 ftpset_id[20];
	UINT8 ftpset_pw[20];
	*/

	///////////////////////////////////////////////////////////////////////////////////////
	// sekim 20150902 Add recovery DCT function
	UINT8               bckup_start;
	UINT8               bckup_wifi_mode;
	UINT8               bckup_wifi_dhcp;
	UINT8               bckup_ap_mode_ssid[WX_MAX_SSID_LEN+1];
	wiced_security_t 	bckup_ap_mode_authtype;
	UINT8               bckup_ap_mode_keylen;
	UINT8            	bckup_ap_mode_keydata[64];
	UINT8               bckup_wifi_ip[16];
	UINT8               bckup_wifi_mask[16];
	UINT8               bckup_wifi_gateway[16];
	UINT8				bckup_gpio_wifi_auto_conn;
	UINT8               bckup_scon_opt1[3];
	UINT8               bckup_scon_opt2[4];
	UINT8               bckup_scon_remote_ip[16];
	UINT8               bckup_scon_datamode;
	UINT16              bckup_scon_local_port;
	UINT16              bckup_scon_remote_port;
	///////////////////////////////////////////////////////////////////////////////////////

	// daniel 160630 mqtt information
	uint8_t	          mqtt_user[50];
	uint8_t	          mqtt_password[50];
	uint8_t           mqtt_clientid[50];
	uint32_t          mqtt_alive;
	uint8_t           mqtt_ip[16];
	uint16_t          mqtt_port;
	uint8_t           mqtt_sslenable;

} WT_PROFILE;

typedef struct WT_SOCKETOPTION
{
	UINT32 scid;
	UINT32 type;
	UINT32 param;
	UINT32 paramValue;
    UINT32 paramSize;
}WT_SOCKETOPTION;


//kaizen
typedef enum
{
	STATION_MODE = 0,
	AP_MODE = 1,
}Wizfi250_Mode;

typedef enum
{
	STOP_WEB_SERVER = 0,
	START_WEB_SERVER = 1,
}WizFi250_WebServer_Cmd;

typedef enum
{
	WEB_CONFIGURATION_DISABLE = 0,
	WEB_CONFIGURATION_ENABLE = 1,
}WizFi250_Web_Configuration_Status;

#if 1	// kaizen 20130514 ID1048 - PS(Power Save) Function for MCU and Wi-Fi.
typedef enum
{
	PS_DISABLE = 0,
	PS_ENABLE = 1,
	PS_THR_ENABLE = 2,		// power save mode with throughput
}Wizfi250_PS_Mode;
#endif

#if 1	// kaizen 20130516 - Certificate Control Command
typedef enum
{
	MCERT_READ = 0,
	MCERT_WRITE = 1,
	MCERT_DELETE = 2,
	MCERT_CERTIFICATE = 3,
	MCERT_KEY = 4,
	MCERT_ROOTCA = 5,		//kaizen 20140428 ID1171 Added for using ROOT CA
}Wizfi250_Certi_Ctr_Cmd;
#endif

#if 1	// kaizen 20130621 - Added GPIO control function
typedef enum
{
	GPIO_SET_LOW = 0,
	GPIO_SET_HIGH = 1,
}WizFi250_GPIO_Control;

typedef enum
{
	GPIO_INPUT_MODE = 0,
	GPIO_OUTPUT_MODE = 1,
	GPIO_INIT_MODE = 2,
}WizFi250_GPIO_Mode;
#endif

typedef enum
{
	COMMAND_MODE = 0,
	DATA_MODE = 1,
}WizFi250_Socket_Mode;


#endif
