/**
 * @date 2013/11/21
 * @version 0.0.0.1
 * @file GMMP_Operation.c
 **/

#include "../../wizfimain/wx_defines.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <time.h>
#include <signal.h>

//#include <pthread.h>
//#include <unistd.h>
#include <stdarg.h>
#include "GMMP_Operation.h"

#include "../define/Define_Control.h"

int g_nTID = 0;
int g_bLog = false;

int g_nErrorLevel = GMMP_ERROR_LEVEL_ERROR;


//GW/Device Request
int GMMP_SetReg(const char* pszAuthID,
		const char* pszAuthKey,
		const char* pszDomainCode,
		const char* pszGWID,
		const char* pszManufactureID)
{
	if(pszAuthID == NULL
			|| pszDomainCode == NULL
			|| pszManufactureID == NULL
			|| strlen(pszAuthID) > LEN_AUTH_ID
			|| strlen(pszDomainCode) > LEN_DOMAIN_CODE
			|| strlen(pszManufactureID) > LEN_MANUFACTURE_ID)
	{
		GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR, "Code[%d], %s\r\n", LIB_PARAM_ERROR, GetStringtoErrorCode(LIB_PARAM_ERROR) );
		return LIB_PARAM_ERROR;
	}

	int nRet = GMMP_SUCCESS;

	if(pszGWID == NULL) //GW  Reg
	{
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_OPS, "[GW->OMP]GW GMMP_SetReg Start\r\n");

		GwRegist_Req stRegist_Req;
		memset(&stRegist_Req, 0 ,sizeof(stRegist_Req) );

		nRet = SetHeader((void*)&stRegist_Req, sizeof(stRegist_Req), 1,  1, OPERATION_GW_REG_REQ, pszAuthID, NULL, GMMP_ENCRYPTION_NOT);
		if(nRet != GMMP_SUCCESS)
		{
			GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR, "Code[%d], %s\r\n", nRet, GetStringtoErrorCode(nRet) );
			return nRet;
		}

		memcpy(stRegist_Req.body.usDomainCode, pszDomainCode, strlen(pszDomainCode) );
		memcpy(stRegist_Req.body.usManufactureID, pszManufactureID, strlen(pszManufactureID) );

		GMMP_Trace(&stRegist_Req.header, &stRegist_Req.body);

		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_REQ, "\r\n");

		nRet = GMMP_GW_Reg_Req((GwRegist_Req*)&stRegist_Req);

		if(nRet != GMMP_SUCCESS)
		{
			GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR, "Code[%d], %s\r\n", nRet, GetStringtoErrorCode(nRet) );
		}

		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "[GW->OMP]GW GMMP_SetReg End \r\n");
	}
	else
	{
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_OPS, "[GW->OMP]Device GMMP_SetReg Start\r\n");

		if(pszAuthKey == NULL || strlen(pszAuthKey) > LEN_AUTH_KEY || strlen(pszGWID) > LEN_GW_ID)
		{
			GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR,"Code[%d], %s\r\n", LIB_PARAM_ERROR, GetStringtoErrorCode(LIB_PARAM_ERROR) );
			return LIB_PARAM_ERROR;
		}

		DeviceRegist_Req stRegist_Req;

		memset(&stRegist_Req, 0 ,sizeof(stRegist_Req) );

		nRet = SetHeader((void*)&stRegist_Req, sizeof(stRegist_Req), 1,  1, OPERATION_DEVICE_REG_REQ, pszAuthID, pszAuthKey, GMMP_ENCRYPTION_NOT);
		if(nRet != GMMP_SUCCESS)
		{
			GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR,"Code[%d], %s\r\n", nRet, GetStringtoErrorCode(nRet) );
			return nRet;
		}

		memcpy(stRegist_Req.body.usDomainCode, pszDomainCode, strlen(pszDomainCode) );
		memcpy(stRegist_Req.body.usGWID, pszGWID, strlen(pszGWID) );
		memcpy(stRegist_Req.body.usManufactureID, pszManufactureID, strlen(pszManufactureID) );

		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_REQ, "\r\n");

		GMMP_Trace(&stRegist_Req.header, &stRegist_Req.body);

		nRet =  GMMP_Device_Reg_Req((DeviceRegist_Req*)&stRegist_Req);

		if(nRet != GMMP_SUCCESS)
		{
			GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR,"Code[%d], %s\r\n", nRet, GetStringtoErrorCode(nRet) );
		}

		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "[GW->OMP]Device GMMP_SetReg End \r\n");
	}

	return nRet;
}

int GMMP_SetDeReg(const char* pszAuthID,
		const char* pszAuthKey,
		const char* pszDomainCode,
		const char* pszGWID,
		const char* pszDeviceID)
{
	if(pszAuthID == NULL
			|| pszAuthKey == NULL
			|| pszDomainCode == NULL
			|| pszGWID == NULL
			|| strlen(pszAuthID) > LEN_AUTH_ID
			|| strlen(pszAuthKey) > LEN_AUTH_KEY
			|| strlen(pszDomainCode) > LEN_DOMAIN_CODE
			|| strlen(pszGWID) > LEN_GW_ID)
	{
		GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR,"Code[%d], %s\r\n", LIB_PARAM_ERROR, GetStringtoErrorCode(LIB_PARAM_ERROR) );

		return LIB_PARAM_ERROR;
	}

	int nRet = GMMP_SUCCESS;

	if(pszDeviceID == NULL) //GW DeReg
	{
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_OPS, "[GW->OMP]GW GMMP_SetDeReg Start \r\n");

		GwDeRegist_Req stDeRegist_Req;
		memset(&stDeRegist_Req, 0 ,sizeof(stDeRegist_Req) );

		nRet = SetHeader((void*)&stDeRegist_Req,  sizeof(stDeRegist_Req), 1, 1,  OPERATION_GW_DEREG_REQ, pszAuthID, pszAuthKey, GMMP_ENCRYPTION_NOT);
		if(nRet != GMMP_SUCCESS)
		{
			GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR,"Code[%d], %s\r\n", nRet, GetStringtoErrorCode(nRet) );
			return nRet;
		}

		memcpy(stDeRegist_Req.body.usDomainCode, pszDomainCode, strlen(pszDomainCode) );
		memcpy(stDeRegist_Req.body.usGWID, pszGWID, strlen(pszGWID) );

		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_REQ, "\r\n");

		GMMP_Trace(&stDeRegist_Req.header, &stDeRegist_Req.body);

		nRet  =  GMMP_GW_DeReg_Req((GwDeRegist_Req*)&stDeRegist_Req);
		if(nRet != GMMP_SUCCESS)
		{
			GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR,"Code[%d], %s\r\n", nRet, GetStringtoErrorCode(nRet) );
		}

		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "[GW->OMP]GW GMMP_SetDeReg End \r\n");
	}
	else
	{
		if(strlen(pszDeviceID) > LEN_DEVICE_ID)
		{
			GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR,"Code[%d], %s\r\n", LIB_PARAM_ERROR, GetStringtoErrorCode(LIB_PARAM_ERROR) );
		}

		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_OPS,"[GW->OMP]Device GMMP_SetDeReg Start \r\n");

		DeviceDeRegist_Req stDeviceDeRegist_Req;

		memset(&stDeviceDeRegist_Req, 0 ,sizeof(stDeviceDeRegist_Req) );

		nRet = SetHeader((void*)&stDeviceDeRegist_Req, sizeof(stDeviceDeRegist_Req), 1,  1, OPERATION_DEVICE_DEREG_REQ, pszAuthID, pszAuthKey, GMMP_ENCRYPTION_NOT);
		if(nRet != GMMP_SUCCESS)
		{
			GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR,"Code[%d], %s\r\n", nRet, GetStringtoErrorCode(nRet) );
			return nRet;
		}

		memcpy(stDeviceDeRegist_Req.body.usDomainCode, pszDomainCode, strlen(pszDomainCode) );
		memcpy(stDeviceDeRegist_Req.body.usGWID, pszGWID, strlen(pszGWID) );
		memcpy(stDeviceDeRegist_Req.body.usDeviceID, pszDeviceID, strlen(pszDeviceID) );

		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_REQ, "\r\n");

		GMMP_Trace(&stDeviceDeRegist_Req.header, &stDeviceDeRegist_Req.body);

		nRet =  GMMP_Device_DeReg_Req((DeviceDeRegist_Req*)&stDeviceDeRegist_Req);
		if(nRet != GMMP_SUCCESS)
		{
			GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR,"Code[%d], %s\r\n", nRet, GetStringtoErrorCode(nRet) );
		}

		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "[GW->OMP]Device GMMP_SetDeReg End \r\n");
	}

	return nRet;
}

int GMMP_SetEncryptInfo(const char* pszAuthID,
		const char* pszAuthKey,
		const char* pszDomainCode,
		const char* pszGWID,
		const char* pszDeviceID)
{
	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_OPS,"[GW->OMP]GMMP_SetEncryptInfo Start \r\n");

	if(pszAuthID == NULL
			|| pszAuthKey == NULL
			|| pszDomainCode == NULL
			|| pszGWID == NULL
			|| strlen(pszAuthID) > LEN_AUTH_ID
			|| strlen(pszAuthKey) > LEN_AUTH_KEY
			|| strlen(pszDomainCode) > LEN_DOMAIN_CODE
			|| strlen(pszGWID) > LEN_GW_ID)
	{
		GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR,"Code[%d], %s\r\n", LIB_PARAM_ERROR, GetStringtoErrorCode(LIB_PARAM_ERROR) );
		return LIB_PARAM_ERROR;
	}

	int nRet = GMMP_SUCCESS;

	EncryptInfo_Req  stEncryptInfo_Req;

	memset(&stEncryptInfo_Req, 0 ,sizeof(stEncryptInfo_Req) );

	nRet = SetHeader((void*)&stEncryptInfo_Req, sizeof(stEncryptInfo_Req), 1,  1, OPERATION_ENCRYPTION_INFO_REQ, pszAuthID, pszAuthKey, GMMP_ENCRYPTION_NOT);
	if(nRet != GMMP_SUCCESS)
	{
		GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR,"Code[%d], %s\r\n", nRet, GetStringtoErrorCode(nRet) );
		return nRet;
	}

	memcpy(stEncryptInfo_Req.body.usDomainCode, pszDomainCode, strlen(pszDomainCode) );
	memcpy(stEncryptInfo_Req.body.usGWID, pszGWID, strlen(pszGWID) );

	if(pszDeviceID != NULL) //Device �뙋�떒
	{
		if( strlen(pszDeviceID) > LEN_DEVICE_ID) //Device ID 湲몄씠 �솗�씤
		{
			GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR,"Code[%d], %s\r\n", LIB_PARAM_ERROR, GetStringtoErrorCode(LIB_PARAM_ERROR) );
		}
		else
		{
			memcpy(stEncryptInfo_Req.body.usDeviceID, pszDeviceID, strlen(pszDeviceID) );
		}
	}

	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_REQ, "\r\n");

	GMMP_Trace(&stEncryptInfo_Req.header, &stEncryptInfo_Req.body);

	nRet =  GMMP_Encrypt_Info_Req(&stEncryptInfo_Req);

	if(nRet != GMMP_SUCCESS)
	{
		GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR,"Code[%d], %s\r\n", nRet, GetStringtoErrorCode(nRet) );
	}

	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "[GW->OMP]GMMP_SetEncryptInfo End \r\n");

	return nRet;
}

int GMMP_SetEncryptKey(const char* pszAuthID,
		const char* pszAuthKey,
		const char* pszDomainCode,
		const char* pszGWID,
		const char* pszDeviceID,
		const char* pszEncryptionKey)
{
	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_OPS,"[GW->OMP]GMMP_SetEncryptKey Start \r\n");

	if(pszAuthID == NULL
			|| pszAuthKey == NULL
			|| pszDomainCode == NULL
			|| pszGWID == NULL
			|| pszEncryptionKey == NULL
			|| strlen(pszAuthID) > LEN_AUTH_ID
			|| strlen(pszAuthKey) > LEN_AUTH_KEY
			|| strlen(pszDomainCode) > LEN_DOMAIN_CODE
			|| strlen(pszGWID) > LEN_GW_ID
			|| strlen(pszEncryptionKey) > LEN_ENCRYTION_KEY)
	{
		GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR,"Code[%d], %s\r\n", LIB_PARAM_ERROR, GetStringtoErrorCode(LIB_PARAM_ERROR) );
		return LIB_PARAM_ERROR;
	}

	int nRet = GMMP_SUCCESS;

	EncryptKey_Req  stEncryptKey_Req;

	memset(&stEncryptKey_Req, 0 ,sizeof(stEncryptKey_Req) );

	nRet = SetHeader((void*)&stEncryptKey_Req, sizeof(stEncryptKey_Req), 1,  1, OPERATION_ENCRYPTION_KEY_REQ, pszAuthID, pszAuthKey, GMMP_ENCRYPTION_NOT);
	if(nRet != GMMP_SUCCESS)
	{
		GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR,"Code[%d], %s\r\n", nRet, GetStringtoErrorCode(nRet) );
		return nRet;
	}

	memcpy(stEncryptKey_Req.body.usDomainCode, pszDomainCode, strlen(pszDomainCode) );
	memcpy(stEncryptKey_Req.body.usGWID, pszGWID, strlen(pszGWID) );
	memcpy(stEncryptKey_Req.body.usEncryptionKey, pszEncryptionKey, strlen(pszEncryptionKey) );

	if(pszDeviceID != NULL) //Device �뙋�떒
	{
		if( strlen(pszDeviceID) > LEN_DEVICE_ID) //Device ID 湲몄씠 �솗�씤
		{
			GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR,"Code[%d], %s\r\n", LIB_PARAM_ERROR, GetStringtoErrorCode(LIB_PARAM_ERROR) );
		}
		else
		{
			memcpy(stEncryptKey_Req.body.usDeviceID, pszDeviceID, strlen(pszDeviceID) );
		}
	}

	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_REQ, "\r\n");

	GMMP_Trace(&stEncryptKey_Req.header, &stEncryptKey_Req.body);

	nRet = GMMP_Encrypt_Key_Req(&stEncryptKey_Req);
	if(nRet  != GMMP_SUCCESS)
	{
		GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR,"Code[%d], %s\r\n", nRet, GetStringtoErrorCode(nRet) );
	}

	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "[GW->OMP]GMMP_SetEncryptKey End \r\n");

	return nRet;
}

