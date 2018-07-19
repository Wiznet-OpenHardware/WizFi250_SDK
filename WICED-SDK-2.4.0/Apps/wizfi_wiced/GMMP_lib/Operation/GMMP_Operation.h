/** GMMP 내부함수 및 변수 설명
  * @date 2013/11/21
 * @version 0.0.0.1
 * @file GMMP_Operation.h
 **/

#ifndef GMMP_OPERATION_H_
#define GMMP_OPERATION_H_

#include "../Log/GMMP_Log.h"
#include "../ErrorCode/StringTable.h"
#include "./Control/GMMP_Control.h"
#include "./Delivery/GMMP_Delivery.h"
#include "./Encrypt/GMMP_Encrypt.h"
#include "./FTP/GMMP_FTP.h"
#include "./Heartbeat/GMMP_Heartbeat.h"
#include "./LOB/GMMP_LOB.h"
#include "./LongSentence/GMMP_LSentence.h"
#include "./Multimedia/GMMP_Multimedia.h"
#include "./Notification/GMMP_Notification.h"
#include "./ProfileInfo/GMMP_ProfileInfo.h"
#include "./Reg/GMMP_Reg.h"
#include "./Remote/GMMP_Remote.h"
#include "../Network/Network.h"
#include "../GMMP.h"

/**
 * @brief GW/Device 등록/해지 Request 기능
 * @param pszAuthID Auth ID 문자열 입력
 * @param pszAuthKey Auth Key 문자열 입력
 * @param pszDomainCode Domain Code
 * @param pszGWID GW ID
 * @param pszManufactureID
 * @return
 */
int GMMP_SetReg(const char* pszAuthID,
		const char* pszAuthKey,
		const char* pszDomainCode,
		const char* pszGWID,
		const char* pszManufactureID);

/**
 *
 * @param pszAuthID
 * @param pszAuthKey
 * @param pszDomainCode
 * @param pszGWID
 * @param pszDeviceID
 * @return
 */
/**
 *
 * @param pszAuthID
 * @param pszAuthKey
 * @param pszDomainCode
 * @param pszGWID
 * @param pszDeviceID
 * @return
 */
/**
 *
 * @param pszAuthID
 * @param pszAuthKey
 * @param pszDomainCode
 * @param pszGWID
 * @param pszDeviceID
 * @return
 */
/**
 *
 * @param pszAuthID
 * @param pszAuthKey
 * @param pszDomainCode
 * @param pszGWID
 * @param pszDeviceID
 * @return
 */
int GMMP_SetDeReg(const char* pszAuthID,
		const char* pszAuthKey,
		const char* pszDomainCode,
		const char* pszGWID,
		const char* pszDeviceID);
/**
 *
 * @param pszAuthID
 * @param pszAuthKey
 * @param pszDomainCode
 * @param pszGWID
 * @param pszDeviceID
 * @return
 */
int GMMP_SetEncryptInfo(const char* pszAuthID,
		const char* pszAuthKey,
		const char* pszDomainCode,
		const char* pszGWID,
		const char* pszDeviceID);
/**
 *
 * @param pszAuthID
 * @param pszAuthKey
 * @param pszDomainCode
 * @param pszGWID
 * @param pszDeviceID
 * @param pszEncryptionKey
 * @return
 */
int GMMP_SetEncryptKey(const char* pszAuthID,
		const char* pszAuthKey,
		const char* pszDomainCode,
		const char* pszGWID,
		const char* pszDeviceID,
		const char* pszEncryptionKey);

/**
 *
 * @param pstGwRegist_Rsp
 * @param pstDeviceRegist_Rsp
 * @return
 */
int GMMP_GetReg(GwRegist_Rsp* pstGwRegist_Rsp, DeviceRegist_Rsp* pstDeviceRegist_Rsp);

/**
 *
 * @param pstGwRegist_Rsp
 * @param pstDeviceDeRegist_Rsp
 * @return
 */
int GMMP_GetDeReg(GwDeRegist_Rsp* pstGwRegist_Rsp, DeviceDeRegist_Rsp* pstDeviceDeRegist_Rsp);
/**
 *
 * @param pstEncryptInfo_Rsp
 * @return
 */
int GMMP_GetEncryptInfo(EncryptInfo_Rsp* pstEncryptInfo_Rsp);
/**
 *
 * @param pstEncryptKey_Rsp
 * @return
 */
int GMMP_GetEncryptKey(EncryptKey_Rsp* pstEncryptKey_Rsp);

/**
 *
 * @param pszAuthID
 * @param pszAuthKey
 * @param pszDomainCode
 * @param pszGWID
 * @param pszDeviceID
 * @return
 */
