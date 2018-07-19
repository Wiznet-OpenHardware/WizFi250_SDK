/**
 * @date 2013/11/21
 * @version 0.0.0.1
 * @file GMMP_ProfileInfo.c
 **/

#include "GMMP_ProfileInfo.h"

int GMMP_Profile_Req(Profile_Req*  pstProfile_Req)
{
	if(pstProfile_Req == NULL)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	if(CheckSocket() != 0)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	return WriteTCP( (char*)pstProfile_Req, sizeof(Profile_Req) );
}

int GMMP_Profile_Rsp(Profile_Rsp* pstProfile_Rsp)
{
	if(pstProfile_Rsp == NULL)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	if(CheckSocket() != 0)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	int nRet = ReadTCP( (char*)&pstProfile_Rsp->header  , sizeof(pstProfile_Rsp->header) );
	if(nRet != GMMP_SUCCESS)
	{
		return nRet;
	}

	ConvertShort cvtshort;
	cvtshort.sU8 = 0;
	memcpy(cvtshort.usShort, pstProfile_Rsp->header.usMessageLength, sizeof(pstProfile_Rsp->header.usMessageLength) );

	int nReadSize = ltobs(cvtshort.sU8)- sizeof(pstProfile_Rsp->header);

	return ReadTCP( (char*)&pstProfile_Rsp->body, nReadSize);
}
