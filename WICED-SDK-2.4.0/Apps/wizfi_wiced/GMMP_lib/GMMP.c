/**
 * @date 2013/11/21
 * @version 0.0.0.1
 * @file GMMP.c
 **/

#include "../wizfimain/wx_defines.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <time.h>
//#include <pthread.h>
//#include <unistd.h>
#include <stdarg.h>
#include "GMMP.h"

char g_szServerIP[LEN_IP] ; ///< �꽌踰� IP �젙蹂대�� ���옣�븳�떎.
int  g_nServerPort = 0; ///<  �꽌踰� Port �젙蹂대�� ���옣�븳�떎.

int g_nNetworkMode = GMMP_NETWORK_ALYWAYS_OFF;///< GMMP �넻�떊 諛⑹떇�쓣 寃곗젙�븳�떎. GMMP_NETWORK_ALYWAYS_ON : TCP Connection�씠 �빆�긽 �쑀吏��맂�떎. GMMP_NETWORK_ALYWAYS_OFF : Operation�쓣 �닔�뻾 �썑 TCP �꽭�뀡�쓣 �걡�뒗�떎.

char g_szAuthID[LEN_AUTH_ID];///< OMP Portal�쓣 �넻�빐 �궗�쟾�뿉 �벑濡앸맂 M2M GW �쓽 Serial Number ���옣 蹂��닔, �옄�룞�솕瑜� �쐞�빐 �궗�슜�맂�떎.

char g_szAuthKey[LEN_AUTH_KEY]; ///< �벑濡� �젅李� �떆 OMP�뿉�꽌 �븷�떦 諛쏆� AuthKey ���옣 蹂��닔, �옄�룞�솕瑜� �쐞�빐 �궗�슜�맂�떎.

char g_szDomainCode[LEN_DOMAIN_CODE]; ///< OMP Portal�쓣 �넻�빐 �궗�쟾�뿉 �벑濡앸맂 �꽌鍮꾩뒪 �쁺�뿭蹂� 援щ텇 肄붾뱶 ���옣 蹂��닔, �옄�룞�솕瑜� �쐞�빐 �궗�슜�맂�떎.

char g_szGWID[LEN_GW_ID]; ///< OMP�뿉�꽌 �븷�떦 諛쏆� Gateway�쓽 ID 臾몄옄�뿴 ���옣 蹂��닔, �옄�룞�솕瑜� �쐞�빐 �궗�슜�맂�떎.

char g_szGWMFID[LEN_MANUFACTURE_ID] = {0,};
char g_szDeviceID[LEN_DEVICE_ID] = {0,};

int g_bIsEncryptionContinue = false; ///< 二쇨린蹂닿퀬 �뜲�씠�꽣 �븫�샇�솕 �궗�슜 �쑀臾� �뙋�떒 ���옣 蹂��닔, �옄�룞�솕瑜� �쐞�빐 �궗�슜�맂�떎.

/**
 * @brief OMP�꽌踰꾨줈 遺��꽣 �닔�떊�븳 �뙣�궥�쓣 �젣怨듯븷 肄쒕갚�븿�닔 �룷�씤�듃 援ъ“泥�
 * @param pstGMMPHeader GMMP Header�쓽 援ъ“泥� �룷�씤�듃
 * @param pstBody GMMP Body�쓽 援ъ“泥� �룷�씤�듃
 * @return �꽦怨�:0, �떎�뙣: 1�씠�긽 , �뿉�윭肄붾뱶 李몄“
 */
typedef int (*callback_Reg)(GMMPHeader* pstGMMPHeader, void* pstBody);

callback_Reg g_CallFunctionRegRecv = NULL; ///< OMP�꽌踰꾨줈 遺��꽣 �닔�떊�븳 �뙣�궥�쓣 �젣怨듯븷 肄쒕갚�븿�닔 �룷�씤�듃 蹂��닔.
//callback_Reg g_CallHeartbeatRegRecv = NULL; ///< OMP�꽌踰꾨줈 遺��꽣 �닔�떊�븳 �뙣�궥 以� Heartbeat �뙣�궥�쓣 �젣怨듯븷 肄쒕갚�븿�닔 �룷�씤�듃 蹂��닔. (TCP Always On mode�뿉�꽌留� �궗�슜�맂�떎.)

