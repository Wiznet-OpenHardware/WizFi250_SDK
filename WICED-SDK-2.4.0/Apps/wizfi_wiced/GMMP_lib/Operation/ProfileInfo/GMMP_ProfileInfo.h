 /** GMMP Operation 중 Profile Info 명령 함수
  * @date 2013/11/21
 * @version 0.0.0.1
 * @file GMMP_ProfileInfo.h
 **/

#ifndef GMMP_PROFILEINFO_H_
#define GMMP_PROFILEINFO_H_

#include "../../Network/Network.h"
#include "Struct_ProfileInfo.h"

/**
 *
 * @param pProfile_Req @ref Profile_Req참조
 * @return 성공 : GMMMP_SUCCESS, 실패 : @ref ErrorCode.h 참조
 */
int GMMP_Profile_Req(Profile_Req*  pProfile_Req);

/**
 *
 * @param pstProfile_Rsp @ref Profile_Rsp참조
 * @return 성공 : GMMMP_SUCCESS, 실패 : @ref ErrorCode.h 참조
 */
int GMMP_Profile_Rsp(Profile_Rsp* pstProfile_Rsp);

#endif /* GMMP_PROFILEINFO_H_ */
