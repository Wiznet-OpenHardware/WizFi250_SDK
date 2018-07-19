#ifndef WX_S2W_FUNCTION_H
#define WX_S2W_FUNCTION_H

//////////////////////////////////////////////////////////////////////////////////////////////////
// sekim 20140710 ID1180 add air command
extern uint8_t g_aircmd_status;
extern uint32_t g_aircmd_rx_len;
extern char* g_aircmd_tx_data;
extern char* g_aircmd_rx_data;
extern wiced_thread_t thread_handle_aircommand;
//////////////////////////////////////////////////////////////////////////////////////////////////

INT32 WXS2w_VPrintf(const char *format, va_list ap);
VOID W_RSP(const char *format, ...);
VOID W_EVT(const char *format, ...);
VOID W_DBG(const char *format, ...);
VOID W_DBG2(const char *format, ...);

void process_air_command_rx_data();

UINT8 WXCmd_MAIRCMD(UINT8 *ptr);

#endif
