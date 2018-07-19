
#include "wx_defines.h"

#include "../wizfi_wiced.h"
#include "wiced_wifi.h"
#include "wwd_assert.h"
#include "wiced_management.h"
#include "platform_dct.h"
#include "wiced_dct.h"
#include "dns.h"

#define CMP_MAC( a, b )  (((a[0])==(b[0]))&& ((a[1])==(b[1]))&& ((a[2])==(b[2]))&& ((a[3])==(b[3]))&& ((a[4])==(b[4]))&& ((a[5])==(b[5])))
#define NULL_MAC( a )  (((a[0])==0)&& ((a[1])==0)&& ((a[2])==0)&& ((a[3])==0)&& ((a[4])==0)&& ((a[5])==0))

extern UINT8 web_server_status;										// kaizen 20130510 For Modified Web Server Status ( Run or Stop ). It is not saved at flash memory.

#define CIRCULAR_RESULT_BUFF_SIZE  10
typedef struct
{
	host_semaphore_type_t num_scan_results_semaphore;
	uint16_t result_buff_write_pos;
	uint16_t result_buff_read_pos;
	wiced_scan_result_t result_buff[CIRCULAR_RESULT_BUFF_SIZE];
	wiced_mac_t bssid_list[50];
} scan_user_data_t;

static void scan_results_handler(wiced_scan_result_t ** result_ptr, void * user_data)
{
	scan_user_data_t* data = (scan_user_data_t*) user_data;
	if ( result_ptr == NULL )
	{
		/* finished */
		data->result_buff[data->result_buff_write_pos].channel = 0xff;
		host_rtos_set_semaphore(&data->num_scan_results_semaphore, WICED_FALSE);
		return;
	}

	wiced_scan_result_t* record = (*result_ptr);

	/* Check the list of BSSID values which have already been printed */
	wiced_mac_t * tmp_mac = data->bssid_list;
	while (!NULL_MAC( tmp_mac->octet ))
	{
		if ( CMP_MAC( tmp_mac->octet, record->BSSID.octet ) )
		{
			/* already seen this BSSID */
			return;
		}
		tmp_mac++;
	}

	/* New BSSID - add it to the list */
	memcpy(&tmp_mac->octet, record->BSSID.octet, sizeof(wiced_mac_t));

	data->result_buff_write_pos++;
	if ( data->result_buff_write_pos >= CIRCULAR_RESULT_BUFF_SIZE )
	{
		data->result_buff_write_pos = 0;
	}
	*result_ptr = &data->result_buff[data->result_buff_write_pos];
	host_rtos_set_semaphore(&data->num_scan_results_semaphore, WICED_FALSE);

	wiced_assert( "Circular result buffer overflow", data->result_buff_write_pos != data->result_buff_read_pos );
}

// sekim 20131024 ID1131 AP Scan Time Control
//int scanwifi2(wiced_ssid_t* optional_ssid, wiced_mac_t* optional_mac, uint16_t* optional_channel_list)
int scanwifi2(wiced_ssid_t* optional_ssid, wiced_mac_t* optional_mac, uint16_t* optional_channel_list, UINT32 scan_time)
{
	scan_user_data_t* data = (scan_user_data_t*) malloc(sizeof(scan_user_data_t));
	memset(data, 0, sizeof(scan_user_data_t));
	host_rtos_init_semaphore(&data->num_scan_results_semaphore);
	wiced_scan_result_t * result_ptr = (wiced_scan_result_t *) &data->result_buff;
	int record_number = 1;

	// sekim 20121113 Modified scanwifi2 and wiced_wifi_scan for optional_channel_list
	//if ( WICED_SUCCESS != wiced_wifi_scan( WICED_SCAN_TYPE_ACTIVE, WICED_BSS_TYPE_ANY, NULL, NULL, NULL, NULL, scan_results_handler, &result_ptr, data ) )
#if 0 //MikeJ 130820 ID1125 - SCAN Option problem
	if ( WICED_SUCCESS != wiced_wifi_scan(WICED_SCAN_TYPE_ACTIVE, WICED_BSS_TYPE_ANY, optional_ssid, optional_mac, NULL, NULL, scan_results_handler, &result_ptr, data) )
#else
	wiced_scan_extended_params_t scan_ex_param;
	scan_ex_param.number_of_probes_per_channel = 5;
	// sekim 20131024 ID1131 AP Scan Time Control
	//scan_ex_param.scan_active_dwell_time_per_channel_ms = 70;
	scan_ex_param.scan_active_dwell_time_per_channel_ms = scan_time;
	scan_ex_param.scan_home_channel_dwell_time_between_channels_ms = 1;
	scan_ex_param.scan_passive_dwell_time_per_channel_ms = 150;
	if ( WICED_SUCCESS != wiced_wifi_scan(WICED_SCAN_TYPE_ACTIVE, WICED_BSS_TYPE_ANY, optional_ssid, optional_mac, optional_channel_list, &scan_ex_param, scan_results_handler, &result_ptr, data) )
#endif
	{
#if 0 //MikeJ 130702 ID1093 - Adjust Response Format
		W_RSP("Error starting scan\r\n");
#else
		W_DBG("Error starting scan\r\n");
#endif
		host_rtos_deinit_semaphore(&data->num_scan_results_semaphore);
		free(data);
		return 1;
	}
#if 0 //MikeJ 130702 ID1093 - Adjust Response Format
	W_RSP("Waiting for scan results...\r\n");
#else
	////////////////////////////////////////////////////////////////////////////////////////////////////
	// sekim 20141022 ENCORED Simple-WiFi-Scan in AirCmd
	//W_RSP("Index/SSID/BSSID/RSSI(-dBm)/MaxDataRate(Mbps)/Security/RadioBand(GHz)/Channel\r\n");
	if ( g_aircmd_status==3 )
		W_RSP("Index/SSID/RSSI(-dBm)/Security/Channel\r\n");
	else
		W_RSP("Index/SSID/BSSID/RSSI(-dBm)/MaxDataRate(Mbps)/Security/RadioBand(GHz)/Channel\r\n");
	////////////////////////////////////////////////////////////////////////////////////////////////////

#endif

	while (host_rtos_get_semaphore(&data->num_scan_results_semaphore, NEVER_TIMEOUT, WICED_FALSE) == WICED_SUCCESS)
	{
		int k;

		wiced_scan_result_t* record = &data->result_buff[data->result_buff_read_pos];
		if ( record->channel == 0xff )
		{
			/* Scan completed */
			break;
		}

		//////////////////////////////////////////////////////////////////////////////////////////
		// sekim 20121113 Modified scanwifi2 and wiced_wifi_scan for optional_channel_list
		int nSkip = 0;
		if ( optional_channel_list )
		{
			if ( optional_channel_list[0] != record->channel && optional_channel_list[0] != 0 )
				nSkip = 1;
#if 1 //MikeJ 130820 ID1125 - SCAN Option problem
			if ( optional_mac != NULL && memcmp(record->BSSID.octet, optional_mac, 6) != 0 ) {
				nSkip = 1;
			}
#endif
		}
		if ( !nSkip )
		//////////////////////////////////////////////////////////////////////////////////////////
		{
#if 0 //MikeJ 130702 ID1093 - Adjust Response Format
			/* Print SSID */
			W_RSP("\r\n#%03d SSID          : ", record_number);
			for(k = 0; k < record->SSID.len; k++)
			{
				W_RSP("%c", record->SSID.val[k]);
			}
			W_RSP("\r\n");

			wiced_assert( "error", ( record->bss_type == WICED_BSS_TYPE_INFRASTRUCTURE ) || ( record->bss_type == WICED_BSS_TYPE_ADHOC ) );

			/* Print other network characteristics */
			W_RSP("     BSSID         : %02X:%02X:%02X:%02X:%02X:%02X\r\n", record->BSSID.octet[0], record->BSSID.octet[1], record->BSSID.octet[2], record->BSSID.octet[3], record->BSSID.octet[4], record->BSSID.octet[5]);
			W_RSP("     RSSI          : %ddBm\r\n", record->signal_strength);
			W_RSP("     Max Data Rate : %.1f Mbits/s\r\n", (float) record->max_data_rate / 1000.0);
			W_RSP("     Network Type  : %s\r\n", ( record->bss_type == WICED_BSS_TYPE_INFRASTRUCTURE ) ? "Infrastructure" : ( record->bss_type == WICED_BSS_TYPE_ADHOC ) ? "Ad hoc" : "Unknown");
			W_RSP("     Security      : %s\r\n", ( record->security == WICED_SECURITY_OPEN ) ? "Open" : ( record->security == WICED_SECURITY_WEP_PSK ) ? "WEP" : ( record->security == WICED_SECURITY_WPA_TKIP_PSK ) ? "WPA" : ( record->security == WICED_SECURITY_WPA2_AES_PSK ) ? "WPA2 AES" : ( record->security == WICED_SECURITY_WPA2_MIXED_PSK ) ? "WPA2 Mixed" : "Unknown");
			W_RSP("     Radio Band    : %s\r\n", ( record->band == WICED_802_11_BAND_5GHZ ) ? "5GHz" : "2.4GHz");
			W_RSP("     Channel       : %d\r\n", record->channel);
#else
			/* Print SSID */
#if 0 //MikeJ 130820 ID1125 - SCAN Option problem
			W_RSP("%03d/", record_number);
#else
			W_RSP("%03d/", record_number++);
#endif
			for(k = 0; k < record->SSID.len; k++)
			{
				W_RSP("%c", record->SSID.val[k]);
			}
			wiced_assert( "error", ( record->bss_type == WICED_BSS_TYPE_INFRASTRUCTURE ) || ( record->bss_type == WICED_BSS_TYPE_ADHOC ) );

			/* Print other network characteristics */
			////////////////////////////////////////////////////////////////////////////////////////////////////
			// sekim 20141022 ENCORED Simple-WiFi-Scan in AirCmd
			if ( g_aircmd_status==3 )
			{
				W_RSP("/%d", record->signal_strength);
				W_RSP("/%s", ( record->security == WICED_SECURITY_OPEN ) ? "Open" : ( record->security == WICED_SECURITY_WEP_PSK ) ? "WEP" : ( record->security == WICED_SECURITY_WPA_TKIP_PSK ) ? "WPA" : ( record->security == WICED_SECURITY_WPA2_AES_PSK ) ? "WPA2" : ( record->security == WICED_SECURITY_WPA2_MIXED_PSK ) ? "WPA2-Mixed" : "Unknown");
				W_RSP("/%d\r\n", record->channel);
			}
			else
			{
				W_RSP("/%02X:%02X:%02X:%02X:%02X:%02X", record->BSSID.octet[0], record->BSSID.octet[1], record->BSSID.octet[2], record->BSSID.octet[3], record->BSSID.octet[4], record->BSSID.octet[5]);
				W_RSP("/%d", record->signal_strength);
				W_RSP("/%.1f", (float) record->max_data_rate / 1000.0);
				W_RSP("/%s", ( record->bss_type == WICED_BSS_TYPE_INFRASTRUCTURE ) ? "Infra" : ( record->bss_type == WICED_BSS_TYPE_ADHOC ) ? "Adhoc" : "Unknown");
				W_RSP("/%s", ( record->security == WICED_SECURITY_OPEN ) ? "Open" : ( record->security == WICED_SECURITY_WEP_PSK ) ? "WEP" : ( record->security == WICED_SECURITY_WPA_TKIP_PSK ) ? "WPA" : ( record->security == WICED_SECURITY_WPA2_AES_PSK ) ? "WPA2" : ( record->security == WICED_SECURITY_WPA2_MIXED_PSK ) ? "WPA2-Mixed" : "Unknown");
				W_RSP("/%s", ( record->band == WICED_802_11_BAND_5GHZ ) ? "5" : "2.4");
				W_RSP("/%d\r\n", record->channel);
			}
			////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
		}
		data->result_buff_read_pos++;
		if ( data->result_buff_read_pos >= CIRCULAR_RESULT_BUFF_SIZE )
		{
			data->result_buff_read_pos = 0;
		}
#if 0 //MikeJ 130820 ID1125 - SCAN Option problem
		record_number++;
#endif
	}

	/* Done! */
#if 0 //MikeJ 130702 ID1093 - Adjust Response Format
	W_RSP("\r\nEnd of scan results\r\n");
#endif
	host_rtos_deinit_semaphore(&data->num_scan_results_semaphore);
	free(data);
	return ERR_CMD_OK;
}

