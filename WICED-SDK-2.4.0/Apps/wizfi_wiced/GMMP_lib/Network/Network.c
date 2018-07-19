/**
 * @date 2013/11/21
 * @version 0.0.0.1
 * @file Network.c
 **/
#include "../../wizfimain/wx_defines.h"

#include <stdio.h>
#include <string.h>
//#include <arpa/inet.h>
//#include <sys/socket.h>
#include <errno.h>
//#include <unistd.h>

// sekim 20151214 for Windows Compile : WINSOCK
//#include<winsock2.h>
//#pragma comment(lib, "ws2_32.lib")

#include "Network.h"
#include "../Operation/GMMP_Operation.h"

//int g_socket = -1;

void CloseSocket()
{
	///////////////////////////////////////////////////////////////////////////
	// sekim 201151222 remove g_socket, add GmmpWicedTCPSend, GmmpWicedTCPRecv
	/*
	if (g_socket <= 0) return;

	closesocket(g_socket);
	g_socket = -1;
	*/
	void GMMPDisconnectSocket();
	GMMPDisconnectSocket();
	///////////////////////////////////////////////////////////////////////////

	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "Close Socket : IP = %s, Port = %d\r\n",g_szServerIP, g_nServerPort);

	return;
}

int CheckSocket()
{
	return 0;
}


int WriteTCP(char* pBuf, int nLen)
{
	////////////////////////////////////////////////////////////////////////////////////
	// sekim 201151222 remove g_socket, add GmmpWicedTCPSend, GmmpWicedTCPRecv
	/*
	int nWriteLen = 0;
	int nWrittedLen = 0;

	if (pBuf == NULL || nLen <= 0) return LIB_PARAM_ERROR;

	do
	{
		nWriteLen = send(g_socket, pBuf+nWriteLen, nLen-nWriteLen, 0);
		if(nWriteLen <= 0)
		{
			g_socket = -1;
			CloseSocket();
			return SERVER_DISCONNECT;
		}

		nWrittedLen += nWriteLen;
	}while(nWrittedLen < nLen);
	*/
	int GmmpWicedTCPSend(void* pData, int nLength);
	if ( GmmpWicedTCPSend(pBuf, nLen)<=0 )
	{
		CloseSocket();
		return SERVER_DISCONNECT;
	}
	////////////////////////////////////////////////////////////////////////////////////

	return GMMP_SUCCESS;
}

void ClearBuffer()
{
	////////////////////////////////////////////////////////////////////////////////////
	// sekim 201151222 remove g_socket, add GmmpWicedTCPSend, GmmpWicedTCPRecv
	/*
	char Buffer[50] = {0,};
	int nReaded = 0;
	while(true)
	{
		nReaded = recv(g_socket, Buffer, 50, 0);
		if(nReaded < 50)
		{
			break;
		}

		// sekim 20151214 for Windows Compile : usleep -> SleepSeconds
		usleep(100);
	}
	*/
	////////////////////////////////////////////////////////////////////////////////////
}

int ReadTCP(char* pBuf, const int nMaxlen)
{
	////////////////////////////////////////////////////////////////////////////////////
	// sekim 201151222 remove g_socket, add GmmpWicedTCPSend, GmmpWicedTCPRecv
	/*
	if (pBuf == NULL || nMaxlen <= 0) return LIB_PARAM_ERROR;

	int nReadedLen = 0;
	int nReadLen = 0;

	memset(pBuf,  0, nMaxlen);

	while(nReadedLen < nMaxlen)
	{
		nReadLen  = recv(g_socket, &pBuf[nReadedLen], nMaxlen - nReadedLen, 0);

		if(nReadLen <= 0)
		{
			//g_socket = -1;
			CloseSocket();
			return SERVER_DISCONNECT;
		}

		nReadedLen += nReadLen;
	}
	*/
	int GmmpWicedTCPRecv(void* pData, int nLength);
	if ( GmmpWicedTCPRecv(pBuf, nMaxlen)<=0 )
	{
		return SERVER_DISCONNECT;
	}
	//////////////////////////////////////////////////////////////////////////////////////////////////

	return GMMP_SUCCESS;
}
