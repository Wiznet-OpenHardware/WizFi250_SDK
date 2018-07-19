/** GMMP Operation 절차 리스트 정의
 * @file Define_Operation.h
 * @date 2013/11/21
 * @version 0.0.0.1
 * @ref GMMPHeader참조
 **/

#ifndef DEFINE_OPERATION_H_
#define DEFINE_OPERATION_H_


#include "Define_Delivery.h"
#include "Define_Control.h"

#define OPERATION_GW_REG_REQ						0x01 ///<OMP에 GW 등록 Request
#define OPERATION_GW_REG_RSP						0x02 ///<OMP에 GW 등록 Response

#define OPERATION_GW_DEREG_REQ						0x03 ///<OMP에 GW 등록 Request
#define OPERATION_GW_DEREG_RSP						0x04 ///<OMP에 GW 등록 Response

#define OPERATION_PROFILE_REQ						0x05 ///<OMP에 GW 등록 Request
#define OPERATION_PROFILE_RSP						0x06 ///<OMP에 GW 등록 Response

#define OPERATION_DEVICE_REG_REQ					0x07 ///<OMP에 GW 등록 Request
#define OPERATION_DEVICE_REG_RSP					0x08 ///<OMP에 GW 등록 Response

#define OPERATION_DEVICE_DEREG_REQ 				0x09 ///<OMP에 GW 등록 Request
#define OPERATION_DEVICE_DEREG_RSP 				0x0a ///<OMP에 GW 등록 Response

#define OPERATION_DELIVERY_REQ						0x0b ///<OMP에 GW 등록 Request
#define OPERATION_DELIVERY_RSP						0x0c ///<OMP에 GW 등록 Response

#define OPERATION_CONTROL_REQ						0x0d ///<OMP에 GW 등록 Request
#define OPERATION_CONTROL_RSP						0x0e ///<OMP에 GW 등록 Response

#define OPERATION_HEARTBEAT_REQ						0x0f ///<OMP에 GW 등록 Request
#define OPERATION_HEARTBEAT_RSP						0x10 ///<OMP에 GW 등록 Response

#define OPERATION_LSENTENCE_REQ						0x11 ///<OMP에 GW 등록 Request
#define OPERATION_LSENTENCE_RSP						0x12 ///<OMP에 GW 등록 Response

#define OPERATION_FTP_INFO_REQ						0x13 ///<OMP에 GW 등록 Request
#define OPERATION_FTP_INFO_RSP						0x14 ///<OMP에 GW 등록 Response

#define OPERATION_REMOTE_INFO_REQ					0x15 ///<OMP에 GW 등록 Request
#define OPERATION_REMOTE_INFO_RSP					0x16 ///<OMP에 GW 등록 Response

#define OPERATION_NOTIFICATION_REQ					0x17 ///<OMP에 GW 등록 Request
#define OPERATION_NOTIFICATION_RSP					0x18 ///<OMP에 GW 등록 Response

#define OPERATION_ENCRYPTION_INFO_REQ				0x19 ///<OMP에 GW 등록 Request
#define OPERATION_ENCRYPTION_INFO_RSP				0x1a ///<OMP에 GW 등록 Response

#define OPERATION_ENCRYPTION_KEY_REQ				0x1b ///<OMP에 GW 등록 Request
#define OPERATION_ENCRYPTION_KEY_RSP				0x1c ///<OMP에 GW 등록 Response

#define OPERATION_MULTIMEDIA_URL_INFO_REQ			0x1d ///<OMP에 GW 등록 Request
#define OPERATION_MULTIMEDIA_URL_INFO_RSP			0x1e ///<OMP에 GW 등록 Response

#define OPERATION_LOB_CLOUD_INFO_REQ				0x1f ///<OMP에 GW 등록 Request
#define OPERATION_LOB_CLOUD_INFO_RSP				0x20 ///<OMP에 GW 등록 Response

#define OPERATION_LOB_FTP_INFO_REQ					0x21 ///<OMP에 GW 등록 Request
#define OPERATION_LOB_FTP_INFO_RSP					0x22 ///<OMP에 GW 등록 Response

#define OPERATION_LOB_UPLOAD_NOTIFICATION_REQ		0x23 ///<OMP에 GW 등록 Request
#define OPERATION_LOB_UPLOAD_NOTIFICATION_RSP		0x24 ///<OMP에 GW 등록 Response


#endif /* DEFINE_OPERATION_H_ */