void UnInitializaion()
{
	InitMemory();
	GMMP_Log_Close();
	CloseSocket();
}

int Initializaion(const char* pszServerIP, const int nPort, const char* pszDomainCode, const char* pszGWAuthID, const int nWriteLog, const int nErrorLevel, const int nNetwrokType, const char* pszLogFileName)
{
	if(pszDomainCode == NULL || pszGWAuthID == NULL)
	{
		return LIB_PARAM_ERROR;
	}

	if(pszLogFileName != NULL)
	{
		if( GMMP_Log_Init(pszLogFileName, true) != GMMP_SUCCESS)
		{
			return LIB_PARAM_ERROR;
		}
	}

	InitMemory();

	SetErrorLevel(nErrorLevel);

	if(SetServerInfo(pszServerIP, nPort, pszGWAuthID, pszDomainCode) != 0)
	{
		return LIB_PARAM_ERROR;
	}

	if(nWriteLog == GMMP_OFF_LOG)
	{
		SetbLog(false);
	}
	else
	{
		SetbLog(true);
	}


	if(nNetwrokType == GMMP_NETWORK_ALYWAYS_OFF)
	{
		SetNetworkType(GMMP_NETWORK_ALYWAYS_OFF);
	}
	else
	{
		SetNetworkType(GMMP_NETWORK_ALYWAYS_ON);
	}

	SetTID(0);

	return GMMP_SUCCESS;
}

void SetEncryption(int nEncryption)
{
	g_bIsEncryptionContinue = nEncryption;
}

int GetEncryption()
{
	return g_bIsEncryptionContinue;
}

void SetNetworkType(int nType)
{
	g_nNetworkMode = nType;
}

int GetNetworkType()
{
	return g_nNetworkMode;
}

void SetAuthKey(const char* pszAuthKey)
{
	memcpy(g_szAuthKey, pszAuthKey, strlen(pszAuthKey) );
}

void SetGWID(const char* pszGWID)
{
	if(pszGWID != NULL)
	{
		memcpy(g_szGWID, pszGWID, strlen(pszGWID) );
	}
}

char* GetGWID()
{
	return g_szGWID;
}

int SetServerInfo(const char* pszServerIP, int nPort, const char* pszAuthID, const char* pszDoamainCode)
{
	if(pszServerIP == NULL || strlen(pszServerIP) > LEN_IP || pszAuthID == NULL || strlen(pszAuthID) > LEN_AUTH_ID || pszDoamainCode == NULL || strlen(pszDoamainCode) > LEN_DOMAIN_CODE)
	{
		return LIB_PARAM_ERROR;
	}

	memcpy(g_szServerIP, pszServerIP, strlen(pszServerIP) );
	memcpy(g_szAuthID, pszAuthID, strlen(pszAuthID) );
	memcpy(g_szDomainCode, pszDoamainCode, strlen(pszDoamainCode) );

	g_nServerPort = nPort;

	int nRet = SetIntiSocket();
	if(nRet != GMMP_SUCCESS)
	{
		return nRet;
	}

	return GMMP_SUCCESS;
}

void SetCallFunction(int (* pCallFunctionName)(GMMPHeader* pstGMMPHeader, void* pstBody))
{
	g_CallFunctionRegRecv = (callback_Reg)pCallFunctionName;
}

