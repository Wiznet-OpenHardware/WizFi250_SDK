/** Log
  * @date 2013/11/21
 * @version 0.0.0.1
 * @file GMMP_Log.h
 **/

#ifndef GMMP_LOG_H_
#define GMMP_LOG_H_

#include "../Define/Define.h"

#define LEN_LOG_DATA			2048*2 ///<로그 메시지 전체 길이
#define LEN_TIME_DATE		9		///<Date String Buffer Len
#define LEN_TIME_TIME		9 		///<Time String Buffer Len

/**
 * @brief 로그 파일 및 형식 초기화.
 * @param pszFileName 저장할 파일 이름
 * @param nDayPart 저장할 파일 이름에 일 기준으로 생성 유무 판단. true, fasle.
 * @return 성공 : GMMMP_SUCCESS, 실패 : @ref ErrorCode.h 참조
 */
int GMMP_Log_Init(const char* pszFileName, int nDayPart);

/**
  * @brief 로그 메시지를 저장한다.
 * @param pMessage 저장할 메시지
 * @param nLen 저장할 메시지 길이
 * @return 성공 : GMMMP_SUCCESS, 실패 : @ref ErrorCode.h 참조
 */
int GMMP_Log_Write(char* pMessage, int nLen);

/**
 *@brief Log파일 닫기
 */
void GMMP_Log_Close();

/**
 *@brief 현재 Date_Time 문자열 가져오기
 * @param pBuffer 버퍼
 * @param nLen 버퍼의 크기
 * @return
 */
int _GetDateTime(char* pBuffer, int nLen);

/**
 *@brief 현재 Date 문자열 가져오기
 * @param pBuffer 버퍼
 * @param nLen 버퍼의 크기
 */
void _GetDate(char* pBuffer, int nLen);

/**
 * @brief 현재 Time 문자열 가져오기
 * @param pBuffer 버퍼
 * @param nLen 버퍼의 크기
 */
void _GetTime(char* pBuffer, int nLen);


#endif /* GMMP_LOG_H_ */
