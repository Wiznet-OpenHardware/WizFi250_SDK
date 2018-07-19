#include "wx_defines.h"

#include "bootloader_app.h"
#include "wiced_tcpip.h"
#include "wiced_dct.h"

WT_SCLIST g_scList[WX_MAX_SCID_RANGE];
wiced_thread_t g_socketopen_thread[WX_MAX_SCID_RANGE];
UINT8 g_scRxBuffer[WX_DATABUFFER_SIZE];

wiced_ip_address_t g_UDP_last_packet_ip;
UINT16 g_UDP_last_packet_port;

// sekim 20140929 ID1188 Data-Idle-Auto-Reset (Autonix)
uint32_t g_time_lastdata;

#define TCP_CLIENT_CONNECT_TIMEOUT        5000

#if 0	// kaizen	20130520 ID1068 - Modified for using dct_security_section
static const char brcm_server_certificate[] =
    "-----BEGIN CERTIFICATE-----\r\n"
    "MIIDnzCCAwigAwIBAgIJANJ7KTgzT4vwMA0GCSqGSIb3DQEBBQUAMIGSMQswCQYD\r\n"
    "VQQGEwJBVTEMMAoGA1UECBMDTlNXMQ8wDQYDVQQHEwZTeWRuZXkxHTAbBgNVBAoT\r\n"
    "FEJyb2FkY29tIENvcnBvcmF0aW9uMQ0wCwYDVQQLEwRXTEFOMREwDwYDVQQDEwhC\r\n"
    "cm9hZGNvbTEjMCEGCSqGSIb3DQEJARYUc3VwcG9ydEBicm9hZGNvbS5jb20wHhcN\r\n"
    "MTIwMzEyMjE0ODIyWhcNMTIwNDExMjE0ODIyWjCBkjELMAkGA1UEBhMCQVUxDDAK\r\n"
    "BgNVBAgTA05TVzEPMA0GA1UEBxMGU3lkbmV5MR0wGwYDVQQKExRCcm9hZGNvbSBD\r\n"
    "b3Jwb3JhdGlvbjENMAsGA1UECxMEV0xBTjERMA8GA1UEAxMIQnJvYWRjb20xIzAh\r\n"
    "BgkqhkiG9w0BCQEWFHN1cHBvcnRAYnJvYWRjb20uY29tMIGfMA0GCSqGSIb3DQEB\r\n"
    "AQUAA4GNADCBiQKBgQDVyKV/R5LCQvLliHFU1303w8nv63THLzmUMGqpf1/N/nZP\r\n"
    "pPsgihr/ZZDFxb4zNKpUqU1yoPl0QpB9TdjFDHzHMzGdo6ZcKGk86xTJSg6Ed6uT\r\n"
    "O/qXDXIvURiDhAdQXmBNDs9xGoIxGH8FwvNHPfLENCfnuHKX9KDye6MvOv5kHwID\r\n"
    "AQABo4H6MIH3MB0GA1UdDgQWBBRDEgx5s41uyudha03Bv3YaGy6NdzCBxwYDVR0j\r\n"
    "BIG/MIG8gBRDEgx5s41uyudha03Bv3YaGy6Nd6GBmKSBlTCBkjELMAkGA1UEBhMC\r\n"
    "QVUxDDAKBgNVBAgTA05TVzEPMA0GA1UEBxMGU3lkbmV5MR0wGwYDVQQKExRCcm9h\r\n"
    "ZGNvbSBDb3Jwb3JhdGlvbjENMAsGA1UECxMEV0xBTjERMA8GA1UEAxMIQnJvYWRj\r\n"
    "b20xIzAhBgkqhkiG9w0BCQEWFHN1cHBvcnRAYnJvYWRjb20uY29tggkA0nspODNP\r\n"
    "i/AwDAYDVR0TBAUwAwEB/zANBgkqhkiG9w0BAQUFAAOBgQCrKwmGU1Zd4ZSdr3zy\r\n"
    "qhKXb3PZwr01k1CXgYHgBv7VomIeMqRjE2S/oAM5tWmjWPuvfKjROJNoFestWM3w\r\n"
    "Pad3TtmGZSnHK13LXVdoGlDb/zYzlQhoAYYxvQrt0VD7sNhyvT9Ec1/trmfMR+jK\r\n"
    "RljZgDdQdIhenl+zlV33enbx2Q==\r\n"
    "-----END CERTIFICATE-----\r\n";