int GMMP_SetProfile(const char* pszAuthID,
		const char* pszAuthKey,
		const char* pszDomainCode,
		const char* pszGWID,
		const char* pszDeviceID);
/**
 *
 * @param pstProfile_Rsp
 * @return
 */
int GMMP_GetProfile(Profile_Rsp* pstProfile_Rsp);
/**
 *
 * @param pszAuthID
 * @param pszAuthKey
 * @param pszDomainCode
 * @param pszGWID
 * @param pszDeviceID
 * @param cReportType
 * @param cMediaType
 * @param pszMessageBody
 * @param nTotalCount
 * @param nCurrentCount
 * @param cEncryption
 * @return
 */
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
		const char cEncryption);
/**
 *
 * @param pstDelivery_Rsp
 * @return
 */
int GMMP_GetDelivery(Delivery_Rsp* pstDelivery_Rsp);

/**
 *
 * @param pszAuthID
 * @param pszAuthKey
 * @param pszDomainCode
 * @param pszGWID
 * @param pszDeviceID
 * @param cControlType
 * @param cResultCode
 * @return
 */
int GMMP_SetControl(const char* pszAuthID,
		const char* pszAuthKey,
		const char* pszDomainCode,
		const char* pszGWID,
		const char* pszDeviceID,
		const char cControlType,
		const char cResultCode);

/**
 *
 * @param pszAuthID
 * @param pszAuthKey
 * @param pszDomainCode
 * @param pszGWID
 * @param pszDeviceID
 * @param cControlType
 * @param cResultCode
 * @param pszMessageBody
 * @param nMessageSize
 * @return
 */
int GMMP_SetNotifi(const char* pszAuthID,
		const char* pszAuthKey,
		const char* pszDomainCode,
		const char* pszGWID,
		const char* pszDeviceID,
		const char cControlType,
		const char cResultCode,
		const char* pszMessageBody,
		const int nMessageSize);

/**
 *
 * @param pstNotifi_Rsp
 * @return
 */
int GMMP_GetNotifi(Notifi_Rsp* pstNotifi_Rsp);

/**
 *
 * @param pszAuthID
 * @param pszAuthKey
 * @param pszDomainCode
 * @param pszGWID
 * @return
 */
int GMMP_SetHB(const char* pszAuthID,
		const char* pszAuthKey,
		const char* pszDomainCode,
		const char* pszGWID);

/**
 *
 * @param pstHB_Rsp
 * @return
 */
int GMMP_GetHB(HB_Rsp* pstHB_Rsp);

/**
 *
 * @param pszAuthID
 * @param pszAuthKey
 * @param pszDomainCode
 * @param pszGWID
 * @param pszDeviceID
 * @param cControlType
 * @return
 */
int GMMP_SetLSentence(const char* pszAuthID,
		const char* pszAuthKey,
		const char* pszDomainCode,
		const char* pszGWID,
		const char* pszDeviceID,
		const char cControlType);

/**
 *
 * @param pstLSentence_Rsp
 * @return
 */
int GMMP_GetLSentence(LSentence_Rsp* pstLSentence_Rsp);

/**
 *
 * @param pszAuthID
 * @param pszAuthKey
 * @param pszDomainCode
 * @param pszGWID
 * @param pszDeviceID
 * @param cControlType
 * @return
 */
int GMMP_SetFTP(const char* pszAuthID,
		const char* pszAuthKey,
		const char* pszDomainCode,
		const char* pszGWID,
		const char* pszDeviceID,
		const char cControlType);

/**
 *
 * @param pstFTP_Rsp
 * @return
 */
int GMMP_GetFTP(FTP_Rsp* pstFTP_Rsp);

/**
 *
 * @param pszAuthID
 * @param pszAuthKey
 * @param pszDomainCode
 * @param pszGWID
 * @param pszDeviceID
 * @param cControlType
 * @return
 */
int GMMP_SetRemote(const char* pszAuthID,
		const char* pszAuthKey,
		const char* pszDomainCode,
		const char* pszGWID,
		const char* pszDeviceID,
		const char cControlType);

/**
 *
 * @param pstRemote_Rsp
 * @return
 */
int GMMP_GetRemote(Remote_Rsp* pstRemote_Rsp);

/**
 *
 * @param pszAuthID
 * @param pszAuthKey
 * @param pszDomainCode
 * @param pszGWID
 * @param pszDeviceID
 * @param cControlType
 * @return
 */
int GMMP_SetMM(const char* pszAuthID,
		const char* pszAuthKey,
		const char* pszDomainCode,
		const char* pszGWID,
		const char* pszDeviceID,
		const char cControlType);

/**
 *
 * @param pstMMURLInfo_Rsp
 * @return
 */
int GMMP_GetMM(MMURLInfo_Rsp* pstMMURLInfo_Rsp);

