/*
 * Copyright 2013, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

/** @file
 *
 */

#include <stdlib.h>
#include <string.h>
#include "wiced.h"
#include "http_server.h"
#include "bootloader_app.h"
#include "wwd_constants.h"
#include <wiced_utilities.h>
#include <resources.h>

// // kaizen 20130412 ID1042
#ifdef BUILD_WIZFI250
#include "stm32f2xx.h"
#include "wizfimain/wx_defines.h"
#include "platform_bootloader.h"
#include "wizfimain/wx_types.h"
#endif


/******************************************************
 *                      Macros
 ******************************************************/

#define CMP_MAC( a, b )  (((a[0])==(b[0]))&& \
                          ((a[1])==(b[1]))&& \
                          ((a[2])==(b[2]))&& \
                          ((a[3])==(b[3]))&& \
                          ((a[4])==(b[4]))&& \
                          ((a[5])==(b[5])))

#define NULL_MAC( a )  (((a[0])==0)&& \
                        ((a[1])==0)&& \
                        ((a[2])==0)&& \
                        ((a[3])==0)&& \
                        ((a[4])==0)&& \
                        ((a[5])==0))

/******************************************************
 *                    Constants
 ******************************************************/

#define SSID_FIELD_NAME            "ssid"
#define SECURITY_FIELD_NAME        "at0"
#define CHANNEL_FIELD_NAME         "chan"
#define BSSID_FIELD_NAME           "bssid"
#define PASSPHRASE_FIELD_NAME      "ap0"
#define PIN_FIELD_NAME             "pin"

#define APP_SCRIPT_PT1     "var elem_num = "
#define APP_SCRIPT_PT2     ";\n var labelname = \""
#define APP_SCRIPT_PT3     "\";\n var fieldname  = \"v"
#define APP_SCRIPT_PT4     "\";\n var fieldvalue = \""
#define APP_SCRIPT_PT5     "\";\n"


#if 1	// kaizen 20140408 ID1154, ID1166 Add AT+FWEBSOPT
	#define SCAN_SCRIPT_PT1    "var elem_num = "
	#define SCAN_SCRIPT_PT2    ";\n var SSID = \""
	#define SCAN_SCRIPT_PT3    "\";\n var RSSIstr  = \""
	#define SCAN_SCRIPT_PT4    "\";\n var SEC = "
	#define SCAN_SCRIPT_PT5    ";\n var SECstr = \""
	#define SCAN_SCRIPT_PT6    "\";\n var CH  = "
	#define SCAN_SCRIPT_PT7    ";\n var BSSID  = \""
	#define SCAN_SCRIPT_PT8    "\";\n"
#else
	#define SCAN_SCRIPT_PT1    "var elem_num = "
	#define SCAN_SCRIPT_PT2    ";\n var SSID = \""
	#define SCAN_SCRIPT_PT3    "\";\n var RSSIstr  = \""
	#define SCAN_SCRIPT_PT4    "\";\n var SEC = "
	#define SCAN_SCRIPT_PT5    ";\n var CH  = "
	#define SCAN_SCRIPT_PT6    ";\n var BSSID  = \""
	#define SCAN_SCRIPT_PT7    "\";\n"
#endif

/* Signal strength defines (in dBm) */
#define RSSI_VERY_POOR             -85
#define RSSI_POOR                  -70
#define RSSI_GOOD                  -55
#define RSSI_VERY_GOOD             -40
#define RSSI_EXCELLENT             -25
#define RSSI_VERY_POOR_STR         "Very Poor"
#define RSSI_POOR_STR              "Poor"
#define RSSI_GOOD_STR              "Good"
#define RSSI_VERY_GOOD_STR         "Very good"
#define RSSI_EXCELLENT_STR         "Excellent"

#define CAPTIVE_PORTAL_REDIRECT_PAGE \
    "<html><head>" \
    "<meta http-equiv=\"refresh\" content=\"0; url=/config/device_settings.html\">" \
    "</head></html>"

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/
typedef struct
{
    wiced_tcp_stream_t* stream;
    wiced_semaphore_t semaphore;
    int result_count;
} process_scan_data_t;

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

// sekim 20140716 ID1182 add s2web_main
#if BUILD_WIZFI250
static int process_s2web_main( const char* params, wiced_tcp_stream_t* stream, void* arg );
#endif

static int process_app_settings_page ( const char* url, wiced_tcp_stream_t* socket, void* arg );
static int process_wps_go            ( const char* url, wiced_tcp_stream_t* socket, void* arg );
static int process_scan              ( const char* url, wiced_tcp_stream_t* socket, void* arg );
static int process_connect           ( const char* url, wiced_tcp_stream_t* socket, void* arg );
static int process_config_save       ( const char* url, wiced_tcp_stream_t* stream, void* arg );
static int process_ota_go		     ( const char* url, wiced_tcp_stream_t* stream, void* arg );
static int process_s2web		     ( const char* url, wiced_tcp_stream_t* stream, void* arg );
static int process_config_s2w_value  ( const char* url, wiced_tcp_stream_t* stream, void* arg );	// kaizen 20130712 ID1099 For Setting TCP/UDP Connection using web server
static int process_config_s2w_ap_value  ( const char* url, wiced_tcp_stream_t* stream, void* arg );	// kaizen 20131113 ID1135 Added AP Mode Setting Using Web Server
static int process_config_s2w_reset		( const char* url, wiced_tcp_stream_t* stream, void* arg ); // kaizen 20131113 ID1135 Added AP Mode Setting Using Web Server
static int get_gpio_setting			 ( const char* url, wiced_tcp_stream_t* stream, void* arg );	// kaizen 20131030 ID1134 Added GPIO Control Page Using Web Server
static int set_gpio_setting			 ( const char* url, wiced_tcp_stream_t* stream, void* arg );	// kaizen 20131030 ID1134 Added GPIO Control Page Using Web Server
static int gpio_config_save			 ( const char* params, wiced_tcp_stream_t* stream, void* arg );	// kaizen 20131030 ID1134 Added GPIO Control Page Using Web Server
static int s2wlogin					 ( const char* params, wiced_tcp_stream_t* stream, void* arg );	// kaizen 20131104 ID1133 Added Web Login Procedure
static int process_s2wconfig_station ( const char* params, wiced_tcp_stream_t* stream, void* arg ); // kaizen 20131118 ID1139 Added Static IP Setting Page in Station Mode
static int get_info_ota				 ( const char* url, wiced_tcp_stream_t* stream, void* arg );	// kaizen 20131210 ID1147 Added function about getting mac address
static int process_serial_setting	 ( const char* params, wiced_tcp_stream_t* stream, void* arg );	// kaizen 20131210 ID1149 Added function about setting serial configuration
static int process_user_info_setting ( const char* params, wiced_tcp_stream_t* stream, void* arg );	// kaizen 20131210 ID1150 Added function in order to change user information as userID and userPW.


// kaizen 20140411 ID1169 Modified procedure for joining AP ( For ShinHeung )
static int process_scan_SH           ( const char* url, wiced_tcp_stream_t* stream, void* arg );

#ifdef BUILD_WIZFI250
// kaizen 20141024 ID1189 Modified in order to print LOGO image and title in web page depending on CUSTOM value.
static int process_get_custom_info    ( const char* params, wiced_tcp_stream_t* stream, void* arg );
#endif

/******************************************************
 *               Variables Definitions
 ******************************************************/

/**
 * URL Handler List
 */

#define slash_redirect "HTTP/1.0 301\r\nLocation: /config/device_settings.html\r\n\r\n"

// kaizen
START_OF_HTTP_PAGE_DATABASE(config_sta_http_page_database)
	//{ "/",                               "text/html", 						  WICED_STATIC_URL_CONTENT, 	.url_content.static_data  = {resource_config_DIR_main_html,    			  sizeof(resource_config_DIR_main_html)-1   			} },
	//{ "/",                               "text/html", 						  WICED_STATIC_URL_CONTENT, 	.url_content.static_data  = {resource_s2web_DIR_wizfi250_main_html,		  sizeof(resource_s2web_DIR_wizfi250_main_html)-1   	} },
	{ "/",                               "text/html", 						  WICED_STATIC_URL_CONTENT, 	.url_content.static_data  = {resource_s2web_DIR_index_html,		 		  sizeof(resource_s2web_DIR_index_html)-1   	} },

	{ "/s2web/s2w_main.html",      		 "text/html", 						  WICED_STATIC_URL_CONTENT, 	.url_content.static_data  = {resource_s2web_DIR_s2w_main_html,			  sizeof(resource_s2web_DIR_s2w_main_html)-1   	} },

	// sekim 20140716 ID1182 add s2web_main
#if BUILD_WIZFI250
	{ "/s2web_main",                   "text/html", 						  WICED_DYNAMIC_URL_CONTENT, 	.url_content.static_data  = {process_s2web_main,	        0 } },
#endif

	{ "/config/device_settings.html",    "text/html",                         WICED_DYNAMIC_URL_CONTENT,    .url_content.dynamic_data = {process_app_settings_page,     0 } },
    { "/config/join.html",               "text/html",                         WICED_STATIC_URL_CONTENT,     .url_content.static_data  = {resource_config_DIR_join_html,               sizeof(resource_config_DIR_join_html)-1               } },
    { "/config/scan_page_outer.html",    "text/html",                         WICED_STATIC_URL_CONTENT,     .url_content.static_data  = {resource_config_DIR_scan_page_outer_html,    sizeof(resource_config_DIR_scan_page_outer_html)-1    } },
	{ "/connect",                        "text/html",                         WICED_DYNAMIC_URL_CONTENT,    .url_content.dynamic_data = {process_connect,               0 } },
	{ "/scan_results.html",              "text/html",                         WICED_DYNAMIC_URL_CONTENT,    .url_content.dynamic_data = {process_scan,                  0 } },
	{ "/config_save",                    "text/html",                         WICED_DYNAMIC_URL_CONTENT,    .url_content.dynamic_data = {process_config_save,           0 } },
    { "/scripts/general_ajax_script.js", "application/javascript",            WICED_STATIC_URL_CONTENT,     .url_content.static_data  = {resource_scripts_DIR_general_ajax_script_js, sizeof(resource_scripts_DIR_general_ajax_script_js)-1 } },
    { "/scripts/wpad.dat",               "application/x-ns-proxy-autoconfig", WICED_STATIC_URL_CONTENT,     .url_content.static_data  = {resource_scripts_DIR_wpad_dat,               sizeof(resource_scripts_DIR_wpad_dat)-1               } },
    { "/images/wps_icon.png",            "image/png",                         WICED_STATIC_URL_CONTENT,     .url_content.static_data  = {resource_images_DIR_wps_icon_png,            sizeof(resource_images_DIR_wps_icon_png)              } },
    { "/images/scan_icon.png",           "image/png",                         WICED_STATIC_URL_CONTENT,     .url_content.static_data  = {resource_images_DIR_scan_icon_png,           sizeof(resource_images_DIR_scan_icon_png)             } },
    { "/images/favicon.ico",             "image/vnd.microsoft.icon",          WICED_STATIC_URL_CONTENT,     .url_content.static_data  = {resource_images_DIR_favicon_ico,             sizeof(resource_images_DIR_favicon_ico)               } },
	//{ "/images/brcmlogo_line.png",       "image/png",                         WICED_STATIC_URL_CONTENT,     .url_content.static_data  = {resource_images_DIR_brcmlogo_line_png,       sizeof(resource_images_DIR_brcmlogo_line_png)         } },
	{ "/styles/buttons.css",             "text/css",                          WICED_STATIC_URL_CONTENT,     .url_content.static_data  = {resource_styles_DIR_buttons_css,             sizeof(resource_styles_DIR_buttons_css)               } },