int GO_Reg(const char* pszGWID, const char* pszManufactureID, int nEncryption)
{
	SetTID(GetTID()+1);

	g_bIsEncryptionContinue = nEncryption;

	int nRet = GMMP_SetReg(g_szAuthID, g_szAuthKey, g_szDomainCode, pszGWID, pszManufactureID);

	if( GetNetworkType() == GMMP_NETWORK_ALYWAYS_ON || nRet != GMMP_SUCCESS)
	{
		return nRet;
	}

	if(pszGWID == NULL)
	{
		GwRegist_Rsp stGwRegist_Rsp;
		memset(&stGwRegist_Rsp, 0 , sizeof(stGwRegist_Rsp) );

		nRet = GMMP_GetReg(&stGwRegist_Rsp, NULL);
		if(nRet != GMMP_SUCCESS)
		{
			return nRet;
		}

		nRet = OG_Reg_Recv(&stGwRegist_Rsp.header, &stGwRegist_Rsp.body, NULL);
		if(nRet == GMMP_SUCCESS && g_bIsEncryptionContinue > false)
		{
			return GO_EncInfo(g_szGWID, NULL);
		}
	}
	else
	{
		DeviceRegist_Rsp stDeviceRegist_Rsp;
		memset(&stDeviceRegist_Rsp, 0 , sizeof(stDeviceRegist_Rsp) );
		nRet = GMMP_GetReg(NULL, &stDeviceRegist_Rsp);
		if(nRet != GMMP_SUCCESS)
		{
			return nRet;
		}

		nRet = OG_Reg_Recv(&stDeviceRegist_Rsp.header, NULL, &stDeviceRegist_Rsp.body);

		if(nRet == GMMP_SUCCESS && g_bIsEncryptionContinue > false)
		{
			return GO_EncInfo(g_szGWID, (char*)stDeviceRegist_Rsp.body.usDeviceID);
		}
	}

	return nRet;
}

int GO_DeReg(const char* pszGWID, const char* pszDeviceID)
{
	SetTID(GetTID()+1);

	int nRet = GMMP_SetDeReg(g_szAuthID, g_szAuthKey, g_szDomainCode, pszGWID, pszDeviceID);
	if( GetNetworkType() == GMMP_NETWORK_ALYWAYS_ON || nRet != GMMP_SUCCESS)
	{
		return nRet;
	}

	if(pszDeviceID == NULL) //GW DeReg
	{
		GwDeRegist_Rsp stGwDeRegist_Rsp;
		memset(&stGwDeRegist_Rsp, 0 , sizeof(stGwDeRegist_Rsp) );

		nRet = GMMP_GetDeReg(&stGwDeRegist_Rsp, NULL);
		if(nRet != GMMP_SUCCESS)
		{
			return nRet;
		}

		nRet = OG_DeReg_Recv(&stGwDeRegist_Rsp.header, &stGwDeRegist_Rsp.body, NULL);
	}
	else
	{
		DeviceDeRegist_Rsp stDeviceDeRegist_Rsp;
		memset(&stDeviceDeRegist_Rsp, 0 , sizeof(stDeviceDeRegist_Rsp) );

		nRet = GMMP_GetDeReg(NULL, &stDeviceDeRegist_Rsp);
		if(nRet != GMMP_SUCCESS)
		{
			return nRet;
		}

		nRet = OG_DeReg_Recv(&stDeviceDeRegist_Rsp.header, NULL, &stDeviceDeRegist_Rsp.body);
	}

	return  nRet;
}

int GO_EncInfo(const char* pszGWID, const char* pszDeviceID)
{
	SetTID(GetTID()+1);

	int nRet = GMMP_SetEncryptInfo(g_szAuthID, g_szAuthKey, g_szDomainCode, pszGWID, pszDeviceID);
	if( GetNetworkType() == GMMP_NETWORK_ALYWAYS_ON || nRet != GMMP_SUCCESS)
	{
		return nRet;
	}

	EncryptInfo_Rsp stEncryptInfo_Rsp;
	memset(&stEncryptInfo_Rsp, 0 , sizeof(stEncryptInfo_Rsp) );

	nRet = GMMP_GetEncryptInfo(&stEncryptInfo_Rsp);
	if(nRet != GMMP_SUCCESS)
	{
		return nRet;
	}

	nRet = OG_Enc_Recv(&stEncryptInfo_Rsp.header, &stEncryptInfo_Rsp.body, NULL);

	return nRet;
}

