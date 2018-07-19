#include "wx_defines.h"

#include "internal/wiced_internal_api.h"
#include "wiced_management.h"

#include "wwd_management.h"
#include "internal/wwd_internal.h"
#if 1 //MikeJ 130528 ID1072 - Improve UART Performance
#include "wiced_platform.h"
#endif

#if 1	// kaizen 20131118 ID1138 Solved Problem about strange operation when change AP mode using GPIO
#include "wiced_dct.h"
#endif

UINT8 g_wxModeState;

UINT8 g_isAutoconnected;

WT_PROFILE g_wxProfile;
UINT8 g_currentScid = WX_INVALID_SCID;
UINT8 g_scTxBuffer[WX_DATABUFFER_SIZE];
UINT32 g_scTxIndex = 0;

wiced_mutex_t g_s2w_wizmutex;
wiced_mutex_t g_something_wizmutex;
wiced_mutex_t g_upart_type1_wizmutex;
// sekim 20130411 add g_socketopen_wizmutex
wiced_mutex_t g_socketopen_wizmutex;

UINT8 g_auto_esc = 0;

wiced_timed_event_t g_nagle_timer;
wiced_timed_event_t g_autoesc_timer;

wiced_thread_t g_thread_handle_wizfiqueue;
wiced_queue_t g_queue_handle_wizfiqueue;

// sekim 20140715 ID1181 add g_queue_handle_maincommand
wiced_queue_t g_queue_handle_maincommand;

/////////////////////////////////////////////////////////////////////////////
// sekim 20130403 Add Task Monitor, WXS2w_SerialInput
wiced_system_monitor_t wizfi_task_monitor_item;
uint32_t wizfi_task_monitor_index = 0;
uint32_t wizfi_task_monitor_stop = 0;
/////////////////////////////////////////////////////////////////////////////

#if 0 //MikeJ 130702 ID1092 - ATCmd update (naming, adding comments)
struct WX_COMMAND
{
	const char *cmd;
	UINT8 (*process)(UINT8 *ptr);
};
#endif

