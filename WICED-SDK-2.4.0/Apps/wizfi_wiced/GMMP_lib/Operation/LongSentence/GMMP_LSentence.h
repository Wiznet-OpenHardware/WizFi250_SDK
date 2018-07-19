/** GMMP Operation 중 Long Sentence 명령에 사용되는 함수
* @file GMMP_LSentence.h
* @date 2013/11/21
* @version 0.0.0.1
* @ref GMMPHeader참조
**/

#ifndef GMMP_LSENTENCE_H_
#define GMMP_LSENTENCE_H_

#include "../../Network/Network.h"
#include "Struct_LSentence.h"

/**
 *
 * @param pLSentence_Req @ref LSentence_Req참조
  * @return 성공 : GMMMP_SUCCESS, 실패 : @ref ErrorCode.h 참조
 */
int GMMP_LSentence_Req(LSentence_Req*  pLSentence_Req);

/**
 *
 * @param pLSentence_Rsp @ref LSentence_Rsp참조
  * @return 성공 : GMMMP_SUCCESS, 실패 : @ref ErrorCode.h 참조
 */
int GMMP_LSentence_Rsp(LSentence_Rsp*  pLSentence_Rsp);

#endif /* GMMP_LSENTENCE_H_ */