int GO_EncKey(const char* pszGWID, const char* pszDeviceID, const char* pszEncryptKey)
{
	SetTID(GetTID()+1);

	int nRet = GMMP_SetEncryptKey(g_szAuthID, g_szAuthKey, g_szDomainCode, pszGWID, pszDeviceID, pszEncryptKey);
	if( GetNetworkType() == GMMP_NETWORK_ALYWAYS_ON || nRet != GMMP_SUCCESS)
	{
		return nRet;
	}

	EncryptKey_Rsp stEncryptKey_Rsp;
	memset(&stEncryptKey_Rsp, 0 , sizeof(stEncryptKey_Rsp) );

	nRet = GMMP_GetEncryptKey(&stEncryptKey_Rsp);
	if(nRet != GMMP_SUCCESS)
	{
		return nRet;
	}

	nRet = OG_Enc_Recv(&stEncryptKey_Rsp.header, NULL, &stEncryptKey_Rsp.body);

	return nRet;
}

int GO_Profile(const char* pszGWID, const char* pszDeviceID, const int nTID /*=0*/)
{
	if(nTID == 0)
	{
		SetTID(GetTID()+1);
	}
	else
	{
		SetTID(nTID);
	}

	int nRet = GMMP_SetProfile(g_szAuthID, g_szAuthKey, g_szDomainCode, pszGWID, pszDeviceID);
	if( GetNetworkType() == GMMP_NETWORK_ALYWAYS_ON || nRet != GMMP_SUCCESS)
	{
		return nRet;
	}

	Profile_Rsp stProfile_Rsp;
	memset(&stProfile_Rsp, 0 , sizeof(stProfile_Rsp) );

	nRet = GMMP_GetProfile(&stProfile_Rsp);
	if(nRet != GMMP_SUCCESS)
	{
		return nRet;
	}

	nRet = OG_Profile_Recv(&stProfile_Rsp.header, &stProfile_Rsp.body);

	return nRet;
}

int GO_Delivery(const char* pszGWID, const char* pszDeviceID, const char cReportType, const char cMediaType, const char* pszMessageBody, int nTotalCount, int nCurrentCount, const char cEncryption)
{
	SetTID(GetTID()+1);

	int nRet =	GMMP_SetDelivery(g_szAuthID, g_szAuthKey, g_szDomainCode, pszGWID, pszDeviceID, cReportType, cMediaType, pszMessageBody, nTotalCount, nCurrentCount, cEncryption);
	if( GetNetworkType() == GMMP_NETWORK_ALYWAYS_ON || nRet != GMMP_SUCCESS)
	{
		return nRet;
	}

	Delivery_Rsp stDelivery_Rsp;
	memset(&stDelivery_Rsp, 0 , sizeof(stDelivery_Rsp) );

	nRet = GMMP_GetDelivery(&stDelivery_Rsp);
	if(nRet != GMMP_SUCCESS)
	{
		return nRet;
	}

	nRet = OG_Delivery_Recv(&stDelivery_Rsp.header, &stDelivery_Rsp.body);

	return nRet;
}

int GO_Control(const char* pszGWID, const char* pszDeviceID, int nTID, const char cControlType, const char cResultCode)
{
	SetTID(nTID);

	return GMMP_SetControl(g_szAuthID, g_szAuthKey, g_szDomainCode, pszGWID, pszDeviceID, cControlType, cResultCode);
}

int GO_Notifi(const char* pszGWID, const char* pszDeviceID, const char cControlType, const char cResultCode, const char* pszMessageBody, const int nMessageBodySize)
{
	if( pszMessageBody != NULL && strlen(pszMessageBody) > MAX_MSG_BODY)
	{
		return LIB_PARAM_ERROR;
	}

	int nRet =	GMMP_SetNotifi(g_szAuthID, g_szAuthKey, g_szDomainCode, pszGWID, pszDeviceID, cControlType, cResultCode, pszMessageBody, nMessageBodySize);
	if( GetNetworkType() == GMMP_NETWORK_ALYWAYS_ON || nRet != GMMP_SUCCESS)
	{
		return nRet;
	}

	Notifi_Rsp stNotifi_Rsp;
	memset(&stNotifi_Rsp, 0 , sizeof(stNotifi_Rsp) );

	nRet = GMMP_GetNotifi(&stNotifi_Rsp);
	if(nRet != GMMP_SUCCESS)
	{
		return nRet;
	}

	nRet = OG_Notifi_Recv(&stNotifi_Rsp.header, &stNotifi_Rsp.body);

	return nRet;
}

