/**
 * @date 2013/11/21
 * @version 0.0.0.1
 * @file GMMP_Multimedia.c
 **/

#include "GMMP_Multimedia.h"

int GMMP_Multimedia_Req(MMURLInfo_Req*  pMMURLInfo_Req)
{
	if(pMMURLInfo_Req == NULL)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	if(CheckSocket() != 0)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	return WriteTCP( (char*)pMMURLInfo_Req, sizeof(MMURLInfo_Req) );
}

int GMMP_Multimedia_Rsp(MMURLInfo_Rsp* pMMURLInfo_Rsp)
{
	if(pMMURLInfo_Rsp == NULL)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	if(CheckSocket() != 0)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	int nRet = ReadTCP( (char*)&pMMURLInfo_Rsp->header  , sizeof(pMMURLInfo_Rsp->header) );
	if(nRet != GMMP_SUCCESS)
	{
		return nRet;
	}

	ConvertShort cvtshort;
	cvtshort.sU8 = 0;
	memcpy(cvtshort.usShort, pMMURLInfo_Rsp->header.usMessageLength, sizeof(pMMURLInfo_Rsp->header.usMessageLength) );

	int nReadSize = ltobs(cvtshort.sU8)- sizeof(pMMURLInfo_Rsp->header);

	return ReadTCP( (char*)&pMMURLInfo_Rsp->body, nReadSize);
}