void network_print_state( char* sta_ssid, char* ap_ssid )
{
#if 0 //MikeJ 130702 ID1093 - Adjust Response Format
	uint8_t interface;
	wiced_ip_address_t ip_address;

	for(interface = 0; interface <= 1; interface++)
	{
		if ( wiced_wifi_is_ready_to_transceive((wiced_interface_t) interface) == WICED_SUCCESS )
		{
			if ( interface == WICED_STA_INTERFACE )
			{
				W_RSP("STA Interface\r\n");
				W_RSP("   AP Name    : %s\r\n", sta_ssid);
			}
			else
			{
				W_RSP("AP Interface\r\n");
				W_RSP("   AP Name    : %s\r\n", ap_ssid);
			}

		    wiced_ip_get_ipv4_address( (wiced_interface_t) interface, &ip_address );
		    W_RSP("   IP Addr    : %u.%u.%u.%u\r\n",
		            (unsigned char) ( ( ( GET_IPV4_ADDRESS( ip_address ) ) >> 24 ) & 0xff ),
		            (unsigned char) ( ( ( GET_IPV4_ADDRESS( ip_address ) ) >> 16 ) & 0xff ),
		            (unsigned char) ( ( ( GET_IPV4_ADDRESS( ip_address ) ) >> 8 ) & 0xff ),
		            (unsigned char) ( ( ( GET_IPV4_ADDRESS( ip_address ) ) >> 0 ) & 0xff ));

		    wiced_ip_get_gateway_address( (wiced_interface_t) interface, &ip_address );
		    W_RSP("   Gateway    : %u.%u.%u.%u\r\n",
		            (unsigned char) ( ( ( GET_IPV4_ADDRESS( ip_address ) ) >> 24 ) & 0xff ),
		            (unsigned char) ( ( ( GET_IPV4_ADDRESS( ip_address ) ) >> 16 ) & 0xff ),
		            (unsigned char) ( ( ( GET_IPV4_ADDRESS( ip_address ) ) >> 8 ) & 0xff ),
		            (unsigned char) ( ( ( GET_IPV4_ADDRESS( ip_address ) ) >> 0 ) & 0xff ));
		}
		else
		{
			if ( interface == WICED_STA_INTERFACE )
				W_RSP("STA Interface : Down\r\n");
			else
				W_RSP("AP Interface  : Down\r\n");
		}
	}
#else
	uint8_t wif;
	wiced_ip_address_t addr;

	if(wiced_wifi_is_ready_to_transceive(WICED_STA_INTERFACE) == WICED_SUCCESS) {
		W_RSP("STA/%s", sta_ssid);
		wif = WICED_STA_INTERFACE;
	} else if(wiced_wifi_is_ready_to_transceive(WICED_AP_INTERFACE) == WICED_SUCCESS) {
		W_RSP("AP/%s", ap_ssid);
		wif = WICED_AP_INTERFACE;
	} else {
		W_RSP("Down///");
		return;
	}

	wiced_ip_get_ipv4_address( (wiced_interface_t) wif, &addr );
	W_RSP("/%u.%u.%u.%u", 
        (unsigned char) ( ( ( GET_IPV4_ADDRESS( addr ) ) >> 24 ) & 0xff ),
        (unsigned char) ( ( ( GET_IPV4_ADDRESS( addr ) ) >> 16 ) & 0xff ),
        (unsigned char) ( ( ( GET_IPV4_ADDRESS( addr ) ) >> 8 ) & 0xff ),
        (unsigned char) ( ( ( GET_IPV4_ADDRESS( addr ) ) >> 0 ) & 0xff ));
	wiced_ip_get_gateway_address( (wiced_interface_t) wif, &addr );
    W_RSP("/%u.%u.%u.%u",
        (unsigned char) ( ( ( GET_IPV4_ADDRESS( addr ) ) >> 24 ) & 0xff ),
        (unsigned char) ( ( ( GET_IPV4_ADDRESS( addr ) ) >> 16 ) & 0xff ),
        (unsigned char) ( ( ( GET_IPV4_ADDRESS( addr ) ) >> 8 ) & 0xff ),
        (unsigned char) ( ( ( GET_IPV4_ADDRESS( addr ) ) >> 0 ) & 0xff ));
#endif
}

////////////////////////////////////////////////////////////////////////////////
void check_psocketlist_and_process_basedon_scon1_option()
{
	uint32_t i = 0;
	uint8_t found_socket = 0;
	for(i = 0; i < WX_MAX_SCID_RANGE; i++)
	{
		if ( g_scList[i].pSocket!=0 )
		{
			found_socket = 1;
			break;
		}
	}
	if ( found_socket==1 )	return;

	// sekim Auto Connection (Service), Booting time => Link-up callback
	if ( strcmp((char*)g_wxProfile.scon_opt1, "SO") == 0 || strcmp((char*)g_wxProfile.scon_opt1, "S") == 0 )
	{
		// sekim 20150508 Disable WizFi250-Reset because of socket_ext_option6
		/*
		// sekim 20150417 if (service mode & disassociated), WizFi250 will be reset. by socket_ext_option6 option.
		if ( !WXLink_IsWiFiLinked() )
		{
			WXS2w_SystemReset();
			return;
		}
		*/

		char szBuff[128] = { 0, };
#if 0 //MikeJ 130806 ID1113 - S option is changed to SO unexpectedly when join is performed
		sprintf(szBuff, "SO,%s,%s,%d,%d,%d", g_wxProfile.scon_opt2, g_wxProfile.scon_remote_ip, g_wxProfile.scon_remote_port, g_wxProfile.scon_local_port, g_wxProfile.scon_datamode);
		if ( WXCmd_SCON((UINT8*)szBuff) == WXCODE_SUCCESS )
		{
		}
#else
		if( strcmp((char*)g_wxProfile.scon_opt1, "SO") == 0 )
		{
			sprintf(szBuff, "SO,%s,%s,%d,%d,%d", g_wxProfile.scon_opt2, g_wxProfile.scon_remote_ip, g_wxProfile.scon_remote_port, g_wxProfile.scon_local_port, g_wxProfile.scon_datamode);
			if ( WXCmd_SCON((UINT8*)szBuff) == WXCODE_SUCCESS )
			{
			}
		}
		else {
			sprintf(szBuff, "O,%s,%s,%d,%d,%d", g_wxProfile.scon_opt2, g_wxProfile.scon_remote_ip, g_wxProfile.scon_remote_port, g_wxProfile.scon_local_port, g_wxProfile.scon_datamode);

			////////////////////////////////////////////////////////////////////////////////////
			// sekim 20150401 AT+SCON=S,USN,........ Bug Fix
			if ( (toupper(g_wxProfile.scon_opt2[0])=='U') && (toupper(g_wxProfile.scon_opt2[1])=='S') )
			{
				sprintf(szBuff, "O,%s,,,%d,%d", g_wxProfile.scon_opt2, g_wxProfile.scon_local_port, g_wxProfile.scon_datamode);
			}
			////////////////////////////////////////////////////////////////////////////////////

			if ( WXCmd_SCON((UINT8*)szBuff) == WXCODE_SUCCESS )
			{
			}

#if 1 // kaizen 20131125 Fixed Bug which changed scon_opt1 value when user set to "S"
			strcpy((char*)g_wxProfile.scon_opt1,(char*)"S");
#else
			sprintf(szBuff, "S,%s,%s,%d,%d,%d", g_wxProfile.scon_opt2, g_wxProfile.scon_remote_ip, g_wxProfile.scon_remote_port, g_wxProfile.scon_local_port, g_wxProfile.scon_datamode);
			if ( WXCmd_SCON((UINT8*)szBuff) == WXCODE_SUCCESS )
			{
			}
#endif
		}
#endif
	}
}

void action_after_linkup_callback()
{
	check_psocketlist_and_process_basedon_scon1_option();

	#if 1
		if ( strcmp((char*)g_wxProfile.web_server_opt,"A") == 0 )
		{
			char szBuff[10] = { 0, };
			strcpy(szBuff, "1");
			WXCmd_FWEBS((UINT8*)szBuff);
		}
	#else	// kaizen 20130726 ID1105 Modified code in order to launch web server when link up event
		if( strcmp((char*)g_wxProfile.web_server_opt, "SO") == 0 )
		{
			char szBuff[10] = { 0, };
			sprintf(szBuff, "1,%s",g_wxProfile.web_server_opt);
			WXCmd_FWEBS((UINT8*)szBuff);
		}
	#endif
}
////////////////////////////////////////////////////////////////////////////////

// sekim 20140508 double check Link Down (for WiFi Direct)
uint8_t g_wifi_linkup = 0;

// sekim 20130224 Link UP/Down Callback
void wifi_link_up_callback(void)
{
	////////////////////////////////////////////////////////////////////////
	// sekim 20140427 sometimes, Wizfi250 fail to get ip-address
	uint32_t i = 0;
	for (i=0; i<150; i++)
	{
		uint8_t wif;
		wiced_ip_address_t addr;

		if ( wiced_wifi_is_ready_to_transceive(WICED_STA_INTERFACE)==WICED_SUCCESS )
		{
			wif = WICED_STA_INTERFACE;
		}
		else if ( wiced_wifi_is_ready_to_transceive(WICED_AP_INTERFACE)==WICED_SUCCESS )
		{
			wif = WICED_AP_INTERFACE;
		}
		else
		{
			W_DBG("wifi_link_up_callback : wait ip address error");
			break;
		}
		wiced_ip_get_ipv4_address((wiced_interface_t)wif, &addr );

		if (  !(((unsigned char) ( ( ( GET_IPV4_ADDRESS( addr ) ) >> 24 ) & 0xff ))==0 &&
				((unsigned char) ( ( ( GET_IPV4_ADDRESS( addr ) ) >> 16 ) & 0xff ))==0 &&
				((unsigned char) ( ( ( GET_IPV4_ADDRESS( addr ) ) >>  8 ) & 0xff ))==0 &&
				((unsigned char) ( ( ( GET_IPV4_ADDRESS( addr ) ) >>  0 ) & 0xff ))==0) )
		{
			break;
		}
		wiced_rtos_delay_milliseconds( 100 );
		W_DBG2(".");
	}
	////////////////////////////////////////////////////////////////////////

	WXS2w_StatusNotify(WXCODE_LINKUP, 0);
	WXS2w_LEDIndication(1, 0, 0, 0, 0, 1);

	send_maincommand_queue(101, 0);

	// sekim 20140508 double check Link Down (for WiFi Direct)
	g_wifi_linkup = 1;
}

void wifi_link_down_callback(void)
{
	// kaizen
	//WXLink_Disassociate(0, 1);

#if 1	// kaizen 20130726 ID1105 Modified code in order to stop web server when link down event
	if ( web_server_status == START_WEB_SERVER )
	{
		char szBuff[10] = { 0, };
		sprintf(szBuff, "0");
		WXCmd_FWEBS((UINT8*)szBuff);
	}
#endif

	WXLink_Disassociate(0, 1, WICED_STA_INTERFACE);

	// sekim 20140508 double check Link Down (for WiFi Direct)
	g_wifi_linkup = 0;

	//daniel 160630 Add for MQTT Resource Clear
	extern int		g_mqtt_isconnected;
	if( g_mqtt_isconnected == 1 )
	{
		extern wiced_result_t MQTTClearResource();
		MQTTClearResource();
	}
	//////////////

}

// kaizen
//UINT8 WXLink_Disassociate(UINT8 bForced, UINT8 bNotify)
UINT8 WXLink_Disassociate(UINT8 bForced, UINT8 bNotify, wiced_interface_t interface)
{
	// close all scids opened
	WXNetwork_CloseSCAllList();

	g_currentScid = WX_INVALID_SCID;

	// call the status notification function
	g_scTxIndex = 0;

	g_isAutoconnected = 0;
	g_wxModeState = WX_MODE_COMMAND;
	WXS2w_LEDIndication(2, 0, 0, 0, 0, 0);

	WXS2w_LEDIndication(1, 0, 0, 0, 0, 0);

	if ( bForced )
	{
		// kaizen
		//if ( wiced_network_down( WICED_STA_INTERFACE )!=WICED_SUCCESS )
		if ( wiced_network_down( interface )!=WICED_SUCCESS )
			W_RSP("WXLink_Disassociate : error");
	}
	if ( bNotify )
	{
		WXS2w_StatusNotify(WXCODE_LINKDOWN, 0);
	}

	return WXCODE_SUCCESS;
}

UINT8 WXLink_IsWiFiLinked()
{
	UINT8 interface;
    for ( interface = 0; interface <= 1; interface++ )
    {
        if ( wiced_wifi_is_ready_to_transceive( (wiced_interface_t) interface ) == WICED_SUCCESS )
        {
			return TRUE;
        }
    }

	return FALSE;
}