int GO_HB(const char* pszGWID)
{
	int nRet =	GMMP_SetHB(g_szAuthID, g_szAuthKey, g_szDomainCode, pszGWID);
	if(GetNetworkType() == GMMP_NETWORK_ALYWAYS_ON || nRet != GMMP_SUCCESS)
	{
		return nRet;
	}

	HB_Rsp stHB_Rsp;
	memset(&stHB_Rsp, 0 , sizeof(stHB_Rsp) );

	nRet = GMMP_GetHB(&stHB_Rsp);
	if(nRet != GMMP_SUCCESS)
	{
		return nRet;
	}

	nRet = OG_HB_Recv(&stHB_Rsp.header, &stHB_Rsp.body);

	return nRet;
}

int GO_LSentence(const char* pszGWID, const char* pszDeviceID, const char cControlType)
{
	int nRet =	GMMP_SetLSentence(g_szAuthID, g_szAuthKey, g_szDomainCode, pszGWID, pszDeviceID, cControlType);
	if( GetNetworkType() == GMMP_NETWORK_ALYWAYS_ON || nRet != GMMP_SUCCESS)
	{
		return nRet;
	}

	LSentence_Rsp stLSentence_Rsp;

	nRet = GMMP_GetLSentence(&stLSentence_Rsp);
	if(nRet != GMMP_SUCCESS)
	{
		return nRet;
	}

	OG_LSentence_Recv(&stLSentence_Rsp.header, &stLSentence_Rsp.body);

	return nRet;
}

int GO_FTP(const char* pszGWID, const char* pszDeviceID, const char cControlType)
{
	int nRet =	GMMP_SetFTP(g_szAuthID, g_szAuthKey, g_szDomainCode, pszGWID, pszDeviceID, cControlType);
	if( GetNetworkType() == GMMP_NETWORK_ALYWAYS_ON || nRet != GMMP_SUCCESS)
	{
		return nRet;
	}

	FTP_Rsp stFTP_Rsp;
	memset(&stFTP_Rsp, 0 , sizeof(stFTP_Rsp) );

	nRet = GMMP_GetFTP(&stFTP_Rsp);
	if(nRet != GMMP_SUCCESS)
	{
		return nRet;
	}

	nRet = OG_FTP_Recv(&stFTP_Rsp.header, &stFTP_Rsp.body);

	return nRet;
}

int GO_Remote(const char* pszGWID, const char* pszDeviceID, const char cControlType)
{
	int nRet =	GMMP_SetRemote(g_szAuthID, g_szAuthKey, g_szDomainCode, pszGWID, pszDeviceID, cControlType);
	if( GetNetworkType() == GMMP_NETWORK_ALYWAYS_ON || nRet != GMMP_SUCCESS)
	{
		return nRet;
	}

	Remote_Rsp stRemote_Rsp;
	memset(&stRemote_Rsp, 0 , sizeof(stRemote_Rsp) );

	nRet = GMMP_GetRemote(&stRemote_Rsp);
	if(nRet != GMMP_SUCCESS)
	{
		return nRet;
	}

	nRet = OG_Remote_Recv(&stRemote_Rsp.header, &stRemote_Rsp.body);

	return nRet;
}

