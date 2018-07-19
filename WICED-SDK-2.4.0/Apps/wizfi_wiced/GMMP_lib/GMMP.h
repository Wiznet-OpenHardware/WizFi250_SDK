/**
 * @mainpage GMMP (Global M2M Protocol) 개요
  * @brief GMMP는 M2M 단말과 개방형 M2M 플랫폼(OMP)사이의 TCP 기반 연동 규격으로서, 단말 등록/해지, 주기보고, 단말 제어 등의 기능을 수행하는 SKTelecom 내부 규격이다.
 * @date 2013/11/21
 * @version 0.0.0.1
 * @file GMMP.h
 **/

#ifndef GMMP_H_
#define GMMP_H_

#include "ErrorCode/StringTable.h"
#include "Operation/GMMP_Operation.h"

/**
 * @defgroup GMMP_Registration_Operation M2M GW/Device 등록
 *등록 절차는 현장에 설치된 M2M GW가 서비스 수행을 시작하기 위해서 최초로 수행해야 하는 절차.\n

전송 패킷 : @ref Struct_Reg_GW.h, @ref Struct_Reg_Device.h 참조

 * @ingroup GMMP_Registration_Operation
 * @{
 */

/**
 *
 *@brief GW,Device 등록 절차 진행 명령 함수, 옵션으로 암호화 진행 유무 Flag가 있다.\n
 등록 절차는 현장에 설치된 M2M GW가 서비스 수행을 시작하기 위해서 최초로 수행해야 하는 절차이다.\n
 등록 절차를 통해 GW ID, Device ID를 OMP로 부터 제공 받는다.\n
 암호화 진행 시 @ref GO_EncInfo을 자동 진행한다.\n
 *@warning GW 등록 절차가 진행되어야만 Device 등록 절차를 진행 할 수 있다.\n
 * @param pszGWID OMP에서 제공받은 GW ID, NULL이면 GW 등록, 아니면 Device 등록 절차를 진행한다.\n
 * @param pszManufactureID M2M GW,Device의 Manufacture ID이다.\n
- AlphaNumeric : 문자,숫자 조합, 특수문자제외, SPACE 허용 안하며 빈자리는 0x00(NULL)로 채운다.\n
Ex) 공장 제조 Serial No. : “AVB12132SET23DT”, “SKTSerial“\n
 * @param nEncryption 옵션 : 암화호 정보 요청 진행 유무 Flag.\n
 * @return 성공 : GMMMP_SUCCESS, 실패 : @ref ErrorCode.h 참조
 */

int GO_Reg(const char* pszGWID,
const char* pszManufactureID,
int nEncryption);
/**
 * @}
 */

/**
 * @defgroup GMMP_DeRegistration_Operation M2M GW/Device 등록 해지
 *해지 절차는 M2M Device가 더 이상 서비스를 수행하지 않음을 OMP에 알리는 절차로서 Device ID값이 OMP에 반납된다.\n

전송 패킷 : @ref Struct_DeReg_GW.h, @ref Struct_DeReg_Device.h 참조

 * @ingroup GMMP_DeRegistration_Operation
 * @{
 */

/**
 *@brief GW,Device 해지 절차 진행 명령 함수
 *@brief 해지 절차는 등록 절차를 통해 OMP로 부터 제공 받은 GW ID, Device ID를 해지 한다.
 *@warning GW 등록 절차가 진행되어야만 Device 등록 절차를 진행 할 수 있다. GW 등록 해제 절차를 진행할 경우 하위 Device에 대해 자동 해지가 된다.
 *@warning OMP 연동을 통해 서비스를 재개하기 위해서는 다시 OMP를 통해 Auth Key, GW ID를 할당 받아야 한다.
 * @param pszGWID OMP로 제공 받은 GW ID값.
 * @param pszDeviceID OMP로 제공 받은 Device ID값.
 * @return 성공 : GMMMP_SUCCESS, 실패 : @ref ErrorCode.h 참조
 */
int GO_DeReg(const char* pszGWID,
		const char* pszDeviceID);


