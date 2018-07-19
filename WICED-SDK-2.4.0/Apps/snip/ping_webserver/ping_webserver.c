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
 * ICMP Ping Webserver Application
 *
 * This application sends regular 'ping' packets (ICMP request-reply)
 * to the network gateway address. Ping results are printed to
 * the UART and to a webpage accessible by Wi-Fi clients on the same
 * network.
 *
 * Features demonstrated
 *  - Wi-Fi STA (client) mode to send regular ICMP pings to the gateway
 *  - Webserver to provide a webpage showing ping results
 *  - mDNS / DNS-SD Network Discovery
 *
 * Application Instructions
 *   1. Modify the CLIENT_AP_SSID/CLIENT_AP_PASSPHRASE Wi-Fi credentials
 *      in the wifi_config_dct.h header file to match your Wi-Fi access point
 *   2. Connect a PC terminal to the serial port of the WICED Eval board,
 *      then build and download the application as described in the WICED
 *      Quick Start Guide
 *
 * The terminal displays WICED startup information including the
 * IP address of the WICED device. The device then attempts to join the
 * Wi-Fi AP and send regular pings to the gateway. Ping replies are
 * printed to the UART.
 *
 * To view ping replies in a web browser:
 *   - Connect a Wi-Fi client such as a phone, tablet, or computer to
 *     the same Wi-Fi AP as the WICED device
 *   - Open a web browser and go to http://192.168.1.99
 *     (replace this IP address with the IP of the WICED device; the IP
 *      address of the WICED device is available on the terminal after
 *      WICED Config Mode completes)
 *   - If your computer has an mDNS (or Bonjour) network service browser,
 *     the WICED device can be found by using the browser to search for
 *     and connect to the service called "Ping App Webserver".
 *
 */

#include "wiced.h"
#include "ping_webserver.h"
#include "http_server.h"
#include "gedday.h"
#include "resources.h"


/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

#define PING_PERIOD  1000
#define PING_TIMEOUT  900

/******************************************************
 *                   Enumerations
 ******************************************************/

/******************************************************
 *                 Type Definitions
 ******************************************************/

/******************************************************
 *                    Structures
 ******************************************************/

/******************************************************
 *               Function Declarations
 ******************************************************/

static wiced_result_t send_ping              ( void* arg );
static int            process_ping           ( const char* url, wiced_tcp_stream_t* socket, void* arg );

/******************************************************
 *               Variable Definitions
 ******************************************************/

static char           ping_description[PING_DESCRIPTION_LEN];
static char           ping_results    [PING_HISTORY_LEN][PING_RESULT_LEN];
static wiced_mutex_t  ping_mutex;
static uint32_t       ping_start_index;
static uint32_t       ping_end_index;

static wiced_http_server_t http_server;
static wiced_timed_event_t ping_timed_event;
static wiced_ip_address_t  ping_target_ip;

START_OF_HTTP_PAGE_DATABASE(web_pages)
    ROOT_HTTP_PAGE_REDIRECT("/apps/ping_webserver/top.html"),
    { "/apps/ping_webserver/top.html",   "text/html",                WICED_STATIC_URL_CONTENT,  .url_content.static_data  = {resource_apps_DIR_ping_webserver_DIR_top_html, sizeof(resource_apps_DIR_ping_webserver_DIR_top_html) -1 }, },
    { "/ping.html",                      "text/html",                WICED_DYNAMIC_URL_CONTENT, .url_content.dynamic_data = {process_ping, 0 }, },
    { "/scripts/general_ajax_script.js", "application/javascript",   WICED_STATIC_URL_CONTENT,  .url_content.static_data  = {resource_scripts_DIR_general_ajax_script_js, sizeof(resource_scripts_DIR_general_ajax_script_js) - 1 }, },
    { "/images/favicon.ico",             "image/vnd.microsoft.icon", WICED_STATIC_URL_CONTENT,  .url_content.static_data  = {resource_images_DIR_favicon_ico, sizeof(resource_images_DIR_favicon_ico)}, },
    { "/images/brcmlogo.png",            "image/png",                WICED_STATIC_URL_CONTENT,  .url_content.static_data  = {resource_images_DIR_brcmlogo_png, sizeof(resource_images_DIR_brcmlogo_png)}, },
    { "/images/brcmlogo_line.png",       "image/png",                WICED_STATIC_URL_CONTENT,  .url_content.static_data  = {resource_images_DIR_brcmlogo_line_png, sizeof(resource_images_DIR_brcmlogo_line_png)}, },
END_OF_HTTP_PAGE_DATABASE()


/******************************************************
 *               Function Definitions
 ******************************************************/

