/**
 * @date 2013/11/21
 * @version 0.0.0.1
 * @file GMMP_Notification.c
 **/

#include "GMMP_Notification.h"

int GMMP_Notifi_Req(Notifi_Req*  pNotifi_Req, int nPacketSize)
{
	if(pNotifi_Req == NULL)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	if(CheckSocket() != 0)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	return WriteTCP( (char*)pNotifi_Req, nPacketSize );
}

int GMMP_Notifi_Rsp(Notifi_Rsp*  pNotifi_Rsp)
{
	if(pNotifi_Rsp == NULL)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	if(CheckSocket() != 0)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	int nRet = ReadTCP( (char*)&pNotifi_Rsp->header  , sizeof(pNotifi_Rsp->header) );
	if(nRet != GMMP_SUCCESS)
	{
		return nRet;
	}

	ConvertShort cvtshort;
	cvtshort.sU8 = 0;
	memcpy(cvtshort.usShort, pNotifi_Rsp->header.usMessageLength, sizeof(pNotifi_Rsp->header.usMessageLength) );

	int nReadSize = ltobs(cvtshort.sU8)- sizeof(pNotifi_Rsp->header);

	return ReadTCP( (char*)&pNotifi_Rsp->body, nReadSize);
}