/**
 * @defgroup GMMP_Encryption_Operation M2M GW/Device 주기 보호 암호화
 M2M GW가 송신하는 주기보고의 데이터가 암호화 되어 전달되기 원하는 서비스에 대해서 GMMP 에서는 M2M GW/Device 주기 보호 암호화 기능을 제공한다.\n
해당 절차는 주기보고의 데이터가 암호화 되어 전달되기 원하는 서비스의 M2M GW만 수행하며, 암호화를 수행할 필요가 없는 서비스의 M2M GW는 주기 보호 암호화 기능을 수행하지 않는다.\n
암호화 알고리즘은 AES(128,192,256), SEED(128,256)을 지원하며, 주기보고 데이터를 암호화 해서 OMP로 전송하는 경우에는 GMMP Header의 Reserved Field 1 파라미터를 0x01로 설정해야 한다.\n
암호화 여부나 암호화 알고리즘 정보는 @ref GO_EncInfo 메시지를 이용하여 OMP로부터 정보를 획득한다.\n
암호화 수행 시 사용하는 키는 M2M GW에서 생성/변경 하며 @ref GO_EncKey 메시지를 이용하여 OMP 에 설정할 수 있다.\n

전송 패킷 : @ref Struct_Encrypt.h 참조

 * @ingroup GMMP_Encryption_Operation
 * @{
 */

/**
  * @brief 주기 보고 데이터를 암호화 할지 여부를 서버에 질의 한다.
 * @param pszGWID GW ID
 * @param pszDeviceID Device ID
 * @return 성공 : GMMMP_SUCCESS, 실패 : @ref ErrorCode.h 참조
 */
int GO_EncInfo(const char* pszGWID,
		const char* pszDeviceID);
/**
 * @brief 주기 보도 데이터를 암호화 사용 시 암호화 방식에 따른 암호화 키 값을 서버에 등록한다.
 * @param pszGWID GW ID
 * @param pszDeviceID Device ID
 * @param pszEncryptKey 사용자에 의해 생성된 암호화 키
 * @return 성공 : GMMMP_SUCCESS, 실패 : @ref ErrorCode.h 참조
 */
int GO_EncKey(const char* pszGWID,
		const char* pszDeviceID,
		const char* pszEncryptKey);
/**
 * @}
 */
/**
 * @}
 */


/**
 * @defgroup GMMP_ProfileInfo_Operation M2M GW/Device Profile Info
 *
M2M GW에 Profile 정보가 없는 경우에 또는 일부 존재 하지 않는 경우 OMP에 Profile 정보를 요청한다.\n

전송 패킷 : @ref Struct_ProfileInfo.h 참조

 * @ingroup GMMP_ProfileInfo_Operation
 * @{
 */
/**
 * @param pszGWID OMP로 제공 받은 GW ID값.
 * @param pszDeviceID OMP로 제공 받은 Device ID값.
 * @param nTID T-ID는 M2M GW와 OMP 간의 트랜잭션을 구분하기 위한 값으로서 Request 요청하는 곳에서 할당하며 Response 받은 곳에서는 해당 T-ID 값을 그대로 반환한다.\n
				M2M GW와 OMP에서 T-ID 사용 범위는 다음과 같다.\n
				※ M2M GW T-ID : 0 ~ 99,999 (10만개)\n
				※ OMP T-ID : M2M GW가 사용 이외의 값\n
 *
 * @return 성공 : GMMMP_SUCCESS, 실패 : @ref ErrorCode.h 참조
 */
int GO_Profile(const char* pszGWID,
		const char* pszDeviceID,
		const int nTID);
/**
 * @}
 */

/**
 * @defgroup GMMP_Delivery_Operation M2M GW/Device 주기 보고
 *
M2M 단말에서 수집된 데이터를 OMP로 전송하는 절차이다.\n
수집된 데이터를 전송하는 M2M 단말은 반드시 등록 절차를 통해 OMP에 등록되어 있어야 한다.\n
수집된 데이터는 타입에 따라서 수집 데이터(collect data), 장애 데이터(alarm data), 이벤트 데이터(event data), 장애 해제(alarm clear) 데이터 등으로 분류 된다.\n
이 값에 대한 설정은 cReportType 변수의 Report Type에 따라 구분된다. @ref Define_Delivery.h 참조

전송 패킷 : @ref Struct_Delivery.h 참조

 * @ingroup GMMP_Delivery_Operation
 * @{
 */