#if 1 // kaizen 20130716 Wiznet web page
	{ "/images/over_the_air_logo.png",   "image/gif",                         WICED_STATIC_URL_CONTENT,     .url_content.static_data  = {resource_images_DIR_over_the_air_logo_png,   sizeof(resource_images_DIR_over_the_air_logo_png)     } },
	{ "/images/wiznet_logo.png",		 "image/png",						  WICED_STATIC_URL_CONTENT,		.url_content.static_data  = {resource_images_DIR_wiznet_logo_png,		  sizeof(resource_images_DIR_wiznet_logo_png)			} },
	{ "/images/s2web_logo.png",		     "image/png",						  WICED_STATIC_URL_CONTENT,		.url_content.static_data  = {resource_images_DIR_s2web_logo_png,		  sizeof(resource_images_DIR_s2web_logo_png)			} },
	{ "/images/gpio_icon.png",		     "image/png",						  WICED_STATIC_URL_CONTENT,		.url_content.static_data  = {resource_images_DIR_gpio_icon_png,		 	 sizeof(resource_images_DIR_gpio_icon_png)			} },
	{ "/images/serial_setting_icon.png",	"image/png",					  WICED_STATIC_URL_CONTENT,		.url_content.static_data  = {resource_images_DIR_serial_setting_icon_png,   sizeof(resource_images_DIR_serial_setting_icon_png)		} },
	{ "/images/user_information_icon.png",	"image/png",					  WICED_STATIC_URL_CONTENT,		.url_content.static_data  = {resource_images_DIR_user_information_icon_png, sizeof(resource_images_DIR_user_information_icon_png)	} },
	{ "/s2web/wizfi_ota.html",           "text/html",                         WICED_STATIC_URL_CONTENT,     .url_content.static_data  = {resource_s2web_DIR_wizfi_ota_html,          sizeof(resource_s2web_DIR_wizfi_ota_html)-1          } },
	{ "/ota_go",                         "text/html",                         WICED_DYNAMIC_URL_CONTENT,    .url_content.dynamic_data = {process_ota_go,                 0 } },
	// kaizen 20131210 ID1147 Added function about getting mac address
	{ "/get_info_ota",                   "text/html",                         WICED_DYNAMIC_URL_CONTENT,    .url_content.dynamic_data = {get_info_ota,                   0 } },

    { "/s2web/wps_pbc.html",            "text/html",                          WICED_STATIC_URL_CONTENT,     .url_content.static_data  = {resource_s2web_DIR_wps_pbc_html,            sizeof(resource_s2web_DIR_wps_pbc_html)-1            } },
    { "/s2web/wps_pin.html",            "text/html",                          WICED_STATIC_URL_CONTENT,     .url_content.static_data  = {resource_s2web_DIR_wps_pin_html,            sizeof(resource_s2web_DIR_wps_pin_html)-1            } },
	{ "/wps_go",                         "text/html",                         WICED_DYNAMIC_URL_CONTENT,    .url_content.dynamic_data = {process_wps_go,                0 } },
	// sekim 20130423 serial-to-web demo
	{ "/s2web/demo.html",                "text/html",                         WICED_STATIC_URL_CONTENT,     .url_content.static_data  = {resource_s2web_DIR_demo_html,                sizeof(resource_s2web_DIR_demo_html)-1                } },
    { "/content.html",                   "text/html",                         WICED_DYNAMIC_URL_CONTENT,    .url_content.dynamic_data = {process_s2web, 0 }, },
    // kaizen 20130712 ID1099 For Setting TCP/UDP Connection using web server
    { "/s2wconfig",   					 "text/html",                         WICED_DYNAMIC_URL_CONTENT,    .url_content.dynamic_data  = {process_config_s2w_value, 0 }, },
    { "/s2web/conn_setting.html",        "text/html",                         WICED_STATIC_URL_CONTENT,     .url_content.static_data   = {resource_s2web_DIR_conn_setting_html,       sizeof(resource_s2web_DIR_conn_setting_html)-1   		} },
    // kaizen 20131113 ID1135 Added AP Mode Setting Using Web Server
    { "/s2wconfig_ap", 					 "text/html",                         WICED_DYNAMIC_URL_CONTENT,    .url_content.dynamic_data  = {process_config_s2w_ap_value, 0 }, },
    { "/s2web/conn_setting_apmode.html", "text/html",                         WICED_STATIC_URL_CONTENT,     .url_content.static_data   = {resource_s2web_DIR_conn_setting_apmode_html,sizeof(resource_s2web_DIR_conn_setting_apmode_html)-1	} },
    { "/s2w_reset", 					 "text/html",                         WICED_DYNAMIC_URL_CONTENT,    .url_content.dynamic_data  = {process_config_s2w_reset, 0 }, },

    // kaizen 20131030 ID1134 Added GPIO Control Page Using Web Server
    { "/s2web/gpio_control_main.html",   "text/html",                         WICED_STATIC_URL_CONTENT,     .url_content.static_data  = {resource_s2web_DIR_gpio_control_main_html,sizeof(resource_s2web_DIR_gpio_control_main_html)-1	} },
    { "/s2web/gpio_control_set.html",    "text/html",                         WICED_DYNAMIC_URL_CONTENT,    .url_content.dynamic_data = {set_gpio_setting,     0 } },
    { "/s2web/gpio_control_get.html",    "text/html",                         WICED_DYNAMIC_URL_CONTENT,    .url_content.dynamic_data = {get_gpio_setting,     0 } },
    { "/gpio_config_save",               "text/html",                         WICED_DYNAMIC_URL_CONTENT,    .url_content.dynamic_data = {gpio_config_save,     0 } },

    // kaizen 20131104 ID1133 Added Web Login Procedure
    { "/s2wlogin",              		 "text/html",                         WICED_DYNAMIC_URL_CONTENT,    .url_content.dynamic_data  = {s2wlogin,     0 } },

    // kaizen 20131118 ID1139 Added Static IP Setting Page in Station Mode
    { "/s2web/conn_setting_station.html", "text/html",                        WICED_STATIC_URL_CONTENT,     .url_content.static_data   = {resource_s2web_DIR_conn_setting_station_html,sizeof(resource_s2web_DIR_conn_setting_station_html)-1	} },
    { "/s2wconfig_station",               "text/html",                        WICED_DYNAMIC_URL_CONTENT,    .url_content.dynamic_data  = {process_s2wconfig_station,     0 } },
    { "/s2web/conn_setting_wps.html", 	  "text/html",                        WICED_STATIC_URL_CONTENT,     .url_content.static_data   = {resource_s2web_DIR_conn_setting_wps_html,sizeof(resource_s2web_DIR_conn_setting_wps_html)-1	} },

    // kaizen 20131210 ID1149 Added function in order to change serial configuration
    { "/s2web/serial_info_setting.html", "text/html",                         WICED_STATIC_URL_CONTENT,     .url_content.static_data  = {resource_s2web_DIR_serial_info_setting_html,	 sizeof(resource_s2web_DIR_serial_info_setting_html)-1	} },
    { "/s2w_serial_setting",             "text/html",                         WICED_DYNAMIC_URL_CONTENT,    .url_content.dynamic_data = {process_serial_setting,     0 } },

    // kaizen 20131210 ID1150 Added function in order to change user information as userID and userPW.
    { "/s2web/change_user_info.html", 	 "text/html",                         WICED_STATIC_URL_CONTENT,     .url_content.static_data  = {resource_s2web_DIR_change_user_info_html,	 sizeof(resource_s2web_DIR_change_user_info_html)-1			 } },
    { "/s2w_user_info_setting",          "text/html",                         WICED_DYNAMIC_URL_CONTENT,    .url_content.dynamic_data = {process_user_info_setting,     0 } },

    // kaizen 20140411 ID1169 Modified procedure for joining AP ( For Original )
    { "/s2web/set_wifi_key.html",	 	 "text/html",                         WICED_STATIC_URL_CONTENT,     .url_content.static_data  = {resource_s2web_DIR_set_wifi_key_html,	 sizeof(resource_s2web_DIR_set_wifi_key_html)-1				 } },

#if 1	// kaizen 20140410 ID1168 Customize for ShinHeung
    { "/customWeb/ShinHeung/image/ShinHeungLogo.jpg",		"image/jpg",	WICED_STATIC_URL_CONTENT,		.url_content.static_data  = {resource_customWeb_DIR_ShinHeung_DIR_image_DIR_ShinHeungLogo_jpg,	sizeof(resource_customWeb_DIR_ShinHeung_DIR_image_DIR_ShinHeungLogo_jpg) 	} },
    { "/customWeb/ShinHeung/conn_setting_apmode_SH.html",	"text/html",	WICED_STATIC_URL_CONTENT,		.url_content.static_data  = {resource_customWeb_DIR_ShinHeung_DIR_conn_setting_apmode_SH_html,	sizeof(resource_customWeb_DIR_ShinHeung_DIR_conn_setting_apmode_SH_html)-1	} },
    { "/customWeb/ShinHeung/conn_setting_SH.html",			"text/html",	WICED_STATIC_URL_CONTENT,		.url_content.static_data  = {resource_customWeb_DIR_ShinHeung_DIR_conn_setting_SH_html,			sizeof(resource_customWeb_DIR_ShinHeung_DIR_conn_setting_SH_html)-1   		} },
    { "/customWeb/ShinHeung/conn_setting_station_SH.html",	"text/html",	WICED_STATIC_URL_CONTENT,		.url_content.static_data  = {resource_customWeb_DIR_ShinHeung_DIR_conn_setting_station_SH_html,	sizeof(resource_customWeb_DIR_ShinHeung_DIR_conn_setting_station_SH_html)-1	} },
    { "/customWeb/ShinHeung/main_SH.html",					"text/html",	WICED_STATIC_URL_CONTENT,		.url_content.static_data  = {resource_customWeb_DIR_ShinHeung_DIR_main_SH_html,					sizeof(resource_customWeb_DIR_ShinHeung_DIR_main_SH_html)					} },
    { "/customWeb/ShinHeung/ota_SH.html",					"text/html",	WICED_STATIC_URL_CONTENT,		.url_content.static_data  = {resource_customWeb_DIR_ShinHeung_DIR_ota_SH_html,          		sizeof(resource_customWeb_DIR_ShinHeung_DIR_ota_SH_html)-1         			} },
    { "/customWeb/ShinHeung/scan_page_outer_SH.html",		"text/html",	WICED_STATIC_URL_CONTENT,		.url_content.static_data  = {resource_customWeb_DIR_ShinHeung_DIR_scan_page_outer_SH_html,		sizeof(resource_customWeb_DIR_ShinHeung_DIR_scan_page_outer_SH_html)-1      } },
    { "/scan_results_sh.html",								"text/html",	WICED_DYNAMIC_URL_CONTENT,		.url_content.dynamic_data = {process_scan_SH,                  0 } },
    { "/customWeb/ShinHeung/user_info_SH.html",				"text/html",	WICED_STATIC_URL_CONTENT,		.url_content.static_data  = {resource_customWeb_DIR_ShinHeung_DIR_user_info_SH_html,			sizeof(resource_customWeb_DIR_ShinHeung_DIR_user_info_SH_html)-1			} },
    // kaizen 20140411 ID1169 Modified procedure for joining AP ( For ShinHeung )
    { "/customWeb/ShinHeung/set_wifi_key_SH.html",			"text/html",	WICED_STATIC_URL_CONTENT,		.url_content.static_data  = {resource_customWeb_DIR_ShinHeung_DIR_set_wifi_key_SH_html,			sizeof(resource_customWeb_DIR_ShinHeung_DIR_set_wifi_key_SH_html)-1			} },
#endif

    { "/images/encored_logo.png",							"image/png",	WICED_STATIC_URL_CONTENT,		.url_content.static_data  = {resource_images_DIR_encored_logo_png,		  sizeof(resource_images_DIR_encored_logo_png)			} },

    // sekim 20150114 POSBANK Dummy Logo
    { "/images/dummy_logo.png",								"image/png",	WICED_STATIC_URL_CONTENT,		.url_content.static_data  = {resource_images_DIR_dummy_logo_png,		  sizeof(resource_images_DIR_dummy_logo_png)			} },

#ifdef BUILD_WIZFI250
    { "/get_custom_info",         							"text/html",	WICED_DYNAMIC_URL_CONTENT,		.url_content.dynamic_data = {process_get_custom_info,     0 } },
#endif

#endif
END_OF_HTTP_PAGE_DATABASE();


// kaizen
#ifdef WICED_ENABLE_HTTPS		// kaizen 20130624 ID1078 - In order to do not use HTTPS page
START_OF_HTTP_PAGE_DATABASE(config_https_page_database)
	ROOT_HTTP_PAGE_REDIRECT("/config/main.html"),
	{ "/config/main.html",				 "text/html",						  WICED_STATIC_URL_CONTENT,		.url_content.static_data  = {resource_config_DIR_main_html,				  sizeof(resource_config_DIR_main_html)-1				} },
	{ "/config/device_settings.html",	 "text/html",						  WICED_DYNAMIC_URL_CONTENT,	.url_content.dynamic_data = {process_app_settings_page,		  0 } },
    { "/config/join.html",               "text/html",                         WICED_STATIC_URL_CONTENT,     .url_content.static_data  = {resource_config_DIR_join_html,               sizeof(resource_config_DIR_join_html)-1               } },
    { "/config/scan_page_outer.html",    "text/html",                         WICED_STATIC_URL_CONTENT,     .url_content.static_data  = {resource_config_DIR_scan_page_outer_html,    sizeof(resource_config_DIR_scan_page_outer_html)-1    } },
    { "/config/wps_pbc.html",            "text/html",                         WICED_STATIC_URL_CONTENT,     .url_content.static_data  = {resource_config_DIR_wps_pbc_html,            sizeof(resource_config_DIR_wps_pbc_html)-1            } },
    { "/config/wps_pin.html",            "text/html",                         WICED_STATIC_URL_CONTENT,     .url_content.static_data  = {resource_config_DIR_wps_pin_html,            sizeof(resource_config_DIR_wps_pin_html)-1            } },
    { "/scripts/general_ajax_script.js", "application/javascript",            WICED_STATIC_URL_CONTENT,     .url_content.static_data  = {resource_scripts_DIR_general_ajax_script_js, sizeof(resource_scripts_DIR_general_ajax_script_js)-1 } },
    { "/scripts/wpad.dat",               "application/x-ns-proxy-autoconfig", WICED_STATIC_URL_CONTENT,     .url_content.static_data  = {resource_scripts_DIR_wpad_dat,               sizeof(resource_scripts_DIR_wpad_dat)-1               } },
    { "/images/wps_icon.png",            "image/png",                         WICED_STATIC_URL_CONTENT,     .url_content.static_data  = {resource_images_DIR_wps_icon_png,            sizeof(resource_images_DIR_wps_icon_png)              } },
    { "/images/scan_icon.png",           "image/png",                         WICED_STATIC_URL_CONTENT,     .url_content.static_data  = {resource_images_DIR_scan_icon_png,           sizeof(resource_images_DIR_scan_icon_png)             } },
    { "/images/favicon.ico",             "image/vnd.microsoft.icon",          WICED_STATIC_URL_CONTENT,     .url_content.static_data  = {resource_images_DIR_favicon_ico,             sizeof(resource_images_DIR_favicon_ico)               } },
    { "/images/wiznet_logo.png",		 "image/png",						  WICED_STATIC_URL_CONTENT,		.url_content.static_data  = {resource_images_DIR_wiznet_logo_png,		  sizeof(resource_images_DIR_wiznet_logo_png)			} },
	{ "/images/s2web_logo.png",		     "image/png",						  WICED_STATIC_URL_CONTENT,		.url_content.static_data  = {resource_images_DIR_s2web_logo_png,		  sizeof(resource_images_DIR_s2web_logo_png)			} },
    { "/images/brcmlogo_line.png",       "image/png",                         WICED_STATIC_URL_CONTENT,     .url_content.static_data  = {resource_images_DIR_brcmlogo_line_png,       sizeof(resource_images_DIR_brcmlogo_line_png)         } },
	{ "/images/over_the_air_logo.gif",   "image/gif",                         WICED_STATIC_URL_CONTENT,     .url_content.static_data  = {resource_images_DIR_over_the_air_logo_gif,   sizeof(resource_images_DIR_over_the_air_logo_gif)     } },
    { "/styles/buttons.css",             "text/css",                          WICED_STATIC_URL_CONTENT,     .url_content.static_data  = {resource_styles_DIR_buttons_css,             sizeof(resource_styles_DIR_buttons_css)               } },
    { "/connect",                        "text/html",                         WICED_DYNAMIC_URL_CONTENT,    .url_content.dynamic_data = {process_connect,               0 } },
    { "/wps_go",                         "text/html",                         WICED_DYNAMIC_URL_CONTENT,    .url_content.dynamic_data = {process_wps_go,                0 } },
	{ "/config/wizfi_ota.html",          "text/html",                         WICED_STATIC_URL_CONTENT,     .url_content.static_data  = {resource_config_DIR_wizfi_ota_html,          sizeof(resource_config_DIR_wizfi_ota_html)-1          } },
	{ "/ota_go",                         "text/html",                         WICED_DYNAMIC_URL_CONTENT,    .url_content.dynamic_data = {process_ota_go,                   0 } },
    { "/config_save",                    "text/html",                         WICED_DYNAMIC_URL_CONTENT,    .url_content.dynamic_data = {process_config_save,           0 } },
    { "/scan_results.html",              "text/html",                         WICED_DYNAMIC_URL_CONTENT,    .url_content.dynamic_data = {process_scan,                  0 } },
    { IOS_CAPTIVE_PORTAL_ADDRESS,        "text/html",                         WICED_STATIC_URL_CONTENT,     .url_content.static_data  = {CAPTIVE_PORTAL_REDIRECT_PAGE, sizeof(CAPTIVE_PORTAL_REDIRECT_PAGE) } },
	// sekim 20130423 serial-to-web demo
	{ "/s2web/demo.html",                "text/html",                         WICED_STATIC_URL_CONTENT,     .url_content.static_data  = {resource_s2web_DIR_demo_html,                sizeof(resource_s2web_DIR_demo_html)-1                } },
    { "/content.html",                   "text/html",                         WICED_DYNAMIC_URL_CONTENT,    .url_content.dynamic_data = {process_s2web, 0 }, },
#ifdef WICED_DISABLE_CONFIG_TLS
    CAPTIVE_PORTAL_REDIRECT(),
