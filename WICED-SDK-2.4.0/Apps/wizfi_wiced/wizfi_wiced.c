/*
 * Copyright 2011, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

/**
 * @file
 *
 * This application provides a serial interface to access wireless
 * and networking functionality using the WICED evaluation board
 * and module. The application may be used to demonstrate usage of
 * the WICED Wi-Fi API or as a base for the development of a full-
 * featured serial-to-Wi-Fi application.
 *
 * The application is architected to enable easy addition of new
 * commands / functions
 *
 */

#include "wizfi_wiced.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "wizfimain/wx_types.h"

#define DELIMIT ((char*) " ")

#include "wizfimain/wx_defines.h"
#include "wizfimain/wx_platform_rtos_socket.h"
#include "wizfimain/wx_s2w_process.h"

#include "wiced_management.h"

#include "platform.h"

#if 1 //MikeJ - WIZnet Bootloader Verification
#include "bootloader_app.h"
#endif


#if 1	// kaizen 20130530 ID1075 - Added Function for Factory Default, Web Server Launch, Run OTA Mode
#include "wiced_platform.h"

wiced_thread_t check_gpio_thread;
static void check_gpio_thread_hndlr( uint32_t arg );
#endif


void app_main(void)
{
#if 1 //MikeJ - WIZnet Bootloader Verification
	uint8_t pass[11] = {0, 1, 0, 4, 1, 9, 5, 5, 7, 8, 1};
	if(bootloader_api->platform_verification(pass) != 0x37142850) return;
#endif

	/* turn off buffers, so IO occurs immediately */
	setvbuf(stdin, NULL, _IONBF, 0);
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);

#if 0  // kaizen 130411 ID1006 - Removed function call for Adjust USART information
	WXS2w_LoadConfiguration();