UINT8 WXCmd_WJOIN(UINT8 *ptr)
{
	INT32 ret;
	wiced_ip_setting_t device_init_ip_settings;


	if (wiced_wifi_is_ready_to_transceive(WICED_STA_INTERFACE) == WICED_SUCCESS)
	{
		W_RSP("Already Associated : Station Mode\r\n");
		return WXCODE_SUCCESS;
	}
	else if (wiced_wifi_is_ready_to_transceive(WICED_AP_INTERFACE) == WICED_SUCCESS)
	{
		W_RSP("Already Started : AP Mode\r\n");
		return WXCODE_SUCCESS;
	}

	// sekim 20140508 double check Link Down (for WiFi Direct)
	extern UINT8 g_used_wp2p;
	g_used_wp2p = 0;

	////////////////////////////////////////////////////////////////////////////////
	// sekim 20130129 WXCmd_WJOIN에서 wiced_network_up 사용
	/*
	if ( g_wxProfile.wifi_dhcp==1 )
	{
#if 1 //MikeJ 130408 ID1014 - Key buffer overflow
		ret = wifi_join_ext(g_wxProfile.wifi_ssid, g_wxProfile.wifi_bssid, g_wxProfile.wifi_channel, g_wxProfile.wifi_authtype, g_wxProfile.wifi_keydata, g_wxProfile.wifi_keylen, NULL, NULL, NULL);
#else
		ret = wifi_join_ext(g_wxProfile.wifi_ssid, g_wxProfile.wifi_bssid, g_wxProfile.wifi_channel, g_wxProfile.wifi_authtype, g_wxProfile.wifi_keydata, strlen(g_wxProfile.wifi_keydata), NULL, NULL, NULL);
#endif
	}
	else
	{
#if 1 //MikeJ 130408 ID1014 - Key buffer overflow
		ret = wifi_join_ext(g_wxProfile.wifi_ssid, g_wxProfile.wifi_bssid, g_wxProfile.wifi_channel, g_wxProfile.wifi_authtype, g_wxProfile.wifi_keydata, g_wxProfile.wifi_keylen, g_wxProfile.wifi_ip, g_wxProfile.wifi_mask, g_wxProfile.wifi_gateway);
#else
		ret = wifi_join_ext(g_wxProfile.wifi_ssid, g_wxProfile.wifi_bssid, g_wxProfile.wifi_channel, g_wxProfile.wifi_authtype, g_wxProfile.wifi_keydata, strlen(g_wxProfile.wifi_keydata), g_wxProfile.wifi_ip, g_wxProfile.wifi_mask, g_wxProfile.wifi_gateway);
#endif
	}
	*/
	if ( g_wxProfile.wifi_mode == 0 ) // Station Mode
	{
		////////////////////////////////////////////////////////////////////////
		// sekim 20131122 ID1142 Add WiFi Auto-Security-Type Feature
		if ( g_wxProfile.wifi_authtype==WICED_SECURITY_UNKNOWN )
		{
			char szBuff_command[256] = {0,};
			char szBuff_sectype[50] = {0,};

			W_DBG("scan_wifi_and_get_sectype start");
			wiced_ssid_t buff_ssid;
			memcpy(buff_ssid.val, g_wxProfile.wifi_ssid, strlen((char*)g_wxProfile.wifi_ssid));
			buff_ssid.len = strlen((char*)g_wxProfile.wifi_ssid);

			wiced_security_t buff_security = scan_wifi_and_get_sectype(&buff_ssid);
			if ( buff_security==WICED_SECURITY_UNKNOWN )
			{
				W_DBG("scan_wifi_and_get_sectype error");
				return WXCODE_FAILURE;
			}

			// sekim 20131203 wifi_keydata & wifi_keylen
			{
				UINT8 buff_keydata[65];
				memset(buff_keydata, 0, sizeof(buff_keydata));
				memcpy(buff_keydata, g_wxProfile.wifi_keydata, g_wxProfile.wifi_keylen);
				sprintf(szBuff_command, "0,%s,%s", authtype_to_str(szBuff_sectype, buff_security), buff_keydata);
				if ( WXCmd_WSEC((UINT8*)szBuff_command)!= WXCODE_SUCCESS )
				{
					W_DBG("WXCmd_WSEC error");
					return WXCODE_FAILURE;
				}
			}
		}
		////////////////////////////////////////////////////////////////////////


		if ( g_wxProfile.wifi_dhcp==1 )
		{
			ret = wiced_network_up(WICED_STA_INTERFACE, WICED_USE_EXTERNAL_DHCP_SERVER, NULL);
		}
		else
		{
			SET_IPV4_ADDRESS(device_init_ip_settings.ip_address, str_to_ip((char*)g_wxProfile.wifi_ip));
			SET_IPV4_ADDRESS(device_init_ip_settings.netmask, str_to_ip((char*)g_wxProfile.wifi_mask));
			SET_IPV4_ADDRESS(device_init_ip_settings.gateway, str_to_ip((char*)g_wxProfile.wifi_gateway));

			ret = wiced_network_up(WICED_STA_INTERFACE, WICED_USE_STATIC_IP, &device_init_ip_settings);

// sekim 20130806 ID1120 Add DNS Server if using Static IP
#if 1
		    {
		    	wiced_ip_address_t 	dns_ip;

		    	dns_client_remove_all_server_addresses();

		    	SET_IPV4_ADDRESS(dns_ip, str_to_ip((char*)g_wxProfile.wifi_dns1) );
		    	dns_client_add_server_address(dns_ip);
		    	SET_IPV4_ADDRESS(dns_ip, str_to_ip((char*)g_wxProfile.wifi_dns2) );
		    	dns_client_add_server_address(dns_ip);
		    }
#endif
		}

		////////////////////////////////////////////////////////////////////////
		// sekim 20140731 ID1183 WEP Shared Problem by ZionTek
		if ( ret!=WICED_SUCCESS && g_wxProfile.wifi_authtype==WICED_SECURITY_WEP_PSK )
		{
			platform_dct_wifi_config_t	wifi_config;
			wiced_dct_read_wifi_config_section( &wifi_config );
			wifi_config.stored_ap_list[0].details.security = WICED_SECURITY_WEP_SHARED;
            wiced_dct_write_wifi_config_section((const platform_dct_wifi_config_t*)&wifi_config);

            W_DBG("WEP with open authentication failed, trying WEP with shared authentication...");

    		if ( g_wxProfile.wifi_dhcp==1 )
    		{
    			ret = wiced_network_up(WICED_STA_INTERFACE, WICED_USE_EXTERNAL_DHCP_SERVER, NULL);
    		}
    		else
    		{
    			SET_IPV4_ADDRESS(device_init_ip_settings.ip_address, str_to_ip((char*)g_wxProfile.wifi_ip));
    			SET_IPV4_ADDRESS(device_init_ip_settings.netmask, str_to_ip((char*)g_wxProfile.wifi_mask));
    			SET_IPV4_ADDRESS(device_init_ip_settings.gateway, str_to_ip((char*)g_wxProfile.wifi_gateway));

    			ret = wiced_network_up(WICED_STA_INTERFACE, WICED_USE_STATIC_IP, &device_init_ip_settings);
    		    {
    		    	wiced_ip_address_t 	dns_ip;
    		    	dns_client_remove_all_server_addresses();

    		    	SET_IPV4_ADDRESS(dns_ip, str_to_ip((char*)g_wxProfile.wifi_dns1) );
    		    	dns_client_add_server_address(dns_ip);
    		    	SET_IPV4_ADDRESS(dns_ip, str_to_ip((char*)g_wxProfile.wifi_dns2) );
    		    	dns_client_add_server_address(dns_ip);
    		    }
    		}

    		if ( ret!=WICED_SUCCESS )
    		{
                wiced_dct_read_wifi_config_section(&wifi_config);
                wifi_config.stored_ap_list[0].details.security = WICED_SECURITY_WEP_PSK;
                wiced_dct_write_wifi_config_section((const platform_dct_wifi_config_t*)&wifi_config);
    		}
		}
		////////////////////////////////////////////////////////////////////////



		if ( ret!=ERR_CMD_OK )
		{
			return WXCODE_FAILURE;
		}
	}
	else if ( g_wxProfile.wifi_mode == 1 ) 		// AP Mode
	{
		SET_IPV4_ADDRESS(device_init_ip_settings.ip_address, str_to_ip((char*)g_wxProfile.wifi_ip));
		SET_IPV4_ADDRESS(device_init_ip_settings.netmask, str_to_ip((char*)g_wxProfile.wifi_mask));
		SET_IPV4_ADDRESS(device_init_ip_settings.gateway, str_to_ip((char*)g_wxProfile.wifi_gateway));

		/* Start the access point */
		ret = wiced_network_up(WICED_AP_INTERFACE, WICED_USE_INTERNAL_DHCP_SERVER, &device_init_ip_settings);
		W_DBG("wiced_network_up result = %d",ret);
	}


	return WXCODE_SUCCESS;
}

#if 1	// kaizen 20130731 ID1108 For cleanning WXCmd_MWIFIPS() status variable at WLEAVE()
extern UINT32 wifi_ps_current_status;
extern UINT32 wifi_ps_delay;
#endif

UINT8 WXCmd_WLEAVE(UINT8 *ptr)
{

#if 1	// kaizen 20130513 For Web Server Service Option
	if ( web_server_status == START_WEB_SERVER )
	{
		if (WXCmd_FWEBS((UINT8*)"0") != WXCODE_SUCCESS)
			return WXCODE_FAILURE;

		wiced_rtos_delay_milliseconds( 1 * SECONDS );				// kaizen 20130513 For waiting end of web server before WXLink_Disassociate.
	}
#else
	if( g_wxProfile.web_server_status == WEB_SERVER_IS_RUN )
	{
		W_DBG("Web Server is running. please stop web server");
		return WXCODE_FAILURE;
	}
#endif


	//daniel 160630 Add MQTT disconnection
	extern int		g_mqtt_isconnected;
	W_DBG("MQTT Thread is run.. : %d", g_mqtt_isconnected);
	if( g_mqtt_isconnected == 1 )
	{
		extern wiced_result_t MQTTStopService();
		MQTTStopService();
	}
	/////////////

	if (wiced_wifi_is_ready_to_transceive(WICED_STA_INTERFACE) == WICED_SUCCESS)
	{
		W_DBG("Disassociation : Station Mode");
		WXLink_Disassociate(1, 1, WICED_STA_INTERFACE);
	}
	else if (wiced_wifi_is_ready_to_transceive(WICED_AP_INTERFACE) == WICED_SUCCESS)
	{
		W_DBG("Stop AP : AP Mode");
		WXLink_Disassociate(1, 1, WICED_AP_INTERFACE);
	}
	else if (wiced_wifi_is_ready_to_transceive(WICED_CONFIG_INTERFACE) == WICED_SUCCESS)
	{
		W_DBG("Stop AP : Config AP Mode");
		WXLink_Disassociate(1, 1, WICED_CONFIG_INTERFACE);
	}
	else
	{
		W_DBG("Nothing is run");
	}

#if 1	// kaizen 20130731 ID1108 For cleanning WXCmd_MWIFIPS() status variable at WLEAVE()
	wifi_ps_current_status = 0;
	wifi_ps_delay = 0;
#endif

	return WXCODE_SUCCESS;
}