#endif /* ifdef WICED_DISABLE_CONFIG_TLS */
    /* Add more pages here */
END_OF_HTTP_PAGE_DATABASE();
#endif

extern const configuration_entry_t* app_configuration;
extern wiced_http_server_t*         http_server;
extern wiced_bool_t                 config_use_wps;
extern char                         config_wps_pin[9];

extern wiced_http_server_t*			http_sta_server;	// kaizen 20130603 ID1078 - In order to do not use http server
/******************************************************
 *               Function Definitions
 ******************************************************/

int process_app_settings_page( const char* url, wiced_tcp_stream_t* stream, void* arg )
{
    wiced_tcp_stream_write(stream, resource_config_DIR_device_settings_html, sizeof(resource_config_DIR_device_settings_html)-1);
    const configuration_entry_t* config_entry;
    char temp_buf[10];
    const char* end_str_ptr;
    int end_str_size;
    uint32_t utoa_size;
    uint8_t* app_dct = (uint8_t*)bootloader_api->get_app_config_dct();

    /* Write the app configuration table */
    char config_count[2] = {'0','0'};
    if( app_configuration != NULL )
    {
        for (config_entry = app_configuration; config_entry->name != NULL; ++config_entry)
        {

            /* Write the table entry start html direct from resource file */
            switch (config_entry->data_type)
            {
                case CONFIG_STRING_DATA:
                    wiced_tcp_stream_write(stream, resource_config_DIR_device_settings_html_dev_settings_str, sizeof(resource_config_DIR_device_settings_html_dev_settings_str)-1);
                    break;
                case CONFIG_UINT8_DATA:
                case CONFIG_UINT16_DATA:
                case CONFIG_UINT32_DATA:
                    wiced_tcp_stream_write(stream, resource_config_DIR_device_settings_html_dev_settings_int, sizeof(resource_config_DIR_device_settings_html_dev_settings_int)-1);
                    break;
                default:
                    wiced_tcp_stream_write(stream, "error", 5);
                    break;
            }

            /* Output javascript to fill the table entry */

            wiced_tcp_stream_write( stream, APP_SCRIPT_PT1, sizeof(APP_SCRIPT_PT1)-1 );
            wiced_tcp_stream_write( stream, config_count, 2 );
            wiced_tcp_stream_write( stream, APP_SCRIPT_PT2, sizeof(APP_SCRIPT_PT2)-1 );
            wiced_tcp_stream_write( stream, config_entry->name, strlen(config_entry->name) );
            wiced_tcp_stream_write( stream, APP_SCRIPT_PT3, sizeof(APP_SCRIPT_PT3)-1 );
            wiced_tcp_stream_write( stream, config_count, 2 );
            wiced_tcp_stream_write( stream, APP_SCRIPT_PT4, sizeof(APP_SCRIPT_PT4)-1 );

            /* Fill in current value */
            switch (config_entry->data_type)
            {
                case CONFIG_STRING_DATA:
                    wiced_tcp_stream_write(stream, app_dct + config_entry->dct_offset, strlen((char*)(app_dct + config_entry->dct_offset)));
                    end_str_ptr = resource_config_DIR_device_settings_html_dev_settings_str_end;
                    end_str_size = sizeof(resource_config_DIR_device_settings_html_dev_settings_str_end)-1;
                    break;
                case CONFIG_UINT8_DATA:
                    memset(temp_buf, ' ', 3);
                    utoa_size = utoa(*(uint8_t*)(app_dct + config_entry->dct_offset), (char*)temp_buf, 0, 3);
                    wiced_tcp_stream_write(stream, temp_buf, utoa_size);
                    end_str_ptr = resource_config_DIR_device_settings_html_dev_settings_int_end;
                    end_str_size = sizeof(resource_config_DIR_device_settings_html_dev_settings_int_end)-1;
                    break;
                case CONFIG_UINT16_DATA:
                    memset(temp_buf, ' ', 5);
                    utoa_size = utoa(*(uint16_t*)(app_dct + config_entry->dct_offset), (char*)temp_buf, 0, 5);
                    wiced_tcp_stream_write(stream, temp_buf, utoa_size);
                    end_str_ptr = resource_config_DIR_device_settings_html_dev_settings_int_end;
                    end_str_size = sizeof(resource_config_DIR_device_settings_html_dev_settings_int_end)-1;
                    break;
                case CONFIG_UINT32_DATA:
                    memset(temp_buf, ' ', 10);
                    utoa_size = utoa(*(uint32_t*)(app_dct + config_entry->dct_offset), (char*)temp_buf, 0, 10);
                    wiced_tcp_stream_write(stream, temp_buf, utoa_size);
                    end_str_ptr = resource_config_DIR_device_settings_html_dev_settings_int_end;
                    end_str_size = sizeof(resource_config_DIR_device_settings_html_dev_settings_int_end)-1;
                    break;
                default:
                    wiced_tcp_stream_write(stream, "error", 5);
                    end_str_ptr = NULL;
                    end_str_size = 0;
                    break;
            }

            wiced_tcp_stream_write(stream, APP_SCRIPT_PT5, sizeof(APP_SCRIPT_PT5)-1);
            wiced_tcp_stream_write(stream, end_str_ptr, end_str_size);


            if (config_count[1] == '9')
            {
                ++config_count[0];
                config_count[1] = '0';
            }
            else
            {
                ++config_count[1];
            }
        }
    }
    wiced_tcp_stream_write(stream, resource_config_DIR_device_settings_html_dev_settings_bottom, sizeof(resource_config_DIR_device_settings_html_dev_settings_bottom)-1);

    return 0;
}

// kaizen 20140408 ID1154, ID1166 Add AT+FWEBSOPT
wiced_bool_t isPrintable(uint8_t* ssid, uint8_t len)
{
	uint8_t i;

	for(i=0; i<len; i++)
	{
		if( ssid[i] < 0x20 || ssid[i] > 0x7f)
			return WICED_FALSE;
	}

	return WICED_TRUE;
}

wiced_result_t scan_handler( wiced_scan_handler_result_t* malloced_scan_result )
{
#if BUILD_WIZFI250	// kaizen 20140408 ID1154, ID1166 Add AT+FWEBSOPT
	wiced_bool_t valid_scan_result = WICED_TRUE;
#endif

	process_scan_data_t* scan_data = (process_scan_data_t*)malloced_scan_result->user_data;

    /* Check if scan is finished (Invalid scan result) */
    if (malloced_scan_result->scan_complete != WICED_TRUE)
    {
        char temp_buffer[40];
        int temp_length;

        malloc_transfer_to_curr_thread( malloced_scan_result );

        wiced_tcp_stream_t* stream = scan_data->stream;

#ifdef BUILD_WIZFI250	// kaizen 20140408 ID1154, ID1166 Add AT+FWEBSOPT
        if( g_wxProfile.show_hidden_ssid  == 0 && malloced_scan_result->ap_details.SSID.len == 0 )
        	valid_scan_result = WICED_FALSE;

        if( g_wxProfile.show_only_english_ssid == 1 && isPrintable(malloced_scan_result->ap_details.SSID.val, malloced_scan_result->ap_details.SSID.len) == WICED_FALSE )
        	valid_scan_result = WICED_FALSE;

        if     ( g_wxProfile.show_rssi_range == 1 && malloced_scan_result->ap_details.signal_strength <= RSSI_POOR )		valid_scan_result = WICED_FALSE;
        else if( g_wxProfile.show_rssi_range == 2 && malloced_scan_result->ap_details.signal_strength <= RSSI_GOOD )		valid_scan_result = WICED_FALSE;
        else if( g_wxProfile.show_rssi_range == 3 && malloced_scan_result->ap_details.signal_strength <= RSSI_VERY_GOOD )	valid_scan_result = WICED_FALSE;


        if( valid_scan_result == WICED_TRUE )
        {
           	/* Write out the start HTML for the row */
    		wiced_tcp_stream_write(stream, resource_config_DIR_scan_results_html_row, sizeof(resource_config_DIR_scan_results_html_row)-1);

    		wiced_tcp_stream_write(stream, SCAN_SCRIPT_PT1, sizeof(SCAN_SCRIPT_PT1)-1);
    		temp_length = sprintf( temp_buffer, "%d", scan_data->result_count );
    		scan_data->result_count++;
    		wiced_tcp_stream_write(stream, temp_buffer, temp_length);

    		wiced_tcp_stream_write(stream, SCAN_SCRIPT_PT2, sizeof(SCAN_SCRIPT_PT2)-1);
    		/* SSID */
    		wiced_tcp_stream_write(stream, malloced_scan_result->ap_details.SSID.val, malloced_scan_result->ap_details.SSID.len);
    		wiced_tcp_stream_write(stream, SCAN_SCRIPT_PT3, sizeof(SCAN_SCRIPT_PT3)-1);
    		/* RSSI */
    		if ( malloced_scan_result->ap_details.signal_strength <= RSSI_VERY_POOR )
    		{
    			wiced_tcp_stream_write( stream, RSSI_VERY_POOR_STR, sizeof( RSSI_VERY_POOR_STR ) - 1 );
    		}
    		else if ( malloced_scan_result->ap_details.signal_strength <= RSSI_POOR )
    		{
    			wiced_tcp_stream_write( stream, RSSI_POOR_STR, sizeof( RSSI_POOR_STR ) - 1 );
    		}
    		else if ( malloced_scan_result->ap_details.signal_strength <= RSSI_GOOD )
    		{
    			wiced_tcp_stream_write( stream, RSSI_POOR_STR, sizeof( RSSI_POOR_STR ) - 1 );
    		}
    		else if ( malloced_scan_result->ap_details.signal_strength <= RSSI_VERY_GOOD )
    		{
    			wiced_tcp_stream_write( stream, RSSI_GOOD_STR, sizeof( RSSI_GOOD_STR ) - 1 );
    		}
    		else
    		{
    			wiced_tcp_stream_write( stream, RSSI_EXCELLENT_STR, sizeof( RSSI_EXCELLENT_STR ) - 1 );
    		}
    		wiced_tcp_stream_write(stream, SCAN_SCRIPT_PT4, sizeof(SCAN_SCRIPT_PT4)-1);

    		/* Security */
    		temp_length = sprintf( temp_buffer, "%d", malloced_scan_result->ap_details.security );
    		wiced_tcp_stream_write(stream, temp_buffer, temp_length);
    		wiced_tcp_stream_write(stream, SCAN_SCRIPT_PT5, sizeof(SCAN_SCRIPT_PT5)-1);

    		/* Security Str */
    		if	( malloced_scan_result->ap_details.security == WICED_SECURITY_OPEN )
    			temp_length = sprintf( temp_buffer, "%s", "OPEN");
    		else if	( malloced_scan_result->ap_details.security == WICED_SECURITY_WEP_PSK )
    			temp_length = sprintf( temp_buffer, "%s", "WEP_PSK");
    		else if	( malloced_scan_result->ap_details.security == WICED_SECURITY_WEP_SHARED )
				temp_length = sprintf( temp_buffer, "%s", "WEP_SHARED");
    		else if( malloced_scan_result->ap_details.security == WICED_SECURITY_WPA_TKIP_PSK )
    			temp_length = sprintf( temp_buffer, "%s", "WPA_TKIP");
    		else if( malloced_scan_result->ap_details.security == WICED_SECURITY_WPA_AES_PSK )
    			temp_length = sprintf( temp_buffer, "%s", "WPA_AES");
    		else if( malloced_scan_result->ap_details.security == WICED_SECURITY_WPA2_AES_PSK )
    			temp_length = sprintf( temp_buffer, "%s", "WPA2_AES");
    		else if( malloced_scan_result->ap_details.security == WICED_SECURITY_WPA2_TKIP_PSK )
    			temp_length = sprintf( temp_buffer, "%s", "WPA2_TKIP");
    		else if( malloced_scan_result->ap_details.security == WICED_SECURITY_WPA2_MIXED_PSK )
    			temp_length = sprintf( temp_buffer, "%s", "WPA2_MIXED");
    		else if( malloced_scan_result->ap_details.security == WICED_SECURITY_WPS_OPEN )
    			temp_length = sprintf( temp_buffer, "%s", "WPS_OPEN");
    		else if( malloced_scan_result->ap_details.security == WICED_SECURITY_WPS_SECURE )
    			temp_length = sprintf( temp_buffer, "%s", "WPS_SECURE");
    		else
    			temp_length = sprintf( temp_buffer, "%s", "UNKNOWN");

    		wiced_tcp_stream_write(stream, temp_buffer, temp_length);
    		wiced_tcp_stream_write(stream, SCAN_SCRIPT_PT6, sizeof(SCAN_SCRIPT_PT6)-1);


    		/* Channel */
    		temp_length = sprintf( temp_buffer, "%d", malloced_scan_result->ap_details.channel );
    		wiced_tcp_stream_write(stream, temp_buffer, temp_length);
    		wiced_tcp_stream_write(stream, SCAN_SCRIPT_PT7, sizeof(SCAN_SCRIPT_PT7)-1);

    		/* BSSID */
    		temp_length = sprintf( temp_buffer, "%02X%02X%02X%02X%02X%02X", malloced_scan_result->ap_details.BSSID.octet[0], malloced_scan_result->ap_details.BSSID.octet[1], malloced_scan_result->ap_details.BSSID.octet[2], malloced_scan_result->ap_details.BSSID.octet[3], malloced_scan_result->ap_details.BSSID.octet[4], malloced_scan_result->ap_details.BSSID.octet[5] );
    		wiced_tcp_stream_write(stream, temp_buffer, temp_length);
    		wiced_tcp_stream_write(stream, SCAN_SCRIPT_PT8, sizeof(SCAN_SCRIPT_PT8)-1);

    		/* Write out the end HTML for the row */
    		wiced_tcp_stream_write(stream, resource_config_DIR_scan_results_html_row_end, sizeof(resource_config_DIR_scan_results_html_row_end)-1);
        }

#else
       	/* Write out the start HTML for the row */
		wiced_tcp_stream_write(stream, resource_config_DIR_scan_results_html_row, sizeof(resource_config_DIR_scan_results_html_row)-1);

		wiced_tcp_stream_write(stream, SCAN_SCRIPT_PT1, sizeof(SCAN_SCRIPT_PT1)-1);
		temp_length = sprintf( temp_buffer, "%d", scan_data->result_count );
		scan_data->result_count++;
		wiced_tcp_stream_write(stream, temp_buffer, temp_length);

		wiced_tcp_stream_write(stream, SCAN_SCRIPT_PT2, sizeof(SCAN_SCRIPT_PT2)-1);
		/* SSID */
		wiced_tcp_stream_write(stream, malloced_scan_result->ap_details.SSID.val, malloced_scan_result->ap_details.SSID.len);
		wiced_tcp_stream_write(stream, SCAN_SCRIPT_PT3, sizeof(SCAN_SCRIPT_PT3)-1);
		/* RSSI */
		if ( malloced_scan_result->ap_details.signal_strength <= RSSI_VERY_POOR )
		{
			wiced_tcp_stream_write( stream, RSSI_VERY_POOR_STR, sizeof( RSSI_VERY_POOR_STR ) - 1 );
		}
		else if ( malloced_scan_result->ap_details.signal_strength <= RSSI_POOR )
		{
			wiced_tcp_stream_write( stream, RSSI_POOR_STR, sizeof( RSSI_POOR_STR ) - 1 );
		}
		else if ( malloced_scan_result->ap_details.signal_strength <= RSSI_GOOD )
		{
			wiced_tcp_stream_write( stream, RSSI_POOR_STR, sizeof( RSSI_POOR_STR ) - 1 );
		}
		else if ( malloced_scan_result->ap_details.signal_strength <= RSSI_VERY_GOOD )
		{
			wiced_tcp_stream_write( stream, RSSI_GOOD_STR, sizeof( RSSI_GOOD_STR ) - 1 );
		}
		else
		{
			wiced_tcp_stream_write( stream, RSSI_EXCELLENT_STR, sizeof( RSSI_EXCELLENT_STR ) - 1 );
		}
		wiced_tcp_stream_write(stream, SCAN_SCRIPT_PT4, sizeof(SCAN_SCRIPT_PT4)-1);

		/* Security */
		temp_length = sprintf( temp_buffer, "%d", malloced_scan_result->ap_details.security );
		wiced_tcp_stream_write(stream, temp_buffer, temp_length);
		wiced_tcp_stream_write(stream, SCAN_SCRIPT_PT5, sizeof(SCAN_SCRIPT_PT5)-1);

		/* Channel */
		temp_length = sprintf( temp_buffer, "%d", malloced_scan_result->ap_details.channel );
		wiced_tcp_stream_write(stream, temp_buffer, temp_length);
		wiced_tcp_stream_write(stream, SCAN_SCRIPT_PT6, sizeof(SCAN_SCRIPT_PT6)-1);

		/* BSSID */
		temp_length = sprintf( temp_buffer, "%02X%02X%02X%02X%02X%02X", malloced_scan_result->ap_details.BSSID.octet[0], malloced_scan_result->ap_details.BSSID.octet[1], malloced_scan_result->ap_details.BSSID.octet[2], malloced_scan_result->ap_details.BSSID.octet[3], malloced_scan_result->ap_details.BSSID.octet[4], malloced_scan_result->ap_details.BSSID.octet[5] );
		wiced_tcp_stream_write(stream, temp_buffer, temp_length);
		wiced_tcp_stream_write(stream, SCAN_SCRIPT_PT7, sizeof(SCAN_SCRIPT_PT7)-1);

		/* Write out the end HTML for the row */
		wiced_tcp_stream_write(stream, resource_config_DIR_scan_results_html_row_end, sizeof(resource_config_DIR_scan_results_html_row_end)-1);
#endif
    }
    else
    {
        wiced_rtos_set_semaphore( &scan_data->semaphore );
    }

    free(malloced_scan_result);

    return WICED_SUCCESS;
}


