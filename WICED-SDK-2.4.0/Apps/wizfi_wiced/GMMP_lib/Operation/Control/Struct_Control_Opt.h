 /** GMMP 제어 명령 중 Message Body의 값이 필요한 경우 사용되는 구조체
 * @file Struct_Control_Opt.h
 * @date 2013/11/21
 * @version 0.0.0.1
 * @ref GMMPHeader참조
 **/


#ifndef STRUCT_CONTROL_OPT_H_
#define STRUCT_CONTROL_OPT_H_

#include "../../Define/Define.h"
#include "../Struct_Common.h"

#pragma pack(1)
/**
 * @struct TimeStamp
 * @brief 제어 명령 중 CONTROL_Time_Sync 일 경우 usMessageBody의 값\n
 * @ref stControlReqHdr 참조
 * 제어 메시지 수신 데이터에 포함되어 있다.
 */
typedef struct {
/**
*@var unTimeStamp
*@brief Unix Time
*/
	U8 unTimeStamp[4];
}TimeStamp;

/**
 * @struct SignalPowerCheck
 * @brief 제어 명령 중 CONTROL_Signal_Power_Check 일 경우 usMessageBody의 값\n
 * @ref stNotificationReqHdr 참조
 * 제어 메시지 수행 후 제어 메시지 결과 전송 시 포함한다.
 */
typedef struct {
/**
*@var unSignalPower
*@brief RSSI(dBm), Ec/Lo(dB)
*/
	U8 unSignalPower[4];
}SignalPowerCheck;

/**
 * @struct Diagnostic
 * @brief 제어 명령 중 CONTROL_Diagnostic 일 경우 usMessageBody의 값\n
 * @ref stNotificationReqHdr 참조
 * 제어 메시지 수행 후 제어 메시지 결과 전송 시 포함한다.
 */
typedef struct {
/**
*@var ucStatusValue
*@brief OK = 0, NOK = 1
*/
	U8 ucStatusValue;
}Diagnostic;

/**
 * @struct StatusCheck
 * @brief 제어 명령 중 CONTROL_Status_Check 일 경우 usMessageBody의 값\n
 * @ref stNotificationReqHdr 참조
 * 제어 메시지 수행 후 제어 메시지 결과 전송 시 포함한다.
 */
typedef struct {
/**
*@var ucOnOffValue
*@brief OK = 0, OFF = 1
*/
/**
*@var RunPauseValue
*@brief Run = 0, Pause = 1
*/

	U8 ucOnOffValue;
	U8 RunPauseValue;
}StatusCheck;

/**
 * @struct FW_APP_Download_Update_Remote
 * @brief 제어 명령 중 CONTROL_FW_Download, CONTROL_FW_Update, CONTROL_App_Download, CONTROL_App_Update, CONTROL_Remote_Access 일 경우 usMessageBody의 값\n
 * @ref stNotificationReqHdr 참조
 * 제어 메시지 수행 후 제어 메시지 결과 전송 시 포함한다.
 */
typedef struct {
/**
*@var unStartTime
*@brief 다운로드 시작 시각, Unix Time(=POSIX time, 1970년 1월1일부터 경과된 초)을 사용한다.

*/
/**
*@var unEndTime
*@brief 다운로드 종료 시각, Unix Time(=POSIX time, 1970년 1월1일부터 경과된 초)을 사용한다.

*/
/**
*@var ucResultCode
*@brief Result Code (0x00 : Success, other : Fail Reason)
*@ref OMPErrorCode.h 참조
*/

	U8 unStartTime[4];
	U8 unEndTime[4];
	U8 ucResultCode;
}FW_APP_Download_Update_Remote;


/**
 * @struct APP_Update
 * @brief 제어 명령 중 CONTROL_App_Update 일 경우 usMessageBody의 값\n
 * @ref stControlReqHdr 참조
 * OMP에서 제어 명령 메시지를 전송하여 GMMP Lib에서 해당 메시지를 수신한다.
 */
typedef struct {
/**
*@var usFileName
*@brief Application 파일 이름, App Name과 버전 정보를 합친 unique 이름  (예, AppName_1.0.0).
*/
/**
*@var ucAppUpdateCommand
*@brief App Activation = 0, App Deactivation = 1
*/

	U8 usFileName[LEN_FILE_NAME];
	U8 ucAppUpdateCommand;
}APP_Update;
#pragma pack(0)

#endif /* STRUCT_CONTROL_OPT_H_ */
