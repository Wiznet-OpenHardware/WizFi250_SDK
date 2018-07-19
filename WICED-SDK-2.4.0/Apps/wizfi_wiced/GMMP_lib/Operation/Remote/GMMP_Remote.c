/**
 * @date 2013/11/21
 * @version 0.0.0.1
 * @file GMMP_Remote.c
 **/

#include "GMMP_Remote.h"


int GMMP_Remote_Req(Remote_Req*  pRemote_Req)
{
	if(pRemote_Req == NULL)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	if(CheckSocket() != 0)
	{
		return SERVER_INFO_NOT_FOUND;
	}
	
	return WriteTCP( (char*)pRemote_Req, sizeof(Remote_Req) );
}

int GMMP_Remote_Rsp(Remote_Rsp* pRemote_Rsp)
{
	if(pRemote_Rsp == NULL)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	if(CheckSocket() != 0)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	int nRet = ReadTCP( (char*)&pRemote_Rsp->header  , sizeof(pRemote_Rsp->header) );
	if(nRet != GMMP_SUCCESS)
	{
		return nRet;
	}

	ConvertShort cvtshort;
	cvtshort.sU8 = 0;
	memcpy(cvtshort.usShort, pRemote_Rsp->header.usMessageLength, sizeof(pRemote_Rsp->header.usMessageLength) );

	int nReadSize = ltobs(cvtshort.sU8)- sizeof(pRemote_Rsp->header);

	return ReadTCP( (char*)&pRemote_Rsp->body, nReadSize);
}
