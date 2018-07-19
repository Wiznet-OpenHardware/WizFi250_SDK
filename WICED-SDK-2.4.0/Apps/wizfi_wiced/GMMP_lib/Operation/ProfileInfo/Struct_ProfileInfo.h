/** GMMP Operation 중 제어 명령 수행 결과 보고 명령에 사용되는 구조체
* @file Struct_ProfileInfo.h
* @date 2013/11/21
* @version 0.0.0.1
* @ref GMMPHeader참조
**/

#ifndef STRUCT_PROFILEINFO_H_
#define STRUCT_PROFILEINFO_H_

#include "../../Define/Define.h"
#include "../Struct_Common.h"

#pragma pack(1)
/**
 * @struct stProfileReqHdr
 * @brief GMMP TCP Packet  구조 중 Request 수행 시 Message Header와 Message Body 영역
 * OMP에서 Profile Info  메시지를 OMP에 전송한다.
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
}stProfileReqHdr; //Profile Request

/**
 * @struct stProfileRspHdr
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
*@var ucResultCode
*@brief Result Code (0x00 : Success, other : Fail Reason) @ref OMPErrorCode.h 참조
*/
/**
*@var unHeartbeatPeriod
*@brief Heartbeat 주기로서 분(min)단위이다.\n
*@brief 0x00 : Non Always On 단말\n
*@brief other : TCP Always On 단말\n
*@brief (Device Profile 조회시에는 해당 값이 0으로 전달)\n
*/
	/**
	*@var unReportPeriod
	*@brief 보고 주기로서 분(minute) 단위이다.\n
	*@brief (Device Profile 조회시에는 해당 값이 0으로 전달)
	*/
/**
*@var unReportOffset
*@brief 보고 주기 Offset 시간으로서 초(sec) 단위이다.\n
@brief 0이면 보고주기때 보고하고, 0이 아니면 보고 주기가 되었을때 단말은 Reporting OffSet Time 이후에 주기보고 동작을 해야한다.
@brief (Device Profile 조회시에는 해당 값이 0으로 전달)
*/
/**
*@var unResponseTimeout
*@brief M2M 단말의 Request메시지에 대한 Response 메시지의 Timeout(최소 30sec 이상으로 설정한다.)\n
*@brief (Device Profile 조회시에는 해당 값이 0으로 전달)\n
*/
/**
*@var usModel
*@brief 제조사에서 부여한 Device 의 이름
*/
/**
*@var usFirmwareVersion
*@brief Firmware Version
*/
/**
*@var usSoftwareVersion
*@brief Software Version
*/
/**
*@var usHardwareVersion
*@brief Hardware Version
*/
	U8  usDomainCode[LEN_DOMAIN_CODE];
	U8  usGWID[LEN_GW_ID];
	U8  usDeviceID[LEN_DEVICE_ID];
	U8  ucResultCode;
	U8  unHeartbeatPeriod[4];
	U8  unReportPeriod[4];
	U8  unReportOffset[4];
	U8  unResponseTimeout[4];
	U8  usModel[32];
	U8  usFirmwareVersion[16];
	U8  usSoftwareVersion[16];
	U8  usHardwareVersion[16];
}stProfileRspHdr; //Profile Response

/**
 * @struct Profile_Req
 * @brief GMMP TCP Packet  구조 중 Request Profile Info 구조체
 * OMP에 Request Profile Info  데이터 전송한다.
 */
typedef struct {
/**
*@var header
*@brief @ref GMMPHeader 참조
*/
/**
*@var body
*@brief @ref stProfileReqHdr 참조
*/
	GMMPHeader		header;
	stProfileReqHdr	body;
}Profile_Req;

/**
 * @struct Profile_Rsp
 * @brief GMMP TCP Packet  구조 중 Response Profile Info 구조체
  * OMP에 Response Profile Info 데이터 수신한다.
 */

typedef struct {
/**
*@var header
*@brief @ref GMMPHeader 참조
*/
/**
*@var body
*@brief @ref stProfileRspHdr 참조
*/
	GMMPHeader		header;
	stProfileRspHdr	body;
}Profile_Rsp;
#pragma pack(0)

#endif /* STRUCT_PROFILEINFO_H_ */
