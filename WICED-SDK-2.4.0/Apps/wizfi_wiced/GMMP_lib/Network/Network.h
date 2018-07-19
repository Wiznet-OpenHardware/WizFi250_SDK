/** TCP Socket 통신을 위한 모듈
 * @file Network.h
 * @date 2013/11/21
 * @version 0.0.0.1
 **/

#ifndef NETWORK_H_
#define NETWORK_H_

#include "../Define/Define.h"
#include "../Util/GMMP_Util.h"
#include "../Log/GMMP_Log.h"


/**
 *@brief 서버와 연결된 TCP Socket을 닫는다.
 */
void CloseSocket();

/**
 * @brief TCP Socket을 연결한다.
 * @return 성공 : GMMMP_SUCCESS, 실패 : @ref ErrorCode.h 참조
 */
int Connect();

/**
 * @brief TCP Socket에 Write한다.
 * @param pBuf Write 버퍼 포인트.
 * @param nLen Write 버퍼 크기.
 * @return 성공 : GMMMP_SUCCESS, 실패 : @ref ErrorCode.h 참조
 */
int WriteTCP(char* pBuf, int nLen);

/**
 * @brief TCP Socket을 Read한다.
 * @param _pBuf Read 버퍼 포인트.
 * @param _nMaxlen Read 버퍼 최대 크기
 * @return 성공 : GMMMP_SUCCESS, 실패 : @ref ErrorCode.h 참조
 */
int ReadTCP(char* _pBuf, const int _nMaxlen);

/**
 * @brief TCP Socket Buffer를 비운다.
 */
void ClearBuffer();

/**
 * @brief TCP Socket 세션 유지를 확인한다. 세션이 연결이 되어 있지 않다면 연결을 시도한다.
 * @return 성공 : GMMMP_SUCCESS, 실패 : @ref ErrorCode.h 참조
 */
int CheckSocket();
#endif /* NETWORK_H_ */