static const char brcm_server_rsa_key[] =
    "-----BEGIN RSA PRIVATE KEY-----\r\n"
    "MIICWwIBAAKBgQDVyKV/R5LCQvLliHFU1303w8nv63THLzmUMGqpf1/N/nZPpPsg\r\n"
    "ihr/ZZDFxb4zNKpUqU1yoPl0QpB9TdjFDHzHMzGdo6ZcKGk86xTJSg6Ed6uTO/qX\r\n"
    "DXIvURiDhAdQXmBNDs9xGoIxGH8FwvNHPfLENCfnuHKX9KDye6MvOv5kHwIDAQAB\r\n"
    "AoGAEiw7PUWVSSQtx6tAjwi+YTYofVeTlrcB+wHen0fvmfAumHiazFpRDzLQCq/T\r\n"
    "ikDI1eeKaNscOXDLHYu3iJCWLqTkmhMBweOuorYMIQIBYbIqUg6OsBoY7oRrJbwg\r\n"
    "9jJEFI2vPEtIWLn3cRRgL9fs6zF3unF0qXMkr3N34BuyEYECQQD5yfF7cWgEKQvS\r\n"
    "P4yaZcGQZdcU3G3OEh+SpgFQmN5fYol2VTKZzd1iR9GQSUjoTrkMEVNiwQtVvYiu\r\n"
    "a9Pt4xffAkEA2xmCW8XkUFc94uRx2MTCjn3so1am79SRc3f4SRbA4rfJfQH883a0\r\n"
    "YYSV2tZICWq8VKpdsGYCFWSajMazgNc7wQJAUq9UbmZl5iqoLRq4MkvIvUHY5qDp\r\n"
    "ADPjm6mz+bgAtFZr5m3haCRLSkM3zalUpwGYI7SAg8ofNGyfGA29g5uOxQJARAUF\r\n"
    "XWxwVyjeg6QcXAmpxQb/Ai6SoP5DMa/bGwW/WCNqoC6P0x3VHjlFNK01rAbA9R/2\r\n"
    "+h6RIwcam/3MGIG5gQJANq3n4PlU8nkn661XfWpEXKrwHwqZqMjtKaLRBwmE1ojU\r\n"
    "B3zc3RMJ5qy0nBYkFdQkbDiomqRjnnT/j4kDJF8Xfw==\r\n"
    "-----END RSA PRIVATE KEY-----\r\n";
#endif

void WXNetwork_Initialize()
{
	platform_dct_wifi_config_t wifi_config;

	wiced_dct_read_wifi_config_section(&wifi_config);
	if( wifi_config.device_configured == WICED_TRUE && g_wxProfile.wifi_mode == STATION_MODE )
	{
		wifi_config.device_configured = WICED_FALSE;
		wiced_dct_write_wifi_config_section( &wifi_config );
	}
}

char g_BuffIPString[16];
void* WXNetwork_WicedV4IPToString(char* pIP, wiced_ip_address_t wiced_ip)
{
	char *pDes = 0;
	if ( pIP )	pDes = pIP;
	else		pDes = g_BuffIPString;

	sprintf(pDes, "%u.%u.%u.%u", (uint8_t)(GET_IPV4_ADDRESS(wiced_ip) >> 24), (uint8_t)(GET_IPV4_ADDRESS(wiced_ip) >> 16), (uint8_t)(GET_IPV4_ADDRESS(wiced_ip) >> 8), (uint8_t)(GET_IPV4_ADDRESS(wiced_ip) >> 0) );
	return pDes;
}

UINT8 WXNetwork_ClearSCList(UINT8 scid)
{
	if ( scid >= WX_MAX_SCID_RANGE )	return WXCODE_EBADCID;
	if ( g_scList[scid].pSocket==0 )	return WXCODE_EBADCID;

	if ( g_scList[scid].dataMode == 1 )
	{
		g_isAutoconnected = 0;
		WXS2w_LEDIndication(2, 0, 0, 0, 0, 0);
	}

	if ( g_scList[scid].pTLSContext )
	{
		free(g_scList[scid].pTLSContext);
		g_scList[scid].pTLSContext = 0;
	}

	memset(&g_scList[scid], 0, sizeof(g_scList[scid]));

	WXS2w_StatusNotify(WXCODE_ECIDCLOSE, scid);

	return WXCODE_SUCCESS;
}

UINT8 WXNetwork_CloseSCList(UINT8 scid)
{
	wiced_result_t result;

	if ( scid >= WX_MAX_SCID_RANGE )		return WXCODE_EBADCID;
	if ( g_scList[scid].pSocket==0 )		return WXCODE_EBADCID;

	if ( g_scList[scid].conType==WX_SC_CONTYPE_TCP )
	{
	    if ( (result=wiced_tcp_disconnect(g_scList[scid].pSocket))!=WICED_SUCCESS )
	    {

	    	if ( g_scList[scid].conMode==WX_SC_MODE_CLIENT )
	    	{
	    		W_DBG("WXNetwork_CloseSCList : error 110 (%d)", result);
	    	}
	    }
	    if ( (result=wiced_tcp_delete_socket(g_scList[scid].pSocket))!=WICED_SUCCESS )
	    {
	    	W_DBG("WXNetwork_CloseSCList : error 120 (%d)", result);
	    }
	}
	else
	{
	    if ( (result=wiced_udp_delete_socket(g_scList[scid].pSocket))!=WICED_SUCCESS )
	    {
	    	W_DBG("WXNetwork_CloseSCList : error 130 (%d)", result);
	    }
	}

	return WXCODE_SUCCESS;
}

UINT8 WXNetwork_CloseSCAllList()
{
	UINT8 status = WXCODE_SUCCESS;
	INT32 i;

	for(i = 0; i < WX_MAX_SCID_RANGE; i++)
	{
		if ( g_scList[i].pSocket )
		{

			status = WXNetwork_CloseSCList(i);

			// sekim 20130816 ID1114, ID1124 add delay() in successive socket process
			wiced_rtos_delay_milliseconds(100);

			// sekim XXXX 20160119 Add socket_ext_option9 for TCP Server Multi-Connection
			if ( g_wxProfile.socket_ext_option9!=0 )
			{
				//wiced_rtos_lock_mutex(&g_socketopen_wizmutexx);
				wiced_rtos_delay_milliseconds(1100);
				//wiced_rtos_unlock_mutex(&g_socketopen_wizmutexx);
			}
		}
	}
	return status;
}

UINT8 WXNetwork_NetIsCidOpen(UINT8 scid)
{
	return !( scid >= WX_MAX_SCID_RANGE || g_scList[scid].pSocket==0 );
}

