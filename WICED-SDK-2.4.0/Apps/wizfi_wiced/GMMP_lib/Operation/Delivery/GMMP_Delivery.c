/**
 * @date 2013/11/21
 * @version 0.0.0.1
 * @file GMMP_Delivery.c
 **/

#include "GMMP_Delivery.h"

int GMMP_Delivery_Req(Delivery_Req*  pDelivery_Req, int nPacketSize)
{
	if(pDelivery_Req == NULL)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	if(CheckSocket() != 0)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	return WriteTCP( (char*)pDelivery_Req, nPacketSize );
}

int GMMP_Delivery_Rsp(Delivery_Rsp*  pDelivery_Rsp)
{
	if(pDelivery_Rsp == NULL)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	if(CheckSocket() != 0)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	int nRet = ReadTCP( (char*)&pDelivery_Rsp->header  , sizeof(pDelivery_Rsp->header) );
	if(nRet != GMMP_SUCCESS)
	{
		return nRet;
	}

	ConvertShort cvtshort;
	cvtshort.sU8 = 0;
	memcpy(cvtshort.usShort, pDelivery_Rsp->header.usMessageLength, sizeof(pDelivery_Rsp->header.usMessageLength) );

	int nReadSize = ltobs(cvtshort.sU8)- sizeof(pDelivery_Rsp->header);

	return ReadTCP( (char*)&pDelivery_Rsp->body, nReadSize);
}