/**
 *
 * @param pszGWID OMP로 제공 받은 GW ID값.
 * @param pszDeviceID OMP로 제공 받은 Device ID값.
 * @param cReportType Report Type\n @ref Define_Delivery.h 참조
 *
 * @param cMediaType Message Body의 미디어 타입을 의미 @ref Struct_Delivery.h 참조
 * @param pszMessageBody Data[2048 Byte]
 * @param nTotalCount Message Body에 전달될 내용이 2048 Bytes를 초과할 경우 여러 개의 메시지로 전송하며 전체 메시지 개수를 표시 한다
 * @param nCurrentCount 여러 개의 메시지로 전송되는 경우 현재 메시지의 순서로서 1에서 Total Count까지의 값이 기록 된다.
 * @param cEncryption 암호화 데이터 유무 판단 Flag @ref GMMP_Encryption_Operation 참조
  * @return 성공 : GMMMP_SUCCESS, 실패 : @ref ErrorCode.h 참조
 */
int GO_Delivery(const char* pszGWID,
		const char* pszDeviceID,
		const char cReportType,
		const char cMediaType,
		const char* pszMessageBody,
		int nTotalCount,
		int nCurrentCount,
		const char cEncryption);
/**
 * @}
 */
/**
 * @defgroup GMMP_Control_Operation M2M GW/Device 단말 제어
 * M2M 단말에 대한 특정 제어 명령을 전달하기 위한 절차

전송 패킷 : @ref Struct_Control.h @ref Struct_Control_Opt.h참조

 * @ingroup GMMP_Control_Operation
 * @{
 */
/**
 *
 * @param pszGWID OMP로 제공 받은 GW ID값.
 * @param pszDeviceID OMP로 제공 받은 Device ID값.
 * @param nTID T-ID는 M2M GW와 OMP 간의 트랜잭션을 구분하기 위한 값으로서 Request 요청하는 곳에서 할당하며 Response 받은 곳에서는 해당 T-ID 값을 그대로 반환한다.\n
				M2M GW와 OMP에서 T-ID 사용 범위는 다음과 같다.\n
				※ M2M GW T-ID : 0 ~ 99,999 (10만개)\n
				※ OMP T-ID : M2M GW가 사용 이외의 값\n
 * @param cControlType Data Collect Period & Time Setting , Reboot, Configuration, Time Sync, Change Server IP/Port(OMP), HB Period, etc\n
 * 				@ref Define_Operation.h 참조
 * @param cResultCode Result Code (0x00 : Success, other : Fail Reason)\n
			@ref OMPErrorCode.h 참조
 *
 * @return 성공 : GMMMP_SUCCESS, 실패 : @ref ErrorCode.h 참조
 */
 int GO_Control(const char* pszGWID,
 		const char* pszDeviceID,
 		int nTID,
 		const char cControlType,
 		const char cResultCode);
/**
 * @defgroup GMMP_Notification_Operation M2M M2M GW/Device 제어 결과 보고
 전송 패킷 : @ref Struct_Notification.h 참조
 * 제어 결과를 플랫폼으로 통보하고 그에 대한 결과를 수신한다.
 */
/**
* @ingroup GMMP_Notification_Operation
* @{
*/
/**
 *
  * @param pszGWID OMP로 제공 받은 GW ID값.
 * @param pszDeviceID OMP로 제공 받은 Device ID값.
 * @param cControlType 단말 제어 메시지에 수신한 값과 동일한 값을 사용한다
 * @param cResultCode Result Code (0x00 : Success, other : Fail Reason)
 * @param pszMessageBody Data[2048 Byte]
 * @param nMessageBodySize pszMessageBody의 크기
 * @return 성공 : GMMMP_SUCCESS, 실패 : @ref ErrorCode.h 참조
 */
int GO_Notifi(const char* pszGWID,
		const char* pszDeviceID,
		const char cControlType,
		const char cResultCode,
		const char* pszMessageBody,
		const int nMessageBodySize);
/**
 * @}
 */

/**
 * @}
 */

/**
 * @defgroup GMMP_Heartbeat_Operation M2M GW/Device Heartbeat
 TCP Always On 단말인 경우 해당 세션을 감시하기 위해 주기적으로 Heartbeat 메시지를 전송한다.\n
 M2M 단말은 Profile 정보의 Heartbeat Period 값을 주기로 전송하며, 0인 경우에는 세션을 유지하지 않는다.

 전송 패킷 : @ref Struct_Heartbeat.h 참조

 * @ingroup GMMP_Heartbeat_Operation
 * @{
 */

