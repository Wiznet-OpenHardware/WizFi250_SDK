/** GMMP Operation 중 Heartbeat 명령에 사용되는 구조체
* @file Struct_Heartbeat.h
* @date 2013/11/21
* @version 0.0.0.1
* @ref GMMPHeader참조
**/

#ifndef STRUCT_HEARTBEAT_H_
#define STRUCT_HEARTBEAT_H_

#include "../../Define/Define.h"
#include "../Struct_Common.h"

#pragma pack(1)
/**
 * @struct stHeartBeatMsgReqHdr
 * @brief GMMP TCP Packet  구조 중 Request 수행 시 Message Header와 Message Body 영역
 * OMP에 Heartbeat 보고 명령을 전송 시 사용된다.
 * @warning (TCP Always On mode에서만 동작한다.)
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
	U8  usDomainCode[LEN_DOMAIN_CODE];
	U8  usGWID[LEN_GW_ID];
}stHeartBeatMsgReqHdr; //Heartbeat Request

/**
 * @struct stHeartBeatMsgRspHdr
 * @brief GMMP TCP Packet  구조 중 Request 수행 시 Message Header와 Message Body 영역
 * OMP에 Heartbeat 보고 응답 명령을 수신 시 사용된다.
 * @warning (TCP Always On mode에서만 동작한다.)
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
	U8  usDomainCode[LEN_DOMAIN_CODE];
	U8  usGWID[LEN_GW_ID];
}stHeartBeatMsgRspHdr; //Heartbeat Response

/**
 * @struct HB_Req
 * @brief GMMP TCP Packet  구조 중 Request Heartbeat 구조체
 * OMP에 Heartbeat 보고 명령을 전송한다.
 */
typedef struct {
/**
*@var header
*@brief @ref GMMPHeader 참조
*/
/**
*@var body
*@brief @ref stHeartBeatMsgReqHdr 참조
*/
	GMMPHeader				header;
	stHeartBeatMsgReqHdr	body;
}HB_Req;

/**
 * @struct HB_Rsp
 * @brief GMMP TCP Packet  구조 중 Response Heartbeat 구조체
 * OMP에 Heartbeat 보고 응답 명령을 수신한다.
 */
typedef struct {
/**
*@var header
*@brief @ref GMMPHeader 참조
*/
/**
*@var body
*@brief @ref stHeartBeatMsgRspHdr 참조
*/
	GMMPHeader				header;
	stHeartBeatMsgRspHdr	body;
}HB_Rsp;
#pragma pack(0)

#endif /* STRUCT_HEARTBEAT_H_ */
