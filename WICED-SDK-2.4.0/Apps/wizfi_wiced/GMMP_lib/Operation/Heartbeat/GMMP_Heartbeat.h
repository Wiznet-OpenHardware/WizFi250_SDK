/** GMMP Operation 중 Heartbeat 명령 힘수
* @file GMMP_Heartbeat.h
* @date 2013/11/21
* @version 0.0.0.1
* @ref GMMPHeader참조
**/

#ifndef GMMP_HEARBEAT_H_
#define GMMP_HEARBEAT_H_

#include "../../Network/Network.h"
#include "Struct_Heartbeat.h"


/**
 *
 * @param pHB_Req @ref HB_Req 참조
  * @return 성공 : GMMMP_SUCCESS, 실패 : @ref ErrorCode.h 참조
 */
int GMMP_Heartbeat_Req(HB_Req*  pHB_Req);

/**
 *
 * @param pHB_Rsp @ref HB_Rsp 참조
  * @return 성공 : GMMMP_SUCCESS, 실패 : @ref ErrorCode.h 참조
 */
int GMMP_Heartbeat_Rsp(HB_Rsp*  pHB_Rsp);

#endif /* GMMP_HEARBEAT_H_ */
