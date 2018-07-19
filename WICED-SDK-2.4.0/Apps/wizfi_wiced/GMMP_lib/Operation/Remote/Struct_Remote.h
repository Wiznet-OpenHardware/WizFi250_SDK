 /** GMMP Operation 중 Remote Inf 명령에 사용되는 구조체
  * @date 2013/11/21
 * @version 0.0.0.1
 * @file Struct_Remote.h
 **/

#ifndef STRUCT_REMOTE_H_
#define STRUCT_REMOTE_H_

#include "../../Define/Define.h"
#include "../Struct_Common.h"

#pragma pack(1)
/**
 * @struct stRemoteAccessInfoReqHdr
 * @brief GMMP TCP Packet  구조 중 Request 수행 시 Message Header와 Message Body 영역
  * OMP에 Remote 접속 정보 요청 명령을 전송 시 사용된다.
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
}stRemoteAccessInfoReqHdr; //Remote Access Info Request

/**
 * @struct stRemoteAccessInfoRspHdr
 * @brief GMMP TCP Packet  구조 중 Response 수행 시 Message Header와 Message Body 영역
 * OMP에서 Remote 접속 정보 요청 명령 응답 완료 수신 시 사용된다.
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
*@var usRemoteServerIP
*@brief 원격 터미널 서버의 IP 주소
*/
/**
*@var usRemoteServerPort
*@brief 원격 터미널 서버의 Port 번호
*/
/**
*@var usRemoteServerUserID
*@brief 원격 터미널 서버 접속 ID
*/
/**
*@var usRemoteServerUserPW
*@brief 원격 터미널 서버 접속 Password
*/
/**
*@var unAccessibleStartTime
*@brief 원격 터미널 접속 가능 시작 시각. Unix Time(=POSIX time, 1970년 1월1일부터 경과된 초)을 사용한다.
*/
/**
*@var unAccessibleEndTime
*@brief 원격 터미널 접속 가능 종료 시각. Unix Time(=POSIX time, 1970년 1월1일부터 경과된 초)을 사용한다.
*/
	U8  usDomainCode[LEN_DOMAIN_CODE];
	U8  usGWID[LEN_GW_ID];
	U8  usDeviceID[LEN_DEVICE_ID];
	U8  ucControlType;
	U8  ucResultCode;
	U8  usRemoteServerIP[16];
	U8  usRemoteServerPort[2];
	U8  usRemoteServerUserID[16];
	U8  usRemoteServerUserPW[16];
	U8  unAccessibleStartTime[4];
	U8  unAccessibleEndTime[4];
}stRemoteAccessInfoRspHdr; //Remote Access Info Response

/**
 * @struct Remote_Req
 * @brief GMMP TCP Packet  구조 중 Request Remote Info 구조체
 * OMP에 Request Remote Info 데이터 전송한다.
 */
typedef struct {
/**
*@var header
*@brief @ref GMMPHeader 참조
*/
/**
*@var body
*@brief @ref stRemoteAccessInfoReqHdr 참조
*/
	GMMPHeader					header;
	stRemoteAccessInfoReqHdr	body;
}Remote_Req;

/**
 * @struct Remote_Rsp
 * @brief GMMP TCP Packet  구조 중 Response Remote Info 구조체
  * OMP에 Response Remote Info 데이터 수신한다.
 */
typedef struct {
/**
*@var header
*@brief @ref GMMPHeader 참조
*/
/**
*@var body
*@brief @ref stRemoteAccessInfoRspHdr 참조
*/
	GMMPHeader					header;
	stRemoteAccessInfoRspHdr	body;
}Remote_Rsp;
#pragma pack(0)

#endif /* STRUCT_REMOTE_H_ */
