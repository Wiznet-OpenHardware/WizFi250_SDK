#ifndef WX_DEFINES_H
#define WX_DEFINES_H

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

// sekim 20130423 FW Revision
//#define WIZFI250_FW_VERSION "0.0.0.9"
//#define WIZFI250_FW_VERSION "0.0.1.0"           // 20130414
//#define WIZFI250_FW_VERSION "0.0.2.1"           // 20130430 WICED SDK 2.3.0 Start
//#define WIZFI250_FW_VERSION "0.0.2.3"           // 20130430 WICED SDK 2.3.1 Start
//#define WIZFI250_FW_VERSION "0.9.0.1"             // 20130924 EVB PP, for Kyung-Dong Boiler sample
//#define WIZFI250_FW_VERSION "0.9.1.0"             // 20131004 WICED SDK 2.4.0 Start, 20131015 Applied EVB & Module PP 100EA
//#define WIZFI250_FW_VERSION "0.9.1.1"             // After 20131015
//#define WIZFI250_FW_VERSION "0.9.1.2"             // After 20131117
//#define WIZFI250_FW_VERSION "0.9.1.3"             // Before migration 2.4.0
//#define WIZFI250_FW_VERSION "0.9.2.0"             // Official 2.4.0 First Migration version
//#define WIZFI250_FW_VERSION "0.9.2.1"				// After 20131203
//#define WIZFI250_FW_VERSION "0.9.2.2"				// After 20131211, Added web server function ( serial configuration, user information setting, gpio control )
//#define WIZFI250_FW_VERSION "0.9.2.3"				// After 20131212 ID1141 add g_socket_extx_option
//#define WIZFI250_FW_VERSION "1.0.0.0"				// After 20131213
//#define WIZFI250_FW_VERSION "1.0.0.1"				// After 20140106 ID1157,ID1158
//#define WIZFI250_FW_VERSION "1.0.0.2"				// Official Release. (ID1161,ID1163,ID1164, ....)
//#define WIZFI250_FW_VERSION "1.0.0.3"				// 20140311 for Shin-Heung ID1163(AT+WANT), ID1165(AT+WBGN)
//#define WIZFI250_FW_VERSION "1.0.0.4"				// 20140410 for Shin-Heung ID1154, ID1166(AT+FWEBSOPT), ID1165(AT+WBGN)
//#define WIZFI250_FW_VERSION "1.0.0.5"				// 20140430 for Shin-Heung ID1172, And Added function for adding Root CA(ID1171)
//#define WIZFI250_FW_VERSION "1.0.0.6"				// 20140430 for Cuckoo ID1173 - Added Function of Custom GPIO Type
//#define WIZFI250_FW_VERSION "1.0.1.0"				// 20140430 WiFi Direct, scon-retry, .....
//#define WIZFI250_FW_VERSION "1.0.1.1"				// 20140512 WiFi Direct Link Down Issue, Stopped Joining by <Esc>
//#define WIZFI250_FW_VERSION "1.0.1.2"				// 20140527 Enable re-listen socket(TCP Server) ID1175
//#define WIZFI250_FW_VERSION "1.0.1.3"				// 20140625 ID1176 add option to clear tcp-idle-connection
//#define WIZFI250_FW_VERSION "1.0.1.4"				// 20140703 Remove(& Recover) ID1175 because of some trouble
//#define WIZFI250_FW_VERSION "1.0.1.5"				// 20140710 ID1179 add wcheck_option, ID1180 add air command
//#define WIZFI250_FW_VERSION "1.0.1.6"				// 20140715 ID1181 add g_queue_handle_maincommand
//#define WIZFI250_FW_VERSION "1.0.1.7"				// 20140716 URIEL/ENCORED Customizing, ID1182 add s2web_main
//#define WIZFI250_FW_VERSION "1.0.1.8"				// 20140804 ID1183 WEP Shared Problem, ID1184 SSID Including Space in Web, ID1185 scan in Web
//#define WIZFI250_FW_VERSION "1.0.1.9"				// 20140919 ID1186 SSID included special character as (~!@#$%^&*()_+|) Problem, URIEL Customizing 2
//#define WIZFI250_FW_VERSION "1.0.2.0"				// 20140929 ID1187 URIEL Customizing 2nd, ID1188 Data-Idle-Auto-Reset (Autonix)
//#define WIZFI250_FW_VERSION "1.0.2.1"				// 20141022 ID1188 This version is only for encored. wiznet_log.png file changed to encored logo.
//#define WIZFI250_FW_VERSION "1.0.2.2"				// 20141024 ID1189 Modified in order to print LOGO image and title in web page depending on CUSTOM value.
//#define WIZFI250_FW_VERSION "1.0.2.3"				// 20141106 ID1190 Modified DHCP_CLIENT_OBJECT_NAME which request for ENCORED.
//#define WIZFI250_FW_VERSION "1.0.2.4"				// 20141117 ID1191 URIEL Factory Default and AP mode & Web with a single-button-click
//#define WIZFI250_FW_VERSION "1.0.2.5"				// 20141125 add WXCmd_FGETADC
//#define WIZFI250_FW_VERSION "1.0.2.6"				// 20141125 POSBANK Customizing (UART Baudrate, 1 Button Click(Reset) 3.5S Click(Factory Default)
//#define WIZFI250_FW_VERSION "1.0.2.7"				// 20150106 POSBANK Customizing Added
//#define WIZFI250_FW_VERSION "1.0.3.0"				// 20150107 add WXCmd_WRFMODE, Change NVRAM (BM14_nvram_20150107.h)
//#define WIZFI250_FW_VERSION "1.0.3.1"				// 20150114 POSBANK Customizing Added(Event Message Disable, Dummy Logo), WXS2w_StatusNotify Bug Fix
//#define WIZFI250_FW_VERSION "1.0.3.x"				// 20150114 POSBANK Customizing Added(Event Message Disable, Dummy Logo), WXS2w_StatusNotify Bug Fix
//#define WIZFI250_FW_VERSION "1.0.3.2"				// 20150209 URIEL Antenna Default (1)
//#define WIZFI250_FW_VERSION "1.0.3.3"				// 20150212 Web Configuration Token Change "#~`" => "#~#" for Chrome-problem of Encored
//#define WIZFI250_FW_VERSION "1.0.3.4"				// 20150319 POSBANK Customizing Added(Recover Button-action)
//#define WIZFI250_FW_VERSION "1.0.3.5"				// 20150401 AT+SCON=S,USN,........ Bug Fix, Reset until association using WCHECK for ATK
//#define WIZFI250_FW_VERSION "1.0.3.6"				// 20150416 AT+MEVTFORM, AT+MEVTTERM, Random Client Socket for Coway
//#define WIZFI250_FW_VERSION "1.0.3.7"				// 20150508 Disable WizFi250-Reset because of socket_ext_option6, "COWAYA" Custom code, Coway ReSend <Connect Event Message>
//#define WIZFI250_FW_VERSION "1.0.3.8"				// 20150512 sekim DHCP PATCH Test
//#define WIZFI250_FW_VERSION "1.0.3.9"				// 20150521 Disable DHCP PATCH. add socket_ext_option8 by URIEL.
//#define WIZFI250_FW_VERSION "1.0.4.0"				// 20150603 kaizen  flag of printing [LISTEN] message for mbed library
//#define WIZFI250_FW_VERSION "1.0.4.1"				// 20150609 can't fix platfrom_spi_stdio
//#define WIZFI250_FW_VERSION "1.0.4.2"				// 20150616 add AT+SDNAME, add "COWAYB",
//#define WIZFI250_FW_VERSION "1.0.4.3"				// 20150622 sekim add FFTPSET/FFTPCMD for choyoung
//#define WIZFI250_FW_VERSION "1.0.4.4(C)"			// 20150717 sekim NVRAM Test for COWAY EMC
//#define WIZFI250_FW_VERSION "1.0.4.5"				// 20150717 sekim Remove <FFTPSET/FFTPCMD for choyoung>, add wcheck_option4
//#define WIZFI250_FW_VERSION "1.0.4.6"				// 20150902 sekim Add recovery DCT function
//#define WIZFI250_FW_VERSION "1.0.4.8"				// 20151123 sekim 20151123 Binary SSID for Coway
//#define WIZFI250_FW_VERSION "1.0.4.9"				// 20151204 sekim 20151204 Binary Key for Coway
//#define WIZFI250_FW_VERSION "1.0.5.0"				// 20151221 sekim XXXX Add GMMP Command & Library
//#define WIZFI250_FW_VERSION "1.0.5.1"				// 20160119 sekim XXXX Add socket_ext_option9 for TCP Server Multi-Connection
#define WIZFI250_FW_VERSION "1.0.5.2"				// 20160630 daniel Add MQTT Command & Library, UDP data mode

