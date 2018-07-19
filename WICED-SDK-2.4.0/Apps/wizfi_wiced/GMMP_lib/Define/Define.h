/** GMMP 怨듯넻 �젙�쓽
 * @file Define.h
 * @date 2013/11/21
 * @version 0.0.0.1
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef DEFINE_H_
#define DEFINE_H_

#include "Define_Operation.h"
#include "../ErrorCode/ErrorCode.h"

#define true							1
#define false							0

#define MAX_MSG_BODY					2048
#define MAX_SMS_MSG_BODY				38

#define LEN_DOMAIN_CODE					10
#define LEN_GW_ID						16
#define LEN_DEVICE_ID					16
#define LEN_AUTH_KEY					16
#define LEN_AUTH_ID						16
#define LEN_MANUFACTURE_ID				16
#define LEN_ENCRYTION_KEY				32
#define LEN_MULTIMEDIA_URL				256
#define LEN_MULTIMEDIA_ID				16
#define LEN_MULTIMEDIA_PW				16
#define LEN_LOB_ACCESS_KEY				20
#define LEN_LOB_SECRET_KEY				16
#define LEN_LOB_DIRECTORY_PATH			16
#define LEN_LOB_PATH					64
#define LEN_TID							4
#define LEN_IP							16
#define LEN_FILE_NAME					32

#define TID_TANGE_MIN 					0
#define TID_TANGE_MAX 					99999

#define GMMP_ON_LOG						true
#define GMMP_OFF_LOG					false

#define GMMP_ERROR_LEVEL_ERROR			0
#define GMMP_ERROR_LEVEL_DEBUG			1
#define GMMP_ERROR_LEVEL_DEBUG_DATA	2

#define GMMP_LOG_MARKET_OPS				0
#define GMMP_LOG_MARKET_REQ				1
#define GMMP_LOG_MARKET_RSP				2
#define GMMP_LOG_MARKET_OPE				3
#define GMMP_LOG_MARKET_ERR				4
#define GMMP_LOG_MARKET_NOT				5

#define GMMP_LOG_KEY_KEY0				"<OMP>"
// sekim 20151222 Clear GMMP Log
//#define GMMP_LOG_KEY_KEY1				"<ID:%s_%d>"
#define GMMP_LOG_KEY_KEY1				""
#define GMMP_LOG_KEY_KEY2				"<Marker:OPS>"
#define GMMP_LOG_KEY_KEY3				"<Marker:REQ>"
#define GMMP_LOG_KEY_KEY4				"<Marker:RSP>"
#define GMMP_LOG_KEY_KEY5				"<Marker:OPE>"
#define GMMP_LOG_KEY_KEY6				"<Marker:ERR>"


#define GMMP_MARKET_LEN					15


#define GMMP_NETWORK_ALYWAYS_OFF 		0
#define GMMP_NETWORK_ALYWAYS_ON 		1

#define GMMP_GW							0x01
#define GMMP_Device  					0x02

#define GMMP_REG						0x01
#define GMMP_DEREG						0x02
#define GMMP_PROFILE					0x03
#define GMMP_DELIVERY					0x04
#define GMMP_CONTROL					0x05
#define GMMP_LSENTENCE 					0x06
#define GMMP_FTP						0x07
#define GMMP_REMOTE						0x08
#define GMMP_HEARTBEAT					0x09
#define GMMP_NOTIFICATION				0x0a
#define GMMP_MULTIMEDIA					0x0b
#define GMMP_LOB						0x0c

#define GMMP_REG_ENCRYPTION_INFO 		0x01
#define GMMP_REG_ENCRYPTION_KEY 		0x02

#define GMMP_ENCRYPTION_USE				0x01
#define GMMP_ENCRYPTION_NOT				0x02

#define GMMP_LOB_CLOUD					0x01
#define GMMP_LOB_FTP					0x02
#define GMMP_LOB_NOTIFICATION			0x03

#define U8									unsigned char

#define GMMP_VERSION						0x21

extern char g_szServerIP[LEN_IP];
extern int g_nServerPort;
extern char g_szAuthID[LEN_AUTH_ID];
extern char g_szAuthKey[LEN_AUTH_KEY];
extern char g_szDomainCode[LEN_DOMAIN_CODE];
extern char g_szGWID[LEN_GW_ID];
extern char g_szGWMFID[LEN_MANUFACTURE_ID];
extern char g_szDeviceID[LEN_DEVICE_ID];

extern int g_nTID;

#define GMMP_DEBUG_STRING	"GMMP Trace : "

#endif /* DEFINE_H_ */