static int process_scan( const char* url, wiced_tcp_stream_t* stream, void* arg )
{
    process_scan_data_t scan_data;

    scan_data.stream = stream;
    scan_data.result_count = 0;

    /* Initialise the semaphore that will tell us when the scan is complete */
    wiced_rtos_init_semaphore(&scan_data.semaphore);

    /* Send the scan results header */
    wiced_tcp_stream_write(stream, resource_config_DIR_scan_results_html, sizeof(resource_config_DIR_scan_results_html)-1);

    /* Start the scan */
    wiced_wifi_scan_networks( scan_handler, &scan_data );

    /* Wait until scan is complete */
    // sekim 20140731 ID1185 Fixed hangup problem when operate scan procedure in web
    //wiced_rtos_get_semaphore(&scan_data.semaphore, WICED_WAIT_FOREVER);
    wiced_rtos_get_semaphore(&scan_data.semaphore, 15000);

    /* Send the static footer HTML data */
    wiced_tcp_stream_write(stream, resource_config_DIR_scan_results_html_bottom, sizeof( resource_config_DIR_scan_results_html_bottom ) - 1);

    /* Clean up */
    wiced_rtos_deinit_semaphore(&scan_data.semaphore);

    return 0;
}


// kaizen 20140411 ID1169 Modified procedure for joining AP ( For ShinHeung )
static int process_scan_SH ( const char* url, wiced_tcp_stream_t* stream, void* arg )
{
    process_scan_data_t scan_data;

    scan_data.stream = stream;
    scan_data.result_count = 0;


    /* Initialise the semaphore that will tell us when the scan is complete */
    wiced_rtos_init_semaphore(&scan_data.semaphore);

    /* Send the scan results header */
    wiced_tcp_stream_write(stream, resource_customWeb_DIR_ShinHeung_DIR_scan_results_SH_html, sizeof(resource_customWeb_DIR_ShinHeung_DIR_scan_results_SH_html)-1);

    /* Start the scan */
    wiced_wifi_scan_networks( scan_handler, &scan_data );

    /* Wait until scan is complete */
    //wiced_rtos_get_semaphore(&scan_data.semaphore, WICED_WAIT_FOREVER);
    wiced_rtos_get_semaphore(&scan_data.semaphore, 15000);						// kaizen 20140414 ID1170 Fixed hangup problem when operate scan procedure in web

    /* Send the static footer HTML data */
    wiced_tcp_stream_write(stream, resource_customWeb_DIR_ShinHeung_DIR_scan_results_SH_html_bottom, sizeof( resource_customWeb_DIR_ShinHeung_DIR_scan_results_SH_html_bottom ) - 1);

    /* Clean up */
    wiced_rtos_deinit_semaphore(&scan_data.semaphore);

    return 0;
}

static int process_wps_go( const char* url, wiced_tcp_stream_t* stream, void* arg )
{
    int params_len = strlen(url);

    /* client has signalled to start client mode via WPS. */
    config_use_wps = WICED_TRUE;

    /* Check if config method is PIN */
    if ( ( strlen( PIN_FIELD_NAME ) + 1 < params_len ) &&
         ( 0 == strncmp( url, PIN_FIELD_NAME "=", strlen( PIN_FIELD_NAME ) + 1 ) ) )
    {
        url += strlen( PIN_FIELD_NAME ) + 1;
        int pinlen = 0;

        /* Find length of pin */
        while ( ( url[pinlen] != '&'    ) &&
                ( url[pinlen] != '\n'   ) &&
                ( url[pinlen] != '\x00' ) &&
                ( params_len > 0 ) )
        {
            pinlen++;
            params_len--;
        }
        memcpy( config_wps_pin, url, pinlen );
        config_wps_pin[pinlen] = '\x00';
    }
    else
    {
        config_wps_pin[0] = '\x00';
    }


#ifdef BUILD_WIZFI250	// kaizen 20130716 ID1101 - Modified WPS function in order to reboot WizFi250 when receive command from web
    wiced_result_t  wps_status;
    UINT32 			wps_mode;

    if(strlen(config_wps_pin) == 0 )    	wps_mode = WICED_WPS_PBC_MODE;
    else							    	wps_mode = WICED_WPS_PIN_MODE;

    config_use_wps = WICED_FALSE;					// kaizen 20130716 Do not use wiced_configure_device_start() routine.
#if 1												// kaizen 20130909 ID1128 Fix the WPS problem
    WXCmd_WLEAVE((UINT8*)"");
    wiced_rtos_delay_milliseconds( 500 * MILLISECONDS );
#else
    wiced_http_server_stop(http_sta_server);		// kaizen 20130716 For connectting to another AP.
#endif

	wps_status = Request_WPS( wps_mode, config_wps_pin );
	if( wps_status != WICED_SUCCESS )
		W_DBG( "Request WPS Fail, Status : %d",wps_status );

	WXS2w_SystemReset();
#else
		/* Config has been set. Turn off HTTP server */
	#ifdef WICED_ENABLE_HTTPS		// kaizen 20130603 ID1078 - In order to do not use http server
		wiced_http_server_stop(http_server);
	#else
		wiced_http_server_stop(http_sta_server);
	#endif
#endif



    return 1;
}

// sekim 20140731 ID1184 SSID including Space problem(by Coway)
#ifdef BUILD_WIZFI250
void str_replace(char* search, char* replace, char* subject, char* dest)
{
    //char  *p = NULL , *old = NULL , *new_subject = NULL ;
	char  *p = NULL , *old = NULL;
    int c = 0 , search_size;

    search_size = strlen(search);

    //Count how many occurences
    for(p = strstr(subject , search) ; p != NULL ; p = strstr(p + search_size , search))
    {
        c++;
    }

    //Final size
    c = ( strlen(replace) - search_size )*c + strlen(subject);

    //New subject with new size
    //new_subject = malloc( c );

    //Set it to blank
    //strcpy(new_subject , "");
    strcpy(dest, "");

    //The start position
    old = subject;

    for(p = strstr(subject , search) ; p != NULL ; p = strstr(p + search_size , search))
    {
        //move ahead and copy some text from original subject , from a certain position
        //strncpy(new_subject + strlen(new_subject) , old , p - old);
    	strncpy(dest + strlen(dest) , old , p - old);

        //move ahead and copy the replacement text
        //strcpy(new_subject + strlen(new_subject) , replace);
    	strcpy(dest + strlen(dest) , replace);

        //The new start position after this search match
        old = p + search_size;
    }

    //Copy the part after the last search match
    //strcpy(new_subject + strlen(new_subject) , old);
    strcpy(dest + strlen(dest) , old);
    //return new_subject;
}
#endif

/**
 * URL handler for signaling web server shutdown
 *
 * The reception of this web server request indicates that the client wants to
 * start the appliance, after shutting down the access point, DHCP server and web server
 * Decodes the URL parameters into the connection configuration buffer, then signals
 * for the web server to shut down
 *
 * @param  socket  : a handle for the TCP socket over which the data will be sent
 * @param  params     : a byte array containing any parameters included in the URL
 * @param  params_len : size of the params byte array in bytes
 */
static int process_connect( const char* params, wiced_tcp_stream_t* stream, void* arg )
{
#ifdef BUILD_WIZFI250	// kaizen 20131011 For using WEP Security option in web configuration
	UINT8 is_wep_security = 0;
    UINT8 wifi_buff_keylen;
    UINT8 wifi_buff_keydata[WSEC_MAX_PSK_LEN];
#endif

	struct
    {
        wiced_bool_t             device_configured;
        wiced_config_ap_entry_t  ap_entry;
    } temp_config;

    memset(&temp_config, 0, sizeof(temp_config));

    /* First, parse AP details */
    while (params[0] == 'a' && params[3] == '=')
    {
        /* Extract the AP index and check validity */
        uint8_t ap_index = params[2]-'0';
        if (ap_index >= CONFIG_AP_LIST_SIZE)
        {
            return -1;
        }

        /* Find the end of the value */
        const char* end_of_value = &params[4];

    	////////////////////////////////////////////////////////////////////////////////////////////////
        // kaizen 20140919 ID1186 SSID included special character as (~!@#$%^&*()_+|) Problem
        /*
        while( (*end_of_value != '&') && (*end_of_value != '\x00') && (*end_of_value != '\n') )
        {
            ++end_of_value;
        }
        */
        while((*end_of_value != '\x00') && (*end_of_value != '\n'))
        {
        	if( strncmp(end_of_value,"+^&",3) == 0 )
        		break;

        	++end_of_value;
        }


        /* Parse either the SSID or PSK*/
#ifdef BUILD_WIZFI250				// kaizen 20131011 For using WEP Security option in web configuration
        if (params[1] == 's')
        {
        	memcpy( temp_config.ap_entry.details.SSID.val, &params[4], end_of_value - &params[4]);
        	temp_config.ap_entry.details.SSID.len = end_of_value - &params[4];
            temp_config.ap_entry.details.SSID.val[temp_config.ap_entry.details.SSID.len] = 0;

        	////////////////////////////////////////////////////////////////////////////////////////////////
        	// sekim 20140731 ID1184 SSID including Space problem(by Coway)
        	/*
        	char szBuffSSID1[40];
        	char szBuffSSID2[40];
        	memset(szBuffSSID1, 0, sizeof(szBuffSSID1));
        	memset(szBuffSSID2, 0, sizeof(szBuffSSID2));

        	memcpy(szBuffSSID1, &params[4], end_of_value - &params[4]);
        	str_replace("%20", " ", szBuffSSID1, szBuffSSID2);

        	memcpy( temp_config.ap_entry.details.SSID.val, szBuffSSID2, strlen(szBuffSSID2));
        	temp_config.ap_entry.details.SSID.len = end_of_value - &params[4];
            temp_config.ap_entry.details.SSID.val[temp_config.ap_entry.details.SSID.len] = 0;
            */
            ////////////////////////////////////////////////////////////////////////////////////////////////
        }
        else if (params[1] == 'p')
        {
        	wifi_buff_keylen = end_of_value - &params[4];

        	if( is_wep_security )
            {
        		if( wifi_buff_keylen == 5 || wifi_buff_keylen == 13 )				// WEP ASCII KEY
        		{
        			wifi_buff_keydata[0] = 0;
            		wifi_buff_keydata[1] = wifi_buff_keylen;
            		strncpy((char*)wifi_buff_keydata+2, (char*)params+4, wifi_buff_keylen );
        		}
        		else if ( wifi_buff_keylen == 10 || wifi_buff_keylen == 26 )		// WEP HEX KEY
        		{
        			UINT8  cnt, tmp[3]={0,};
        			UINT32 val;

        			wifi_buff_keylen /= 2;

        			wifi_buff_keydata[0] = 0;
            		wifi_buff_keydata[1] = wifi_buff_keylen;

            		for( cnt=0; cnt<wifi_buff_keylen; cnt++ )
            		{
            			memcpy(tmp, &params[4+(cnt*2)], 2);
            			if(WXParse_Hex(tmp, &val) != WXCODE_SUCCESS) return WXCODE_EINVAL;

            			wifi_buff_keydata[cnt+2] = val;
            		}
        		}

        		wifi_buff_keylen += 2;
            }
            else																	// Other Security
            {
                strncpy((char*)wifi_buff_keydata, (char*)&params[4], wifi_buff_keylen);
            }

    		temp_config.ap_entry.security_key_length = wifi_buff_keylen;
    		memcpy( temp_config.ap_entry.security_key, wifi_buff_keydata, wifi_buff_keylen);
    		temp_config.ap_entry.security_key[wifi_buff_keylen] = 0;
        }
        else if (params[1] == 't')
        {
        	temp_config.ap_entry.details.security = (wiced_security_t) atoi( &params[4] );

        	if( temp_config.ap_entry.details.security == WICED_SECURITY_WEP_PSK || temp_config.ap_entry.details.security == WICED_SECURITY_WEP_SHARED )
        	{
        		is_wep_security = 1;
        	}
        }
        else
        {
            return -1;
        }

    	////////////////////////////////////////////////////////////////////////////////////////////////
        // kaizen 20140919 ID1186 SSID included special character as (~!@#$%^&*()_+|) Problem
        //params = end_of_value + 1;
        params = end_of_value + 3;
#endif
    }

    /* Save updated config details */
    temp_config.device_configured = WICED_TRUE;
    bootloader_api->write_wifi_config_dct(0, &temp_config, sizeof(temp_config));


#ifdef BUILD_WIZFI250	// kaizen 20130712 ID1099 For Setting TCP/UDP Connection using web server
    WT_PROFILE temp_dct;
    wiced_dct_read_app_section( &temp_dct, sizeof(WT_PROFILE) );

    strcpy((char*)temp_dct.wifi_ssid, (char*)temp_config.ap_entry.details.SSID.val);

    //strncpy((char*)temp_dct.wifi_keydata, (char*)temp_config.ap_entry.security_key, temp_config.ap_entry.security_key_length);	// kaizen 20130716 ID1100 Fixed bug which security key is changed when execute process_connect()
    memcpy((char*)temp_dct.wifi_keydata, (char*)temp_config.ap_entry.security_key, temp_config.ap_entry.security_key_length);		// kaizen 20131011 For using WEP Security option in web configuration

    temp_dct.wifi_authtype = temp_config.ap_entry.details.security;
    temp_dct.wifi_keylen   = temp_config.ap_entry.security_key_length;		// kaizen 20130716 ID1100 Fixed bug which security key is changed when execute process_connect()
    temp_dct.wifi_mode     = STATION_MODE;									// kaizen 20130717 ID1102 Fixed bug which WizFi250 do not set association information in AP mode


    //if( strncmp((char*)g_wxProfile.custom_idx,"SHINHEUNG",sizeof("SHINHEUNG")) == 0 )								// kaizen 20140410 ID1168 Customize for ShinHeung
   	if( CHKCUSTOM("SHINHEUNG") )
    {
		// Set Message Level
		temp_dct.msgLevel = 2;

		// Set SFORM
		memcpy(temp_dct.xrdf_main,"110001110", sizeof(temp_dct.xrdf_main));
		temp_dct.xrdf_data[0] = 0x7b;	temp_dct.xrdf_data[1] = 0x3b;	temp_dct.xrdf_data[2] = 0x7d;
		temp_dct.xrdf_data[3] = 0x00;	temp_dct.xrdf_data[4] = 0x0a;

		// Set Serial
		temp_dct.usart_init_structure.USART_BaudRate 			= 460800;
		temp_dct.usart_init_structure.USART_WordLength          = USART_WordLength_8b;
		temp_dct.usart_init_structure.USART_Parity              = USART_Parity_No;
		temp_dct.usart_init_structure.USART_StopBits            = USART_StopBits_1;
		temp_dct.usart_init_structure.USART_HardwareFlowControl = USART_HardwareFlowControl_RTS_CTS;
		temp_dct.usart_init_structure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
    }

    wiced_dct_write_app_section( &temp_dct, sizeof(WT_PROFILE) );

    WXS2w_SystemReset();	// kaizen 20130520 ID1039 - When Module Reset, Socket close & Disassociation
#endif	// BUILD_WIZFI250


    /* Config has been set. Turn off HTTP server */
#ifdef WICED_ENABLE_HTTPS		// kaizen 20130603 ID1078 - In order to do not use http server
    wiced_http_server_stop(http_server);
#else
    wiced_http_server_stop(http_sta_server);
#endif

    return 0;
}

