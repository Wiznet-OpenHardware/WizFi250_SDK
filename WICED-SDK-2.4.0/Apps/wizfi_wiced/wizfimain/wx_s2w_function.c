
#include "wx_defines.h"

// sekim g_upart_type1_wizmutex
extern wiced_mutex_t g_upart_type1_wizmutex;

//////////////////////////////////////////////////////////////////////////////////////////////////
// sekim 20140710 ID1180 add air command
uint8_t g_aircmd_status = 0;
uint32_t g_aircmd_rx_len = 0;
char* g_aircmd_tx_data;
char* g_aircmd_rx_data;

wiced_thread_t thread_handle_aircommand;
//////////////////////////////////////////////////////////////////////////////////////////////////

INT32 WXS2w_VPrintf(const char *format, va_list ap)
{
	static char buf[WX_MAX_PRINT_LEN];
	INT32 len;

	len = vsnprintf(buf, sizeof(buf), format, ap);
	if (len < 0)
		return -1;

	if (len >= sizeof(buf))
	{
		len = sizeof(buf) - 1;
	}

	/////////////////////////////////////////////////////////////////
	// sekim 20140710 ID1180 add air command
	// WXHal_CharNPut(buf, len);
	if ( g_aircmd_status==3 )
	{
		uint32_t loop_i;
		uint32_t aircmdtx_len;
		uint32_t buf_len = strlen(buf);
		for (loop_i=0; loop_i<buf_len; loop_i++)
		{
			aircmdtx_len = strlen(g_aircmd_tx_data);
			if ( aircmdtx_len<(WX_MAX_PACKET_SIZE-10) )
			{
				g_aircmd_tx_data[aircmdtx_len] = buf[loop_i];
				g_aircmd_tx_data[aircmdtx_len+1] = 0;
			}
			else if ( aircmdtx_len<(WX_MAX_PACKET_SIZE-1) )
			{
				g_aircmd_tx_data[aircmdtx_len] = '.';
				g_aircmd_tx_data[aircmdtx_len+1] = 0;
			}
			else
			{
				VOID W_DBG3(const char *format, ...);
				//W_DBG3("g_aircmd_tx_data overflow\r\n");
				W_DBG3(".");
			}
		}
	}
	else
	{
		WXHal_CharNPut(buf, len);
	}
	/////////////////////////////////////////////////////////////////

	return len;
}

// msgLevel ==> 1:Response 2:Event 3:Debug
VOID W_RSP(const char *format, ...)
{
	if ( 1>g_wxProfile.msgLevel )	return;

	wiced_rtos_lock_mutex(&g_upart_type1_wizmutex);
	va_list args;

	va_start(args, format);
	WXS2w_VPrintf(format, args);
	va_end(args);
	wiced_rtos_unlock_mutex(&g_upart_type1_wizmutex);
}

VOID W_EVT(const char *format, ...)
{
	if ( 2>g_wxProfile.msgLevel )	return;

	wiced_rtos_lock_mutex(&g_upart_type1_wizmutex);
	va_list args;

	va_start(args, format);
	WXS2w_VPrintf(format, args);
	va_end(args);
	wiced_rtos_unlock_mutex(&g_upart_type1_wizmutex);
}

VOID W_DBG(const char *format, ...)
{
	if ( 3>g_wxProfile.msgLevel )	return;

	wiced_rtos_lock_mutex(&g_upart_type1_wizmutex);
	va_list args;

	va_start(args, format);
	WXS2w_VPrintf(format, args);
	va_end(args);

	W_RSP("\r\n");
	wiced_rtos_unlock_mutex(&g_upart_type1_wizmutex);
}

