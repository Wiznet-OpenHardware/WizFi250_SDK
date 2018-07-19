/** GMMP Operation 중 FTP 정보 요청 명령 함수
* @file GMMP_FTP.h
* @date 2013/11/21
* @version 0.0.0.1
* @ref GMMPHeader참조
**/

#ifndef GMMP_FTP_H_
#define GMMP_FTP_H_


#include "../../Network/Network.h"
#include "Struct_FTP.h"

/**
 *
 * @param pFTP_Req @ref FTP_Req참조
  * @return 성공 : GMMMP_SUCCESS, 실패 : @ref ErrorCode.h 참조
 */
int GMMP_FTP_Info_Req(FTP_Req*  pFTP_Req);

/**
 *
 * @param pFTP_Rsp @ref FTP_Rsp참조
  * @return 성공 : GMMMP_SUCCESS, 실패 : @ref ErrorCode.h 참조
 */
int GMMP_FTP_Info_Rsp(FTP_Rsp*  pFTP_Rsp);

#endif /* GMMP_FTP_H_ */