#define WIZFI250_HW_VERSION "WizFi250 Rev 1.0"

// kaizen 20140414 FW_Customize
#define WIZFI250_FW_CUSTOM	"WIZNET    "
//#define WIZFI250_FW_CUSTOM	"SHINHEUNG"

#define CHKCUSTOM(a) (strncmp((char*)g_wxProfile.custom_idx, a, sizeof(a))==0)

#ifdef TTHREAD_POSIX
#include <pthread.h> 
#endif
#ifdef WIN32
#include <process.h>
#endif

#ifdef WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#include <winsock2.h>
	#pragma comment(lib,"ws2_32.lib")
	#include <ws2tcpip.h>
#endif

#include "wx_types.h"
#include "wiced_wifi.h"

#define WX_SC_CONTYPE_UDP 1
#define WX_SC_CONTYPE_TCP 2

#define WX_SC_MODE_CLIENT 0
#define WX_SC_MODE_SERVER 1

#if 1	// kaizen 20130703 ID1096 - For printing SSL Mode using AT+SMGMT=? command.
#define WX_SC_TLS_MODE_NORMAL (UINT8)'N'
#define WX_SC_TLS_MODE_SECURE (UINT8)'S'
#endif

// Status codes
#define WXCODE_SUCCESS             0
#define WXCODE_FAILURE             1
#define WXCODE_EINVAL              2
#define WXCODE_EBADCID             3