UINT8 WXNetwork_ScidGet(VOID)
{
	UINT32 i;
	for(i = 0; i < WX_MAX_SCID_RANGE; i++)
	{
		if ( g_scList[i].pSocket==0 )	return i;
	}
	return WX_INVALID_SCID;
}

#if 0 //MikeJ 130806 ID1112 - Target IP/Port shouldn't be changed when packet was received on UDP Client
UINT8 WXNetwork_NetRx(UINT8 scid, VOID *buf, UINT32 len)
#else
UINT8 WXNetwork_NetRx(UINT8 scid, VOID *buf, UINT32 len, wiced_ip_address_t* rmtIP, UINT16* rmtPort)
#endif
{
	UINT8 *p = (UINT8 *) buf;

	char szUartBuff[256] = { 0, };
	char xrdfbuff[30] =	{ 0, };

	// sekim 20140625 ID1176 add option to clear tcp-idle-connection
	g_scList[scid].tcp_time_lastdata = host_rtos_get_time();

	// sekim 20140929 ID1188 Data-Idle-Auto-Reset (Autonix)
	g_time_lastdata = g_scList[scid].tcp_time_lastdata;

	if ( g_wxModeState == WX_MODE_DATA )
	{
		WXHal_CharNPut(p, len);
		return WXCODE_SUCCESS;
	}

	// sekim 20120405 Output <Received Data> {1,192.168.1.23,5000,1}0123456789
	if ( g_wxProfile.xrdf_main[0] == '1' )	{ sprintf(szUartBuff, "%c", g_wxProfile.xrdf_data[0]); }
	if ( g_wxProfile.xrdf_main[1] == '1' )	{ sprintf(xrdfbuff, "%x", scid); strcat(szUartBuff, xrdfbuff); }
	if ( g_wxProfile.xrdf_main[2] == '1' )	{ sprintf(xrdfbuff, "%c", g_wxProfile.xrdf_data[1]); strcat(szUartBuff, xrdfbuff); }
#if 0 //MikeJ 130806 ID1112 - Target IP/Port shouldn't be changed when packet was received on UDP Client
	if ( g_wxProfile.xrdf_main[3] == '1' )	{ sprintf(xrdfbuff, "%s", (char*)WXNetwork_WicedV4IPToString(0, g_scList[scid].remoteIp)); strcat(szUartBuff, xrdfbuff); }
	if ( g_wxProfile.xrdf_main[2] == '1' )	{ sprintf(xrdfbuff, "%c", g_wxProfile.xrdf_data[1]); strcat(szUartBuff, xrdfbuff);	}
	if ( g_wxProfile.xrdf_main[4] == '1' )	{ sprintf(xrdfbuff, "%d", g_scList[scid].remotePort); strcat(szUartBuff, xrdfbuff);	}
#else
	if ( g_wxProfile.xrdf_main[3] == '1' )	{
		if(rmtIP != NULL)
			sprintf(xrdfbuff, "%s", (char*)WXNetwork_WicedV4IPToString(0, *rmtIP));
		else sprintf(xrdfbuff, "%s", (char*)WXNetwork_WicedV4IPToString(0, g_scList[scid].remoteIp));
		strcat(szUartBuff, xrdfbuff); 
	}
	if ( g_wxProfile.xrdf_main[2] == '1' )	{ sprintf(xrdfbuff, "%c", g_wxProfile.xrdf_data[1]); strcat(szUartBuff, xrdfbuff);	}
	if ( g_wxProfile.xrdf_main[4] == '1' )	{
		if(rmtPort != NULL) 
			sprintf(xrdfbuff, "%d", *rmtPort); 
		else sprintf(xrdfbuff, "%d", g_scList[scid].remotePort); 
		strcat(szUartBuff, xrdfbuff);
	}
#endif
	if ( g_wxProfile.xrdf_main[2] == '1' )	{ sprintf(xrdfbuff, "%c", g_wxProfile.xrdf_data[1]); strcat(szUartBuff, xrdfbuff);	}
	if ( g_wxProfile.xrdf_main[5] == '1' )	{ sprintf(xrdfbuff, "%d", len); strcat(szUartBuff, xrdfbuff);	}
	if ( g_wxProfile.xrdf_main[6] == '1' )	{ sprintf(xrdfbuff, "%c", g_wxProfile.xrdf_data[2]); strcat(szUartBuff, xrdfbuff);	}

	extern wiced_mutex_t g_upart_type1_wizmutex;
	wiced_rtos_lock_mutex(&g_upart_type1_wizmutex);

	WXHal_CharNPut(szUartBuff, strlen(szUartBuff));
	WXHal_CharNPut(p, len);

	if ( g_wxProfile.xrdf_main[7] == '1' )	{ sprintf(szUartBuff, "%c", g_wxProfile.xrdf_data[3]); }
	if ( g_wxProfile.xrdf_main[8] == '1' )	{ sprintf(xrdfbuff, "%c", g_wxProfile.xrdf_data[4]); strcat(szUartBuff, xrdfbuff);	}
	WXHal_CharNPut(szUartBuff, strlen(szUartBuff));

	wiced_rtos_unlock_mutex(&g_upart_type1_wizmutex);
	return WXCODE_SUCCESS;
}