int GO_MM(const char* pszGWID, const char* pszDeviceID, const char cControlType)
{
	int nRet =	GMMP_SetMM(g_szAuthID, g_szAuthKey, g_szDomainCode, pszGWID, pszDeviceID, cControlType);
	if( GetNetworkType() == GMMP_NETWORK_ALYWAYS_ON || nRet != GMMP_SUCCESS)
	{
		return nRet;
	}

	MMURLInfo_Rsp stMMURLInfo_Rsp;
	memset(&stMMURLInfo_Rsp, 0 , sizeof(stMMURLInfo_Rsp) );

	nRet = GMMP_GetMM(&stMMURLInfo_Rsp);
	if(nRet == GMMP_SUCCESS)
	{
		return nRet;
	}

	nRet = OG_MM_Recv(&stMMURLInfo_Rsp.header, &stMMURLInfo_Rsp.body);

	return nRet;
}

int GO_LOB(const char* pszGWID, const char* pszDeviceID, const char  cLOBType)
{
	int nRet = GMMP_SUCCESS;

	SetTID(GetTID()+1);

	if(cLOBType == GMMP_LOB_CLOUD)
	{
		nRet = GMMP_SetLOB_Cloud(g_szAuthID, g_szAuthKey, g_szDomainCode, pszGWID, pszDeviceID);
		if( GetNetworkType() == GMMP_NETWORK_ALYWAYS_ON || nRet != GMMP_SUCCESS)
		{
			return nRet;
		}

		LOBCloud_Rsp stLOBCloud_Rsp;
		memset(&stLOBCloud_Rsp, 0 , sizeof(stLOBCloud_Rsp) );

		nRet = GMMP_GetLOB_Cloud(&stLOBCloud_Rsp);
		if(nRet != GMMP_SUCCESS)
		{
			return nRet;
		}

		nRet =OG_LOB_Recv(&stLOBCloud_Rsp.header, &stLOBCloud_Rsp.body, NULL);
	}
	else
	{
		nRet = GMMP_SetLOB_FTP(g_szAuthID, g_szAuthKey, g_szDomainCode, pszGWID, pszDeviceID);
		if( GetNetworkType() == GMMP_NETWORK_ALYWAYS_ON || nRet != GMMP_SUCCESS)
		{
			return nRet;
		}

		LOBFTP_Rsp stLOBFTP_Rsp;
		memset(&stLOBFTP_Rsp, 0 , sizeof(stLOBFTP_Rsp) );

		nRet = GMMP_GetLOB_FTP(&stLOBFTP_Rsp);
		if(nRet != GMMP_SUCCESS)
		{
			return nRet;
		}

		nRet = OG_LOB_Recv(&stLOBFTP_Rsp.header, NULL, &stLOBFTP_Rsp.body);
	}

	return  nRet;
}

int GO_LOB_Notifi(const char* pszGWID, const char* pszDeviceID, const char cStorageType, const char* pszLOBPath)
{
	SetTID(GetTID()+1);

	int nRet =	GMMP_SetLOB_Notifi(g_szAuthID, g_szAuthKey, g_szDomainCode, pszGWID, pszDeviceID, cStorageType, pszLOBPath);
	if( GetNetworkType() == GMMP_NETWORK_ALYWAYS_ON || nRet != GMMP_SUCCESS)
	{
		return nRet;
	}

	LOBNotifi_Rsp stLOBNotifi_Rsp;
	memset(&stLOBNotifi_Rsp, 0 , sizeof(stLOBNotifi_Rsp) );

	nRet = GMMP_GetLOB_Notifi(&stLOBNotifi_Rsp);
	if(nRet != GMMP_SUCCESS)
	{
		return nRet;
	}

	nRet = OG_LOB_Notifi_Recv(&stLOBNotifi_Rsp.header, &stLOBNotifi_Rsp.body);

	return nRet;
}


