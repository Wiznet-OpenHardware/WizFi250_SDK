  /** GMMP Operation 중 주기보고 명령 함수
 * @file GMMP_Delivery.h
 * @date 2013/11/21
 * @version 0.0.0.1
 * @ref GMMPHeader참조
 **/
#ifndef GMMP_DELIVERY_H_
#define GMMP_DELIVERY_H_

#include "../../Network/Network.h"
#include "Struct_Delivery.h"


/**
 * @param pControl_Req @ref Delivery_Req참조
 *  @param nPacketSize 패킷 크기
 * @return 성공 : GMMMP_SUCCESS, 실패 : @ref ErrorCode.h 참조
 */
int GMMP_Delivery_Req(Delivery_Req*  pControl_Req, int nPacketSize);

/**
 * @param pControl_Rsp @ref Delivery_Rsp참조
 * @return 성공 : GMMMP_SUCCESS, 실패 : @ref ErrorCode.h 참조
 */
int GMMP_Delivery_Rsp(Delivery_Rsp*  pControl_Rsp);

#endif /* GMMP_DELIVERY_H_ */