UINT8 WXNetwork_NetTx(UINT8 scid, VOID *buf, UINT32 len)
{
	UINT32 status = WXCODE_SUCCESS;
	wiced_result_t result;
	wiced_packet_t* transmit_packet;
	char* tx_data;

	uint16_t available_data_length;

	if ( scid >= WX_MAX_SCID_RANGE )	return WXCODE_EBADCID;
	if ( g_scList[scid].pSocket==0 )	return WXCODE_EBADCID;

	// sekim 20140625 ID1176 add option to clear tcp-idle-connection
	g_scList[scid].tcp_time_lastdata = host_rtos_get_time();
	// sekim 20140929 ID1188 Data-Idle-Auto-Reset (Autonix)
	g_time_lastdata = g_scList[scid].tcp_time_lastdata;

	if ( g_scList[scid].conType==WX_SC_CONTYPE_TCP )
	{
		// if TCP/Server and not yet accept, then drop the data
		if ( g_scList[scid].conMode==WX_SC_MODE_SERVER && g_scList[scid].bAccepted==0 )
		{
			return WXCODE_SUCCESS;
		}

		result = wiced_tcp_send_buffer(g_scList[scid].pSocket, buf, len);


		if ( result!=WICED_SUCCESS )
		{
			W_DBG("WXNetwork_NetTx : wiced_tcp_send_buffer error (%d) %s:%d/%d", result, WXNetwork_WicedV4IPToString(0, g_scList[scid].remoteIp), g_scList[scid].remotePort, g_scList[scid].localPort);
			// sekim 20131212 ID1141 add g_socket_extx_option
			//status = WXCODE_FAILURE;
			status = WXCODE_TCPSENDERROR;
		}
	}
	else if ( g_scList[scid].conType==WX_SC_CONTYPE_UDP )
	{
	    if (wiced_packet_create_udp(g_scList[scid].pSocket, len, &transmit_packet, (uint8_t**)&tx_data, &available_data_length) != WICED_SUCCESS)
	    {
	    	W_DBG("UDP tx packet creation failed");
	        return WXCODE_FAILURE;
	    }

	    memcpy(tx_data, buf, len);

	    wiced_packet_set_data_end(transmit_packet, (uint8_t*)tx_data + len);

	    if ( wiced_udp_send(g_scList[scid].pSocket, &g_scList[scid].remoteIp, g_scList[scid].remotePort, transmit_packet)!=WICED_SUCCESS )
	    {
	    	W_DBG("UDP tx packet failed");
	        wiced_packet_delete(transmit_packet);
	        return WXCODE_FAILURE;
	    }
	}
	else
	{
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
WT_SCLIST g_SocketOpenSCInfo;
UINT8 g_SocketOpenSCList;
extern wiced_mutex_t g_socketopen_wizmutex;

void ClearSocketOpenThread(UINT8 scid)
{
	struct WizFiQueue buffMessage;

	buffMessage.queue_id = 111;
	buffMessage.queue_opt = scid;

	extern wiced_queue_t g_queue_handle_wizfiqueue;
	wiced_rtos_push_to_queue(&g_queue_handle_wizfiqueue, (void*)&buffMessage, WICED_NO_WAIT);
}

void SocketOpenProcess(uint32_t arguments)
{
    wiced_result_t result;
	wiced_packet_t* rx_packet;
	wiced_tcp_socket_t socket_tcp;
	wiced_udp_socket_t socket_udp;

    char* rx_data;
	uint16_t rx_data_length;
	uint16_t available_data_length;
	uint8_t tcp_restart_check = 0;
	uint8_t command_mode_when_disconnected = 0;

	UINT8 scid = g_SocketOpenSCList;
	WT_SCLIST buff_SocketOpenSCInfo;
	memcpy(&buff_SocketOpenSCInfo, &g_SocketOpenSCInfo, sizeof(g_SocketOpenSCInfo));

	////////////////////////////////////////////////////////////////////////////////////////////////////////
	// sekim 20150616 add AT+SDNAME
	if ( strcmp(WXNetwork_WicedV4IPToString(0, buff_SocketOpenSCInfo.remoteIp), "0.0.0.0")==0 )
	{
		if ( strlen(g_wxProfile.domainname_for_scon)>3 )
		{
			wiced_ip_address_t buff_ip_address;
			if ( wiced_hostname_lookup((char*)g_wxProfile.domainname_for_scon, &buff_ip_address, 5000 )==WICED_SUCCESS )
			{
				memcpy(&buff_SocketOpenSCInfo.remoteIp, &buff_ip_address, sizeof(wiced_ip_address_t));
			}
			else
			{
				W_DBG("SocketProcess : wiced_hostname_lookup failed (%s)", g_wxProfile.domainname_for_scon);
			}
		}
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////////


	wiced_rtos_unlock_mutex(&g_socketopen_wizmutex);

	while(1)
	{
		tcp_restart_check = 0;
		UINT8 bTCP = (buff_SocketOpenSCInfo.conType==WX_SC_CONTYPE_TCP)?1:0;
		UINT8 bServer = (buff_SocketOpenSCInfo.conMode==WX_SC_MODE_SERVER)?1:0;

		//W_DBG("SocketOpenProcess : Start %s:%d/%d", WXNetwork_WicedV4IPToString(0, g_scList[scid].remoteIp), g_scList[scid].remotePort, g_scList[scid].localPort);

		//////////////////////////////////////////////////////////////////////////////////////////
		// sekim 20130404 AP? STA? 에 따른 수정
		// Create Socket
		//if ( bTCP )	result = wiced_tcp_create_socket(&socket_tcp, WICED_STA_INTERFACE);
		//else		result = wiced_udp_create_socket(&socket_udp, buff_SocketOpenSCInfo.localPort, WICED_STA_INTERFACE);
		wiced_interface_t interface = (g_wxProfile.wifi_mode==AP_MODE)?WICED_AP_INTERFACE:WICED_STA_INTERFACE;
		if ( bTCP )	result = wiced_tcp_create_socket(&socket_tcp, interface);
#if 0 //MikeJ 130624 ID1084 - Modified local-port checking part
		else		result = wiced_udp_create_socket(&socket_udp, buff_SocketOpenSCInfo.localPort, interface);
#else
		else {
			result = wiced_udp_create_socket(&socket_udp, buff_SocketOpenSCInfo.localPort, interface);
			g_scList[scid].localPort = socket_udp.socket.nx_udp_socket_port;
		}
#endif
		//////////////////////////////////////////////////////////////////////////////////////////

		if ( result!=WICED_SUCCESS )
		{
			W_DBG("SocketProcess : socket creation failed (%d)", result);
			// sekim 20140526 WXNetwork_ClearSCList if wiced_tcp_create_socket
			WXNetwork_ClearSCList(scid);
			ClearSocketOpenThread(scid);
			return;
		}

		g_scList[scid].pSocket = (bTCP)?(void*)(&socket_tcp):(void*)(&socket_udp);

		g_scList[scid].conType = buff_SocketOpenSCInfo.conType;
		g_scList[scid].conMode = buff_SocketOpenSCInfo.conMode;
		g_scList[scid].dataMode = buff_SocketOpenSCInfo.dataMode;
		g_scList[scid].tlsMode = buff_SocketOpenSCInfo.tlsMode;
#if 0 //MikeJ 130624 ID1084 - Modified local-port checking part
		g_scList[scid].localPort = buff_SocketOpenSCInfo.localPort;
#else
		if(bTCP) g_scList[scid].localPort = buff_SocketOpenSCInfo.localPort;
#endif
		g_scList[scid].remotePort = buff_SocketOpenSCInfo.remotePort;
		memcpy(&g_scList[scid].remoteIp, &buff_SocketOpenSCInfo.remoteIp, sizeof(buff_SocketOpenSCInfo.remoteIp));

		// sekim 201506 After connected, switch into data mode
		/*
		if ( buff_SocketOpenSCInfo.dataMode==1 )
		{
			g_isAutoconnected = 1;
			g_currentScid = scid;

			// sekim 20130801 TCP Serve/Data mode -> Command Mode -> Disconnect -> To be Command mode
			if ( command_mode_when_disconnected==0 )
			{
				g_wxModeState = WX_MODE_DATA;
				WXS2w_LEDIndication(2, 0, 0, 0, 0, 1);
			}
		}
		*/

		if ( bTCP )
		{
			if ( bServer )
			{
				if ( wiced_tcp_listen(&socket_tcp, g_scList[scid].localPort)!=WICED_SUCCESS )
				{
					W_DBG("SocketOpenProcess : listen error %d, %d", scid, g_scList[scid].localPort);
					wiced_tcp_delete_socket(&socket_tcp);
					WXNetwork_ClearSCList(scid);
					ClearSocketOpenThread(scid);
					return;
				}

				if ( g_scList[scid].tlsMode=='S' )
				{
					// sekim 20130311 2.2.1 Migration, about TLS
					g_scList[scid].pTLSContext = malloc_named("TLS-Context", sizeof(wiced_tls_advanced_context_t));
					memset(g_scList[scid].pTLSContext, 0, sizeof(wiced_tls_advanced_context_t));
					((wiced_tls_advanced_context_t*)g_scList[scid].pTLSContext)->context_type = WICED_TLS_ADVANCED_CONTEXT;

#if 1 // kaizen	20130520 ID1068 - Modified for using dct_security_section
					platform_dct_security_t const* dct_security = wiced_dct_get_security_section( );
					if ((result=wiced_tls_init_advanced_context(g_scList[scid].pTLSContext, dct_security->certificate, dct_security->private_key ))!=WICED_SUCCESS )
						W_DBG("SocketProcess : wiced_tls_init_context failed (%d)", result);
					if ( (result=wiced_tcp_enable_tls(&socket_tcp, g_scList[scid].pTLSContext))!=WICED_SUCCESS )
						W_DBG("SocketProcess : wiced_tcp_enable_tls failed (%d)", result);

#else
					if ( (result=wiced_tls_init_advanced_context(g_scList[scid].pTLSContext, brcm_server_certificate, brcm_server_rsa_key))!=WICED_SUCCESS )
						W_DBG("SocketProcess : wiced_tls_init_context failed (%d)", result);
					if ( (result=wiced_tcp_enable_tls(&socket_tcp, g_scList[scid].pTLSContext))!=WICED_SUCCESS )
						W_DBG("SocketProcess : wiced_tcp_enable_tls failed (%d)", result);
#endif
				}

				// kaizen 20140529
				if ( g_wxProfile.enable_listen_msg )
					WXS2w_StatusNotify(WXCODE_LISTEN, scid);

				while (1)
				{
					wiced_result_t result = wiced_tcp_accept(&socket_tcp);

					if ( result==WICED_TIMEOUT )	continue;
					else
					{
						if ( result!=WICED_SUCCESS )
						{
							W_DBG("SocketOpenProcess : accept error %d, %d, %d", result, scid, g_scList[scid].localPort);
							wiced_tcp_delete_socket(&socket_tcp);
							WXNetwork_ClearSCList(scid);
							ClearSocketOpenThread(scid);
							return;
						}
						else
						{
							break;
						}
					}
				}

				g_scList[scid].bAccepted = 1;

				/////////////////////////////////////////////////////////////////////////////////
				// sekim 20130131 Accepted Client Information(IP, Port) Using NetX API
				ULONG peer_ip_address = 0;
				ULONG peer_port = 0;
				if ( NX_SUCCESS!=nx_tcp_socket_peer_info_get(&socket_tcp.socket, &peer_ip_address, &peer_port) )
				{
					W_DBG("SocketProcess : nx_tcp_socket_peer_info_get error");
				}
				else
				{
					SET_IPV4_ADDRESS(g_scList[scid].remoteIp, peer_ip_address);
					g_scList[scid].remotePort = peer_port;
				}
				/////////////////////////////////////////////////////////////////////////////////
			}
			else
			{
				if ( g_scList[scid].tlsMode=='S' )
				{
#if 1				// kaizen 20140428 ID1088 Modified to connect to TCP Server using exchanging certificate.
					g_scList[scid].pTLSContext = malloc_named("TLS-Context", sizeof(wiced_tls_advanced_context_t));
					memset(g_scList[scid].pTLSContext, 0, sizeof(wiced_tls_advanced_context_t));
					((wiced_tls_advanced_context_t*)g_scList[scid].pTLSContext)->context_type = WICED_TLS_ADVANCED_CONTEXT;

					platform_dct_security_t const* dct_security = wiced_dct_get_security_section( );

					if ((result=wiced_tls_init_advanced_context(g_scList[scid].pTLSContext, dct_security->certificate, dct_security->private_key ))!=WICED_SUCCESS )
						W_DBG("SocketProcess : wiced_tls_init_context failed (%d)", result);
					if ( (result=wiced_tcp_enable_tls(&socket_tcp, g_scList[scid].pTLSContext))!=WICED_SUCCESS )
						W_DBG("SocketProcess : wiced_tcp_enable_tls failed (%d)", result);
#else
					// sekim 20130311 2.2.1 Migration, about TLS
					g_scList[scid].pTLSContext = malloc_named("TLS-Context", sizeof(wiced_tls_simple_context_t));
					memset(g_scList[scid].pTLSContext, 0, sizeof(wiced_tls_simple_context_t));
					((wiced_tls_simple_context_t*)g_scList[scid].pTLSContext)->context_type = WICED_TLS_SIMPLE_CONTEXT;
					if ( (result=wiced_tls_init_simple_context(g_scList[scid].pTLSContext))!=WICED_SUCCESS )
						W_DBG("SocketProcess : wiced_tls_init_context failed (%d)", result);
					if ( (result=wiced_tcp_enable_tls(&socket_tcp, g_scList[scid].pTLSContext))!=WICED_SUCCESS )
						W_DBG("SocketProcess : wiced_tcp_enable_tls failed (%d)", result);
#endif
				}

#if 0 //MikeJ 130624 ID1084 - Modified local-port checking part
				wiced_tcp_bind(&socket_tcp, WICED_ANY_PORT);
				g_scList[scid].localPort = 0;
#else
				////////////////////////////////////////////////////////////////////////////////
				// sekim 20150416 TCP client random local port
				static uint16_t tcp_client_port_add = 0;
			    if ( tcp_client_port_add==0 )
			    {
			    	uint16_t random_value = 0;
			    	extern wiced_result_t wiced_wifi_get_random( uint16_t* val );
			    	wiced_wifi_get_random(&random_value);
			    	tcp_client_port_add = random_value%10000 + 1;
			    }
				if ( g_scList[scid].localPort==0 )
				{
					g_scList[scid].localPort = NX_SEARCH_PORT_START + tcp_client_port_add++;
				}
				////////////////////////////////////////////////////////////////////////////////


				wiced_tcp_bind(&socket_tcp, g_scList[scid].localPort);
				g_scList[scid].localPort = socket_tcp.socket.nx_tcp_socket_port;
#endif

				result = wiced_tcp_connect(&socket_tcp, &g_scList[scid].remoteIp, g_scList[scid].remotePort, TCP_CLIENT_CONNECT_TIMEOUT);
				if ( result!=WICED_SUCCESS )
				{
					W_DBG("SocketOpenProcess : connect error %s, %d", WXNetwork_WicedV4IPToString(0, g_scList[scid].remoteIp), g_scList[scid].localPort);
					wiced_tcp_delete_socket(&socket_tcp);
					WXNetwork_ClearSCList(scid);
					ClearSocketOpenThread(scid);
					return;
				}
				//W_DBG("SocketOpenProcess : connect OK %s, %d", WXNetwork_WicedV4IPToString(0, g_scList[scid].remoteIp), g_scList[scid].localPort);
			}

			// sekim 201506 After connected, switch into data mode
			if ( buff_SocketOpenSCInfo.dataMode==1 )
			{
				g_isAutoconnected = 1;
				g_currentScid = scid;

				// sekim 20130801 TCP Serve/Data mode -> Command Mode -> Disconnect -> To be Command mode
				if ( command_mode_when_disconnected==0 )
				{
					g_wxModeState = WX_MODE_DATA;
					WXS2w_LEDIndication(2, 0, 0, 0, 0, 1);
				}
			}

			WXS2w_StatusNotify(WXCODE_CON_SUCCESS, scid);

			// sekim 20140625 ID1176 add option to clear tcp-idle-connection
			g_scList[scid].tcp_time_lastdata = host_rtos_get_time();
			// sekim 20140929 ID1188 Data-Idle-Auto-Reset (Autonix)
			g_time_lastdata = g_scList[scid].tcp_time_lastdata;

			while(1)
			{
				result = wiced_tcp_receive(&socket_tcp, &rx_packet, WICED_WAIT_FOREVER);

				//W_DBG("SocketOpenProcess : wiced_tcp_receive %d, %d", result, socket_tcp.socket.nx_tcp_socket_state);

				if ( result!=WICED_SUCCESS )
				{
					// sekim wiced_tcp_receive return value processing
					if ( result==WICED_PENDING )
					{
						/*
						if ( socket_tcp.socket.nx_tcp_socket_state!=NX_TCP_CLOSED &&
							 socket_tcp.socket.nx_tcp_socket_state!=NX_TCP_FIN_WAIT_1 &&
							 socket_tcp.socket.nx_tcp_socket_state!=NX_TCP_FIN_WAIT_2 )
							 */
						if ( socket_tcp.socket.nx_tcp_socket_state==NX_TCP_ESTABLISHED )
							continue;
					}

					//W_DBG("SocketOpenProcess : Disconnect (%d, %d) %s, %d", result, socket_tcp.socket.nx_tcp_socket_state, WXNetwork_WicedV4IPToString(0, g_scList[scid].remoteIp), g_scList[scid].localPort);

					////////////////////////////////////////////////////////////////////////////////////
					// sekim 20140625 ID1176 add option to clear tcp-idle-connection
					/*
					////////////////////////////////////////////////////////////////////////////////////
					// sekim 20130411 tcp_restart_check
					tcp_restart_check = 1;
#if 1 // sekim 20130416 ID1050
					if ( !bServer || (result==2 && socket_tcp.socket.nx_tcp_socket_state==7) )
#else
					if ( result==2 && socket_tcp.socket.nx_tcp_socket_state==7 )
#endif
					{
						tcp_restart_check = 0;
					}
					////////////////////////////////////////////////////////////////////////////////////
					 */
					if ( bServer )	tcp_restart_check = 1;
					else 			tcp_restart_check = 0;
					if ( g_scList[scid].notRestartTCPServer )	tcp_restart_check = 0;
					////////////////////////////////////////////////////////////////////////////////////

					if ( g_wxModeState==WX_MODE_COMMAND )	command_mode_when_disconnected = 1;
					else									command_mode_when_disconnected = 0;

					wiced_packet_delete(rx_packet);
					wiced_tcp_disconnect(&socket_tcp);
					wiced_tcp_delete_socket(&socket_tcp);
					WXNetwork_ClearSCList(scid);
					break;
				}

				wiced_packet_get_data(rx_packet, 0, (uint8_t**)&rx_data, &rx_data_length, &available_data_length);
#if 0 //MikeJ 130806 ID1112 - Target IP/Port shouldn't be changed when packet was received on UDP Client
				WXNetwork_NetRx(scid, rx_data, rx_data_length);
#else
				WXNetwork_NetRx(scid, rx_data, rx_data_length, NULL, NULL);
#endif
				wiced_packet_delete(rx_packet);
			}
		}
		else
		{
#if 1 //MikeJ 130806 ID1112 - Target IP/Port shouldn't be changed when packet was received on UDP Client
			wiced_ip_address_t rmtIp;
			UINT16 rmtPort;
#endif

			// daniel 20160630 After connected, switch into data mode in UDP mode
			if ( buff_SocketOpenSCInfo.dataMode==1 )
			{
				g_isAutoconnected = 1;
				g_currentScid = scid;

				// sekim 20130801 TCP Serve/Data mode -> Command Mode -> Disconnect -> To be Command mode
				if ( command_mode_when_disconnected==0 )
				{
					g_wxModeState = WX_MODE_DATA;
					WXS2w_LEDIndication(2, 0, 0, 0, 0, 1);
				}
			}
			/////

			WXS2w_StatusNotify(WXCODE_CON_SUCCESS, scid);

			while(1)
			{
				result = wiced_udp_receive(&socket_udp, &rx_packet, WICED_WAIT_FOREVER);
				if ( result!=WICED_SUCCESS )
				{
					//W_DBG("SocketOpenProcess : Disconnect (%d) %s, %d", result, WXNetwork_WicedV4IPToString(0, g_scList[scid].remoteIp), g_scList[scid].localPort);

					wiced_packet_delete(rx_packet);
					wiced_udp_delete_socket(&socket_udp);
					WXNetwork_ClearSCList(scid);
					break;
				}

#if 0 //MikeJ 130806 ID1112 - Target IP/Port shouldn't be changed when packet was received on UDP Client
				wiced_udp_packet_get_info(rx_packet, &g_scList[scid].remoteIp, &g_scList[scid].remotePort);

				wiced_packet_get_data(rx_packet, 0, (uint8_t**)&rx_data, &rx_data_length, &available_data_length);
				WXNetwork_NetRx(scid, rx_data, rx_data_length);
				wiced_packet_delete(rx_packet);
#else
				wiced_packet_get_data(rx_packet, 0, (uint8_t**)&rx_data, &rx_data_length, &available_data_length);
				if ( bServer ) {
					wiced_udp_packet_get_info(rx_packet, &g_scList[scid].remoteIp, &g_scList[scid].remotePort);
					WXNetwork_NetRx(scid, rx_data, rx_data_length, NULL, NULL);
				} else {
					wiced_udp_packet_get_info(rx_packet, &rmtIp, &rmtPort);
					WXNetwork_NetRx(scid, rx_data, rx_data_length, &rmtIp, &rmtPort);
				}
				wiced_packet_delete(rx_packet);
#endif
			}
		}

		if ( tcp_restart_check==0 )		break;

		///////////////////////////////////////////////////////////////////////////////////////////
		// sekim 20130709 ID1089 if Socket Close by AT Command, don't restart <TCP Server>
		if ( g_scList[scid].notRestartTCPServer==1 )
		{
			g_scList[scid].notRestartTCPServer = 0;
			break;
		}
		///////////////////////////////////////////////////////////////////////////////////////////

		// sekim XXXX 20160119 Add socket_ext_option9 for TCP Server Multi-Connection
		if ( bTCP & bServer )
		{
			if ( g_wxProfile.socket_ext_option9==1 )		break;
		}

	}

	ClearSocketOpenThread(scid);

	return;
}

UINT16 g_socket_open_thread_stack_xxx = 4;
void LaunchSocketOpen(UINT8 scid, UINT8 bTCP, UINT8 bServer, UINT8 tlsMode, wiced_ip_address_t remote_ip, UINT16 remote_port, UINT16 local_port, UINT8 bDataMode)
{
	wiced_rtos_lock_mutex(&g_socketopen_wizmutex);

	g_SocketOpenSCList = scid;

	g_SocketOpenSCInfo.conType = (bTCP)?WX_SC_CONTYPE_TCP:WX_SC_CONTYPE_UDP;
	g_SocketOpenSCInfo.conMode = (bServer)?WX_SC_MODE_SERVER:WX_SC_MODE_CLIENT;
	g_SocketOpenSCInfo.dataMode = bDataMode;
	g_SocketOpenSCInfo.tlsMode = tlsMode;
	g_SocketOpenSCInfo.localPort = local_port;
	g_SocketOpenSCInfo.remotePort = remote_port;
	memcpy(&g_SocketOpenSCInfo.remoteIp, &remote_ip, sizeof(remote_ip));

	wiced_result_t result;
	char szThread[20];
	sprintf(szThread, "Socket-%d", scid);
	// sekim 20130208 SocketOpen Thread Optimizing (g_socket_open_thread_stack_xxx)
	if ( (result=wiced_rtos_create_thread((wiced_thread_t*)&g_socketopen_thread[scid], WICED_APPLICATION_PRIORITY + 3, szThread, SocketOpenProcess, 1024*g_socket_open_thread_stack_xxx, NULL))!=WICED_SUCCESS )
	{
		W_DBG("wiced_rtos_create_thread : SocketOpenProcess error %d (rebooting?)", result);
	}

	// sekim 20130816 ID1114, ID1124 add delay() in successive socket process
	wiced_rtos_delay_milliseconds(100);
}

// sekim XXXX 20160119 Add socket_ext_option9 for TCP Server Multi-Connection
UINT16 g_last_TSN_localport = 0;

// sekim 20140625 ID1176 add option to clear tcp-idle-connection
void check_tcp_idle_time()
{
	uint32_t index_scid;

	// sekim 20140929 ID1188 Data-Idle-Auto-Reset (Autonix)
	if ( g_wxProfile.socket_ext_option5!=0 )
	{
		for(index_scid = 0; index_scid < WX_MAX_SCID_RANGE; index_scid++)
		{
			if ( g_scList[index_scid].pSocket==0 )					continue;
			if ( g_scList[index_scid].conType!=WX_SC_CONTYPE_TCP )	continue;
			if ( g_scList[index_scid].remotePort==0 )				continue;

			uint32_t diff_seconds = 0;
			uint32_t current_time = host_rtos_get_time();
			diff_seconds = current_time - g_scList[index_scid].tcp_time_lastdata;

			if ( (diff_seconds/1000)>=g_wxProfile.socket_ext_option5 )
			{
				//WXNetwork_CloseSCList(index_scid);
				wiced_tcp_disconnect((wiced_tcp_socket_t*)g_scList[index_scid].pSocket);
			}
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////
	// sekim 20140929 ID1188 Data-Idle-Auto-Reset (Autonix)
	if ( g_wxProfile.socket_ext_option7!=0 )
	{
		uint32_t diff_seconds = 0;
		uint32_t current_time = host_rtos_get_time();
		diff_seconds = current_time - g_time_lastdata;

		if ( (diff_seconds/1000)>=g_wxProfile.socket_ext_option7 )
		{
			WXS2w_SystemReset();
		}
	}
	////////////////////////////////////////////////////////////////////////////////////////////////

	// sekim XXXX 20160119 Add socket_ext_option9 for TCP Server Multi-Connection
	if ( g_wxProfile.socket_ext_option9!=0 )
	{
		UINT8 scid;
		UINT8 found_listen_port = 0;

		for(scid=0; scid<WX_MAX_SCID_RANGE; scid++)
		{
			if ( g_scList[scid].localPort==g_last_TSN_localport )
			{
				if ( g_scList[scid].remotePort==0 )
					found_listen_port = 1;
			}

		}



		if ( (WXNetwork_ScidGet()!=WX_INVALID_SCID) && found_listen_port==0 && g_last_TSN_localport!=0 )
		{
			char szBuff[100];
			sprintf(szBuff, "O,TSN,,,%d,0", g_last_TSN_localport);
			WXCmd_SCON((UINT8*)szBuff);
		}



	}
}


