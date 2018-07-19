/** GMMP Operation 중 LOB 명령에 사용되는 구조체
* @file Struct_LOB.h
* @date 2013/11/21
* @version 0.0.0.1
* @ref GMMPHeader참조
**/

#ifndef STRUCT_LOB_H_
#define STRUCT_LOB_H_

#include "../../Define/Define.h"
#include "../Struct_Common.h"

#pragma pack(1)
/**
 * @struct stLOBCloudAccessInfoReqHdr
 * @brief GMMP TCP Packet  구조 중 Request 수행 시 Message Header와 Message Body 영역
 * OMP에 LOB 명령 중 Cloud 접속 정보 요청 명령을 전송 시 사용된다.
 */
typedef struct {
/**
*@var usDomainCode
*@brief 서비스 영역별 구분 코드\n
- AlphaNumeric : 문자,숫자 조합, 특수문자제외, SPACE 허용 안하며 빈자리는 0x00(NULL)로 채운다.\n
*/
/**
*@var usGWID
*@brief M2M GW ID
*/
/**
*@var usDeviceID
*@brief M2M Device ID, 값이 0x0이면 GW 명령이다.
*/
	U8  usDomainCode[LEN_DOMAIN_CODE];
	U8  usGWID[LEN_GW_ID];
	U8  usDeviceID[LEN_DEVICE_ID];
}stLOBCloudAccessInfoReqHdr; //LOB Cloud Info Request

/**
 * @struct stLOBCloudAccessInfoRspHdr
 * @brief GMMP TCP Packet  구조 중 Response 수행 시 Message Header와 Message Body 영역
 * OMP에 LOB 명령 중 Cloud 접속 정보 제공 명령을 수신 시 사용된다.
 */
typedef struct {
/**
*@var usDomainCode
*@brief 서비스 영역별 구분 코드\n
- AlphaNumeric : 문자,숫자 조합, 특수문자제외, SPACE 허용 안하며 빈자리는 0x00(NULL)로 채운다.\n
*/
/**
*@var usGWID
*@brief M2M GW ID
*/
/**
*@var usDeviceID
*@brief M2M Device ID, 값이 0x0이면 GW 명령이다.
*/
/**
*@var ucResultCode
*@brief Result Code (0x00 : Success, other : Fail Reason) @ref OMPErrorCode.h 참조
*/
/**
*@var usAccessKey
*@brief LOB Upload Cloud 서버 접속 Access Key 값.
*/
/**
*@var usSecretKey
*@brief LOB Upload Cloud 서버 접속 Secret Key 값.
*/
/**
*@var usDirectoryPath
*@brief LOB Upload Cloud 서버의 Upload 디렉토리 경로.
*/
	U8  usDomainCode[LEN_DOMAIN_CODE];
	U8  usGWID[LEN_GW_ID];
	U8  usDeviceID[LEN_DEVICE_ID];
	U8  ucResultCode;
	U8  usAccessKey[LEN_LOB_ACCESS_KEY];
	U8  usSecretKey[LEN_LOB_SECRET_KEY];
	U8  usDirectoryPath[LEN_LOB_DIRECTORY_PATH];
}stLOBCloudAccessInfoRspHdr; //LOB Cloud Info Response

/**
 * @struct LOBCloud_Req
 * @brief GMMP TCP Packet  구조 중 Request LOB Cloud Access Info 구조체
  * OMP에 LOB 명령 중 Cloud 접속 정보 요청 명령을 전송한다.
 */
typedef struct {
/**
*@var header
*@brief @ref GMMPHeader 참조
*/
/**
*@var body
*@brief @ref stLOBCloudAccessInfoReqHdr 참조
*/
	GMMPHeader					header;
	stLOBCloudAccessInfoReqHdr	body;
}LOBCloud_Req;

/**
 * @struct LOBCloud_Rsp
 * @brief GMMP TCP Packet  구조 중 Response LOB Cloud Access Info 구조체
 * OMP에 LOB 명령 중 Cloud 접속 정보 제공 명령을 수신한다.
 */