//Profile Info Request
int GMMP_SetProfile(const char* pszAuthID, const char* pszAuthKey, const char* pszDomainCode, const char* pszGWID, const char* pszDeviceID)
{
	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_OPS,"[GW->OMP]GMMP_SetProfile Start \r\n");

	if(pszAuthID == NULL
			|| pszAuthKey == NULL
			|| pszDomainCode == NULL
			|| pszGWID ==NULL
			|| strlen(pszAuthID) > LEN_AUTH_ID
			|| strlen(pszAuthKey) > LEN_AUTH_KEY
			|| strlen(pszDomainCode) > LEN_DOMAIN_CODE
			|| strlen(pszGWID) > LEN_GW_ID)
	{
		GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR,"Code[%d], %s\r\n", LIB_PARAM_ERROR, GetStringtoErrorCode(LIB_PARAM_ERROR) );
		return LIB_PARAM_ERROR;
	}

	int nRet = GMMP_SUCCESS;

	Profile_Req  stProfile_Req;

	memset(&stProfile_Req, 0 ,sizeof(stProfile_Req) );

	nRet = SetHeader((void*)&stProfile_Req, sizeof(stProfile_Req), 1,  1, OPERATION_PROFILE_REQ, pszAuthID, pszAuthKey, GMMP_ENCRYPTION_NOT);
	if(nRet != GMMP_SUCCESS)
	{
		GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR,"Code[%d], %s\r\n", nRet, GetStringtoErrorCode(nRet) );
		return nRet;
	}

	memcpy(stProfile_Req.body.usDomainCode, pszDomainCode, strlen(pszDomainCode) );
	memcpy(stProfile_Req.body.usGWID, pszGWID, strlen(pszGWID) );

	if(pszDeviceID != NULL) //Device �뙋�떒
	{
		if( strlen(pszDeviceID) > LEN_DEVICE_ID) //Device ID 湲몄씠 �솗�씤
		{
			GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR,"Code[%d], %s\r\n", LIB_PARAM_ERROR, GetStringtoErrorCode(LIB_PARAM_ERROR) );
		}
		else
		{
			memcpy(stProfile_Req.body.usDeviceID, pszDeviceID, strlen(pszDeviceID) );
		}
	}

	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_REQ, "\r\n");

	GMMP_Trace(&stProfile_Req.header, &stProfile_Req.body);

	nRet =  GMMP_Profile_Req(&stProfile_Req);
	if(nRet != GMMP_SUCCESS)
	{
		GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR,"Code[%d], %s\r\n", nRet, GetStringtoErrorCode(nRet) );
	}

	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "[GW->OMP]GMMP_SetProfile End \r\n");

	return nRet;
}

//�닔吏� �뜲�씠�꽣 蹂닿퀬  Request
int GMMP_SetDelivery(const char* pszAuthID,
		const char* pszAuthKey,
		const char* pszDomainCode,
		const char* pszGWID,
		const char* pszDeviceID,
		const char cReportType,
		const char cMediaType,
		const char* pszMessageBody,
		const int nTotalCount,
		const int nCurrentCount,
		const char cEncryption)
{
	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_OPS,"[GW->OMP]GMMP_SetDelivery Start \r\n");

	if(pszAuthID == NULL
			|| pszAuthKey == NULL
			|| pszDomainCode == NULL
			|| pszGWID == NULL
			|| strlen(pszAuthID) > LEN_AUTH_ID
			|| strlen(pszAuthKey) > LEN_AUTH_KEY
			|| strlen(pszDomainCode) > LEN_DOMAIN_CODE
			|| strlen(pszGWID) > LEN_GW_ID
			|| cReportType < 0x00
			|| cReportType > 0x04
			|| cMediaType < 0x01
			|| pszMessageBody == NULL
			|| strlen(pszMessageBody) > MAX_MSG_BODY)
	{
		GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR,"Code[%d], %s\r\n", LIB_PARAM_ERROR, GetStringtoErrorCode(LIB_PARAM_ERROR) );
		return LIB_PARAM_ERROR;
	}

	int nMessageBodyLen = strlen(pszMessageBody);

	int nRet = GMMP_SUCCESS;

	Delivery_Req  stDelivery_Req;

	memset(&stDelivery_Req, 0 ,sizeof(stDelivery_Req) );

	int PacketSize = sizeof(stDelivery_Req) - MAX_MSG_BODY + nMessageBodyLen ; //Message Body�쓽 �냽�꽦�� Optinal�씠�떎

	nRet = SetHeader((void*)&stDelivery_Req, PacketSize, nTotalCount,  nCurrentCount, OPERATION_DELIVERY_REQ, pszAuthID, pszAuthKey, cEncryption);
	if(nRet != GMMP_SUCCESS)
	{
		GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR,"Code[%d], %s\r\n", nRet, GetStringtoErrorCode(nRet) );
		return nRet;
	}

	memcpy(stDelivery_Req.body.usDomainCode, pszDomainCode, strlen(pszDomainCode) );
	memcpy(stDelivery_Req.body.usGWID, pszGWID, strlen(pszGWID) );

	if(pszDeviceID != NULL) //Device �뙋�떒
	{
		if( strlen(pszDeviceID) > LEN_DEVICE_ID) //Device ID 湲몄씠 �솗�씤
		{
			GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR,"Code[%d], %s\r\n", LIB_PARAM_ERROR, GetStringtoErrorCode(LIB_PARAM_ERROR) );
		}
		else
		{
			memcpy(stDelivery_Req.body.usDeviceID, pszDeviceID, strlen(pszDeviceID) );
		}
	}

	stDelivery_Req.body.ucReportType = cReportType;
	stDelivery_Req.body.ucMediaType = cMediaType;

	if(nMessageBodyLen > 0)
	{
		memcpy(stDelivery_Req.body.usMessageBody, pszMessageBody, nMessageBodyLen );
	}

	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_REQ, "\r\n");

	GMMP_Trace(&stDelivery_Req.header, &stDelivery_Req.body);

	nRet =  GMMP_Delivery_Req(&stDelivery_Req, PacketSize);
	if(nRet != GMMP_SUCCESS)
	{
		GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR,"Code[%d], %s\r\n", nRet, GetStringtoErrorCode(nRet) );
	}

	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "[GW->OMP]GMMP_SetDelivery End \r\n");

	return nRet;
}

//�젣�뼱 �닔�떊 湲곕뒫�� Thread�뿉�꽌 泥섎━
//�젣�뼱 �닔�떊 蹂닿퀬  Response
int GMMP_SetControl(const char* pszAuthID, const char* pszAuthKey, const char* pszDomainCode, const char* pszGWID, const char* pszDeviceID, const char cControlType, const char cResultCode)
{
	if(pszAuthID == NULL
			|| pszAuthKey == NULL
			|| pszDomainCode == NULL
			|| pszGWID ==NULL
			|| strlen(pszAuthID) > LEN_AUTH_ID
			|| strlen(pszAuthKey) > LEN_AUTH_KEY
			|| strlen(pszDomainCode) > LEN_DOMAIN_CODE
			|| strlen(pszGWID) > LEN_GW_ID)
	{
		GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR,"Code[%d], %s\r\n", LIB_PARAM_ERROR, GetStringtoErrorCode(LIB_PARAM_ERROR) );
		return LIB_PARAM_ERROR;
	}

	int nRet = GMMP_SUCCESS;

	Control_Rsp stControl_Rsp;

	memset(&stControl_Rsp, 0 ,sizeof(stControl_Rsp) );

	nRet = SetHeader((void*)&stControl_Rsp, sizeof(stControl_Rsp), 1,  1, OPERATION_CONTROL_RSP, pszAuthID, pszAuthKey, GMMP_ENCRYPTION_NOT);
	if(nRet != GMMP_SUCCESS)
	{
		GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR,"Code[%d], %s\r\n", nRet, GetStringtoErrorCode(nRet) );
		return nRet;
	}

	memcpy(stControl_Rsp.body.usDomainCode, pszDomainCode, strlen(pszDomainCode) );
	memcpy(stControl_Rsp.body.usGWID, pszGWID, strlen(pszGWID) );

	if(pszDeviceID != NULL) //Device �뙋�떒
	{
		if( strlen(pszDeviceID) > LEN_DEVICE_ID) //Device ID 湲몄씠 �솗�씤
		{
			GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR,"Code[%d], %s\r\n", LIB_PARAM_ERROR, GetStringtoErrorCode(LIB_PARAM_ERROR) );
		}
		else
		{
			memcpy(stControl_Rsp.body.usDeviceID, pszDeviceID, strlen(pszDeviceID) );
		}
	}

	stControl_Rsp.body.ucControlType = cControlType;
	stControl_Rsp.body.ucResultCode = cResultCode;

	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_RSP,"\r\n");

	GMMP_Trace(&stControl_Rsp.header, &stControl_Rsp.body);

	nRet =  GMMP_Control_Rsp(&stControl_Rsp);

	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_OPE,"\r\n");

	if(nRet != GMMP_SUCCESS)
	{
		GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR,"Code[%d], %s\r\n", nRet, GetStringtoErrorCode(nRet) );
	}

	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "[GW->OMP]GMMP_SetControl End \r\n");

	return nRet;
}

//�젣�뼱 �룞�옉 �셿猷� 寃곌낵 蹂닿퀬 Request
int GMMP_SetNotifi(const char* pszAuthID,
		const char* pszAuthKey,
		const char* pszDomainCode,
		const char* pszGWID,
		const char* pszDeviceID,
		const char cControlType,
		const char cResultCode,
		const char* pszMessageBody,
		const int nMessageSize)
{
	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_OPS,"[GW->OMP]GMMP_SetNotifi Start \r\n");

	if(pszAuthID == NULL
			|| pszAuthKey == NULL
			|| pszDomainCode == NULL
			|| pszGWID ==NULL
			|| strlen(pszAuthID) > LEN_AUTH_ID
			|| strlen(pszAuthKey) > LEN_AUTH_KEY
			|| strlen(pszDomainCode) > LEN_DOMAIN_CODE
			|| strlen(pszGWID) > LEN_GW_ID)
	{
		GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR,"Code[%d], %s\r\n", LIB_PARAM_ERROR, GetStringtoErrorCode(LIB_PARAM_ERROR) );
		return LIB_PARAM_ERROR;
	}

	int nRet = GMMP_SUCCESS;

	Notifi_Req stNotifi_Req;

	memset(&stNotifi_Req, 0 ,sizeof(stNotifi_Req) );

	int PacketSize =sizeof(stNotifi_Req) - MAX_MSG_BODY + nMessageSize ; //Message Body�쓽 �냽�꽦�� Optinal�씠�떎

	nRet = SetHeader((void*)&stNotifi_Req, PacketSize, 1,  1, OPERATION_NOTIFICATION_REQ, pszAuthID, pszAuthKey, GMMP_ENCRYPTION_NOT);
	if(nRet != GMMP_SUCCESS)
	{
		GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR,"Code[%d], %s\r\n", nRet, GetStringtoErrorCode(nRet) );
		return nRet;
	}

	memcpy(stNotifi_Req.body.usDomainCode, pszDomainCode, strlen(pszDomainCode) );
	memcpy(stNotifi_Req.body.usGWID, pszGWID, strlen(pszGWID) );

	if(pszDeviceID != NULL) //Device �뙋�떒
	{
		if( strlen(pszDeviceID) > LEN_DEVICE_ID) //Device ID 湲몄씠 �솗�씤
		{
			GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR,"Code[%d], %s\r\n", LIB_PARAM_ERROR, GetStringtoErrorCode(LIB_PARAM_ERROR) );
		}
		else
		{
			memcpy(stNotifi_Req.body.usDeviceID, pszDeviceID, strlen(pszDeviceID) );
		}
	}

	stNotifi_Req.body.ucControlType = cControlType;
	stNotifi_Req.body.ucResultCode = cResultCode;

	if(pszMessageBody != NULL) //Message �궗�슜 �뙋�떒
	{
		if(nMessageSize > MAX_MSG_BODY)
		{
			GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR,"Code[%d], %s\r\n", LIB_PARAM_ERROR, GetStringtoErrorCode(LIB_PARAM_ERROR) );
		}
		else
		{
			memcpy(stNotifi_Req.body.usMessageBody, pszMessageBody, nMessageSize);
		}
	}

	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_REQ, "\r\n");

	GMMP_Trace(&stNotifi_Req.header, &stNotifi_Req.body);

	nRet =  GMMP_Notifi_Req(&stNotifi_Req, PacketSize);

	if(nRet != GMMP_SUCCESS)
	{
		GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR,"Code[%d], %s\r\n", nRet, GetStringtoErrorCode(nRet) );
	}

	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "[GW->OMP]GMMP_SetNotifi End \r\n");

	return nRet;
}

int GMMP_SetHB(const char* pszAuthID,
		const char* pszAuthKey,
		const char* pszDomainCode,
		const char* pszGWID)
{
	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_OPS,"[GW->OMP]GMMP_SetHB Start \r\n");

	if(pszAuthID == NULL
			|| pszAuthKey == NULL
			|| pszDomainCode == NULL
			|| pszGWID ==NULL
			|| strlen(pszAuthID) > LEN_AUTH_ID
			|| strlen(pszAuthKey) > LEN_AUTH_KEY
			|| strlen(pszDomainCode) > LEN_DOMAIN_CODE
			|| strlen(pszGWID) > LEN_GW_ID)
	{
		GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR, "Code[%d], %s\r\n", LIB_PARAM_ERROR, GetStringtoErrorCode(LIB_PARAM_ERROR) );
		return LIB_PARAM_ERROR;
	}

	int nRet = GMMP_SUCCESS;

	HB_Req stHB_Req;

	memset(&stHB_Req, 0 ,sizeof(stHB_Req) );

	nRet = SetHeader((void*)&stHB_Req, sizeof(stHB_Req), 1,  1, OPERATION_HEARTBEAT_REQ, pszAuthID, pszAuthKey, GMMP_ENCRYPTION_NOT);
	if(nRet != GMMP_SUCCESS)
	{
		GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR,"Code[%d], %s\r\n", nRet, GetStringtoErrorCode(nRet) );
		return nRet;
	}

	memcpy(stHB_Req.body.usDomainCode, pszDomainCode, strlen(pszDomainCode) );
	memcpy(stHB_Req.body.usGWID, pszGWID, strlen(pszGWID) );

	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_REQ, "\r\n");

	GMMP_Trace(&stHB_Req.header, &stHB_Req.body);

	nRet =  GMMP_Heartbeat_Req(&stHB_Req);

	if(nRet != GMMP_SUCCESS)
	{
		GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR,"Code[%d], %s\r\n", nRet, GetStringtoErrorCode(nRet) );
	}

	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "[GW->OMP]GMMP_SetHB End \r\n");

	return nRet;
}