const struct WX_COMMAND g_WXCmdTable[] =
{
#if 0 //MikeJ 130702 ID1092 - ATCmd update (naming, adding comments)
	{ "+WJOIN", WXCmd_WJOIN },
	{ "+WLEAVE", WXCmd_WLEAVE },
	{ "+WSCAN=", WXCmd_WSCAN },
	{ "+WSET=", WXCmd_WSET },
	{ "+WSEC=", WXCmd_WSEC },
	{ "+WNET=", WXCmd_WNET },
	{ "+WSTAT", WXCmd_WSTAT },
	{ "+WREG=", WXCmd_WREG },
	{ "+SDATA", WXCmd_SDATA },
	{ "+SCON=", WXCmd_SCON },
	{ "+SMGMT=", WXCmd_SMGMT },
	{ "+SSEND=", WXCmd_SSEND },
	{ "+SFORM=", WXCmd_SFORM },
	{ "+SOPT1=", WXCmd_SOPT1 },
	{ "+SOPT2=", WXCmd_SOPT2 },

	// kaizen 20121107 Working
	{ "+MPROF=", WXCmd_MPROF },
	{ "+MFDEF=", WXCmd_MFDEF },
	{ "+MRESET", WXCmd_MRESET },
	{ "+MMSG=", WXCmd_MMSG },
	{ "+MMAC=", WXCmd_MMAC },
	{ "+MINFO", WXCmd_MINFO },

#if 1	// kaizen 20130514 ID1048 - PS(Power Save) Function for MCU and Wi-Fi.
	{ "+MMCUPS=", WXCmd_MMCUPS },
	{ "+MWIFIPS=",WXCmd_MWIFIPS },
#endif
#if 1	// kaizen 20130514 ID1047 -WiFi Tx Power Function
	{ "+MTXP=",	 WXCmd_MTXP },
#endif

	{ "+MCERT=", WXCmd_MCERT },

	{ "+USET=", WXCmd_USET },
#if 1 //MikeJ 130410 ID1034 - Add ECHO on/off function
	{ "E=", WXCmd_Echo },
#endif

	{ "+FPING=", WXCmd_FPING },
	{ "+FDNS=",  WXCmd_FDNS },
	{ "+FWEBS=", WXCmd_FWEBS },
	{ "+FWPS=",  WXCmd_FWPS },
	//  kaizen 20130704 ID1081 Added OTA Command + ID1097 Added GPIO Control Function
	{ "+FOTA",   WXCmd_FOTA },
	{ "+FGPIO=", WXCmd_FGPIO },

	{ "+FS2WEB=", WXCmd_S2WEB },
	{ "+SMODE=", WXCmd_SMODE },
#else
	{ "+WJOIN",    WXCmd_WJOIN,   "WiFi Association", NULL },
	{ "+WLEAVE",   WXCmd_WLEAVE,  "WiFi Disassociation", NULL },
	{ "+WSCAN",    WXCmd_WSCAN,   "WiFi Scan", "=<SSID>,<BSSID>,<Channel>" },
	//////////////////////////////////////////////////////////////////////////////////////////
	// sekim XXXX 20151123 Binary SSID for Coway
	// Note : Coway F/W and Standard F/W
	// Standard F/W
#define COWAY_FW_20151123
#ifdef COWAY_FW_20151123
	{ "+WSET2=",   WXCmd_WSET,    "WiFi Configuration", "=? or =<WiFiMode>,<SSID>,<BSSID>,<Channel>" },
	{ "+WSET=",    WXCmd_WSET_TEMPforCoway,   "WiFi Configuration(Binary SSID)", "=? or =<WiFiMode>,<SSID>" },
#else
	{ "+WSET=",    WXCmd_WSET,    "WiFi Configuration", "=? or =<WiFiMode>,<SSID>,<BSSID>,<Channel>" },
	{ "+WSET2=",   WXCmd_WSET2,   "WiFi Configuration(Binary SSID)", "=? or =<WiFiMode>,<SSID>" },
#endif
	//////////////////////////////////////////////////////////////////////////////////////////
	{ "+WSEC=",    WXCmd_WSEC,    "WiFi Security Configuration", "=? or =<WiFiMode>,<SecType>,<PreSharedKey>" },
	{ "+WNET=",    WXCmd_WNET,    "Network Configuration", "=? or =<DHCP>,<IP>,<SN>,<GW>" },
	{ "+WSTAT",    WXCmd_WSTAT,   "Get Current WiFi Status", NULL },
	{ "+WWPS=",    WXCmd_WWPS,    "WiFi WPS Association", "=<Mode>,<PinNum>" },
	{ "+WREG=",    WXCmd_WREG,    "Country Configuration", "=? or =<Country>" },

	// sekim 20130806 ID1120 Add DNS Server if using Static IP
	{ "+WADNS=",   WXCmd_WADNS,   "Add DNS Server", "=? or =<DNSServer1>,<DNSServer2>" },

	// sekim 20140214 ID1163 Add AT+WANT
	{ "+WANT=",    WXCmd_WANT,    "Antenna Configuration", "=? or =<Antenna Type> (0:uFL,1:PA,3:Auto)" },

	{ "+WBGN=",    WXCmd_WBGN,    "WiFi 802.11bgn mode Configuration", "=? or =<mode> (0:bgn, 1:bg, 2:b)" },

	// sekim 20140710 ID1179 add wcheck_option
	{ "+WCHECK=",  WXCmd_WCHECK,  "Check WiFi Association", "=<Option>,<Value>" },

	{ "+SCON=",    WXCmd_SCON,    "Socket Open/Connect", "=? or =<OpenTyp>,<SockTyp>,<R-IP>,<R-Port>,<L-Port>,<D-Mode>" },
	{ "+SMGMT=",   WXCmd_SMGMT,   "Socket View/Close", "=? or =<SocketID>" },
	{ "+SSEND=",   WXCmd_SSEND,   "Send Data", "=<SocketID>,<RemoteIP>,<RemotePort>,<SendSize>" },
	{ "+SDATA",    WXCmd_SDATA,   "Return to Data Mode", NULL },
	{ "+SFORM=",   WXCmd_SFORM,   "Define Data Receive Header Form", "=? or =<Format>,<Start>,<Delim>,<End>,<Cls1>,<Cls2>" },
	{ "+SOPT1=",   WXCmd_SOPT1,   NULL, NULL },
	{ "+SOPT2=",   WXCmd_SOPT2,   NULL, NULL },

	// sekim 20150616 add AT+SDNAME
	{ "+SDNAME=",  WXCmd_SDNAME,  "DomainName for AT+SCON(if Remote-IP is 0.0.0.0)", "=? or =<DomainName>" },

	{ "+MPROF=",   WXCmd_MPROF,   "Profile Management", "=<Action>" },
	{ "+MFDEF=",   WXCmd_MFDEF,   "Factory Reset", "=FR" },
	{ "+MRESET",   WXCmd_MRESET,  "Reset", NULL },
	{ "+MMSG=",    WXCmd_MMSG,    "Set Message Print Level", "=? or =<Level>" },
	{ "+MMAC=",    WXCmd_MMAC,    "Set MAC Address", "=? or =<MAC>" },
	{ "+MINFO",    WXCmd_MINFO,   "Get System Information", NULL },

#if 1 //MikeJ 130410 ID1034 - Add ECHO on/off function + 130702 ID1092 - ATCmd update (naming, adding comments)
	{ "+MECHO=",   WXCmd_MECHO,   "Set/Get Echo Mode", "=? or =<Mode>" },
	{ "+MHELP",    WXCmd_MHELP,   "Print Command Description and Usage", NULL },
#endif
#if 1	// kaizen 20130514 ID1048 - PS(Power Save) Function for MCU and Wi-Fi.
	{ "+MMCUPS=",  WXCmd_MMCUPS,  "ToDo", NULL },
	{ "+MWIFIPS=", WXCmd_MWIFIPS, "Wi-Fi Power Save Mode Enable/Disable", "=? or =<Action>,<Delay> (<Delay> can be used when set 2 of <Action> parameter" },
#endif
#if 1	// kaizen 20130514 ID1047 -WiFi Tx Power Function
	{ "+MTXP=",	   WXCmd_MTXP,    "ToDo", NULL },
#endif
	{ "+MCERT=",   WXCmd_MCERT,   "Certificate Management", "=<Read/Write>,<Type>" },
	// sekim 20131023 add MEXT1
	{ "+MEXT1=",   WXCmd_MEXT1,   NULL, NULL },
	// kaizen 20131112 ID1137 Added AT Command of MCWUI(Change Web server User Information)
	{ "+MCWUI=",   WXCmd_MCWUI,   NULL, NULL },

	{ "+USET=",    WXCmd_USET,    "UART Configuration", "=? or =<BR>,<Parity>,<WordL>,<StopB>,<FlowC>" },

	// sekim 20131125 Add SPI Interface
	{ "+MSPI=",    WXCmd_MSPI,    "SPI Configuration", "=<STDIOmode>,<Rising/Falling Edge>,<Idle Low/High>,<MSB/LSB First>" },

	{ "+FPING=",   WXCmd_FPING,   "PING Test", "=<RepeatCnt>,<TargetIP>" },
	{ "+FDNS=",    WXCmd_FDNS,    "DNS Query", "=<HostName>,<Timeout>" },
	{ "+FWEBS=",   WXCmd_FWEBS,   "WEB Server Start/Stop", "=? or =<Action>,<Type>" },

	// sekim 20140716 ID1182 add s2web_main
	{ "+FWEBM=",   WXCmd_FWEBM,   "WEB Server Menu", "=? or =Option" },

#if 0  //  kaizen 20130704 ID1081 Added OTA Command + ID1097 Added GPIO Control Function
	{ "+FEXT1",    WXCmd_FEXT1,   NULL, NULL },
#else
	{ "+FOTA",    WXCmd_FOTA,   NULL, NULL },
	{ "+FGPIO=",  WXCmd_FGPIO,  NULL, NULL },
#endif

	// sekim 20141125 add WXCmd_FGETADC
	{ "+FGETADC",  WXCmd_FGETADC,  "Get ADC value(GPIO5)", NULL },

	// sekim 20131212 ID1141 add g_socket_extx_option
	{ "+FSOCK=", WXCmd_FSOCK,     "SOCKET Extension Option", "=<Option>,<Value>" },

	//Only For Kiturami
	{ "+FS2WEB=",  WXCmd_S2WEB,   NULL, NULL },
	{ "+SMODE=",   WXCmd_SMODE,   NULL, NULL },

	// kaizen 20140408 ID1154, ID1166 Add AT+FWEBSOPT
	{ "+FWEBSOPT=",  WXCmd_FWEBSOPT,   NULL, NULL },

	// sekim 20150622 add FFTPSET/FFTPCMD for choyoung
	/*
	{ "+FFTPSET=",  WXCmd_FFTPSET,   "FTP Client Configuration", "=<IP>,<Port>,<ID>,<Password>" },
	{ "+FFTPCMD=",  WXCmd_FFTPCMD,   "FTP Client Command(LIST/DELE/RETR/STOR)", "=<IP>,<Port>,<ID>,<Password>" },
	*/


	// kaizen 20140409 ID1156 Added Wi-Fi Direct Function
	{ "+WP2P_START",	WXCmd_WP2P_Start,   NULL, NULL },
	{ "+WP2P_STOP",		WXCmd_WP2P_Stop,   NULL, NULL },
	{ "+WP2P_PEERLIST",	WXCmd_WP2P_PeerList,   NULL, NULL },
	{ "+WP2P_INVITE=",	WXCmd_WP2P_Invite,   NULL, NULL },

	// kaizen 20140410 ID1168 Customize for ShinHeung
	{ "+MCUSTOM=",	WXCmd_MCUSTOM, NULL, NULL },

	// kaizen 20140430 ID1173 - Added Function of Custom GPIO TYPE
	{ "+MCSTGPIO=",	WXCmd_MCSTGPIO, NULL, NULL },

	// sekim 20140710 ID1180 add air command
	{ "+MAIRCMD=", WXCmd_MAIRCMD,     "Air Command Option", "=? or =<OpenTyp>,<Port>,<Option>" },

	// sekim 20150107 added WXCmd_WRFMODE
	{ "+WRFMODE=", WXCmd_WRFMODE,     "RF Interference mode", "=? or =<mode> (0~4)" },

	// kaizen 20140905 flag of printing [LISTEN] message for mbed library
	{ "+MEVTMSG=", WXCmd_MEVTMSG,     "", "=0 or 1" },

	// sekim 20150416 add AT+MEVTFORM, AT+MEVTDELAY for Coway
	{ "+MEVTFORM=",  WXCmd_MEVTFORM,  "Define Event Message Format", "=? or =<Format> (Format 0:no display, 1:full text, 2:code)" },
	{ "+MEVTDELAY=", WXCmd_MEVTDELAY, "Event Message Delay", "=? or =<delay> (10ms~1000ms)" },

	// sekim 20150508 Coway ReSend <Connect Event Message>
	{ "+COWAYA=", WXCmd_COWAYA, "ReSend Connect Event Message", "=? or =<time>,<check-data>" },

	// sekim XXXXXX 20151222 Add GMMP Command
	{ "+GMMPSET=", WXCmd_GMMPSET, "GMMP Set Configuration", "=? or =<Mode>,<ServerIP>,<Port>,<DomainCode>,<GWAuthID>,<GWMFID>,<DeviceID>" },
	{ "+GMMPDATA=", WXCmd_GMMPDATA, "GMMP Delivery Data", "=? or =<Option>,<DeliveryData>" },
	{ "+GMMPOPT=", WXCmd_GMMPOPT, "GMMP Option", "=? or =<Option>" },

	// daniel 160630 MQTT Command (copied WizFi310 format)
	{ "+MQTTSET=", WXCmd_MQTTSET,"MQTT Set Configuration", "=? or =<user>,<Password>,<Client-ID>,<Alive>" },
	{ "+MQTTCON=", WXCmd_MQTTCON,"MQTT Status/Connect/Disconnect", "=? or =<1/0, Connect/Disconnect>,<IP>,<Port>,<1/0, SSL Enable>" },
	{ "+MQTTSUB=", WXCmd_MQTTSUB,"MQTT Subscribe/Unsubscribe Topic", "=? or =<1/0, Subscribe/Unsubscribe><Topic>" },
	{ "+MQTTPUB=", WXCmd_MQTTPUB,"MQTT Publish Topic", "=? or =<Topic><Message Length>" },


	{ NULL, NULL, NULL }	// Should be last item
#endif
};

