/**
 * @date 2013/11/21
 * @version 0.0.0.1
 * @file GMMP_Reg.c
 **/

#include "GMMP_Reg.h"

//Request
int GMMP_GW_Reg_Req(GwRegist_Req* pRegist_Req)
{
	if(pRegist_Req == NULL)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	if(CheckSocket() != 0)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	return WriteTCP( (char*)pRegist_Req, sizeof(GwRegist_Req) );
}

int GMMP_GW_Reg_Rsp(GwRegist_Rsp* pRegist_Rsp)
{
	if(pRegist_Rsp == NULL)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	if(CheckSocket() != 0)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	int nRet = ReadTCP( (char*)&pRegist_Rsp->header  , sizeof(pRegist_Rsp->header) );
	if(nRet != GMMP_SUCCESS)
	{
		return nRet;
	}

	ConvertShort cvtshort;
	cvtshort.sU8 = 0;
	memcpy(cvtshort.usShort, pRegist_Rsp->header.usMessageLength, sizeof(pRegist_Rsp->header.usMessageLength) );

	int nReadSize = ltobs(cvtshort.sU8)- sizeof(pRegist_Rsp->header);

	return ReadTCP( (char*)&pRegist_Rsp->body, nReadSize);
}

int GMMP_Device_Reg_Req(DeviceRegist_Req* pRegist_Req)
{
	if(pRegist_Req == NULL)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	if(CheckSocket() != 0)
	{
		return SERVER_INFO_NOT_FOUND;
	}
	
	return WriteTCP( (char*)pRegist_Req, sizeof(DeviceRegist_Req) );
}

int GMMP_Device_Reg_Rsp(DeviceRegist_Rsp* pRegist_Rsp)
{
	if(pRegist_Rsp == NULL)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	if(CheckSocket() != 0)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	int nRet = ReadTCP( (char*)&pRegist_Rsp->header  , sizeof(pRegist_Rsp->header) );
	if(nRet != GMMP_SUCCESS)
	{
		return nRet;
	}

	ConvertShort cvtshort;
	cvtshort.sU8 = 0;
	memcpy(cvtshort.usShort, pRegist_Rsp->header.usMessageLength, sizeof(pRegist_Rsp->header.usMessageLength) );

	int nReadSize = ltobs(cvtshort.sU8)- sizeof(pRegist_Rsp->header);

	return ReadTCP( (char*)&pRegist_Rsp->body, nReadSize);
}


int GMMP_GW_DeReg_Req(GwDeRegist_Req* pDeRegist_Req)
{
	if(pDeRegist_Req == NULL)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	if(CheckSocket() != 0)
	{
		return SERVER_INFO_NOT_FOUND;
	}
	
	return WriteTCP( (char*)pDeRegist_Req, sizeof(GwDeRegist_Req) );
}

int GMMP_GW_DeReg_Rsp(GwDeRegist_Rsp* pDeRegist_Rsp)
{
	if(pDeRegist_Rsp == NULL)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	if(CheckSocket() != 0)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	int nRet = ReadTCP( (char*)&pDeRegist_Rsp->header  , sizeof(pDeRegist_Rsp->header) );
	if(nRet != GMMP_SUCCESS)
	{
		return nRet;
	}

	ConvertShort cvtshort;
	cvtshort.sU8 = 0;
	memcpy(cvtshort.usShort, pDeRegist_Rsp->header.usMessageLength, sizeof(pDeRegist_Rsp->header.usMessageLength) );

	int nReadSize = ltobs(cvtshort.sU8)- sizeof(pDeRegist_Rsp->header);

	return ReadTCP( (char*)&pDeRegist_Rsp->body, nReadSize);
}

int GMMP_Device_DeReg_Req(DeviceDeRegist_Req*  pDeRegist_Req)
{
	if(pDeRegist_Req == NULL)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	if(CheckSocket() != 0)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	
	return WriteTCP( (char*)pDeRegist_Req, sizeof(DeviceDeRegist_Req) );
}

int GMMP_Device_DeReg_Rsp(DeviceDeRegist_Rsp* pDeRegist_Req)
{
	if(pDeRegist_Req == NULL)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	if(CheckSocket() != 0)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	int nRet = ReadTCP( (char*)&pDeRegist_Req->header  , sizeof(pDeRegist_Req->header) );
	if(nRet != GMMP_SUCCESS)
	{
		return nRet;
	}

	ConvertShort cvtshort;
	cvtshort.sU8 = 0;
	memcpy(cvtshort.usShort, pDeRegist_Req->header.usMessageLength, sizeof(pDeRegist_Req->header.usMessageLength) );

	int nReadSize = ltobs(cvtshort.sU8)- sizeof(pDeRegist_Req->header);

	return ReadTCP( (char*)&pDeRegist_Req->body, nReadSize);
}
