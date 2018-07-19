 /** GMMP Operation 중 Notification 명령 함수
  * @date 2013/11/21
 * @version 0.0.0.1
 * @file GMMP_Notification.h
 **/

#ifndef GMMP_NOTIFICATION_H_
#define GMMP_NOTIFICATION_H_

#include "../../Network/Network.h"
#include "Struct_Notification.h"

/**
 *
 * @param pNotifi_Req @ref Notifi_Req참조
 * @param nPacketSize 실제 전송해야하는 패킷 크기
   * @return 성공 : GMMMP_SUCCESS, 실패 : @ref ErrorCode.h 참조
 */
int GMMP_Notifi_Req(Notifi_Req*  pNotifi_Req, int nPacketSize);

/**
 *
 * @param pNotifi_Rsp @ref Notifi_Rsp참조
 * @return 성공 : GMMMP_SUCCESS, 실패 : @ref ErrorCode.h 참조
 */
int GMMP_Notifi_Rsp(Notifi_Rsp*  pNotifi_Rsp);

#endif /* GMMP_NOTIFICATION_H_ */