/**
 *
 * @param pszGWID GW ID
 * @return 성공 : GMMMP_SUCCESS, 실패 : @ref ErrorCode.h 참조
 */
int GO_HB(const char* pszGWID);
/**
 * @}
 */


/**
 * @defgroup GMMP_LSentence_Operation M2M GW/Device Long Sentence
M2M GW/Device 제어 수신(@ref GMMP_Control_Operation)메시지의  ControlType이 User-Defined Control(@ref Define_Control.h) 타입일 경우 본 Long Sentence의 절차를 따른다.\n
SMS 를 통한 제어는 최대 80byte 까지 전송가능하며, 이중 헤더정보를 제외하면 제어메시지에 포함되는 Body 의 길이는 38byte 로 제한된다.\n
따라서 38byte를 초과하는 제어 데이터를 단말에게 전송해야 하는 경우에는  Long Sentence 기능을 이용한다.

 전송 패킷 : @ref Struct_LSentence.h 참조

 * @ingroup GMMP_LSentence_Operation
 * @{
 */

/**
 *
 * @param pszGWID OMP로 제공 받은 GW ID값.
 * @param pszDeviceID OMP로 제공 받은 Device ID값.
 * @param cControlType 단말 제어 메시지에 수신한 값과 동일한 값을 사용한다
 * @return 성공 : GMMMP_SUCCESS, 실패 : @ref ErrorCode.h 참조
 */
int GO_LSentence(const char* pszGWID,
		const char* pszDeviceID,
		const char cControlType);
/**
 * @}
 */


/**
 * @defgroup GMMP_FTP_Operation M2M GW/Device FTP Info
 * M2M GW/Device 제어 수신(@ref GMMP_Control_Operation)메시지의  ControlType이 F/W Download, F/W Update(@ref Define_Control.h) 타입일 경우 본 FTP Info의 절차를 따른다.\n

 전송 패킷 : @ref Struct_FTP.h 참조

 * @ingroup GMMP_FTP_Operation
 * @{
 */
/**
 *
 * @param pszGWID OMP로 제공 받은 GW ID값.
 * @param pszDeviceID OMP로 제공 받은 Device ID값.
 * @param cControlType 단말 제어 메시지에 수신한 값과 동일한 값을 사용한다
 * @return 성공 : GMMMP_SUCCESS, 실패 : @ref ErrorCode.h 참조
 */
int GO_FTP(const char* pszGWID,
		const char* pszDeviceID,
		const char cControlType);
/**
 * @}
 */


/**
 * @defgroup GMMP_Remote_Operation M2M GW/Device Remote Access Info
특정 M2M 단말에 원격으로 직접 접속하여 단말 제어를 위한 다양한 명령을 수행하거나 M2M단말 하위의 M2M Device(Metering 장비 등) 의\n
제어/진단 등의 기능을 지원하기 위해서 GMMP 에서는 Remote Access 기능을 제공한다.\n


전송 패킷 : @ref Struct_Remote.h 참조

 * @ingroup GMMP_Remote_Operation
 * @{
 */

/**
 *
 * @param pszGWID OMP로 제공 받은 GW ID값.
 * @param pszDeviceID OMP로 제공 받은 Device ID값.
 * @param cControlType 단말 제어 메시지에 수신한 값과 동일한 값을 사용한다
 * @return 성공 : GMMMP_SUCCESS, 실패 : @ref ErrorCode.h 참조
 */
int GO_Remote(const char* pszGWID,
		const char* pszDeviceID,
		const char cControlType);
/**
 * @}
 */


/**
 * @defgroup GMMP_Multimedia_Operation M2M GW/Device Multimedia Control
 * GMMP는 멀티미디어 서버를 경유하여 통신을 수행하는 Operation을 지원하기 위해서 Multimedia Control기능을 제공한다.\n
 *
전송 패킷 : @ref Struct_Multimedia.h 참조

 * @ingroup GMMP_Multimedia_Operation
 * @{
 */
/**
 *
 * @param pszGWID OMP로 제공 받은 GW ID값.
 * @param pszDeviceID OMP로 제공 받은 Device ID값.
 * @param cControlType 단말 제어 메시지에 수신한 값과 동일한 값을 사용한다
 * @return 성공 : GMMMP_SUCCESS, 실패 : @ref ErrorCode.h 참조
 */