static int process_config_save( const char* params, wiced_tcp_stream_t* stream, void* arg )
{
    if ( app_configuration != NULL )
    {
        uint32_t earliest_offset = 0xFFFFFFFF;
        uint32_t end_of_last_offset = 0x0;
        const configuration_entry_t* config_entry;

        /* Calculate how big the app config details are */
        for ( config_entry = app_configuration; config_entry->name != NULL; ++config_entry )
        {
            if ( config_entry->dct_offset < earliest_offset )
            {
                earliest_offset = config_entry->dct_offset;
            }
            if ( config_entry->dct_offset + config_entry->data_size > end_of_last_offset )
            {
                end_of_last_offset = config_entry->dct_offset + config_entry->data_size;
            }
        }

        uint8_t* app_dct = malloc( end_of_last_offset - earliest_offset );
        memcpy( app_dct, (uint8_t*) bootloader_api->get_app_config_dct( ) + earliest_offset, ( end_of_last_offset - earliest_offset ) );
        if ( app_dct != NULL )
        {
            while ( params[0] == 'v' && params[3] == '=' )
            {
                /* Extract the variable index and check validity */
                uint16_t variable_index = ( ( params[1] - '0' ) << 8 ) | ( params[2] - '0' );

                /* Find the end of the value */
                const char* end_of_value = &params[4];
                while ( ( *end_of_value != '&' ) && ( *end_of_value != '\n' ) )
                {
                    ++end_of_value;
                }

                /* Parse param */
                config_entry = &app_configuration[variable_index];
                switch ( config_entry->data_type )
                {
                    case CONFIG_STRING_DATA:
                        memcpy( (uint8_t*) ( app_dct + config_entry->dct_offset ), &params[4], end_of_value - &params[4]);
                        ( (uint8_t*) ( app_dct + config_entry->dct_offset ) )[end_of_value - &params[4]] = 0;
                        break;
                    case CONFIG_UINT8_DATA:
                        *(uint8_t*) ( app_dct + config_entry->dct_offset - earliest_offset ) = (uint8_t) atoi( &params[4] );
                        break;
                    case CONFIG_UINT16_DATA:
                        *(uint16_t*) ( app_dct + config_entry->dct_offset - earliest_offset ) = (uint16_t) atoi( &params[4] );
                        break;
                    case CONFIG_UINT32_DATA:
                        *(uint32_t*) ( app_dct + config_entry->dct_offset - earliest_offset ) = (uint32_t) atoi( &params[4] );
                        break;
                    default:
                        break;
                }

                params = end_of_value + 1;
            }

            /* Write the app DCT */
            bootloader_api->write_app_config_dct( earliest_offset, app_dct, end_of_last_offset - earliest_offset );
            // kaizen 20130412 ID1042
		#ifdef BUILD_WIZFI250
            Load_Profile();
            Apply_To_Wifi_Dct();
		#endif

            /* Free the temporary buffer */
            free( app_dct );
        }
    }

    #define CONFIG_SAVE_SUCCESS  "Config saved"
    wiced_tcp_stream_write(stream, CONFIG_SAVE_SUCCESS, sizeof(CONFIG_SAVE_SUCCESS)-1);

    return 0;
}

// sekim 20140716 ID1182 add s2web_main
#ifdef BUILD_WIZFI250
struct WT_PROFILE;
static int process_s2web_main( const char* params, wiced_tcp_stream_t* stream, void* arg )
{
	/*
	#define S2WEB_MAIN_1	"<tr onclick=\"conn_setting()\"><td align=\"center\"><img src=\"../images/scan_icon.png\" style=\"vertical-align:middle\" alt=\"Scan icon\" /></td><td><p>S2W Setting & Scan Network</p></td></tr>"
	#define S2WEB_MAIN_2	"<tr onclick=\"wps_pbc()\"><td align=\"center\"><img src=\"../images/wps_icon.png\" style=\"vertical-align:middle\" alt=\"WPS icon\" /></td><td><p>WPS(Push button)</p></td></tr>"
	#define S2WEB_MAIN_3	"<tr onclick=\"wps_pin()\"><td align=\"center\"><img src=\"../images/wps_icon.png\" style=\"vertical-align:middle\" alt=\"WPS icon\" /></td><td><p>WPS(PIN)</p></td></tr>"
	#define S2WEB_MAIN_4	"<tr onclick=\"document.location.href='../s2web/wizfi_ota.html'\"><td align=\"center\"><img src=\"../images/over_the_air_logo.png\" width=\"150\" height=\"120\" style=\"vertical-align:middle\" alt=\"OTA icon\" /></td><td><p>Change to OTA Mode</p></td></tr>"
	#define S2WEB_MAIN_5	"<tr onclick=\"document.location.href='../s2web/gpio_control_main.html'\"><td align=\"center\"><img src=\"../images/gpio_icon.png\" width=\"150\" height=\"120\" style=\"vertical-align:middle\" alt=\"GPIO icon\" /></td><td><p>GPIO Control</p></td></tr>"
	#define S2WEB_MAIN_6	"<tr onclick=\"document.location.href='../s2web/serial_info_setting.html'\"><td align=\"center\"><img src=\"../images/serial_setting_icon.png\" width=\"150\" height=\"120\" style=\"vertical-align:middle\" alt=\"Serial Setting icon\" /></td><td><p>Serial Setting</p></td></tr>"
	#define S2WEB_MAIN_7	"<tr onclick=\"document.location.href='../s2web/change_user_info.html'\"><td align=\"center\"><img src=\"../images/user_information_icon.png\" width=\"200\" height=\"120\" style=\"vertical-align:middle\" alt=\"User Information Setting icon\" /></td><td><p>User Information Setting</p></td></tr>"

	if ( g_wxProfile.s2web_main[0]!='0' )	wiced_tcp_stream_write(stream, S2WEB_MAIN_1, sizeof(S2WEB_MAIN_1)-1);
	if ( g_wxProfile.s2web_main[1]!='0' )	wiced_tcp_stream_write(stream, S2WEB_MAIN_2, sizeof(S2WEB_MAIN_2)-1);
	if ( g_wxProfile.s2web_main[2]!='0' )	wiced_tcp_stream_write(stream, S2WEB_MAIN_3, sizeof(S2WEB_MAIN_3)-1);
	if ( g_wxProfile.s2web_main[3]!='0' )	wiced_tcp_stream_write(stream, S2WEB_MAIN_4, sizeof(S2WEB_MAIN_4)-1);
	if ( g_wxProfile.s2web_main[4]!='0' )	wiced_tcp_stream_write(stream, S2WEB_MAIN_5, sizeof(S2WEB_MAIN_5)-1);
	if ( g_wxProfile.s2web_main[5]!='0' )	wiced_tcp_stream_write(stream, S2WEB_MAIN_6, sizeof(S2WEB_MAIN_6)-1);
	if ( g_wxProfile.s2web_main[6]!='0' )	wiced_tcp_stream_write(stream, S2WEB_MAIN_7, sizeof(S2WEB_MAIN_7)-1);
	*/

#define S2WEB_MAIN_1	"<img src=\"../images/scan_icon.png\" style=\"vertical-align:middle\" alt=\"Scan icon\" onclick=\"conn_setting()\"/><p onclick=\"conn_setting()\">S2W Setting & Scan Network</p>"
#define S2WEB_MAIN_2	"<img src=\"../images/wps_icon.png\" style=\"vertical-align:middle\" alt=\"WPS icon\"  onclick=\"wps_pbc()\"/><p onclick=\"wps_pbc()\">WPS(Push button)</p>"
#define S2WEB_MAIN_3	"<img src=\"../images/wps_icon.png\" style=\"vertical-align:middle\" alt=\"WPS icon\"  onclick=\"wps_pin()\"/><p onclick=\"wps_pin()\">WPS(PIN)</p>"
#define S2WEB_MAIN_4	"<img src=\"../images/over_the_air_logo.png\" width=\"150\" height=\"120\" style=\"vertical-align:middle\" alt=\"OTA icon\"  onclick=\"document.location.href='../s2web/wizfi_ota.html'\"/><p onclick=\"document.location.href='../s2web/wizfi_ota.html'\">Change to OTA Mode</p>"
#define S2WEB_MAIN_5	"<img src=\"../images/gpio_icon.png\" width=\"150\" height=\"120\" style=\"vertical-align:middle\" alt=\"GPIO icon\"  onclick=\"document.location.href='../s2web/gpio_control_main.html'\"/><p onclick=\"document.location.href='../s2web/gpio_control_main.html'\">GPIO Control</p>"
#define S2WEB_MAIN_6	"<img src=\"../images/serial_setting_icon.png\" width=\"150\" height=\"120\" style=\"vertical-align:middle\" alt=\"Serial Setting icon\"  onclick=\"document.location.href='../s2web/serial_info_setting.html'\"/><p onclick=\"document.location.href='../s2web/serial_info_setting.html'\">Serial Setting</p>"
#define S2WEB_MAIN_7	"<img src=\"../images/user_information_icon.png\" width=\"200\" height=\"120\" style=\"vertical-align:middle\" alt=\"User Information Setting icon\"  onclick=\"document.location.href='../s2web/change_user_info.html'\"/><p onclick=\"document.location.href='../s2web/change_user_info.html'\">User Information Setting</p>"

	if ( g_wxProfile.s2web_main[0]!='0' )	wiced_tcp_stream_write(stream, S2WEB_MAIN_1, sizeof(S2WEB_MAIN_1)-1);
	if ( g_wxProfile.s2web_main[1]!='0' )	wiced_tcp_stream_write(stream, S2WEB_MAIN_2, sizeof(S2WEB_MAIN_2)-1);
	if ( g_wxProfile.s2web_main[2]!='0' )	wiced_tcp_stream_write(stream, S2WEB_MAIN_3, sizeof(S2WEB_MAIN_3)-1);
	if ( g_wxProfile.s2web_main[3]!='0' )	wiced_tcp_stream_write(stream, S2WEB_MAIN_4, sizeof(S2WEB_MAIN_4)-1);
	if ( g_wxProfile.s2web_main[4]!='0' )	wiced_tcp_stream_write(stream, S2WEB_MAIN_5, sizeof(S2WEB_MAIN_5)-1);
	if ( g_wxProfile.s2web_main[5]!='0' )	wiced_tcp_stream_write(stream, S2WEB_MAIN_6, sizeof(S2WEB_MAIN_6)-1);
	if ( g_wxProfile.s2web_main[6]!='0' )	wiced_tcp_stream_write(stream, S2WEB_MAIN_7, sizeof(S2WEB_MAIN_7)-1);

    return 0;
}

#endif

#ifdef BUILD_WIZFI250	// kaizen 20131210 ID1147 Added function about getting mac address
static const char* ssid_hexlist = "0123456789ABCDEF";
#define SSID_HEX_VAL( x )  ssid_hexlist[(int)((x) & 0xF)]
#endif

static int get_info_ota( const char* url, wiced_tcp_stream_t* stream, void* arg )
{
#ifdef BUILD_WIZFI250 // kaizen 20131210 ID1147 Added function about getting mac address
	wiced_mac_t my_mac;
	char ap_ssid[33] = {0, };
	char message[256] = {0, };

	wiced_wifi_get_mac_address( &my_mac );

	strcpy( ap_ssid, "WIZFI_OTA_");
	int i;
	int pos = strlen(ap_ssid);
	for( i = 0; i < 6; i++ )
	{
		ap_ssid[pos+i*2]   = SSID_HEX_VAL((my_mac.octet[i] >> 4 ) );
		ap_ssid[pos+i*2+1] = SSID_HEX_VAL((my_mac.octet[i] >> 0 ) );
	}

	//if( strncmp((char*)g_wxProfile.custom_idx,"SHINHEUNG",sizeof("SHINHEUNG")) == 0 )								// kaizen 20140410 ID1168 Customize for ShinHeung
	if( CHKCUSTOM("SHINHEUNG") )
	{
		sprintf(message,"WLAN Interface will be restart. Web Server stopped. Reconnect to %s\r\n",ap_ssid);
	}
	else
	{
		sprintf(message,"WizFi250 will be restart. Web Server stopped. Reconnect to %s\r\n",ap_ssid);
	}

	wiced_tcp_stream_write( stream, resource_s2web_DIR_conn_setting_response_html_ota_response_start, sizeof( resource_s2web_DIR_conn_setting_response_html_ota_response_start ) - 1 );
	wiced_tcp_stream_write( stream, message, sizeof(message) -1 );
	wiced_tcp_stream_write( stream, resource_s2web_DIR_conn_setting_response_html_ota_response_end, sizeof(resource_s2web_DIR_conn_setting_response_html_ota_response_end) -1 );

#endif

	return 1;
}