const char *g_wxCodeMessageList[] =
{ 
	"[OK]\r\n",                        // WXCODE_SUCCESS
	"[ERROR]\r\n",                     // WXCODE_FAILURE
	"[ERROR: INVALID INPUT]\r\n",      // WXCODE_EINVAL
	"[ERROR: INVALID SCID]\r\n",       // WXCODE_EBADCID
	// sekim add WXCODE_WIFISTATUS_ERROR
	"[ERROR: WiFi Status]\r\n",        // WXCODE_WIFISTATUS_ERROR
	"[ERROR: Mode Status]\r\n",        // WXCODE_MODESTATUS_ERROR
	"\r\n[CONNECT %x]\r\n",            // WXCODE_CON_SUCCESS
	"\r\n[DISCONNECT %x]\r\n",         // WXCODE_ECIDCLOSE
	"\r\n[Link-Up Event]\r\n",         // WXCODE_LINKUP
	"\r\n[Link-Down Event]\r\n",       // WXCODE_LINKDOWN
	"\r\n[Reset Event]\r\n",           // WXCODE_RESET
	// sekim 20131212 ID1141 add g_socket_extx_option
	"\r\n[TCP Send Error]\r\n",        // WXCODE_TCPSENDERROR
	// sekim 20130429 add WXCODE_P2PFAIL
	"\r\n[P2P Fail Event]\r\n",        // WXCODE_P2PFAIL
	// kaizen 20140529 add WXCODE_LISTEN
	"\r\n[LISTEN %x]\r\n",             // WXCODE_LISTEN

	/*
	"\r\n[CONNECT %x]\r\n",            // WXCODE_CON_SUCCESS
	"\r\n[DISCONNECT %x]\r\n",         // WXCODE_ECIDCLOSE
	"\r\n[Link-Up Event]\r\n",         // WXCODE_LINKUP
	"\r\n[Link-Down Event]\r\n",       // WXCODE_LINKDOWN
	*/
};

////////////////////////////////////////////////////////////////////////////////////
// sekim 20150416 add AT+MEVTFORM, AT+MEVTDELAY for Coway
const char *g_wxCodeSimpleMessageList[] =
{
	"\r\n[0]\r\n",      // Boot
	"\r\n[1]\r\n",      // WXCODE_CON_SUCCESS
	"\r\n[2]\r\n",      // WXCODE_ECIDCLOSE
	"\r\n[3]\r\n",      // WXCODE_LINKUP
	"\r\n[4]\r\n",      // WXCODE_LINKDOWN
	"\r\n[5]\r\n",      // WXCODE_RESET
	"\r\n[6]\r\n",      // WXCODE_TCPSENDERROR
	"\r\n[7]\r\n",      // WXCODE_P2PFAIL
	"\r\n[8]\r\n",      // WXCODE_LISTEN
	"",      			// Null
};
const char* ProcessSimpleMessage(UINT8 status)
{
	if ( status<WXCODE_CON_SUCCESS )	{ return g_wxCodeMessageList[status]; }
	if ( status>WXCODE_LISTEN )			{ W_DBG("ProcessSimpleMessage error 1 \r\n"); return g_wxCodeSimpleMessageList[9]; }

	UINT32 index = status - WXCODE_CON_SUCCESS + 1;
	UINT8 format = g_wxProfile.event_msg_format[index];

	if ( format=='0' )			return g_wxCodeSimpleMessageList[9];
	else if ( format=='1' )		return g_wxCodeMessageList[status];
	else if ( format=='2' )		return g_wxCodeSimpleMessageList[index];

	W_DBG("ProcessSimpleMessage error 2 \r\n");
	return g_wxCodeSimpleMessageList[9];
}
////////////////////////////////////////////////////////////////////////////////////