typedef struct {
/**
*@var header
*@brief @ref GMMPHeader 참조
*/
/**
*@var body
*@brief @ref stLOBCloudAccessInfoRspHdr 참조
*/
	GMMPHeader					header;
	stLOBCloudAccessInfoRspHdr	body;
}LOBCloud_Rsp;

/**
 * @struct stLOBFTPAccessInfoReqHdr
 * @brief GMMP TCP Packet  구조 중 Request 수행 시 Message Header와 Message Body 영역
 * OMP에 LOB 명령 중 FTP 접속 정보 요청 명령을 전송 시 사용된다.
 */

typedef struct {
/**
*@var usDomainCode
*@brief 서비스 영역별 구분 코드\n
- AlphaNumeric : 문자,숫자 조합, 특수문자제외, SPACE 허용 안하며 빈자리는 0x00(NULL)로 채운다.\n
*/
/**
*@var usGWID
*@brief M2M GW ID
*/
/**
*@var usDeviceID
*@brief M2M Device ID, 값이 0x0이면 GW 명령이다.
*/
	U8  usDomainCode[LEN_DOMAIN_CODE];
	U8  usGWID[LEN_GW_ID];
	U8  usDeviceID[LEN_DEVICE_ID];
}stLOBFTPAccessInfoReqHdr; //LOB FTP Info Request

/**
 * @struct stLOBFTPAccessInfoRspHdr
 * @brief GMMP TCP Packet  구조 중 Response 수행 시 Message Header와 Message Body 영역
 * OMP에 LOB 명령 중 FTP 접속 정보 제공 명령을 수신 시 사용된다.
 */
typedef struct {
/**
*@var usDomainCode
*@brief 서비스 영역별 구분 코드\n
- AlphaNumeric : 문자,숫자 조합, 특수문자제외, SPACE 허용 안하며 빈자리는 0x00(NULL)로 채운다.\n
*/
/**
*@var usGWID
*@brief M2M GW ID
*/
/**
*@var usDeviceID
*@brief M2M Device ID, 값이 0x0이면 GW 명령이다.
*/
/**
*@var ucResultCode
*@brief Result Code (0x00 : Success, other : Fail Reason) @ref OMPErrorCode.h 참조
*/
/**
*@var usFTPServerIP
*@brief LOB Upload FTP 서버 IP 주소
*/
/**
*@var usFTPServerPort
*@brief LOB Upload FTP 서버 Port
*/
/**
*@var usFTPServerID
*@brief LOB Upload FTP 서버 접속 ID
*/
/**
*@var usFTPServerPW
*@brief LOB Upload FTP 서버 접속 Password
*/
/**
*@var usFilePath
*@brief LOB Upload 디렉토리 경로
*/
	U8  usDomainCode[LEN_DOMAIN_CODE];
	U8  usGWID[LEN_GW_ID];
	U8  usDeviceID[LEN_DEVICE_ID];
	U8  ucResultCode;
	U8  usFTPServerIP[16];
	U8  usFTPServerPort[2];
	U8  usFTPServerID[16];
	U8  usFTPServerPW[16];
	U8  usFilePath[64];
}stLOBFTPAccessInfoRspHdr; //LOB FTP Info Response

/**
 * @struct LOBFTP_Req
 * @brief GMMP TCP Packet  구조 중 Request LOB FTP Access Info 구조체
  * OMP에 LOB 명령 중 FTP 접속 정보 요청 명령을 전송한다.
 */
typedef struct {
/**
*@var header
*@brief @ref GMMPHeader 참조
*/
/**
*@var body
*@brief @ref stLOBFTPAccessInfoReqHdr 참조
*/
	GMMPHeader					header;
	stLOBFTPAccessInfoReqHdr	body;
}LOBFTP_Req;

/**
 * @struct LOBFTP_Rsp
 * @brief GMMP TCP Packet  구조 중 Response LOB FTP Access Info 구조체
 * OMP에 LOB 명령 중 FTP 접속 정보 제공 명령을 수신한다.
 */
