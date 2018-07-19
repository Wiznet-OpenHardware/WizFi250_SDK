/** GMMP 에러 코드를 문자열로 변환 하여 반환.
  * @date 2013/11/21
 * @version 0.0.0.1
 * @file StringTable.h
 **/


#ifndef STRINGTABLE_H_
#define STRINGTABLE_H_

#include "ErrorCode.h"
#include "OMPErrorCode.h"
#include "../Operation/Struct_Common.h"

/**
 *@brief 라이브러리 사용중 발생하는 에러 코드를 문자열로 변환하여 제공한다.\n
 *@brief 변환 리스트\n
	GMMP_SUCCESS\n
	GMMP_ERROR_UNKWOUN\n
	GMMP_LOG_ERROR_OPEN\n
	GMMP_LOG_ERROR_WRITE\n
	GMMP_LOG_ERROR_CLOSE\n
	GMMP_LOG_NO_WRITE_DATA\n
	GMMP_LOG_LONG_STRING\n
	SERVER_INFO_NOT_FOUND\n
	SERVER_REQUEST_TIMEOUT\n
	SERVER_SOCKET_ERROR\n
	SERVER_CONNECT_ERROR\n
	SERVER_DISCONNECT\n
	LIB_PARAM_ERROR\n
	GMMP_THREAD_ERROR_NOCRATE\n
	GMMP_MALLOC_ERROR\n
 * @param nErrorCode 에러코드 입력
 * @return 문자열로 변환한 값
 */
char* GetStringtoErrorCode(int nErrorCode);


/**
 *@brief OMP 서버에서 제공하는 에러코드를 문자열로 변환하여 제공한다.\n
 *@brief 변환 리스트\n
	UnKnown Error Code\n
	STATUS_OK\n
	STATUS_CREATED\n
	STATUS_ACCEPTED\n
	STATUS_BAD_REQUEST\n
	STATUS_PERMISSION_DENIED\n
	STATUS_FORBIDDEN\n
	STATUS_NOT_FOUND\n
	STATUS_METHOD_NOT_ALLOWED\n
	STATUS_NOT_ACCEPTABLE\n
	STATUS_REQUEST_TIMEOUT\n
	STATUS_CONFLICT\n
	STATUS_UNSUPPORTED_MEDIA_TYPE\n
	STATUS_INTERNAL_SERVER_ERROR\n
	STATUS_NOT_IMPLEMENTED\n
	STATUS_BAD_GATEWAY\n
	STATUS_SERVICE_UNAVAILABLE\n
	STATUS_GATEWAY_TIMEOUT\n
	STATUS_EXPIRED\n
	STATUS_SMS_SEND_FAIL\n
	UnDefined User_Defined_Result_Code\n
	STATUS_DB_TIMEOUT\n
	STATUS_MSPTCP_DISCONNECTED\n
	STATUS_GMMPTCP_PORT_CLOSED\n
	STAUS_AO_CONTROL_SEND_FAIL\n
 * @param cOMPErrorCode 에러코드 입력
 * @return 문자열로 변환한 값
 */
char* GetStringtoOMPErrorCode(const unsigned char  cOMPErrorCode);

/**
 * @brief 헥사코드를 정수형으로 변환한다.
 * @param hex
 * @return 정수값
 */
int xtoi(const char hex);

#endif /* STRINGTABLE_H_ */
