/** GMMP 공통 구조체
  * @date 2013/11/21
 * @version 0.0.0.1
 * @file Struct_Common.h
 **/

#ifndef STRUCT_H_
#define STRUCT_H_

#include "../Define/Define.h"

#pragma pack(1)
/**
 *@struct GMMPHeader
 *@brief GMMP TCP Packet Common Header
 */
typedef struct {
/**@var ucVersion
* @brief GMMP Version
*/
    U8  ucVersion;

/**@var usMessageLength
* @brief GMMP 통신 패킷의 전체 길이.
*/
/**@var ucMessageType
* @brief 메시지의 유형으로서 단말등록, 주기보고, 단말제어 등 각각에 대해 요청과 응답을 구분한다.\n
* @ref Define_Operation.h 참조
*/
/**@var unOriginTimeStamp
* @brief Unix Time(=POSIX time, 1970년 1월1일부터 경과된 초)을 사용한다. 메시지를 전송하는 쪽에서 해당 값을 설정한다.
*/
/**@var usTotalCount
* @brief Message Body에 전달될 내용이 2048 Bytes를 초과할 경우 여러 개의 메시지로 전송하며 전체 메시지 개수를 표시 한다.
*/
/**@var usCurrentCount
* @brief 여러 개의 메시지로 전송되는 경우 현재 메시지의 순서로서 1에서 Total Count까지의 값이 기록 된다.
*/
/**@var usAuthID
* @brief M2M GW 의 Serial Number 값이며, MSISDN 또는 MAC Address값을 사용할 수도 있다.
*/
/**@var usAuthKey
* @brief GMMP Message를 인증하기 위해 사용되는 Key 값으로 등록 절차 시 OMP에서 할당한다.\n
*  (등록 이전에 Default 로 16byte를 0x00으로 채운다.)
*/
/**@var usTID
* @brief T-ID는 M2M GW와 OMP 간의 트랜잭션을 구분하기 위한 값으로서 Request 요청하는 곳에서 할당하며 Response 받은 곳에서는 해당 T-ID 값을 그대로 반환한다.
M2M GW와 OMP에서 T-ID 사용 범위는 다음과 같다. (“6.4 T-ID 사용 범위”절 참조)\n
※ M2M GW T-ID : 0 ~ 99,999 (10만개)\n
※ OMP T-ID : M2M GW가 사용 이외의 값.
*/
/**@var ucReserved1
* @brief Message Body를 암호화하여 전송시 0x01로 설정한다. 그렇지 않은 경우에는 0x00으로 설정한다.
*/
/**@var ucReserved2
* @brief 현재 사용하지 않음 ( 0x00으로 설정한다.).
*/
    U8  usMessageLength[2];
    U8  ucMessageType;
    U8  unOriginTimeStamp[4];
    U8  usTotalCount[2];
    U8  usCurrentCount[2];
    U8  usAuthID[LEN_AUTH_ID];
    U8  usAuthKey[LEN_AUTH_KEY];
    U8  usTID[LEN_TID];
    U8  ucReserved1;
    U8  ucReserved2;
}GMMPHeader; //기본 구조체

/**@union ConvertShort
* @brief short형 변수를 endian 변환을 위해 사용된다.
*/

typedef union{
	U8		usShort[2];
	short	sU8;
}ConvertShort;

/**@union ConvertInt
* @brief short형 변수를 endian 변환을 위해 사용된다.
*/
typedef union{
	U8		usInt[4];
	int		sU8;
}ConvertInt;


/**
 *@struct SMSMessage
 *@brief GMMP SMS Packet
 *@brief SMS 메시지는 Header Message와 최대 38byte를 갖는 Body Message로 구성된다. \n
 *@brief Header 메시지는 제어 명령을 구분할 수 있는 T-ID(Transaction ID)와 제어의 종류를 나타내는 Control Type Code값을 포함한다.
 *@ref Define_Operation.h 참조
 */
typedef struct {
/**@var unTID
* @brief T-ID는 M2M GW와 OMP 간의 트랜잭션을 구분하기 위한 값으로서 Request 요청하는 곳에서 할당하며 Response 받은 곳에서는 해당 T-ID 값을 그대로 반환한다.
M2M GW와 OMP에서 T-ID 사용 범위는 다음과 같다. (“6.4 T-ID 사용 범위”절 참조)\n
※ M2M GW T-ID : 0 ~ 99,999 (10만개)\n
※ OMP T-ID : M2M GW가 사용 이외의 값.
*/
/**@var ucControlTypeCode
* @brief M2M 단말로의 제어 명령 전달 시 제어 명령에 대한 Code값이다.
* @ref Define_Operation.h 참조
*/
/**@var ucLongSentenceFlag
* @brief 0x00 : 제어 데이터의 크기가 38 bytes 이하여서 SMS Control Message Body 에 제어 데이터가 포함되어 있으며, Long Sentence Operation을 수행할 필요 없다.
* @brief 0x01 : 제어 데이터의 크기가 38 bytes 이상이므로 SMS Control Message Body 에 제어 데이터가 포함되어 있지않고, M2M GW/Device Long Sentence(@ref GMMP_LSentence_Operation 참조)을 수행하여 제어 데이터를 획득해야 한다.
*/
/**@var usDeviceID
* @brief M2M Device ID
* @warning 0x00이면 GW의 단말제어로 간주
*/
/**@var usSMSMessageBody
* @brief 단말에게 전달될 제어 명령에 대한 내용이 포함된다.
*/
	U8 	unTID[4];
	U8		ucControlTypeCode;
	U8		ucLongSentenceFlag;
	U8		usDeviceID[LEN_DEVICE_ID];
	U8		usSMSMessageBody[38];
}SMSMessage;

#pragma pack(0)
#endif /* STRUCT_H_ */



