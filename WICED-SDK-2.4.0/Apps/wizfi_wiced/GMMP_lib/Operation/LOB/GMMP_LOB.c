/**
 * @date 2013/11/21
 * @version 0.0.0.1
 * @file GMMP_LOB.c
 **/

#include "GMMP_LOB.h"

int GMMP_LOB_Cloud_Info_Req(LOBCloud_Req*  pLOBCloud_Req)
{
	if(pLOBCloud_Req == NULL)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	if(CheckSocket() != 0)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	return WriteTCP( (char*)pLOBCloud_Req, sizeof(LOBCloud_Req) );
}

int GMMP_LOB_Cloud_Info_Rsp(LOBCloud_Rsp*  pLOBCloud_Rsp)
{
	if(pLOBCloud_Rsp == NULL)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	if(CheckSocket() != 0)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	int nRet = ReadTCP( (char*)&pLOBCloud_Rsp->header  , sizeof(pLOBCloud_Rsp->header) );
	if(nRet != GMMP_SUCCESS)
	{
		return nRet;
	}

	ConvertShort cvtshort;
	cvtshort.sU8 = 0;
	memcpy(cvtshort.usShort, pLOBCloud_Rsp->header.usMessageLength, sizeof(pLOBCloud_Rsp->header.usMessageLength) );

	int nReadSize = ltobs(cvtshort.sU8)- sizeof(pLOBCloud_Rsp->header);

	return ReadTCP( (char*)&pLOBCloud_Rsp->body, nReadSize);
}

int GMMP_LOB_FTP_Info_Req(LOBFTP_Req*  pLOBFTP_Req)
{
	if(pLOBFTP_Req == NULL)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	if(CheckSocket() != 0)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	return WriteTCP( (char*)pLOBFTP_Req, sizeof(LOBFTP_Req) );
}

int GMMP_LOB_FTP_Info_Rsp(LOBFTP_Rsp*  pLOBFTP_Rsp)
{
	if(pLOBFTP_Rsp == NULL)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	if(CheckSocket() != 0)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	int nRet = ReadTCP( (char*)&pLOBFTP_Rsp->header  , sizeof(pLOBFTP_Rsp->header) );
	if(nRet != GMMP_SUCCESS)
	{
		return nRet;
	}

	ConvertShort cvtshort;
	cvtshort.sU8 = 0;
	memcpy(cvtshort.usShort, pLOBFTP_Rsp->header.usMessageLength, sizeof(pLOBFTP_Rsp->header.usMessageLength) );

	int nReadSize = ltobs(cvtshort.sU8)- sizeof(pLOBFTP_Rsp->header);

	return ReadTCP( (char*)&pLOBFTP_Rsp->body, nReadSize);
}

int GMMP_LOB_Notification_Req(LOBNotifi_Req*  pLOBNotifi_Req)
{
	if(pLOBNotifi_Req == NULL)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	if(CheckSocket() != 0)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	return WriteTCP( (char*)pLOBNotifi_Req, sizeof(LOBNotifi_Req) );
}

int GMMP_LOB_Notification_Rsp(LOBNotifi_Rsp*  pLOBNotifi_Rsp)
{
	if(pLOBNotifi_Rsp == NULL)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	if(CheckSocket() != 0)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	int nRet = ReadTCP( (char*)&pLOBNotifi_Rsp->header  , sizeof(pLOBNotifi_Rsp->header) );
	if(nRet != GMMP_SUCCESS)
	{
		return nRet;
	}

	ConvertShort cvtshort;
	cvtshort.sU8 = 0;
	memcpy(cvtshort.usShort, pLOBNotifi_Rsp->header.usMessageLength, sizeof(pLOBNotifi_Rsp->header.usMessageLength) );

	int nReadSize = ltobs(cvtshort.sU8)- sizeof(pLOBNotifi_Rsp->header);

	return ReadTCP( (char*)&pLOBNotifi_Rsp->body, nReadSize);
}
