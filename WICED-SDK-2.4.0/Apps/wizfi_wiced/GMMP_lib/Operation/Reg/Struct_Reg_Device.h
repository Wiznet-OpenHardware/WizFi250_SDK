 /** GMMP Operation 중 Device Registration 명령에 사용되는 구조체
  * @date 2013/11/21
 * @version 0.0.0.1
 * @file Struct_Reg_Device.h
 **/

#ifndef STRUCT_REG_DEVICE_H_
#define STRUCT_REG_DEVICE_H_

#include "../../Define/Define.h"
#include "../Struct_Common.h"

#pragma pack(1)
/**
 * @struct stDeviceRegistrationReqHdr
 * @brief GMMP TCP Packet  구조 중 Request 수행 시 Message Header와 Message Body 영역
  * OMP에서 Device Registration 메시지를 OMP에 전송한다.
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
*@var usManufactureID
*@brief M2M GW의 Manufacture ID 이다.\n
- AlphaNumeric : 문자,숫자 조합, 특수문자제외, SPACE 허용 안하며 빈자리는 0x00(NULL)로 채운다.
Ex) 공장 제조 Serial No. : “AVB12132SET23DT”, “SKTSerial“
*/
	U8  usDomainCode[LEN_DOMAIN_CODE];
	U8  usGWID[LEN_GW_ID];
	U8  usManufactureID[LEN_MANUFACTURE_ID];
}stDeviceRegistrationReqHdr; //Device Registration Request

/**
 * @struct stDeviceRegistrationRspHdr
 * @brief GMMP TCP Packet  구조 중 Response 수행 시 Message Header와 Message Body 영역
 * OMP에서 Device Registration 메시지를 OMP에 수신한다.
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
}stDeviceRegistrationRspHdr; //Device Registration Response

/**
 * @struct DeviceRegist_Req
 * @brief GMMP TCP Packet  구조 중 Request Device Regist 구조체
 * OMP에 Request Device Regist 데이터 전송한다.
 */
typedef struct {
/**
*@var header
*@brief @ref GMMPHeader 참조
*/
/**
*@var body
*@brief @ref stDeviceRegistrationReqHdr 참조
*/
	GMMPHeader				header;
	stDeviceRegistrationReqHdr	body;
}DeviceRegist_Req;

/**
 * @struct DeviceRegist_Rsp
 * @brief GMMP TCP Packet  구조 중 Response Device Regist 구조체
  * OMP에 Response Device Regist 데이터 수신한다.
 */
typedef struct {
/**
*@var header
*@brief @ref GMMPHeader 참조
*/
/**
*@var body
*@brief @ref stDeviceRegistrationRspHdr 참조
*/
	GMMPHeader				header;
	stDeviceRegistrationRspHdr	body;
}DeviceRegist_Rsp;
#pragma pack(0)

#endif /* STRUCT_REG_DEVICE_H_ */
