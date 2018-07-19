/** GMMP Operation 중 LOB 관련 명령에 사용되는 함수
* @file GMMP_LOB.h
* @date 2013/11/21
* @version 0.0.0.1
* @ref GMMPHeader참조
**/

#ifndef GMMP_LOB_H_
#define GMMP_LOB_H_

#include "../../Network/Network.h"
#include "Struct_LOB.h"


/**
 *
 * @param pLOBCloud_Req @ref LOBCloud_Req참조
  * @return 성공 : GMMMP_SUCCESS, 실패 : @ref ErrorCode.h 참조
 */
int GMMP_LOB_Cloud_Info_Req(LOBCloud_Req*  pLOBCloud_Req);

/**
 *
 * @param pLOBCloud_Rsp @ref LOBCloud_Rsp참조
  * @return 성공 : GMMMP_SUCCESS, 실패 : @ref ErrorCode.h 참조
 */
int GMMP_LOB_Cloud_Info_Rsp(LOBCloud_Rsp*  pLOBCloud_Rsp);

/**
 *
 * @param pLOBFTP_Req @ref LOBFTP_Req참조
  * @return 성공 : GMMMP_SUCCESS, 실패 : @ref ErrorCode.h 참조
 */
int GMMP_LOB_FTP_Info_Req(LOBFTP_Req*  pLOBFTP_Req);

/**
 *
 * @param pLOBFTP_Rsp @ref LOBFTP_Rsp참조
  * @return 성공 : GMMMP_SUCCESS, 실패 : @ref ErrorCode.h 참조
 */
int GMMP_LOB_FTP_Info_Rsp(LOBFTP_Rsp*  pLOBFTP_Rsp);

/**
 *
 * @param pLOBNotifi_Req @ref LOBNotifi_Req참조
  * @return 성공 : GMMMP_SUCCESS, 실패 : @ref ErrorCode.h 참조
 */
int GMMP_LOB_Notification_Req(LOBNotifi_Req*  pLOBNotifi_Req);

/**
 *
 * @param pLOBNotifi_Rsp @ref LOBNotifi_Rsp참조
  * @return 성공 : GMMMP_SUCCESS, 실패 : @ref ErrorCode.h 참조
 */
int GMMP_LOB_Notification_Rsp(LOBNotifi_Rsp*  pLOBNotifi_Rsp);

#endif /* GMMP_LOB_H_ */