UINT8 WXCmd_WSCAN(UINT8 *ptr)
{
	UINT8 *p;
	UINT8 status;
	UINT32 buff_cnannel = 0;

	wiced_ssid_t* optional_ssid = 0;
	wiced_mac_t* optional_mac = 0;
#if 0 //MikeJ 130820 ID1125 - SCAN Option problem
	uint16_t optional_channel_list[1] = {0};
#else
	uint16_t optional_channel_list[2] = {0};
#endif
	wiced_ssid_t buff_ssid;
	wiced_mac_t buff_mac;

	// sekim 20131024 ID1131 AP Scan Time Control
	UINT32 buff_scan_time = 70; // default

	p = WXParse_NextParamGet(&ptr);
#if 0 //MikeJ 130702 ID1092 - ATCmd update (naming, adding comments)
	if (p)
	{
#else
	if (p && *(p+1)!=0 && *(p+1)!='?')
	{
		if(*p == '=') p++;	
		else return WXCODE_EINVAL;
#endif

		if ( strcmp((char*)p, "?")==0 )
		{
			// sekim 20131024 ID1131 AP Scan Time Control
			//if ( ERR_CMD_OK!=scanwifi2(optional_ssid, optional_mac, optional_channel_list) )
			if ( ERR_CMD_OK!=scanwifi2(optional_ssid, optional_mac, optional_channel_list, buff_scan_time) )
				return WXCODE_FAILURE;
			return WXCODE_SUCCESS;
		}

		if ( strlen((char*)p)>0 )
		{
#if 1 //MikeJ 130408 ID1012 - Allow 32 characters SSID
			if ( strlen((char*)p) > WX_MAX_SSID_LEN )	return WXCODE_EINVAL;
#else
			if ( strlen((char*)p)>=WX_MAX_SSID_LEN )	return WXCODE_EINVAL;
#endif
			memcpy(buff_ssid.val, p, strlen((char*)p));
			buff_ssid.len = strlen((char*)p);
			optional_ssid = &buff_ssid;
		}
		// sekim 20130329 AP Scan Bug Fix(issued by Mike)
		/*
		else
		{
			return WXCODE_EINVAL;
		}
		*/
	}

	p = WXParse_NextParamGet(&ptr);
	if (p)
	{
		if ( strlen((char*)p)>0 )
		{
			status = WXParse_Mac(p, (UINT8*)&(buff_mac.octet));
			if ( status != WXCODE_SUCCESS )  return WXCODE_EINVAL;
			optional_mac = &buff_mac;
		}
	}

	p = WXParse_NextParamGet(&ptr);
	if (p)
	{
		status = WXParse_Int(p, &buff_cnannel);
		if ( buff_cnannel<1 || buff_cnannel>14 )	return WXCODE_EINVAL;
		optional_channel_list[0] = buff_cnannel;
		if (status != WXCODE_SUCCESS )	return WXCODE_EINVAL;
	}

	// sekim 20131024 ID1131 AP Scan Time Control
	p = WXParse_NextParamGet(&ptr);
	if (p)
	{
		status = WXParse_Int(p, &buff_scan_time);
		if ( buff_scan_time<50 || buff_scan_time>1000 )	return WXCODE_EINVAL;
		if (status != WXCODE_SUCCESS )	return WXCODE_EINVAL;
	}

	// sekim 20131024 ID1131 AP Scan Time Control
	//if ( ERR_CMD_OK!=scanwifi2(optional_ssid, optional_mac, optional_channel_list) )
	if ( ERR_CMD_OK!=scanwifi2(optional_ssid, optional_mac, optional_channel_list, buff_scan_time) )
		return WXCODE_FAILURE;

	return WXCODE_SUCCESS;
}

extern wiced_result_t wiced_dct_read_wifi_config_section( platform_dct_wifi_config_t* wifi_config_dct );

// SSID, BSSID, Channel, Mode(Station Mode/AP Mode/Adhoc)
UINT8 WXCmd_WSET(UINT8 *ptr)
{
	UINT8 *p;
	UINT8 status;

#if 1 //MikeJ 130408 ID1012 - Allow 32 characters SSID
	UINT8 wifi_buff_ssid[WX_MAX_SSID_LEN+1];
#else
	UINT8 wifi_buff_ssid[WX_MAX_SSID_LEN];
#endif
	UINT8 wifi_buff_bssid[18];
	UINT32 wifi_buff_channel;
	UINT32 wifi_buff_mode;
#if 0 //MikeJ 130806 ID1116 - Couldn't display 32 characters SSID
	memset(wifi_buff_ssid, 0, WX_MAX_SSID_LEN);
#else
	memset(wifi_buff_ssid, 0, WX_MAX_SSID_LEN+1);
#endif
	memset(wifi_buff_bssid, 0, 18);
	wifi_buff_channel = 0;
	wifi_buff_mode = 0;
	wiced_mac_t		dummy_bssid;
	wiced_bool_t	b_already_set=WICED_FALSE;

	platform_dct_wifi_config_t wifi_config;
	wiced_config_ap_entry_t * infra_entry;
	wiced_config_soft_ap_t  * ap_entry;

	wiced_dct_read_wifi_config_section(&wifi_config);
	infra_entry = &wifi_config.stored_ap_list[0];
	ap_entry 	= &wifi_config.soft_ap_settings;

#if 1	// kaizen 130412 ID1037 Do not save other information to DCT
	WT_PROFILE temp_wxProfile;
	wiced_dct_read_app_section( &temp_wxProfile, sizeof(WT_PROFILE) );
#endif

	if ( strcmp((char*)ptr, "?")==0 )
	{
		sprintf((char*)wifi_buff_bssid,"%02x:%02x:%02x:%02x:%02x:%02x",infra_entry->details.BSSID.octet[0], infra_entry->details.BSSID.octet[1],infra_entry->details.BSSID.octet[2],infra_entry->details.BSSID.octet[3],infra_entry->details.BSSID.octet[4],infra_entry->details.BSSID.octet[5]);

#if 1 //MikeJ 130408 ID1012 - Allow 32 characters SSID + //MikeJ 130409 ID1015 - WiFi Mode Parameter order change (last -> first)
		if( g_wxProfile.wifi_mode == STATION_MODE ) {			// Station Mode
			memcpy(wifi_buff_ssid, infra_entry->details.SSID.val, infra_entry->details.SSID.len);	// len never over 32
			wifi_buff_ssid[infra_entry->details.SSID.len] = 0;
			W_RSP("%d,%s,%s,%d\r\n", g_wxProfile.wifi_mode, wifi_buff_ssid, wifi_buff_bssid, infra_entry->details.channel);
		} else if ( g_wxProfile.wifi_mode == AP_MODE ) {		// AP Mode
			memcpy(wifi_buff_ssid, ap_entry->SSID.val, ap_entry->SSID.len);
			wifi_buff_ssid[ap_entry->SSID.len] = 0;
			W_RSP("%d,%s,%s,%d\r\n", g_wxProfile.wifi_mode, wifi_buff_ssid, wifi_buff_bssid, ap_entry->channel);
		} else return WXCODE_EINVAL;
#else
		if( g_wxProfile.wifi_mode == 0 )			// Station Mode
			W_RSP("%s,%s,%d,%d\r\n", infra_entry->details.SSID.val, wifi_buff_bssid, infra_entry->details.channel,g_wxProfile.wifi_mode);
		else if ( g_wxProfile.wifi_mode == 1 )		// AP Mode
			W_RSP("%s,%s,%d,%d\r\n", ap_entry->SSID.val, wifi_buff_bssid, ap_entry->channel,g_wxProfile.wifi_mode);
		else
			return WXCODE_EINVAL;
#endif

		return WXCODE_SUCCESS;
	}

#if 1 //MikeJ 130409 ID1015 - WiFi Mode Parameter order change (last -> first)
	p = WXParse_NextParamGet(&ptr);
	if (p)
	{
		status = WXParse_Int(p, &wifi_buff_mode);
		if ( wifi_buff_mode!=STATION_MODE && wifi_buff_mode!=AP_MODE )	return WXCODE_EINVAL;
		if (status != WXCODE_SUCCESS )	return WXCODE_EINVAL;
	}
#endif

	p = WXParse_NextParamGet(&ptr);
	if (!p)		return WXCODE_EINVAL;
#if 1 //MikeJ 130408 ID1012 - Allow 32 characters SSID
	if ( strlen((char*)p)==0 || strlen((char*)p) > WX_MAX_SSID_LEN ) return WXCODE_EINVAL;
	strcpy((char*)wifi_buff_ssid, (char*)p);
#else
	if ( strlen((char*)p)==0 || strlen((char*)p)>=WX_MAX_SSID_LEN )	return WXCODE_EINVAL;
	strcpy((char*)wifi_buff_ssid, (char*)p);
#endif

	p = WXParse_NextParamGet(&ptr);
	if (p)
	{
#if 1 //MikeJ 130408 ID1013 - Check WSET Validation
		wiced_mac_t buff_mac;
		status = WXParse_Mac(p, (UINT8*)&(buff_mac.octet));
		if ( status != WXCODE_SUCCESS )  return WXCODE_EINVAL;
		mac_to_str((char*)wifi_buff_bssid, &buff_mac);
#else
		if ( strlen((char*)p)>=18 )	return WXCODE_EINVAL;
		strcpy((char*)wifi_buff_bssid, (char*)p);
#endif
	}

	p = WXParse_NextParamGet(&ptr);
#if 1 //MikeJ 130408 ID1013 - Check WSET Validation
	if (p)
	{
		status = WXParse_Int(p, &wifi_buff_channel);
		if (status != WXCODE_SUCCESS )	return WXCODE_EINVAL;
		if (wifi_buff_channel == 0 || wifi_buff_channel > 14) return WXCODE_EINVAL;
	}
	else
	{
		if(wifi_buff_mode == AP_MODE) wifi_buff_channel = 6;
		//else wifi_buff_channel = 0;	// 0 Channel means all ???
	}
#else
	if (p)
	{
		status = WXParse_Int(p, &wifi_buff_channel);
		if (status != WXCODE_SUCCESS )	return WXCODE_EINVAL;
	}
#endif

#if 0 //MikeJ 130409 ID1015 - WiFi Mode Parameter order change (last -> first)
	p = WXParse_NextParamGet(&ptr);
	if (p)
	{
		status = WXParse_Int(p, &wifi_buff_mode);
		if ( wifi_buff_mode!=0 && wifi_buff_mode!=1 )	return WXCODE_EINVAL;
		if (status != WXCODE_SUCCESS )	return WXCODE_EINVAL;
	}
#endif

	if(wifi_buff_mode == STATION_MODE)		// kaizen 20130412
	{
		// kaizen 20130403 이미 설정되어 있는 값을 똑같이 설정하려고 할 때는 Flash Memory에 저장하지 않도록 수정
		dummy_bssid = str_to_mac((char*)wifi_buff_bssid);
		if( strcmp((char*)infra_entry->details.SSID.val,(char*)wifi_buff_ssid)==0 && infra_entry->details.channel == (UINT8)wifi_buff_channel
				&& memcmp(infra_entry->details.BSSID.octet,dummy_bssid.octet,sizeof(6))==0 && temp_wxProfile.wifi_mode == wifi_buff_mode )
		{
			b_already_set = WICED_TRUE;
		}
		else
		{
			infra_entry->details.SSID.len = strlen((char*)wifi_buff_ssid);
			strcpy( (char*)infra_entry->details.SSID.val, (char*)wifi_buff_ssid );
			strcpy( (char*)g_wxProfile.wifi_ssid, (char*)wifi_buff_ssid );

			infra_entry->details.BSSID = str_to_mac((char*)wifi_buff_bssid);
			strcpy( (char*)g_wxProfile.wifi_bssid, (char*)wifi_buff_bssid );

			infra_entry->details.channel 	= (UINT8)wifi_buff_channel;
			g_wxProfile.wifi_channel  		= (UINT8)wifi_buff_channel;
		}
	}
	else if(wifi_buff_mode == AP_MODE)		// kaizen 20130412
	{
		// kaizen 20130403 이미 설정되어 있는 값을 똑같이 설정하려고 할 때는 Flash Memory에 저장하지 않도록 수정
		if( strcmp((char*)ap_entry->SSID.val,(char*)wifi_buff_ssid)==0 && ap_entry->channel == (UINT8)wifi_buff_channel && temp_wxProfile.wifi_mode == wifi_buff_mode )
			b_already_set = WICED_TRUE;

		else
		{
			ap_entry->SSID.len = strlen((char*)wifi_buff_ssid);
			strcpy( (char*)ap_entry->SSID.val, (char*)wifi_buff_ssid );				// AP Mode를 위한 Wifi_config_dct에 SSID 반영
			strcpy( (char*)g_wxProfile.ap_mode_ssid, (char*)wifi_buff_ssid);

			ap_entry->channel 			= (UINT8)wifi_buff_channel;					// AP Mode를 위한 Wifi_config_dct에 SSID 반영
			g_wxProfile.ap_mode_channel	= (UINT8)wifi_buff_channel;
		}
	}

	g_wxProfile.wifi_mode = (UINT32)wifi_buff_mode;

	// kaizen 20130403		Flash에 저장되어 있는 값과 설정하려는 파라미터와 같으면 Flash에 저장하지 않음(Flash의 Access 횟수를 최대한 줄이기 위해)
	if(!b_already_set)
	{
#if 1	// kaizen 130412 ID1037 Do not save other information to DCT
		if(g_wxProfile.wifi_mode == STATION_MODE )
		{
			strcpy( (char*)temp_wxProfile.wifi_ssid, (char*)g_wxProfile.wifi_ssid );
			strcpy( (char*)temp_wxProfile.wifi_bssid, (char*)g_wxProfile.wifi_bssid );
			temp_wxProfile.wifi_channel = g_wxProfile.wifi_channel;
		}
		else if(g_wxProfile.wifi_mode == AP_MODE)
		{
			strcpy( (char*)temp_wxProfile.ap_mode_ssid, (char*)g_wxProfile.ap_mode_ssid );
			temp_wxProfile.ap_mode_channel = g_wxProfile.ap_mode_channel;
		}
		temp_wxProfile.wifi_mode = g_wxProfile.wifi_mode;

		if ( WICED_SUCCESS!=wiced_dct_write_wifi_config_section(&wifi_config) )					W_DBG("wiced_dct_write_wifi_config_section error ");
		if ( WICED_SUCCESS!=wiced_dct_write_app_section( &temp_wxProfile, sizeof(WT_PROFILE) ))	W_DBG("wiced_dct_write_app_section error ");
#else
		// sekim 20133081 USART_Cmd(USART1, DISABLE) in wiced_dct_write_xxxx
		USART_Cmd(USART1, DISABLE);
		if ( WICED_SUCCESS!=wiced_dct_write_wifi_config_section(&wifi_config) )					W_DBG("wiced_dct_write_wifi_config_section error ");
		if ( WICED_SUCCESS!=wiced_dct_write_app_section( &g_wxProfile, sizeof(WT_PROFILE) )	)	W_DBG("wiced_dct_write_app_section error ");
		USART_Cmd(USART1, ENABLE);
#endif
	}

	return WXCODE_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////////////////
// sekim 20151123 Binary SSID for Coway
UINT8 WXCmd_WSET_TEMPforCoway(UINT8 *ptr)
{
	if ( ptr )
	{
		if ( ptr[0]=='0' )
		{
			return WXCmd_WSET2(ptr);
		}
	}

	return WXCmd_WSET(ptr);
}

UINT8 WXCmd_WSET2(UINT8 *ptr)
{
	UINT8 *p;
	UINT8 status;

	UINT8 wifi_buff_ssid[WX_MAX_SSID_LEN+1];
	UINT8 wifi_buff_bssid[18];
	UINT32 wifi_buff_channel;
	UINT32 wifi_buff_mode;

	memset(wifi_buff_ssid, 0, WX_MAX_SSID_LEN+1);
	memset(wifi_buff_bssid, 0, 18);
	wifi_buff_channel = 0;
	wifi_buff_mode = 0;
	wiced_mac_t		dummy_bssid;
	wiced_bool_t	b_already_set=WICED_FALSE;

	platform_dct_wifi_config_t wifi_config;
	wiced_config_ap_entry_t * infra_entry;

	wiced_dct_read_wifi_config_section(&wifi_config);
	infra_entry = &wifi_config.stored_ap_list[0];

	WT_PROFILE temp_wxProfile;
	wiced_dct_read_app_section( &temp_wxProfile, sizeof(WT_PROFILE) );

	if ( strcmp((char*)ptr, "?")==0 )
	{
		if( g_wxProfile.wifi_mode == STATION_MODE )
		{
			memcpy(wifi_buff_ssid, infra_entry->details.SSID.val, infra_entry->details.SSID.len);	// len never over 32
			wifi_buff_ssid[infra_entry->details.SSID.len] = 0;
			W_RSP("%d,%s,%s,%d\r\n", g_wxProfile.wifi_mode, wifi_buff_ssid, wifi_buff_bssid, infra_entry->details.channel);
		}
		return WXCODE_SUCCESS;
	}

	p = WXParse_NextParamGet(&ptr);
	if (p)
	{
		status = WXParse_Int(p, &wifi_buff_mode);
		if ( wifi_buff_mode!=STATION_MODE && wifi_buff_mode!=AP_MODE )	return WXCODE_EINVAL;
		if (status != WXCODE_SUCCESS )	return WXCODE_EINVAL;
	}

	// sekim 20151123 Binary SSID for Coway
	p = ptr;
	if (!p)		return WXCODE_EINVAL;
	if ( strlen((char*)p)==0 || strlen((char*)p) > WX_MAX_SSID_LEN ) return WXCODE_EINVAL;
	strcpy((char*)wifi_buff_ssid, (char*)p);

	if(wifi_buff_mode == STATION_MODE)
	{
		dummy_bssid = str_to_mac((char*)wifi_buff_bssid);
		if( strcmp((char*)infra_entry->details.SSID.val,(char*)wifi_buff_ssid)==0 && infra_entry->details.channel == (UINT8)wifi_buff_channel
				&& memcmp(infra_entry->details.BSSID.octet,dummy_bssid.octet,sizeof(6))==0 && temp_wxProfile.wifi_mode == wifi_buff_mode )
		{
			b_already_set = WICED_TRUE;
		}
		else
		{
			infra_entry->details.SSID.len = strlen((char*)wifi_buff_ssid);
			strcpy( (char*)infra_entry->details.SSID.val, (char*)wifi_buff_ssid );
			strcpy( (char*)g_wxProfile.wifi_ssid, (char*)wifi_buff_ssid );

			infra_entry->details.BSSID = str_to_mac((char*)wifi_buff_bssid);
			strcpy( (char*)g_wxProfile.wifi_bssid, (char*)wifi_buff_bssid );

			infra_entry->details.channel 	= (UINT8)wifi_buff_channel;
			g_wxProfile.wifi_channel  		= (UINT8)wifi_buff_channel;
		}
	}

	g_wxProfile.wifi_mode = (UINT32)wifi_buff_mode;

	if(!b_already_set)
	{
		if(g_wxProfile.wifi_mode == STATION_MODE )
		{
			strcpy( (char*)temp_wxProfile.wifi_ssid, (char*)g_wxProfile.wifi_ssid );
			strcpy( (char*)temp_wxProfile.wifi_bssid, (char*)g_wxProfile.wifi_bssid );
			temp_wxProfile.wifi_channel = g_wxProfile.wifi_channel;
		}
		temp_wxProfile.wifi_mode = g_wxProfile.wifi_mode;

		if ( WICED_SUCCESS!=wiced_dct_write_wifi_config_section(&wifi_config) )					W_DBG("wiced_dct_write_wifi_config_section error ");
		if ( WICED_SUCCESS!=wiced_dct_write_app_section( &temp_wxProfile, sizeof(WT_PROFILE) ))	W_DBG("wiced_dct_write_app_section error ");
	}

	return WXCODE_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////////////////////


UINT8 WXCmd_WSEC(UINT8 *ptr)
{
	UINT8 	*p;
#if 1 //MikeJ 130408 ID1014 - Key buffer overflow
	UINT8	wifi_buff_keydata[WSEC_MAX_PSK_LEN];
	UINT8	wifi_buff_keylen;
#else
	UINT8 	status;
	UINT8 	wifi_buff_keydata[128];
#endif
	UINT8	authtype_str[10];
	UINT32 	wifi_buff_authtype;
	UINT32	wifi_mode;
	wiced_bool_t	b_already_set=WICED_FALSE;

	platform_dct_wifi_config_t 	wifi_config;
	wiced_config_ap_entry_t * infra_entry;
	wiced_config_soft_ap_t  * ap_entry;

	wiced_dct_read_wifi_config_section(&wifi_config);
	infra_entry = &wifi_config.stored_ap_list[0];
	ap_entry 	= &wifi_config.soft_ap_settings;

	memset(wifi_buff_keydata, 0, sizeof(wifi_buff_keydata));

#if 1	// kaizen 130412 ID1037 Do not save other information to DCT
	WT_PROFILE temp_wxProfile;
	wiced_dct_read_app_section( &temp_wxProfile, sizeof(WT_PROFILE) );
#endif

	if ( strcmp((char*)ptr, "?")==0 )
	{
#if 1 //MikeJ 130408 ID1014 - Key buffer overflow + //MikeJ 130409 ID1015 - WiFi Mode Parameter order change (last -> first)
		BOOL isHex = FALSE;
		UINT8 *seckey;
		UINT8 seclen, i;
		wiced_security_t sectype;

		//----------------- Key Assign by Mode --------------------
		if( g_wxProfile.wifi_mode == STATION_MODE )
		{											// Station Mode
			sectype = infra_entry->details.security;
			seckey = (UINT8*)infra_entry->security_key;
			seclen = infra_entry->security_key_length;
		}
		else if( g_wxProfile.wifi_mode == AP_MODE )
		{											// AP Mode
			sectype = ap_entry->security;
			seckey = (UINT8*)ap_entry->security_key;
			seclen = ap_entry->security_key_length;
		}
		else return WXCODE_EINVAL;

		//----------------- Check if it is ASCII or HEX --------------------
		authtype_to_str((char*)authtype_str, sectype);
		// sekim 20131122 ID1142 Add WiFi Auto-Security-Type Feature
		//if ( authtype_str[0]==0 ) return WXCODE_FAILURE;
		W_RSP("%d,%s,", g_wxProfile.wifi_mode, authtype_str );
		
		if(sectype == WICED_SECURITY_WEP_PSK)
		{															// WEP Security
#if 0 //MikeJ 130806 ID1117 - WEP Security problem
			for(i=0; i<seclen; i++)
			{	// Check if key can be able to print
				if(!isprint(seckey[i])) break;
			}
			if(i != seclen) isHex = TRUE;	// HEX case
#else
			for(i=0; i<seckey[1]; i++)
			{	// Check if key can be able to print
				if(!isprint(seckey[i+2])) break;
			}
			if(i != seckey[1]) isHex = TRUE;	// HEX case
#endif
		}
		else if(sectype & (WPA_SECURITY | WPA2_SECURITY))
		{															// WPA Security
			if(seclen == WSEC_MAX_PSK_LEN) isHex = TRUE; // HEX case
		}
		else
		{															// OPEN Security
			W_RSP("0\r\n");
			return WXCODE_SUCCESS;
		}
		//----------------- Print response --------------------

		if(isHex == FALSE)											// Print Key value as ASCII
		{
#if 0 //MikeJ 130806 ID1117 - WEP Security problem
			seckey[seclen] = '\0';	// Not necessary but Just in case
			W_RSP( "%s\r\n", seckey );
#else
			if(sectype == WICED_SECURITY_WEP_PSK) {
				seckey[2+seckey[1]] = '\0';	// Not necessary but Just in case
				W_RSP( "%s\r\n", seckey+2 );
			} else {
				seckey[seclen] = '\0';	// Not necessary but Just in case
				W_RSP( "%s\r\n", seckey );
			}
#endif
		}
		else														// Print Key value as HEX
		{
#if 0 //MikeJ 130806 ID1117 - WEP Security problem
			for(i=0; i<seclen; i++) 
				W_RSP( "%02x ", seckey[i] );
#else
			if(sectype == WICED_SECURITY_WEP_PSK) {
				for(i=0; i<seckey[1]; i++) W_RSP( "%02x ", seckey[i+2] );
			} else {
				for(i=0; i<seclen; i++) W_RSP( "%02x ", seckey[i] );
			}
#endif
			W_RSP( "\r\n" );
		}
#else
		if( g_wxProfile.wifi_mode == 0 )		// Station Mode
		{
			authtype_to_str((char*)authtype_str, infra_entry->details.security);
			if ( authtype_str[0]==0 )
			{
				return WXCODE_FAILURE;
			}

			W_RSP("%s,%s,%d\r\n", authtype_str, infra_entry->security_key, g_wxProfile.wifi_mode );		// kaizen 20130312
		}
		else if( g_wxProfile.wifi_mode == 1 )	// AP Mode
		{
			authtype_to_str((char*)authtype_str, ap_entry->security);
			if ( authtype_str[0]==0 )
			{
				return WXCODE_FAILURE;
			}

			W_RSP("%s,%s,%d\r\n", authtype_str, ap_entry->security_key, g_wxProfile.wifi_mode );		// kaizen 20130312
		}
		else
			return WXCODE_EINVAL;
#endif

		return WXCODE_SUCCESS;
	}

#if 1 //MikeJ 130408 ID1014 - Key buffer overflow + //MikeJ 130409 ID1015 - WiFi Mode Parameter order change (last -> first)
	p = WXParse_NextParamGet(&ptr);	// WiFi Mode - Mandatory
	if ( !p ) return WXCODE_EINVAL;
	if ( WXParse_Int(p, &wifi_mode) != WXCODE_SUCCESS ) return WXCODE_EINVAL;
	if ( wifi_mode!=STATION_MODE && wifi_mode!=AP_MODE ) return WXCODE_EINVAL;

	p = WXParse_NextParamGet(&ptr);	// Encryption Method - Mandatory
	/////////////////////////////////////////////////////////////////////////////////////
	// sekim 20131122 ID1142 Add WiFi Auto-Security-Type Feature
	/*
	if ( !p ) return WXCODE_EINVAL;
	wifi_buff_authtype = str_to_authtype(upstr((char*)p));
	if ( wifi_buff_authtype==WICED_SECURITY_UNKNOWN ) return WXCODE_EINVAL;
	if ( wifi_mode == AP_MODE && wifi_buff_authtype == WICED_SECURITY_WEP_PSK )
		return WXCODE_EINVAL;
	*/
	wifi_buff_authtype = WICED_SECURITY_UNKNOWN;
	if ( p )
	{
		wifi_buff_authtype = str_to_authtype(upstr((char*)p));
		if ( wifi_buff_authtype==WICED_SECURITY_UNKNOWN ) return WXCODE_EINVAL;
		if ( wifi_mode == AP_MODE && wifi_buff_authtype == WICED_SECURITY_WEP_PSK )
			return WXCODE_EINVAL;
	}
	/////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////
	// sekim 20151204 Binary Key for Coway
	//p = WXParse_NextParamGet(&ptr);	// Pre-Shared Key - Partially Mandatory
	p = ptr;
	////////////////////////////////////////////////////////////////////////////////////////////////

	wifi_buff_keylen = strlen((char*)p);
	if ( !p && wifi_buff_authtype!=WICED_SECURITY_OPEN )
	{
		return WXCODE_EINVAL;
	}
	else if ( wifi_buff_authtype == WICED_SECURITY_WEP_PSK )		// WEP case verification
	{
		if ( wifi_buff_keylen == 5 || wifi_buff_keylen == 13 )
		{							// ASCII: 5 or 13
#if 0 //MikeJ 130621 ID1082 - WEP not work problem
			strcpy((char*)wifi_buff_keydata, (char*)p);
#else
			wifi_buff_keydata[0] = 0;
			wifi_buff_keydata[1] = wifi_buff_keylen;
			strcpy((char*)wifi_buff_keydata+2, (char*)p);
#endif
		}
		else if ( wifi_buff_keylen == 10 || wifi_buff_keylen == 26 )
		{							// HEX: 10 or 26
			UINT8 cnt, tmp[3]={0,};
			UINT32 val;
#if 1 //MikeJ 130806 ID1117 - WEP Security problem
			wifi_buff_keylen /= 2;
#endif

#if 1 //MikeJ 130621 ID1082 - WEP not work problem
			wifi_buff_keydata[0] = 0;
			wifi_buff_keydata[1] = wifi_buff_keylen;
#endif
#if 0 //MikeJ 130806 ID1117 - WEP Security problem
			for(cnt=0, wifi_buff_keylen/=2; cnt<wifi_buff_keylen; cnt++)
#else
			for(cnt=0; cnt<wifi_buff_keylen; cnt++)
#endif
			{
				memcpy(tmp, &p[cnt*2], 2);
				if(WXParse_Hex(tmp, &val) != WXCODE_SUCCESS) return WXCODE_EINVAL;
#if 0 //MikeJ 130621 ID1082 - WEP not work problem
				wifi_buff_keydata[cnt] = val;
#else
				wifi_buff_keydata[cnt+2] = val;
#endif
			}
		}
		else return WXCODE_EINVAL;	// Wrong Input
#if 1 //MikeJ 130806 ID1117 - WEP Security problem
		wifi_buff_keylen += 2;
#endif
	}
	else if ( wifi_buff_authtype & (WPA_SECURITY | WPA2_SECURITY) )	// WPA1, 2 case verification
	{
		if ( wifi_buff_keylen >= 8 && wifi_buff_keylen < 64 )
		{							// ASCII: 8 ~ 63
#if 0	// kaizen 20130722 ID1103 Modified bug which security key is not changed when enter 12345678 after 123456789
			strcpy((char*)wifi_buff_keydata, (char*)p);
#else
			strncpy((char*)wifi_buff_keydata, (char*)p, wifi_buff_keylen );
#endif
		}
		else if ( wifi_buff_keylen == 128 )
		{							// HEX: 128
			UINT8 cnt, tmp[3]={0,};
			UINT32 val;
			for(cnt=0, wifi_buff_keylen/=2; cnt<wifi_buff_keylen; cnt++)
			{
				memcpy(tmp, &p[cnt*2], 2);
				if(WXParse_Hex(tmp, &val) != WXCODE_SUCCESS) return WXCODE_EINVAL;
				wifi_buff_keydata[cnt] = val;
			}
		}
		else return WXCODE_EINVAL;	// Wrong Input
	}

	if( wifi_mode == STATION_MODE )
	{	// kaizen 20130403 이미 설정되어 있는 값을 똑같이 설정하려고 할 때는 Flash Memory에 저장하지 않도록 수정
#if 1	// kaizen 20130722 ID1103 Modified bug which security key is not changed when enter 12345678 after 123456789
		if( infra_entry->details.security == (wiced_security_t)wifi_buff_authtype
#if 0 //MikeJ 130806 ID1117 - WEP Security problem
			&& strncmp(infra_entry->security_key,(char*)wifi_buff_keydata, wifi_buff_keylen)==0 && temp_wxProfile.wifi_mode == wifi_mode && infra_entry->security_key_length == wifi_buff_keylen )
#else
			&& memcmp(infra_entry->security_key,(char*)wifi_buff_keydata, wifi_buff_keylen)==0 && temp_wxProfile.wifi_mode == wifi_mode && infra_entry->security_key_length == wifi_buff_keylen )
#endif
#else
			if( infra_entry->details.security == (wiced_security_t)wifi_buff_authtype
				&& strncmp(infra_entry->security_key,(char*)wifi_buff_keydata, wifi_buff_keylen)==0 && temp_wxProfile.wifi_mode == wifi_mode )
#endif
		{
			b_already_set = WICED_TRUE;
		}
		else
		{
			infra_entry->details.security = (wiced_security_t) wifi_buff_authtype;	// Station Mode를 위한 wifi_config_dct에 authtype 반영
			g_wxProfile.wifi_authtype = (wiced_security_t) wifi_buff_authtype;

			infra_entry->security_key_length = wifi_buff_keylen;					// kaizen 20130312
			memcpy( (char*)infra_entry->security_key, (char*)wifi_buff_keydata, wifi_buff_keylen );
			g_wxProfile.wifi_keylen = wifi_buff_keylen;
			memcpy( (char*)g_wxProfile.wifi_keydata, (char*)wifi_buff_keydata, wifi_buff_keylen );
			g_wxProfile.wifi_keydata[wifi_buff_keylen] = '\0';
		}
	}
	else if( wifi_mode == AP_MODE )
	{	// kaizen 20130403 이미 설정되어 있는 값을 똑같이 설정하려고 할 때는 Flash Memory에 저장하지 않도록 수정
#if 1	// kaizen 20130722 ID1103 Modified bug which security key is not changed when enter 12345678 after 123456789
		if( ap_entry->security == (wiced_security_t)wifi_buff_authtype
			&& strncmp(ap_entry->security_key,(char*)wifi_buff_keydata, wifi_buff_keylen)==0 && temp_wxProfile.wifi_mode == wifi_mode && ap_entry->security_key_length == wifi_buff_keylen )
#else
		if( ap_entry->security == (wiced_security_t)wifi_buff_authtype
			&& strncmp(ap_entry->security_key,(char*)wifi_buff_keydata, wifi_buff_keylen)==0 && temp_wxProfile.wifi_mode == wifi_mode )
#endif
		{
			b_already_set = WICED_TRUE;
		}
		else
		{
			ap_entry->security = (wiced_security_t) wifi_buff_authtype;				// AP Mode를 위한 wifi_config_dct에 authtype 반영
			g_wxProfile.ap_mode_authtype = (wiced_security_t) wifi_buff_authtype;

			ap_entry->security_key_length = wifi_buff_keylen;						// kaizen 20130312 AP Mode를 위한 wifi_config_dct에 Key 반영
			memcpy( (char*)ap_entry->security_key, (char*)wifi_buff_keydata, wifi_buff_keylen );
			g_wxProfile.ap_mode_keylen = wifi_buff_keylen;
			memcpy( (char*)g_wxProfile.ap_mode_keydata, (char*)wifi_buff_keydata, wifi_buff_keylen );
		}
	}
#else
	p = WXParse_NextParamGet(&ptr);
	if (!p)		return WXCODE_EINVAL;
	wifi_buff_authtype = str_to_authtype(upstr((char*)p));

	if ( wifi_buff_authtype==WICED_SECURITY_UNKNOWN )	return WXCODE_EINVAL;

	p = WXParse_NextParamGet(&ptr);
	if (p)
	{
		if ( strlen((char*)p)>=(sizeof(wifi_buff_keydata)-1) )	return WXCODE_EINVAL;
		if ( g_wxProfile.wifi_mode == 1 && ( strlen((char*)p) < 8 || strlen((char*)p) > 64 ) )	return WXCODE_EINVAL;
		strcpy((char*)wifi_buff_keydata, (char*)p);
	}

	p = WXParse_NextParamGet(&ptr);
	if (p)
	{
		status = WXParse_Int(p, &wifi_mode);
		if ( wifi_mode!=0 && wifi_mode!=1 )	return WXCODE_EINVAL;
		if (status != WXCODE_SUCCESS )		return WXCODE_EINVAL;
	}

	if( wifi_mode == STATION_MODE )
	{
		// kaizen 20130403 이미 설정되어 있는 값을 똑같이 설정하려고 할 때는 Flash Memory에 저장하지 않도록 수정
		if( infra_entry->details.security == (wiced_security_t)wifi_buff_authtype && strcmp(infra_entry->security_key,(char*)wifi_buff_keydata)==0 )
			b_already_set = WICED_TRUE;

		else
		{
			infra_entry->details.security = (wiced_security_t) wifi_buff_authtype;	// Station Mode를 위한 wifi_config_dct에 authtype 반영
			g_wxProfile.wifi_authtype = (wiced_security_t) wifi_buff_authtype;

			infra_entry->security_key_length = strlen((char*)wifi_buff_keydata);
			strcpy( (char*)infra_entry->security_key, (char*)wifi_buff_keydata );	// kaizen 20130312
			strcpy( (char*)g_wxProfile.wifi_keydata, (char*)wifi_buff_keydata );
		}
	}
	else if( wifi_mode == AP_MODE )
	{
		// kaizen 20130403 이미 설정되어 있는 값을 똑같이 설정하려고 할 때는 Flash Memory에 저장하지 않도록 수정
		if( ap_entry->security == (wiced_security_t)wifi_buff_authtype && strcmp(ap_entry->security_key,(char*)wifi_buff_keydata)==0 )
			b_already_set = WICED_TRUE;

		else
		{
			ap_entry->security = (wiced_security_t) wifi_buff_authtype;				// AP Mode를 위한 wifi_config_dct에 authtype 반영
			g_wxProfile.ap_mode_authtype = (wiced_security_t) wifi_buff_authtype;

			ap_entry->security_key_length = strlen((char*)wifi_buff_keydata );
			strcpy( (char*)ap_entry->security_key, (char*)wifi_buff_keydata );		// kaizen 20130312 AP Mode를 위한 wifi_config_dct에 Key 반영
			strcpy( (char*)g_wxProfile.ap_mode_keydata, (char*)wifi_buff_keydata );
		}
	}
#endif

#if 1	// kaizen 130412
	g_wxProfile.wifi_mode = (UINT32) wifi_mode;
#endif

	// kaizen 20130403		Flash에 저장되어 있는 값과 설정하려는 파라미터와 같으면 Flash에 저장하지 않음(Flash의 Access 횟수를 최대한 줄이기 위해)
	if( !b_already_set )
	{
#if 1	// kaizen 130412 ID1037 Do not save other information to DCT
		if(g_wxProfile.wifi_mode == STATION_MODE)
		{
			temp_wxProfile.wifi_authtype = (wiced_security_t)g_wxProfile.wifi_authtype;
			temp_wxProfile.wifi_keylen   = g_wxProfile.wifi_keylen;
			memcpy( (char*)temp_wxProfile.wifi_keydata, (char*)g_wxProfile.wifi_keydata, temp_wxProfile.wifi_keylen );
		}
		else if(g_wxProfile.wifi_mode == AP_MODE)
		{
			temp_wxProfile.ap_mode_authtype = (wiced_security_t)g_wxProfile.ap_mode_authtype;
			temp_wxProfile.ap_mode_keylen   = g_wxProfile.ap_mode_keylen;
			memcpy( (char*)temp_wxProfile.ap_mode_keydata, (char*)g_wxProfile.ap_mode_keydata, temp_wxProfile.ap_mode_keylen );
		}
#if 1 //MikeJ 130821 - WiFi Mode should be saved when WSET or WSEC was set
		temp_wxProfile.wifi_mode = g_wxProfile.wifi_mode;
#endif

		if ( WICED_SUCCESS!=wiced_dct_write_wifi_config_section(&wifi_config) )					W_DBG("wiced_dct_write_wifi_config_section error ");
		if ( WICED_SUCCESS!=wiced_dct_write_app_section( &temp_wxProfile, sizeof(WT_PROFILE) ))	W_DBG("wiced_dct_write_app_section error ");
#else
		// sekim 20133081 USART_Cmd(USART1, DISABLE) in wiced_dct_write_xxxx
		USART_Cmd(USART1, DISABLE);
		if ( WICED_SUCCESS!=wiced_dct_write_wifi_config_section(&wifi_config) )					W_DBG("wiced_dct_write_wifi_config_section error ");
		if ( WICED_SUCCESS!=wiced_dct_write_app_section( &g_wxProfile, sizeof(WT_PROFILE) )	)	W_DBG("wiced_dct_write_app_section error ");
		USART_Cmd(USART1, ENABLE);
#endif
	}

	return WXCODE_SUCCESS;
}

UINT8 WXCmd_WNET(UINT8 *ptr)
{
	UINT8 *p;
	UINT8 status;

	UINT32 dhcp_buff;
	WT_IPADDR ip1, ip2, ip3;
	memset(&ip1, 0, 4);
	memset(&ip2, 0, 4);
	memset(&ip3, 0, 4);

	if ( strcmp((char*)ptr, "?")==0 )
	{
		W_RSP("%d,%s,%s,%s\r\n", g_wxProfile.wifi_dhcp, g_wxProfile.wifi_ip, g_wxProfile.wifi_mask, g_wxProfile.wifi_gateway);
		return WXCODE_SUCCESS;
	}

	p = WXParse_NextParamGet(&ptr);
#if 0 //MikeJ 130821 ID1126 - DHCP option is not required but optional
	if (!p)		return WXCODE_EINVAL;
	status = WXParse_Int(p, &dhcp_buff);
	if ( dhcp_buff!=0 && dhcp_buff!=1 )	return WXCODE_EINVAL;
	if (status != WXCODE_SUCCESS )	return WXCODE_EINVAL;
#else
	if (p)
	{
		status = WXParse_Int(p, &dhcp_buff);
		if (status != WXCODE_SUCCESS )	return WXCODE_EINVAL;
		if ( dhcp_buff!=0 && dhcp_buff!=1 )	return WXCODE_EINVAL;
	} else dhcp_buff = 2;
#endif

	p = WXParse_NextParamGet(&ptr);
	if (p)
	{
		status = WXParse_Ip(p, (UINT8*)&ip1);
		if (status != WXCODE_SUCCESS )	return WXCODE_EINVAL;

		sprintf((char*)g_wxProfile.wifi_ip, "%d.%d.%d.%d", ((UINT8*)&ip1)[0], ((UINT8*)&ip1)[1], ((UINT8*)&ip1)[2], ((UINT8*)&ip1)[3]);		//kaizen 20130401
	}

	p = WXParse_NextParamGet(&ptr);
	if (p)
	{
		status = WXParse_Ip(p, (UINT8*)&ip2);
		if (status != WXCODE_SUCCESS )	return WXCODE_EINVAL;

		sprintf((char*)g_wxProfile.wifi_mask, "%d.%d.%d.%d", ((UINT8*)&ip2)[0], ((UINT8*)&ip2)[1], ((UINT8*)&ip2)[2], ((UINT8*)&ip2)[3]);	//kaizen 20130401
	}

	p = WXParse_NextParamGet(&ptr);
	if (p)
	{
		status = WXParse_Ip(p, (UINT8*)&ip3);
		if (status != WXCODE_SUCCESS )	return WXCODE_EINVAL;

		sprintf((char*)g_wxProfile.wifi_gateway, "%d.%d.%d.%d", ((UINT8*)&ip3)[0], ((UINT8*)&ip3)[1], ((UINT8*)&ip3)[2], ((UINT8*)&ip3)[3]);	//kaizen 20130401
	}

#if 0 //MikeJ 130821 ID1126 - DHCP option is not required but optional
	g_wxProfile.wifi_dhcp = dhcp_buff;
#else
	if(dhcp_buff != 2) g_wxProfile.wifi_dhcp = dhcp_buff;
#endif

//  kaizen 20130401
//	sprintf((char*)g_wxProfile.wifi_ip, "%d.%d.%d.%d", ((UINT8*)&ip1)[0], ((UINT8*)&ip1)[1], ((UINT8*)&ip1)[2], ((UINT8*)&ip1)[3]);
//	sprintf((char*)g_wxProfile.wifi_mask, "%d.%d.%d.%d", ((UINT8*)&ip2)[0], ((UINT8*)&ip2)[1], ((UINT8*)&ip2)[2], ((UINT8*)&ip2)[3]);
//	sprintf((char*)g_wxProfile.wifi_gateway, "%d.%d.%d.%d", ((UINT8*)&ip3)[0], ((UINT8*)&ip3)[1], ((UINT8*)&ip3)[2], ((UINT8*)&ip3)[3]);

	return WXCODE_SUCCESS;
}

#include "internal/wiced_internal_api.h"
#include "wiced_management.h"
#include "wiced_wifi.h"

UINT8 WXCmd_WSTAT(UINT8 *ptr)
{
	wiced_mac_t mac;
#if 0 //MikeJ 130702 ID1093 - Adjust Response Format
	wiced_wifi_get_mac_address( &mac );
	W_RSP("WICED Version  : " WICED_VERSION "\r\n");
	W_RSP("Platform       : " PLATFORM "\r\n");
	W_RSP("MAC Address    : %02X:%02X:%02X:%02X:%02X:%02X\r\n", mac.octet[0],mac.octet[1],mac.octet[2],mac.octet[3],mac.octet[4],mac.octet[5]);

	{
		extern char last_joined_ssid[32];
		extern char last_started_ssid[32];
		network_print_state(last_joined_ssid, last_started_ssid);
	}

	uint8_t dbm = 0;
	wiced_wifi_get_tx_power(&dbm);
	W_RSP("Transmit Power : %ddBm\r\n", dbm);

    int32_t rssi = 0;
    wiced_wifi_get_rssi(&rssi);
    W_RSP("RSSI           : %d\r\n", (int)rssi);
#else
	uint8_t dbm = 0;
	int32_t rssi = 0;
#if 0 //MikeJ 130806 ID1116 - Couldn't display 32 characters SSID
	extern char last_joined_ssid[32];
	extern char last_started_ssid[32];
#else
	extern char last_joined_ssid[];
	extern char last_started_ssid[];
#endif

	// sekim 20140929 ID1187 URIEL Customizing 2 : 9600, AT+WSTAT, W_AP_xxxxxxxx
	//W_RSP("IF/SSID/IP-Addr/Gateway/MAC/TxPower(dBm)/RSSI(-dBm)\r\n");
	if( !CHKCUSTOM("URIEL") )
	{
		W_RSP("IF/SSID/IP-Addr/Gateway/MAC/TxPower(dBm)/RSSI(-dBm)\r\n");
	}

	network_print_state(last_joined_ssid, last_started_ssid);

	wiced_wifi_get_mac_address( &mac );
	wiced_wifi_get_tx_power(&dbm);
	wiced_wifi_get_rssi(&rssi);
	if(rssi < 0) rssi*=-1;
	W_RSP("/%02X:%02X:%02X:%02X:%02X:%02X", mac.octet[0],mac.octet[1],mac.octet[2],mac.octet[3],mac.octet[4],mac.octet[5]);
	W_RSP("/%d", dbm);
    W_RSP("/%d\r\n", (int)rssi);
#endif

	return WXCODE_SUCCESS;
}

UINT8 WXCmd_WREG(UINT8 *ptr)
{
	UINT8 *p;

	if ( strcmp((char*)ptr, "?")==0 )
	{
		W_RSP("%c%c\r\n", g_wxProfile.wifi_countrycode[0], g_wxProfile.wifi_countrycode[1]);
		return WXCODE_SUCCESS;
	}

	p = WXParse_NextParamGet(&ptr);
	if (!p)		return WXCODE_EINVAL;
	if ( strlen((char*)p)!=2 )		return WXCODE_EINVAL;

	char constCountryCode[][3] = {
     "AF", "AL", "DZ", "AS", "AO", "AI", "AG", "AR", "AM", "AW", "AU", "AT", "AZ", "BS", "BH",
	 "0B", "BD", "BB", "BY", "BE", "BZ", "BJ", "BM", "BT", "BO", "BA", "BW", "BR", "IO", "BN",
	 "BG", "BF", "BI", "KH", "CM", "CA", "CV", "KY", "CF", "TD", "CL", "CN", "CX", "CO", "KM",
	 "CG", "CD", "CR", "CI", "HR", "CU", "CY", "CZ", "DK", "DJ", "DM", "DO", "EC", "EG", "SV",
	 "GQ", "ER", "EE", "ET", "FK", "FO", "FJ", "FI", "FR", "GF", "PF", "TF", "GA", "GM", "GE",
	 "DE", "GH", "GI", "GR", "GD", "GP", "GU", "GT", "GG", "GN", "GW", "GY", "HT", "VA", "HN",
	 "HK", "HU", "IS", "IN", "ID", "IR", "IQ", "IE", "IL", "IT", "JM", "JP", "JE", "JO", "KZ",
	 "KE", "KI", "KR", "0A", "KW", "KG", "LA", "LV", "LB", "LS", "LR", "LY", "LI", "LT", "LU",
	 "MO", "MK", "MG", "MW", "MY", "MV", "ML", "MT", "IM", "MQ", "MR", "MU", "YT", "MX", "FM",
	 "MD", "MC", "MN", "ME", "MS", "MA", "MZ", "MM", "NA", "NR", "NP", "NL", "AN", "NC", "NZ",
	 "NI", "NE", "NG", "NF", "MP", "NO", "OM", "PK", "PW", "PA", "PG", "PY", "PE", "PH", "PL",
	 "PT", "PR", "QA", "RE", "RO", "RU", "RW", "KN", "LC", "PM", "VC", "WS", "MF", "ST", "SA",
	 "SN", "RS", "SC", "SL", "SG", "SK", "SI", "SB", "SO", "ZA", "ES", "LK", "SR", "SZ", "SE",
	 "CH", "SY", "TW", "TJ", "TZ", "TH", "TG", "TO", "TT", "TN", "TR", "TM", "TC", "TV", "UG",
	 "UA", "AE", "GB", "US", "Q2", "UM", "UY", "UZ", "VU", "VE", "VN", "VG", "VI", "WF", "0C",
	 "EH", "YE", "ZM", "ZW", "" };

	int i=0;
	UINT8 found = 0;
	while(1)
	{
		if ( strlen(constCountryCode[i])==0 )		{ break; }
		if ( strcmp(constCountryCode[i], (char*)p)==0 )	{ found = 1; break; }
		i++;
	}
	if ( found==0 )		return WXCODE_EINVAL;

	memcpy(g_wxProfile.wifi_countrycode, p, 2);
	g_wxProfile.wifi_countrycode[2] = 0;


#if 1	// kaizen 20130729 ID1076 - When execute command to need system reset, WizFi250 should print [OK] message.
	WXS2w_StatusNotify(WXCODE_SUCCESS, 0);
#endif

	// sekim need to reboot
	// kaizen 20130408
	Save_Profile();
	Apply_To_Wifi_Dct();
	NVIC_SystemReset();

	return WXCODE_SUCCESS;
}


#if 1 //MikeJ 130702 ID1092 - ATCmd update (naming, adding comments)
UINT8 WXCmd_WWPS(UINT8 *ptr)
{
	UINT8 			*p;
	UINT8 			status;
	UINT32 			wps_mode;
	char			wps_pin_num[9]={0};
	int				wps_pin_num_len;
	wiced_result_t  wps_status;

    if( wiced_wifi_is_ready_to_transceive(WICED_STA_INTERFACE) == WICED_SUCCESS )
    {
    	W_DBG("Already run STA Mode");
    	return WXCODE_FAILURE;
    }
    else if( wiced_wifi_is_ready_to_transceive(WICED_AP_INTERFACE) == WICED_SUCCESS )
    {
    	W_DBG("Already run AP Mode");
    	return WXCODE_FAILURE;
    }

	p = WXParse_NextParamGet(&ptr);
	if (!p)		return WXCODE_EINVAL;
	status = WXParse_Int(p, &wps_mode);
	if ( status != WXCODE_SUCCESS )												return WXCODE_EINVAL;
	if ( wps_mode != WICED_WPS_PBC_MODE && wps_mode != WICED_WPS_PIN_MODE )		return WXCODE_EINVAL;

	if ( wps_mode == WICED_WPS_PIN_MODE )
	{
		p = WXParse_NextParamGet(&ptr);
		strcpy( (char*)wps_pin_num, (char*)p);
		wps_pin_num_len = strlen(wps_pin_num);
		if ( wps_pin_num_len > 8 )		return WXCODE_EINVAL;
	}
	else
	{
		wps_pin_num[0] = '\x00';
	}

	wizfi_task_monitor_stop = 1;
	wps_status = Request_WPS( wps_mode, wps_pin_num );
	if( wps_status != WICED_SUCCESS )
	{
		W_DBG( "Request WPS Fail, Status : %d",wps_status );
		return WXCODE_FAILURE;
	}
	wizfi_task_monitor_stop = 0;
	wiced_update_system_monitor(&wizfi_task_monitor_item, MAXIMUM_ALLOWED_INTERVAL_BETWEEN_WIZFIMAINTASK);

	return WXCODE_SUCCESS;
}
#endif

#if 1	// kaizen 20130716 ID1092 - ATCmd update (naming, adding comments)
static const wiced_wps_device_detail_t wizfi_wps_details=
{
	.device_name     = PLATFORM,
	.manufacturer    = "Broadcom",
	.model_name      = PLATFORM,
	.model_number    = "2.0",
	.serial_number   = "1408248",
	.device_category = WICED_WPS_DEVICE_COMPUTER,
	.sub_category    = 7,
	//.default_pin     = "12345670",
	.config_methods  = WPS_CONFIG_LABEL | WPS_CONFIG_VIRTUAL_PUSH_BUTTON | WPS_CONFIG_VIRTUAL_DISPLAY_PIN
};

wiced_result_t Request_WPS( wiced_wps_mode_t mode, char * p_wps_pin_num )
{
	wiced_result_t result;
	wiced_wps_credential_t* wps_credentials = MALLOC_OBJECT("wps",wiced_wps_credential_t);


	if ( p_wps_pin_num[0] == '\x00' )
	{
		result = wiced_wps_enrollee(mode, &wizfi_wps_details, "00000000", wps_credentials, 1);
	}
	else
	{
		result = wiced_wps_enrollee(mode, &wizfi_wps_details, p_wps_pin_num, wps_credentials, 1);
	}

    if (result == WICED_SUCCESS)
    {
    	struct
        {
            wiced_bool_t             device_configured;
            wiced_config_ap_entry_t  ap_entry;
        } temp_config;

        memset(&temp_config, 0, sizeof(temp_config));
        memcpy(&temp_config.ap_entry.details.SSID,     &wps_credentials->ssid, sizeof(wiced_ssid_t));
        memcpy(&temp_config.ap_entry.details.security, &wps_credentials->security, sizeof(wiced_security_t));
        memcpy(temp_config.ap_entry.security_key,       wps_credentials->passphrase, wps_credentials->passphrase_length);
        temp_config.ap_entry.security_key_length = wps_credentials->passphrase_length;
        temp_config.device_configured = WICED_TRUE;
        bootloader_api->write_wifi_config_dct(0, &temp_config, sizeof(temp_config));

#if 1 // kaizen
        WT_PROFILE temp_dct;
    	wiced_dct_read_app_section( &temp_dct, sizeof(WT_PROFILE) );

    	strcpy((char*)temp_dct.wifi_ssid, (char*)temp_config.ap_entry.details.SSID.val);
        strcpy((char*)temp_dct.wifi_keydata, (char*)temp_config.ap_entry.security_key);
        temp_dct.wifi_keylen   = temp_config.ap_entry.security_key_length;	// kaizen 20130716 ID1100 Modified bug which security key is changed when execute WPS
        temp_dct.wifi_authtype = temp_config.ap_entry.details.security;
        temp_dct.wifi_mode 	   = STATION_MODE;								// kaizen 20130717 ID1102 Fixed bug which WizFi250 do not set association information in AP mode

        wiced_dct_write_app_section( &temp_dct, sizeof(WT_PROFILE) );
        Load_Profile();
#endif
    }
    else
    {
        /* TODO: WPS failed.. Do something */
    }
    free(wps_credentials);

    return result;
}
#endif

// sekim 20130806 ID1120 Add DNS Server if using Static IP
UINT8 WXCmd_WADNS(UINT8 *ptr)
{
	UINT8 *p;
	UINT8 status;

	WT_IPADDR ip1, ip2;
	memset(&ip1, 0, 4);
	memset(&ip2, 0, 4);

	if ( strcmp((char*)ptr, "?")==0 )
	{
		W_RSP("%s,%s\r\n", g_wxProfile.wifi_dns1, g_wxProfile.wifi_dns2);
		return WXCODE_SUCCESS;
	}

	p = WXParse_NextParamGet(&ptr);
	if (p)
	{
		status = WXParse_Ip(p, (UINT8*)&ip1);
		if (status != WXCODE_SUCCESS )	return WXCODE_EINVAL;

		sprintf((char*)g_wxProfile.wifi_dns1, "%d.%d.%d.%d", ((UINT8*)&ip1)[0], ((UINT8*)&ip1)[1], ((UINT8*)&ip1)[2], ((UINT8*)&ip1)[3]);		//kaizen 20130401
	}

	p = WXParse_NextParamGet(&ptr);
	if (p)
	{
		status = WXParse_Ip(p, (UINT8*)&ip2);
		if (status != WXCODE_SUCCESS )	return WXCODE_EINVAL;

		sprintf((char*)g_wxProfile.wifi_dns2, "%d.%d.%d.%d", ((UINT8*)&ip2)[0], ((UINT8*)&ip2)[1], ((UINT8*)&ip2)[2], ((UINT8*)&ip2)[3]);	//kaizen 20130401
	}

    {
    	wiced_ip_address_t 	dns_ip;

    	dns_client_remove_all_server_addresses();

    	SET_IPV4_ADDRESS(dns_ip, str_to_ip((char*)g_wxProfile.wifi_dns1) );
    	dns_client_add_server_address(dns_ip);
    	SET_IPV4_ADDRESS(dns_ip, str_to_ip((char*)g_wxProfile.wifi_dns2) );
    	dns_client_add_server_address(dns_ip);
    }

	return WXCODE_SUCCESS;
}

//////////////////////////////////////////////////////////////////////////////
// sekim 20140214 ID1163 Add AT+WANT
UINT8 WXCmd_WANT(UINT8 *ptr)
{
	UINT8 *p;
	UINT8 status;
	UINT32 ant_type;

	if ( strcmp((char*)ptr, "?")==0 )
	{
		W_RSP("%d(%d)\r\n", g_wxProfile.antenna_type, GetAntennaType(0));
		return WXCODE_SUCCESS;
	}

	p = WXParse_NextParamGet(&ptr);
	if (!p)		return WXCODE_EINVAL;
	status = WXParse_Int(p, &ant_type);
	if ( status != WXCODE_SUCCESS )							return WXCODE_EINVAL;
	if ( ant_type!=0 && ant_type!=1 && ant_type!=3 )		return WXCODE_EINVAL;

	g_wxProfile.antenna_type = ant_type;
	if ( wiced_wifi_select_antenna(ant_type)!=WICED_SUCCESS )
	{
		return WXCODE_FAILURE;
	}

	return WXCODE_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////////


// sekim 20131122 ID1142 Add WiFi Auto-Security-Type Feature
wiced_security_t scan_wifi_and_get_sectype(wiced_ssid_t* optional_ssid)
{
	scan_user_data_t* data = (scan_user_data_t*) malloc(sizeof(scan_user_data_t));
	memset(data, 0, sizeof(scan_user_data_t));
	host_rtos_init_semaphore(&data->num_scan_results_semaphore);
	wiced_scan_result_t * result_ptr = (wiced_scan_result_t *) &data->result_buff;

	wiced_security_t result_security = WICED_SECURITY_UNKNOWN;

	wiced_scan_extended_params_t scan_ex_param;
	scan_ex_param.number_of_probes_per_channel = 5;
	scan_ex_param.scan_active_dwell_time_per_channel_ms = 70;
	scan_ex_param.scan_home_channel_dwell_time_between_channels_ms = 1;
	scan_ex_param.scan_passive_dwell_time_per_channel_ms = 150;
	if ( WICED_SUCCESS != wiced_wifi_scan(WICED_SCAN_TYPE_ACTIVE, WICED_BSS_TYPE_ANY, optional_ssid, NULL, NULL, &scan_ex_param, scan_results_handler, &result_ptr, data) )
	{
		W_DBG("Error starting scan\r\n");

		host_rtos_deinit_semaphore(&data->num_scan_results_semaphore);
		free(data);
		return result_security;
	}

	while (host_rtos_get_semaphore(&data->num_scan_results_semaphore, NEVER_TIMEOUT, WICED_FALSE) == WICED_SUCCESS)
	{
		wiced_scan_result_t* record = &data->result_buff[data->result_buff_read_pos];
		if ( record->channel == 0xff )
		{
			break;
		}
		if ( (record->bss_type!=WICED_BSS_TYPE_INFRASTRUCTURE) )
		{
			continue;
		}

		if ( record->security!=WICED_SECURITY_UNKNOWN )
		{
			result_security = record->security;
		}

		data->result_buff_read_pos++;
		if ( data->result_buff_read_pos >= CIRCULAR_RESULT_BUFF_SIZE )
		{
			data->result_buff_read_pos = 0;
		}
	}

	host_rtos_deinit_semaphore(&data->num_scan_results_semaphore);
	free(data);

	return result_security;
}

////////////////////////////////////////////////////////////////////////////////
// sekim 20140214 ID1163 Add AT+WANT
#include "internal/SDPCM.h"
#include "Network/wwd_buffer_interface.h"

UINT8 GetAntennaType(UINT8 bAP)
{
    wiced_buffer_t buffer;
    wiced_buffer_t response;
    wiced_result_t result;
    UINT8 ant_type1 = 0xff, ant_type2 = 0xff;

    wiced_get_ioctl_buffer( &buffer, sizeof(wiced_antenna_t) );
    if ( bAP )
    	result = wiced_send_ioctl(SDPCM_GET, WLC_GET_TXANT, buffer, &response, SDPCM_AP_INTERFACE);
    else
    	result = wiced_send_ioctl(SDPCM_GET, WLC_GET_TXANT, buffer, &response, SDPCM_STA_INTERFACE);
    if ( result == WICED_SUCCESS )
    {
    	// I have no idea about getting antenna_info, so I did it as below
    	wiced_antenna_t* antenna_info = (wiced_antenna_t*) host_buffer_get_current_piece_data_pointer( response );
    	ant_type1 = *(uint8_t*)antenna_info;
        host_buffer_release( response, WICED_NETWORK_RX );
    }
    wiced_get_ioctl_buffer( &buffer, sizeof(wiced_antenna_t) );
    if ( bAP )
    	result = wiced_send_ioctl(SDPCM_GET, WLC_GET_ANTDIV, buffer, &response, SDPCM_AP_INTERFACE);
    else
    	result = wiced_send_ioctl(SDPCM_GET, WLC_GET_ANTDIV, buffer, &response, SDPCM_STA_INTERFACE);
    if ( result == WICED_SUCCESS )
    {
    	wiced_antenna_t* antenna_info = (wiced_antenna_t*) host_buffer_get_current_piece_data_pointer( response );
    	ant_type2 = *(uint8_t*)antenna_info;
        host_buffer_release( response, WICED_NETWORK_RX );
    }

    if ( ant_type1!=0 && ant_type1!=1 && ant_type1!=3 )
    {
    	W_DBG("Invalid Antenna Type 1 (%d)", ant_type1);
    	return 9;
    }
    else if ( ant_type1!=ant_type2 )
    {
    	W_DBG("Invalid Antenna Type 2 (%d, %d)", ant_type1, ant_type2);
    	return 9;
    }

    return ant_type1;
}
////////////////////////////////////////////////////////////////////////////////


// sekim 20140225 ID1165 Add WiFi Rate Command (by Shin-Heung)
UINT8 set_80211bgn_mode(UINT8 bgn_mode)
{
    wiced_buffer_t buffer;
    uint32_t* data;
    wiced_result_t result;

	// bgn_mode 1(disable nmode) 2(disable nmode and gmode)
    if ( bgn_mode==0 )
    {
		data = wiced_get_ioctl_buffer( &buffer, (uint16_t) 4 );
		if ( data == NULL )			return WXCODE_FAILURE;
		*data = (uint32_t)1;
		if ( (result=wiced_send_ioctl( SDPCM_SET, WLC_SET_GMODE, buffer, 0, SDPCM_STA_INTERFACE )) != WICED_SUCCESS )
		{
			W_DBG("set_80211bgn_mode error 1(%d)", result);
			return WXCODE_FAILURE;
		}

	    data = wiced_get_iovar_buffer( &buffer, (uint16_t) 4, "nmode" );
	    if ( data == NULL )	        return WXCODE_FAILURE;
	    *data = (uint32_t)1;
	    if ( (result=wiced_send_iovar( SDPCM_SET, buffer, 0, SDPCM_STA_INTERFACE )) != WICED_SUCCESS )
	    {
	    	W_DBG("set_80211bgn_mode error 2(%d)", result);
	        return WXCODE_FAILURE;
	    }
    }
    else if ( bgn_mode==1 )
    {
	    data = wiced_get_iovar_buffer( &buffer, (uint16_t) 4, "nmode" );
	    if ( data == NULL )	        return WXCODE_FAILURE;
	    *data = (uint32_t)0;
	    if ( (result=wiced_send_iovar( SDPCM_SET, buffer, 0, SDPCM_STA_INTERFACE )) != WICED_SUCCESS )
	    {
	    	W_DBG("set_80211bgn_mode error 3(%d)", result);
	        return WXCODE_FAILURE;
	    }

	    data = wiced_get_ioctl_buffer( &buffer, (uint16_t) 4 );
		if ( data == NULL )			return WXCODE_FAILURE;
		*data = (uint32_t)1;
		if ( (result=wiced_send_ioctl( SDPCM_SET, WLC_SET_GMODE, buffer, 0, SDPCM_STA_INTERFACE )) != WICED_SUCCESS )
		{
			W_DBG("set_80211bgn_mode error 4(%d)", result);
			return WXCODE_FAILURE;
		}
    }
    else if ( bgn_mode==2 )
    {
	    data = wiced_get_iovar_buffer( &buffer, (uint16_t) 4, "nmode" );
	    if ( data == NULL )	        return WXCODE_FAILURE;
	    *data = (uint32_t)0;
	    if ( (result=wiced_send_iovar( SDPCM_SET, buffer, 0, SDPCM_STA_INTERFACE )) != WICED_SUCCESS )
	    {
	    	W_DBG("set_80211bgn_mode error 5(%d)", result);
	        return WXCODE_FAILURE;
	    }

	    data = wiced_get_ioctl_buffer( &buffer, (uint16_t) 4 );
		if ( data == NULL )			return WXCODE_FAILURE;
		*data = (uint32_t)0;
		if ( (result=wiced_send_ioctl( SDPCM_SET, WLC_SET_GMODE, buffer, 0, SDPCM_STA_INTERFACE )) != WICED_SUCCESS )
		{
			W_DBG("set_80211bgn_mode error 6(%d)", result);
			return WXCODE_FAILURE;
		}
    }
	return WXCODE_SUCCESS;
}

UINT8 WXCmd_WBGN(UINT8 *ptr)
{
	UINT8 *p;
	UINT8 status;
	UINT32 bgnmode;

	p = WXParse_NextParamGet(&ptr);
	if (!p)		return WXCODE_EINVAL;
	status = WXParse_Int(p, &bgnmode);
	if ( status != WXCODE_SUCCESS )					return WXCODE_EINVAL;
	if ( bgnmode!=0 && bgnmode!=1 && bgnmode!=2 )	return WXCODE_EINVAL;

	if ( set_80211bgn_mode(bgnmode)!=WXCODE_SUCCESS )	return WXCODE_FAILURE;

	return WXCODE_SUCCESS;
}

#if 1			// kaizen 20140409 ID1156 Added Wi-Fi Direct Function For Test
#include "wiced_p2p.h"
#include "wps_host.h"

static const besl_p2p_device_detail_t device_details =
{
    .wps_device_details =
    {
   		// sekim 20140428 P2P device name WizFi250-P2P
        //.device_name     = "Wiced",
    	.device_name     = "WizFi250-P2P",
        .manufacturer    = "Broadcom",
        .model_name      = "BCM943362",
        .model_number    = "Wiced",
        .serial_number   = "12345670",
        .device_category = WICED_WPS_DEVICE_COMPUTER,
        .sub_category    = 7,
        .config_methods  = WPS_CONFIG_PUSH_BUTTON | WPS_CONFIG_VIRTUAL_PUSH_BUTTON | WPS_CONFIG_VIRTUAL_DISPLAY_PIN,
    },
    .group_owner_intent = 1,
    .ap_ssid_suffix     = "wiced!",
    // sekim 20140428 P2P device name WizFi250-P2P
    //.device_name        = "WICED-P2P",
    .device_name        = "WizFi250-P2P",
};

static p2p_workspace_t workspace;

// sekim 20140508 double check Link Down (for WiFi Direct)
UINT8 g_used_wp2p = 0;
UINT8 WXCmd_WP2P_Start(UINT8 *ptr)
{
	// sekim 20140430 add checking p2p condition
	if ( g_wxProfile.msgLevel!=2 )
	{
		W_DBG("can't start p2p(msg level error)");
		return WXCODE_FAILURE;
	}
	if ( g_wxProfile.wifi_mode!=STATION_MODE )
	{
		W_DBG("can't start p2p(wifi mode error)");
		return WXCODE_FAILURE;
	}

	besl_p2p_init( &workspace, &device_details );
	besl_p2p_start( &workspace );

	g_used_wp2p = 1;

	return WXCODE_SUCCESS;
}

UINT8 WXCmd_WP2P_Stop(UINT8 *ptr)
{
	besl_p2p_stop( &workspace );

	return WXCODE_SUCCESS;
}

UINT8 WXCmd_WP2P_PeerList(UINT8 *ptr)
{
    p2p_discovered_device_t* devices;
    uint8_t device_count;

    besl_p2p_get_discovered_peers(&workspace, &devices, &device_count);

    W_RSP("P2P Peers:\r\n");
    for(; device_count != 0; )
    {
        --device_count;
        W_RSP(" %u: '%s' on channel %u\r\n", device_count, devices[device_count].device_name, devices[device_count].channel);
    }

    return WXCODE_SUCCESS;
}

UINT8 WXCmd_WP2P_Invite(UINT8 *ptr)
{
	UINT8 *p;
	UINT8 status;
	UINT32 id;

	p = WXParse_NextParamGet(&ptr);
	if (!p)		return WXCODE_EINVAL;
	status = WXParse_Int(p, &id);
	if ( status != WXCODE_SUCCESS )			return WXCODE_EINVAL;

    besl_p2p_invite(&workspace, &workspace.discovered_devices[id]);

	return WXCODE_SUCCESS;
}
#endif

//////////////////////////////////////////////////////////////////////////////
// sekim 20140710 ID1179 add wcheck_option
wiced_thread_t thread_handle_wcheck;
uint8_t g_wcheck_status = 0;
UINT8 WXCmd_WCHECK(UINT8 *ptr)
{
	UINT8 *p;
	UINT32 buff1 = 0, buff2 = 0, buff3 = 0, buff4 = 0;
	UINT8 status;

	if ( strcmp((char*)ptr, "?") == 0 )
	{
		// sekim add wcheck_option4 for Smart-ANT)
		//W_RSP("%d,%d,%d\r\n", g_wxProfile.wcheck_option1, g_wxProfile.wcheck_option2, g_wxProfile.wcheck_option3);
		W_RSP("%d,%d,%d,%d\r\n", g_wxProfile.wcheck_option1, g_wxProfile.wcheck_option2, g_wxProfile.wcheck_option3, g_wxProfile.wcheck_option4);
		return WXCODE_SUCCESS;
	}

	p = WXParse_NextParamGet(&ptr);
	if ( !p )							return WXCODE_EINVAL;
	status = WXParse_Int(p, &buff1);
	if ( status != WXCODE_SUCCESS )		return WXCODE_EINVAL;

	if ( buff1==0 )
	{
		g_wxProfile.wcheck_option1 = 0;
		return WXCODE_SUCCESS;
	}

	p = WXParse_NextParamGet(&ptr);
	if ( !p )							return WXCODE_EINVAL;
	status = WXParse_Int(p, &buff2);
	if ( status != WXCODE_SUCCESS )		return WXCODE_EINVAL;

	p = WXParse_NextParamGet(&ptr);
	if ( !p )							return WXCODE_EINVAL;
	status = WXParse_Int(p, &buff3);
	if ( status != WXCODE_SUCCESS )		return WXCODE_EINVAL;

	// sekim add wcheck_option4 for Smart-ANT)
	p = WXParse_NextParamGet(&ptr);
	if ( p )
	{
		status = WXParse_Int(p, &buff4);
		if ( status != WXCODE_SUCCESS )		return WXCODE_EINVAL;
	}

	if ( !(buff1>=10 && buff1<=3600*24) )	return WXCODE_EINVAL;
	if ( !(buff2>=5 && buff2<=20) )			return WXCODE_EINVAL;
	if ( !(buff3>=1 && buff3<=20) )			return WXCODE_EINVAL;

	g_wxProfile.wcheck_option1 = buff1;
	g_wxProfile.wcheck_option2 = buff2;
	g_wxProfile.wcheck_option3 = buff3;
	// sekim add wcheck_option4 for Smart-ANT)
	g_wxProfile.wcheck_option4 = buff4;

	if ( g_wxProfile.wcheck_option1>0 )
	{
		if ( g_wcheck_status==0 )
		{
			wifi_check_process();
		}
	}

	return WXCODE_SUCCESS;
}

void wifi_check_thread()
{
	UINT8				i;
	UINT8				cnt_timeout;
	uint32_t			elapsed_ms;
	uint32_t			loop_delay;
	wiced_result_t		wiced_status;
	wiced_ip_address_t	addr;

	g_wcheck_status = 1;
	while(1)
	{
		wiced_rtos_delay_milliseconds(1000);
		for (loop_delay=1; loop_delay<g_wxProfile.wcheck_option1; loop_delay++)
		{
			wiced_rtos_delay_milliseconds(1000);
		}
		////////////////////////////////////////////////////////////////////////////////////////////////
		// sekim 20150401 Reset until association using WCHECK for ATK
		//if ( wiced_wifi_is_ready_to_transceive(WICED_STA_INTERFACE)!=WICED_SUCCESS )	continue;
		if ( wiced_wifi_is_ready_to_transceive(WICED_STA_INTERFACE)!=WICED_SUCCESS )
		{
			if ( strcmp((char*)g_wxProfile.scon_opt1, "SO") == 0 || strcmp((char*)g_wxProfile.scon_opt1, "S") == 0 )
			{
				send_maincommand_queue(107, 0);
			}
		}
		////////////////////////////////////////////////////////////////////////////////////////////////
		if ( g_wxProfile.wcheck_option1==0 )											continue;

		wiced_ip_get_gateway_address( (wiced_interface_t)(wiced_interface_t)g_wxProfile.wifi_mode, &addr );

		cnt_timeout = 0;
		for(i=0; i<g_wxProfile.wcheck_option2; i++)
		{
			wiced_status = wiced_ping((wiced_interface_t)g_wxProfile.wifi_mode, &addr, 1000, &elapsed_ms);

			if ( wiced_status == WICED_SUCCESS )
			{
				//W_DBG("Ping Reply : %3lu ms", (uint32_t)elapsed_ms );
			}
			else if ( wiced_status == WICED_TIMEOUT )
			{
				//W_DBG("Ping timeout");
				cnt_timeout++;
			}
			else
			{
				W_DBG("Ping error");
			}
		}
		//W_DBG("Ping : %d/%d", cnt_timeout, g_wxProfile.wcheck_option2);

		if ( cnt_timeout>=g_wxProfile.wcheck_option3 )
		{
			////////////////////////////////////////////////////////////////////////////////////////
			// sekim add wcheck_option4 for Smart-ANT)
			/*
			W_DBG("Reset because of too many ping timeout(%d/%d)", cnt_timeout, g_wxProfile.wcheck_option2);
			send_maincommand_queue(107, 0);
			*/
			if ( g_wxProfile.wcheck_option4==1 )
			{
				static int smart_antenna = 1;
				if ( g_wxProfile.antenna_type==3 )
				{
					if ( smart_antenna==1 )			smart_antenna = 0;
					else if ( smart_antenna==0 )	smart_antenna = 1;
					wiced_wifi_select_antenna(smart_antenna);
					W_DBG("Smart Antenna(%d) because of too many ping timeout(%d/%d)", smart_antenna, cnt_timeout, g_wxProfile.wcheck_option2);
				}
			}
			else
			{
				W_DBG("Reset because of too many ping timeout(%d/%d)", cnt_timeout, g_wxProfile.wcheck_option2);
				send_maincommand_queue(107, 0);
			}
			////////////////////////////////////////////////////////////////////////////////////////
		}
	}
	g_wcheck_status = 0;
}

void wifi_check_process()
{
	wiced_result_t result;

	if ( g_wcheck_status!=0 )				return;
	if ( g_wxProfile.wcheck_option1==0 )	return;

	if ( (result=wiced_rtos_create_thread((wiced_thread_t*)&thread_handle_wcheck, WICED_APPLICATION_PRIORITY + 3,
			"WCHECK-PING", wifi_check_thread, 1024*2, NULL))!=WICED_SUCCESS )
	{
		W_DBG("wiced_rtos_create_thread : wifi_check_thread error %d (rebooting?)", result);
	}
}
//////////////////////////////////////////////////////////////////////////////

// sekim 20150107 added WXCmd_WRFMODE
UINT8 WXCmd_WRFMODE(UINT8 *ptr)
{
	UINT8 *p;
	UINT8 status;
	UINT32 buff_rfmode1;

	if ( strcmp((char*)ptr, "?")==0 )
	{
		uint32_t mode1 = 999;

		if ( wwd_wifi_get_interference_mode( &mode1 )!=WICED_SUCCESS )
			W_DBG("wwd_wifi_get_interference_mode error ");

		W_RSP("%d(%d)\r\n", g_wxProfile.rfmode1, mode1);
		return WXCODE_SUCCESS;
	}

	p = WXParse_NextParamGet(&ptr);
	if (!p)		return WXCODE_EINVAL;
	status = WXParse_Int(p, &buff_rfmode1);
	if ( status != WXCODE_SUCCESS )				return WXCODE_EINVAL;
	if ( buff_rfmode1<0 || buff_rfmode1>4 )		return WXCODE_EINVAL;

	g_wxProfile.rfmode1 = buff_rfmode1;
	if ( wwd_wifi_set_interference_mode(buff_rfmode1)!=WICED_SUCCESS )
	{
		return WXCODE_FAILURE;
	}

	return WXCODE_SUCCESS;
}
