#ifndef WX_S2W_PROCESS_H
#define WX_S2W_PROCESS_H

#define WX_MODE_COMMAND	0
#define WX_MODE_DATA	1


extern WT_PROFILE g_wxProfile;
extern UINT8 g_isAutoconnected;
extern UINT8 g_wxModeState;
extern UINT8 g_currentScid;

extern UINT8 g_scTxBuffer[WX_DATABUFFER_SIZE];
extern UINT32 g_scTxIndex;

/////////////////////////////////////////////////////////////////////////////
// sekim 20130403 Add Task Monitor, WXS2w_SerialInput
#include "wiced_management.h"
#include "wiced_time.h"
#define DEFAULT_NUMBER_OF_SYSTEM_MONITORS    (5)
#define MAXIMUM_ALLOWED_INTERVAL_BETWEEN_WIZFIMAINTASK (30000*MILLISECONDS)
extern wiced_system_monitor_t* system_monitors[DEFAULT_NUMBER_OF_SYSTEM_MONITORS];
extern wiced_system_monitor_t wizfi_task_monitor_item;
extern uint32_t wizfi_task_monitor_index;
extern uint32_t wizfi_task_monitor_stop;
/////////////////////////////////////////////////////////////////////////////

VOID WXS2w_StatusNotify(UINT8 status, UINT32 arg);

UINT8 WXS2w_DataBufferTransmit(VOID);

VOID WXS2w_LoadConfiguration(VOID);
VOID WXS2w_Initialize(VOID);
VOID WXS2w_Process(UINT8 *cmd);
VOID WXS2w_CommandCharProcess(UINT8 ch);
VOID WXS2w_DataCharProcess(UINT8 ch);
VOID WXS2w_SerialInput(VOID);
VOID WXS2w_LEDIndication(UINT8 ledType, UINT8 init, UINT8 repeat, UINT8 delay1, UINT8 delay2, UINT8 on);

#if 1	// kaizen 20130520 ID1039 - When Module Reset, Socket close & Disassociation
VOID WXS2w_SystemReset();
#endif

#if 1 // kaizen 20130814 Modified AT+FGPIO function
VOID WXS2w_GPIO_Init(VOID);
#endif

void send_maincommand_queue(uint16_t queue_id, uint16_t queue_opt);

#endif
