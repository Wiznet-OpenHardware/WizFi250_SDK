 /** GMMP Operation 중 제어 명령에 사용되는 구조체
 * @file Struct_Control.h
 * @date 2013/11/21
 * @version 0.0.0.1
 * @ref GMMPHeader참조
 **/

#ifndef STRUCT_CONTROL_H_
#define STRUCT_CONTROL_H_

#include "../../Define/Define.h"
#include "../Struct_Common.h"
#include "Struct_Control_Opt.h"

#pragma pack(1)

/**
 * @struct stControlReqHdr
 * @brief GMMP TCP Packet  구조 중 Request 수행 시 Message Header와 Message Body 영역
 * OMP에서 제어 명령 메시지를 전송하여 GMMP Lib에서 해당 메시지를 수신한다.
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
*@brief 제어 명령 구분자
*@ref Define_Control.h 참조
*/
/**
*@var usMessageBody
*@brief Data [2048byte]
*@ref Struct_Control_Opt.h 참조
*/

	U8  usDomainCode[LEN_DOMAIN_CODE];
	U8  usGWID[LEN_GW_ID];
	U8  usDeviceID[LEN_DEVICE_ID];
	U8  ucControlType;
	U8  usMessageBody[MAX_MSG_BODY];
}stControlReqHdr; //Control Request

/**
* @struct stControlRspHdr
* @brief GMMP TCP Packet  구조 중 Response 수행 시 Message Header와 Message Body 영역
* OMP에서 전송한 제어 메시지에 대한 응답을 전송한다.
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
*@brief 제어 명령 구분자\n
*@ref Define_Control.h 참조
*/
/**
*@var ucResultCode
*@brief Result Code (0x00 : Success, other : Fail Reason) @ref OMPErrorCode.h 참조
*/
	U8  usDomainCode[LEN_DOMAIN_CODE];
	U8  usGWID[LEN_GW_ID];
	U8  usDeviceID[LEN_DEVICE_ID];
	U8  ucControlType;
	U8  ucResultCode;
}stControlRspHdr; //Control Response


/**
* @struct Control_Req
* @brief GMMP TCP Packet  구조 중 Request Control 제어 구조
*  OMP에서 메시지를 전송하여 GMMP Lib에서 해당 메시지를 수신한다.
*/
typedef struct {
/**
*@var header
*@brief
*@ref GMMPHeader 참조
*/
/**
*@var body
*@brief @ref stControlReqHdr 참조
*/
	GMMPHeader			header;
	stControlReqHdr	body;
}Control_Req;

/**
 * @struct Control_Rsp
 * @brief GMMP TCP Packet  구조 중 Response Control 제어 구조
 * OMP에서 전송한 제어 메시지에 대한 응답을 전송한다.
 */
typedef struct {
/**
*@var header
*@brief @ref GMMPHeader 참조
*/
/**
*@var body
*@brief @ref stControlRspHdr 참조
*/
	GMMPHeader			header;
	stControlRspHdr	body;
}Control_Rsp;
#pragma pack(0)

#endif /* STRUCT_CONTROL_H_ */
