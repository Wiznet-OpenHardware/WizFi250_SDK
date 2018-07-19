  /** GMMP Operation 중 주기보고 명령에 사용되는 구조체
 * @file Struct_Delivery.h
 * @date 2013/11/21
 * @version 0.0.0.1
 * @ref GMMPHeader참조
 **/

#ifndef STRUCT_DELIVERY_H_
#define STRUCT_DELIVERY_H_


#include "../../Define/Define.h"
#include "../Struct_Common.h"

#pragma pack(1)

/**
 * @struct stPacketDeliveryReqHdr
 * @brief GMMP TCP Packet  구조 중 Request 수행 시 Message Header와 Message Body 영역
 * OMP에 주기 보고 명령을 전송 시 사용된다.
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
*@var ucReportType
*@brief 주기 보고 명령 구분자
*@ref Define_Delivery.h 참조
*/
/**
*@var ucMediaType
*@brief 주기 보고 하는 데이터의 타입\n
*/
/**
*@var usMessageBody
*@brief Data [2048byte]
*/
	U8  usDomainCode[LEN_DOMAIN_CODE];
	U8  usGWID[LEN_GW_ID];
	U8  usDeviceID[LEN_DEVICE_ID];
	U8  ucReportType;
	U8  ucMediaType;
	U8  usMessageBody[MAX_MSG_BODY];
}stPacketDeliveryReqHdr; //Delivery Request

/**
 * @struct stPacketDeliveryRspHdr
 * @brief GMMP TCP Packet  구조 중 Response 수행 시 Message Header와 Message Body 영역
 * OMP에서 주기 보고 명령 응답 완료 수신 시 사용된다.
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
*@var unBackOffTime
*@brief Result Code가 STATUS_NOT_ACCEPTABLE(OMP 과부하) 인 경우 해당 시간 값 만큼 이후 시간에 주기보고를 재시도 해야 한다.\n
* Optional (선택적인 필드임을 의미한다)
*/
	U8  usDomainCode[LEN_DOMAIN_CODE];
	U8  usGWID[LEN_GW_ID];
	U8  usDeviceID[LEN_DEVICE_ID];
	U8  ucResultCode;
	U8  unBackOffTime[4];
}stPacketDeliveryRspHdr; //Delivery Response

/**
 * @struct Delivery_Req
 * @brief GMMP TCP Packet  구조 중 Request Delivery 구조체
 *  OMP에 주기보고 데이터를 전송한다.
 */
typedef struct {
/**
*@var header
*@brief @ref GMMPHeader 참조
*/
/**
*@var body
*@brief @ref stPacketDeliveryReqHdr 참조
*/
	GMMPHeader				header;
	stPacketDeliveryReqHdr	body;
}Delivery_Req;

/**
 * @struct Delivery_Rsp
 * @brief GMMP TCP Packet  구조 중 Response Delivery 구조체
 * OMP에 주기보고 데이터 전송 응답을 수신한다.
 */
typedef struct {
/**
*@var header
*@brief @ref GMMPHeader 참조
*/
/**
*@var body
*@brief @ref stPacketDeliveryRspHdr 참조
*/
	GMMPHeader				header;
	stPacketDeliveryRspHdr	body;
}Delivery_Rsp;
#pragma pack(0)

#endif /* STRUCT_DELIVERY_H_ */