VOID W_DBG2(const char *format, ...)
{
	if ( 3>g_wxProfile.msgLevel )	return;

	wiced_rtos_lock_mutex(&g_upart_type1_wizmutex);
	va_list args;

	va_start(args, format);
	WXS2w_VPrintf(format, args);
	va_end(args);
	wiced_rtos_unlock_mutex(&g_upart_type1_wizmutex);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// sekim 20140710 ID1180 add air command
VOID W_DBG3(const char *format, ...)
{
	if ( 3>g_wxProfile.msgLevel )	return;

	wiced_rtos_lock_mutex(&g_upart_type1_wizmutex);
	va_list args;

	va_start(args, format);
	{
		static char buf[100];
		uint32_t len;

		len = vsnprintf(buf, sizeof(buf), format, args);
		if (len >= 0)
		{
			if (len >= sizeof(buf))
			{
				len = sizeof(buf) - 1;
			}
			WXHal_CharNPut(buf, len);
		}
	}

	va_end(args);
	wiced_rtos_unlock_mutex(&g_upart_type1_wizmutex);
}

void AirCommandDaemonProcess(uint32_t arguments)
{
    wiced_result_t result;
	wiced_packet_t* rx_packet;
	wiced_packet_t* transmit_packet;
	wiced_udp_socket_t socket_udp;
	wiced_tcp_socket_t socket_tcp;

    char* rx_data;
    char* tx_data;
	uint16_t rx_data_length;
	uint16_t available_data_length;

	uint32_t tx_len;
	uint32_t buff_i;
	wiced_ip_address_t rmtIp;
	uint16_t rmtPort;
	uint8_t bTCP = (g_wxProfile.aircmd_mode=='T')?1:0;

	wiced_interface_t interface = (g_wxProfile.wifi_mode==AP_MODE)?WICED_AP_INTERFACE:WICED_STA_INTERFACE;

	// sekim 20140805 Continuous AirCmd Service
	while(1)
	{
		if ( g_wxProfile.aircmd_mode=='T' )
			result = wiced_tcp_create_socket(&socket_tcp, interface);
		else
			result = wiced_udp_create_socket(&socket_udp, g_wxProfile.aircmd_port, interface);

		W_DBG3("AirCmdDaemonProcess : start(%s,%c,%d,%02x)\r\n", g_wxProfile.aircmd_opentype, g_wxProfile.aircmd_mode, g_wxProfile.aircmd_port, g_wxProfile.aircmd_opt);

		if ( result!=WICED_SUCCESS )
		{
			W_DBG3("AirCmdDaemonProcess : wiced_udp_create_socket failed (%d)\r\n", result);
			return;
		}

		g_aircmd_rx_data = (char*)malloc(WX_MAX_PACKET_SIZE);
		g_aircmd_tx_data = (char*)malloc(WX_MAX_PACKET_SIZE);

		if ( g_aircmd_rx_data==0 || g_aircmd_tx_data==0 )
		{
			W_DBG3("AirCmdDaemonProcess : memory allocation error\r\n");
			return;
		}

		if ( bTCP )
		{
			if ( wiced_tcp_listen(&socket_tcp, g_wxProfile.aircmd_port)!=WICED_SUCCESS )
			{
				W_DBG3("AirCmdDaemonProcess : listen error %d\r\n", g_wxProfile.aircmd_port);
				wiced_tcp_delete_socket(&socket_tcp);
				return;
			}
			while (1)
			{
				wiced_result_t result = wiced_tcp_accept(&socket_tcp);

				if ( result==WICED_TIMEOUT )	continue;
				else
				{
					if ( result!=WICED_SUCCESS )
					{
						W_DBG3("AirCmdDaemonProcess : accept error %d, %d\r\n", result, g_wxProfile.aircmd_port);
						wiced_tcp_delete_socket(&socket_tcp);
						return;
					}
					else		break;
				}
			}

			/////////////////////////////////////////////////////////////////////////////////
			ULONG peer_ip_address = 0;
			ULONG peer_port = 0;
			if ( NX_SUCCESS!=nx_tcp_socket_peer_info_get(&socket_tcp.socket, &peer_ip_address, &peer_port) )
			{
				W_DBG3("AirCmdDaemonProcess : nx_tcp_socket_peer_info_get error\r\n");
			}
			else
			{
				SET_IPV4_ADDRESS(rmtIp, peer_ip_address);
				rmtPort = peer_port;
				//W_DBG3("AirCmdDaemonProcess : Connected (%d) %s:%d", result, WXNetwork_WicedV4IPToString(0, rmtIp), rmtPort);
			}
			/////////////////////////////////////////////////////////////////////////////////
		}

		while(1)
		{
			g_aircmd_status = 1;

			if ( bTCP )
			{
				result = wiced_tcp_receive(&socket_tcp, &rx_packet, WICED_WAIT_FOREVER);
				if ( result!=WICED_SUCCESS )
				{
					if ( result==WICED_PENDING )
					{
						if ( socket_tcp.socket.nx_tcp_socket_state==NX_TCP_ESTABLISHED )
							continue;
					}
					wiced_packet_delete(rx_packet);
					break;
				}
				wiced_packet_get_data(rx_packet, 0, (uint8_t**)&rx_data, &rx_data_length, &available_data_length);
			}
			else
			{
				result = wiced_udp_receive(&socket_udp, &rx_packet, WICED_WAIT_FOREVER);
				if ( result!=WICED_SUCCESS )
				{
					W_DBG3("AirCmdDaemonProcess : wiced_udp_receive error (%d)\r\n", result);
					wiced_packet_delete(rx_packet);
					wiced_udp_delete_socket(&socket_udp);
					break;
				}
				wiced_packet_get_data(rx_packet, 0, (uint8_t**)&rx_data, &rx_data_length, &available_data_length);
				wiced_udp_packet_get_info(rx_packet, &rmtIp, &rmtPort);
			}

			// Check if WizFiAirCmd Packet? WizFi250AirCmd:
			if ( memcmp("WizFi250AirCmd:", rx_data, 15)!=0 || rx_data[rx_data_length-1]!=0x0d )
			{
				memcpy(g_aircmd_rx_data, rx_data, rx_data_length);
				g_aircmd_rx_data[rx_data_length] = 0;
				W_DBG3("AirCmdDaemonProcess : rx invalid data(%d) \r\n", rx_data_length);
				//W_DBG3("%s\r\n", g_aircmd_rx_data);
				wiced_packet_delete(rx_packet);
				continue;
			}

			memcpy(g_aircmd_rx_data, (rx_data+15), rx_data_length-15);
			g_aircmd_rx_data[rx_data_length-15] = 0;
			//W_DBG3("AirCmdDaemonProcess : rx \r\n");
			//W_DBG3("%s\r\n", g_aircmd_rx_data);

			// wait till processing the command
			memset(g_aircmd_tx_data, 0, sizeof(g_aircmd_tx_data));
			send_maincommand_queue(106, 0);
			g_aircmd_status = 2;

			for (buff_i=0; buff_i<100; buff_i++)
			{
				wiced_rtos_delay_milliseconds(100);
				//W_DBG3("<%d>", g_aircmd_status);
				if ( g_aircmd_status==4 )	break;
			}
			wiced_packet_delete(rx_packet);

			// maybe timeout
			if ( strlen(g_aircmd_tx_data)==0 )
			{
				W_DBG3("AirCmdDaemonProcess : g_aircmd_status error (%d)\r\n", strlen(g_aircmd_tx_data));
				//break;
			}

			if ( bTCP )
			{
				tx_len = strlen(g_aircmd_tx_data);
				result = wiced_tcp_send_buffer(&socket_tcp, g_aircmd_tx_data, tx_len);
				if ( result!=WICED_SUCCESS )
				{
					W_DBG3("AirCmdDaemonProcess : wiced_tcp_send_buffer error (%d) %d", result, tx_len);
					break;
				}
			}
			else
			{
				tx_len = strlen(g_aircmd_tx_data);
				if (wiced_packet_create_udp(&socket_udp, tx_len, &transmit_packet, (uint8_t**)&tx_data, &available_data_length) != WICED_SUCCESS)
				{
					W_DBG3("AirCmdDaemonProcess : wiced_packet_create_udp error (%d)\r\n", strlen(g_aircmd_tx_data));
					break;
				}
				memcpy(tx_data, g_aircmd_tx_data, tx_len);
				wiced_packet_set_data_end(transmit_packet, (uint8_t*)tx_data + tx_len);

				if ( wiced_udp_send(&socket_udp, &rmtIp, rmtPort, transmit_packet)!=WICED_SUCCESS )
				{
					W_DBG3("AirCmdDaemonProcess : wiced_udp_send error\r\n");
					wiced_packet_delete(transmit_packet);
					break;
				}
			}
			//W_DBG3("AirCmdDaemonProcess : wiced_tcp/udp_send OK (%d)\r\n", tx_len);
		}

		if ( bTCP )
		{
			W_DBG3("AirCmdDaemonProcess : Disconnect (%d) %s:%d\r\n", result, WXNetwork_WicedV4IPToString(0, rmtIp), rmtPort);
			wiced_tcp_disconnect(&socket_tcp);
			wiced_tcp_delete_socket(&socket_tcp);
		}
		else
		{
			wiced_udp_delete_socket(&socket_udp);
		}

		free(g_aircmd_rx_data);
		free(g_aircmd_tx_data);

		g_aircmd_status = 0;
	}

	W_DBG3("AirCmdDaemonProcess : stop\r\n");
	return;
}

void AirCommandStart()
{
	if ( g_aircmd_status!=0 )
	{
		W_DBG("AirCmdDaemonProcess is already running.");
		return;
	}

	if ( WICED_SUCCESS!=wiced_rtos_create_thread(&thread_handle_aircommand,
			WICED_APPLICATION_PRIORITY + 2, "WizFiAirCommandTask", AirCommandDaemonProcess, (1024*4), NULL) )
	{
		W_DBG("wiced_rtos_create_thread : AirCmdDaemonProcess error");
	}
}

void process_air_command_rx_data()
{
	UINT32 aircmd_i;

	g_aircmd_status = 3;
	memset(g_aircmd_tx_data, 0, sizeof(g_aircmd_tx_data));
	for (aircmd_i=0; aircmd_i<strlen(g_aircmd_rx_data); aircmd_i++)
	{
		WXS2w_CommandCharProcess(g_aircmd_rx_data[aircmd_i]);
	}
	g_aircmd_status = 4;
}

UINT8 WXCmd_MAIRCMD(UINT8 *ptr)
{
	UINT8 *p;
	UINT8 status = 0;
	UINT8 buff_opentype[3] = {0,};
	UINT8 buff_mode = 0;
	UINT32 buff_port = 0;
	UINT32 buff_opt = 0;

	if ( strcmp((char*)ptr, "?")==0 )
	{
		W_RSP("%s,%c,%d,%d\r\n", g_wxProfile.aircmd_opentype, g_wxProfile.aircmd_mode, g_wxProfile.aircmd_port, g_wxProfile.aircmd_opt);
		return WXCODE_SUCCESS;
	}

	p = WXParse_NextParamGet(&ptr);
	if ( !p )					return WXCODE_EINVAL;
	if ( strlen((char*)p)>=3 )	return WXCODE_EINVAL;
	strcpy((char*)buff_opentype, (char*)p);

	if ( strcmp(upstr((char*)buff_opentype), "S") != 0 && strcmp(upstr((char*)buff_opentype), "O") != 0 )
		return WXCODE_EINVAL;

	if ( strcmp(upstr((char*)buff_opentype), "O")==0 )
	{
		if ( !WXLink_IsWiFiLinked() )	return WXCODE_WIFISTATUS_ERROR;
		if ( g_aircmd_status!=0 )		return WXCODE_EINVAL;
	}

	p = WXParse_NextParamGet(&ptr);
	if ( !p )					return WXCODE_EINVAL;
	if ( strlen((char*)p)!=1 )	return WXCODE_EINVAL;
	if ( strcmp(upstr((char*)p), "T")==0 )		buff_mode =  'T';
	else if ( strcmp(upstr((char*)p), "U")==0 )	buff_mode =  'U';
	else										return WXCODE_EINVAL;

	p = WXParse_NextParamGet(&ptr);
	if ( p )	status = WXParse_Int(p, &buff_port);
	if ( status != WXCODE_SUCCESS )		return WXCODE_EINVAL;

	p = WXParse_NextParamGet(&ptr);
	if ( p )	status = WXParse_Hex(p, &buff_opt);
	if ( status != WXCODE_SUCCESS )		return WXCODE_EINVAL;

	strcpy((char*)g_wxProfile.aircmd_opentype, (char*)buff_opentype);
	g_wxProfile.aircmd_mode = buff_mode;
	g_wxProfile.aircmd_port = buff_port;
	g_wxProfile.aircmd_opt = buff_opt;

	if ( strcmp(upstr((char*)buff_opentype), "O")==0 )		AirCommandStart();

	return WXCODE_SUCCESS;
}


