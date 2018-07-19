 /** GMMP Operation 중 Device DeRegistration 명령에 사용되는 구조체
  * @date 2013/11/21
 * @version 0.0.0.1
 * @file Struct_DeReg_Device.h
 **/

#ifndef STRUCT_DEREG_DEVICE_H_
#define STRUCT_DEREG_DEVICE_H_

#include "../../Define/Define.h"
#include "../Struct_Common.h"

#pragma pack(1)
/**
 * @struct stDeviceDeRegistrationReqHdr
 * @brief GMMP TCP Packet  구조 중 Request 수행 시 Message Header와 Message Body 영역
  * OMP에서 Device DeRegistration 메시지를 OMP에 전송한다.
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
}stDeviceDeRegistrationReqHdr; //GW Registration Request

/**
 * @struct stDeviceDeRegistrationRspHdr
 * @brief GMMP TCP Packet  구조 중 Response 수행 시 Message Header와 Message Body 영역
 * OMP에서 Device DeRegistration 메시지를 OMP에 수신한다.
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
}stDeviceDeRegistrationRspHdr; //GW Registration Response

/**
 * @struct DeviceDeRegist_Req
 * @brief GMMP TCP Packet  구조 중 Request Device DeRegist 구조체
 * OMP에 Request  Device DeRegist 데이터 전송한다.
 */
typedef struct {
/**
*@var header
*@brief @ref GMMPHeader 참조
*/
/**
*@var body
*@brief @ref stDeviceDeRegistrationReqHdr 참조
*/
	GMMPHeader					header;
	stDeviceDeRegistrationReqHdr	body;
}DeviceDeRegist_Req;

/**
 * @struct DeviceDeRegist_Rsp
 * @brief GMMP TCP Packet  구조 중 Response Device DeRegist 구조체
  * OMP에 Response Device DeRegist 데이터 수신한다.
 */
typedef struct {
/**
*@var header
*@brief @ref GMMPHeader 참조
*/
/**
*@var body
*@brief @ref stDeviceDeRegistrationRspHdr 참조
*/
	GMMPHeader					header;
	stDeviceDeRegistrationRspHdr	body;
}DeviceDeRegist_Rsp;
#pragma pack(0)

#endif /* STRUCT_DEREG_DEVICE_H_ */
