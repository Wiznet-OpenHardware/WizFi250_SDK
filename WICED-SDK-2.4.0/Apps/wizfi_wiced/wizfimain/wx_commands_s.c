#include "wx_defines.h"
#include "wiced_tcpip.h"

#include "wiced_dct.h"
#if 1 //MikeJ 130528 ID1072 - Improve UART Performance
#include "wiced_platform.h"
#include "platform_common_config.h"
#endif

UINT8 WXCmd_SDATA(UINT8 *ptr)
{
	if ( !g_isAutoconnected )
	{
		return WXCODE_MODESTATUS_ERROR;
	}

	if ( !WXNetwork_NetIsCidOpen(g_currentScid) && (memcmp(g_wxProfile.scon_opt2, "TS", 2) != 0) )
	{
		return WXCODE_FAILURE;
	}

	g_wxModeState = WX_MODE_DATA;
	WXS2w_LEDIndication(2, 0, 0, 0, 0, 1);

	return WXCODE_SUCCESS;
}

UINT8 WXCmd_SCON(UINT8 *ptr)
{
	UINT8 *p;
	UINT8 status;
#if 1 //MikeJ 130624 ID1084 - Modified local-port checking part
	UINT8 i;
#endif

	UINT8 status_remote_ip = WXCODE_EINVAL;
	UINT8 status_remote_port = WXCODE_EINVAL;
	UINT8 status_local_port = WXCODE_EINVAL;
	UINT8 scid;

	UINT8 buff_scon1[3] = { 0, };
	UINT8 buff_scon2[4] = { 0, };
	WT_IPADDR buff_remote_ip;
#if 0 //MikeJ 130624 ID1084 - Modified local-port checking part
	UINT32 buff_remote_port;
	UINT32 buff_local_port;
#else
	UINT32 buff_remote_port = 0;
	UINT32 buff_local_port = 0;
#endif
	UINT8 buff_datamode = 0;

	memset(&buff_remote_ip, 0, 4);

	if ( strcmp((char*)ptr, "?") == 0 )
	{
		W_RSP("%s,%s,%s,%d,%d,%d\r\n", g_wxProfile.scon_opt1, g_wxProfile.scon_opt2, g_wxProfile.scon_remote_ip, g_wxProfile.scon_remote_port, g_wxProfile.scon_local_port, g_wxProfile.scon_datamode);
		return WXCODE_SUCCESS;
	}

	p = WXParse_NextParamGet(&ptr);
	if ( !p )						return WXCODE_EINVAL;
	if ( strlen((char*)p) >= 3 )	return WXCODE_EINVAL;
	strcpy((char*)buff_scon1, (char*)p);

	if ( strcmp(upstr((char*)buff_scon1), "SO") != 0 && strcmp(upstr((char*)buff_scon1), "S") != 0 && strcmp(upstr((char*)buff_scon1), "O") != 0 )
		return WXCODE_EINVAL;

	p = WXParse_NextParamGet(&ptr);
	if ( !p )						return WXCODE_EINVAL;
	if ( strlen((char*)p) >= 4 )	return WXCODE_EINVAL;
	strcpy((char*)buff_scon2, (char*)p);
#if 1 //MikeJ 130806 - Modify lower case to upper case
	upstr((char*)buff_scon2);
#endif
	if (   (toupper(buff_scon2[0]) != 'T' && toupper(buff_scon2[0]) != 'U')
		|| (toupper(buff_scon2[1]) != 'S' && toupper(buff_scon2[1]) != 'C')
		|| (toupper(buff_scon2[2]) != 'N' && toupper(buff_scon2[2]) != 'S') )
		return WXCODE_EINVAL;

	p = WXParse_NextParamGet(&ptr);
	if ( p )	status_remote_ip = WXParse_Ip(p, (UINT8 *) &buff_remote_ip);

	p = WXParse_NextParamGet(&ptr);
	if ( p )	status_remote_port = WXParse_Int(p, &buff_remote_port);

	p = WXParse_NextParamGet(&ptr);
	if ( p )	status_local_port = WXParse_Int(p, &buff_local_port);

	// sekim XXXXX 20151223 Enable TCP Multi Socket
/*
#if 1 //MikeJ 130624 ID1084 - Modified local-port checking part
	for(i=0; i<WX_MAX_SCID_RANGE; i++) {
		if(buff_local_port != 0 && buff_local_port == g_scList[i].localPort) return WXCODE_EINVAL;
	}
#endif
*/

	p = WXParse_NextParamGet(&ptr);
	if ( p )
	{
		status = WXParse_Boolean(p, &buff_datamode);
		if ( status != WXCODE_SUCCESS )		return status;
	}

	if ( g_isAutoconnected )		return WXCODE_MODESTATUS_ERROR;

	if ( !WXLink_IsWiFiLinked() )
	{
		if ( toupper(buff_scon1[0]) == 'O' || toupper(buff_scon1[1]) == 'O' )
			return WXCODE_WIFISTATUS_ERROR;
	}

	if ( (toupper(buff_scon2[0]) == 'T' && toupper(buff_scon2[1]) == 'S') )
	{
#if 1 //MikeJ 130624 ID1085 - Added remote-address checking part
		memset(&buff_remote_ip, 0, 4);
		buff_remote_port = 0;
#endif
		if ( status_local_port != WXCODE_SUCCESS || !is_valid_port(buff_local_port) )	return WXCODE_EINVAL;
	}
	else if ( (toupper(buff_scon2[0]) == 'T' && toupper(buff_scon2[1]) == 'C') )
	{
#if 1 //MikeJ 130410 ID1018 - When use TCP/UDP Client, verify local port.
#if 0 //MikeJ 130624 ID1084 - Modified local-port checking part
		if ( status_local_port != WXCODE_SUCCESS || !is_valid_port(buff_local_port) )	return WXCODE_EINVAL;
#else
		//UINT8 chkaddr[4];
		if ( !is_valid_port_include_zero(buff_local_port) )	return WXCODE_EINVAL;
#endif
#endif
		if ( status_remote_port != WXCODE_SUCCESS || !is_valid_port(buff_remote_port) )	return WXCODE_EINVAL;
		if ( status_remote_ip != WXCODE_SUCCESS )										return WXCODE_EINVAL;

		//MikeJ 130624 ID1085 - Added remote-address checking part ==> Removed by sekim
	}
	else if ( (toupper(buff_scon2[0]) == 'U' && toupper(buff_scon2[1]) == 'S') )
	{
		if ( status_local_port != WXCODE_SUCCESS || !is_valid_port(buff_local_port) )	return WXCODE_EINVAL;
#if 1 //MikeJ 130806 ID1111 - Check Remote IP,Port availability on UDP Server option
		if ( status_remote_port == WXCODE_SUCCESS ) {
			if( status_remote_ip != WXCODE_SUCCESS || !is_valid_port(buff_remote_port) )
				return WXCODE_EINVAL;
		}
		if ( status_remote_ip == WXCODE_SUCCESS ) {
			if( status_remote_port != WXCODE_SUCCESS ) return WXCODE_EINVAL;
			for(i=0; i<WX_MAX_SCID_RANGE; i++) {
				UINT8 chkaddr[4];
				chkaddr[0] = buff_remote_ip[3];
				chkaddr[1] = buff_remote_ip[2];
				chkaddr[2] = buff_remote_ip[1];
				chkaddr[3] = buff_remote_ip[0];
				if(memcmp(chkaddr, &g_scList[i].remoteIp.ip.v4, 4) == 0 && 
					buff_remote_port == g_scList[i].remotePort) return WXCODE_EINVAL;
			}
		}
#endif
	}
	else if ( (toupper(buff_scon2[0]) == 'U' && toupper(buff_scon2[1]) == 'C') )
	{
#if 1 //MikeJ 130410 ID1018 - When use TCP/UDP Client, verify local port.
#if 0 //MikeJ 130624 ID1084 - Modified local-port checking part
		if ( status_local_port != WXCODE_SUCCESS || !is_valid_port(buff_local_port) )	return WXCODE_EINVAL;
#else
		if ( !is_valid_port_include_zero(buff_local_port) )	return WXCODE_EINVAL;
#endif
#endif
		if ( status_remote_port != WXCODE_SUCCESS || !is_valid_port(buff_remote_port) )	return WXCODE_EINVAL;
		if ( status_remote_ip != WXCODE_SUCCESS )										return WXCODE_EINVAL;
	}

	strcpy((char*)g_wxProfile.scon_opt1, (char*)buff_scon1);
	strcpy((char*)g_wxProfile.scon_opt2, (char*)buff_scon2);
	sprintf((char*)g_wxProfile.scon_remote_ip, "%d.%d.%d.%d", buff_remote_ip[0], buff_remote_ip[1], buff_remote_ip[2], buff_remote_ip[3]);
	g_wxProfile.scon_remote_port = buff_remote_port;
	g_wxProfile.scon_local_port = buff_local_port;
	g_wxProfile.scon_datamode = buff_datamode;
	if ( !(toupper(buff_scon1[0]) == 'O' || toupper(buff_scon1[1]) == 'O') )
		return WXCODE_SUCCESS;

#if 0 //MikeJ 130410 ID1033 - If local port is using, error will be returned..
	for(scid=0; scid<WX_MAX_SCID_RANGE; scid++) {
		if ( g_scList[scid].localPort == buff_local_port ) return WXCODE_EINVAL;
	}
#endif
	if ( buff_datamode == 1 )
	{
		UINT32 i;
		UINT8 found_valid_scid = 0;
		for(i = 0; i < WX_MAX_SCID_RANGE; i++)
		{
			if ( g_scList[i].pSocket )	{ found_valid_scid = 1; break; }
		}
		if ( found_valid_scid==1 )	return WXCODE_EINVAL;
	}

	{
		UINT8 bTCP = (toupper(buff_scon2[0])=='T')?1:0;
		UINT8 bServer = (toupper(buff_scon2[1])=='S')?1:0;
		UINT8 tlsMode = toupper(buff_scon2[2]);

#if 1	// kaizen 20130703 ID1056 Do not support multiple socket function when use SSL socket.
		if( tlsMode == 'S' )
		{
			for(scid=0; scid<WX_MAX_SCID_RANGE; scid++)
			{
				if ( g_scList[scid].tlsMode == 'S') return WXCODE_EINVAL;
				if ( g_scList[scid].tlsMode == 'N') return WXCODE_EINVAL;
			}
		}
		else if ( tlsMode == 'N' )
		{
			for(scid=0; scid<WX_MAX_SCID_RANGE; scid++)
			{
				if ( g_scList[scid].tlsMode == 'S') return WXCODE_EINVAL;
			}
		}
#endif

		wiced_ip_address_t INITIALISER_IPV4_ADDRESS(remote_ip, str_to_ip((char*)g_wxProfile.scon_remote_ip));
		scid = WXNetwork_ScidGet();
		if ( (scid=WXNetwork_ScidGet())==WX_INVALID_SCID )	return WXCODE_EBADCID;

		// sekim XXXX 20160119 Add socket_ext_option9 for TCP Server Multi-Connection
		{
			extern UINT16 g_last_TSN_localport;
			if ( bTCP && bServer )	g_last_TSN_localport = g_wxProfile.scon_local_port;
		}

		LaunchSocketOpen(scid, bTCP, bServer, tlsMode, remote_ip, g_wxProfile.scon_remote_port, g_wxProfile.scon_local_port, buff_datamode);

		//W_DBG("WXCmd_SCON : buff_datamode(%d), g_isAutoconnected(%d), g_wxModeState(%d)", buff_datamode, g_isAutoconnected, g_wxModeState);
	}

	return WXCODE_SUCCESS;
}

