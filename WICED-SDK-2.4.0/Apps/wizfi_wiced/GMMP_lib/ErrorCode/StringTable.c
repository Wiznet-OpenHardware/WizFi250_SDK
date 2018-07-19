/**
 * @date 2013/11/21
 * @version 0.0.0.1
 * @file StringTable.c
 **/


#include "StringTable.h"

//OMP Error Code to String
const char* pszUnknwonError = "UnKnown Error Code";
const char* pszOMPErrorcode[] ={
		"STATUS_OK",
		"STATUS_CREATED",
		"STATUS_ACCEPTED",
		"STATUS_BAD_REQUEST",
		"STATUS_PERMISSION_DENIED",
		"STATUS_FORBIDDEN",
		"STATUS_NOT_FOUND",
		"STATUS_METHOD_NOT_ALLOWED",
		"STATUS_NOT_ACCEPTABLE",
		"STATUS_REQUEST_TIMEOUT",
		"STATUS_CONFLICT",//10
		"STATUS_UNSUPPORTED_MEDIA_TYPE",
		"STATUS_INTERNAL_SERVER_ERROR",
		"STATUS_NOT_IMPLEMENTED",
		"STATUS_BAD_GATEWAY",
		"STATUS_SERVICE_UNAVAILABLE",
		"STATUS_GATEWAY_TIMEOUT",
		"STATUS_EXPIRED",
		"STATUS_SMS_SEND_FAIL"
};

const char* pszUser_Defiened_Result_Code0 = "UnDefined User_Defined_Result_Code";
const char* pszUser_Defiened_Result_Code1 = "STATUS_DB_TIMEOUT";
const char* pszUser_Defiened_Result_Code2 = "STATUS_MSPTCP_DISCONNECTED";
const char* pszUser_Defiened_Result_Code3 = "STATUS_GMMPTCP_PORT_CLOSED";
const char* pszUser_Defiened_Result_Code4 = "STAUS_AO_CONTROL_SEND_FAIL";

const char* pszLibErrorcode[] ={
		"GMMP_SUCCESS",
		"GMMP_ERROR_UNKWOUN",
		"GMMP_LOG_ERROR_OPEN",
		"GMMP_LOG_ERROR_WRITE",
		"GMMP_LOG_ERROR_CLOSE",
		"GMMP_LOG_NO_WRITE_DATA",
		"GMMP_LOG_LONG_STRING",
		"SERVER_INFO_NOT_FOUND",
		"SERVER_REQUEST_TIMEOUT",
		"SERVER_SOCKET_ERROR",
		"SERVER_CONNECT_ERROR",
		"SERVER_DISCONNECT",
		"LIB_PARAM_ERROR",
		"GMMP_MALLOC_ERROR",
		"GMMP_HEADER_SIZE_ERROR"
};


char* GetStringtoErrorCode(int nErrorCode)
{
	char* pReturnString = (char*)pszUnknwonError;

	nErrorCode -= GMMP_ERROR_DEFAULT;

	if(nErrorCode <  GMMP_ERRPR_MAX)
	{
		pReturnString = (char*)pszLibErrorcode[nErrorCode];
	}

	return pReturnString;
}

char* GetStringtoOMPErrorCode(const unsigned char  cOMPErrorCode)
{
	char* pReturnString = (char*)pszUnknwonError;

	if(STATUS_OK <= cOMPErrorCode && cOMPErrorCode <= STATUS_SMS_SEND_FAIL )
	{
		int nErrorCode = xtoi(cOMPErrorCode);

		pReturnString = (char*)pszOMPErrorcode[nErrorCode];
	}
	else if(User_Defined_Result_Code_Min <= cOMPErrorCode && cOMPErrorCode <= User_Defined_Result_Code_Max)
	{
		pReturnString = (char*)pszUser_Defiened_Result_Code0;

		if(cOMPErrorCode == STATUS_DB_TIMEOUT )
		{
			pReturnString = (char*)pszUser_Defiened_Result_Code1;
		}
		else if(cOMPErrorCode == STATUS_MSPTCP_DISCONNECTED )
		{
			pReturnString = (char*)pszUser_Defiened_Result_Code2;
		}
		else if(cOMPErrorCode == STATUS_GMMPTCP_PORT_CLOSED )
		{
			pReturnString = (char*)pszUser_Defiened_Result_Code3;
		}
		else if(cOMPErrorCode == STAUS_AO_CONTROL_SEND_FAIL )
		{
			pReturnString = (char*)pszUser_Defiened_Result_Code4;
		}
	}

	return pReturnString;
}

int xtoi(const char hex)
{
	ConvertInt cvtInt;
	memset(cvtInt.usInt, 0,  sizeof(cvtInt.usInt) );

	cvtInt.usInt[0] = hex;

  return cvtInt.sU8;
}


