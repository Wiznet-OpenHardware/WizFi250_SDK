// kaizen works
#ifndef WX_COMMANDS_F_H
#define WX_COMMANDS_F_H

UINT8 WXCmd_FPING	(UINT8 *ptr);
UINT8 WXCmd_FDNS	(UINT8 *ptr);
UINT8 WXCmd_FWEBS	(UINT8 *ptr);
UINT8 WXCmd_FWEBM	(UINT8 *ptr);
UINT8 WXCmd_FOTA	(UINT8 *ptr);	// kaizen 20130704 ID1081 Added OTA Command
UINT8 WXCmd_FGPIO	(UINT8 *ptr);	// kaizen 20130704 ID1097 Added GPIO Control Function
UINT8 WXCmd_FSOCK	(UINT8 *ptr);

UINT8 WXCmd_S2WEB	(UINT8 *ptr);
UINT8 WXCmd_SMODE	(UINT8 *ptr);

UINT8 WXCmd_FWEBSOPT(UINT8 *ptr);	// kaizen 20140408 ID1154, ID1166 Add AT+FWEBSOPT
UINT8 WXCmd_FGETADC(UINT8 *ptr);

UINT8 WXCmd_FFTPSET(UINT8 *ptr);
UINT8 WXCmd_FFTPCMD(UINT8 *ptr);

void Send_Ping(UINT32 loop_cnt, wiced_ip_address_t *ping_target_ip, wiced_interface_t interface );
void ap_sta_mode_webserver_thread();
void config_mode_webserver_thread();

#endif