#endif

	//////////////////////////////////////////////////////////////////////////////////////////////////////
	// sekim 20150416 add AT+MEVTFORM, AT+MEVTDELAY for Coway
	//W_EVT("WizFi250 Version %s (WIZnet Co.Ltd)\r\n", WIZFI250_FW_VERSION);
	extern const char *g_wxCodeSimpleMessageList[];
	if ( g_wxProfile.event_msg_format[0]=='1' )			W_EVT("WizFi250 Version %s (WIZnet Co.Ltd)\r\n", WIZFI250_FW_VERSION);
	else if ( g_wxProfile.event_msg_format[0]=='2' )	W_EVT(g_wxCodeSimpleMessageList[0]);
	//////////////////////////////////////////////////////////////////////////////////////////////////////


	// sekim 20130224 Link UP/Down Callback
	wiced_network_register_link_callback(wifi_link_up_callback, wifi_link_down_callback);

	// sekim 20140214 ID1163 Add AT+WANT
	if ( wiced_wifi_select_antenna(g_wxProfile.antenna_type)!=WICED_SUCCESS )
	{
		W_DBG("Antenna configure error(%d)", g_wxProfile.antenna_type);
	}

	// sekim 20150107 added WXCmd_WRFMODE
	if ( wwd_wifi_set_interference_mode(g_wxProfile.rfmode1)!=WICED_SUCCESS )
	{
		W_DBG("Set Interference mode error(%d)", g_wxProfile.rfmode1);
	}

	// sekim 20140710 ID1179 add wcheck_option
	if ( g_wxProfile.wcheck_option1>0 )
	{
		extern uint8_t g_wcheck_status;
		if ( g_wcheck_status==0 )
		{
			wifi_check_process();
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////
	// sekim app_main
	// sekim 20141117 ID1191 URIEL Factory Default and AP mode & Web with a single-button-click
	//WXS2w_LEDIndication(0, 1, 3, 5, 50, 0);
	if( CHKCUSTOM("URIEL") && g_wxProfile.gpio_wifi_auto_conn==1 )
	{
	}
	else
	{
		WXS2w_LEDIndication(0, 1, 3, 5, 50, 0);
	}

	WXNetwork_Initialize();

	WXS2w_Initialize();

#if 1 // kaizen 20130814 Modified AT+FGPIO function
	WXS2w_GPIO_Init();
#endif

#if 1	// kaizen 20130530 ID1075 - Added Function for Factory Default, Web Server Launch, Run OTA Mode
	wiced_rtos_create_thread(&check_gpio_thread, WICED_APPLICATION_PRIORITY + 3, "check gpio thread", check_gpio_thread_hndlr, 4*1024, NULL);
#endif

	// sekim Autonix Auto-Reset
	g_time_lastdata = host_rtos_get_time();

	WXS2w_SerialInput();
	////////////////////////////////////////////////////////////////////////////////////////
}

#if 1	// kaizen 20130530 ID1075 - Added Function for Factory Default, Web Server Launch, Run OTA Mode
static const char* hexlist = "0123456789ABCDEF";
#define HEX_VAL( x )  hexlist[(int)((x) & 0xF)]

#include "wiced_dct.h"	// kaizen 20131118 ID1138 Solved Problem about strange operation when change AP mode using GPIO

#include "stm32f2xx_platform.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// sekim 20150902 Add recovery DCT function
void RecoveryDCT(WT_PROFILE* p_dct, int bBackup)
{
	W_DBG("RecoveryDCT Start (%d)", bBackup);
	if ( bBackup==1 )
	{
		p_dct->bckup_wifi_mode                 = p_dct->wifi_mode                             ;
		p_dct->bckup_wifi_dhcp                 = p_dct->wifi_dhcp                             ;
		memcpy(p_dct->bckup_ap_mode_ssid,        p_dct->ap_mode_ssid, WX_MAX_SSID_LEN+1)      ;
		p_dct->bckup_ap_mode_authtype          = p_dct->ap_mode_authtype                      ;
		p_dct->bckup_ap_mode_keylen            = p_dct->ap_mode_keylen                        ;
		memcpy(p_dct->bckup_ap_mode_keydata,     p_dct->ap_mode_keydata, 64)                  ;
		memcpy(p_dct->bckup_wifi_ip,             p_dct->wifi_ip, 16)                          ;
		memcpy(p_dct->bckup_wifi_mask,           p_dct->wifi_mask, 16)                        ;
		memcpy(p_dct->bckup_wifi_gateway,        p_dct->wifi_gateway, 16)                     ;
		memcpy(p_dct->bckup_scon_opt1,           p_dct->scon_opt1, 3)                         ;
		memcpy(p_dct->bckup_scon_opt2,           p_dct->scon_opt2, 4)                         ;
		memcpy(p_dct->bckup_scon_remote_ip,      p_dct->scon_remote_ip, 16)                   ;
		p_dct->bckup_scon_datamode             = p_dct->scon_datamode                         ;
		p_dct->bckup_scon_local_port           = p_dct->scon_local_port                       ;
		p_dct->bckup_scon_remote_port          = p_dct->scon_remote_port                      ;
	}
	else
	{
		p_dct->wifi_mode                       = p_dct->bckup_wifi_mode                       ;
		p_dct->wifi_dhcp                       = p_dct->bckup_wifi_dhcp                       ;
		memcpy(p_dct->ap_mode_ssid,              p_dct->bckup_ap_mode_ssid, WX_MAX_SSID_LEN+1);
		p_dct->ap_mode_authtype                = p_dct->bckup_ap_mode_authtype                ;
		p_dct->ap_mode_keylen                  = p_dct->bckup_ap_mode_keylen                  ;
		memcpy(p_dct->ap_mode_keydata,           p_dct->bckup_ap_mode_keydata, 64)            ;
		memcpy(p_dct->wifi_ip,                   p_dct->bckup_wifi_ip, 16)                    ;
		memcpy(p_dct->wifi_mask,                 p_dct->bckup_wifi_mask, 16)                  ;
		memcpy(p_dct->wifi_gateway,              p_dct->bckup_wifi_gateway, 16)               ;
		memcpy(p_dct->scon_opt1,                 p_dct->bckup_scon_opt1, 3)                   ;
		memcpy(p_dct->scon_opt2,                 p_dct->bckup_scon_opt2, 4)                   ;
		memcpy(p_dct->scon_remote_ip,            p_dct->bckup_scon_remote_ip, 16)             ;
		p_dct->scon_datamode                   = p_dct->bckup_scon_datamode                   ;
		p_dct->scon_local_port                 = p_dct->bckup_scon_local_port                 ;
		p_dct->scon_remote_port                = p_dct->bckup_scon_remote_port                ;
	}
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ProcessActionButtonClicked(uint32_t BtnClickCount )
{
	if ( BtnClickCount == 1 )
	{
		W_RSP("Set AP Mode & Web Server Launch\r\n");

		// kaizen 20131129 ID1153 Delete WLEAVE Procedure because WizFi250 can be restart
		//WXCmd_WLEAVE((UINT8*)"");
		//wiced_rtos_delay_milliseconds( 1 * SECONDS );

		wiced_mac_t my_mac;
		char ap_ssid[33] = {0, };
		char password[20] = {0,};

		// kaizen 20131118 ID1138 Solved Problem about strange operation when change AP mode using GPIO
		WT_PROFILE temp_dct;
		wiced_dct_read_app_section( &temp_dct, sizeof(WT_PROFILE) );

		///////////////////////////////////////////////////////////////////////////
		// sekim 20150902 Add recovery DCT function
		if ( g_wxProfile.mext1_1==11 )
		{
			RecoveryDCT(&temp_dct, 1);
			temp_dct.bckup_start = 1;
		}
		///////////////////////////////////////////////////////////////////////////


		// kaizen 20140410 ID1168 Customize for ShinHeung
		//if( strncmp((char*)g_wxProfile.custom_idx,"SHINHEUNG",sizeof("SHINHEUNG")) == 0 )
		if( CHKCUSTOM("SHINHEUNG") )
		{
			wiced_wifi_get_mac_address( &my_mac );

			strcpy( ap_ssid, "SAM4S_PRT_");
			int i;
			int pos = strlen(ap_ssid);
			for( i = 0; i < 6; i++ )
			{
				ap_ssid[pos+i*2]   = HEX_VAL((my_mac.octet[i] >> 4 ) );
				ap_ssid[pos+i*2+1] = HEX_VAL((my_mac.octet[i] >> 0 ) );
			}

			// Mode
			temp_dct.wifi_mode = AP_MODE;
			temp_dct.wifi_dhcp = 0;	// static ip

			// Set AP SSID			// kaizen 20140430 ID1172 Fixed bug about setting wrong SSID
			int ssid_len = strlen(ap_ssid);
			memcpy((char*)temp_dct.ap_mode_ssid,(char*)ap_ssid,ssid_len);
			temp_dct.ap_mode_ssid[ssid_len] = '\0';

			// Set AP Password
			temp_dct.ap_mode_authtype = WICED_SECURITY_WPA2_MIXED_PSK;
			strcpy((char*)password,"123456789");
			temp_dct.ap_mode_keylen = strlen(password);
			memcpy((char*)temp_dct.ap_mode_keydata,(char*)password,temp_dct.ap_mode_keylen);
			temp_dct.ap_mode_keydata[temp_dct.ap_mode_keylen] = '\0';

			// Set IP/Gateway/Subnet
			strcpy((char*)temp_dct.wifi_ip,(char*)"192.168.12.1");
			strcpy((char*)temp_dct.wifi_gateway,(char*)"192.168.12.1");
			strcpy((char*)temp_dct.wifi_mask,(char*)"255.255.255.0");

			temp_dct.gpio_wifi_auto_conn = 1;

			// Set Message Level
			temp_dct.msgLevel = 2;

			// Set SFORM
			memcpy(temp_dct.xrdf_main,"110001110", sizeof(temp_dct.xrdf_main));

			temp_dct.xrdf_data[0] = 0x7b;	temp_dct.xrdf_data[1] = 0x3b;	temp_dct.xrdf_data[2] = 0x7d;
			temp_dct.xrdf_data[3] = 0x00;	temp_dct.xrdf_data[4] = 0x0a;

			strcpy((char*)temp_dct.scon_opt1, (char*)"S");
			strcpy((char*)temp_dct.scon_opt2, (char*)DCTD_SCON_OPT2);
			strcpy((char*)temp_dct.scon_remote_ip, (char*)DCTD_SCON_REMOTE_IP);
			temp_dct.scon_datamode = DCTD_SCON_DATAMODE;
			temp_dct.scon_local_port = DCTD_SCON_LOCAL_PORT;
			temp_dct.scon_remote_port = DCTD_SCON_REMOTE_PORT;
			temp_dct.scon_datamode = COMMAND_MODE;

			// Set Serial
			temp_dct.usart_init_structure.USART_BaudRate 			= 460800;
			temp_dct.usart_init_structure.USART_WordLength          = USART_WordLength_8b;
			temp_dct.usart_init_structure.USART_Parity              = USART_Parity_No;
			temp_dct.usart_init_structure.USART_StopBits            = USART_StopBits_1;
			temp_dct.usart_init_structure.USART_HardwareFlowControl = USART_HardwareFlowControl_RTS_CTS;
			temp_dct.usart_init_structure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;

			wiced_dct_write_app_section( &temp_dct, sizeof(WT_PROFILE) );

			WXS2w_StatusNotify(WXCODE_SUCCESS, 0);
			WXS2w_SystemReset();
		}
		else
		{
			// sekim 20140715 URIEL Customizing
			if( CHKCUSTOM("URIEL") )
			{
				uint8_t buff_spi_stdio = g_wxProfile.spi_stdio;
				Default_Profile();
				Save_Profile();
				wiced_dct_read_app_section( &temp_dct, sizeof(WT_PROFILE) );
				temp_dct.spi_stdio = buff_spi_stdio;

				// sekim 20141117 ID1191 URIEL Factory Default and AP mode & Web with a single-button-click
				W_RSP("(with Factory Default)\r\n");
			}

			wiced_wifi_get_mac_address( &my_mac );

			strcpy( ap_ssid, "WizFi250_AP_");

			// sekim 20140929 ID1187 URIEL Customizing 2 : 9600, AT+WSTAT, W_AP_xxxxxxxx
			if( CHKCUSTOM("URIEL") )
			{
				strcpy( ap_ssid, "W_AP_");
			}
			// kaizen 20141024 ID1189 Modified in order to print LOGO image and title in web page depending on CUSTOM value.
			else if( CHKCUSTOM("ENCORED") )
			{
				strcpy( ap_ssid, "GetIT_");
			}

			int i;
			int pos = strlen(ap_ssid);
			for( i = 0; i < 6; i++ )
			{
				ap_ssid[pos+i*2]   = HEX_VAL((my_mac.octet[i] >> 4 ) );
				ap_ssid[pos+i*2+1] = HEX_VAL((my_mac.octet[i] >> 0 ) );
			}

			// Mode
			temp_dct.wifi_mode = AP_MODE;
			temp_dct.wifi_dhcp = 0;	// static ip

			// Set AP SSID			// kaizen 20140430 ID1172 Fixed bug about setting wrong SSID
			int ssid_len = strlen(ap_ssid);
			memcpy((char*)temp_dct.ap_mode_ssid,(char*)ap_ssid,ssid_len);
			temp_dct.ap_mode_ssid[ssid_len] = '\0';

			// Set AP Password
			temp_dct.ap_mode_authtype = WICED_SECURITY_WPA2_MIXED_PSK;
			strcpy((char*)password,"123456789");
			temp_dct.ap_mode_keylen = strlen(password);
			memcpy((char*)temp_dct.ap_mode_keydata,(char*)password,temp_dct.ap_mode_keylen);
			temp_dct.ap_mode_keydata[temp_dct.ap_mode_keylen] = '\0';

			// Set IP/Gateway/Subnet
			strcpy((char*)temp_dct.wifi_ip,(char*)"192.168.12.1");
			strcpy((char*)temp_dct.wifi_gateway,(char*)"192.168.12.1");
			strcpy((char*)temp_dct.wifi_mask,(char*)"255.255.255.0");

			temp_dct.gpio_wifi_auto_conn = 1;

			// kaizen 20131129 Set Default parameter to scon variable
			strcpy((char*)temp_dct.scon_opt1, (char*)DCTD_SCON_OPT1);
			strcpy((char*)temp_dct.scon_opt2, (char*)DCTD_SCON_OPT2);
			strcpy((char*)temp_dct.scon_remote_ip, (char*)DCTD_SCON_REMOTE_IP);
			temp_dct.scon_datamode = DCTD_SCON_DATAMODE;
			temp_dct.scon_local_port = DCTD_SCON_LOCAL_PORT;
			temp_dct.scon_remote_port = DCTD_SCON_REMOTE_PORT;

			wiced_dct_write_app_section( &temp_dct, sizeof(WT_PROFILE) );

			WXS2w_StatusNotify(WXCODE_SUCCESS, 0);
			WXS2w_SystemReset();
		}
	}
	else if ( BtnClickCount == 2 )
	{
		W_RSP("Set OTA Mode\r\n");
		wiced_start_ota_upgrade();			 // Function does not return
	}
	else if ( BtnClickCount == 3 )
	{
		W_RSP("Set Factory Default\r\n");
		WXCmd_MFDEF((UINT8*)"FR");			 // Function does not return
	}
	else
		W_RSP("Error\r\n");
}

static void check_gpio_thread_hndlr( uint32_t arg )
{
	uint32_t		ActionButtonClickCount = 0;
	uint32_t		nCountAfterClick = 0;
	uint32_t		ActionButtonBuff = 0;
	wiced_bool_t 	tempActionButton = 0;

	// sekim 20140430 ID1152 add scon_retry
	uint32_t 		nCoundLoop1 = 0, nCoundLoop2 = 0, nCoundLoop3 = 0, nCoundLoop4 = 0, nCoundLoop5 = 0;

	// sekim 20141117 ID1191 URIEL Factory Default and AP mode & Web with a single-button-click
	uint32_t		quicksetup_foundnotclick = 0;
	uint32_t		quicksetup_clickcount = 0;

	while(1)
	{
		tempActionButton = wiced_gpio_input_get(WICED_GPIO_3);

		// sekim 20141117 ID1191 URIEL Factory Default and AP mode & Web with a single-button-click
		if( CHKCUSTOM("URIEL") )
		{
			if ( tempActionButton==1 )
			{
				quicksetup_foundnotclick = 1;
			}

			if ( quicksetup_foundnotclick==1 )
			{
				if ( tempActionButton==0 )	quicksetup_clickcount++;
				else 						quicksetup_clickcount = 0;
				if ( quicksetup_clickcount > 20 )
				{
					send_maincommand_queue(105, 1);
					quicksetup_clickcount = 0;
				}
			}
		}
		/////////////////////////////////////////////////////////////////////////////
		// sekim 20141218 POSBANK Customizing
		//20150319 POSBANK Customizing Added(Recover Button-action)
		//else if( CHKCUSTOM("POSBANK") )
		else if( CHKCUSTOM("XXX-POSBANK") )
		{
			if ( tempActionButton==1 )
			{
				quicksetup_foundnotclick = 1;
			}

			if ( quicksetup_foundnotclick==1 )
			{
				if ( tempActionButton==0 )
				{
					quicksetup_clickcount++;
				}
				else
				{
					if ( quicksetup_clickcount>0 )
					{
						if ( quicksetup_clickcount>75 )
						{
							memset(g_wxProfile.custom_idx, 0, sizeof(g_wxProfile.custom_idx));
							strcpy((char*)g_wxProfile.custom_idx, (char*) "WIZNET" );
							send_maincommand_queue(105, 3);
						}
						else if ( quicksetup_clickcount>25 )
						{
							send_maincommand_queue(105, 3);
						}
						else if ( quicksetup_clickcount > 0 )
						{
							WXS2w_StatusNotify(WXCODE_SUCCESS, 0);
							WXS2w_SystemReset();
						}
					}
					quicksetup_clickcount = 0;
				}
			}
		}
		/////////////////////////////////////////////////////////////////////////////
		else
		{
			if( ActionButtonBuff != tempActionButton )
			{
				ActionButtonBuff = tempActionButton;
				if ( ActionButtonBuff == 0 )	{ ActionButtonClickCount++; nCountAfterClick = 0; }
			}

			if ( ActionButtonClickCount > 0 )
			{
				nCountAfterClick++;

				if ( nCountAfterClick > 5 )
				{
					// sekim 20140625 ID1178 move ProcessActionButtonClicked to main loop
					//ProcessActionButtonClicked( ActionButtonClickCount );
					send_maincommand_queue(105, ActionButtonClickCount);

					nCountAfterClick = 0;
					ActionButtonClickCount = 0;
				}
			}
		}

		wiced_rtos_delay_milliseconds( 100 * MILLISECONDS );

		//////////////////////////////////////////////////////////////////////////////////////////
		// sekim 20140430 ID1152 add scon_retry
		nCoundLoop1++;
		if ( g_wxProfile.socket_ext_option6>0 )
		{
			if ( nCoundLoop1 > (g_wxProfile.socket_ext_option6*10) )
			{
				send_maincommand_queue(103, 0);
				nCoundLoop1 = 0;
			}
		}
		//////////////////////////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////////////////////
		// sekim 20140508 double check Link Down (for WiFi Direct)
		static uint8_t wifi_status_error = 0;
		extern uint8_t g_wifi_linkup;
		extern UINT8 g_used_wp2p;
		nCoundLoop2++;
		if ( (nCoundLoop2>30) || (wifi_status_error>0) )
		{
			if ( g_wifi_linkup==1 )
			{
				if ( WXLink_IsWiFiLinked()==FALSE )
				{
					wifi_status_error++;
				}
				else
				{
					wifi_status_error = 0;
				}

				if ( wifi_status_error>10 )
				{
					// sekim 20140508 double check Link Down (for WiFi Direct)
					if ( g_used_wp2p==1 )	send_maincommand_queue(102, 0);

					wifi_status_error = 0;
				}
			}
			else
			{
				wifi_status_error = 0;
			}
			nCoundLoop2 = 0;
		}
		//////////////////////////////////////////////////////////////////////////////////////////


		//////////////////////////////////////////////////////////////////////////////////////////
		// sekim 20140625 ID1176 add option to clear tcp-idle-connection
		nCoundLoop3++;
		if ( nCoundLoop3>10 )
		{
			send_maincommand_queue(104, 0);
			nCoundLoop3 = 0;
		}
		//////////////////////////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////////////////////
		// sekim 20150508 Coway ReSend <Connect Event Message>
		nCoundLoop4++;
		if ( g_wxProfile.cowaya_check_time>0 )
		{
			if ( nCoundLoop4 > (g_wxProfile.cowaya_check_time*10) )
			{
				extern UINT8 g_cowaya_recv_checkdata;

				if ( g_isAutoconnected==1 )
				{
					if ( g_cowaya_recv_checkdata==0 )
					{
						WXS2w_StatusNotify(WXCODE_CON_SUCCESS, 0);
					}
				}
				nCoundLoop4 = 0;
			}
		}

		//////////////////////////////////////////////////////////////////////////////////////////

		//////////////////////////////////////////////////////////////////////////////////////////
		// sekim 20150519 add socket_ext_option8. Reset if not connected in TCP Client/Service
		if ( g_wxProfile.socket_ext_option8>0 )
		{
			if ( WXLink_IsWiFiLinked() )
			{
				nCoundLoop5++;
				if ( nCoundLoop5 > (g_wxProfile.socket_ext_option8*10) )
				{
					if ( strcmp((char*)g_wxProfile.scon_opt1, "SO") == 0 || strcmp((char*)g_wxProfile.scon_opt1, "S") == 0 )
					{
						uint8_t found_socket = 0;
						uint32_t index_scid = 0;
						for(index_scid = 0; index_scid < WX_MAX_SCID_RANGE; index_scid++)
						{
							if ( g_scList[index_scid].pSocket!=0 )
							{
								found_socket = 1;
								break;
							}
						}
						if ( found_socket==0 )
						{
							WXS2w_SystemReset();
							return;
						}
					}

					nCoundLoop5 = 0;
				}
			}
			else
			{
				nCoundLoop5 = 0;
			}
		}
		//////////////////////////////////////////////////////////////////////////////////////////

		// kaizen 20131126 Add SPI XON Procedure
		extern wiced_result_t wiced_check_spi_rx_status(void);
		wiced_check_spi_rx_status();

		// sekim 20131125 Add SPI Interface
		/*
		{
			extern char szXXXX2[1024];
			if ( szXXXX2[0] )
			{
				wiced_uart_transmit_bytes(0, (void*)szXXXX2, strlen(szXXXX2));
				wiced_uart_transmit_bytes(0, (void*)"\r\n", 2);
				szXXXX2[0] = 0;
			}
		}
		*/

#if 0		// kaizen 20131213 ID1141 Added set g_socket_extx_option3 to 0 in order to do not use this sequence.

		////////////////////////////////////////////////////////////////////////////////
		// sekim 20131212 ID1141 add g_socket_extx_option
		{
			extern uint32_t g_socket_extx_option3;
			extern uint32_t g_socket_critical_error;

			if ( g_socket_extx_option3>0 )
			{
				if ( g_socket_critical_error>=g_socket_extx_option3 )
				{
					g_socket_critical_error = 0;
					W_DBG("g_socket_extx_option3 overflow");
					WXCmd_SMGMT	((UINT8*)"ALL");
					WXCmd_WLEAVE((UINT8*)"");
				}
			}
		}
#endif
		////////////////////////////////////////////////////////////////////////////////
	}
}
#endif