/**
 *
 * @param pszAuthID
 * @param pszAuthKey
 * @param pszDomainCode
 * @param pszGWID
 * @param pszDeviceID
 * @return
 */
int GMMP_SetLOB_Cloud(const char* pszAuthID,
		const char* pszAuthKey,
		const char* pszDomainCode,
		const char* pszGWID,
		const char* pszDeviceID);

/**
 *
 * @param pstLOBCloud_Rsp
 * @return
 */
int GMMP_GetLOB_Cloud(LOBCloud_Rsp* pstLOBCloud_Rsp);

/**
 *
 * @param pszAuthID
 * @param pszAuthKey
 * @param pszDomainCode
 * @param pszGWID
 * @param pszDeviceID
 * @return
 */
int GMMP_SetLOB_FTP(const char* pszAuthID,
		const char* pszAuthKey,
		const char* pszDomainCode,
		const char* pszGWID,
		const char* pszDeviceID);

/**
 *
 * @param pstLOBFTP_Rsp
 * @return
 */
int GMMP_GetLOB_FTP(LOBFTP_Rsp* pstLOBFTP_Rsp);
/**
 *
 * @param pszAuthID
 * @param pszAuthKey
 * @param pszDomainCode
 * @param pszGWID
 * @param pszDeviceID
 * @param cStorageType
 * @param pszLOBPath
 * @return
 */
int GMMP_SetLOB_Notifi(const char* pszAuthID,
		const char* pszAuthKey,
		const char* pszDomainCode,
		const char* pszGWID,
		const char* pszDeviceID,
		const char cStorageType,
		const char* pszLOBPath);
/**
 *
 * @param pstLOBNotifi_Rsp
 * @return
 */
int GMMP_GetLOB_Notifi(LOBNotifi_Rsp* pstLOBNotifi_Rsp);

/**
 *
 * @param pstGMMPHeader
 * @param pBody
 * @return
 */
int GMMP_Read(GMMPHeader* pstGMMPHeader, void** pBody);
/**
 *
 * @param pstGMMPHeader
 * @param pBody
 * @return
 */
int GMMP_Recv(GMMPHeader* pstGMMPHeader, void* pBody);
/**
 *
 * @param pstGMMPHeader
 * @param pstRspHdr
 * @return
 */
int GMMP_Ctrl_Recv(GMMPHeader* pstGMMPHeader,  stControlReqHdr* pstRspHdr);
/**
 *
 * @param pstGMMPHeader
 * @param pBody
 * @return
 */
int GMMP_Trace(GMMPHeader* pstGMMPHeader, void* pBody);

/**
 *
 * @param nTid
 */
void SetTID(int nTid);
/**
 *
 * @return
 */
int GetTID();

/**
 *
 * @param pData
  * @param nPacketSize
 * @param nTotalCount
 * @param nCurrentCount
 * @param cMessageType
 * @param pszAuthID
 * @param pszAuthKey
 * @param cEncryption
 * @return
 */
int SetHeader(void* pData,
		int nPacketSize,
		int nTotalCount,
		int nCurrentCount,
		const char cMessageType,
		const char* pszAuthID,
		const char* pszAuthKey,
		const char cEncryption);

/**
 *
 * @return
 */
int SetIntiSocket();

/**
 *
 * @param Type
 * @param nOutBufferSize
 * @return
 */
char* MallocBody(const char Type, int* nOutBufferSize);

/**
 *
 * @return
 */
int GetTime();

/**
 *
 * @param bLog
 */
void	SetbLog(int bLog);
/**
 *
 * @return
 */
int GetbLog();
/**
 *
 * @param nErrorLevle
 */
void SetErrorLevel(int nErrorLevle);
/**
 *
 * @return
 */
int GetErrorLevel();


/**
 *
 * @param nErrorLevel
 * @param nLogMakerLevel
 * @param __format
 * @return
 */
int GMMP_Printf(const unsigned int nErrorLevel, const unsigned int nLogMakerLevel, char *__restrict __format, ...);

/**
 *
 * @param nErrorLevel
 * @param pMessage
 * @return
 */
int GMMP_PrintfDebug(const unsigned int nErrorLevel, char* pMessage);

/**
 *
 * @param nLogMakerLevel
 * @param pMessage
 * @return
 */
int GMMP_PrintfLog(const unsigned int nLogMakerLevel, char* pMessage);

/*
int GMMP_PrintfDebug(const unsigned int nErrorLevel, __const char *__restrict __format, ...);
int GMMP_Printf(const unsigned int nErrorLevel, const unsigned int nLogMakerLevel, __const char *__restrict __format, ...);
int GMMP_LogPrintf(const unsigned int nLogMakerLevel, __const char *__restrict __format, ...);
*/
#endif /* GMMP_OPERATION_H_ */
