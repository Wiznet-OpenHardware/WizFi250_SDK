/**
 * @date 2013/11/21
 * @version 0.0.0.1
 * @file GMMP_LSentence.c
 **/

#include "GMMP_LSentence.h"

int GMMP_LSentence_Req(LSentence_Req*  pLSentence_Req)
{
	if(pLSentence_Req == NULL)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	if(CheckSocket() != 0)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	return WriteTCP( (char*)pLSentence_Req, sizeof(LSentence_Req) );
}

int GMMP_LSentence_Rsp(LSentence_Rsp*  pLSentence_Rsp)
{
	if(pLSentence_Rsp == NULL)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	if(CheckSocket() != 0)
	{
		return SERVER_INFO_NOT_FOUND;
	}

	return ReadTCP( (char*)pLSentence_Rsp, sizeof(LSentence_Rsp) );
}