UINT8 WXCmd_SMGMT(UINT8 *ptr)
{
	UINT8 *p;
	UINT32 i = 0, buff_scid = 0;
	UINT8 status = WXCODE_EINVAL;

	if ( ptr[0] == '?' && ptr[1] == '\0' )
	{
		//
		//extern WT_SCLIST g_scList[WX_MAX_SCID_RANGE];
		UINT8 count_valid_cid = 0;
		for(i = 0; i < WX_MAX_SCID_RANGE; i++)
		{
			if ( g_scList[i].pSocket==0 )	continue;
			count_valid_cid++;
		}
#if 1 //MikeJ 130410 ID1020 - Remove socket memory address
		W_RSP("Number of Sockets : %d (SCID/Mode/Remote/Local/DataMode)\r\n", count_valid_cid);
#else
		W_RSP("Number of Sockets : %d (SCID/Socket/Mode/Remote/Local/DataMode)\r\n", count_valid_cid);
#endif
		for(i = 0; i < WX_MAX_SCID_RANGE; i++)
		{
			if ( g_scList[i].pSocket==0 )	continue;

#if 1 //MikeJ 130410 ID1020 - Remove socket memory address
			W_RSP("%d/", i);
#else
			W_RSP("%d/0x%08x/", i, g_scList[i].pSocket);
#endif
			(g_scList[i].conType == WX_SC_CONTYPE_UDP) ? W_RSP("U") : W_RSP("T");
#if 1	// kaizen 20130703 ID1096 - For printing SSL Mode using AT+SMGMT=? command.
			(g_scList[i].conMode == WX_SC_MODE_SERVER) ? W_RSP("S") : W_RSP("C");
			(g_scList[i].tlsMode == WX_SC_TLS_MODE_SECURE ) ? W_RSP("S/") : W_RSP("N/");
#else
			(g_scList[i].conMode == WX_SC_MODE_SERVER) ? W_RSP("S/") : W_RSP("C/");
#endif

			if ( g_scList[i].remoteIp.version==WICED_IPV4 )
				W_RSP("%s:", WXNetwork_WicedV4IPToString(0, g_scList[i].remoteIp));
			else if ( g_scList[i].remoteIp.version==WICED_IPV6 )
				W_RSP("");
			else
				W_RSP("");

#if 0 //MikeJ 130702 ID1093 - Adjust Response Format
			if ( g_scList[i].remotePort == 0 )	W_RSP("/");
#else
			if ( g_scList[i].remotePort == 0 )	W_RSP("0/");
#endif
			else								W_RSP("%d/", g_scList[i].remotePort);

			W_RSP("%d/%d", g_scList[i].localPort, g_scList[i].dataMode);
			W_RSP("\r\n");
		}

		return WXCODE_SUCCESS;
	}

	p = WXParse_NextParamGet(&ptr);
	if ( p )
	{
		if ( strcmp(upstr((char*)p), "ALL") == 0 )
		{
			///////////////////////////////////////////////////////////////////////////////////////////
			// sekim 20130709 ID1089 if Socket Close by AT Command, don't restart <TCP Server>
			for(i = 0; i < WX_MAX_SCID_RANGE; i++)
			{
				if ( g_scList[i].pSocket )
				{
					// sekim 20140625 Bug Fix in AT+SMGMT
					//if ( g_scList[i].conType==WX_SC_CONTYPE_TCP && g_scList[i].conType==WX_SC_MODE_SERVER )
					if ( g_scList[i].conType==WX_SC_CONTYPE_TCP && g_scList[i].conMode==WX_SC_MODE_SERVER )
					{
						g_scList[i].notRestartTCPServer = 1;
					}
				}
			}
			///////////////////////////////////////////////////////////////////////////////////////////

			WXNetwork_CloseSCAllList();
		}
		else
		{
			status = WXParse_Int(p, &buff_scid);
			if ( status != WXCODE_SUCCESS )					return WXCODE_EBADCID;
			if ( !WXNetwork_NetIsCidOpen(buff_scid) )		return WXCODE_EINVAL;

			///////////////////////////////////////////////////////////////////////////////////////////
			// sekim 20130709 ID1089 if Socket Close by AT Command, don't restart <TCP Server>
			if ( g_scList[buff_scid].pSocket )
			{
				// sekim 20140625 Bug Fix in AT+SMGMT
				//if ( g_scList[buff_scid].conType==WX_SC_CONTYPE_TCP && g_scList[buff_scid].conType==WX_SC_MODE_SERVER )
				if ( g_scList[buff_scid].conType==WX_SC_CONTYPE_TCP && g_scList[buff_scid].conMode==WX_SC_MODE_SERVER )
				{
					g_scList[buff_scid].notRestartTCPServer = 1;
				}
			}
			///////////////////////////////////////////////////////////////////////////////////////////

			WXNetwork_CloseSCList(buff_scid);
		}
	}

	return WXCODE_SUCCESS;
}