//Long Sentence 湲곕뒫 Request
int GMMP_SetLSentence(const char* pszAuthID,
		const char* pszAuthKey,
		const char* pszDomainCode,
		const char* pszGWID,
		const char* pszDeviceID,
		const char cControlType)
{
	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_OPS,"[GW->OMP]GMMP_SetLSentence Start \r\n");

	if(pszAuthID == NULL
			|| pszAuthKey == NULL
			|| pszDomainCode == NULL
			|| pszGWID ==NULL
			|| strlen(pszAuthID) > LEN_AUTH_ID
			|| strlen(pszAuthKey) > LEN_AUTH_KEY
			|| strlen(pszDomainCode) > LEN_DOMAIN_CODE
			|| strlen(pszGWID) > LEN_GW_ID)
	{
		GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR,"Code[%d], %s\r\n", LIB_PARAM_ERROR, GetStringtoErrorCode(LIB_PARAM_ERROR) );
		return LIB_PARAM_ERROR;
	}

	int nRet = GMMP_SUCCESS;

	LSentence_Req stLSentence_Req;

	memset(&stLSentence_Req, 0 ,sizeof(stLSentence_Req) );

	nRet = SetHeader((void*)&stLSentence_Req, sizeof(stLSentence_Req), 1,  1, OPERATION_LSENTENCE_REQ, pszAuthID, pszAuthKey, GMMP_ENCRYPTION_NOT);
	if(nRet != GMMP_SUCCESS)
	{
		GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR,"Code[%d], %s\r\n", nRet, GetStringtoErrorCode(nRet) );
		return nRet;
	}

	memcpy(stLSentence_Req.body.usDomainCode, pszDomainCode, strlen(pszDomainCode) );
	memcpy(stLSentence_Req.body.usGWID, pszGWID, strlen(pszGWID) );

	if(pszDeviceID != NULL) //Device �뙋�떒
	{
		if( strlen(pszDeviceID) > LEN_DEVICE_ID) //Device ID 湲몄씠 �솗�씤
		{
			GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR,"Code[%d], %s\r\n", LIB_PARAM_ERROR, GetStringtoErrorCode(LIB_PARAM_ERROR) );
		}
		else
		{
			memcpy(stLSentence_Req.body.usDeviceID, pszDeviceID, strlen(pszDeviceID) );
		}
	}

	stLSentence_Req.body.ucControlType = cControlType;

	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_REQ, "\r\n");

	GMMP_Trace(&stLSentence_Req.header, &stLSentence_Req.body);

	nRet =  GMMP_LSentence_Req(&stLSentence_Req);
	if(nRet != GMMP_SUCCESS)
	{
		GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR,"Code[%d], %s\r\n", nRet, GetStringtoErrorCode(nRet) );
	}

	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "[GW->OMP]GMMP_SetLSentence End \r\n");

	return nRet;
}

//FTP Info Request
int GMMP_SetFTP(const char* pszAuthID,
		const char* pszAuthKey,
		const char* pszDomainCode,
		const char* pszGWID,
		const char* pszDeviceID,
		const char cControlType)
{
	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_OPS,"[GW->OMP]GMMP_SetFTP Start \r\n");

	if(pszAuthID == NULL
			|| pszAuthKey == NULL
			|| pszDomainCode == NULL
			|| pszGWID ==NULL
			|| strlen(pszAuthID) > LEN_AUTH_ID
			|| strlen(pszAuthKey) > LEN_AUTH_KEY
			|| strlen(pszDomainCode) > LEN_DOMAIN_CODE
			|| strlen(pszGWID) > LEN_GW_ID)
	{
		GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR,"Code[%d], %s\r\n", LIB_PARAM_ERROR, GetStringtoErrorCode(LIB_PARAM_ERROR) );
		return LIB_PARAM_ERROR;
	}

	int nRet = GMMP_SUCCESS;

	FTP_Req stFTP_Req;

	memset(&stFTP_Req, 0 ,sizeof(stFTP_Req) );

	nRet = SetHeader((void*)&stFTP_Req, sizeof(stFTP_Req), 1,  1, OPERATION_FTP_INFO_REQ, pszAuthID, pszAuthKey, GMMP_ENCRYPTION_NOT);
	if(nRet != GMMP_SUCCESS)
	{
		GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR,"Code[%d], %s\r\n", nRet, GetStringtoErrorCode(nRet) );
		return nRet;
	}

	memcpy(stFTP_Req.body.usDomainCode, pszDomainCode, strlen(pszDomainCode) );
	memcpy(stFTP_Req.body.usGWID, pszGWID, strlen(pszGWID) );

	if(pszDeviceID != NULL) //Device �뙋�떒
	{
		if( strlen(pszDeviceID) > LEN_DEVICE_ID) //Device ID 湲몄씠 �솗�씤
		{
			GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR,"Code[%d], %s\r\n", LIB_PARAM_ERROR, GetStringtoErrorCode(LIB_PARAM_ERROR) );
		}
		else
		{
			memcpy(stFTP_Req.body.usDeviceID, pszDeviceID, strlen(pszDeviceID) );
		}
	}

	stFTP_Req.body.ucControlType = cControlType;

	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_REQ, "\r\n");

	GMMP_Trace(&stFTP_Req.header, &stFTP_Req.body);

	nRet =  GMMP_FTP_Info_Req(&stFTP_Req);
	if(nRet != GMMP_SUCCESS)
	{
		GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR,"Code[%d], %s\r\n", nRet, GetStringtoErrorCode(nRet) );
	}

	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "[GW->OMP]GMMP_SetFTP End \r\n");

	return nRet;
}

//Remote Access Info Request
int GMMP_SetRemote(const char* pszAuthID,
		const char* pszAuthKey,
		const char* pszDomainCode,
		const char* pszGWID,
		const char* pszDeviceID,
		const char cControlType)
{
	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_OPS,"[GW->OMP]GMMP_SetRemote Start \r\n");

	if(pszAuthID == NULL
			|| pszAuthKey == NULL
			|| pszDomainCode == NULL
			|| pszGWID ==NULL
			|| strlen(pszAuthID) > LEN_AUTH_ID
			|| strlen(pszAuthKey) > LEN_AUTH_KEY
			|| strlen(pszDomainCode) > LEN_DOMAIN_CODE
			|| strlen(pszGWID) > LEN_GW_ID)
	{
		GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR, "Code[%d], %s\r\n", LIB_PARAM_ERROR, GetStringtoErrorCode(LIB_PARAM_ERROR) );
		return LIB_PARAM_ERROR;
	}

	int nRet = GMMP_SUCCESS;

	Remote_Req stRemote_Req;

	memset(&stRemote_Req, 0 ,sizeof(stRemote_Req) );

	nRet = SetHeader((void*)&stRemote_Req, sizeof(stRemote_Req), 1,  1, OPERATION_REMOTE_INFO_REQ, pszAuthID, pszAuthKey, GMMP_ENCRYPTION_NOT);
	if(nRet != GMMP_SUCCESS)
	{
		GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR, "Code[%d], %s\r\n", nRet, GetStringtoErrorCode(nRet) );
		return nRet;
	}

	memcpy(stRemote_Req.body.usDomainCode, pszDomainCode, strlen(pszDomainCode) );
	memcpy(stRemote_Req.body.usGWID, pszGWID, strlen(pszGWID) );

	if(pszDeviceID != NULL) //Device �뙋�떒
	{
		if( strlen(pszDeviceID) > LEN_DEVICE_ID) //Device ID 湲몄씠 �솗�씤
		{
			GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR,"Code[%d], %s\r\n", LIB_PARAM_ERROR, GetStringtoErrorCode(LIB_PARAM_ERROR) );
		}
		else
		{
			memcpy(stRemote_Req.body.usDeviceID, pszDeviceID, strlen(pszDeviceID) );
		}
	}

	stRemote_Req.body.ucControlType = cControlType;

	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_REQ, "\r\n");

	GMMP_Trace(&stRemote_Req.header, &stRemote_Req.body);

	nRet =  GMMP_Remote_Req(&stRemote_Req);
	if(nRet != GMMP_SUCCESS)
	{
		GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR, "Code[%d], %s\r\n", nRet, GetStringtoErrorCode(nRet) );
	}

	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "[GW->OMP]GMMP_SetRemote End \r\n");

	return nRet;
}

//Multimedia Server URL Info Request
int GMMP_SetMM(const char* pszAuthID,
		const char* pszAuthKey,
		const char* pszDomainCode,
		const char* pszGWID,
		const char* pszDeviceID,
		const char cControlType)
{
	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_OPS,"[GW->OMP]GMMP_SetMM Start \r\n");

	if(pszAuthID == NULL
			|| pszAuthKey == NULL
			|| pszDomainCode == NULL
			|| pszGWID ==NULL
			|| strlen(pszAuthID) > LEN_AUTH_ID
			|| strlen(pszAuthKey) > LEN_AUTH_KEY
			|| strlen(pszDomainCode) > LEN_DOMAIN_CODE
			|| strlen(pszGWID) > LEN_GW_ID)
	{
		GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR, "Code[%d], %s\r\n", LIB_PARAM_ERROR, GetStringtoErrorCode(LIB_PARAM_ERROR) );
		return LIB_PARAM_ERROR;
	}

	int nRet = GMMP_SUCCESS;

	MMURLInfo_Req stMMURLInfo_Req;

	memset(&stMMURLInfo_Req, 0 ,sizeof(stMMURLInfo_Req) );

	nRet = SetHeader((void*)&stMMURLInfo_Req, sizeof(stMMURLInfo_Req), 1,  1, OPERATION_MULTIMEDIA_URL_INFO_REQ, pszAuthID, pszAuthKey, GMMP_ENCRYPTION_NOT);
	if(nRet != GMMP_SUCCESS)
	{
		GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR, "Code[%d], %s\r\n", nRet, GetStringtoErrorCode(nRet) );
		return nRet;
	}

	memcpy(stMMURLInfo_Req.body.usDomainCode, pszDomainCode, strlen(pszDomainCode) );
	memcpy(stMMURLInfo_Req.body.usGWID, pszGWID, strlen(pszGWID) );

	if(pszDeviceID != NULL) //Device �뙋�떒
	{
		if( strlen(pszDeviceID) > LEN_DEVICE_ID) //Device ID 湲몄씠 �솗�씤
		{
			GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR,"Code[%d], %s\r\n", LIB_PARAM_ERROR, GetStringtoErrorCode(LIB_PARAM_ERROR) );
		}
		else
		{
			memcpy(stMMURLInfo_Req.body.usDeviceID, pszDeviceID, strlen(pszDeviceID) );
		}
	}

	stMMURLInfo_Req.body.ucControlType = cControlType;

	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_REQ, "\r\n");

	GMMP_Trace(&stMMURLInfo_Req.header, &stMMURLInfo_Req.body);

	nRet =  GMMP_Multimedia_Req(&stMMURLInfo_Req);
	if(nRet != GMMP_SUCCESS)
	{
		GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR, "Code[%d], %s\r\n", nRet, GetStringtoErrorCode(nRet) );
	}

	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "[GW->OMP]GMMP_SetMM End \r\n");

	return nRet;
}

//LOB Cloud Info Request
int GMMP_SetLOB_Cloud(const char* pszAuthID,
		const char* pszAuthKey,
		const char* pszDomainCode,
		const char* pszGWID,
		const char* pszDeviceID)
{
	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_OPS,"[GW->OMP]GMMP_SetLOB_Cloud Start \r\n");

	if(pszAuthID == NULL
			|| pszAuthKey == NULL
			|| pszDomainCode == NULL
			|| pszGWID ==NULL
			|| strlen(pszAuthID) > LEN_AUTH_ID
			|| strlen(pszAuthKey) > LEN_AUTH_KEY
			|| strlen(pszDomainCode) > LEN_DOMAIN_CODE
			|| strlen(pszGWID) > LEN_GW_ID)
	{
		GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR, "Code[%d], %s\r\n", LIB_PARAM_ERROR, GetStringtoErrorCode(LIB_PARAM_ERROR) );
		return LIB_PARAM_ERROR;
	}

	int nRet = GMMP_SUCCESS;

	LOBCloud_Req stLOBCloud_Req;

	memset(&stLOBCloud_Req, 0 ,sizeof(stLOBCloud_Req) );

	nRet = SetHeader((void*)&stLOBCloud_Req, sizeof(stLOBCloud_Req), 1,  1, OPERATION_LOB_CLOUD_INFO_REQ, pszAuthID, pszAuthKey, GMMP_ENCRYPTION_NOT);
	if(nRet != GMMP_SUCCESS)
	{
		GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR, "Code[%d], %s\r\n", nRet, GetStringtoErrorCode(nRet) );
		return nRet;
	}

	memcpy(stLOBCloud_Req.body.usDomainCode, pszDomainCode, strlen(pszDomainCode) );
	memcpy(stLOBCloud_Req.body.usGWID, pszGWID, strlen(pszGWID) );

	if(pszDeviceID != NULL) //Device �뙋�떒
	{
		if( strlen(pszDeviceID) > LEN_DEVICE_ID) //Device ID 湲몄씠 �솗�씤
		{
			GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR,"Code[%d], %s\r\n", LIB_PARAM_ERROR, GetStringtoErrorCode(LIB_PARAM_ERROR) );
		}
		else
		{
			memcpy(stLOBCloud_Req.body.usDeviceID, pszDeviceID, strlen(pszDeviceID) );
		}
	}

	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_REQ, "\r\n");

	GMMP_Trace(&stLOBCloud_Req.header, &stLOBCloud_Req.body);

	nRet =  GMMP_LOB_Cloud_Info_Req(&stLOBCloud_Req);
	if(nRet != GMMP_SUCCESS)
	{
		GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR, "Code[%d], %s\r\n", nRet, GetStringtoErrorCode(nRet) );
	}

	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "[GW->OMP]GMMP_SetLOB_Cloud End \r\n");

	return nRet;
}

//LOB FTP Info Request
int GMMP_SetLOB_FTP(const char* pszAuthID,
		const char* pszAuthKey,
		const char* pszDomainCode,
		const char* pszGWID,
		const char* pszDeviceID)
{
	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_OPS,"[GW->OMP]GMMP_SetLOB_FTP Start \r\n");

	if(pszAuthID == NULL
			|| pszAuthKey == NULL
			|| pszDomainCode == NULL
			|| pszGWID ==NULL
			|| strlen(pszAuthID) > LEN_AUTH_ID
			|| strlen(pszAuthKey) > LEN_AUTH_KEY
			|| strlen(pszDomainCode) > LEN_DOMAIN_CODE
			|| strlen(pszGWID) > LEN_GW_ID)
	{
		GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR, "Code[%d], %s\r\n", LIB_PARAM_ERROR, GetStringtoErrorCode(LIB_PARAM_ERROR) );
		return LIB_PARAM_ERROR;
	}

	int nRet = GMMP_SUCCESS;

	LOBFTP_Req stLOBFTP_Req;

	memset(&stLOBFTP_Req, 0 ,sizeof(stLOBFTP_Req) );

	nRet = SetHeader((void*)&stLOBFTP_Req, sizeof(stLOBFTP_Req), 1,  1, OPERATION_LOB_FTP_INFO_REQ, pszAuthID, pszAuthKey, GMMP_ENCRYPTION_NOT);
	if(nRet != GMMP_SUCCESS)
	{
		GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR, "Code[%d], %s\r\n", nRet, GetStringtoErrorCode(nRet) );
		return nRet;
	}

	memcpy(stLOBFTP_Req.body.usDomainCode, pszDomainCode, strlen(pszDomainCode) );
	memcpy(stLOBFTP_Req.body.usGWID, pszGWID, strlen(pszGWID) );

	if(pszDeviceID != NULL) //Device �뙋�떒
	{
		if( strlen(pszDeviceID) > LEN_DEVICE_ID) //Device ID 湲몄씠 �솗�씤
		{
			GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR,"Code[%d], %s\r\n", LIB_PARAM_ERROR, GetStringtoErrorCode(LIB_PARAM_ERROR) );
		}
		else
		{
			memcpy(stLOBFTP_Req.body.usDeviceID, pszDeviceID, strlen(pszDeviceID) );
		}
	}

	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_REQ, "\r\n");

	GMMP_Trace(&stLOBFTP_Req.header, &stLOBFTP_Req.body);

	nRet =  GMMP_LOB_FTP_Info_Req(&stLOBFTP_Req);
	if(nRet != GMMP_SUCCESS)
	{
		GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR, "Code[%d], %s\r\n", nRet, GetStringtoErrorCode(nRet) );
	}

	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "[GW->OMP]GMMP_SetLOB_FTP End \r\n");

	return nRet;
}

