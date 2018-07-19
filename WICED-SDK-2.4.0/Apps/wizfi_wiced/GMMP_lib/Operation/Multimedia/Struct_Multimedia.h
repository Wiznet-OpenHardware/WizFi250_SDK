/** GMMP Operation 중 Multimedia URL Info 명령에 사용되는 구조체
* @file Struct_Multimedia.h
* @date 2013/11/21
* @version 0.0.0.1
* @ref GMMPHeader참조
**/

#ifndef STRUCT_MULTIMEDIA_H_
#define STRUCT_MULTIMEDIA_H_

#include "../../Define/Define.h"
#include "../Struct_Common.h"

#pragma pack(1)
/**
 * @struct stMultimediaURLInfoReqHdr
 * @brief GMMP TCP Packet  구조 중 Request 수행 시 Message Header와 Message Body 영역
 * OMP에 Multimedia URL Info 명령을 전송 시 사용된다.
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
*@brief 제어 명령 수신 메시지에 수신한 값(0x12)과 동일한 값을 사용한다 @ref Define_Control.h 참조
*@ref Define_Control.h 참조
*/
	U8  usDomainCode[LEN_DOMAIN_CODE];
	U8  usGWID[LEN_GW_ID];
	U8  usDeviceID[LEN_DEVICE_ID];
	U8  ucControlType;
}stMultimediaURLInfoReqHdr; //Encryption Info Request

/**
 * @struct stMultimediaURLInfoRspHdr
 * @brief GMMP TCP Packet  구조 중 Response 수행 시 Message Header와 Message Body 영역
 * OMP에서 Multimedia URL Info 제공 명령 수신 시 사용된다.
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
*@brief 제어 명령 수신 메시지에 수신한 값(0x12)과 동일한 값을 사용한다 @ref Define_Control.h 참조
*@ref Define_Control.h 참조
*/
/**
*@var ucResultCode
*@brief Result Code (0x00 : Success, other : Fail Reason) @ref OMPErrorCode.h 참조
*/
/**
*@var usServerURL
*@brief 멀티미디어 서버 URL 정보.
*/
/**
*@var usServerID
*@brief 멀티미디어 서버 접속 ID.
*/
/**
*@var usServerPassword
*@brief 멀티미디어 서버 접속 Password.
*/
	U8  usDomainCode[LEN_DOMAIN_CODE];
	U8  usGWID[LEN_GW_ID];
	U8  usDeviceID[LEN_DEVICE_ID];
	U8  ucControlType;
	U8  ucResultCode;
	U8  usServerURL[LEN_MULTIMEDIA_URL];
	U8  usServerID[LEN_MULTIMEDIA_ID];
	U8  usServerPassword[LEN_MULTIMEDIA_PW];
}stMultimediaURLInfoRspHdr; ///Encryption Info Response

/**
 * @struct MMURLInfo_Req
 * @brief GMMP TCP Packet  구조 중 Request Multimedia URL Info 구조체
 * OMP에 Request Multimedia URL Info 데이터 전송한다.
 */
typedef struct {
/**
*@var header
*@brief @ref GMMPHeader 참조
*/
/**
*@var body
*@brief @ref stMultimediaURLInfoReqHdr 참조
*/
	GMMPHeader					header;
	stMultimediaURLInfoReqHdr	body;
}MMURLInfo_Req;

/**
 * @struct MMURLInfo_Rsp
 * @brief GMMP TCP Packet  구조 중 Response Multimedia URL Info 구조체
  * OMP에 Response Multimedia URL Info 데이터 수신한다.
 */
typedef struct {
/**
*@var header
*@brief @ref GMMPHeader 참조
*/
/**
*@var body
*@brief @ref stMultimediaURLInfoRspHdr 참조
*/
	GMMPHeader					header;
	stMultimediaURLInfoRspHdr	body;
}MMURLInfo_Rsp;
#pragma pack(0)
#endif /* STRUCT_MULTIMEDIA_H_ */