UINT8 WXCmd_SSEND(UINT8 *ptr)
{
	UINT8 *p;
#if 0 //MikeJ 130528 ID1072 - Improve UART Performance
	UINT8 status;
	WT_IPADDR buff_ipAddr;
	UINT32 buff_scid = 0, buff_port = 0, buff_length = 0;
#else
	UINT8 status, retry_cnt;
	WT_IPADDR buff_ipAddr = {0,};
	UINT32 buff_scid = 0, buff_port = 0, buff_length = 0, tmp_length;
	UINT32 wait_time, left_time, start_time, cur_time;
	wiced_result_t ret;
#endif

	p = WXParse_NextParamGet(&ptr);
	if ( !p )							return WXCODE_EINVAL;
	status = WXParse_Int(p, &buff_scid);
	if ( status != WXCODE_SUCCESS )		return WXCODE_EINVAL;

	///////////////////////////////////////////////////////////////////////////////////////////
	// sekim 20130129 UDP 인 경우, remote IP/Port에 입력하고, remote IP/Port에 값이 없으면 오류로 처리
	//if ( g_scList[buff_scid].conType==WX_SC_CONTYPE_UDP )
	{
		p = WXParse_NextParamGet(&ptr);
		if ( g_scList[buff_scid].conType==WX_SC_CONTYPE_UDP && p )
		{
			status = WXParse_Ip(p, (UINT8 *) &buff_ipAddr);
			if ( status==WXCODE_SUCCESS )
			{
				SET_IPV4_ADDRESS(g_scList[buff_scid].remoteIp, MAKE_IPV4_ADDRESS(buff_ipAddr[0], buff_ipAddr[1], buff_ipAddr[2], buff_ipAddr[3]));
			}
		}

		p = WXParse_NextParamGet(&ptr);
		if ( g_scList[buff_scid].conType==WX_SC_CONTYPE_UDP && p )
		{
			status = WXParse_Int(p, &buff_port);
			if ( status==WXCODE_SUCCESS && buff_port>0 )
			{
				g_scList[buff_scid].remotePort = buff_port;
			}
		}

		if ( !(g_scList[buff_scid].remoteIp.version==WICED_IPV4 || g_scList[buff_scid].remoteIp.version==WICED_IPV6) || g_scList[buff_scid].remotePort==0 )
		{
			return WXCODE_EINVAL;
		}
	}
	///////////////////////////////////////////////////////////////////////////////////////////

	p = WXParse_NextParamGet(&ptr);
	if ( !p )							return WXCODE_EINVAL;
	status = WXParse_Int(p, &buff_length);
	if ( status != WXCODE_SUCCESS )		return WXCODE_EINVAL;

#if 0 //MikeJ 130528 ID1072 - Improve UART Performance
	if ( !WXNetwork_NetIsCidOpen(buff_scid) )						return WXCODE_EINVAL;
	if ( buff_length >= sizeof(g_scTxBuffer) || buff_length <= 0 )	return WXCODE_EINVAL;

	g_scTxIndex = buff_length;
	g_currentScid = buff_scid;

#if 1	// kaizen 20130516 ID1067 - Modified bug about occurring watchodg event at listening serial input
	wizfi_task_monitor_stop = 1;
	WXHal_CharNGet(g_scTxBuffer, g_scTxIndex);
	wizfi_task_monitor_stop = 0;
	wiced_update_system_monitor(&wizfi_task_monitor_item, MAXIMUM_ALLOWED_INTERVAL_BETWEEN_WIZFIMAINTASK);
#else
	WXHal_CharNGet(g_scTxBuffer, g_scTxIndex);
#endif
	status = WXS2w_DataBufferTransmit();
#else
	if ( !WXNetwork_NetIsCidOpen(buff_scid) )	return WXCODE_EINVAL;
	g_currentScid = buff_scid;

	if(buff_ipAddr[0] == 0 && buff_ipAddr[1] == 0 && buff_ipAddr[2] == 0 && buff_ipAddr[3] == 0)
		W_RSP("[%d,,,%d", buff_scid, buff_length);
	else W_RSP("[%d,%d.%d.%d.%d,%d,%d", buff_scid, buff_ipAddr[0], buff_ipAddr[1], 
		buff_ipAddr[2], buff_ipAddr[3], buff_port, buff_length);

	p = WXParse_NextParamGet(&ptr);
	if ( p ) {
		status = WXParse_Int(p, (UINT32*)&wait_time);
		if ( status != WXCODE_SUCCESS )		return WXCODE_EINVAL;
		W_RSP(",%d]\r\n", wait_time);
	} else {
		wait_time = buff_length + 10000; // base 10s + 100s per 1Mb
		W_RSP("]\r\n");
	}

	wizfi_task_monitor_stop = 1;
	start_time = host_rtos_get_time();

	while(buff_length > 0) {
		cur_time = host_rtos_get_time();
		if(wait_time > cur_time - start_time) 
			left_time = wait_time - (cur_time - start_time);
		else left_time = 0;
		
		tmp_length = MIN(buff_length, 1400);
		ret = wiced_uart_receive_bytes(STDIO_UART, g_scTxBuffer, tmp_length, left_time);	//WICED_NEVER_TIMEOUT);
		if(ret == WICED_TIMEOUT) {
			W_DBG("WXCmd_SSEND: Timeout\r\n");
			status = WXCODE_FAILURE;
			goto SSEND_EXIT;
		}
		
		g_scTxIndex = tmp_length;
		retry_cnt = 3;
		while(retry_cnt--) {
			status = WXS2w_DataBufferTransmit();
			cur_time = host_rtos_get_time();
			if(wait_time <= cur_time - start_time) {
				if(buff_length > tmp_length) status = WXCODE_FAILURE;
				goto SSEND_EXIT;
			}
			if(status == WXCODE_SUCCESS) break;
			W_DBG("WXCmd_SSEND: DataBufferTransmit try again\r\n");
			g_scTxIndex = tmp_length;
		}
		if(retry_cnt == 0) {
			W_DBG("WXCmd_SSEND: DataBufferTransmit fail\r\n");
			break;
		}

		buff_length -= tmp_length;
	}

SSEND_EXIT:
	wizfi_task_monitor_stop = 0;
	wiced_update_system_monitor(&wizfi_task_monitor_item, MAXIMUM_ALLOWED_INTERVAL_BETWEEN_WIZFIMAINTASK);
#endif

	return status;
}

