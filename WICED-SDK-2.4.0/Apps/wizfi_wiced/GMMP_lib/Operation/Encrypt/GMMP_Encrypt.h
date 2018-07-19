/** GMMP Operation 중 암호화 설정 관련 명령에 사용되는 구조체
* @file GMMP_Encrypt.h
* @date 2013/11/21
* @version 0.0.0.1
* @ref GMMPHeader참조
**/

#ifndef GMMP_ENCRYPT_H_
#define GMMP_ENCRYPT_H_

#include "../../Network/Network.h"
#include "Struct_Encrypt.h"


/**
 *
 * @param pEncryptInfo_Req @ref EncryptInfo_Req 참조
  * @return 성공 : GMMMP_SUCCESS, 실패 : @ref ErrorCode.h 참조
 */
int GMMP_Encrypt_Info_Req(EncryptInfo_Req*  pEncryptInfo_Req);

/**
 *
 * @param pEncryptInfo_Rsp @ref EncryptInfo_Rsp 참조
  * @return 성공 : GMMMP_SUCCESS, 실패 : @ref ErrorCode.h 참조
 */
int GMMP_Encrypt_Info_Rsp(EncryptInfo_Rsp*  pEncryptInfo_Rsp);

/**
 *
 * @param pEncryptKey_Req @ref EncryptKey_Req 참조
  * @return 성공 : GMMMP_SUCCESS, 실패 : @ref ErrorCode.h 참조
 */
int GMMP_Encrypt_Key_Req(EncryptKey_Req*  pEncryptKey_Req);

/**
 *
 * @param pEncryptKey_Rsp @ref EncryptKey_Rsp 참조
  * @return 성공 : GMMMP_SUCCESS, 실패 : @ref ErrorCode.h 참조
 */
int GMMP_Encrypt_Key_Rsp(EncryptKey_Rsp*  pEncryptKey_Rsp);

#endif /* GMMP_ENCRYPT_H_ */
