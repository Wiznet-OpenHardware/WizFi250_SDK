/**
 * @date 2013/11/21
 * @version 0.0.0.1
 * @file GMMP_Heartbeat.c
 **/

#include "GMMP_Heartbeat.h"

int GMMP_Heartbeat_Req(HB_Req*  pHB_Req)
{
	if(pHB_Req == NULL)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	if(CheckSocket() != 0)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	return WriteTCP( (char*)pHB_Req, sizeof(HB_Req) );
}

int GMMP_Heartbeat_Rsp(HB_Rsp*  pHB_Rsp)
{
	if(pHB_Rsp == NULL)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	if(CheckSocket() != 0)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	int nRet = ReadTCP( (char*)&pHB_Rsp->header  , sizeof(pHB_Rsp->header) );
	if(nRet != GMMP_SUCCESS)
	{
		return nRet;
	}

	ConvertShort cvtshort;
	cvtshort.sU8 = 0;
	memcpy(cvtshort.usShort, pHB_Rsp->header.usMessageLength, sizeof(pHB_Rsp->header.usMessageLength) );

	int nReadSize = ltobs(cvtshort.sU8)- sizeof(pHB_Rsp->header);

	return ReadTCP( (char*)&pHB_Rsp->body, nReadSize);
}