UINT8 WXCmd_SFORM(UINT8 *ptr)
{
	UINT8 *p;
	UINT8 status = WXCODE_SUCCESS;
	UINT32 i;
	UINT32 buffhex;

	UINT8 mainbuff[9]; // {1,TCP,1.2.3.4,4000,5}ABCDE\r\n
#if 1 //MikeJ 130410 ID1021 - If not input param, garbage value is entered.
	UINT8 databuff[5] = {'{', ',', '}', '\r', '\n'};
#else
	UINT8 databuff[5];
#endif

	if ( strcmp((char*)ptr, "?") == 0 )
	{
		W_RSP("%c%c%c%c%c%c%c%c%c,%02x,%02x,%02x,%02x,%02x", g_wxProfile.xrdf_main[0], g_wxProfile.xrdf_main[1], 
			g_wxProfile.xrdf_main[2], g_wxProfile.xrdf_main[3], g_wxProfile.xrdf_main[4], g_wxProfile.xrdf_main[5], 
			g_wxProfile.xrdf_main[6], g_wxProfile.xrdf_main[7], g_wxProfile.xrdf_main[8], g_wxProfile.xrdf_data[0], 
			g_wxProfile.xrdf_data[1], g_wxProfile.xrdf_data[2], g_wxProfile.xrdf_data[3], g_wxProfile.xrdf_data[4]);
		return WXCODE_SUCCESS;
	}

	p = WXParse_NextParamGet(&ptr);
	if ( !p )								return WXCODE_EINVAL;
	for(i = 0; i < 9; i++)
	{
		if ( p[i] == '0' || p[i] == '1' )	mainbuff[i] = p[i];
		else								status = WXCODE_EINVAL;
	}
	if ( status != WXCODE_SUCCESS )			return status;
#if 1 //MikeJ 130410 ID1021 - If not input param, garbage value is entered.
	if( p[i] != '\0' ) return WXCODE_EINVAL;
#endif

	for(i = 0; i < 5; i++)
	{
		p = WXParse_NextParamGet(&ptr);
		if ( p )
		{
			if ( WXCODE_SUCCESS != WXParse_Hex(p, &buffhex) )	status = WXCODE_EINVAL;
			else												databuff[i] = buffhex;
		}
	}
	if ( status != WXCODE_SUCCESS )			return status;

	memcpy(g_wxProfile.xrdf_main, mainbuff, sizeof(mainbuff));
	memcpy(g_wxProfile.xrdf_data, databuff, sizeof(databuff));

	return WXCODE_SUCCESS;
}