// sekim add WXCODE_WIFISTATUS_ERROR
/*
#define WXCODE_CON_SUCCESS         4
#define WXCODE_ECIDCLOSE           5
#define WXCODE_LINKUP              6
#define WXCODE_LINKDOWN            7
*/
#define WXCODE_WIFISTATUS_ERROR    4
#define WXCODE_MODESTATUS_ERROR    5
#define WXCODE_CON_SUCCESS         6
#define WXCODE_ECIDCLOSE           7
#define WXCODE_LINKUP              8
#define WXCODE_LINKDOWN            9
#define WXCODE_RESET              10
// sekim 20131212 ID1141 add g_socket_extx_option
#define WXCODE_TCPSENDERROR       11
// sekim 20130429 add WXCODE_P2PFAIL
#define WXCODE_P2PFAIL 		      12
// kaizen 20140529 add WXCODE_LISTEN
#define WXCODE_LISTEN 		      13



#define WXASCII_BACKSPC  0x08
#define WXASCII_CR       0x0D
#define WXASCII_LF       0x0A



#define WX_INVALID_SCID		0xff
#define WX_MAX_SCID_RANGE	8

#define WX_CMDBUF_SIZE		256
#define WX_DATABUFFER_SIZE	1500

#define WX_MAX_PRINT_LEN	256

// kaizen
#define WX_WORD_SIZE		4
#define WX_IPv4_ADDR_SIZE	16

#define DHCPS_ENABLE  1
#define DHCPS_DISABLE 0

#if 1	// kaizen	20130514 ID 1062 - Fixed Bug that WizFi250 was hang up when set <UDP data mode> and send file more than 2Kbyte
#define WX_MAX_PACKET_SIZE	1460
#endif


// sekim 20120919 add <lwip Header File>
/*
#include "lwip/init.h"
#include "lwip/tcpip.h"
#include "netif/etharp.h"
#include "lwip/sockets.h"
#include "lwip/inet.h"
#include "lwip/dhcp.h"
*/


// sekim  <all includes files> for performance
#include "wx_platform_rtos_misc.h"
#include "../wizfi_wiced.h"
#include "wx_debug.h"
#include "wx_s2w_process.h"
#include "wx_s2w_function.h"
#include "wx_platform_rtos_socket.h"
#include "wx_general_parse.h"
#include "wx_commands_f.h"
#include "wx_commands_m.h"
#include "wx_commands_s.h"
#include "wx_commands_w.h"
#include "wx_commands_misc.h"
//daniel 160630 add MQTT Commands
#include "wx_commands_mqtt.h"

#include "wwd_debug.h"
#include <sys/types.h>

// sekim 20120919 add isprint from lwip
#ifndef isprint
#define in_range(c, lo, up)  ((u8_t)c >= lo && (u8_t)c <= up)
#define isprint(c)           in_range(c, 0x20, 0x7f)
#define isdigit(c)           in_range(c, '0', '9')
#define isxdigit(c)          (isdigit(c) || in_range(c, 'a', 'f') || in_range(c, 'A', 'F'))
#define islower(c)           in_range(c, 'a', 'z')
#define isspace(c)           (c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v')
#endif

#if 1 //MikeJ 130702 ID1092 - ATCmd update (naming, adding comments)
struct WX_COMMAND
{
	const char *cmd;
	UINT8 (*process)(UINT8 *ptr);
	INT8 *description;
	INT8 *usage;
};
#endif

#endif
