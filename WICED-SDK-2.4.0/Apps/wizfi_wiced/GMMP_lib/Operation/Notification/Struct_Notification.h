/** GMMP Operation 중 제어 명령 수행 결과 보고 명령에 사용되는 구조체
* @file Struct_Notification.h
* @date 2013/11/21
* @version 0.0.0.1
* @ref GMMPHeader참조
**/

#ifndef STRUCT_NOTIFICATION_H_
#define STRUCT_NOTIFICATION_H_

#include "../../Define/Define.h"
#include "../Struct_Common.h"

#pragma pack(1)
/**
 * @struct stNotificationReqHdr
 * @brief GMMP TCP Packet  구조 중 Request 수행 시 Message Header와 Message Body 영역
 * OMP에서 제어 메시지를 수행 후 제어 메시지 결과를 OMP에 보고한다.
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
*@brief 제어 명령 수신 메시지에 수신한 값과 동일한 값을 사용한다. @ref Define_Control.h 참조
*/
/**
*@var ucResultCode
*@brief Result Code (0x00 : Success, other : Fail Reason) @ref OMPErrorCode.h 참조
*/
/**
*@var usMessageBody
*@brief Data [2048byte] ucControlType 정의에 의해 세부 내용이 변경 된다.
*@brief  @ref Struct_Control_Opt.h 참조
*@warning 옵션으로 사용하지 않을 수 있다.
*/
	U8  usDomainCode[LEN_DOMAIN_CODE];
	U8  usGWID[LEN_GW_ID];
	U8  usDeviceID[LEN_DEVICE_ID];
	U8  ucControlType;
	U8  ucResultCode;
	U8  usMessageBody[MAX_MSG_BODY];
}stNotificationReqHdr; //Notification Request

/**
 * @struct stNotificationRspHdr
 * @brief GMMP TCP Packet  구조 중 Response 수행 시 Message Header와 Message Body 영역
  * 제어 메시지 결과 응답을를 수신한다.
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
*@brief 제어 명령 수신 메시지에 수신한 값과 동일한 값을 사용한다 @ref Define_Control.h 참조
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
}stNotificationRspHdr; //Notification Response

/**
 * @struct Notifi_Req
 * @brief GMMP TCP Packet  구조 중 Request Notification 구조체
 * OMP에 Request Notification 데이터 전송한다.
 */
typedef struct {
/**
*@var header
*@brief @ref GMMPHeader 참조
*/
/**
*@var body
*@brief @ref stNotificationReqHdr 참조
*/
	GMMPHeader				header;
	stNotificationReqHdr	body;
}Notifi_Req;

/**
 * @struct Notifi_Rsp
 * @brief GMMP TCP Packet  구조 중 Response Notification 구조체
  * OMP에 Response Notification 데이터 수신한다.
 */
typedef struct {
/**
*@var header
*@brief @ref GMMPHeader 참조
*/
/**
*@var body
*@brief @ref stNotificationRspHdr 참조
*/
	GMMPHeader				header;
	stNotificationRspHdr	body;
}Notifi_Rsp;
#pragma pack(0)

#endif /* STRUCT_NOTIFICATION_H_ */