#include "wiced_wifi.h"
#include "wiced_management.h"

#include "bootloader_app.h"
#include "wwd_constants.h"
#include <wiced_utilities.h>
#include <resources.h>


UINT8 WXCmd_SOPT1(UINT8 *ptr)
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

	if ( buff1 == 10 )
	{
		if ( buff2 < 1 || buff2 > 10000 )	return WXCODE_EINVAL;

		g_wxProfile.timer_autoesc = buff2;
	}
	else if ( buff1 == 11 )
	{
	}
	else if ( buff1 == 31 )
	{
		*(UINT8*)buff3 = buff4;
	}
	else if ( buff1 == 32 )
	{
		WDUMP((UINT8*)buff3, buff4);
	}
	else if ( buff1 == 33 )
	{
		WDUMPEXT((UINT8*)buff3, buff4);
	}
	else if ( buff1 == 35 )
	{
		extern UINT16 g_socket_open_thread_stack_xxx;
		g_socket_open_thread_stack_xxx = buff2;
	}
	// sekim XXX Test Function : WXCmd_SOPT1
	else if ( buff1 == 41 )
	{
		wiced_network_up( WICED_STA_INTERFACE, WICED_USE_EXTERNAL_DHCP_SERVER, NULL );
	}
	else if ( buff1 == 42 )
	{
		wiced_ip_setting_t device_init_ip_settings =
		{
		    INITIALISER_IPV4_ADDRESS( .ip_address, MAKE_IPV4_ADDRESS(192,168, 12,111) ),
		    INITIALISER_IPV4_ADDRESS( .netmask,    MAKE_IPV4_ADDRESS(255,255,255,  0) ),
		    INITIALISER_IPV4_ADDRESS( .gateway,    MAKE_IPV4_ADDRESS(192,168, 12,  1) ),
		};
		wiced_network_up( WICED_STA_INTERFACE, WICED_USE_STATIC_IP, &device_init_ip_settings );
	}
	else if ( buff1 == 43 )
	{
		wiced_network_down( WICED_STA_INTERFACE );
	}
	else if ( buff1 == 44 )
	{
		UINT8 scid = 0;
	    if ( wiced_rtos_is_current_thread(&g_socketopen_thread[scid])!=WICED_SUCCESS )
	    {
	    	if ( wiced_rtos_delete_thread(&g_socketopen_thread[scid])!=WICED_SUCCESS )
	    		W_DBG("ProcessWizFiQueue : wiced_rtos_delete_thread error");
	    }
	}
	else if ( buff1 == 45 )
	{
		WXS2w_LEDIndication(buff2, 0, 0, 0, 0, buff3);
	}
	else if ( buff1 == 51 )
	{
	}
	//////////////////////////////////////////////////////////////////////////////////////////
	else if ( buff1 == 61 )
	{
		static const wiced_wps_device_detail_t wps_details =
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

		wiced_result_t result;
		char config_wps_pin[9] = {0,};
		wiced_wps_credential_t wps_credentials;
		memset(&wps_credentials, 0, sizeof(wiced_wps_credential_t));
		//result = wiced_wps_enrollee(WICED_WPS_PBC_MODE, &wps_details, config_wps_pin, &wps_credentials, 1);
		result = wiced_wps_enrollee(buff2, &wps_details, config_wps_pin, &wps_credentials, 1);
        if (result == WICED_SUCCESS)
        {
        }
	}
	else if ( buff1 == 62 ) // UART, WiFi Throughput Test
	{
		UINT32 ch_count = 0;
		UINT8 ch = 0;
		wizfi_task_monitor_stop = 1;

		// UART Rx
		if ( buff2==1 )
		{
			while(1)
			{
				ch = getchar();
				ch_count++;
				if ( ch=='x' )	break;
			}
			W_RSP("UART Rx : %d \r\n", ch_count);
		}
		else if ( buff2==2 )
		{
			VOID WXHal_CharNPut(const VOID *buf, UINT32 len);
			char szTemp[101] = "12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678\r\n";
			int gg;
			for (gg=0; gg<buff3; gg++)
			{
				WXHal_CharNPut(szTemp, 100);
			}
			W_RSP("UART Tx : (%d, %d)\r\n", buff3, buff3*100);
		}
		else if ( buff2==3 )
		{
			void TCP_Throughput_Test(UINT8 type);
			TCP_Throughput_Test(3);
		}
		else if ( buff2==4 )
		{
			void TCP_Throughput_Test(UINT8 type);
			TCP_Throughput_Test(4);
		}
		wizfi_task_monitor_stop = 0;
	}

	return WXCODE_SUCCESS;
}


