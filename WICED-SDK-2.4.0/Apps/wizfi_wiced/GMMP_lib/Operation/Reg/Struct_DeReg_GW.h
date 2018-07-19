 /** GMMP Operation 중 GW DeRegistration 명령에 사용되는 구조체
  * @date 2013/11/21
 * @version 0.0.0.1
 * @file Struct_DeReg_GW.h
 **/

#ifndef STRUCT_DEREG_GW_H_
#define STRUCT_DEREG_GW_H_

#include "../../Define/Define.h"
#include "../Struct_Common.h"

#pragma pack(1)
/**
 * @struct stGwDeRegistrationReqHdr
 * @brief GMMP TCP Packet  구조 중 Request 수행 시 Message Header와 Message Body 영역
 * OMP에서 GW DeRegistration 메시지를 OMP에 전송한다.
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
}stGwDeRegistrationReqHdr; //GW Registration Request

/**
 * @struct stGwDeRegistrationRspHdr
 * @brief GMMP TCP Packet  구조 중 Response 수행 시 Message Header와 Message Body 영역
 * OMP에서 GW DeRegistration 메시지를 OMP에 수신한다.
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
*@var ucResultCode
*@brief Result Code (0x00 : Success, other : Fail Reason) @ref OMPErrorCode.h 참조
*/
	U8  usDomainCode[LEN_DOMAIN_CODE];
	U8  usGWID[LEN_GW_ID];
	U8  ucResultCode;
}stGwDeRegistrationRspHdr; //GW Registration Response

/**
 * @struct GwDeRegist_Req
 * @brief GMMP TCP Packet  구조 중 Request GW DeRegist 구조체
 * OMP에 Request GW DeRegist 데이터 전송한다.
 */
typedef struct {
/**
*@var header
*@brief @ref GMMPHeader 참조
*/
/**
*@var body
*@brief @ref stGwDeRegistrationReqHdr 참조
*/
	GMMPHeader					header;
	stGwDeRegistrationReqHdr	body;
}GwDeRegist_Req;

/**
 * @struct GwDeRegist_Rsp
 * @brief GMMP TCP Packet  구조 중 Response GW DeRegist 구조체
  * OMP에 Response GW DeRegist 데이터 수신한다.
 */
typedef struct {
/**
*@var header
*@brief @ref GMMPHeader 참조
*/
/**
*@var body
*@brief @ref stGwDeRegistrationRspHdr 참조
*/
	GMMPHeader					header;
	stGwDeRegistrationRspHdr	body;
}GwDeRegist_Rsp;
#pragma pack(0)

#endif /* STRUCT_DEREG_GW_H_ */