VOID WXS2w_StatusNotify(UINT8 status, UINT32 arg)
{
	const char *msg;

	if ( (status == WXCODE_ECIDCLOSE) && g_isAutoconnected && (g_currentScid==arg) )
	{
		g_currentScid = WX_INVALID_SCID;
	}

	if ( status > (sizeof(g_wxCodeMessageList) / sizeof(g_wxCodeMessageList[0])) )
	{
		W_DBG("WXS2w_StatusNotify error 100 : %d", status);
		return;
	}

	msg = g_wxCodeMessageList[status];
	if ( !msg )
	{
		W_DBG("WXS2w_StatusNotify error 110 : ");
		return;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////
	// sekim 20150508 Coway ReSend <Connect Event Message>
	extern UINT8 g_cowaya_recv_checkdata;
	if ( status==WXCODE_CON_SUCCESS || status==WXCODE_ECIDCLOSE )	g_cowaya_recv_checkdata = 0;
	///////////////////////////////////////////////////////////////////////////////////////////////////

	// sekim 20150416 add AT+MEVTFORM, AT+MEVTDELAY for Coway
	msg = ProcessSimpleMessage(status);

	// sekim 20150416 add AT+MEVTFORM, AT+MEVTDELAY for Coway
	if ( g_wxProfile.event_msg_delay>0 )
	{
		wiced_rtos_lock_mutex(&g_upart_type1_wizmutex);
		wiced_rtos_delay_milliseconds(g_wxProfile.event_msg_delay);
	}

	// sekim WXS2w_StatusNotify에서 W_EVT? W_RSP?
	//////////////////////////////////////////////////////////////////////////////////////////
	// sekim 20150114 POSBANK Disable Some Event Message, WXS2w_StatusNotify Bug Fix
	/*
	if ( WXCODE_CON_SUCCESS<status )	W_RSP(msg);
	else								W_EVT(msg);
	*/
	if( CHKCUSTOM("POSBANK") )
	{
		W_EVT(msg, arg);
	}
	else
	{
		if ( WXCODE_CON_SUCCESS<status )	W_RSP(msg, arg);
		else								W_EVT(msg, arg);
	}
	//////////////////////////////////////////////////////////////////////////////////////////

	// sekim 20150416 add AT+MEVTFORM, AT+MEVTDELAY for Coway
	if ( g_wxProfile.event_msg_delay>0 )
	{
		wiced_rtos_unlock_mutex(&g_upart_type1_wizmutex);
		wiced_rtos_delay_milliseconds(g_wxProfile.event_msg_delay);
	}

	if ( status==WXCODE_LINKUP && g_wxProfile.wifi_mode == 0 )		// Staion Mode
	{
		wiced_ip_address_t ip_address;
		wiced_ip_get_ipv4_address((wiced_interface_t)WICED_STA_INTERFACE, &ip_address);
		// sekim 20150416 add AT+MEVTFORM, AT+MEVTDELAY for Coway
		if ( g_wxProfile.event_msg_format[9]=='1' )
			W_EVT("  IP Addr    : %u.%u.%u.%u\r\n", (unsigned char) ( ( ( GET_IPV4_ADDRESS( ip_address ) ) >> 24 ) & 0xff ),(unsigned char) ( ( ( GET_IPV4_ADDRESS( ip_address ) ) >> 16 ) & 0xff ),(unsigned char) ( ( ( GET_IPV4_ADDRESS( ip_address ) ) >> 8 ) & 0xff ),(unsigned char) ( ( ( GET_IPV4_ADDRESS( ip_address ) ) >> 0 ) & 0xff ));

		wiced_ip_get_gateway_address( (wiced_interface_t)WICED_STA_INTERFACE, &ip_address );
		// sekim 20150416 add AT+MEVTFORM, AT+MEVTDELAY for Coway
		if ( g_wxProfile.event_msg_format[9]=='1' )
			W_EVT("  Gateway    : %u.%u.%u.%u\r\n", (unsigned char) ( ( ( GET_IPV4_ADDRESS( ip_address ) ) >> 24 ) & 0xff ),(unsigned char) ( ( ( GET_IPV4_ADDRESS( ip_address ) ) >> 16 ) & 0xff ),(unsigned char) ( ( ( GET_IPV4_ADDRESS( ip_address ) ) >> 8 ) & 0xff ),(unsigned char) ( ( ( GET_IPV4_ADDRESS( ip_address ) ) >> 0 ) & 0xff ));

		// sekim 20150107 added WXCmd_WRFMODE
		if ( wwd_wifi_set_interference_mode(g_wxProfile.rfmode1)!=WICED_SUCCESS )
		{
			W_DBG("Set Interference mode error(%d)", g_wxProfile.rfmode1);
		}
	}
	else if ( status==WXCODE_LINKUP && g_wxProfile.wifi_mode == 1 )		// AP Mode
	{
		wiced_ip_address_t ip_address;
		wiced_ip_get_ipv4_address((wiced_interface_t)WICED_AP_INTERFACE, &ip_address);
		// sekim 20150416 add AT+MEVTFORM, AT+MEVTDELAY for Coway
		if ( g_wxProfile.event_msg_format[9]=='1' )
			W_EVT("  IP Addr    : %u.%u.%u.%u\r\n", (unsigned char) ( ( ( GET_IPV4_ADDRESS( ip_address ) ) >> 24 ) & 0xff ),(unsigned char) ( ( ( GET_IPV4_ADDRESS( ip_address ) ) >> 16 ) & 0xff ),(unsigned char) ( ( ( GET_IPV4_ADDRESS( ip_address ) ) >> 8 ) & 0xff ),(unsigned char) ( ( ( GET_IPV4_ADDRESS( ip_address ) ) >> 0 ) & 0xff ));

		wiced_ip_get_gateway_address( (wiced_interface_t)WICED_AP_INTERFACE, &ip_address );
		// sekim 20150416 add AT+MEVTFORM, AT+MEVTDELAY for Coway
		if ( g_wxProfile.event_msg_format[9]=='1' )
			W_EVT("  Gateway    : %u.%u.%u.%u\r\n", (unsigned char) ( ( ( GET_IPV4_ADDRESS( ip_address ) ) >> 24 ) & 0xff ),(unsigned char) ( ( ( GET_IPV4_ADDRESS( ip_address ) ) >> 16 ) & 0xff ),(unsigned char) ( ( ( GET_IPV4_ADDRESS( ip_address ) ) >> 8 ) & 0xff ),(unsigned char) ( ( ( GET_IPV4_ADDRESS( ip_address ) ) >> 0 ) & 0xff ));
	}

	if ( ((status==WXCODE_ECIDCLOSE) && (!g_isAutoconnected)) || (status==WXCODE_EBADCID) )
	{
		g_wxModeState = WX_MODE_COMMAND;
		WXS2w_LEDIndication(2, 0, 0, 0, 0, 0);

		g_scTxIndex = 0;
	}
}

static wiced_result_t f_nagle_timer(void *arg)
{
	wiced_rtos_stop_timer(&g_nagle_timer.timer);

	struct WizFiQueue buffMessage;

	buffMessage.queue_id = 101;
	buffMessage.queue_opt = 0;

	wiced_rtos_push_to_queue(&g_queue_handle_wizfiqueue, (void*)&buffMessage, WICED_NO_WAIT);

	return WICED_SUCCESS;
}

static wiced_result_t f_autoesc_timer(void* arg)
{
	wiced_rtos_stop_timer(&g_autoesc_timer.timer);

	if ( g_auto_esc == 3 )
	{
		g_wxModeState = WX_MODE_COMMAND;
		WXS2w_LEDIndication(2, 0, 0, 0, 0, 0);

		g_auto_esc = 0;

		// sekim 20121107 After changed to <Data Mode>, Printf [OK]
		WXS2w_StatusNotify(WXCODE_SUCCESS, 0);
	}
	else
	{
		static char esc[4] = { '+', '+', '+', '+' };

    	wiced_rtos_stop_timer(&g_nagle_timer.timer);
    	wiced_rtos_reload_timer(&g_nagle_timer.timer);
    	wiced_rtos_start_timer(&g_nagle_timer.timer);

		UINT8   i=0;
		while (i < g_auto_esc)
		{
			WXS2w_DataCharProcess(esc[i++]);
		}
		g_auto_esc = 0;
	}

	return WICED_SUCCESS;
}

void ProcessActionButtonClicked(uint32_t BtnClickCount);
void action_after_linkup_callback();
void check_psocketlist_and_process_basedon_scon1_option();


VOID WXS2w_SerialInput(VOID)
{
	static char esc[4] = { '+', '+', '+', '+' };
	static UINT8 inited;
	static UINT8 prev = 0;
	UINT8 ch;

	if ( prev == 0 )
		prev = 0;

	if ( !inited )
	{
		inited = 1;
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	// sekim 20130403 Add Task Monitor (WXS2w_SerialInput)
	//while (WXHal_CharGet(&ch) == 1)	{
	while (1)
	{
		////////////////////////////////////////////////////////////////////////////////////////////
		// sekim 20140715 ID1181 add g_queue_handle_maincommand
		/*
		wizfi_task_monitor_stop = 1;
		WXHal_CharGet(&ch);
		wizfi_task_monitor_stop = 0;
		wiced_update_system_monitor(&wizfi_task_monitor_item, MAXIMUM_ALLOWED_INTERVAL_BETWEEN_WIZFIMAINTASK);
		*/
		while(1)
		{
			wiced_update_system_monitor(&wizfi_task_monitor_item, MAXIMUM_ALLOWED_INTERVAL_BETWEEN_WIZFIMAINTASK);

			if ( wiced_uart_receive_bytes(WICED_UART_1, &ch, 1, 100)==WICED_SUCCESS )
			{
				break;
			}

			//////////////////////////////////////////////////////////////////////////////////////////
			// sekim 20140715 ID1181 add g_queue_handle_maincommand
			{
				while(1)
				{
					WizFiQueue buffWizFiQueue;
					if ( wiced_rtos_pop_from_queue(&g_queue_handle_maincommand, &(buffWizFiQueue), 1)!=WICED_SUCCESS )
					{
						break;
					}
					//W_DBG("g_queue_handle_maincommand : (%d, %d) ", buffWizFiQueue.queue_id, buffWizFiQueue.queue_opt);

					if ( buffWizFiQueue.queue_id == 101 )		action_after_linkup_callback();
					else if ( buffWizFiQueue.queue_id == 102 )	wifi_link_down_callback();
					else if ( buffWizFiQueue.queue_id == 103 )	check_psocketlist_and_process_basedon_scon1_option();
					else if ( buffWizFiQueue.queue_id == 104 )	check_tcp_idle_time();
					else if ( buffWizFiQueue.queue_id == 105 )	ProcessActionButtonClicked(buffWizFiQueue.queue_opt);
					else if ( buffWizFiQueue.queue_id == 106 )	process_air_command_rx_data();
					else if ( buffWizFiQueue.queue_id == 107 )
					{
						if ( buffWizFiQueue.queue_opt==1 )		WXS2w_StatusNotify(WXCODE_P2PFAIL, 0);
				        WXS2w_SystemReset();
					}
				}
			}
			//////////////////////////////////////////////////////////////////////////////////////////
		}
		//////////////////////////////////////////////////////////////////////////////////////////

#if 1 //MikeJ 130410 ID1034 - Add ECHO on/off function
		if ( g_wxModeState == WX_MODE_COMMAND && g_wxProfile.echo_mode == TRUE )
		{
			if(ch == 0x0d) {
				WXHal_CharPut(0x0d);
				WXHal_CharPut(0x0a);
			} else if(ch != 0x0a) {
				WXHal_CharPut(ch);
			}
		}
#else
		if ( (g_wxModeState == WX_MODE_COMMAND) )
		{
			WXHal_CharPut(ch);
		}
#endif

		switch (g_wxModeState)
		{
		case WX_MODE_COMMAND:
			WXS2w_CommandCharProcess(ch);
			break;

		case WX_MODE_DATA:

			// Handle auto connect mode escape sequence.
			if ( ch == '+' && g_auto_esc < 3 )
			{
				g_auto_esc++;

		    	wiced_rtos_stop_timer(&g_autoesc_timer.timer);
		    	wiced_rtos_reload_timer(&g_autoesc_timer.timer);
		    	wiced_rtos_start_timer(&g_autoesc_timer.timer);
			}
			else if ( g_auto_esc )
			{
				UINT8 i;

				esc[g_auto_esc] = ch;

				i = 0;
				while (i <= g_auto_esc)
				{
					WXS2w_DataCharProcess(esc[i++]);
				}

				esc[g_auto_esc] = '+';
				g_auto_esc = 0;
			}
			else
			{
				g_auto_esc = 0;
				WXS2w_DataCharProcess(ch);
			}

			break;

		}
		prev = ch;
	}
}

// sekim 20140715 ID1181 add g_queue_handle_maincommand
void send_maincommand_queue(uint16_t queue_id, uint16_t queue_opt)
{
	WizFiQueue buffWizFiQueue;

	// if not-urgent-job can be passed
	if ( queue_id==103 || queue_id==104 )
	{
		if ( wiced_rtos_is_queue_empty(&g_queue_handle_maincommand)!=WICED_SUCCESS )
		{
			//W_DBG("send_maincommand_queue : if not-urgent-job can be passed");
			return;
		}
	}

	buffWizFiQueue.queue_id = queue_id;
	buffWizFiQueue.queue_opt = queue_opt;

	wiced_rtos_push_to_queue(&g_queue_handle_maincommand, (void*)&buffWizFiQueue, WICED_NO_WAIT);
}

void Create_Queue_Timer()
{
	if ( wiced_rtos_init_queue(&g_queue_handle_wizfiqueue, "WizFi queue", sizeof(struct WizFiQueue ), 20) != WICED_SUCCESS )
	{
		W_DBG("Create_Queue_Timer : error");
		return;
	}

	// sekim 20140715 ID1181 add g_queue_handle_maincommand
	if ( wiced_rtos_init_queue(&g_queue_handle_maincommand, "Command queue", sizeof(struct WizFiQueue ), 20) != WICED_SUCCESS )
	{
		W_DBG("Create_Queue_Timer : error 2");
		return;
	}

	void ProcessWizFiQueue(uint32_t arguments);
	if ( WICED_SUCCESS!=wiced_rtos_create_thread(&g_thread_handle_wizfiqueue, WICED_APPLICATION_PRIORITY + 2, "WizFiQueueTask", ProcessWizFiQueue, (1024*4), NULL) )
	{
		W_DBG("wiced_rtos_create_thread : ProcessWizFiQueue error");
	}

	wiced_rtos_init_mutex(&g_s2w_wizmutex);
	wiced_rtos_init_mutex(&g_something_wizmutex);
	wiced_rtos_init_mutex(&g_upart_type1_wizmutex);
	wiced_rtos_init_mutex(&g_socketopen_wizmutex);

	// sekim 20131212 ID1141 add g_socket_extx_option
	if ( wiced_rtos_register_timed_event(&g_nagle_timer, WICED_NETWORKING_WORKER_THREAD, &f_nagle_timer, g_wxProfile.socket_ext_option4, 0)!=WICED_SUCCESS )
	{
		W_DBG("wiced_rtos_register_timed_event : g_nagle_timer error");
	}
	wiced_rtos_stop_timer(&g_nagle_timer.timer);
	if ( wiced_rtos_register_timed_event(&g_autoesc_timer, WICED_NETWORKING_WORKER_THREAD, &f_autoesc_timer, g_wxProfile.timer_autoesc, 0)!=WICED_SUCCESS )
	{
		W_DBG("wiced_rtos_register_timed_event : g_autoesc_timer error");
	}
	wiced_rtos_stop_timer(&g_autoesc_timer.timer);
}

VOID WXS2w_LoadConfiguration(VOID)
{
	Load_Profile();

#if 1	// kaizen 20131018 -ID1129 Add FW_VERSION Check Routine
	if( strcmp((char*)g_wxProfile.fw_version,WIZFI250_FW_VERSION) != 0 )
	{
		Default_Profile();
		Save_Profile();
	}
#endif

	Apply_To_Wifi_Dct();
}

////////////////////////////////////////////////////////////////////////////////////////////////
// sekim 20150902 Add recovery DCT function
wiced_timed_event_t g_bckup_recovery_timer;
static wiced_result_t f_bckup_recovery_timer(void *arg)
{
	wiced_rtos_stop_timer(&g_bckup_recovery_timer.timer);

	void RecoveryDCT(WT_PROFILE* p_dct, int bBackup);
	RecoveryDCT(&g_wxProfile, 0);
	g_wxProfile.bckup_start =0 ;

	wiced_dct_write_app_section( &g_wxProfile, sizeof(WT_PROFILE) );
	WXS2w_SystemReset();

	return WICED_SUCCESS;
}
////////////////////////////////////////////////////////////////////////////////////////////////

VOID WXS2w_Initialize(VOID)
{
	Create_Queue_Timer();

	// kaizen
	//WXLink_Disassociate(0, 0);
	WXLink_Disassociate(0, 0, WICED_STA_INTERFACE);

	// sekim 20120305 To Be Implemented (Load Profile)

	// sekim wiced_management_wifi_off 후에 WiFi 동작 오류
	/*
	// Apply wifi_countrycode
	wiced_management_wifi_off();
	wiced_country_code_t pCode = MK_CNTRY(g_wxProfile.wifi_countrycode[0],g_wxProfile.wifi_countrycode[1],0);
	wiced_set_country(*pCode);
	wiced_management_wifi_on();
	*/



#if 1 	// kaizen 20131118 ID1138 Solved Problem about strange operation when change AP mode using GPIO
	if ( strcmp((char*)g_wxProfile.scon_opt1, "SO") == 0 || strcmp((char*)g_wxProfile.scon_opt1, "S") == 0 || g_wxProfile.gpio_wifi_auto_conn == 1 )
	{
		if ( WXCmd_WJOIN(0) == WXCODE_SUCCESS )
		{
		}

		if ( g_wxProfile.gpio_wifi_auto_conn == 1 )
		{
			WT_PROFILE temp_dct;
			wiced_dct_read_app_section( &temp_dct, sizeof(WT_PROFILE) );
			temp_dct.gpio_wifi_auto_conn = 0;
			g_wxProfile.gpio_wifi_auto_conn = 0;	// kaizen 20131206

			wiced_dct_write_app_section( &temp_dct, sizeof(WT_PROFILE) );

			char szBuff[10] = { 0, };
			strcpy(szBuff, "1");
			WXCmd_FWEBS((UINT8*)szBuff);

			// sekim 20140715 URIEL Customizing but apply for all
			strcpy((char*)g_wxProfile.aircmd_opentype, "O");
			g_wxProfile.aircmd_mode = 'T';
			g_wxProfile.aircmd_port = 50001;
			g_wxProfile.aircmd_opt = 1;
			void AirCommandStart();
			AirCommandStart();

			//////////////////////////////////////////////////////////////////////////////////////////
			// sekim 20150902 Add recovery DCT function
			// if mext1_1==11, recovery DCT function start
			if ( g_wxProfile.mext1_1==11 )
			{
				int time_bckup_recovery = 5*60;
				if ( g_wxProfile.mext1_1>0 )	time_bckup_recovery = g_wxProfile.mext1_2;

				if ( wiced_rtos_register_timed_event(&g_bckup_recovery_timer, WICED_NETWORKING_WORKER_THREAD, &f_bckup_recovery_timer, time_bckup_recovery*1000, 0)!=WICED_SUCCESS )
				{
					W_DBG("wiced_rtos_register_timed_event : g_autoesc_timer error");
				}
				wiced_rtos_start_timer(&g_bckup_recovery_timer.timer);
			}
			//////////////////////////////////////////////////////////////////////////////////////////
		}
		else
		{
			// sekim 20140731 just simple a bug
			//if ( g_wxProfile.aircmd_mode=='S' )
			if ( g_wxProfile.aircmd_opentype[0]=='S' || g_wxProfile.aircmd_opentype[1]=='S')
			{
				void AirCommandStart();
				AirCommandStart();
			}
		}

		// sekim 20140710 ID1179 add wcheck_option
		wifi_check_process();
	}
#else
	if ( strcmp((char*)g_wxProfile.scon_opt1, "SO") == 0 || strcmp((char*)g_wxProfile.scon_opt1, "S") == 0 )
	{
		if ( WXCmd_WJOIN(0) == WXCODE_SUCCESS )
		{
			// sekim Auto Connection (Service), Booting time => Link-up callback
			/*
			char szBuff[128] = { 0, };
			sprintf(szBuff, "SO,%s,%s,%d,%d,%d", g_wxProfile.scon_opt2, g_wxProfile.scon_remote_ip, g_wxProfile.scon_remote_port, g_wxProfile.scon_local_port, g_wxProfile.scon_datamode);
			if ( WXCmd_SCON((UINT8*)szBuff) == WXCODE_SUCCESS )
			{
			}
			*/

#if 0	// kaizen 20130513 For Web Server Service Option	// kaizen 20130726 ID1105 Modified code in order to launch web server when link up event
			if( strcmp((char*)g_wxProfile.web_server_opt, "SO") == 0 )
			{
				char szBuff[10] = { 0, };
				sprintf(szBuff, "1,%s",g_wxProfile.web_server_opt);
				WXCmd_FWEBS((UINT8*)szBuff);
			}
#endif
		}
	}
#endif

}

VOID WXS2w_CommandCharProcess(UINT8 ch)
{
	static UINT8 prevBuf[WX_CMDBUF_SIZE];
	static UINT8 buf[WX_CMDBUF_SIZE];
	static UINT32 index;

#if 1 //MikeJ 130410 ID1034 - Add ECHO on/off function
	if ( ch == WXASCII_LF )
	{
		return;
	}
	else if ( ch == WXASCII_CR )
	{
		if ( index == 0 ) return;
#else
	if ( ch == WXASCII_CR || ch == WXASCII_LF )
	{
		if ( index == 0 )
		{
			W_RSP("\r\n");
			return;
		}
#endif
		// End of command; Call function to process the command.
		buf[index] = '\0';
		memcpy(prevBuf, buf, index + 1);
		index = 0;
		WXS2w_Process(buf);
		memset(buf, 0, WX_CMDBUF_SIZE);

		// Store the command characters in the command buffer and also handle the backspaces.
	}
	else if ( ch == WXASCII_BACKSPC )
	{
		index = index ? (index - 1) : 0;
	}
	else if ( index == 1 && ch == '/' && toupper(buf[0]) == 'A' )
	{
		index = 0;

		strcpy((char *) buf, (const char *) prevBuf);
		WXS2w_Process(buf);
	}
	else if ( index < sizeof(buf) - 1 )
	{
		buf[index++] = ch;
	}
}

UINT8 WXS2w_DataBufferTransmit(VOID)
{
	UINT8 status;

	if ( !g_scTxIndex )
	{
		return WXCODE_SUCCESS;
	}

	if ( g_wxModeState == WX_MODE_DATA && (!WXNetwork_NetIsCidOpen(g_currentScid)) )
	{
		W_DBG("WXS2w_DataBufferTransmit : mode error");
	}

	////////////////////////////////////////////////////////////////////////
	// sekim 20150508 Coway ReSend <Connect Event Message>
	if ( g_wxProfile.cowaya_check_time>0 )
	{
		if ( memcmp(g_scTxBuffer, g_wxProfile.cowaya_check_data, strlen(g_wxProfile.cowaya_check_data))==0 )
		{
			extern UINT8 g_cowaya_recv_checkdata;
			g_cowaya_recv_checkdata = 1;
		}
	}
	////////////////////////////////////////////////////////////////////////


	status = WXNetwork_NetTx(g_currentScid, g_scTxBuffer, g_scTxIndex);
	if ( status != WXCODE_SUCCESS )
	{
		//W_DBG("WXS2w_DataBufferTransmit : WXNetwork_NetTx error");
	}

	g_scTxIndex = 0;
	return status;
}

void ProcessWizFiQueue(uint32_t arguments)
{
	WizFiQueue buffWizFiQueue;

	while (1)
	{
        if ( wiced_rtos_pop_from_queue(&g_queue_handle_wizfiqueue, &(buffWizFiQueue), 500)!=WICED_SUCCESS )
        {
        	continue;
        }

		if ( buffWizFiQueue.queue_id == 101 )
		{
			wiced_rtos_lock_mutex(&g_s2w_wizmutex);

			UINT8 status;
			status = WXS2w_DataBufferTransmit();

			if ( status != WXCODE_SUCCESS )
			{
				WXS2w_StatusNotify(status, 0);
			}

			wiced_rtos_unlock_mutex(&g_s2w_wizmutex);
		}
		else if ( buffWizFiQueue.queue_id == 111 )
		{
			wiced_result_t result;
			UINT8 scid = buffWizFiQueue.queue_opt;
    		int kill_count;
    		for (kill_count=0; kill_count<10; kill_count++)
    		{
    			wiced_rtos_delay_milliseconds(10);
    		    if ( (result=wiced_rtos_is_current_thread(&g_socketopen_thread[scid]))!=WICED_SUCCESS )
    		    {
    		    	if ( (result=wiced_rtos_delete_thread(&g_socketopen_thread[scid]))!=WICED_SUCCESS )
    		    		W_DBG("ProcessWizFiQueue : wiced_rtos_delete_thread error (%d) %d", kill_count, result);
    		    	else
    		    		break;
    		    }
    		    else
    		    {
    		    	W_DBG("ProcessWizFiQueue : wiced_rtos_is_current_thread error %d", result);
    		    }
    		}
		}
	}
}


VOID WXS2w_DataCharProcess(UINT8 ch)
{
	UINT8 status;

	if ( g_wxModeState == WX_MODE_DATA )
	{
		// TCP server without a connection
		if ( g_currentScid == WX_INVALID_SCID )
		{
			// sekim TCP Server but, nothing connected. Ignore data.
			//W_DBG("WXS2w_DataCharProcess : error 101");
			return;
		}

		// Locking to avoid race with expiry timer and send task
		wiced_rtos_lock_mutex(&g_s2w_wizmutex);

		g_scTxBuffer[g_scTxIndex] = ch;
		g_scTxIndex++;

		////////////////////////////////////////////////////////////////////////////////////
		// sekim Nagle Timer 없이 무조건 Transmit
#if 1	// kaizen	20130514 ID 1062 - Fixed Bug that WizFi250 was hang up when set <UDP data mode> and send file more than 2Kbyte
		if ( g_scTxIndex > WX_MAX_PACKET_SIZE )
#else
		if ( g_scTxIndex > sizeof(g_scTxBuffer))
#endif
		{
			status = WXS2w_DataBufferTransmit();
			if ( status != WXCODE_SUCCESS )
			{
				W_DBG("WXS2w_DataCharProcess : error 211");
				WXS2w_StatusNotify(status, 0);
			}
		}
		else
		{
			if ( (g_scTxIndex == 1) || ((g_scTxIndex % 100) == 0) )
			{
		    	wiced_rtos_stop_timer(&g_nagle_timer.timer);
		    	wiced_rtos_reload_timer(&g_nagle_timer.timer);
		    	wiced_rtos_start_timer(&g_nagle_timer.timer);
			}
		}
		////////////////////////////////////////////////////////////////////////////////////

		wiced_rtos_unlock_mutex(&g_s2w_wizmutex);
	}
	else
	{
		W_DBG("WXS2w_DataCharProcess : error 110");
	}
}

VOID WXS2w_Process(UINT8 *cmd)
{
	UINT8 status = WXCODE_EINVAL;
	UINT8 *ptr;

	UINT32 i;

#if 0 //MikeJ 130410 ID1034 - Add ECHO on/off function
	W_RSP("\r\n");
#endif

	if ( toupper(cmd[0]) != 'A' || toupper(cmd[1]) != 'T' )
	{
		WXS2w_StatusNotify(WXCODE_EINVAL, 0);
		return;
	}
	if ( cmd[2] == '\0' )
	{
		WXS2w_StatusNotify(WXCODE_SUCCESS, 0);
		return;
	}
	ptr = (cmd + 2);

	for(i = 0; i < sizeof(g_WXCmdTable) / sizeof(g_WXCmdTable[0]); i++)
	{
		UINT32 len = strlen(g_WXCmdTable[i].cmd);

		if ( !WXParse_StrnCaseCmp((char *) ptr, g_WXCmdTable[i].cmd, len) )
		{
			status = g_WXCmdTable[i].process(ptr + len);
			break;
		}
	}

	WXS2w_StatusNotify(status, 0);

	return;
}

// sekim 20130318 LED Indication (Association LED, Data mode LED)
#include "wiced_platform.h"
VOID WXS2w_LEDIndication(UINT8 led, UINT8 init, UINT8 repeat, UINT8 delay1, UINT8 delay2, UINT8 on)
{
	UINT32 i;

	if ( init==1 )
	{
	    wiced_gpio_init(WICED_LED1, OUTPUT_PUSH_PULL);
	    wiced_gpio_init(WICED_LED2, OUTPUT_PUSH_PULL);

	    wiced_gpio_output_high(WICED_LED1);
	    wiced_gpio_output_high(WICED_LED2);
	}

	for (i=0; i<repeat; i++)
	{
		if ( led==0 || led==1 )	wiced_gpio_output_low(WICED_LED1);
		if ( led==0 || led==2 )	wiced_gpio_output_low(WICED_LED2);
		wiced_rtos_delay_milliseconds(delay1);
		if ( led==0 || led==1 )	wiced_gpio_output_high(WICED_LED1);
		if ( led==0 || led==2 )	wiced_gpio_output_high(WICED_LED2);
		wiced_rtos_delay_milliseconds(delay2);
	}

	if ( (led==0 || led==1) && on==0 )	wiced_gpio_output_high(WICED_LED1);
	if ( (led==0 || led==1) && on!=0 )	wiced_gpio_output_low(WICED_LED1);
	if ( (led==0 || led==2) && on==0 )	wiced_gpio_output_high(WICED_LED2);
	if ( (led==0 || led==2) && on!=0 )	wiced_gpio_output_low(WICED_LED2);
}

#if 1	// kaizen 20130520 ID1039 - When Module Reset, Socket close & Disassociation
#include "wx_commands_w.h"
#include "wx_commands_s.h"
VOID WXS2w_SystemReset()
{
	WXCmd_SMGMT	((UINT8*)"ALL");
	WXCmd_WLEAVE((UINT8*)"");

	wiced_rtos_delay_milliseconds( 500 * MILLISECONDS );	// waiting socket close & Disassociation

	NVIC_SystemReset();
}
#endif

#if 1 // kaizen 20130814 Modified AT+FGPIO function
VOID WXS2w_GPIO_Init(VOID)
{
	UINT8 i;
	UINT32 gpio_num, gpio_config_value, gpio_set_value;

	for(i=0; i<USER_GPIO_MAX_COUNT; i++)
	{
		gpio_num 			= g_wxProfile.user_gpio_conifg[i].gpio_num - 1;
		gpio_config_value 	= g_wxProfile.user_gpio_conifg[i].gpio_config_value;
		wiced_gpio_init(gpio_num, gpio_config_value);

		if ( g_wxProfile.user_gpio_conifg[i].mode == 1 )	// if output mode
		{
			gpio_set_value = g_wxProfile.user_gpio_conifg[i].gpio_set_value;
			if 		( gpio_set_value == GPIO_SET_LOW )	wiced_gpio_output_low(gpio_num);
			else if	( gpio_set_value == GPIO_SET_HIGH )	wiced_gpio_output_high(gpio_num);
		}
	}

}
#endif