int GMMP_SetLOB_Notifi(const char* pszAuthID,
		const char* pszAuthKey,
		const char* pszDomainCode,
		const char* pszGWID,
		const char* pszDeviceID,
		const char cStorageType,
		const char* pszLOBPath)
{
	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_OPS,"[GW->OMP]GMMP_SetLOB_Notifi Start \r\n");

	if(pszAuthID == NULL
			|| pszAuthKey == NULL
			|| pszDomainCode == NULL
			|| pszGWID ==NULL
			|| strlen(pszAuthID) > LEN_AUTH_ID
			|| strlen(pszAuthKey) > LEN_AUTH_KEY
			|| strlen(pszDomainCode) > LEN_DOMAIN_CODE
			|| strlen(pszGWID) > LEN_GW_ID)
	{
		GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR, "Code[%d], %s\r\n", LIB_PARAM_ERROR, GetStringtoErrorCode(LIB_PARAM_ERROR) );
		return LIB_PARAM_ERROR;
	}

	int nRet = GMMP_SUCCESS;

	LOBNotifi_Req stLOBNotifi_Req;

	memset(&stLOBNotifi_Req, 0 ,sizeof(stLOBNotifi_Req) );

	nRet = SetHeader((void*)&stLOBNotifi_Req, sizeof(stLOBNotifi_Req), 1,  1, OPERATION_LOB_UPLOAD_NOTIFICATION_REQ, pszAuthID, pszAuthKey, GMMP_ENCRYPTION_NOT);
	if(nRet != GMMP_SUCCESS)
	{
		GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR, "Code[%d], %s\r\n", nRet, GetStringtoErrorCode(nRet) );
		return nRet;
	}

	memcpy(stLOBNotifi_Req.body.usDomainCode, pszDomainCode, strlen(pszDomainCode) );
	memcpy(stLOBNotifi_Req.body.usGWID, pszGWID, strlen(pszGWID) );

	if(pszDeviceID != NULL) //Device �뙋�떒
	{
		if( strlen(pszDeviceID) > LEN_DEVICE_ID) //Device ID 湲몄씠 �솗�씤
		{
			GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR,"Code[%d], %s\r\n", LIB_PARAM_ERROR, GetStringtoErrorCode(LIB_PARAM_ERROR) );
		}
		else
		{
			memcpy(stLOBNotifi_Req.body.usDeviceID, pszDeviceID, strlen(pszDeviceID) );
		}
	}
	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_REQ, "\r\n");

	GMMP_Trace(&stLOBNotifi_Req.header, &stLOBNotifi_Req.body);

	nRet =  GMMP_LOB_Notification_Req(&stLOBNotifi_Req);
	if(nRet != GMMP_SUCCESS)
	{
		GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR, "Code[%d], %s\r\n", nRet, GetStringtoErrorCode(nRet) );
	}

	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "[GW->OMP]GMMP_SetLOB_Notifi End \r\n");

	return nRet;
}

//GW/Device �벑濡�/�빐吏� Response
int GMMP_GetReg(GwRegist_Rsp* pstGwRegist_Rsp, DeviceRegist_Rsp* pstDeviceRegist_Rsp)
{
	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "[OMP->GW]GMMP_GetReg Start \r\n");

	int nRet = GMMP_SUCCESS;

	if(pstGwRegist_Rsp != NULL)
	{
		nRet = GMMP_GW_Reg_Rsp(pstGwRegist_Rsp);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_RSP, "\r\n");
		GMMP_Trace(&pstGwRegist_Rsp->header, &pstGwRegist_Rsp->body);
	}
	else
	{
		nRet = GMMP_Device_Reg_Rsp(pstDeviceRegist_Rsp);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_RSP, "\r\n");
		GMMP_Trace(&pstDeviceRegist_Rsp->header, &pstDeviceRegist_Rsp->body);
	}

	CloseSocket();

	if(nRet != GMMP_SUCCESS)
	{
		GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR,"Code[%d], %s\r\n", nRet, GetStringtoErrorCode(nRet) );
	}

	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "[OMP->GW]GMMP_GetReg End \r\n");

	return nRet;
}

int GMMP_GetDeReg(GwDeRegist_Rsp* pstGwRegist_Rsp, DeviceDeRegist_Rsp* pstDeviceDeRegist_Rsp)
{
	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT,"[OMP->GW]GMMP_GetDeReg Start \r\n");

	int nRet = GMMP_SUCCESS;

	if(pstGwRegist_Rsp != NULL)
	{
		nRet = GMMP_GW_DeReg_Rsp(pstGwRegist_Rsp);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_RSP, "\r\n");
		GMMP_Trace(&pstGwRegist_Rsp->header, &pstGwRegist_Rsp->body);
	}
	else
	{
		nRet = GMMP_Device_DeReg_Rsp(pstDeviceDeRegist_Rsp);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_RSP, "\r\n");
		GMMP_Trace(&pstDeviceDeRegist_Rsp->header, &pstDeviceDeRegist_Rsp->body);
	}

	CloseSocket();

	if(nRet != GMMP_SUCCESS)
	{
		GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR,"Code[%d], %s\r\n", nRet, GetStringtoErrorCode(nRet) );
	}

	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "[OMP->GW]GMMP_GetDeReg End \r\n");

	return nRet;
}

int GMMP_GetEncryptInfo(EncryptInfo_Rsp* pstEncryptInfo_Rsp)
{
	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT,"[OMP->GW]GMMP_GetEncryptInfo Start \r\n");

	int nRet = GMMP_Encrypt_Info_Rsp(pstEncryptInfo_Rsp);

	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_RSP, "\r\n");

	GMMP_Trace(&pstEncryptInfo_Rsp->header, &pstEncryptInfo_Rsp->body);

	//CloseSocket();

	if(nRet != GMMP_SUCCESS)
	{
		GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR,"Code[%d], %s\r\n", nRet, GetStringtoErrorCode(nRet) );
	}

	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "[OMP->GW]GMMP_GetEncryptInfo End \r\n");

	return nRet;
}

int GMMP_GetEncryptKey(EncryptKey_Rsp* pstEncryptKey_Rsp)
{
	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT,"[OMP->GW]GMMP_GetEncryptKey Start \r\n");

	int nRet = GMMP_Encrypt_Key_Rsp(pstEncryptKey_Rsp);

	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_RSP, "\r\n");

	GMMP_Trace(&pstEncryptKey_Rsp->header, &pstEncryptKey_Rsp->body);

	CloseSocket();

	if(nRet != GMMP_SUCCESS)
	{
		GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR,"Code[%d], %s\r\n", nRet, GetStringtoErrorCode(nRet) );
	}

	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "[OMP->GW]GMMP_GetEncryptKey End \r\n");

	return nRet;
}

//Profile Info Response
int GMMP_GetProfile(Profile_Rsp* pstProfile_Rsp)
{
	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT,"[OMP->GW]GMMP_GetProfile Start \r\n");

	int nRet = GMMP_Profile_Rsp(pstProfile_Rsp);

	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_RSP, "\r\n");

	GMMP_Trace(&pstProfile_Rsp->header, &pstProfile_Rsp->body);

	CloseSocket();

	if(nRet != GMMP_SUCCESS)
	{
		GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR,"Code[%d], %s\r\n", nRet, GetStringtoErrorCode(nRet) );
	}

	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "[OMP->GW]GMMP_GetProfile End \r\n");

	return nRet;
}

//�닔吏� �뜲�씠�꽣 蹂닿퀬  Response
int GMMP_GetDelivery(Delivery_Rsp* pstDelivery_Rsp)
{
	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "[OMP->GW]GMMP_GetDelivery Start \r\n");

	int nRet = GMMP_Delivery_Rsp(pstDelivery_Rsp);

	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_RSP, "\r\n");

	GMMP_Trace(&pstDelivery_Rsp->header, &pstDelivery_Rsp->body);

	CloseSocket();

	if(nRet != GMMP_SUCCESS)
	{
		GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR,"Code[%d], %s\r\n", nRet, GetStringtoErrorCode(nRet) );
	}

	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "[OMP->GW]GMMP_GetDelivery End \r\n");

	return nRet;
}
//�젣�뼱 �룞�옉 �셿猷� 寃곌낵 蹂닿퀬 Response
int GMMP_GetNotifi(Notifi_Rsp* pstNotifi_Rsp)
{
	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "[OMP->GW]GMMP_GetNotifi Start \r\n");

	int nRet = GMMP_Notifi_Rsp(pstNotifi_Rsp);

	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_RSP, "\r\n");

	GMMP_Trace(&pstNotifi_Rsp->header, &pstNotifi_Rsp->body);

	CloseSocket();

	if(nRet != GMMP_SUCCESS)
	{
		GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR,"Code[%d], %s\r\n", nRet, GetStringtoErrorCode(nRet) );
	}

	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "[OMP->GW]GMMP_GetNotifi End \r\n");

	return nRet;
}

int GMMP_GetHB(HB_Rsp* pstHB_Rsp) //TCP Always ON 紐⑤뱶�씪 寃쎌슦 ( Thread �깮�꽦) �궗�슜 TCP Disconnect�븯吏� �븡�쓬
{
	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "[OMP->GW]GMMP_GetHB Start \r\n");

	int nRet = GMMP_Heartbeat_Rsp(pstHB_Rsp);

	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_RSP, "\r\n");

	GMMP_Trace(&pstHB_Rsp->header, &pstHB_Rsp->body);

	if(nRet != GMMP_SUCCESS)
	{
		GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR,"Code[%d], %s\r\n", nRet, GetStringtoErrorCode(nRet) );
	}

	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "[OMP->GW]GMMP_GetHB End \r\n");

	return nRet;
}

//Long Sentence Response
int GMMP_GetLSentence(LSentence_Rsp* pstLSentence_Rsp)
{
	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "[OMP->GW]GMMP_GetLSentence Start \r\n");

	int nRet = GMMP_LSentence_Rsp(pstLSentence_Rsp);

	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_RSP, "\r\n");

	GMMP_Trace(&pstLSentence_Rsp->header, &pstLSentence_Rsp->body);

	//CloseSocket();

	if(nRet != GMMP_SUCCESS)
	{
		GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR,"Code[%d], %s\r\n", nRet, GetStringtoErrorCode(nRet) );
	}

	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "[OMP->GW]GMMP_GetLSentence End \r\n");

	return nRet;
}

//FTP Info Response
int GMMP_GetFTP(FTP_Rsp* pstFTP_Rsp)
{
	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "[OMP->GW]GMMP_GetFTP Start \r\n");

	int nRet = GMMP_FTP_Info_Rsp(pstFTP_Rsp);

	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_RSP, "\r\n");

	GMMP_Trace(&pstFTP_Rsp->header, &pstFTP_Rsp->body);

	//CloseSocket();

	if(nRet != GMMP_SUCCESS)
	{
		GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR,"Code[%d], %s\r\n", nRet, GetStringtoErrorCode(nRet) );
	}

	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "[OMP->GW]GMMP_GetFTP End \r\n");

	return nRet;
}

//Remote Access Info Response
int GMMP_GetRemote(Remote_Rsp* pstRemote_Rsp)
{
	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "[OMP->GW]GMMP_GetEncryptKey Start \r\n");

	int nRet = GMMP_Remote_Rsp(pstRemote_Rsp);

	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_RSP, "\r\n");

	GMMP_Trace(&pstRemote_Rsp->header, &pstRemote_Rsp->body);

	//CloseSocket();

	if(nRet != GMMP_SUCCESS)
	{
		GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR, "Code[%d], %s\r\n", nRet, GetStringtoErrorCode(nRet) );
	}

	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "[OMP->GW]GMMP_GetEncryptKey End \r\n");

	return nRet;
}

//Multimedia Server URL Info Response
int GMMP_GetMM(MMURLInfo_Rsp* pstMMURLInfo_Rsp)
{
	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "[OMP->GW]GMMP_GetMM Start \r\n");

	int nRet = GMMP_Multimedia_Rsp(pstMMURLInfo_Rsp);

	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_RSP, "\r\n");

	GMMP_Trace(&pstMMURLInfo_Rsp->header, &pstMMURLInfo_Rsp->body);

	//CloseSocket();

	if(nRet != GMMP_SUCCESS)
	{
		GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR, "Code[%d], %s\r\n", nRet, GetStringtoErrorCode(nRet) );
	}

	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "[OMP->GW]GMMP_GetMM End \r\n");

	return nRet;
}

//LOB Cloud Info Response
int GMMP_GetLOB_Cloud(LOBCloud_Rsp* pstLOBCloud_Rsp)
{
	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "[GW->OMP]GMMP_GetLOB_Cloud Start \r\n");

	int nRet = GMMP_LOB_Cloud_Info_Rsp(pstLOBCloud_Rsp);

	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_RSP, "\r\n");

	GMMP_Trace(&pstLOBCloud_Rsp->header, &pstLOBCloud_Rsp->body);

	//CloseSocket();

	if(nRet != GMMP_SUCCESS)
	{
		GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR, "Code[%d], %s\r\n", nRet, GetStringtoErrorCode(nRet) );
	}

	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "[GW->OMP]GMMP_GetLOB_Cloud End \r\n");

	return nRet;
}

//LOB FTP Info Response
int GMMP_GetLOB_FTP(LOBFTP_Rsp* pstLOBFTP_Rsp)
{
	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "[OMP->GW]GMMP_GetLOB_FTP Start \r\n");

	int nRet = GMMP_LOB_FTP_Info_Rsp(pstLOBFTP_Rsp);

	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_RSP, "\r\n");

	GMMP_Trace(&pstLOBFTP_Rsp->header, &pstLOBFTP_Rsp->body);

	//CloseSocket();

	if(nRet != GMMP_SUCCESS)
	{
		GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR, "Code[%d], %s\r\n", nRet, GetStringtoErrorCode(nRet) );
	}

	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "[OMP->GW]GMMP_GetLOB_FTP End \r\n");

	return nRet;
}