static int process_ota_go( const char* url, wiced_tcp_stream_t* stream, void* arg )
{
	wiced_start_ota_upgrade();

#if 0	// kaizen 20131210 delete wiced_http_server_stop function because WizFi250 will restart.
	/* Config has been set. Turn off HTTP server */
#ifdef WICED_ENABLE_HTTPS				// kaizen 20130624 ID1078 - In order to do not use HTTPS page
    wiced_http_server_stop(http_server);
#else
    wiced_http_server_stop(http_sta_server);
#endif
#endif // #if 0

    return 1;
}


// sekim 20130423 serial-to-web demo
char g_s2web_data[256];
static int process_s2web( const char* url, wiced_tcp_stream_t* stream, void* arg )
{
    wiced_tcp_stream_write( stream, resource_s2web_DIR_table_html, sizeof( resource_s2web_DIR_table_html ) - 1 );

    wiced_tcp_stream_write( stream, g_s2web_data, strlen(g_s2web_data) );
    wiced_tcp_stream_write( stream, resource_s2web_DIR_table_html_row_end, sizeof( resource_s2web_DIR_table_html_row_end ) - 1 );

    wiced_tcp_stream_write( stream, resource_s2web_DIR_table_html_list_end, sizeof(resource_s2web_DIR_table_html_list_end) -1 );

    return 0;
}

// kaizen 20130712 ID1099 For Setting TCP/UDP Connection using web server
static int process_config_s2w_value( const char* url, wiced_tcp_stream_t* stream, void* arg )
{
#ifdef BUILD_WIZFI250
	WT_PROFILE temp_dct;
	UINT8	dummy_ip[16];
	UINT32	dummy_port;
	UINT8	status = 0;
	UINT8   temp_string[1024];
	char   *ptr;
	char    conf_method[20];
	char    wifi_mode[20];

	wiced_dct_read_app_section( &temp_dct, sizeof(WT_PROFILE) );

	ptr = strtok((char*)url,"^");
	strcpy((char*)temp_string, (char*)ptr);

	ptr = strtok((char*)temp_string,"/");

	while( ptr != NULL )
	{
		if( strncmp(ptr,"PT:",3) == 0 )		// protocol
		{
			ptr += 3;
			strcpy((char*)temp_dct.scon_opt2,(char*)ptr);
		}
		else if( strncmp(ptr,"DI:",3) == 0 ) // destination IP
		{
			ptr += 3;
			status = WXParse_Ip((UINT8*)ptr,(UINT8*)&dummy_ip);
			if(status == 0)		strcpy((char*)temp_dct.scon_remote_ip,(char*)ptr);
			else				break;
		}
		else if( strncmp(ptr,"DP:",3) == 0 )	// destination Port
		{
			ptr += 3;
			status = WXParse_Int((UINT8*)ptr, &dummy_port);
			if( !is_valid_port(dummy_port) )
			{
				status = 1;
				break;
			}

			temp_dct.scon_remote_port = dummy_port;
		}
		else if( strncmp(ptr,"SP:",3) == 0 )
		{
			ptr += 3;
			status = WXParse_Int((UINT8*)ptr, &dummy_port);
			if( !is_valid_port(dummy_port) )
			{
				status = 1;
				break;
			}

			temp_dct.scon_local_port = dummy_port;
		}
		else if( strncmp(ptr,"CM:",3) == 0 )
		{
			ptr += 3;
			strcpy((char*)conf_method,(char*)ptr);
			conf_method[strlen(conf_method)] = '\0';
		}
		else if( strncmp(ptr,"MD:",3) == 0 )
		{
			if( strcmp(conf_method,"conn_setting") == 0 )
			{
				ptr += 3;
				strcpy((char*)wifi_mode,(char*)ptr);
				wifi_mode[strlen(wifi_mode)] = '\0';

				if( strcmp((char*)wifi_mode,"AP") == 0 )			temp_dct.wifi_mode = AP_MODE;
				else if( strcmp((char*)wifi_mode,"STATION") == 0 )	temp_dct.wifi_mode = STATION_MODE;
				else
				{
					status = 1;
					break;
				}
			}
			else if( strcmp(conf_method,"wps_pbc") == 0 || strcmp(conf_method,"wps_pin") == 0)
			{
				ptr += 3;
				temp_dct.wifi_dhcp = 1;
			}
		}

		ptr = strtok(NULL, "/");
	}

	if(status != 0)
	{
		wiced_tcp_stream_write( stream, resource_s2web_DIR_conn_setting_response_html_fail_response, sizeof(resource_s2web_DIR_conn_setting_response_html_fail_response) -1 );
		return 0;
	}

	//if( strncmp((char*)g_wxProfile.custom_idx,"SHINHEUNG",sizeof("SHINHEUNG")) == 0 )								// kaizen 20140410 ID1168 Customize for ShinHeung
	if( CHKCUSTOM("SHINHEUNG") )
	{
		temp_dct.scon_opt1[0]='S';
		temp_dct.scon_opt1[1]='\0';
		temp_dct.scon_datamode = COMMAND_MODE;
		wiced_dct_write_app_section( &temp_dct, sizeof(WT_PROFILE) );

		if ( temp_dct.wifi_mode == AP_MODE && strcmp(conf_method,"conn_setting") == 0 )
		{
			wiced_tcp_stream_write( stream, resource_customWeb_DIR_ShinHeung_DIR_conn_setting_response_SH_html_success_AP_mode_response, sizeof(resource_customWeb_DIR_ShinHeung_DIR_conn_setting_response_SH_html_success_AP_mode_response) -1 );
		}
		else if ( temp_dct.wifi_mode == STATION_MODE  && strcmp(conf_method,"conn_setting") == 0 )
		{
			wiced_tcp_stream_write( stream, resource_customWeb_DIR_ShinHeung_DIR_conn_setting_response_SH_html_success_response, sizeof(resource_customWeb_DIR_ShinHeung_DIR_conn_setting_response_SH_html_success_response) -1 );
		}
	}
	else
	{
		temp_dct.scon_opt1[0]='S';
		temp_dct.scon_opt1[1]='\0';
		temp_dct.scon_datamode = DATA_MODE;
		wiced_dct_write_app_section( &temp_dct, sizeof(WT_PROFILE) );

		// kaizen 20131113 ID1135 Added AP Mode Setting Using Web Server
		if ( temp_dct.wifi_mode == AP_MODE && strcmp(conf_method,"conn_setting") == 0 )
		{
			wiced_tcp_stream_write( stream, resource_s2web_DIR_conn_setting_response_html_success_AP_mode_response, sizeof(resource_s2web_DIR_conn_setting_response_html_success_AP_mode_response) -1 );
		}
		else if ( temp_dct.wifi_mode == STATION_MODE  && strcmp(conf_method,"conn_setting") == 0 )
		{
			wiced_tcp_stream_write( stream, resource_s2web_DIR_conn_setting_response_html_success_response, sizeof(resource_s2web_DIR_conn_setting_response_html_success_response) -1 );
		}

		if( strcmp(conf_method,"wps_pbc") == 0 )
			wiced_tcp_stream_write( stream, resource_s2web_DIR_conn_setting_response_html_success_wps_pbc_respose, sizeof(resource_s2web_DIR_conn_setting_response_html_success_wps_pbc_respose) -1 );
		else if( strcmp(conf_method,"wps_pin") == 0 )
			wiced_tcp_stream_write( stream, resource_s2web_DIR_conn_setting_response_html_success_wps_pin_respose, sizeof(resource_s2web_DIR_conn_setting_response_html_success_wps_pin_respose) -1 );
	}

#endif // BUILD_WIZFI250
	return 0;
}

// kaizen 20131113 ID1135 Added AP Mode Setting Using Web Server
static int process_config_s2w_ap_value( const char* url, wiced_tcp_stream_t* stream, void* arg )
{
#ifdef BUILD_WIZFI250
	WT_PROFILE temp_dct;
	UINT8	len;
	UINT8	dummy_ip[16];
	UINT8	status = 0;
	UINT8   temp_string[1024];
	UINT8	str_security[9];
	wiced_security_t sec_type = WICED_SECURITY_OPEN;
	char   *ptr;


	wiced_dct_read_app_section( &temp_dct, sizeof(WT_PROFILE) );

	ptr = strtok((char*)url,"^");
	strcpy((char*)temp_string, (char*)ptr);

	ptr = strtok((char*)temp_string,"/");

	while( ptr != NULL )
	{
		if( strncmp(ptr,"SD:",3) == 0 )									// SSID
		{
			ptr += 3;
			len = strlen((char*)ptr);
			if( len > WX_MAX_SSID_LEN+1 )
			{
				status = 1;
				break;
			}
			memcpy( (char*)temp_dct.ap_mode_ssid, (char*)ptr, len);
			temp_dct.ap_mode_ssid[len] = '\0';
		}
		else if( strncmp(ptr,"SS:",3) == 0 )							// Security
		{
			ptr += 3;
			strcpy((char*)str_security, (char*)ptr);
			sec_type = str_to_authtype(upstr((char*)str_security));

			if(sec_type == -1)
			{
				status = 1;
				break;
			}

			temp_dct.ap_mode_authtype = sec_type;
		}
		else if( strncmp(ptr,"PK:",3) == 0 )							// Security Key
		{
			ptr += 3;

			if(sec_type == WICED_SECURITY_OPEN )						// OPEN
			{
				strcpy( (char*)temp_dct.ap_mode_keydata, "");
			}
			else														// WPA/WPA2
			{
				len = strlen( (char*)ptr );
				if(len > WSEC_MAX_PSK_LEN)
				{
					status = 1;
					break;
				}

				temp_dct.ap_mode_keylen = len;
				memcpy( (char*)temp_dct.ap_mode_keydata,(char*)ptr,len );
				temp_dct.ap_mode_keydata[len] = '\0';
			}
		}
		else if( strncmp(ptr,"WI:",3) == 0 )							// Wi-Fi IP
		{
			ptr += 3;
			status = WXParse_Ip((UINT8*)ptr, (UINT8*)&dummy_ip);
			if(status != 0 )	break;

			strcpy((char*)temp_dct.wifi_ip, (char*)ptr);
		}
		else if( strncmp(ptr,"GI:",3) == 0 ) 							// Gateway IP
		{
			ptr += 3;
			status = WXParse_Ip((UINT8*)ptr,(UINT8*)&dummy_ip);
			if(status != 0)		break;

			strcpy((char*)temp_dct.wifi_gateway, (char*)ptr);
		}
		else if( strncmp(ptr,"SM:",3) == 0 )							// Subnet Mask
		{
			ptr += 3;
			status = WXParse_Ip((UINT8*)ptr,(UINT8*)&dummy_ip);
			if(status != 0)		break;

			strcpy((char*)temp_dct.wifi_mask, (char*)ptr);
		}

		ptr = strtok(NULL, "/");
	}


	if(status != 0)
	{
		wiced_tcp_stream_write( stream, resource_s2web_DIR_conn_setting_response_html_fail_set_ap_response, sizeof(resource_s2web_DIR_conn_setting_response_html_fail_set_ap_response) -1 );
		return 0;
	}

	//if( strncmp((char*)g_wxProfile.custom_idx,"SHINHEUNG",sizeof("SHINHEUNG")) == 0 )								// kaizen 20140410 ID1168 Customize for ShinHeung
	if( CHKCUSTOM("SHINHEUNG") )
	{
		// Set Message Level
		temp_dct.msgLevel = 2;

		// Set SFORM
		memcpy(temp_dct.xrdf_main,"110001110", sizeof(temp_dct.xrdf_main));
		temp_dct.xrdf_data[0] = 0x7b;	temp_dct.xrdf_data[1] = 0x3b;	temp_dct.xrdf_data[2] = 0x7d;
		temp_dct.xrdf_data[3] = 0x00;	temp_dct.xrdf_data[4] = 0x0a;

		// Set Serial
		temp_dct.usart_init_structure.USART_BaudRate 			= 460800;
		temp_dct.usart_init_structure.USART_WordLength          = USART_WordLength_8b;
		temp_dct.usart_init_structure.USART_Parity              = USART_Parity_No;
		temp_dct.usart_init_structure.USART_StopBits            = USART_StopBits_1;
		temp_dct.usart_init_structure.USART_HardwareFlowControl = USART_HardwareFlowControl_RTS_CTS;
		temp_dct.usart_init_structure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;

		wiced_dct_write_app_section( &temp_dct, sizeof(WT_PROFILE) );
		wiced_tcp_stream_write( stream, resource_customWeb_DIR_ShinHeung_DIR_conn_setting_response_SH_html_success_set_ap_response, sizeof(resource_customWeb_DIR_ShinHeung_DIR_conn_setting_response_SH_html_success_set_ap_response) -1 );
	}
	else
	{
		wiced_dct_write_app_section( &temp_dct, sizeof(WT_PROFILE) );
		wiced_tcp_stream_write( stream, resource_s2web_DIR_conn_setting_response_html_success_set_ap_response, sizeof(resource_s2web_DIR_conn_setting_response_html_success_set_ap_response) -1 );
	}


#endif
	return 0;
}

// kaizen 20131113 ID1135 Added AP Mode Setting Using Web Server
static int process_config_s2w_reset( const char* url, wiced_tcp_stream_t* stream, void* arg )
{
#ifdef BUILD_WIZFI250
	WXS2w_SystemReset();
#endif

	return 0;
}


#ifdef BUILD_WIZFI250		// kaizen 20131030 ID1134 Added GPIO Control Page Using Web Server
#define WEB_GPIO_ELEM_NUM 		"var elem_num = "
#define WEB_GPIO_LABEL_NAME 	";\n var labelname = \""
#define WEB_GPIO_FIELD_NAME		"\";\n var fieldname  = \"v"
#define WEB_GPIO_FIELD_MODE		"\";\n var gpio_mode = \""
#define WEB_GPIO_FIELD_VALUE	"var gpio_value = \""
#define WEB_GPIO_FIELD_END		"\";\n"

#define GPIO_MODE_INPUT			"INPUT"
#define GPIO_MODE_OUTPUT		"OUTPUT"
#define GPIO_VALUE_HIGH			"HIGH"
#define GPIO_VALUE_LOW			"LOW"
#endif


