/** GMMP Operation 중 Multimedia URL Info 명령에 사용되는 함수
* @file GMMP_Multimedia.h
* @date 2013/11/21
* @version 0.0.0.1
* @ref GMMPHeader참조
**/

#ifndef GMMP_MULTIMEDIA_H_
#define GMMP_MULTIMEDIA_H_

#include "../../Network/Network.h"
#include "Struct_Multimedia.h"

/**
 *
 * @param pMMURLInfo_Req @ref MMURLInfo_Req참조
  * @return 성공 : GMMMP_SUCCESS, 실패 : @ref ErrorCode.h 참조
 */
int GMMP_Multimedia_Req(MMURLInfo_Req*  pMMURLInfo_Req);

/**
 *
 * @param pMMURLInfo_Rsp @ref MMURLInfo_Rsp참조
  * @return 성공 : GMMMP_SUCCESS, 실패 : @ref ErrorCode.h 참조
 */
int GMMP_Multimedia_Rsp(MMURLInfo_Rsp* pMMURLInfo_Rsp);

#endif /* GMMP_MULTIMEDIA_H_ */