int GMMP_GetLOB_Notifi(LOBNotifi_Rsp* pstLOBNotifi_Rsp)
{
	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "[OMP->GW]GMMP_GetLOB_Notifi Start \r\n");

	int nRet = GMMP_LOB_Notification_Rsp(pstLOBNotifi_Rsp);

	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_RSP, "\r\n");

	GMMP_Trace(&pstLOBNotifi_Rsp->header, &pstLOBNotifi_Rsp->body);

	CloseSocket();

	if(nRet != GMMP_SUCCESS)
	{
		GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR, "Code[%d], %s\r\n", nRet, GetStringtoErrorCode(nRet) );
	}

	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "[OMP->GW]GMMP_GetLOB_Notifi End \n\r\n");

	return nRet;
}

int GMMP_Read(GMMPHeader* pstGMMPHeader, void** pBody)
{
	ConvertShort cvtShort;
	//ConvertInt cvtint;

	int nRet = 0;
	int nHeaderSize = sizeof(GMMPHeader);

	memset(pstGMMPHeader, 0 ,  nHeaderSize);

	cvtShort.sU8 	= 0;
	//cvtint.sU8		= 0;

	nRet = 0;

	nRet = ReadTCP((char*)pstGMMPHeader, nHeaderSize);
	if(nRet != GMMP_SUCCESS)
	{
		GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR, "Read Header Socket Error Code[%d], %s\r\n", nRet, GetStringtoErrorCode(nRet));

		return nRet;
	}

	memcpy(cvtShort.usShort, pstGMMPHeader->usMessageLength, sizeof(pstGMMPHeader->usMessageLength) );
	cvtShort.sU8 = btols(cvtShort.sU8);

	if(cvtShort.sU8 < nHeaderSize)
	{
		ClearBuffer();

		return GMMP_HEADER_SIZE_ERROR;
	}

	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "Recv Header Type = %x, Read Body Size = %d\r\n", pstGMMPHeader->ucMessageType, cvtShort.sU8 - nHeaderSize  );

	int nBodySize = 0;

	if( (*pBody = MallocBody(pstGMMPHeader->ucMessageType, &nBodySize) )== NULL || nBodySize <= 0)
	{
		GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR, "malloc Error Code[%d], %s\r\n", GMMP_MALLOC_ERROR, GetStringtoErrorCode(GMMP_MALLOC_ERROR));

		return GMMP_MALLOC_ERROR;
	}

	memset(*pBody, 0, nBodySize);

	int ReadPacketSize = cvtShort.sU8 -  nHeaderSize;

	nRet = ReadTCP((char*)*pBody, ReadPacketSize);
	if(nRet != GMMP_SUCCESS)
	{
		GMMP_Printf(GMMP_ERROR_LEVEL_ERROR, GMMP_LOG_MARKET_ERR, "Read Body Socket Code[%d], %s\r\n", GMMP_MALLOC_ERROR, GetStringtoErrorCode(GMMP_MALLOC_ERROR));

		CloseSocket();

		return nRet;
	}

	return GMMP_SUCCESS;
}

int GMMP_Recv(GMMPHeader* pstGMMPHeader, void* pBody)
{
	GMMP_Trace(pstGMMPHeader, pBody);

	char cMessageType =  pstGMMPHeader->ucMessageType ;

	if(cMessageType == OPERATION_GW_REG_RSP)
	{
		stGwRegistrationRspHdr* pstRspHdr =(stGwRegistrationRspHdr*) pBody;
		OG_Reg_Recv(pstGMMPHeader, pstRspHdr, NULL);
	}
	else if(cMessageType == OPERATION_GW_DEREG_RSP)
	{
		stGwDeRegistrationRspHdr* pstRspHdr =(stGwDeRegistrationRspHdr*) pBody;
		OG_DeReg_Recv(pstGMMPHeader, pstRspHdr, NULL);
	}
	else if(cMessageType  == OPERATION_PROFILE_RSP)
	{
		stProfileRspHdr* pstRspHdr =(stProfileRspHdr*) pBody;
		OG_Profile_Recv(pstGMMPHeader, pstRspHdr);
	}
	else if(cMessageType == OPERATION_DEVICE_REG_RSP)
	{
		stDeviceRegistrationRspHdr* pstRspHdr =(stDeviceRegistrationRspHdr*) pBody;
		OG_Reg_Recv(pstGMMPHeader, NULL, pstRspHdr);
	}
	else if(cMessageType == OPERATION_DEVICE_DEREG_RSP)
	{
		stDeviceDeRegistrationRspHdr* pstRspHdr =(stDeviceDeRegistrationRspHdr*) pBody;
		OG_DeReg_Recv(pstGMMPHeader, NULL,  pstRspHdr);
	}
	else if(cMessageType  == OPERATION_DELIVERY_RSP)
	{
		stPacketDeliveryRspHdr* pstRspHdr =(stPacketDeliveryRspHdr*) pBody;
		OG_Delivery_Recv(pstGMMPHeader,  pstRspHdr);
	}
	else if(cMessageType  == OPERATION_CONTROL_REQ)
	{
		stControlReqHdr* pstReqHdr =(stControlReqHdr*) pBody;
		GMMP_Ctrl_Recv(pstGMMPHeader,  pstReqHdr);
	}
	else if(cMessageType  == OPERATION_HEARTBEAT_RSP)
	{
		stHeartBeatMsgRspHdr* pstRspHdr =(stHeartBeatMsgRspHdr*) pBody;
		OG_HB_Recv(pstGMMPHeader,  pstRspHdr);
	}
	else if(cMessageType  == OPERATION_LSENTENCE_RSP)
	{
		stLSentenceRspHdr* pstRspHdr =(stLSentenceRspHdr*) pBody;
		OG_LSentence_Recv(pstGMMPHeader,  pstRspHdr);
	}
	else if(cMessageType  == OPERATION_FTP_INFO_RSP)
	{
		stFtpInfoRspHdr* pstRspHdr =(stFtpInfoRspHdr*) pBody;
		OG_FTP_Recv(pstGMMPHeader,  pstRspHdr);
	}
	else if(cMessageType  == OPERATION_REMOTE_INFO_RSP)
	{
		stRemoteAccessInfoRspHdr* pstRspHdr =(stRemoteAccessInfoRspHdr*) pBody;
		OG_Remote_Recv(pstGMMPHeader,  pstRspHdr);
	}
	else if(cMessageType  == OPERATION_NOTIFICATION_RSP)
	{
		stNotificationRspHdr* pstRspHdr =(stNotificationRspHdr*) pBody;
		OG_Notifi_Recv(pstGMMPHeader,  pstRspHdr);
	}
	else if(cMessageType  == OPERATION_ENCRYPTION_INFO_RSP)
	{
		stEncryptionInfoRspHdr* pstRspHdr =(stEncryptionInfoRspHdr*) pBody;
		OG_Enc_Recv(pstGMMPHeader,  pstRspHdr, NULL);
	}
	else if(cMessageType  == OPERATION_ENCRYPTION_KEY_RSP)
	{
		stEncryptionKeyRspHdr* pstRspHdr =(stEncryptionKeyRspHdr*) pBody;
		OG_Enc_Recv(pstGMMPHeader,  NULL, pstRspHdr);
	}
	else if(cMessageType  == OPERATION_MULTIMEDIA_URL_INFO_RSP)
	{
		stMultimediaURLInfoRspHdr* pstRspHdr =(stMultimediaURLInfoRspHdr*) pBody;
		OG_MM_Recv(pstGMMPHeader,  pstRspHdr);
	}
	else if(cMessageType  == OPERATION_LOB_CLOUD_INFO_RSP)
	{
		stLOBCloudAccessInfoRspHdr* pstRspHdr =(stLOBCloudAccessInfoRspHdr*) pBody;
		OG_LOB_Recv(pstGMMPHeader,  pstRspHdr, NULL);
	}
	else if(cMessageType  == OPERATION_LOB_FTP_INFO_RSP)
	{
		stLOBFTPAccessInfoRspHdr* pstRspHdr =(stLOBFTPAccessInfoRspHdr*) pBody;
		OG_LOB_Recv(pstGMMPHeader,  NULL, pstRspHdr);
	}
	else if(cMessageType  == OPERATION_LOB_UPLOAD_NOTIFICATION_RSP)
	{
		stLOBUploadNotificationRspHdr* pstRspHdr =(stLOBUploadNotificationRspHdr*) pBody;
		OG_LOB_Notifi_Recv(pstGMMPHeader,  pstRspHdr);
	}

	return GMMP_SUCCESS;
}

int GMMP_Ctrl_Recv(GMMPHeader* pstGMMPHeader,  stControlReqHdr* pstReqHdr)
{
	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "GMMP_Ctrl_Recv Rsp\r\n");

	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "Auth Key =%.*s\r\n", sizeof(pstGMMPHeader->usAuthKey), pstGMMPHeader->usAuthKey);
	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "Domain Code =%.*s\r\n", sizeof(pstReqHdr->usDomainCode), pstReqHdr->usDomainCode);
	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "GW ID=%.*s\r\n", sizeof(pstReqHdr->usGWID), pstReqHdr->usGWID);
	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "Device ID =%.*s\r\n", sizeof(pstReqHdr->usDeviceID), pstReqHdr->usDeviceID);
	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "Control Code[0x%x]\r\n", pstReqHdr->ucControlType);

	U8 cControlType = pstReqHdr->ucControlType;

	//�떒�닚 �젣�뼱 紐낅졊�뼱�씤 寃쎌슦
	switch(cControlType)
	{
		case CONTROL_Reset:
				GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "CONTROL_Reset\r\n");
				break;
		case CONTROL_Turn_Off:
				GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "CONTROL_Turn_Off\r\n");
				break;
		case CONTROL_Report_On:
				GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "CONTROL_Report_On\r\n");
				break;
		case CONTROL_Report_Off:
				GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "CONTROL_Report_Off\r\n");
				break;
		case CONTROL_Time_Sync:
				GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "CONTROL_Time_Sync\r\n");
				break;
		case CONTROL_Pause:
				GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "CONTROL_Pause\r\n");
				break;
		case CONTROL_Restart:
				GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "CONTROL_Restart\r\n");
				break;
		case CONTROL_Signal_Power_Check:
				GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "CONTROL_Signal_Power_Check\r\n");
				break;
		case CONTROL_Diagnostic:
				GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "CONTROL_Diagnostic\r\n");
				break;
		case CONTROL_Reserved:
				GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "CONTROL_Reserved\r\n");
				break;
		case CONTROL_Profile_Reset:
				GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "CONTROL_Profile_Reset\r\n");
				break;
		case CONTROL_Status_Check:
				GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "CONTROL_Status_Check\r\n");
				break;
		case CONTROL_FW_Download: //FTP �뿰寃�
				GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "CONTROL_FW_Download\r\n");
				break;
		case CONTROL_FW_Update:
				GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "CONTROL_FW_Update\r\n");
				break;
		case CONTROL_App_Download: //FTP �뿰寃�
				GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "CONTROL_App_Download\r\n");
				break;
		case CONTROL_App_Update:
				GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "CONTROL_App_Update\r\n");
				break;
		case CONTROL_Remote_Access: //Remote Info �뿰寃�
				GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "CONTROL_Remote_Access\r\n");
				break;
		case CONTROL_Multimedia_Control_Start: //Multimedia URL Info �뿰寃�
				GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "CONTROL_Multimedia_Control_Start\r\n");
				break;
		case CONTROL_Multimedia_Control_Pause:
				GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "CONTROL_Multimedia_Control_Pause\r\n");
				break;
		case CONTROL_Multimedia_Control_Stop:
				GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "CONTROL_Multimedia_Control_Stop\r\n");
				break;
		case CONTROL_Multimedia_Control_Restart:
				GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "CONTROL_Multimedia_Control_Restart\r\n");
				break;
		case CONTROL_Frequency_Band_Change:
				GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "CONTROL_Frequency_Band_Change\r\n");
				break;
		case CONTROL_Mode_Change:
				GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "CONTROL_Mode_Change\r\n");
				break;
		case CONTROL_Communication_Status_Check:
				GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "CONTROL_Communication_Status_Check\r\n");
				break;
		case CONTROL_Firmware_Version_Check:
				GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "CONTROL_Firmware_Version_Check\r\n");
				break;

		default:
			break;
	}

	if(cControlType >= Industrial_Reserved_Min && cControlType <= Industrial_Reserved_Max)
	{
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "Industrial_Reserved Range %x\r\n", cControlType);
	}
	else if(cControlType >= User_defined_Control_Min && cControlType <= User_defined_Control_Max)
	{
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "User_defined_Control Range %x\r\n", cControlType);
	}
	else if(cControlType >= Reserved_Min && cControlType <= Reserved_Max)
	{
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_NOT, "Reserved  Range %x\r\n", cControlType);
	}

	 OG_Ctrl_Recv( pstGMMPHeader, pstReqHdr);

	return 1;
}

void SetTID(int nTid)
{
	g_nTID = nTid;
}

int GetTID()
{
	return g_nTID;
}

//protected
int SetHeader(void* pData,
		int nPacketSize,
		int nTotalCount,
		int nCurrentCount,
		const char cMessageType,
		const char* pszAuthID,
		const char* pszAuthKey,
		const char cEncryption)
{
	if(pData == NULL || pszAuthID == NULL)
	{
		return LIB_PARAM_ERROR;
	}

	GMMPHeader* pHeader = (GMMPHeader*)pData;
	ConvertShort cvtShort;

	memset(pHeader, 0, sizeof(GMMPHeader) );
	memset(cvtShort.usShort, 0, sizeof(cvtShort.usShort) );

	pHeader->ucVersion = GMMP_VERSION;

	ConvertInt cvtInt;

	//////////////////////////////////////////////////////////////////////////////
	// sekim 20151222 TimeStamp in GMMP with host_rtos_get_time()
	/*
	time_t t = time(NULL);
	memcpy(cvtInt.usInt  , &t, sizeof(cvtInt.usInt) );
	*/
	unsigned int time_current = host_rtos_get_time()*1000;
	memcpy(cvtInt.usInt  , &time_current, sizeof(cvtInt.usInt) );
	//////////////////////////////////////////////////////////////////////////////

	cvtInt.sU8 = ltobi(cvtInt.sU8);

	memcpy(&pHeader->unOriginTimeStamp, &cvtInt.usInt, sizeof(cvtInt.usInt) );
	memcpy(&pHeader->usAuthID, pszAuthID, strlen(pszAuthID));

	if(pszAuthKey != NULL)
	{
		memcpy(&pHeader->usAuthKey, pszAuthKey, strlen(pszAuthKey));
	}

	memcpy(&cvtInt.usInt, &g_nTID, sizeof(g_nTID) );
	cvtInt.sU8 = ltobi(cvtInt.sU8);

	memcpy(pHeader->usTID, &cvtInt.usInt, 4);

	cvtShort.sU8 = ltobs((short)nPacketSize);
	memcpy(pHeader->usMessageLength, cvtShort.usShort, sizeof(cvtShort.usShort) );

	cvtShort.sU8 =  ltobs((short)nTotalCount);
	memcpy(pHeader->usTotalCount, cvtShort.usShort, sizeof(cvtShort.usShort) );

	cvtShort.sU8 =  ltobs((short)nCurrentCount);
	memcpy(pHeader->usCurrentCount, cvtShort.usShort, sizeof(cvtShort.usShort) );

	pHeader->ucMessageType = cMessageType;

	pHeader->ucReserved1 = cEncryption;

	return GMMP_SUCCESS;
}

