 /** GMMP Operation 중 Registration 명령 함수
 * @date 2013/11/21
 * @version 0.0.0.1
 * @file GMMP_Reg.h
 **/

#ifndef GMMP_REG_H_
#define GMMP_REG_H_

#include "../../Network/Network.h"

#include "Struct_Reg_GW.h"
#include "Struct_Reg_Device.h"

#include "Struct_DeReg_GW.h"
#include "Struct_DeReg_Device.h"




//public
/**
 *
 * @param pGwRegist_Req @ref GwRegist_Req참조
* @return 성공 : GMMMP_SUCCESS, 실패 : @ref ErrorCode.h 참조
 */
int GMMP_GW_Reg_Req(GwRegist_Req*  pGwRegist_Req);

/**
 *
 * @param pGwRegist_Rsp @ref GwRegist_Rsp참조
* @return 성공 : GMMMP_SUCCESS, 실패 : @ref ErrorCode.h 참조
 */
int GMMP_GW_Reg_Rsp(GwRegist_Rsp* pGwRegist_Rsp);

/**
 *
 * @param pDeviceRegist_Req @ref DeviceRegist_Req참조
* @return 성공 : GMMMP_SUCCESS, 실패 : @ref ErrorCode.h 참조
 */
int GMMP_Device_Reg_Req(DeviceRegist_Req*  pDeviceRegist_Req);

/**
 *
 * @param pDeviceRegist_Rsp @ref DeviceRegist_Rsp참조
* @return 성공 : GMMMP_SUCCESS, 실패 : @ref ErrorCode.h 참조
 */
int GMMP_Device_Reg_Rsp(DeviceRegist_Rsp*  pDeviceRegist_Rsp);

/**
 *
 * @param pGwDeRegist_Req @ref GwDeRegist_Req참조
* @return 성공 : GMMMP_SUCCESS, 실패 : @ref ErrorCode.h 참조
 */
int GMMP_GW_DeReg_Req(GwDeRegist_Req* pGwDeRegist_Req);

/**
 *
 * @param pGwDeRegist_Rsp @ref GwDeRegist_Rsp참조
* @return 성공 : GMMMP_SUCCESS, 실패 : @ref ErrorCode.h 참조
 */
int GMMP_GW_DeReg_Rsp(GwDeRegist_Rsp* pGwDeRegist_Rsp);

/**
 *
 * @param pDeviceDeRegist_Req @ref DeviceDeRegist_Req참조
* @return 성공 : GMMMP_SUCCESS, 실패 : @ref ErrorCode.h 참조
 */
int GMMP_Device_DeReg_Req(DeviceDeRegist_Req*  pDeviceDeRegist_Req);

/**
 *
 * @param pDeviceDeRegist_Rsp @ref DeviceDeRegist_Rsp참조
* @return 성공 : GMMMP_SUCCESS, 실패 : @ref ErrorCode.h 참조
 */
int GMMP_Device_DeReg_Rsp(DeviceDeRegist_Rsp*  pDeviceDeRegist_Rsp);


#endif /* GMMP_REG_H_ */

