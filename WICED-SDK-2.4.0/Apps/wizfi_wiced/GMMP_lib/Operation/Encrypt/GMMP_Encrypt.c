/**
 * @date 2013/11/21
 * @version 0.0.0.1
 * @file GMMP_Encrypt.c
 **/

#include "GMMP_Encrypt.h"

int GMMP_Encrypt_Info_Req(EncryptInfo_Req*  pEncryptInfo_Req)
{
	if(pEncryptInfo_Req == NULL)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	if(CheckSocket() != 0)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	return WriteTCP( (char*)pEncryptInfo_Req, sizeof(EncryptInfo_Req) );
}

int GMMP_Encrypt_Info_Rsp(EncryptInfo_Rsp*  pEncryptInfo_Rsp)
{
	if(pEncryptInfo_Rsp == NULL)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	if(CheckSocket() != 0)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	int nRet = ReadTCP( (char*)&pEncryptInfo_Rsp->header  , sizeof(pEncryptInfo_Rsp->header) );
	if(nRet != GMMP_SUCCESS)
	{
		return nRet;
	}

	ConvertShort cvtshort;
	cvtshort.sU8 = 0;
	memcpy(cvtshort.usShort, pEncryptInfo_Rsp->header.usMessageLength, sizeof(pEncryptInfo_Rsp->header.usMessageLength) );

	int nReadSize = ltobs(cvtshort.sU8)- sizeof(pEncryptInfo_Rsp->header);

	return ReadTCP( (char*)&pEncryptInfo_Rsp->body, nReadSize);
}

int GMMP_Encrypt_Key_Req(EncryptKey_Req*  pEncryptKey_Req)
{
	if(pEncryptKey_Req == NULL)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	if(CheckSocket() != 0)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	return WriteTCP( (char*)pEncryptKey_Req, sizeof(EncryptKey_Req) );
}

int GMMP_Encrypt_Key_Rsp(EncryptKey_Rsp*  pEncryptKey_Rsp)
{
	if(pEncryptKey_Rsp == NULL)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	if(CheckSocket() != 0)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	int nRet = ReadTCP( (char*)&pEncryptKey_Rsp->header  , sizeof(pEncryptKey_Rsp->header) );
	if(nRet != GMMP_SUCCESS)
	{
		return nRet;
	}

	ConvertShort cvtshort;
	cvtshort.sU8 = 0;
	memcpy(cvtshort.usShort, pEncryptKey_Rsp->header.usMessageLength, sizeof(pEncryptKey_Rsp->header.usMessageLength) );

	int nReadSize = ltobs(cvtshort.sU8)- sizeof(pEncryptKey_Rsp->header);

	return ReadTCP( (char*)&pEncryptKey_Rsp->body, nReadSize);
}