int SetIntiSocket()
{
	// sekim 20151222 not try to connect when Initializaion
	//return Connect();
	return GMMP_SUCCESS;
}

void	SetbLog(int bDebug)
{
	g_bLog = bDebug;
}

int GetbLog()
{
	return g_bLog;
}

void SetErrorLevel(int nErrorLevel)
{
	g_nErrorLevel = nErrorLevel;

	return;
}

int GetErrorLevel()
{
	return g_nErrorLevel;
}

char* MallocBody(const char Type, int* nBodySize)
{
	char* pBuffer = NULL;
//	int nBodySize = 0;
	switch(Type)
	{
		case OPERATION_GW_REG_RSP:
		{
			GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_RSP, "[OMP->GW]OPERATION_GW_REG_RSP Start \r\n");

			pBuffer = (char*)malloc(sizeof(stGwRegistrationRspHdr) );
			*nBodySize = sizeof(stGwRegistrationRspHdr);
			break;
		}
		case OPERATION_GW_DEREG_RSP:
		{
			GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_RSP,"[OMP->GW]OPERATION_GW_DEREG_RSP Start \r\n");

			pBuffer = (char*)malloc(sizeof(stGwDeRegistrationRspHdr) );
			*nBodySize = sizeof(stGwDeRegistrationRspHdr);
			break;
		}
		case OPERATION_PROFILE_RSP:
		{
			GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_RSP,"[OMP->GW]OPERATION_PROFILE_RSP Start \r\n");

			pBuffer = (char*)malloc(sizeof(stProfileRspHdr) );
			*nBodySize = sizeof(stProfileRspHdr);
			break;
		}
		case OPERATION_DEVICE_REG_RSP:
		{
			GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_RSP,"[OMP->GW]OPERATION_DEVICE_REG_RSP Start \r\n");

			pBuffer = (char*)malloc(sizeof(stDeviceRegistrationRspHdr) );
			*nBodySize = sizeof(stDeviceRegistrationRspHdr);
			break;
		}
		case OPERATION_DEVICE_DEREG_RSP:
		{
			GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_RSP,"[OMP->GW]OPERATION_DEVICE_DEREG_RSP Start \r\n");

			pBuffer = (char*)malloc(sizeof(stDeviceDeRegistrationRspHdr) );
			*nBodySize = sizeof(stDeviceDeRegistrationRspHdr);
			break;
		}
		case OPERATION_DELIVERY_RSP:
		{
			GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_RSP,"[OMP->GW]OPERATION_DELIVERY_RSP Start \r\n");

			pBuffer = (char*)malloc(sizeof(stPacketDeliveryRspHdr) );
			*nBodySize = sizeof(stPacketDeliveryRspHdr);
			break;
		}
		case OPERATION_CONTROL_REQ:
		{
			GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_OPS,"[OMP->GW]OPERATION_CONTROL_REQ Start \r\n");

			pBuffer = (char*)malloc(sizeof(stControlReqHdr) );
			*nBodySize = sizeof(stControlReqHdr);
			break;
		}
		case OPERATION_HEARTBEAT_RSP:
		{
			GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_RSP,"[OMP->GW]OPERATION_HEARTBEAT_RSP Start \r\n");

			pBuffer = (char*)malloc(sizeof(stHeartBeatMsgRspHdr) );
			*nBodySize = sizeof(stHeartBeatMsgRspHdr);
			break;
		}
		case OPERATION_LSENTENCE_RSP:
		{
			GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_RSP,"[OMP->GW]OPERATION_LSENTENCE_RSP Start \r\n");

			pBuffer = (char*)malloc(sizeof(stLSentenceRspHdr) );
			*nBodySize = sizeof(stLSentenceRspHdr);
			break;
		}
		case OPERATION_FTP_INFO_RSP:
		{
			GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_RSP,"[OMP->GW]OPERATION_FTP_INFO_RSP Start \r\n");

			pBuffer = (char*)malloc(sizeof(stFtpInfoRspHdr) );
			*nBodySize = sizeof(stFtpInfoRspHdr);
			break;
		}
		case OPERATION_REMOTE_INFO_RSP:
		{
			GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_RSP,"[OMP->GW]OPERATION_REMOTE_INFO_RSP Start \r\n");

			pBuffer = (char*)malloc(sizeof(stRemoteAccessInfoRspHdr) );
			*nBodySize = sizeof(stRemoteAccessInfoRspHdr);
			break;
		}
		case OPERATION_NOTIFICATION_RSP:
		{
			GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_RSP,"[OMP->GW]OPERATION_NOTIFICATION_RSP Start \r\n");

			pBuffer = (char*)malloc(sizeof(stNotificationRspHdr) );
			*nBodySize = sizeof(stNotificationRspHdr);
			break;
		}
		case OPERATION_ENCRYPTION_INFO_RSP:
		{
			GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_RSP,"[OMP->GW]OPERATION_ENCRYPTION_INFO_RSP Start \r\n");

			pBuffer = (char*)malloc(sizeof(stEncryptionInfoRspHdr) );
			*nBodySize = sizeof(stEncryptionInfoRspHdr);
			break;
		}
		case OPERATION_ENCRYPTION_KEY_RSP:
		{
			GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_RSP,"[OMP->GW]OPERATION_ENCRYPTION_KEY_RSP Start \r\n");

			pBuffer = (char*)malloc(sizeof(stEncryptionKeyRspHdr) );
			*nBodySize = sizeof(stEncryptionKeyRspHdr);
			break;
		}
		case OPERATION_MULTIMEDIA_URL_INFO_RSP:
		{
			GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_RSP,"[OMP->GW]OPERATION_MULTIMEDIA_URL_INFO_RSP Start \r\n");

			pBuffer = (char*)malloc(sizeof(stMultimediaURLInfoRspHdr) );
			*nBodySize = sizeof(stMultimediaURLInfoRspHdr);
			break;
		}
		case OPERATION_LOB_CLOUD_INFO_RSP:
		{
			GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_RSP,"[OMP->GW]OPERATION_LOB_CLOUD_INFO_RSP Start \r\n");

			pBuffer = (char*)malloc(sizeof(stLOBCloudAccessInfoRspHdr) );
			*nBodySize = sizeof(stLOBCloudAccessInfoRspHdr);
			break;
		}
		case OPERATION_LOB_FTP_INFO_RSP:
		{
			GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_RSP,"[OMP->GW]OPERATION_LOB_FTP_INFO_RSP Start \r\n");

			pBuffer = (char*)malloc(sizeof(stLOBFTPAccessInfoRspHdr) );
			*nBodySize = sizeof(stLOBFTPAccessInfoRspHdr);
			break;
		}
		case OPERATION_LOB_UPLOAD_NOTIFICATION_RSP:
		{
			GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_RSP,"[OMP->GW]OPERATION_LOB_UPLOAD_NOTIFICATION_RSP Start \r\n");

			pBuffer = (char*)malloc(sizeof(stLOBUploadNotificationRspHdr) );
			*nBodySize = sizeof(stLOBUploadNotificationRspHdr);
			break;
		}
		default:
		{
			nBodySize = 0;
		}
	}

	return pBuffer;
}

