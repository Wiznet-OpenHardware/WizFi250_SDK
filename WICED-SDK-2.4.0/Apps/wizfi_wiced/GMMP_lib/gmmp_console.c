#include "../wizfimain/wx_defines.h"

//#include <stdlib.h>
//#include <string.h>
//#include <time.h>
//#include <signal.h>
//#include <unistd.h>
//#include <pthread.h>

#include "GMMP_lib/GMMP.h"
#include "GMMP_lib/Define/Define.h"

char g_pszMessage[256] = "Temperature=28C";

const int nErrorLevel = GMMP_ERROR_LEVEL_DEBUG_DATA;

int g_nTimerDelivery = 0;
int g_nTimerHB = 0;

int g_gmmp_step = -1; // -1(Not Started) 0(Disconnected) 1(Connected) 2(GW Registration) 3(Get Profile) 4(Device Registration)

wiced_tcp_socket_t* g_pGmmpSocket = 0;

uint8_t* g_pGMMPTCPBuffer = 0;
uint32_t g_nGMMPTCPBufferLen = 0;

uint8_t g_nGMMPThreadCount = 0;

int GW_Delivery()
{
	int nRet = 0;
	int nTotalCount = 0;
	int nLoop = 0;
	int nMessageBodyLen = strlen(g_pszMessage);

	if(nMessageBodyLen < MAX_MSG_BODY)
	{
		nTotalCount = 1;
	}
	else
	{
		nTotalCount = nMessageBodyLen/MAX_MSG_BODY;

		if(nMessageBodyLen%MAX_MSG_BODY > 0)
		{
			nTotalCount++;
		}
	}

	int nMessagePos = 0;
	int nSendLen = 0;
	int nSendedLen = nMessageBodyLen;

	char szMessage[MAX_MSG_BODY];

	for(nLoop = 1 ; nLoop <= nTotalCount ; nLoop++)
	{
		memset(szMessage, 0, sizeof(szMessage) );

		if(nSendedLen >= MAX_MSG_BODY)
		{
			nSendLen = MAX_MSG_BODY;
		}
		else
		{
			nSendLen = nSendedLen;
		}

		memcpy(szMessage, g_pszMessage+nMessagePos, nSendLen);

		nRet = GO_Delivery(GetGWID(), NULL, DELIVERY_COLLECT_DATA,  0x01, szMessage, nTotalCount, nLoop, GMMP_ENCRYPTION_NOT);

		if(nRet < 0)
		{
			return 1;
		}
		nSendedLen -= nSendLen;
		nMessagePos+= nSendedLen;
	}

	return 0;
}

void GmmpSendThread(uint32_t arg)
{
	int nCountHB = 0;
	int nCountDelivery = 0;

	g_nGMMPThreadCount++;

	while(1)
	{
		wiced_rtos_delay_milliseconds(1000);

		if ( !WXLink_IsWiFiLinked() )
		{
			continue;
		}
		if ( g_gmmp_step==-2 || g_gmmp_step==-3 )
		{
			continue;
		}

		if ( g_nTimerHB>0 )
		{
			nCountHB++;
			if ( nCountHB>=(g_nTimerHB*6) )
			{
				SetTID(GetTID()+1);

				int nRet = GMMP_SetHB(g_szAuthID, g_szAuthKey, g_szDomainCode, GetGWID());
				if(nRet != GMMP_SUCCESS)
				{
					W_DBG("GmmpSendThread : GMMP_SetHB error (%d)", nRet);
				}
				nCountHB = 0;
				continue;
			}
		}

		if ( g_nTimerDelivery>0 )
		{
			nCountDelivery++;
			if ( nCountDelivery>=(g_nTimerDelivery*6) )
			{
				int nRet = GW_Delivery();
				if(nRet != GMMP_SUCCESS)
				{
					W_DBG("GmmpSendThread : GW_Delivery error (%d)", nRet);
				}
				nCountDelivery = 0;
				continue;
			}
		}
	}

	g_nGMMPThreadCount--;

	return;
}

void GMMPDisconnectSocket()
{
	if ( g_pGmmpSocket )
	{
		wiced_tcp_disconnect(g_pGmmpSocket);
	}
}

void GMMPClearSocket(int bShowMessage)
{
	if ( g_pGmmpSocket )
	{
		wiced_tcp_disconnect(g_pGmmpSocket);
		wiced_tcp_delete_socket(g_pGmmpSocket);

		if ( bShowMessage )
			W_RSP("\r\n[GMMP Disconnect]\r\n");
	}
	g_pGmmpSocket = 0;

	wiced_rtos_delay_milliseconds(100);
}

