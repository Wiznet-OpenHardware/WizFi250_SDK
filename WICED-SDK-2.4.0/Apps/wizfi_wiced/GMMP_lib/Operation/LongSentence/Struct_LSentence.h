/** GMMP Operation 중 Long Sentence 명령에 사용되는 구조체
* @file Struct_LSentence.h
* @date 2013/11/21
* @version 0.0.0.1
* @ref GMMPHeader참조
**/

#ifndef STRUCT_LSENTENCE_H_
#define STRUCT_LSENTENCE_H_


#include "../../Define/Define.h"
#include "../Struct_Common.h"

#pragma pack(1)
/**
 * @struct stLSentenceReqHdr
 * @brief GMMP TCP Packet  구조 중 Request 수행 시 Message Header와 Message Body 영역
 * OMP에 Long Sentence 명령을 전송 시 사용된다.
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
*@brief 제어 명령 수신 메시지에 수신한 값(0x80~0x9f)과 동일한 값을 사용한다
*@ref Define_Control.h 참조
*/
	U8  usDomainCode[LEN_DOMAIN_CODE];
	U8  usGWID[LEN_GW_ID];
	U8  usDeviceID[LEN_DEVICE_ID];
	U8 ucControlType;
}stLSentenceReqHdr; //Long Sentence Request

/**
 * @struct stLSentenceRspHdr
 * @brief GMMP TCP Packet  구조 중 Response 수행 시 Message Header와 Message Body 영역
 * OMP에서 Long Sentence 정보 제공 명령  수신 시 사용된다.
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
*@brief 제어 명령 수신 메시지에 수신한 값(0x80~0x9f)과 동일한 값을 사용한다
*@ref Define_Control.h 참조
*/
/**
*@var ucResultCode
*@brief Result Code (0x00 : Success, other : Fail Reason) @ref OMPErrorCode.h 참조
*/
/**
*@var usMessageBody
*@brief Data[2048 Byte]
*/
	U8  usDomainCode[LEN_DOMAIN_CODE];
	U8  usGWID[LEN_GW_ID];
	U8  usDeviceID[LEN_DEVICE_ID];
	U8  ucControlType;
	U8  ucResultCode;
	U8  usMessageBody[MAX_MSG_BODY];
}stLSentenceRspHdr; //Long Sentence Response

/**
 * @struct LSentence_Req
 * @brief GMMP TCP Packet  구조 중 Request Long Sentence 구조체
 * OMP에 Long Sentence 데이터 전송한다.
 */
typedef struct {
/**
*@var header
*@brief @ref GMMPHeader 참조
*/
/**
*@var body
*@brief @ref stLSentenceReqHdr 참조
*/
	GMMPHeader					header;
	stLSentenceReqHdr	body;
}LSentence_Req;

/**
 * @struct LSentence_Rsp
 * @brief GMMP TCP Packet  구조 중 Response Long Sentence 구조체
 * OMP에 Long Sentence 데이터 전송 응답을 수신한다.
 */
typedef struct {
/**
*@var header
*@brief @ref GMMPHeader 참조
*/
/**
*@var body
*@brief @ref stLSentenceRspHdr 참조
*/
	GMMPHeader					header;
	stLSentenceRspHdr	body;
}LSentence_Rsp;
#pragma pack(0)
#endif /* STRUCT_LSENTENCE_H_ */