int GMMP_Trace(GMMPHeader* pstGMMPHeader, void* pBody)
{
	ConvertInt cvtint;

	char cMessageType =  pstGMMPHeader->ucMessageType ;

	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "AuthID =%.*s\r\n", strlen((char*)pstGMMPHeader->usAuthID), pstGMMPHeader->usAuthID);
	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Auth Key =%.*s\r\n", strlen((char*)pstGMMPHeader->usAuthKey), pstGMMPHeader->usAuthKey);

	memcpy(cvtint.usInt, pstGMMPHeader->usTID, sizeof(pstGMMPHeader->usTID) );
	cvtint.sU8 = btoli(cvtint.sU8);

	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "T-ID =%.d\r\n", cvtint.sU8 );

	if(cMessageType == OPERATION_GW_REG_REQ)
	{
		stGwRegistrationReqHdr* pstRspHdr =(stGwRegistrationReqHdr*) pBody;

		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Domain Code =%.*s\r\n", strlen((char*)pstRspHdr->usDomainCode), pstRspHdr->usDomainCode);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "ManufactureID =%.*s\r\n", strlen((char*)pstRspHdr->usManufactureID), pstRspHdr->usManufactureID);
	}
	else if(cMessageType == OPERATION_GW_REG_RSP)
	{
		stGwRegistrationRspHdr* pstRspHdr =(stGwRegistrationRspHdr*) pBody;

		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Domain Code =%.*s\r\n", strlen((char*)pstRspHdr->usDomainCode), pstRspHdr->usDomainCode);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "GW ID = %.*s\r\n", strlen((char*)pstRspHdr->usGWID), pstRspHdr->usGWID);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Result Code[0x%x], %s\r\n", pstRspHdr->ucResultCode, GetStringtoOMPErrorCode(pstRspHdr->ucResultCode) );
	}
	else if(cMessageType == OPERATION_GW_DEREG_REQ)
	{
		stGwDeRegistrationReqHdr* pstRspHdr =(stGwDeRegistrationReqHdr*) pBody;

		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Domain Code =%.*s\r\n", strlen((char*)pstRspHdr->usDomainCode), pstRspHdr->usDomainCode);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "GW ID = %.*s\r\n", strlen((char*)pstRspHdr->usGWID), pstRspHdr->usGWID);
	}
	else if(cMessageType == OPERATION_GW_DEREG_RSP)
	{
		stGwDeRegistrationRspHdr* pstRspHdr =(stGwDeRegistrationRspHdr*) pBody;

		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Domain Code =%.*s\r\n", strlen((char*)pstRspHdr->usDomainCode), pstRspHdr->usDomainCode);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "GW ID = %.*s\r\n", strlen((char*)pstRspHdr->usGWID), pstRspHdr->usGWID);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Result Code[0x%x], %s\r\n", pstRspHdr->ucResultCode, GetStringtoOMPErrorCode(pstRspHdr->ucResultCode) );
	}
	else if(cMessageType  == OPERATION_PROFILE_REQ)
	{
		stProfileReqHdr* pstRspHdr =(stProfileReqHdr*) pBody;

		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Domain Code =%.*s\r\n", strlen((char*)pstRspHdr->usDomainCode), pstRspHdr->usDomainCode);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "GW ID = %.*s\r\n", strlen((char*)pstRspHdr->usGWID), pstRspHdr->usGWID);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Device ID=%.*s\r\n", strlen((char*)pstRspHdr->usDeviceID), pstRspHdr->usDeviceID);
	}
	else if(cMessageType  == OPERATION_PROFILE_RSP)
	{
		stProfileRspHdr* pstRspHdr =(stProfileRspHdr*) pBody;

		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Domain Code =%.*s\r\n", strlen((char*)pstRspHdr->usDomainCode), pstRspHdr->usDomainCode);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "GW ID=%.*s\r\n", strlen((char*)pstRspHdr->usGWID), pstRspHdr->usGWID);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Device ID =%.*s\r\n", strlen((char*)pstRspHdr->usDeviceID), pstRspHdr->usDeviceID);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Result Code[0x%x], %s\r\n", pstRspHdr->ucResultCode, GetStringtoOMPErrorCode(pstRspHdr->ucResultCode) );

		memcpy(cvtint.usInt, pstRspHdr->unHeartbeatPeriod, sizeof(pstRspHdr->unHeartbeatPeriod) );
		cvtint.sU8 = btoli(cvtint.sU8);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "unHeartbeatPeriod = %d\r\n", cvtint.sU8);

		memcpy(cvtint.usInt, pstRspHdr->unReportPeriod, sizeof(pstRspHdr->unReportPeriod) );
		cvtint.sU8 = btoli(cvtint.sU8);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "unReportPeriod = %d\r\n", cvtint.sU8);

		memcpy(cvtint.usInt, pstRspHdr->unReportOffset, sizeof(pstRspHdr->unReportOffset) );
		cvtint.sU8 = btoli(cvtint.sU8);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "unReportOffset = %d\r\n", cvtint.sU8);

		memcpy(cvtint.usInt, pstRspHdr->unResponseTimeout, sizeof(pstRspHdr->unResponseTimeout) );
		cvtint.sU8 = btoli(cvtint.sU8);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "unResponseTimeout = %d\r\n", cvtint.sU8);


		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "usModel =%.*s\r\n", strlen((char*)pstRspHdr->usModel), pstRspHdr->usModel);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "usFirmwareVersion =%.*s\r\n", strlen((char*)pstRspHdr->usFirmwareVersion), pstRspHdr->usFirmwareVersion);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "usSoftwareVersion =%.*s\r\n", strlen((char*)pstRspHdr->usSoftwareVersion), pstRspHdr->usSoftwareVersion);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "usHardwareVersion =%.*s\r\n", strlen((char*)pstRspHdr->usHardwareVersion), pstRspHdr->usHardwareVersion);
	}
	else if(cMessageType == OPERATION_DEVICE_REG_REQ)
	{
		stDeviceRegistrationReqHdr* pstRspHdr =(stDeviceRegistrationReqHdr*) pBody;

		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Domain Code =%.*s\r\n", strlen((char*)pstRspHdr->usDomainCode), pstRspHdr->usDomainCode);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "GW ID=%.*s\r\n", strlen((char*)pstRspHdr->usGWID), pstRspHdr->usGWID);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "ManufactureID =%.*s\r\n", strlen((char*)pstRspHdr->usManufactureID), pstRspHdr->usManufactureID);
	}
	else if(cMessageType == OPERATION_DEVICE_REG_RSP)
	{
		stDeviceRegistrationRspHdr* pstRspHdr =(stDeviceRegistrationRspHdr*) pBody;

		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Domain Code =%.*s\r\n", strlen((char*)pstRspHdr->usDomainCode), pstRspHdr->usDomainCode);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "GW ID=%.*s\r\n", strlen((char*)pstRspHdr->usGWID), pstRspHdr->usGWID);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Device ID =%.*s\r\n", strlen((char*)pstRspHdr->usDeviceID), pstRspHdr->usDeviceID);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Result Code[0x%x], %s\r\n", pstRspHdr->ucResultCode, GetStringtoOMPErrorCode(pstRspHdr->ucResultCode) );
	}
	else if(cMessageType == OPERATION_DEVICE_DEREG_REQ)
	{
		stDeviceDeRegistrationReqHdr* pstRspHdr =(stDeviceDeRegistrationReqHdr*) pBody;

		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Domain Code =%.*s\r\n", strlen((char*)pstRspHdr->usDomainCode), pstRspHdr->usDomainCode);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "GW ID=%.*s\r\n", strlen((char*)pstRspHdr->usGWID), pstRspHdr->usGWID);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Device ID =%.*s\r\n", strlen((char*)pstRspHdr->usDeviceID), pstRspHdr->usDeviceID);
	}
	else if(cMessageType == OPERATION_DEVICE_DEREG_RSP)
	{
		stDeviceDeRegistrationRspHdr* pstRspHdr =(stDeviceDeRegistrationRspHdr*) pBody;

		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Domain Code =%.*s\r\n", strlen((char*)pstRspHdr->usDomainCode), pstRspHdr->usDomainCode);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "GW ID=%.*s\r\n", strlen((char*)pstRspHdr->usGWID), pstRspHdr->usGWID);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Device ID =%.*s\r\n", strlen((char*)pstRspHdr->usDeviceID), pstRspHdr->usDeviceID);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Result Code[0x%x], %s\r\n", pstRspHdr->ucResultCode, GetStringtoOMPErrorCode(pstRspHdr->ucResultCode) );
	}
	else if(cMessageType == OPERATION_DELIVERY_REQ)
	{
		stPacketDeliveryReqHdr* pstRspHdr =(stPacketDeliveryReqHdr*) pBody;

		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Domain Code =%.*s\r\n", strlen((char*)pstRspHdr->usDomainCode), pstRspHdr->usDomainCode);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "GW ID=%.*s\r\n", strlen((char*)pstRspHdr->usGWID), pstRspHdr->usGWID);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Device ID =%.*s\r\n", strlen((char*)pstRspHdr->usDeviceID), pstRspHdr->usDeviceID);

		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "ucReportType[0x%x]\r\n", pstRspHdr->ucReportType);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "ucMediaType[0x%x]\r\n", pstRspHdr->ucMediaType);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "usMessageBody =%.*s\r\n", strlen((char*)pstRspHdr->usMessageBody), pstRspHdr->usMessageBody);
	}
	else if(cMessageType  == OPERATION_DELIVERY_RSP)
	{
		stPacketDeliveryRspHdr* pstRspHdr =(stPacketDeliveryRspHdr*) pBody;

		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Domain Code =%.*s\r\n", strlen((char*)pstRspHdr->usDomainCode), pstRspHdr->usDomainCode);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "GW ID=%.*s\r\n", strlen((char*)pstRspHdr->usGWID), pstRspHdr->usGWID);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Device ID =%.*s\r\n", strlen((char*)pstRspHdr->usDeviceID), pstRspHdr->usDeviceID);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Result Code[0x%x], %s\r\n", pstRspHdr->ucResultCode, GetStringtoOMPErrorCode(pstRspHdr->ucResultCode) );

		memcpy(cvtint.usInt, pstRspHdr->unBackOffTime, sizeof(pstRspHdr->unBackOffTime) );
		cvtint.sU8 = btoli(cvtint.sU8);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "unBackOffTime = %d\r\n", cvtint.sU8);
	}
	else if(cMessageType  == OPERATION_CONTROL_REQ)
	{
		stControlReqHdr* pstReqHdr =(stControlReqHdr*) pBody;

		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Domain Code =%.*s\r\n", strlen((char*)pstReqHdr->usDomainCode), pstReqHdr->usDomainCode);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "GW ID=%.*s\r\n", strlen((char*)pstReqHdr->usGWID), pstReqHdr->usGWID);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Device ID =%.*s\r\n", strlen((char*)pstReqHdr->usDeviceID), pstReqHdr->usDeviceID);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Control Code[0x%x]\r\n", pstReqHdr->ucControlType);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "usMessageBody =%.*s\r\n", strlen((char*)pstReqHdr->usMessageBody), pstReqHdr->usMessageBody);
	}
	else if(cMessageType  == OPERATION_CONTROL_RSP)
	{
		stControlRspHdr* pstRspHdr =(stControlRspHdr*) pBody;

		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Domain Code =%.*s\r\n", strlen((char*)pstRspHdr->usDomainCode), pstRspHdr->usDomainCode);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "GW ID=%.*s\r\n", strlen((char*)pstRspHdr->usGWID), pstRspHdr->usGWID);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Device ID =%.*s\r\n", strlen((char*)pstRspHdr->usDeviceID), pstRspHdr->usDeviceID);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "ucResultCode Code[0x%x]\r\n", pstRspHdr->ucResultCode);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Control Code[0x%x]\r\n", pstRspHdr->ucControlType);
	}
	else if(cMessageType  == OPERATION_HEARTBEAT_REQ)
	{
		stHeartBeatMsgRspHdr* pstRspHdr =(stHeartBeatMsgRspHdr*) pBody;

		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Domain Code =%.*s\r\n", strlen((char*)pstRspHdr->usDomainCode), pstRspHdr->usDomainCode);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "GW ID=%.*s\r\n", strlen((char*)pstRspHdr->usGWID), pstRspHdr->usGWID);

	}
	else if(cMessageType  == OPERATION_HEARTBEAT_RSP)
	{
		stHeartBeatMsgRspHdr* pstRspHdr =(stHeartBeatMsgRspHdr*) pBody;

		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Domain Code =%.*s\r\n", strlen((char*)pstRspHdr->usDomainCode), pstRspHdr->usDomainCode);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "GW ID=%.*s\r\n", strlen((char*)pstRspHdr->usGWID), pstRspHdr->usGWID);
	}
	else if(cMessageType  == OPERATION_LSENTENCE_REQ)
	{
		stLSentenceReqHdr* pstRspHdr =(stLSentenceReqHdr*) pBody;

		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Domain Code =%.*s\r\n", strlen((char*)pstRspHdr->usDomainCode), pstRspHdr->usDomainCode);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "GW ID=%.*s\r\n", strlen((char*)pstRspHdr->usGWID), pstRspHdr->usGWID);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Device ID =%.*s\r\n", strlen((char*)pstRspHdr->usDeviceID), pstRspHdr->usDeviceID);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Control Code[0x%x]\r\n", pstRspHdr->ucControlType);
	}
	else if(cMessageType  == OPERATION_LSENTENCE_RSP)
	{
		stLSentenceRspHdr* pstRspHdr =(stLSentenceRspHdr*) pBody;

		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Domain Code =%.*s\r\n", strlen((char*)pstRspHdr->usDomainCode), pstRspHdr->usDomainCode);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "GW ID=%.*s\r\n", strlen((char*)pstRspHdr->usGWID), pstRspHdr->usGWID);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Device ID =%.*s\r\n", strlen((char*)pstRspHdr->usDeviceID), pstRspHdr->usDeviceID);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Result Code[0x%x], %s\r\n", pstRspHdr->ucResultCode, GetStringtoOMPErrorCode(pstRspHdr->ucResultCode) );
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Control Code[0x%x]\r\n", pstRspHdr->ucControlType);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "MessageBody =%.*s\r\n", strlen((char*)pstRspHdr->usMessageBody), pstRspHdr->usMessageBody);
	}
	else if(cMessageType  == OPERATION_FTP_INFO_REQ)
	{
		stFtpInfoReqHdr* pstRspHdr =(stFtpInfoReqHdr*) pBody;

		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Domain Code =%.*s\r\n", strlen((char*)pstRspHdr->usDomainCode), pstRspHdr->usDomainCode);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "GW ID=%.*s\r\n", strlen((char*)pstRspHdr->usGWID), pstRspHdr->usGWID);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Device ID =%.*s\r\n", strlen((char*)pstRspHdr->usDeviceID), pstRspHdr->usDeviceID);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Control Code[0x%x]\r\n", pstRspHdr->ucControlType);
	}
	else if(cMessageType  == OPERATION_FTP_INFO_RSP)
	{
		stFtpInfoRspHdr* pstRspHdr =(stFtpInfoRspHdr*) pBody;

		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Domain Code =%.*s\r\n", strlen((char*)pstRspHdr->usDomainCode), pstRspHdr->usDomainCode);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "GW ID=%.*s\r\n", strlen((char*)pstRspHdr->usGWID), pstRspHdr->usGWID);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Device ID =%.*s\r\n", strlen((char*)pstRspHdr->usDeviceID), pstRspHdr->usDeviceID);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Result Code[0x%x], %s\r\n", pstRspHdr->ucResultCode, GetStringtoOMPErrorCode(pstRspHdr->ucResultCode) );
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Control Code[0x%x]\r\n", pstRspHdr->ucControlType);

		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "FTP ServerIP =%.*s\r\n", strlen((char*)pstRspHdr->usFTPServerIP), pstRspHdr->usFTPServerIP);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "FTP ServerID =%.*s\r\n", strlen((char*)pstRspHdr->usFTPServerID), pstRspHdr->usFTPServerID);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "FTP ServerPW =%.*s\r\n", strlen((char*)pstRspHdr->usFTPServerPW), pstRspHdr->usFTPServerPW);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "File Path =%.*s\r\n", strlen((char*)pstRspHdr->usFilePath), pstRspHdr->usFilePath);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "File Name =%.*s\r\n", strlen((char*)pstRspHdr->usFileName), pstRspHdr->usFileName);

		memcpy(cvtint.usInt, pstRspHdr->uunFileSize, sizeof(pstRspHdr->uunFileSize) );
		cvtint.sU8 = btoli(cvtint.sU8);

		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "File Size =%d\r\n", cvtint.sU8);
	}
	else if(cMessageType  == OPERATION_REMOTE_INFO_REQ)
	{
		stRemoteAccessInfoReqHdr* pstRspHdr =(stRemoteAccessInfoReqHdr*) pBody;

		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Domain Code =%.*s\r\n", strlen((char*)pstRspHdr->usDomainCode), pstRspHdr->usDomainCode);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "GW ID=%.*s\r\n", strlen((char*)pstRspHdr->usGWID), pstRspHdr->usGWID);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Device ID =%.*s\r\n", strlen((char*)pstRspHdr->usDeviceID), pstRspHdr->usDeviceID);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Control Code[0x%x]\r\n", pstRspHdr->ucControlType);
	}
	else if(cMessageType  == OPERATION_REMOTE_INFO_RSP)
	{
		stRemoteAccessInfoRspHdr* pstRspHdr =(stRemoteAccessInfoRspHdr*) pBody;

		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Domain Code =%.*s\r\n", strlen((char*)pstRspHdr->usDomainCode), pstRspHdr->usDomainCode);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "GW ID=%.*s\r\n", strlen((char*)pstRspHdr->usGWID), pstRspHdr->usGWID);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Device ID =%.*s\r\n", strlen((char*)pstRspHdr->usDeviceID), pstRspHdr->usDeviceID);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Result Code[0x%x], %s\r\n", pstRspHdr->ucResultCode, GetStringtoOMPErrorCode(pstRspHdr->ucResultCode) );
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Control Code[0x%x]\r\n", pstRspHdr->ucControlType);

		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Remote ServerIP =%.*s\r\n", strlen((char*)pstRspHdr->usRemoteServerIP), pstRspHdr->usRemoteServerIP);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Remote ServerID =%.*s\r\n", strlen((char*)pstRspHdr->usRemoteServerUserID), pstRspHdr->usRemoteServerUserID);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Remote ServerPW =%.*s\r\n", strlen((char*)pstRspHdr->usRemoteServerUserPW), pstRspHdr->usRemoteServerUserPW);
	}
	else if(cMessageType  == OPERATION_NOTIFICATION_REQ)
	{
		stNotificationReqHdr* pstRspHdr =(stNotificationReqHdr*) pBody;

		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Domain Code =%.*s\r\n", strlen((char*)pstRspHdr->usDomainCode), pstRspHdr->usDomainCode);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "GW ID=%.*s\r\n", strlen((char*)pstRspHdr->usGWID), pstRspHdr->usGWID);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Device ID =%.*s\r\n", strlen((char*)pstRspHdr->usDeviceID), pstRspHdr->usDeviceID);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Result Code[0x%x], %s\r\n", pstRspHdr->ucResultCode, GetStringtoOMPErrorCode(pstRspHdr->ucResultCode) );
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Control Code[0x%x]\r\n", pstRspHdr->ucControlType);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "MessageBody =%.*s\r\n", strlen((char*)pstRspHdr->usMessageBody), pstRspHdr->usMessageBody);
	}
	else if(cMessageType  == OPERATION_NOTIFICATION_RSP)
	{
		stNotificationRspHdr* pstRspHdr =(stNotificationRspHdr*) pBody;

		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Domain Code =%.*s\r\n", strlen((char*)pstRspHdr->usDomainCode), pstRspHdr->usDomainCode);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "GW ID=%.*s\r\n", strlen((char*)pstRspHdr->usGWID), pstRspHdr->usGWID);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Device ID =%.*s\r\n", strlen((char*)pstRspHdr->usDeviceID), pstRspHdr->usDeviceID);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Result Code[0x%x], %s\r\n", pstRspHdr->ucResultCode, GetStringtoOMPErrorCode(pstRspHdr->ucResultCode) );
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Control Code[0x%x]\r\n", pstRspHdr->ucControlType);
	}
	else if(cMessageType  == OPERATION_ENCRYPTION_INFO_REQ)
	{
		stEncryptionInfoReqHdr* pstRspHdr =(stEncryptionInfoReqHdr*) pBody;

		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Domain Code =%.*s\r\n", strlen((char*)pstRspHdr->usDomainCode), pstRspHdr->usDomainCode);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "GW ID=%.*s\r\n", strlen((char*)pstRspHdr->usGWID), pstRspHdr->usGWID);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Device ID=%.*s\r\n", strlen((char*)pstRspHdr->usDeviceID), pstRspHdr->usDeviceID);
	}

	else if(cMessageType  == OPERATION_ENCRYPTION_INFO_RSP)
	{
		stEncryptionInfoRspHdr* pstRspHdr =(stEncryptionInfoRspHdr*) pBody;

		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Domain Code =%.*s\r\n", strlen((char*)pstRspHdr->usDomainCode), pstRspHdr->usDomainCode);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "GW ID=%.*s\r\n", strlen((char*)pstRspHdr->usGWID), pstRspHdr->usGWID);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Device ID =%.*s\r\n", strlen((char*)pstRspHdr->usDeviceID), pstRspHdr->usDeviceID);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Encryption Flag [0x%x]\r\n", pstRspHdr->ucEncryptionFlag );
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Encryption Algorithm [0x%x]\r\n", pstRspHdr->ucEncryptionAlgorithm );
	}
	else if(cMessageType  == OPERATION_ENCRYPTION_KEY_REQ)
	{
		stEncryptionKeyReqHdr* pstRspHdr =(stEncryptionKeyReqHdr*) pBody;

		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Domain Code =%.*s\r\n", strlen((char*)pstRspHdr->usDomainCode), pstRspHdr->usDomainCode);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "GW ID=%.*s\r\n", strlen((char*)pstRspHdr->usGWID), pstRspHdr->usGWID);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Device ID =%.*s\r\n", strlen((char*)pstRspHdr->usDeviceID), pstRspHdr->usDeviceID);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "usEncryptionKey =%.*s\r\n", strlen((char*)pstRspHdr->usEncryptionKey), pstRspHdr->usEncryptionKey);
	}
	else if(cMessageType  == OPERATION_ENCRYPTION_KEY_RSP)
	{
		stEncryptionKeyRspHdr* pstRspHdr =(stEncryptionKeyRspHdr*) pBody;

		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Domain Code =%.*s\r\n", strlen((char*)pstRspHdr->usDomainCode), pstRspHdr->usDomainCode);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "GW ID=%.*s\r\n", strlen((char*)pstRspHdr->usGWID), pstRspHdr->usGWID);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Device ID =%.*s\r\n", strlen((char*)pstRspHdr->usDeviceID), pstRspHdr->usDeviceID);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Result Code[0x%x], %s\r\n", pstRspHdr->ucResultCode, GetStringtoOMPErrorCode(pstRspHdr->ucResultCode) );
	}
	else if(cMessageType  == OPERATION_MULTIMEDIA_URL_INFO_REQ)
	{
		stMultimediaURLInfoReqHdr* pstRspHdr =(stMultimediaURLInfoReqHdr*) pBody;

		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Domain Code =%.*s\r\n", strlen((char*)pstRspHdr->usDomainCode), pstRspHdr->usDomainCode);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "GW ID=%.*s\r\n", strlen((char*)pstRspHdr->usGWID), pstRspHdr->usGWID);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Device ID =%.*s\r\n", strlen((char*)pstRspHdr->usDeviceID), pstRspHdr->usDeviceID);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Control Code[0x%x]\r\n", pstRspHdr->ucControlType);
	}
	else if(cMessageType  == OPERATION_MULTIMEDIA_URL_INFO_RSP)
	{
		stMultimediaURLInfoRspHdr* pstRspHdr =(stMultimediaURLInfoRspHdr*) pBody;

		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Domain Code =%.*s\r\n", strlen((char*)pstRspHdr->usDomainCode), pstRspHdr->usDomainCode);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "GW ID=%.*s\r\n", strlen((char*)pstRspHdr->usGWID), pstRspHdr->usGWID);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Device ID =%.*s\r\n", strlen((char*)pstRspHdr->usDeviceID), pstRspHdr->usDeviceID);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Result Code[0x%x], %s\r\n", pstRspHdr->ucResultCode, GetStringtoOMPErrorCode(pstRspHdr->ucResultCode) );

		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Server URL =%.*s\r\n", strlen((char*)pstRspHdr->usServerURL), pstRspHdr->usServerURL);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Server ID =%.*s\r\n", strlen((char*)pstRspHdr->usServerID), pstRspHdr->usServerID);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Server PW =%.*s\r\n", strlen((char*)pstRspHdr->usServerPassword), pstRspHdr->usServerPassword);
	}
	else if(cMessageType  == OPERATION_LOB_CLOUD_INFO_REQ)
	{
		stLOBCloudAccessInfoReqHdr* pstRspHdr =(stLOBCloudAccessInfoReqHdr*) pBody;

		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Domain Code =%.*s\r\n", strlen((char*)pstRspHdr->usDomainCode), pstRspHdr->usDomainCode);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "GW ID=%.*s\r\n", strlen((char*)pstRspHdr->usGWID), pstRspHdr->usGWID);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Device ID =%.*s\r\n", strlen((char*)pstRspHdr->usDeviceID), pstRspHdr->usDeviceID);
	}
	else if(cMessageType  == OPERATION_LOB_CLOUD_INFO_RSP)
	{
		stLOBCloudAccessInfoRspHdr* pstRspHdr =(stLOBCloudAccessInfoRspHdr*) pBody;

		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Domain Code =%.*s\r\n", strlen((char*)pstRspHdr->usDomainCode), pstRspHdr->usDomainCode);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "GW ID=%.*s\r\n", strlen((char*)pstRspHdr->usGWID), pstRspHdr->usGWID);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Device ID =%.*s\r\n", strlen((char*)pstRspHdr->usDeviceID), pstRspHdr->usDeviceID);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Result Code[0x%x], %s\r\n", pstRspHdr->ucResultCode, GetStringtoOMPErrorCode(pstRspHdr->ucResultCode) );

		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Access Key =%.*s\r\n", strlen((char*)pstRspHdr->usAccessKey), pstRspHdr->usAccessKey);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Secret Key =%.*s\r\n", strlen((char*)pstRspHdr->usSecretKey), pstRspHdr->usSecretKey);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Directory Path =%.*s\r\n", strlen((char*)pstRspHdr->usDirectoryPath), pstRspHdr->usDirectoryPath);
	}
	else if(cMessageType  == OPERATION_LOB_FTP_INFO_REQ)
	{
		stLOBFTPAccessInfoReqHdr* pstRspHdr =(stLOBFTPAccessInfoReqHdr*) pBody;

		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Domain Code =%.*s\r\n", strlen((char*)pstRspHdr->usDomainCode), pstRspHdr->usDomainCode);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "GW ID=%.*s\r\n", strlen((char*)pstRspHdr->usGWID), pstRspHdr->usGWID);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Device ID =%.*s\r\n", strlen((char*)pstRspHdr->usDeviceID), pstRspHdr->usDeviceID);
	}
	else if(cMessageType  == OPERATION_LOB_FTP_INFO_RSP)
	{
		stLOBFTPAccessInfoRspHdr* pstRspHdr =(stLOBFTPAccessInfoRspHdr*) pBody;

		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Domain Code =%.*s\r\n", strlen((char*)pstRspHdr->usDomainCode), pstRspHdr->usDomainCode);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "GW ID=%.*s\r\n", strlen((char*)pstRspHdr->usGWID), pstRspHdr->usGWID);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Device ID =%.*s\r\n", strlen((char*)pstRspHdr->usDeviceID), pstRspHdr->usDeviceID);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Result Code[0x%x], %s\r\n", pstRspHdr->ucResultCode, GetStringtoOMPErrorCode(pstRspHdr->ucResultCode) );

		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "FTP IP =%.*s\r\n", strlen((char*)pstRspHdr->usFTPServerIP), pstRspHdr->usFTPServerIP);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "FTP ID =%.*s\r\n", strlen((char*)pstRspHdr->usFTPServerID), pstRspHdr->usFTPServerID);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "FTP PW =%.*s\r\n", strlen((char*)pstRspHdr->usFTPServerPW), pstRspHdr->usFTPServerPW);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "File Path =%.*s\r\n", strlen((char*)pstRspHdr->usFilePath), pstRspHdr->usFilePath);
	}
	else if(cMessageType  == OPERATION_LOB_UPLOAD_NOTIFICATION_REQ)
	{
		stLOBUploadNotificationReqHdr* pstRspHdr =(stLOBUploadNotificationReqHdr*) pBody;

		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Domain Code =%.*s\r\n", strlen((char*)pstRspHdr->usDomainCode), pstRspHdr->usDomainCode);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "GW ID=%.*s\r\n", strlen((char*)pstRspHdr->usGWID), pstRspHdr->usGWID);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Device ID =%.*s\r\n", strlen((char*)pstRspHdr->usDeviceID), pstRspHdr->usDeviceID);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "ucStorageType Code[0x%x]\r\n", pstRspHdr->ucStorageType);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "usLOBPath =%.*s\r\n", strlen((char*)pstRspHdr->usLOBPath), pstRspHdr->usLOBPath);
	}
	else if(cMessageType  == OPERATION_LOB_UPLOAD_NOTIFICATION_RSP)
	{
		stLOBUploadNotificationRspHdr* pstRspHdr =(stLOBUploadNotificationRspHdr*) pBody;

		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Domain Code =%.*s\r\n", strlen((char*)pstRspHdr->usDomainCode), pstRspHdr->usDomainCode);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "GW ID=%.*s\r\n", strlen((char*)pstRspHdr->usGWID), pstRspHdr->usGWID);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Device ID =%.*s\r\n", strlen((char*)pstRspHdr->usDeviceID), pstRspHdr->usDeviceID);
		GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG_DATA, GMMP_LOG_MARKET_NOT, "Result Code[0x%x], %s\r\n", pstRspHdr->ucResultCode, GetStringtoOMPErrorCode(pstRspHdr->ucResultCode) );
	}

	return GMMP_SUCCESS;
}