void GmmpRecvThread(uint32_t arg)
{
    wiced_result_t result;
	wiced_tcp_socket_t socket_tcp;

	GMMPHeader stGMMPHeader;
	void* pBody = NULL;
	int nRet = 0;

	g_nGMMPThreadCount++;

	while(1)
	{
		if ( !WXLink_IsWiFiLinked() )
		{
			GMMPClearSocket(0);
			wiced_rtos_delay_milliseconds(1000);
			continue;
		}

		if ( g_gmmp_step==-2 || g_gmmp_step==-3 )
		{
			GMMPClearSocket(0);
			wiced_rtos_delay_milliseconds(1000);
			continue;
		}

		wiced_interface_t interface = (g_wxProfile.wifi_mode==AP_MODE)?WICED_AP_INTERFACE:WICED_STA_INTERFACE;
		result = wiced_tcp_create_socket(&socket_tcp, interface);

		if ( result!=WICED_SUCCESS )
		{
			W_DBG("GmmpRecvThread : socket creation failed (%d)", result);
			g_pGmmpSocket = 0;
			continue;
		}

		g_pGmmpSocket = &socket_tcp;

		wiced_ip_address_t INITIALISER_IPV4_ADDRESS(remote_ip, str_to_ip((char*)g_szServerIP));

		static uint16_t tcp_client_port_add = 0;
		if ( tcp_client_port_add==0 )
		{
			uint16_t random_value = 0;
			extern wiced_result_t wiced_wifi_get_random( uint16_t* val );
			wiced_wifi_get_random(&random_value);
			tcp_client_port_add = random_value%10000 + 1;
		}
		UINT16 localPort = NX_SEARCH_PORT_START + tcp_client_port_add++;
		if ( tcp_client_port_add>10000 )	tcp_client_port_add = 0;

		wiced_tcp_bind(&socket_tcp, localPort);
		localPort = socket_tcp.socket.nx_tcp_socket_port;

		W_DBG("GmmpRecvThread : connect try %s:%d, %d", WXNetwork_WicedV4IPToString(0, remote_ip), g_nServerPort, localPort);
		result = wiced_tcp_connect(&socket_tcp, &remote_ip, g_nServerPort, 5000);
		if ( result!=WICED_SUCCESS )
		{
			W_DBG("GmmpRecvThread : connect error %s:%d, %d", WXNetwork_WicedV4IPToString(0, remote_ip), g_nServerPort, localPort);
			GMMPClearSocket(0);
			continue;
		}

		W_RSP("\r\n[GMMP Connect]\r\n");
		g_gmmp_step = 1;

		while(1)
		{
			if ( g_gmmp_step==1 )
			{
				if ( GO_Reg(NULL, g_szGWMFID, false)!=GMMP_SUCCESS )		W_DBG("GmmpRecvThread : GO_Reg (GW) Error");
			}
			else if ( g_gmmp_step==2 )
			{
				if ( GO_Profile(GetGWID(), NULL, g_nTID)<0 )				W_DBG("GmmpRecvThread : GO_Profile Error");
			}
			else if ( g_gmmp_step==3 )
			{
				if ( GO_Reg(GetGWID(), g_szDeviceID, false)!=GMMP_SUCCESS )	W_DBG("GmmpRecvThread : GO_Reg (Device) Error");
			}
			else if ( g_gmmp_step==-2 )
			{
				// sekim XXXXX g_gmmp_step critical error
				W_DBG("GmmpRecvThread : g_gmmp_step critical error");
				break;
			}
			else if ( g_gmmp_step==-3 )
			{
				// sekim XXXXX g_gmmp_step IDLE
				W_DBG("GmmpRecvThread : g_gmmp_step IDLE");
				break;
			}

			nRet = GetReadData(&stGMMPHeader, &pBody);
			if(nRet != GMMP_SUCCESS)
			{
				W_DBG("GmmpRecvThread : GetReadData error %d ", nRet);
				break;
			}
			if ( pBody!=NULL )	free(pBody);
			else				W_DBG("GmmpRecvThread : GetReadData error pBody Null");

			pBody = NULL;
		}

		GMMPClearSocket(1);
	}

	g_nGMMPThreadCount--;

	return;
}