int OG_Reg_Recv(GMMPHeader* pstGMMPHeader, stGwRegistrationRspHdr* pstGWBody, stDeviceRegistrationRspHdr* pstDeviceBody)
{
	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_OPE, "\r\n");

	int nRet = 0;

	if(g_CallFunctionRegRecv != NULL)
	{
		if(pstGWBody != NULL)
		{
			nRet= (*g_CallFunctionRegRecv)(pstGMMPHeader, pstGWBody);

			if(nRet != GMMP_SUCCESS)
			{
				return nRet;
			}

			if(GetNetworkType() == GMMP_NETWORK_ALYWAYS_ON && g_bIsEncryptionContinue > false)
			{
				return GO_EncInfo(g_szGWID, NULL);
			}
		}
		else
		{
			nRet = (*g_CallFunctionRegRecv)(pstGMMPHeader, pstDeviceBody);

			if(nRet != GMMP_SUCCESS)
			{
				return nRet;
			}

			if(GetNetworkType() == GMMP_NETWORK_ALYWAYS_ON && g_bIsEncryptionContinue > false)
			{
				return GO_EncInfo(g_szGWID, (char*)pstDeviceBody->usDeviceID);
			}
		}
	}

	return GMMP_SUCCESS;
}

int OG_DeReg_Recv(GMMPHeader* pstGMMPHeader, stGwDeRegistrationRspHdr* pstGWBody, stDeviceDeRegistrationRspHdr* pstDeviceBody)
{
	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_OPE, "\r\n");

	if(g_CallFunctionRegRecv != NULL)
	{
		if(pstGWBody != NULL)
		{
			return (*g_CallFunctionRegRecv)(pstGMMPHeader, pstGWBody);
		}
		else
		{
			return (*g_CallFunctionRegRecv)(pstGMMPHeader, pstDeviceBody);
		}
	}

	return GMMP_SUCCESS;
}

int OG_Profile_Recv(GMMPHeader* pstGMMPHeader, stProfileRspHdr* pstBody)
{
	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_OPE, "\r\n");

	if(g_CallFunctionRegRecv != NULL)
	{
		return (*g_CallFunctionRegRecv)(pstGMMPHeader, pstBody);
	}

	return GMMP_SUCCESS;
}

int OG_Delivery_Recv(GMMPHeader* pstGMMPHeader, stPacketDeliveryRspHdr* pstBody)
{
	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_OPE, "\r\n");

	if(g_CallFunctionRegRecv != NULL)
	{
		return (*g_CallFunctionRegRecv)(pstGMMPHeader, pstBody);
	}

	return GMMP_SUCCESS;
}

int OG_Ctrl_Recv(GMMPHeader* pstGMMPHeader, stControlReqHdr* pstBody)
{
	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_REQ, "\r\n");

	if(g_CallFunctionRegRecv != NULL)
	{
		return (*g_CallFunctionRegRecv)(pstGMMPHeader, pstBody);
	}

	return GMMP_SUCCESS;
}

int OG_HB_Recv(GMMPHeader* pstGMMPHeader, stHeartBeatMsgRspHdr* pstBody)
{
	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_OPE, "\r\n");

	if(g_CallFunctionRegRecv != NULL)
	{
		return (*g_CallFunctionRegRecv)(pstGMMPHeader, pstBody);
	}

	return GMMP_SUCCESS;
}

int OG_LSentence_Recv(GMMPHeader* pstGMMPHeader, stLSentenceRspHdr* pstBody)
{
	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_OPE, "\r\n");

	if(g_CallFunctionRegRecv != NULL)
	{
		return (*g_CallFunctionRegRecv)(pstGMMPHeader, pstBody);
	}

	return GMMP_SUCCESS;
}

int OG_FTP_Recv(GMMPHeader* pstGMMPHeader, stFtpInfoRspHdr* pstBody)
{
	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_OPE, "\r\n");

	if(g_CallFunctionRegRecv != NULL)
	{
		return (*g_CallFunctionRegRecv)(pstGMMPHeader, pstBody);
	}

		return GMMP_SUCCESS;
}

int OG_Remote_Recv(GMMPHeader* pstGMMPHeader, stRemoteAccessInfoRspHdr* pstBody)
{
	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_OPE, "\r\n");

	if(g_CallFunctionRegRecv != NULL)
	{
		return (*g_CallFunctionRegRecv)(pstGMMPHeader, pstBody);
	}

	return GMMP_SUCCESS;
}