int GMMP_Printf(const unsigned int nErrorLevel, const unsigned int nLogMakerLevel, char *__restrict __format, ...)
{
	char *pszWriteBuffer;
	pszWriteBuffer = (char*)malloc(LEN_LOG_DATA);
	memset(pszWriteBuffer, 0, LEN_LOG_DATA);

	va_list argptr;
	va_start(argptr, __format);
	vsprintf(pszWriteBuffer, __format, argptr);
	va_end(argptr);

	int nRet = GMMP_SUCCESS;
	if(nLogMakerLevel == GMMP_LOG_MARKET_NOT)
	{
		nRet =  GMMP_PrintfDebug(nErrorLevel, pszWriteBuffer);
	}
	else
	{
		nRet =  GMMP_PrintfLog(nLogMakerLevel, pszWriteBuffer);
	}

	free(pszWriteBuffer);
	pszWriteBuffer = NULL;

	return nRet;
}

char g_bPrintfDebug = 0;
char g_bPrintfLog = 1;

int GMMP_PrintfDebug(const unsigned int nErrorLevel,  char* pMessage)
{
	// sekim 20151214 add g_bPrintfDebug, g_bPrintfLog
	if ( g_bPrintfDebug==0 )	return 0;

	int bLog = false;

	if( GetErrorLevel() == GMMP_ERROR_LEVEL_ERROR && nErrorLevel == GMMP_ERROR_LEVEL_ERROR )
	{
		bLog = true;
	}
	else if( GetErrorLevel() == GMMP_ERROR_LEVEL_DEBUG && nErrorLevel == GMMP_ERROR_LEVEL_DEBUG )
	{
		bLog = true;
	}
	else if( GetErrorLevel() == GMMP_ERROR_LEVEL_DEBUG && nErrorLevel == GMMP_ERROR_LEVEL_ERROR )
	{
		bLog = true;
	}
	else if( GetErrorLevel() == GMMP_ERROR_LEVEL_DEBUG_DATA)
	{
		bLog = true;
	}

	if(bLog == true)
	{
		char szTime[LEN_TIME_DATE*3];
		memset(szTime, 0, sizeof(szTime) );
		if( _GetDateTime(szTime, sizeof(szTime) ) != GMMP_SUCCESS)
		{
			return GMMP_LOG_LONG_STRING;
		}

		char *pszWriteBuffer;
		pszWriteBuffer = (char*)malloc(LEN_LOG_DATA*3);
		memset(pszWriteBuffer, 0, LEN_LOG_DATA*3);

		if(nErrorLevel == GMMP_ERROR_LEVEL_ERROR)
		{
			sprintf(pszWriteBuffer,"Error %s%s %s", GMMP_DEBUG_STRING, szTime, pMessage);
		}
		else
		{
			sprintf(pszWriteBuffer,"%s%s %s", GMMP_DEBUG_STRING, szTime, pMessage);
		}

		// sekim 20151222 printf => W_DBG2
		//printf(pszWriteBuffer);
		W_DBG2(pszWriteBuffer);

		if(GetbLog() )
		{
			GMMP_Log_Write(pszWriteBuffer, strlen(pszWriteBuffer) );
		}

		free(pszWriteBuffer);
		pszWriteBuffer = NULL;
	}

	return GMMP_SUCCESS;
}

int GMMP_PrintfLog(const unsigned int nLogMakerLevel,  char* pMessage)
{
	// sekim 20151214 add g_bPrintfDebug, g_bPrintfLog
	if ( g_bPrintfLog==0 )	return 0;

	static char szLogMarkerID[LEN_TIME_DATE*4];

	if( GetErrorLevel() == GMMP_ERROR_LEVEL_DEBUG)
	{
		return GMMP_SUCCESS;
	}

	char szTime[LEN_TIME_DATE*3];
	memset(szTime, 0 , sizeof(szTime) );
	if( _GetDateTime(szTime, sizeof(szTime) ) != GMMP_SUCCESS)
	{
		return GMMP_LOG_LONG_STRING;
	}

	char szMarker[GMMP_MARKET_LEN];
	memset(szMarker, 0, sizeof(szMarker) );

	if(nLogMakerLevel == GMMP_LOG_MARKET_OPS)
	{
		memset(szLogMarkerID, 0 , sizeof(szLogMarkerID) );
		// sekim 20151214 for Windows Compile : remove getpid()
		//sprintf(szLogMarkerID,GMMP_LOG_KEY_KEY1, szTime, getpid() );
		szLogMarkerID[0] = 0;
		sprintf(szMarker,GMMP_LOG_KEY_KEY2);
	}
	else if(nLogMakerLevel == GMMP_LOG_MARKET_REQ)
	{
		sprintf(szMarker,GMMP_LOG_KEY_KEY3);
	}
	else if(nLogMakerLevel == GMMP_LOG_MARKET_RSP)
	{
		sprintf(szMarker,GMMP_LOG_KEY_KEY4);
	}
	else if(nLogMakerLevel == GMMP_LOG_MARKET_OPE)
	{
		sprintf(szMarker,GMMP_LOG_KEY_KEY5);
	}
	else if(nLogMakerLevel == GMMP_LOG_MARKET_ERR)
	{
		sprintf(szMarker,GMMP_LOG_KEY_KEY6);
	}

	char *pszWriteBuffer;
	pszWriteBuffer = (char*)malloc(LEN_LOG_DATA+3);
	memset(pszWriteBuffer, 0, LEN_LOG_DATA );

	// sekim 20151222 Clear GMMP Log
	//sprintf(pszWriteBuffer,"%s %s%s%s %s", szTime, GMMP_LOG_KEY_KEY0, szLogMarkerID, szMarker, pMessage);
	sprintf(pszWriteBuffer,"%s%s%s%s %s", szTime, GMMP_LOG_KEY_KEY0, szLogMarkerID, szMarker, pMessage);

	// sekim 20151222 Clear GMMP Log
	//printf(pszWriteBuffer);
	W_DBG2(pszWriteBuffer);

	if(GetbLog() )
	{
		GMMP_Log_Write(pszWriteBuffer, strlen(pszWriteBuffer) );
	}

	free(pszWriteBuffer);
	pszWriteBuffer = NULL;

	return GMMP_SUCCESS;
}