int GmmpRecvCallback(GMMPHeader* pstGMMPHeader, void* pBody)
{
	uint8_t cMessageType = pstGMMPHeader->ucMessageType;

	if (cMessageType == OPERATION_GW_REG_RSP)
	{
		stGwRegistrationRspHdr* pstRspHdr =(stGwRegistrationRspHdr*) pBody;

		if (pstRspHdr->ucResultCode != 0x00)
		{
			W_DBG("GmmpRecvCallback : OPERATION_GW_REG_RSP Error(%d)", pstRspHdr->ucResultCode);
			g_gmmp_step = -2;
			return 1;
		}

		SetAuthKey((char*)pstGMMPHeader->usAuthKey);
		SetGWID((char*)pstRspHdr->usGWID);

		g_gmmp_step = 2;
	}
	else if(cMessageType == OPERATION_PROFILE_RSP)
	{
		stProfileRspHdr* pstRspHdr =(stProfileRspHdr*) pBody;

		if(pstRspHdr->ucResultCode != 0x00)
		{
			W_DBG("GmmpRecvCallback : OPERATION_PROFILE_RSP Error(%d)", pstRspHdr->ucResultCode);
			g_gmmp_step = -2;
			return 1;
		}

		g_nTimerDelivery = Char2int((char *)pstRspHdr->unReportPeriod, sizeof(pstRspHdr->unReportPeriod));
		g_nTimerHB = Char2int((char *)pstRspHdr->unHeartbeatPeriod, sizeof(pstRspHdr->unHeartbeatPeriod));

		g_gmmp_step = 3;
	}
	else if(cMessageType == OPERATION_DELIVERY_RSP)
	{
		stPacketDeliveryRspHdr* pstRspHdr =(stPacketDeliveryRspHdr*) pBody;

		if(pstRspHdr->ucResultCode != 0x00)
		{
			return 1;
		}
	}
	else if(cMessageType == OPERATION_DEVICE_REG_RSP)
	{
		stProfileRspHdr* pstRspHdr =(stProfileRspHdr*) pBody;

		if(pstRspHdr->ucResultCode != 0x00)
		{
			W_DBG("GmmpRecvCallback : OPERATION_DEVICE_REG_RSP Error(%d)", pstRspHdr->ucResultCode);
			g_gmmp_step = -2;
			return 1;
		}

		g_gmmp_step = 4;
	}
	else if(cMessageType == OPERATION_HEARTBEAT_RSP)
	{
	}
	else if(cMessageType == OPERATION_CONTROL_REQ)
	{
		stControlReqHdr* pstReqHdr = (stControlReqHdr*) pBody;

		U8 cControlType = pstReqHdr->ucControlType;

		char cResult = 0x00;

		int nTID_RECV = Char2int((char*)pstGMMPHeader->usTID, sizeof(pstGMMPHeader->usTID));

		GO_Control((char*)pstReqHdr->usGWID, (char*)pstReqHdr->usDeviceID, nTID_RECV, (char)pstReqHdr->ucControlType, cResult);

		W_RSP("\r\n[GMMP Control %d]\r\n", cControlType);

		GO_Notifi((char*)pstReqHdr->usGWID, (char*)pstReqHdr->usDeviceID, (char)pstReqHdr->ucControlType, cResult, NULL, 0);
	}
	else if(cMessageType == OPERATION_NOTIFICATION_RSP)
	{
	}

	return 0;
}

int GmmpWicedTCPSend(void* pData, int nLength)
{
	wiced_result_t result;

	if ( g_pGmmpSocket==0 )	return 0;

	result = wiced_tcp_send_buffer(g_pGmmpSocket, pData, nLength);
	if ( result!=WICED_SUCCESS )
	{
		W_DBG("GmmpWicedTCPSend : wiced_tcp_send_buffer error");
		wiced_tcp_disconnect(g_pGmmpSocket);
		return 0;
	}

	return nLength;
}

