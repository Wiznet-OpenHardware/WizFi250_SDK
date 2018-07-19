 /** GMMP Operation 중 Encryption Info/ Encryption Key 명령에 사용되는 구조체
  * @date 2013/11/21
 * @version 0.0.0.1
 * @file Struct_Encrypt.h
 **/

#ifndef STRUCT_ENCRYPT_H_
#define STRUCT_ENCRYPT_H_

#include "../../Define/Define.h"
#include "../Struct_Common.h"

#pragma pack(1)
/**
 * @struct stEncryptionInfoReqHdr
 *@brief GMMP TCP Packet  구조 중 Request 수행 시 Message Header와 Message Body 영역.
 * OMP에 암호화 관련 정보 요청 명령을 전송 시 사용된다.
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
	U8  usGWID[LEN_GW_ID]; //<GW ID
	U8  usDeviceID[LEN_DEVICE_ID];//Device ID
}stEncryptionInfoReqHdr; //Encryption Info Request

/**
 * @struct stEncryptionInfoRspHdr
 *@brief GMMP TCP Packet  구조 중 Response 수행 시 Message Header와 Message Body 영역.
 * OMP에서 암호화 관련 정보 요청 명령 응답 완료 수신 시 사용된다
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
*@var ucEncryptionFlag
*@brief 암호화 설정 여부.\n
*0x00 : 암호화 설정 안함.\n
*0x01 : 암호화 설정.
*/
/**
*@var ucEncryptionAlgorithm
*@brief 암호화 알고리즘 정보.\n
0x01 : AES 128\n
0x02 : AES 192\n
0x03 : AES 256\n
0x04 : SEED 128\n
0x05 : SEED 256\n
*/
	U8  usDomainCode[LEN_DOMAIN_CODE];
	U8  usGWID[LEN_GW_ID];
	U8  usDeviceID[LEN_DEVICE_ID];
	U8  ucResultCode;
	U8  ucEncryptionFlag;
	U8  ucEncryptionAlgorithm;
}stEncryptionInfoRspHdr; ///Encryption Info Response

/**
 * @struct EncryptInfo_Req
 * @brief GMMP TCP Packet  구조 중 Request Encryption Info 구조체
 *  OMP에 암호화 정보 요청을 전송한다.
 */
typedef struct {
/**
*@var header
*@brief @ref GMMPHeader 참조
*/
/**
*@var body
*@brief @ref stEncryptionInfoReqHdr 참조
*/
	GMMPHeader					header;
	stEncryptionInfoReqHdr	body;
}EncryptInfo_Req;

/**
 * @struct EncryptInfo_Rsp
 * @brief GMMP TCP Packet  구조 중 Response Encryption Info 구조체
 *  OMP에 암호화 정보 요청 응답을 수신한다.
 */
typedef struct {
/**
*@var header
*@brief @ref GMMPHeader 참조
*/
/**
*@var body
*@brief @ref stEncryptionInfoRspHdr 참조
*/
	GMMPHeader					header;
	stEncryptionInfoRspHdr	body;
}EncryptInfo_Rsp;

/**
 * @struct stEncryptionKeyReqHdr
 *@brief GMMP TCP Packet  구조 중 Request 수행 시 Message Header와 Message Body 영역.
 * OMP에 생성한 암호화 키 설정 요청 명령을 전송 시 사용된다.
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
*@brief M2M Device ID, 값이 0x0이면 GW 주기 보고 명령이다.
*/
/**
*@var usEncryptionKey
*@brief 생성된 암호화 키 문자열.
*/
	U8  usDomainCode[LEN_DOMAIN_CODE];
	U8  usGWID[LEN_GW_ID];
	U8  usDeviceID[LEN_DEVICE_ID];
	U8  usEncryptionKey[LEN_ENCRYTION_KEY];
}stEncryptionKeyReqHdr; //Encryption Key Request

/**
 * @struct stEncryptionKeyRspHdr
 *@brief GMMP TCP Packet  구조 중 Response 수행 시 Message Header와 Message Body 영역.
 * OMP에 생성한 암호화 키 설정 요청 완료 명령 수신 시 사용된다.
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
*@brief M2M Device ID, 값이 0x0이면 GW 주기 보고 명령이다.
*/
/**
*@var ucResultCode
*@brief Result Code (0x00 : Success, other : Fail Reason) @ref OMPErrorCode.h 참조
*/
	U8  usDomainCode[LEN_DOMAIN_CODE];
	U8  usGWID[LEN_GW_ID];
	U8  usDeviceID[LEN_DEVICE_ID];
	U8  ucResultCode;
}stEncryptionKeyRspHdr; ///Encryption Key Response

/**
 * @struct EncryptKey_Req
 * @brief GMMP TCP Packet  구조 중 Request Encryption Key 구조체
 * OMP에 암호화 키 등록 요청 메시지를 전송한다.
 */
typedef struct {
/**
*@var header
*@brief @ref GMMPHeader 참조
*/
/**
*@var body
*@brief @ref stEncryptionKeyReqHdr 참조
*/
	GMMPHeader					header;
	stEncryptionKeyReqHdr	body;
}EncryptKey_Req;

/**
 * @struct EncryptKey_Rsp
 * @brief GMMP TCP Packet  구조 중 Response Encryption Key 구조체
 * OMP에 암호화 키 등록을 요청 완료 메시지를 수신한다.
 */
typedef struct {
/**
*@var header
*@brief @ref GMMPHeader 참조
*/
/**
*@var body
*@brief @ref stEncryptionKeyRspHdr 참조
*/
	GMMPHeader					header;
	stEncryptionKeyRspHdr	body;
}EncryptKey_Rsp;
#pragma pack(0)

#endif /* STRUCT_ENCRYPT_INFO_H_ */
