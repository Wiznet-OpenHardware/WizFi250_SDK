/** GMMP Operation 중 제어 명령 리스트 정의
 * @file Define_Control.h
 * @date 2013/11/21
 * @version 0.0.0.1
 * @ref GMMPHeader참조
 **/

#ifndef DEFINE_CONTROL_H_
#define DEFINE_CONTROL_H_

//Control Command define using ControlReqHdr
#define CONTROL_Reset								0x01		///<단말 리셋
#define CONTROL_Turn_Off							0x02		///<단말 Turn off
#define CONTROL_Report_On							0x03		///<단말 보고 데이터 전송 On
#define CONTROL_Report_Off							0x04		///<단말 보고 데이터 전송 Off
#define CONTROL_Time_Sync							0x05		///<단말 시각 동기화 설정
#define CONTROL_Pause								0x06		///<단말 동작 일시중지
#define CONTROL_Restart								0x07		///<단말 동작 재시작

#define CONTROL_Signal_Power_Check					0x08
#define CONTROL_Diagnostic							0x09		///<단말 상태 진단
#define CONTROL_Reserved							0x0a		///<Reserved
#define CONTROL_Profile_Reset						0x0b		///<단말 Profile 정보 재설정

#define CONTROL_Status_Check						0x0c		///<단말 동작 상태 조회
#define CONTROL_FW_Download							0x0d		///<단말 펌웨어 다운로드
#define CONTROL_FW_Update							0x0e		///<단말 펌웨어 업그레이드

#define CONTROL_App_Download						0x0f		///<단말 App 다운로드
#define CONTROL_App_Update							0x10		///<단말 App 업그레이드

#define CONTROL_Remote_Access						0x11		///<단말 원격 터미널 접속
#define CONTROL_Multimedia_Control_Start			0x12		///<멀티미디어 통신 제어 시작
#define CONTROL_Multimedia_Control_Pause			0x13		///<멀티미디어 통신 제어 멈춤
#define CONTROL_Multimedia_Control_Stop				0x14		///<멀티미디어 통신 제어 종료
#define CONTROL_Multimedia_Control_Restart			0x15		///<멀티미디어 통신 제어 재시작

#define Industrial_Reserved_Min						0x20		///<산업별 Reserved
#define Industrial_Reserved_Max						0x7f		///<산업별 Reserved
#define User_defined_Control_Min					0x80		///<사용자 정의 제어 타입

#define User_defined_Control_Max					0x9f		///<사용자 정의 제어 타입

#define CONTROL_Frequency_Band_Change				0xa0		///<단말 주파수 밴드 변경

#define CONTROL_Mode_Change							0xa1		///<단말 3G/LTE 변경

#define CONTROL_Communication_Status_Check			0xa2		///<단말 통신상태 요청
#define CONTROL_Firmware_Version_Check				0xa3		///<단말 펌웨어 버전 조회
#define Reserved_Min								0xa4		///<Reserved
#define Reserved_Max								0xff		///<Reserved

#endif /* DEFINE_CONTROL_H_ */