int GmmpWicedTCPRecv(void* pData, int nLength)
{
	wiced_result_t result;
	wiced_packet_t* rx_packet;

    char* rx_data;
	uint16_t rx_data_length;
	uint16_t available_data_length;

	uint8_t* pDesStart = pData;
	uint32_t nDesLen = nLength;

	if ( g_pGmmpSocket==0 )	return 0;

	if ( g_pGMMPTCPBuffer )
	{
		if ( nDesLen==g_nGMMPTCPBufferLen )
		{
			memcpy(pDesStart, g_pGMMPTCPBuffer, nDesLen);
			free(g_pGMMPTCPBuffer);
			g_pGMMPTCPBuffer = 0;
			g_nGMMPTCPBufferLen = 0;
			return nLength;
		}
		else if ( nDesLen<g_nGMMPTCPBufferLen )
		{
			memcpy(pDesStart, g_pGMMPTCPBuffer, nDesLen);

			uint8_t* pNew = malloc(g_nGMMPTCPBufferLen - nDesLen);
			memcpy(pNew, (g_pGMMPTCPBuffer + nDesLen), (g_nGMMPTCPBufferLen - nDesLen));

			free(g_pGMMPTCPBuffer);

			g_pGMMPTCPBuffer = pNew;
			g_nGMMPTCPBufferLen = g_nGMMPTCPBufferLen - nDesLen;
			return nLength;
		}
		else if ( nDesLen>g_nGMMPTCPBufferLen )
		{
			memcpy(pDesStart, g_pGMMPTCPBuffer, g_nGMMPTCPBufferLen);
			free(g_pGMMPTCPBuffer);

			pDesStart = pData + g_nGMMPTCPBufferLen;
			nDesLen = nDesLen - g_nGMMPTCPBufferLen;

			g_pGMMPTCPBuffer = 0;
			g_nGMMPTCPBufferLen = 0;
		}
	}

	uint8_t bDisconnect = 0;
	while(1)
	{
		result = wiced_tcp_receive(g_pGmmpSocket, &rx_packet, WICED_WAIT_FOREVER);

		if ( result==WICED_SUCCESS )
		{
			wiced_packet_get_data(rx_packet, 0, (uint8_t**)&rx_data, &rx_data_length, &available_data_length);

			if ( rx_data_length==nDesLen )
			{
				memcpy(pDesStart, rx_data, nDesLen);
				wiced_packet_delete(rx_packet);
				break;
			}
			else if ( rx_data_length>nDesLen )
			{
				memcpy(pDesStart, rx_data, nDesLen);

				uint8_t* pNew = malloc(rx_data_length - nDesLen);
				memcpy(pNew, (rx_data + nDesLen), (rx_data_length - nDesLen));

				g_pGMMPTCPBuffer = pNew;
				g_nGMMPTCPBufferLen = rx_data_length - nDesLen;

				wiced_packet_delete(rx_packet);
				break;
			}
			else if ( rx_data_length<nDesLen )
			{
				memcpy(pDesStart, rx_data, rx_data_length);

				pDesStart = pDesStart + rx_data_length;
				nDesLen = nDesLen - rx_data_length;

				wiced_packet_delete(rx_packet);
				continue;
			}
		}
		else if ( result==WICED_PENDING )
		{
			if ( g_pGmmpSocket->socket.nx_tcp_socket_state==NX_TCP_ESTABLISHED )
				continue;
		}
		else
		{
			bDisconnect = 1;
			break;
		}
	}

	if ( bDisconnect==1 )
	{
		W_DBG("GmmpWicedTCPRecv : wiced_tcp_receive error");
		return 0;
	}

	return nLength;
}


wiced_thread_t g_hGmmpRecvThread;
wiced_thread_t g_hGmmpSendThread;
int GMMPStopService()
{
	g_gmmp_step = -3;
	GMMPDisconnectSocket();
	//wiced_rtos_delay_milliseconds(1000);
	return 0;
}

int GMMPStartService(char* szServerIP, int nPort, char* szDomainCode, char* szGWAuthID, char* szGWMFID, char* szDeviceID, const int nNetwrokType)
{
	W_DBG("GMMPStartService : %s, %d, %s, %s, %s, %s, %d", szServerIP, nPort, szDomainCode, szGWAuthID, szGWMFID, szDeviceID, nNetwrokType);

	if ( g_gmmp_step>=0 )
	{
		W_DBG("GMMP service is already running.");
		return 1;
	}

	g_gmmp_step = 0;

	if ( Initializaion(szServerIP, nPort, szDomainCode, szGWAuthID, GMMP_ON_LOG, nErrorLevel, nNetwrokType, "Log")!=0 )
	{
		W_DBG("GMMPStartService : GMMP Init Error");
	}
	memcpy(g_szGWMFID, szGWMFID, strlen(szGWMFID));
	memcpy(g_szDeviceID, szDeviceID, strlen(szDeviceID));
	SetCallFunction(GmmpRecvCallback);

	if ( g_nGMMPThreadCount==0 )
	{
		wiced_rtos_create_thread(&g_hGmmpSendThread, WICED_APPLICATION_PRIORITY + 4, "Gmmp_Send_Thread", GmmpSendThread, 6*1024, NULL);
		wiced_rtos_create_thread(&g_hGmmpRecvThread, WICED_APPLICATION_PRIORITY + 5, "Gmmp_Recv_Thread ", GmmpRecvThread, 4*1024, NULL);
	}
	else if ( g_nGMMPThreadCount==2 )
	{
		W_DBG("GMMPStartService : GMMP Thread is running");
	}
	else
	{
		W_DBG("GMMPStartService : GMMP Thread Error");
		return 1;
	}

	return 0;
}