// kaizen 20131030 ID1134 Added GPIO Control Page Using Web Server
int get_gpio_setting( const char* url, wiced_tcp_stream_t* stream, void* arg )
{
#ifdef BUILD_WIZFI250
	const char* end_str_ptr;
    int end_str_size;
    int gpio_index, len=0;
    char config_count[2] = {'0','0'};
    char gpio_label_name[8] = {0};

	WT_PROFILE temp_dct = g_wxProfile;
	//wiced_dct_read_app_section( &temp_dct, sizeof(WT_PROFILE) );


	// Send GPIO Control Main Page
    wiced_tcp_stream_write(stream, resource_s2web_DIR_gpio_control_get_html, sizeof(resource_s2web_DIR_gpio_control_get_html)-1);


	for(gpio_index=0; gpio_index< USER_GPIO_MAX_COUNT; gpio_index++)
	{
	    // Send GPIO Elemnt Information
	    wiced_tcp_stream_write(stream, resource_s2web_DIR_gpio_control_get_html_get_gpio_mode, sizeof(resource_s2web_DIR_gpio_control_get_html_get_gpio_mode)-1);
		wiced_tcp_stream_write(stream, WEB_GPIO_ELEM_NUM, sizeof(WEB_GPIO_ELEM_NUM)-1);
		wiced_tcp_stream_write(stream, config_count, sizeof(config_count));

		// Send GPIO Label Information
		sprintf(gpio_label_name, "GPIO_%d",temp_dct.user_gpio_conifg[gpio_index].gpio_num);
		len = strlen(gpio_label_name);
		gpio_label_name[len] = '\0';

		wiced_tcp_stream_write(stream, WEB_GPIO_LABEL_NAME, sizeof(WEB_GPIO_LABEL_NAME)-1);
		//wiced_tcp_stream_write(stream, gpio_label_name, sizeof(gpio_label_name)-1);
		wiced_tcp_stream_write(stream, gpio_label_name, len);

		wiced_tcp_stream_write(stream, WEB_GPIO_FIELD_NAME, sizeof(WEB_GPIO_FIELD_NAME)-1);
		wiced_tcp_stream_write(stream, config_count, sizeof(config_count));


		// Send GPIO Mode Information
		wiced_tcp_stream_write(stream, WEB_GPIO_FIELD_MODE, sizeof(WEB_GPIO_FIELD_MODE)-1);

		if( temp_dct.user_gpio_conifg[gpio_index].mode == 1 )			// GPIO OUTPUT MODE
		{
			wiced_tcp_stream_write(stream, GPIO_MODE_OUTPUT, sizeof(GPIO_MODE_OUTPUT)-1);

			// Send GPIO_FIELD_END Data
			end_str_ptr = resource_s2web_DIR_gpio_control_get_html_get_gpio_mode_end;
		    end_str_size = sizeof(resource_s2web_DIR_gpio_control_get_html_get_gpio_mode_end)-1;

		    wiced_tcp_stream_write(stream, WEB_GPIO_FIELD_END, sizeof(WEB_GPIO_FIELD_END)-1);
			wiced_tcp_stream_write(stream, end_str_ptr, end_str_size);

			// Send GPIO Value Information
			wiced_tcp_stream_write(stream, resource_s2web_DIR_gpio_control_get_html_get_gpio_value, sizeof(resource_s2web_DIR_gpio_control_get_html_get_gpio_value)-1);
			wiced_tcp_stream_write(stream, WEB_GPIO_FIELD_VALUE, sizeof(WEB_GPIO_FIELD_VALUE)-1);

			if( temp_dct.user_gpio_conifg[gpio_index].gpio_set_value == GPIO_SET_LOW )
				wiced_tcp_stream_write(stream, GPIO_VALUE_LOW, sizeof(GPIO_VALUE_LOW)-1);
			else if( temp_dct.user_gpio_conifg[gpio_index].gpio_set_value == GPIO_SET_HIGH )
				wiced_tcp_stream_write(stream, GPIO_VALUE_HIGH, sizeof(GPIO_VALUE_HIGH)-1);

			end_str_ptr = resource_s2web_DIR_gpio_control_get_html_get_gpio_value_end;
		    end_str_size = sizeof(resource_s2web_DIR_gpio_control_get_html_get_gpio_value_end)-1;
		}

		else if( temp_dct.user_gpio_conifg[gpio_index].mode == 0 )		// GPIO INPUT MODE
		{
			wiced_tcp_stream_write(stream, GPIO_MODE_INPUT, sizeof(GPIO_MODE_INPUT)-1);

			// Send GPIO_FIELD_END Data
			end_str_ptr = resource_s2web_DIR_gpio_control_get_html_get_gpio_mode_end;
		    end_str_size = sizeof(resource_s2web_DIR_gpio_control_get_html_get_gpio_mode_end)-1;

		    wiced_tcp_stream_write(stream, WEB_GPIO_FIELD_END, sizeof(WEB_GPIO_FIELD_END)-1);
			wiced_tcp_stream_write(stream, end_str_ptr, end_str_size);

			// Send GPIO Value Information
			wiced_tcp_stream_write(stream, resource_s2web_DIR_gpio_control_get_html_get_gpio_value, sizeof(resource_s2web_DIR_gpio_control_get_html_get_gpio_value)-1);
			wiced_tcp_stream_write(stream, WEB_GPIO_FIELD_VALUE, sizeof(WEB_GPIO_FIELD_VALUE)-1);

			if( wiced_gpio_input_get(temp_dct.user_gpio_conifg[gpio_index].gpio_num - 1) == 0 )
				wiced_tcp_stream_write(stream, GPIO_VALUE_LOW, sizeof(GPIO_VALUE_LOW)-1);
			else if( wiced_gpio_input_get(temp_dct.user_gpio_conifg[gpio_index].gpio_num - 1) == 1 )
				wiced_tcp_stream_write(stream, GPIO_VALUE_HIGH, sizeof(GPIO_VALUE_HIGH)-1);

			end_str_ptr = resource_s2web_DIR_gpio_control_get_html_get_gpio_value_end;
		    end_str_size = sizeof(resource_s2web_DIR_gpio_control_get_html_get_gpio_value_end)-1;
		}


		wiced_tcp_stream_write(stream, WEB_GPIO_FIELD_END, sizeof(WEB_GPIO_FIELD_END)-1);
		wiced_tcp_stream_write(stream, end_str_ptr, end_str_size);

        if (config_count[1] == '9')
        {
            ++config_count[0];
            config_count[1] = '0';
        }
        else
        {
            ++config_count[1];
        }
	}

	wiced_tcp_stream_write(stream, resource_s2web_DIR_gpio_control_get_html_get_gpio_end, sizeof(resource_s2web_DIR_gpio_control_get_html_get_gpio_end)-1 );
	wiced_tcp_stream_write(stream, resource_s2web_DIR_gpio_control_get_html_gpio_settings_bottom, sizeof(resource_s2web_DIR_gpio_control_get_html_gpio_settings_bottom)-1 );
#endif

    return 0;
}

static int set_gpio_setting			 ( const char* url, wiced_tcp_stream_t* stream, void* arg )
{
#ifdef BUILD_WIZFI250
	const char* end_str_ptr;
    int end_str_size;
    int gpio_index, len=0;
    char config_count[2] = {'0','0'};
    char gpio_label_name[8] = {0};

	WT_PROFILE temp_dct = g_wxProfile;

	// Send GPIO Control Main Page
    wiced_tcp_stream_write(stream, resource_s2web_DIR_gpio_control_set_html, sizeof(resource_s2web_DIR_gpio_control_set_html)-1);


	for(gpio_index=0; gpio_index< USER_GPIO_MAX_COUNT; gpio_index++)
	{
	    // Send GPIO Elemnt Information
	    wiced_tcp_stream_write(stream, resource_s2web_DIR_gpio_control_set_html_get_gpio_mode, sizeof(resource_s2web_DIR_gpio_control_set_html_get_gpio_mode)-1);
		wiced_tcp_stream_write(stream, WEB_GPIO_ELEM_NUM, sizeof(WEB_GPIO_ELEM_NUM)-1);
		wiced_tcp_stream_write(stream, config_count, sizeof(config_count));

		// Send GPIO Label Information
		sprintf(gpio_label_name, "GPIO_%d",temp_dct.user_gpio_conifg[gpio_index].gpio_num);
		len = strlen(gpio_label_name);
		gpio_label_name[len] = '\0';

		wiced_tcp_stream_write(stream, WEB_GPIO_LABEL_NAME, sizeof(WEB_GPIO_LABEL_NAME)-1);
		wiced_tcp_stream_write(stream, gpio_label_name, len);

		wiced_tcp_stream_write(stream, WEB_GPIO_FIELD_NAME, sizeof(WEB_GPIO_FIELD_NAME)-1);
		wiced_tcp_stream_write(stream, config_count, sizeof(config_count));


		// Send GPIO Mode Information
		wiced_tcp_stream_write(stream, WEB_GPIO_FIELD_MODE, sizeof(WEB_GPIO_FIELD_MODE)-1);

		if( temp_dct.user_gpio_conifg[gpio_index].mode == 1 )			// GPIO OUTPUT MODE
		{
			wiced_tcp_stream_write(stream, GPIO_MODE_OUTPUT, sizeof(GPIO_MODE_OUTPUT)-1);

			// Send GPIO_FIELD_END Data
			end_str_ptr = resource_s2web_DIR_gpio_control_set_html_get_gpio_mode_end;
		    end_str_size = sizeof(resource_s2web_DIR_gpio_control_set_html_get_gpio_mode_end)-1;

		    wiced_tcp_stream_write(stream, WEB_GPIO_FIELD_END, sizeof(WEB_GPIO_FIELD_END)-1);
			wiced_tcp_stream_write(stream, end_str_ptr, end_str_size);

			// Send GPIO Value Information
			wiced_tcp_stream_write(stream, resource_s2web_DIR_gpio_control_set_html_get_gpio_value, sizeof(resource_s2web_DIR_gpio_control_set_html_get_gpio_value)-1);
			wiced_tcp_stream_write(stream, WEB_GPIO_FIELD_VALUE, sizeof(WEB_GPIO_FIELD_VALUE)-1);

			if( temp_dct.user_gpio_conifg[gpio_index].gpio_set_value == GPIO_SET_LOW )
				wiced_tcp_stream_write(stream, GPIO_VALUE_LOW, sizeof(GPIO_VALUE_LOW)-1);
			else if( temp_dct.user_gpio_conifg[gpio_index].gpio_set_value == GPIO_SET_HIGH )
				wiced_tcp_stream_write(stream, GPIO_VALUE_HIGH, sizeof(GPIO_VALUE_HIGH)-1);

			end_str_ptr = resource_s2web_DIR_gpio_control_set_html_get_gpio_value_end;
		    end_str_size = sizeof(resource_s2web_DIR_gpio_control_set_html_get_gpio_value_end)-1;
		}

		else if( temp_dct.user_gpio_conifg[gpio_index].mode == 0 )		// GPIO INPUT MODE
		{
			wiced_tcp_stream_write(stream, GPIO_MODE_INPUT, sizeof(GPIO_MODE_INPUT)-1);

			// Send GPIO_FIELD_END Data
			end_str_ptr = resource_s2web_DIR_gpio_control_set_html_get_gpio_mode_end;
		    end_str_size = sizeof(resource_s2web_DIR_gpio_control_set_html_get_gpio_mode_end)-1;

		    wiced_tcp_stream_write(stream, WEB_GPIO_FIELD_END, sizeof(WEB_GPIO_FIELD_END)-1);
			wiced_tcp_stream_write(stream, end_str_ptr, end_str_size);

			// Send GPIO Value Information
			wiced_tcp_stream_write(stream, resource_s2web_DIR_gpio_control_set_html_get_gpio_value, sizeof(resource_s2web_DIR_gpio_control_set_html_get_gpio_value)-1);
			wiced_tcp_stream_write(stream, WEB_GPIO_FIELD_VALUE, sizeof(WEB_GPIO_FIELD_VALUE)-1);

			if( wiced_gpio_input_get(temp_dct.user_gpio_conifg[gpio_index].gpio_num - 1) == 0 )
				wiced_tcp_stream_write(stream, GPIO_VALUE_LOW, sizeof(GPIO_VALUE_LOW)-1);
			else if( wiced_gpio_input_get(temp_dct.user_gpio_conifg[gpio_index].gpio_num - 1) == 1 )
				wiced_tcp_stream_write(stream, GPIO_VALUE_HIGH, sizeof(GPIO_VALUE_HIGH)-1);

			end_str_ptr = resource_s2web_DIR_gpio_control_set_html_get_gpio_value_end;
		    end_str_size = sizeof(resource_s2web_DIR_gpio_control_set_html_get_gpio_value_end)-1;
		}


		wiced_tcp_stream_write(stream, WEB_GPIO_FIELD_END, sizeof(WEB_GPIO_FIELD_END)-1);
		wiced_tcp_stream_write(stream, end_str_ptr, end_str_size);

        if (config_count[1] == '9')
        {
            ++config_count[0];
            config_count[1] = '0';
        }
        else
        {
            ++config_count[1];
        }
	}

	wiced_tcp_stream_write(stream, resource_s2web_DIR_gpio_control_set_html_get_gpio_end, sizeof(resource_s2web_DIR_gpio_control_set_html_get_gpio_end)-1 );
	wiced_tcp_stream_write(stream, resource_s2web_DIR_gpio_control_set_html_gpio_settings_bottom, sizeof(resource_s2web_DIR_gpio_control_set_html_gpio_settings_bottom)-1 );
#endif

    return 0;
}

