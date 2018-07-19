 /** GMMP Operation 중 FTP 정보 요청 명령에 사용되는 구조체
 * @file Struct_FTP.h
 * @date 2013/11/21
 * @version 0.0.0.1
 * @ref GMMPHeader참조
 **/


#ifndef STRUCT_FTP_H_
#define STRUCT_FTP_H_

#include "../../Define/Define.h"
#include "../Struct_Common.h"

#pragma pack(1)
/**
 * @struct stFtpInfoReqHdr
 * @brief GMMP TCP Packet  구조 중 Request 수행 시 Message Header와 Message Body 영역
 * OMP에 FTP 접속 정보 요청 명령을 전송 시 사용된다.
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
*@var ucControlType
*@brief 제어 명령 수신 메시지에 수신한 값(0x0d, 0x0e, 0x0f 또는 0x10)과 동일한 값을 사용한다. @ref Define_Control.h 참조
*@ref Define_Control.h 참조
*/
	U8  usDomainCode[LEN_DOMAIN_CODE];
	U8  usGWID[LEN_GW_ID];
	U8  usDeviceID[LEN_DEVICE_ID];
	U8  ucControlType;
}stFtpInfoReqHdr; //FTP Info Request

/**
 * @struct stFtpInfoRspHdr
 * @brief GMMP TCP Packet  구조 중 Response 수행 시 Message Header와 Message Body 영역
 * OMP에서 FTP 접속 정보 요청 명령 응답 완료 수신 시 사용된다.
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
*@var ucControlType
*@brief 제어 명령 수신 메시지에 수신한 값(0x0d, 0x0e, 0x0f 또는 0x10)과 동일한 값을 사용한다. @ref Define_Control.h 참조
*@ref Define_Control.h 참조
*/
/**
*@var ucResultCode
*@brief Result Code (0x00 : Success, other : Fail Reason) @ref OMPErrorCode.h 참조
*/

/**
*@var usFTPServerIP
*@brief Firmware 를 다운받기 위해 접속해야 할 FTP 서버 IP 주소.
*/
/**
*@var usFTPServerPort
*@brief Firmware 를 다운받기 위해 접속해야 할 FTP 서버 Port.
*/
/**
*@var usFTPServerID
*@brief Firmware 를 다운받기 위해 접속해야 할 FTP 서버 접속 ID.
*/
/**
*@var usFTPServerPW
*@brief Firmware 를 다운받기 위해 접속해야 할 FTP 서버 접속 Password.
*/
/**
*@var usFilePath
*@brief FTP 서버의 Firmware 파일이 위치하고 있는 디렉토리 경로.
*/
/**
*@var usFileName
*@brief FTP 서버의 Firmware 파일 이름.
*/
/**
*@var usFileHash
*@brief Firmware 파일을 검사하기 위한 Hash 값.
*/
/**
*@var uunFileSize
*@brief Firmware 파일 사이즈(단위 : byte).
*/
	U8  usDomainCode[LEN_DOMAIN_CODE];
	U8  usGWID[LEN_GW_ID];
	U8  usDeviceID[LEN_DEVICE_ID];
	U8  ucControlType;
	U8  ucResultCode;
	U8  usFTPServerIP[16];
	U8  usFTPServerPort[2];
	U8  usFTPServerID[16];
	U8  usFTPServerPW[16];
	U8  usFilePath[64];
	U8  usFileName[32];
	U8  usFileHash[32];
	U8  uunFileSize[4];
}stFtpInfoRspHdr; //FTP Info Response

/**
 * @struct FTP_Req
 * @brief GMMP TCP Packet  구조 중 Request FTP Info 구조체
 * OMP에 FTP 정보 요청 메시지를 전송 한다.
 */
typedef struct {
/**
*@var header
*@brief @ref GMMPHeader 참조
*/
/**
*@var body
*@brief @ref stFtpInfoReqHdr 참조
*/
	GMMPHeader		header;
	stFtpInfoReqHdr	body;
}FTP_Req;

/**
 * @struct FTP_Rsp
 * @brief GMMP TCP Packet  구조 중 Response FTP Info 구조체
 * OMP에 FTP 정보 메시지를 수신 한다.
 */
typedef struct {
/**
*@var header
*@brief @ref GMMPHeader 참조
*/
/**
*@var body
*@brief @ref stFtpInfoRspHdr 참조
*/
	GMMPHeader		header;
	stFtpInfoRspHdr	body;
}FTP_Rsp;

/**
 * @struct MultiApp_Req
 * @brief GMMP TCP Packet  구조 중 Request FTP Info 구조체
 * OMP에 FTP 정보 요청 메시지를 전송 한다.
 */
typedef struct {
/**
*@var header
*@brief @ref GMMPHeader 참조
*/
/**
*@var body
*@brief @ref stFtpInfoReqHdr 참조
*/
	GMMPHeader		header;
	stFtpInfoReqHdr	body;
}MultiApp_Req;

/**
 * @struct MultiApp_Rsp
 * @brief GMMP TCP Packet  구조 중 Response FTP Info 구조체
 * OMP에 FTP 정보 메시지를 수신 한다.
 */
typedef struct {
/**
*@var header
*@brief @ref GMMPHeader 참조
*/
/**
*@var body
*@brief @ref stFtpInfoRspHdr 참조
*/
	GMMPHeader		header;
	stFtpInfoRspHdr	body;
}MultiApp_Rsp;

#endif /* STRUCT_FTP_H_ */