int GO_MM(const char* pszGWID,
		const char* pszDeviceID,
		const char cControlType);
/**
 * @}
 */


/**
 * @defgroup GMMP_LOB_Operation M2M GW/Device LOB Upload
큰 사이즈의 주기보고 데이터(LOB : Large Object) 를 OMP 플랫폼이 아닌 외부 서버(Cloud 서버, FTP 서버) 에 업로드 하고,
이후 고객사가 외부 서버에 접속 하여 주기보고 한 큰 사이즈의 데이터를 조회 할 수 있는 Operation 을 지원하기 위해서 M2M GW/Device LOB Upload 기능을 제공한다.

전송 패킷 : @ref Struct_LOB.h 참조

 * @ingroup GMMP_LOB_Operation
 * @{
 */

/**
 *
 * @param pszGWID OMP로 제공 받은 GW ID값.
 * @param pszDeviceID OMP로 제공 받은 Device ID값.
 * @param cLOBType Cloud 접속 정보 요청 (GMMP_LOB_CLOUD:0x01), FTP 접속 정보 요청(GMMP_LOB_FTP:0x02)
 * @return 성공 : GMMMP_SUCCESS, 실패 : @ref ErrorCode.h 참조
 */
int GO_LOB(const char* pszGWID,
		const char* pszDeviceID,
		const char  cLOBType);
/**
 *
 * @param pszGWID OMP로 제공 받은 GW ID값.
 * @param pszDeviceID OMP로 제공 받은 Device ID값.
 * @param cStorageType LOB Storage 형태 (0x01 : Cloud, 0x02 : FTP)
 * @param pszLOBPath LOB Upload 디렉토리 경로
 * @return 성공 : GMMMP_SUCCESS, 실패 : @ref ErrorCode.h 참조
 */
int GO_LOB_Notifi(const char* pszGWID,
		const char* pszDeviceID,
		const char cStorageType,
		const char* pszLOBPath);

/**
 * @}
 */

int OG_Reg_Recv(GMMPHeader* pstGMMPHeader, stGwRegistrationRspHdr* pstGWBody, stDeviceRegistrationRspHdr* pstDeviceBody);
int OG_DeReg_Recv(GMMPHeader* pstGMMPHeader, stGwDeRegistrationRspHdr* pstGWBody, stDeviceDeRegistrationRspHdr* pstDeviceBody);
int OG_Profile_Recv(GMMPHeader* pstGMMPHeader, stProfileRspHdr* pstBody);
int OG_Delivery_Recv(GMMPHeader* pstGMMPHeader, stPacketDeliveryRspHdr* pstBody);
int OG_Ctrl_Recv(GMMPHeader* pstGMMPHeader, stControlReqHdr* pstBody);
int OG_HB_Recv(GMMPHeader* pstGMMPHeader, stHeartBeatMsgRspHdr* pstBody);
int OG_LSentence_Recv(GMMPHeader* pstGMMPHeader, stLSentenceRspHdr* pstBody);
int OG_FTP_Recv(GMMPHeader* pstGMMPHeader, stFtpInfoRspHdr* pstBody);
int OG_Remote_Recv(GMMPHeader* pstGMMPHeader, stRemoteAccessInfoRspHdr* pstBody);
int OG_Notifi_Recv(GMMPHeader* pstGMMPHeader, stNotificationRspHdr* pstBody);
int OG_Enc_Recv(GMMPHeader* pstGMMPHeader, stEncryptionInfoRspHdr* pstInfoBody, stEncryptionKeyRspHdr* pstKeyBody);
int OG_MM_Recv(GMMPHeader* pstGMMPHeader, stMultimediaURLInfoRspHdr* pstBody);
int OG_LOB_Recv(GMMPHeader* pstGMMPHeader, stLOBCloudAccessInfoRspHdr* pstCloudBody, stLOBFTPAccessInfoRspHdr* pstFTPBody);
int OG_LOB_Notifi_Recv(GMMPHeader* pstGMMPHeader, stLOBUploadNotificationRspHdr* pstBody);

