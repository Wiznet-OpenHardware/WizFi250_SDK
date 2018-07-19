/** GMMP 라이브러리 에러코드
 * @file ErrorCode.h
 * @date 2013/11/21
 * @version 0.0.0.1
 **/

#ifndef ERRORCODE_H_
#define ERRORCODE_H_

//error type define
#define GMMP_SUCCESS							0

#define GMMP_ERROR_DEFAULT					10000
#define GMMP_ERROR_UNKWOUN					10001
#define GMMP_LOG_ERROR_OPEN					10002
#define GMMP_LOG_ERROR_WRITE				10003
#define GMMP_LOG_ERROR_CLOSE				10004
#define GMMP_LOG_NO_WRITE_DATA				10005
#define GMMP_LOG_LONG_STRING				10006
#define SERVER_INFO_NOT_FOUND				10007
#define SERVER_REQUEST_TIMEOUT				10008
#define SERVER_SOCKET_ERROR					10009
#define SERVER_CONNECT_ERROR				10010
#define SERVER_DISCONNECT					10011
#define LIB_PARAM_ERROR						10012
#define GMMP_MALLOC_ERROR					10013
#define GMMP_HEADER_SIZE_ERROR				10014

#define GMMP_ERRPR_MAX						10015

#endif /* ERRORCODE_H_ */
