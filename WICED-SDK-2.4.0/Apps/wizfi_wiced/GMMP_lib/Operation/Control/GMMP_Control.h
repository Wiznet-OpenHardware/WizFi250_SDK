 /** GMMP Operation 중 제어 명령 함수
 * @file GMMP_Control.h
 * @date 2013/11/21
 * @version 0.0.0.1
 * @ref GMMPHeader참조
 **/

#ifndef GMMP_CONTROL_H_
#define GMMP_CONTROL_H_

#include "../../Network/Network.h"
#include "Struct_Control.h"



/**
 *@brief 제어 명령 수신 구조체
 * @param pControl_Req @ref Control_Req참조
 * @return 성공 : GMMMP_SUCCESS, 실패 : @ref ErrorCode.h 참조
 */
int GMMP_Control_Req(Control_Req*  pControl_Req);

/**
 * @brief 제어 명령 확인 전송 구조체
 * @param pControl_Req @ref Control_Rsp참조
 * @return 성공 : GMMMP_SUCCESS, 실패 : @ref ErrorCode.h 참조
 */
int GMMP_Control_Rsp(Control_Rsp*  pControl_Req);

#endif /* GMMP_CONTROL_H_ */