UINT8 WXCmd_SOPT2(UINT8 *ptr)
{
	return WXCODE_SUCCESS;
}

// sekim 20150616 add AT+SDNAME
UINT8 WXCmd_SDNAME(UINT8 *ptr)
{
	UINT8 *p;
	if ( strcmp((char*)ptr, "?") == 0 )
	{
		W_RSP("%s\r\n", g_wxProfile.domainname_for_scon);
		return WXCODE_SUCCESS;
	}

	p = WXParse_NextParamGet(&ptr);
	if (!p)		return WXCODE_EINVAL;
	if ( strlen((char*)p)==0 || strlen((char*)p) >= sizeof(g_wxProfile.domainname_for_scon) ) return WXCODE_EINVAL;
	strcpy((char*)g_wxProfile.domainname_for_scon, (char*)p);

	return WXCODE_SUCCESS;
}


// sekim 20130514 TCP Throughput Test
void TCP_Throughput_Test(UINT8 type)
{
	wiced_result_t result;
	wiced_packet_t* rx_packet;
	wiced_tcp_socket_t socket_tcp;

	char* rx_data;
	uint16_t rx_data_length;
	uint16_t available_data_length;

	W_RSP("TCP Throughput Start \r\n");

	wiced_interface_t interface = WICED_STA_INTERFACE;
	result = wiced_tcp_create_socket(&socket_tcp, interface);

	if ( result!=WICED_SUCCESS )
	{
		W_RSP("TCP Throughput : Error 101 \r\n");
		return;
	}

	if ( wiced_tcp_listen(&socket_tcp, 2000)!=WICED_SUCCESS )
	{
		W_RSP("TCP Throughput : Error 105 \r\n");
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
				W_RSP("TCP Throughput : Error 120 \r\n");
				wiced_tcp_delete_socket(&socket_tcp);
				return;
			}
			else
			{
				break;
			}
		}
	}

	W_RSP("TCP Throughput : Accepted \r\n");

	UINT32 total_data = 0;

	if ( type==3 )
	{
		while(1)
		{
			result = wiced_tcp_receive(&socket_tcp, &rx_packet, WICED_WAIT_FOREVER);

			if ( result!=WICED_SUCCESS )
			{
				// sekim wiced_tcp_receive return value processing
				if ( result==WICED_PENDING )
				{
					W_RSP("TCP Throughput : Error Pending - 100 \r\n");
					if ( socket_tcp.socket.nx_tcp_socket_state==NX_TCP_ESTABLISHED )
					{
						W_RSP("TCP Throughput : Error Pending - 110 \r\n");
						continue;
					}
				}

				wiced_packet_delete(rx_packet);
				wiced_tcp_disconnect(&socket_tcp);
				wiced_tcp_delete_socket(&socket_tcp);
				W_RSP("TCP Throughput : Error 130 \r\n");
				break;
			}

			wiced_packet_get_data(rx_packet, 0, (uint8_t**)&rx_data, &rx_data_length, &available_data_length);

			W_RSP("TCP Throughput : Recv Loop (%d, %d, %d)\r\n", rx_data_length, available_data_length, total_data);

			total_data += rx_data_length;
			//if ( rx_data[rx_data_length-1]== 'x' )
			if ( total_data>1000000 )
			{
				wiced_packet_delete(rx_packet);
				wiced_tcp_disconnect(&socket_tcp);
				wiced_tcp_delete_socket(&socket_tcp);
				break;
			}

			wiced_packet_delete(rx_packet);
		}
		W_RSP("TCP Throughput : Recv End (%d)\r\n", total_data);
	}
	else if ( type==4 )
	{
		char szTemp[101] = "12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678\r\n";

		int gg, hh;
		UINT32 total_send = 0;
		for (gg=0; gg<14; gg++)
		{
			memcpy((g_scTxBuffer+(100*gg)), szTemp, 100);
		}
		hh = 0;
		for (; total_send<1000000; )
		{
			result = wiced_tcp_send_buffer(&socket_tcp, g_scTxBuffer, 1400);
			if ( result != WXCODE_SUCCESS )
			{
				W_RSP("TCP Throughput : Error 210 \r\n");
			}
			hh++;
			total_send += 1400;
			W_RSP("TCP Throughput : Send Loop (%d, %d)\r\n", hh, total_send);
		}
		W_RSP("TCP Throughput : Send End (%d)\r\n", total_send);

		wiced_tcp_disconnect(&socket_tcp);
		wiced_tcp_delete_socket(&socket_tcp);
	}

	return;
}


