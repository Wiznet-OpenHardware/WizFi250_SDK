#ifndef WX_COMMANDS_W_H
#define WX_COMMANDS_W_H

void wifi_link_up_callback(void);
void wifi_link_down_callback(void);

// kaizen
//UINT8 WXLink_Disassociate(UINT8 bForced, UINT8 bNotify);
UINT8 WXLink_Disassociate(UINT8 bForced, UINT8 bNotify, wiced_interface_t interface);

UINT8 WXLink_IsWiFiLinked();

UINT8 WXCmd_WJOIN(UINT8 *ptr);
UINT8 WXCmd_WLEAVE(UINT8 *ptr);
UINT8 WXCmd_WSCAN(UINT8 *ptr);
UINT8 WXCmd_WSET(UINT8 *ptr);
UINT8 WXCmd_WSET2(UINT8 *ptr);
UINT8 WXCmd_WSET_TEMPforCoway(UINT8 *ptr);
UINT8 WXCmd_WSEC(UINT8 *ptr);
UINT8 WXCmd_WNET(UINT8 *ptr);
UINT8 WXCmd_WSTAT(UINT8 *ptr);
UINT8 WXCmd_WREG(UINT8 *ptr);
#if 1 //MikeJ 130702 ID1092 - ATCmd update (naming, adding comments)
UINT8 WXCmd_WWPS	(UINT8 *ptr);
#endif
#if 1	// kaizen 20130716 ID1092 - ATCmd update (naming, adding comments)
wiced_result_t Request_WPS( wiced_wps_mode_t mode, char * p_wps_pin_num );
#endif

UINT8 WXCmd_WADNS(UINT8 *ptr);
UINT8 WXCmd_WANT(UINT8 *ptr);

wiced_security_t scan_wifi_and_get_sectype(wiced_ssid_t* optional_ssid);

UINT8 GetAntennaType(UINT8 bAP);

UINT8 set_80211bgn_mode(UINT8 bgn_mode);
UINT8 WXCmd_WBGN(UINT8 *ptr);

#if 1		// kaizen 20140409 ID1156 Added Wi-Fi Direct Function
UINT8 WXCmd_WP2P_Start		(UINT8 *ptr);
UINT8 WXCmd_WP2P_Stop		(UINT8 *ptr);
UINT8 WXCmd_WP2P_PeerList	(UINT8 *ptr);
UINT8 WXCmd_WP2P_Invite		(UINT8 *ptr);
#endif

// sekim 20140710 ID1179 add wcheck_option
UINT8 WXCmd_WCHECK(UINT8 *ptr);
void wifi_check_process();

// sekim 20150107 added WXCmd_WRFMODE
UINT8 WXCmd_WRFMODE(UINT8 *ptr);


#endif
