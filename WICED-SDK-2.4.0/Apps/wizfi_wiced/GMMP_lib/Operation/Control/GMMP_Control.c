/**
 * @date 2013/11/21
 * @version 0.0.0.1
 * @file GMMP_Control.c
 **/

#include "GMMP_Control.h"

int GMMP_Control_Req(Control_Req*  pControl_Req)
{
	if(pControl_Req == NULL)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	if(CheckSocket() != 0)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	int nRet = ReadTCP( (char*)&pControl_Req->header  , sizeof(pControl_Req->header) );
	if(nRet == GMMP_SUCCESS)
	{
		return nRet;
	}

	ConvertShort cvtshort;
	cvtshort.sU8 = 0;
	memcpy(cvtshort.usShort, pControl_Req->header.usMessageLength, sizeof(pControl_Req->header.usMessageLength) );

	int nReadSize = ltobs(cvtshort.sU8) - sizeof(pControl_Req->header) ;

	return ReadTCP( (char*)&pControl_Req->body, nReadSize);
}

int GMMP_Control_Rsp(Control_Rsp*  pControl_Rsp)
{
	if(pControl_Rsp == NULL)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	if(CheckSocket() != 0)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	return WriteTCP( (char*)pControl_Rsp, sizeof(Control_Rsp) );
}