/**
 * @brief GMMP 라이브러리 초기화 및 기본 정보 입력
 * @param pszServerIP 접속할 서버 IP, @ref g_szServerIP에 저장된다,
 * @param nPort 접속할 서버 port, @ref g_nServerPort에 저장된다.
 * @param pszDomainCode OMP Portal을 통해 등록된 서비스 코드, @ref g_szDomainCode에 저장된다,
 * @param pszGWAuthID M2M GW 의 Serial Number 값이며, MSISDN 또는 MAC Address값을 사용할 수도 있다. @ref g_szAuthID에 저장된다.
 * @param nGMMPMode 로그 저장 유무, GMMP_OFF_LOG, GMMP_ON_LOG
 * @param nErrorLevel	로그 출력 레벨
 * @param nNetwrokType 통신 타입
 * @param pszLogFileName 로그 파일 이름
 * @return 성공 : GMMP_SUCCESS, 실패: 1이상 , 에러코드 참조
 */
int Initializaion(const char* pszServerIP,
		const int nPort,
		const char* pszDomainCode,
		const char* pszGWAuthID,
		const int nGMMPMode,
		const int nErrorLevel,
		const int nNetwrokType,
		const char* pszLogFileName);

/**
* @brief GMMP 라이브러리 UnInitializaion
*/
void UnInitializaion();

/**
 * @brief TCP Always On 에서 사용하는 함수
 * Thread를 생성 하고 이 함수를 연결하여 사용한다.
 * @param pstGMMPHeader @ref GMMPHeader 구조체 포인트
 * @param pBody Body 메시지 구조체 포인트
 * @return 성공 : GMMMP_SUCCESS, 실패 : @ref ErrorCode.h 참조
 */
int GetReadData(GMMPHeader* pstGMMPHeader, void** pBody);

/**
 *char 타입의 데이터를  int로 변환한다.
 * @param pBuffer in
 * @param nSize in pBuffer size
 * @return int
 */
int Char2int(char* pBuffer, int nSize);

/**
 *char 타입의 데이터를  short로 변환한다.
 * @param pBuffer
 * @param nSize
 * @return
 */
int Char2short(char* pBuffer, short nSize);
/**
 * 현재 시간을 복사한다.
 * @param pBuffer 복사 받을 버퍼
 */
void GetNowTime(char* pBuffer);
/**
 *@fn void InitMemory();
 *@brief 내부 변수 초기화
 */
void InitMemory();

/**
 * @brief OMP로 제공 받은 AuthKey값을 저장하는 함수
 * @param pszAuthKey OMP로 부터 제공 받은 Auth Key 값
 */
void SetAuthKey(const char* pszAuthKey);
/**
 @brief 저장된  AuthKey
 * @return 저장된 Auth Key
 */
char* GetAuthKey();


/**
 * @brief OMP로 제공 받은 GW ID값을 저장하는 함수
 * @param pszGWID OMP로 부터 제공 받은 GW ID 값
 */
void SetGWID(const char* pszGWID);

/**
 *
 * @return 저장된 GW ID
 */
char* GetGWID();

/**
 *@brief 서버 정보 및 GW Auth ID, DomainCode를 저장한다.
 * @param pszServerIP Server IP
 * @param nPort Server Port
 * @param pszAuthID Auth ID
 * @param pszDomainCode DomainCode
 * @return 성공 : GMMP_SUCCESS, 실패 : LIB_PARAM_ERROR
 */
int SetServerInfo(const char* pszServerIP, int nPort, const char* pszAuthID, const char* pszDomainCode);

/**
 *@brief 주기 보고 암호화 여부 저장
 * @param nEncryption 0 or 1
 */
void SetEncryption(int nEncryption);

/**
 *@brief 저장된 주기 보고 암호화 여부 기본값  0
 * @return 전역 변수 g_bIsEncryptionContinue 리턴
 */
int GetEncryption();


/**
 * TCP_Always_On 모드와 TCP_Always_Off 모드 설정
 * @param nType @ref Define.h의 GMMP_NETWORK_ALYWAYS_ON/GMMP_NETWORK_ALYWAYS_OFF 참조
 */
void SetNetworkType(int nType);
/**
 *
 * @return NetwortType
 */
int GetNetworkType();

/**
 *@brief OMP서버로 부터 수신한 패킷을 제공할 콜백함수 저장 함수
 * @param pCallFunctionName 콜백을 제공 받을 함수 포인트
 */
void SetCallFunction(int (* pCallFunctionName)(GMMPHeader* pstGMMPHeader, void* pstBody));

#endif /* GMMP_H_ */