void application_start( )
{
    /* Init data */
    ping_start_index = 0;
    ping_end_index   = 0;
    memset( ping_results,    ' ', sizeof( ping_results ) );
    memset( ping_description, 0, sizeof( ping_description ) );
    wiced_rtos_init_mutex(&ping_mutex);
    wiced_result_t result;

    /* Initialise the device */
    wiced_init( );

    /* Configure the device */
    //wiced_configure_device( app_config );  /* Config bypassed in local makefile and wifi_config_dct.h */

    /* Bring up the network on the STA interface */
    result = wiced_network_up( WICED_STA_INTERFACE, WICED_USE_EXTERNAL_DHCP_SERVER, NULL );

    if ( result == WICED_SUCCESS )
    {
        /* The ping target is the gateway */
        wiced_ip_get_gateway_address( WICED_STA_INTERFACE, &ping_target_ip );

        /* Setup a regular ping event and setup the callback to run in the networking worker thread */
        wiced_rtos_register_timed_event( &ping_timed_event, WICED_NETWORKING_WORKER_THREAD, &send_ping, PING_PERIOD, 0 );

        uint32_t ipv4 = GET_IPV4_ADDRESS(ping_target_ip);

        char ping_description_raw[200];
        sprintf(ping_description_raw, "Pinging %u.%u.%u.%u every %ums with a %ums timeout.",
                                                 (unsigned int)((ipv4 >> 24) & 0xFF),
                                                 (unsigned int)((ipv4 >> 16) & 0xFF),
                                                 (unsigned int)((ipv4 >>  8) & 0xFF),
                                                 (unsigned int)((ipv4 >>  0) & 0xFF),
                                                 PING_PERIOD,
                                                 PING_TIMEOUT);
        /* Print ping description to the UART */
        WPRINT_APP_INFO(("%s\r\n", ping_description_raw));

    /* Print ping description to HTML for the webpage */
    sprintf( ping_description, "%s%sLast %d replies ...", ping_description_raw, resource_apps_DIR_ping_webserver_DIR_table_html_desc1_end, PING_HISTORY_LEN );

    /* Start a web server to display ping results */
    wiced_http_server_start( &http_server, 80, web_pages, WICED_STA_INTERFACE );

    /* Start Gedday to advertise the webservice */
    gedday_init(WICED_STA_INTERFACE, "wiced_ping_webserver");
    gedday_add_service("Ping App Webserver", "_http._tcp.local", 80, 300, "");
    }
    else
    {
        WPRINT_APP_INFO(("Unable to bring up network connection\r\n"));
    }
}


/* Sends a ping to the target */
static wiced_result_t send_ping( void* arg )
{
    uint32_t elapsed_ms;
    wiced_result_t status;

    status = wiced_ping( WICED_STA_INTERFACE, &ping_target_ip, PING_TIMEOUT, &elapsed_ms );

    wiced_rtos_lock_mutex( &ping_mutex );  /* Stop webserver thread reading ping data halfway through a write */

    if ( status == WICED_SUCCESS )
    {
        WPRINT_APP_INFO(("Ping Reply : %lu ms\r\n", (unsigned long)elapsed_ms ));
        sprintf( (char*) ping_results[ping_end_index], "Ping reply : %5ld ms", (long)elapsed_ms );
    }
    else if ( status == WICED_TIMEOUT )
    {
        WPRINT_APP_INFO(("Ping timeout\r\n"));
        sprintf( (char*) ping_results[ping_end_index], "Ping reply : timeout" );
    }
    else
    {
        WPRINT_APP_INFO(("Ping error\r\n"));
        sprintf( (char*) ping_results[ping_end_index], "Ping error" );
    }

    ping_end_index = ( ping_end_index + 1 ) % PING_HISTORY_LEN;
    ping_start_index = ( ( ping_results[ping_end_index][0] == ' ' ) ? 0 : ping_end_index );

    wiced_rtos_unlock_mutex( &ping_mutex );

    return WICED_SUCCESS;
}


/* Update the ping webpage with the latest ping results */
static int process_ping( const char* url, wiced_tcp_stream_t* stream, void* arg )
{
    int a;

    wiced_tcp_stream_write( stream, resource_apps_DIR_ping_webserver_DIR_table_html, sizeof( resource_apps_DIR_ping_webserver_DIR_table_html ) - 1 );
    wiced_tcp_stream_write( stream, ping_description, strlen( ping_description ) );
    wiced_tcp_stream_write( stream, resource_apps_DIR_ping_webserver_DIR_table_html_desc2_end, sizeof( resource_apps_DIR_ping_webserver_DIR_table_html_desc2_end ) - 1 );

    wiced_rtos_lock_mutex( &ping_mutex ); /* Stops app thread writing ping data halfway through a read */
    for ( a = PING_HISTORY_LEN - 1; a >= 0; a-- )
    {
        char* res = ping_results[( ping_start_index + a ) % PING_HISTORY_LEN];
        wiced_tcp_stream_write( stream, res, strlen(res) );
        wiced_tcp_stream_write( stream, resource_apps_DIR_ping_webserver_DIR_table_html_row_end, sizeof( resource_apps_DIR_ping_webserver_DIR_table_html_row_end ) - 1 );
    }
    wiced_tcp_stream_write( stream, resource_apps_DIR_ping_webserver_DIR_table_html_list_end, sizeof(resource_apps_DIR_ping_webserver_DIR_table_html_list_end) -1 );
    wiced_rtos_unlock_mutex( &ping_mutex );
    return 0;
}