int OG_Notifi_Recv(GMMPHeader* pstGMMPHeader, stNotificationRspHdr* pstBody)
{
	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_OPE, "\r\n");

	if(g_CallFunctionRegRecv != NULL)
	{
		return (*g_CallFunctionRegRecv)(pstGMMPHeader, pstBody);
	}

	return GMMP_SUCCESS;
}

int OG_Enc_Recv(GMMPHeader* pstGMMPHeader, stEncryptionInfoRspHdr* pstInfoBody, stEncryptionKeyRspHdr* pstKeyBody)
{
	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_OPE, "\r\n");

	if(g_CallFunctionRegRecv != NULL)
	{
		if(pstInfoBody != NULL)
		{
			return (*g_CallFunctionRegRecv)(pstGMMPHeader, pstInfoBody);
		}
		else
		{
			return (*g_CallFunctionRegRecv)(pstGMMPHeader, pstKeyBody);
		}
	}

	return GMMP_SUCCESS;
}

int OG_MM_Recv(GMMPHeader* pstGMMPHeader, stMultimediaURLInfoRspHdr* pstBody)
{
	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_OPE, "\r\n");

	if(g_CallFunctionRegRecv != NULL)
	{
		return (*g_CallFunctionRegRecv)(pstGMMPHeader, pstBody);
	}

	return GMMP_SUCCESS;
}

int OG_LOB_Recv(GMMPHeader* pstGMMPHeader, stLOBCloudAccessInfoRspHdr* pstCloudBody, stLOBFTPAccessInfoRspHdr* pstFTPBody)
{
	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_OPE, "\r\n");

	if(g_CallFunctionRegRecv != NULL)
	{
		if(pstCloudBody != NULL)
		{
			return (*g_CallFunctionRegRecv)(pstGMMPHeader, pstCloudBody);
		}
		else
		{
			return (*g_CallFunctionRegRecv)(pstGMMPHeader, pstFTPBody);
		}
	}

	return GMMP_SUCCESS;
}

int OG_LOB_Notifi_Recv(GMMPHeader* pstGMMPHeader, stLOBUploadNotificationRspHdr* pstBody)
{
	GMMP_Printf(GMMP_ERROR_LEVEL_DEBUG, GMMP_LOG_MARKET_OPE, "\r\n");

	if(g_CallFunctionRegRecv != NULL)
	{
		return (*g_CallFunctionRegRecv)(pstGMMPHeader, pstBody);
	}

	return GMMP_SUCCESS;
}

int GetReadData(GMMPHeader* pstGMMPHeader, void** pBody)
{
	int nHeaderCound = 0;
	int nRet = 0;
	while(true)
	{
		nRet = GMMP_Read(pstGMMPHeader, pBody);
		if(nRet == GMMP_HEADER_SIZE_ERROR)
		{
			if(nHeaderCound > 3)
			{
				break;
			}
			nHeaderCound++;
			continue;
		}
		else if(nRet == GMMP_SUCCESS)
		{
			break;
		}
		else
		{
			return nRet;
		}
	}

	GMMP_Recv(pstGMMPHeader, *pBody);

	return nRet;
}

int Char2int(char* pBuffer, int nSize)
{
	if(nSize != sizeof(int) )
	{
		return 0;
	}

	int nInt = 0;
	memcpy(&nInt, pBuffer, sizeof(int) );
	return btoli(nInt);
}

int Char2short(char* pBuffer, short nSize)
{
	if(nSize != sizeof(short) )
	{
		return 0;
	}

	short nShort = 0;
	memcpy(&nShort, pBuffer, sizeof(short) );
	return btols(nShort);
}


void InitMemory()
{
	memset(g_szDomainCode, 0 , sizeof(g_szAuthID) );
	memset(g_szAuthID, 0 , sizeof(g_szAuthID) );
	memset(g_szAuthKey, 0 , sizeof(g_szAuthKey) );
	memset(g_szGWID, 0 , sizeof(g_szGWID) );

	memset(g_szGWMFID, 0 , sizeof(g_szGWMFID) );
	memset(g_szDeviceID, 0 , sizeof(g_szDeviceID) );

	g_nServerPort = 0;

	g_CallFunctionRegRecv = NULL;
}
