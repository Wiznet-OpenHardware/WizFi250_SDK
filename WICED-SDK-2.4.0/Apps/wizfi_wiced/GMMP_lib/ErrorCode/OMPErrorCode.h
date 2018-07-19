/** OMP에서 제공하는 에러코드
 * @file OMPErrorCode.h
 * @date 2013/11/21
 * @version 0.0.0.1
 **/


#ifndef OMPERRORCODE_H_
#define OMPERRORCODE_H_

#define STATUS_OK												0x00
#define STATUS_CREATED										0x01
#define STATUS_ACCEPTED										0x02
#define STATUS_BAD_REQUEST									0x03
#define STATUS_PERMISSION_DENIED							0x04
#define STATUS_FORBIDDEN										0x05
#define STATUS_NOT_FOUND										0x06
#define STATUS_METHOD_NOT_ALLOWED							0x07
#define STATUS_NOT_ACCEPTABLE								0x08
#define STATUS_REQUEST_TIMEOUT								0x09
#define STATUS_CONFLICT										0x0a
#define STATUS_UNSUPPORTED_MEDIA_TYPE						0x0b
#define STATUS_INTERNAL_SERVER_ERROR						0x0c
#define STATUS_NOT_IMPLEMENTED								0x0d
#define STATUS_BAD_GATEWAY									0x0e
#define STATUS_SERVICE_UNAVAILABLE							0x0f
#define STATUS_GATEWAY_TIMEOUT								0x10
#define STATUS_DELETED										0x11
#define STATUS_EXPIRED										0x12
#define STATUS_SMS_SEND_FAIL								0x13

#define User_Defined_Result_Code_Min						0x80
#define User_Defined_Result_Code_Max						0xff

#define STATUS_DB_TIMEOUT									0x81	///<과부하에 의한 DB Access time out의 경우
#define STATUS_MSPTCP_DISCONNECTED							0x82	///<MNP와 MSP간의 TCP 소켓이 끊어 졌을 경우
#define STATUS_GMMPTCP_PORT_CLOSED							0x83	///<단말과 GMMP Listen Port의 TCP연결이 끊어진 경우
#define STAUS_AO_CONTROL_SEND_FAIL							0x84	///<TCP Always On 제어 전송 실패인 경우


#endif /* OMPERRORCODE_H_ */
