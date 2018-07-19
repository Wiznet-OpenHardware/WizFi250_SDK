#ifndef WX_PLATFORM_RTOS_SOCKET_H
#define WX_PLATFORM_RTOS_SOCKET_H

extern WT_SCLIST g_scList[WX_MAX_SCID_RANGE];
extern wiced_thread_t g_socketopen_thread[WX_MAX_SCID_RANGE];
extern UINT8 g_scRxBuffer[WX_DATABUFFER_SIZE];

// sekim 20140929 ID1188 Data-Idle-Auto-Reset (Autonix)
extern uint32_t g_time_lastdata;

void WXNetwork_Initialize();
void* WXNetwork_WicedV4IPToString(char* pIP, wiced_ip_address_t wiced_ip);

UINT8 WXNetwork_ClearSCList(UINT8 scid);
UINT8 WXNetwork_CloseSCList(UINT8 scid);
UINT8 WXNetwork_CloseSCAllList();
UINT8 WXNetwork_NetIsCidOpen(UINT8 scid);
UINT8 WXNetwork_ScidGet(VOID);
#if 0 //MikeJ 130806 ID1112 - Target IP/Port shouldn't be changed when packet was received on UDP Client
UINT8 WXNetwork_NetRx(UINT8 scid, VOID *buf, UINT32 len);
#else
UINT8 WXNetwork_NetRx(UINT8 scid, VOID *buf, UINT32 len, wiced_ip_address_t* rmtIP, UINT16* rmtPort);
#endif
UINT8 WXNetwork_NetTx(UINT8 scid, VOID *buf, UINT32 len);

void SocketOpenProcess(uint32_t arguments);
void LaunchSocketOpen(UINT8 scid, UINT8 bTCP, UINT8 bServer, UINT8 tlsMode, wiced_ip_address_t remote_ip, UINT16 remote_port, UINT16 local_port, UINT8 bDataMode);

void check_tcp_idle_time();

#endif

