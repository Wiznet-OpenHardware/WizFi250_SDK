 /** GMMP Operation 중 Remote Info 명령 함수
  * @date 2013/11/21
 * @version 0.0.0.1
 * @file GMMP_Remote.h
 **/

#ifndef GMMP_REMOTE_H_
#define GMMP_REMOTE_H_


#include "../../Define/Define.h"
#include "../../Network/Network.h"
#include "Struct_Remote.h"

/**
 *
 * @param pRemote_Req @ref Remote_Req참조
* @return 성공 : GMMMP_SUCCESS, 실패 : @ref ErrorCode.h 참조
 */
int GMMP_Remote_Req(Remote_Req*  pRemote_Req);

/**
 *
 * @param pRemote_Rsp @ref Remote_Rsp참조
* @return 성공 : GMMMP_SUCCESS, 실패 : @ref ErrorCode.h 참조
 */
int GMMP_Remote_Rsp(Remote_Rsp* pRemote_Rsp);

#endif /* GMMP_REMOTE_H_ */