typedef struct {
/**
*@var header
*@brief @ref GMMPHeader 참조
*/
/**
*@var body
*@brief @ref stLOBFTPAccessInfoRspHdr 참조
*/
	GMMPHeader					header;
	stLOBFTPAccessInfoRspHdr	body;
}LOBFTP_Rsp;

/**
 * @struct stLOBUploadNotificationReqHdr
 * @brief GMMP TCP Packet  구조 중 Request 수행 시 Message Header와 Message Body 영역
 * OMP에 LOB 명령 수행 완료 및 Upload 정보 보고 명령을 전송 시 사용된다.
 */
typedef struct {
/**
*@var usDomainCode
*@brief 서비스 영역별 구분 코드\n
- AlphaNumeric : 문자,숫자 조합, 특수문자제외, SPACE 허용 안하며 빈자리는 0x00(NULL)로 채운다.\n
*/
/**
*@var usGWID
*@brief M2M GW ID
*/
/**
*@var usDeviceID
*@brief M2M Device ID, 값이 0x0이면 GW 명령이다.
*/
/**
*@var ucStorageType
*@brief LOB Storage 형태\n
0x01 : Cloud\n
0x02 : FTP
*/
/**
*@var usLOBPath
*@brief LOB Upload 디렉토리 경로
*/
	U8  usDomainCode[LEN_DOMAIN_CODE];
	U8  usGWID[LEN_GW_ID];
	U8  usDeviceID[LEN_DEVICE_ID];
	U8  ucStorageType;
	U8  usLOBPath[LEN_LOB_PATH];
}stLOBUploadNotificationReqHdr; //LOB Upload Notification Request

/**
 * @struct stLOBUploadNotificationRspHdr
 * @brief GMMP TCP Packet  구조 중 Response 수행 시 Message Header와 Message Body 영역
 * OMP에 LOB 명령 수행 완료 및 Upload 정보 보고 응답 명령을 수신 시 사용된다.
 */
typedef struct {
/**
*@var usDomainCode
*@brief 서비스 영역별 구분 코드\n
- AlphaNumeric : 문자,숫자 조합, 특수문자제외, SPACE 허용 안하며 빈자리는 0x00(NULL)로 채운다.\n
*/
/**
*@var usGWID
*@brief M2M GW ID
*/
/**
*@var usDeviceID
*@brief M2M Device ID, 값이 0x0이면 GW 명령이다.
*/
/**
*@var ucResultCode
*@brief Result Code (0x00 : Success, other : Fail Reason) @ref OMPErrorCode.h 참조
*/
	U8  usDomainCode[LEN_DOMAIN_CODE];
	U8  usGWID[LEN_GW_ID];
	U8  usDeviceID[LEN_DEVICE_ID];
	U8  ucResultCode;
}stLOBUploadNotificationRspHdr; //LOB Upload Notification Response

/**
 * @struct LOBNotifi_Req
 * @brief GMMP TCP Packet  구조 중 Request LOG Upload Notification 구조체
  * OMP에 LOB 명령 수행 완료 및 Upload 정보 보고 명령을 전송한다.
 */
typedef struct {
/**
*@var header
*@brief @ref GMMPHeader 참조
*/
/**
*@var body
*@brief @ref stLOBUploadNotificationReqHdr 참조
*/
	GMMPHeader					header;
	stLOBUploadNotificationReqHdr	body;
}LOBNotifi_Req;

/**
 * @struct LOBNotifi_Rsp
 * @brief GMMP TCP Packet  구조 중 Request LOG Upload Notification 구조체
   * OMP에 LOB 명령 수행 완료 및 Upload 정보 보고 응답 명령을 수신한다.
 */
typedef struct {
/**
*@var header
*@brief @ref GMMPHeader 참조
*/
/**
*@var body
*@brief @ref stLOBUploadNotificationRspHdr 참조
*/
	GMMPHeader					header;
	stLOBUploadNotificationRspHdr	body;
}LOBNotifi_Rsp;
#pragma pack(0)

#endif /* STRUCT_LOB_H_ */
