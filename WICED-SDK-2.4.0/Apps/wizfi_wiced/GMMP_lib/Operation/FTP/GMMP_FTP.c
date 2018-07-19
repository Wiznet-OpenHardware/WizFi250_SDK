/**
 * @date 2013/11/21
 * @version 0.0.0.1
 * @file GMMP_FTP.c
 **/

#include "GMMP_FTP.h"

int GMMP_FTP_Info_Req(FTP_Req*  pFTP_Req)
{
	if(pFTP_Req == NULL)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	if(CheckSocket() != 0)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	return WriteTCP( (char*)pFTP_Req, sizeof(FTP_Req) );
}

int GMMP_FTP_Info_Rsp(FTP_Rsp*  pFTP_Rsp)
{
	if(pFTP_Rsp == NULL)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	if(CheckSocket() != 0)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	int nRet = ReadTCP( (char*)&pFTP_Rsp->header  , sizeof(pFTP_Rsp->header) );
	if(nRet != GMMP_SUCCESS)
	{
		return nRet;
	}

	ConvertShort cvtshort;
	cvtshort.sU8 = 0;
	memcpy(cvtshort.usShort, pFTP_Rsp->header.usMessageLength, sizeof(pFTP_Rsp->header.usMessageLength) );

	int nReadSize = ltobs(cvtshort.sU8)- sizeof(pFTP_Rsp->header);

	return ReadTCP( (char*)&pFTP_Rsp->body, nReadSize);
}