// kaizen 20131030 ID1134 Added GPIO Control Page Using Web Server
static int gpio_config_save( const char* params, wiced_tcp_stream_t* stream, void* arg )
{
#ifdef BUILD_WIZFI250
	typedef struct s_gpio_info
	{
		char gpio_num[10];
		char gpio_mode[10];
		char gpio_value[10];
	}s_gpio_info;
	s_gpio_info gpio_info[USER_GPIO_MAX_COUNT];

	UINT8   temp_string[1024];
	UINT8	temp_string2[USER_GPIO_MAX_COUNT][50];
	UINT8  column_cnt=0;
	UINT8  field_cnt=0;
	UINT8  len;
	char   *column;
	char   *field;
	char   *ptr;
	char   szBuff[50]  = {0, };

	UINT32 gpio_num;
	UINT32 gpio_value = 0;



	column = strtok((char*)params,"^");
	strcpy((char*)temp_string, (char*)column);

	column = strtok((char*)temp_string,"&");

	// seperate coulumn
	while( column != NULL )
	{
		strcpy((char*)temp_string2[column_cnt],(char*)column);

		column = strtok(NULL, "&");
		column_cnt++;
	}

	// seperate field
	for(column_cnt=0;column_cnt<USER_GPIO_MAX_COUNT;column_cnt++)
	{
		field = strtok((char*)temp_string2[column_cnt],",");

		while(field != NULL)
		{
			switch(field_cnt)
			{
				case 0:
					ptr = strstr((char*)field, "_");
					ptr++;
					len = strlen((char*)ptr);
					strncpy( gpio_info[column_cnt].gpio_num, ptr, len );
					gpio_info[column_cnt].gpio_num[len] = '\0';
					break;
				case 1:
					len = strlen((char*)field);
					strncpy((char*)gpio_info[column_cnt].gpio_mode,(char*)field,len);
					gpio_info[column_cnt].gpio_mode[len] = '\0';
					break;
				case 2:
					len = strlen((char*)field);
					strncpy((char*)gpio_info[column_cnt].gpio_value,(char*)field,strlen((char*)field));
					gpio_info[column_cnt].gpio_value[len] = '\0';
					break;
				default:
					break;
			}
			field = strtok(NULL, ",");
			field_cnt++;
		}
		field_cnt = 0;
	}

	// Adjust GPIO information to g_wxProfile
	for(column_cnt=0;column_cnt<USER_GPIO_MAX_COUNT;column_cnt++)
	{
		WXParse_Int((UINT8*)gpio_info[column_cnt].gpio_num, &gpio_num);
		if(g_wxProfile.user_gpio_conifg[column_cnt].gpio_num == gpio_num )
		{
			if( strcmp(gpio_info[column_cnt].gpio_mode,"INPUT") == 0 )
			{
				sprintf(szBuff,"%d,%d,%d",2, gpio_num, INPUT_PULL_UP );
				WXCmd_FGPIO((UINT8 *)szBuff);
			}
			else if( strcmp(gpio_info[column_cnt].gpio_mode, "OUTPUT") == 0 )
			{
				sprintf(szBuff,"%d,%d,%d",2, gpio_num, OUTPUT_PUSH_PULL );
				WXCmd_FGPIO((UINT8 *)szBuff);

				if( strcmp(gpio_info[column_cnt].gpio_value, "HIGH") == 0 )
					gpio_value = 1;
				else if( strcmp(gpio_info[column_cnt].gpio_value, "LOW") == 0 )
					gpio_value = 0;

				sprintf(szBuff,"%d,%d,%d",1, gpio_num, gpio_value);
				WXCmd_FGPIO((UINT8 *)szBuff);
			}
		}
	}

	wiced_tcp_stream_write( stream, resource_s2web_DIR_conn_setting_response_html_success_gpio_setting_response, sizeof(resource_s2web_DIR_conn_setting_response_html_success_gpio_setting_response) -1 );
#endif

	return 0;
}

// kaizen 20131104 ID1133 Added Web Login Procedure
static int s2wlogin( const char* params, wiced_tcp_stream_t* stream, void* arg )
{
#ifdef BUILD_WIZFI250
	UINT8	   len;
	UINT8	   status = 0;
	UINT8      temp_string[1024];
	char   *ptr;


	ptr = strtok((char*)params,"^");
	strcpy((char*)temp_string, (char*)ptr);

	ptr = strtok((char*)temp_string,"/");

	while( ptr != NULL )
	{
		if( strncmp(ptr,"UN:",3) == 0 )									// USER_ID
		{
			ptr += 3;
			len = strlen((char*)ptr);

			if( len > MAX_USER_ID_SIZE || len <= 0 || strncmp((char*)g_wxProfile.user_id,ptr,len) != 0 )
			{
				status = 1;
				break;
			}
		}
		else if( strncmp(ptr,"PW:",3) == 0 )							// USER_PASSWORD
		{
			ptr += 3;
			len = strlen((char*)ptr);
			if( len > MAX_USER_PASSWORD_SIZE || len <= 0 || strncmp((char*)g_wxProfile.user_password,ptr,len) != 0 )
			{
				status = 1;
				break;
			}
		}

		ptr = strtok(NULL, "/");
	}


	if( CHKCUSTOM("ENCORED") )	status = 0;			// 20141024 ID1189 If using ENCORED custom firmware, It does not need id and password when connect web server.


	if( status != 0 )
	{
		wiced_tcp_stream_write( stream, resource_s2web_DIR_conn_setting_response_html_fail_login, sizeof(resource_s2web_DIR_conn_setting_response_html_fail_login) -1 );
	}
	else
	{
		//if( strncmp((char*)g_wxProfile.custom_idx,"SHINHEUNG",sizeof("SHINHEUNG")) == 0 )								// kaizen 20140410 ID1168 Customize for ShinHeung
		if( CHKCUSTOM("SHINHEUNG") )
		{
			wiced_tcp_stream_write( stream, resource_customWeb_DIR_ShinHeung_DIR_conn_setting_response_SH_html_success_login, sizeof(resource_customWeb_DIR_ShinHeung_DIR_conn_setting_response_SH_html_success_login) -1 );
		}
		else
		{
			wiced_tcp_stream_write( stream, resource_s2web_DIR_conn_setting_response_html_success_login, sizeof(resource_s2web_DIR_conn_setting_response_html_success_login) -1 );
		}
	}
#endif

	return 0;
}

// kaizen 20131118 ID1139 Added Static IP Setting Page in Station Mode
static int process_s2wconfig_station( const char* params, wiced_tcp_stream_t* stream, void* arg )
{
#ifdef BUILD_WIZFI250
	WT_PROFILE temp_dct;
	UINT8	len;
	UINT8	dummy_ip[16];
	UINT8	status = 0;
	UINT8   temp_string[1024];
	char   *ptr;


	wiced_dct_read_app_section( &temp_dct, sizeof(WT_PROFILE) );

	ptr = strtok((char*)params,"^");
	strcpy((char*)temp_string, (char*)ptr);

	ptr = strtok((char*)temp_string,"/");

	while( ptr != NULL )
	{
		if( strncmp(ptr,"DH:",3) == 0 )									// DHCP Mode
		{
			ptr += 3;
			len = strlen((char*)ptr);

			if( strncmp((char*)ptr,"STATIC",len) == 0 )
				temp_dct.wifi_dhcp = 0;
			else if( strncmp((char*)ptr,"DHCP",len) == 0 )
				temp_dct.wifi_dhcp = 1;
			else
			{
				status = 1;
				break;
			}
		}
		else if( strncmp(ptr,"WI:",3) == 0 )							// Wi-Fi IP
		{
			ptr += 3;
			status = WXParse_Ip((UINT8*)ptr, (UINT8*)&dummy_ip);
			if(status != 0 )	break;

			strcpy((char*)temp_dct.wifi_ip, (char*)ptr);
		}
		else if( strncmp(ptr,"GI:",3) == 0 ) 							// Gateway IP
		{
			ptr += 3;
			status = WXParse_Ip((UINT8*)ptr,(UINT8*)&dummy_ip);
			if(status != 0)		break;

			strcpy((char*)temp_dct.wifi_gateway, (char*)ptr);
		}
		else if( strncmp(ptr,"SM:",3) == 0 )							// Subnet Mask
		{
			ptr += 3;
			status = WXParse_Ip((UINT8*)ptr,(UINT8*)&dummy_ip);
			if(status != 0)		break;

			strcpy((char*)temp_dct.wifi_mask, (char*)ptr);
		}

		ptr = strtok(NULL, "/");
	}

	if(status != 0)
	{
		wiced_tcp_stream_write( stream, resource_s2web_DIR_conn_setting_response_html_fail_station_respose, sizeof(resource_s2web_DIR_conn_setting_response_html_fail_station_respose) -1 );
		return 0;
	}

	wiced_dct_write_app_section( &temp_dct, sizeof(WT_PROFILE) );


	//if( strncmp((char*)g_wxProfile.custom_idx,"SHINHEUNG",sizeof("SHINHEUNG")) == 0 )								// kaizen 20140410 ID1168 Customize for ShinHeung
	if( CHKCUSTOM("SHINHEUNG") )
	{
		wiced_tcp_stream_write( stream, resource_customWeb_DIR_ShinHeung_DIR_conn_setting_response_SH_html_success_station_respose, sizeof(resource_s2web_DIR_conn_setting_response_html_success_station_respose) -1 );
	}
	else
	{

		wiced_tcp_stream_write( stream, resource_s2web_DIR_conn_setting_response_html_success_station_respose, sizeof(resource_s2web_DIR_conn_setting_response_html_success_station_respose) -1 );
	}
#endif

	return 0;
}

// kaizen 20131210 ID1149 Added function about setting serial configuration
static int process_serial_setting	 ( const char* params, wiced_tcp_stream_t* stream, void* arg )
{
#ifdef BUILD_WIZFI250
	UINT8	len;
	UINT8	baudrate[10], parity[2], word_len[2], stop_bit[4], flow_ctrl[3];

	UINT8   temp_string[512];
	UINT8	cmd_string[256];
	char   *ptr;


	ptr = strtok((char*)params,"^");
	strcpy((char*)temp_string, (char*)ptr);

	ptr = strtok((char*)temp_string,"/");


	while( ptr != NULL )
	{
		if( strncmp(ptr,"BR:",3) == 0 )									// Baudrate
		{
			ptr += 3;
			len = strlen((char*)ptr);

			strncpy((char*)baudrate,(char*)ptr,len);
			baudrate[len]='\0';

		}
		else if( strncmp(ptr,"PR:",3) == 0 )							// Parity
		{
			ptr += 3;
			len = strlen((char*)ptr);

			strncpy((char*)parity,(char*)ptr,len);
			parity[len]='\0';

		}
		else if( strncmp(ptr,"WL:",3) == 0 )							// Word Length
		{
			ptr += 3;
			len = strlen((char*)ptr);

			strncpy((char*)word_len,(char*)ptr,len);
			word_len[len]='\0';

		}
		else if( strncmp(ptr,"SB:",3) == 0 )							// Stop Bit
		{
			ptr += 3;
			len = strlen((char*)ptr);

			strncpy((char*)stop_bit,(char*)ptr,len);
			stop_bit[len]='\0';

		}
		else if( strncmp(ptr,"FC:",3) == 0 )							// Flow Control
		{
			ptr += 3;
			len = strlen((char*)ptr);

			strncpy((char*)flow_ctrl,(char*)ptr,len);
			flow_ctrl[len]='\0';

		}

		ptr = strtok(NULL, "/");
	}

	wiced_tcp_stream_write( stream, resource_s2web_DIR_conn_setting_response_html_success_serial_setting_response, sizeof(resource_s2web_DIR_conn_setting_response_html_success_serial_setting_response) -1 );

	sprintf((char*)cmd_string,"%s,%s,%s,%s,%s,%s",baudrate,parity,word_len,stop_bit,flow_ctrl,"W");		// kaizen 20131211 ID1149 Added extra parameter in order to do not restart in WXCmd_USET function.
	WXCmd_USET((UINT8*)cmd_string);
#endif

	return 0;
}

// kaizen 20131210 ID1150 Added function in order to change user information as userID and userPW.
static int process_user_info_setting ( const char* params, wiced_tcp_stream_t* stream, void* arg )
{
#ifdef BUILD_WIZFI250
	WT_PROFILE temp_dct;

	UINT8	len, status = 0;
	UINT8   temp_string[256];
	char   *ptr;

	wiced_dct_read_app_section( &temp_dct, sizeof(WT_PROFILE) );

	ptr = strtok((char*)params,"^");
	strcpy((char*)temp_string, (char*)ptr);

	ptr = strtok((char*)temp_string,"/");

	while( ptr != NULL )
	{
		if( strncmp(ptr,"CI:",3) == 0 )									// Current ID
		{
			ptr += 3;
			len = strlen((char*)ptr);

			if( strncmp((char*)temp_dct.user_id,(char*)ptr, len) != 0 || len > MAX_USER_ID_SIZE || len <= 0 )
				status = 1;
		}
		else if( strncmp(ptr,"CP:",3) == 0 )							// Current Password
		{
			ptr += 3;
			len = strlen((char*)ptr);

			if( strncmp((char*)temp_dct.user_password,(char*)ptr, len) != 0 || len > MAX_USER_ID_SIZE || len <= 0 )
				status = 1;
		}
		else if( strncmp(ptr,"NI:",3) == 0 )							// New ID
		{
			ptr += 3;

			if( status != 1 )
			{
				len = strlen((char*)ptr);

				if( len > MAX_USER_ID_SIZE || len <= 0 )
				{
					status = 1;
					break;
				}

				strncpy((char*)temp_dct.user_id,(char*)ptr, len);
				temp_dct.user_id[len] = '\0';
			}
		}
		else if( strncmp(ptr,"NP:",3) == 0 )							// New Password
		{
			ptr += 3;

			if( status != 1 )
			{
				len = strlen((char*)ptr);

				if( len > MAX_USER_PASSWORD_SIZE || len <= 0 )
				{
					status = 1 ;
					break;
				}
				strncpy((char*)temp_dct.user_password,(char*)ptr, len);
				temp_dct.user_password[len] = '\0';
			}
		}

		ptr = strtok(NULL, "/");
	}

	if(status != 0)		// ERROR
		wiced_tcp_stream_write( stream, resource_s2web_DIR_conn_setting_response_html_fail_user_info_setting_response, sizeof(resource_s2web_DIR_conn_setting_response_html_fail_user_info_setting_response)-1 );
	else
	{
		wiced_dct_write_app_section( &temp_dct, sizeof(WT_PROFILE) );
		wiced_tcp_stream_write( stream, resource_s2web_DIR_conn_setting_response_html_success_user_info_setting_response, sizeof(resource_s2web_DIR_conn_setting_response_html_success_user_info_setting_response) -1 );
	}

#endif

	return 0;
}

// kaizen 20141024 ID1189 Modified in order to print LOGO image and title in web page depending on CUSTOM value.
#define PAGE_LOGOPATH_DEVNAME \
		"<script language=\"JavaScript\" type=\"text/javascript\">\r\n" \
		"\t var logo_path   = '%s' \r\n" \
		"\t var dev_name    = '%s' \r\n" \
		"\t sessionStorage.setItem(\"logo_path\",logo_path)\r\n" \
		"\t sessionStorage.setItem(\"dev_name\",dev_name)\r\n" \
		"</script>"

#ifdef BUILD_WIZFI250
// kaizen 20141024 ID1189 Modified in order to print LOGO image and title in web page depending on CUSTOM value.
static int process_get_custom_info ( const char* params, wiced_tcp_stream_t* stream, void* arg )
{
	char resp[1024];

	///////////////////////////////////////////////////////////////////////////////////////////////////
	// sekim 20150114 POSBANK Dummy Logo
	/*
	if( CHKCUSTOM("ENCORED") )
	{
		sprintf(resp,PAGE_LOGOPATH_DEVNAME,"../images/encored_logo.png","GetIT");
	}
	else
	{
		sprintf(resp,PAGE_LOGOPATH_DEVNAME,"../images/wiznet_logo.png","WizFi250");
	}
	*/
	if ( g_wxProfile.mext1_1==101 )
	{
		sprintf(resp,PAGE_LOGOPATH_DEVNAME,"../images/dummy_logo.png","");
	}
	else
	{
		if( CHKCUSTOM("ENCORED") )
		{
			sprintf(resp,PAGE_LOGOPATH_DEVNAME,"../images/encored_logo.png","GetIT");
		}
		else
		{
			sprintf(resp,PAGE_LOGOPATH_DEVNAME,"../images/wiznet_logo.png","WizFi250");
		}
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////

	wiced_tcp_stream_write( stream, resp, strlen(resp) );

	return 0;
}
#endif
