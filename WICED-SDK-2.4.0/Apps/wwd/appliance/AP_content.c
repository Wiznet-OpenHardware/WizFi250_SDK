/*
 * Copyright 2013, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

/**
 * @file
 * This provides the content (web pages) which will be served when the
 * Appliance App is in configuration mode and is acting as an Access Point.
 *
 * It does not use a filesystem, all pages are generated by handler functions.
 * These functions simply copy data into the outgoing packet buffer.
 *
 */

#include "web_server.h"
#include "brcmlogos.h"
#include "wwd_wifi.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "wwd_assert.h"
#include "appliance.h"

#include "internal/SDPCM.h"
#include "wwd_wlioctl.h"

/******************************************************
 *             Defines
 ******************************************************/
#define CIRCULAR_RESULT_BUFF_SIZE  (5)
#define MAX_SCAN_RESULTS           (100)
#define SSID_FIELD_NAME            "ssid"
#define SECURITY_FIELD_NAME        "sec"
#define CHANNEL_FIELD_NAME         "chan"
#define BSSID_FIELD_NAME           "bssid"
#define PASSPHRASE_FIELD_NAME      "pwd"
#define PIN_FIELD_NAME             "pin"

/* Signal strength defines */

#define RSSI_VERY_POOR             -80
#define RSSI_POOR                  -70
#define RSSI_GOOD                  -68
#define RSSI_VERY_GOOD             -58
#define RSSI_VERY_POOR_STR         "Very Poor"
#define RSSI_POOR_STR              "Poor"
#define RSSI_GOOD_STR              "Good"
#define RSSI_VERY_GOOD_STR         "Very good"
#define RSSI_EXCELLENT_STR         "Excellent"

/* Macros for comparing MAC addresses */
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
 *             Structures
 ******************************************************/

typedef struct
{
    uint8_t scan_done;
    host_semaphore_type_t result_avail_sema;
    wiced_scan_result_t* result_buff;
} scan_cb_t;


/******************************************************
 *             Static Function Prototypes
 ******************************************************/

int  process_favicon       ( void* socket, char * params, int params_len );
int  process_brcmlogo      ( void* socket, char * params, int params_len );

static int  process_top           ( void* socket, char * params, int params_len );
static int  process_wps_icon      ( void* socket, char * params, int params_len );
static int  process_wps_pbc       ( void* socket, char * params, int params_len );
static int  process_wps_pin       ( void* socket, char * params, int params_len );
static int  process_wps_go        ( void* socket, char * params, int params_len );
static int  process_scan_icon     ( void* socket, char * params, int params_len );
static int  process_scanjoin      ( void* socket, char * params, int params_len );
static int  process_scan          ( void* socket, char * params, int params_len );
static int  process_connect       ( void* socket, char * params, int params_len );
static int  process_ajax          ( void* socket, char * params, int params_len );
static void scan_results_handler  ( wiced_scan_result_t ** result_ptr, void * user_data );
static int  decode_connect_params ( char * params, int params_len );
static void url_decode            ( char * str );
/******************************************************
 *             Global variables
 ******************************************************/

extern const unsigned char scan_icon[2197];
extern const unsigned char wps_icon[1880];

/**
 * URL Handler List
 */
const url_list_elem_t config_AP_url_list[] = {
                                     { "/",             "text/html",                process_top },
                                     { "/wps_icon.png", "image/png",                process_wps_icon },
                                     { "/scan_icon.png","image/png",                process_scan_icon },
                                     { "/index.html",   "text/html",                process_top },
                                     { "/scan",         "text/html\r\n"
                                                        "Expires: Tue, 03 Jul 2001 06:00:00 GMT\r\n"
                                                        "Cache-Control: no-store, no-cache, must-revalidate, max-age=0\r\n"
                                                        "Cache-Control: post-check=0, pre-check=0\r\n"
                                                        "Pragma: no-cache\r\n",     process_scan },
                                     { "/scanjoin",     "text/html",                process_scanjoin },
                                     { "/favicon.ico",  "image/vnd.microsoft.icon", process_favicon },
                                     { "/brcmlogo.jpg", "image/jpeg",               process_brcmlogo },
                                     { "/connect",      "text/html",                process_connect },
                                     { "/ajax.js",      "application/javascript",   process_ajax },
                                     { "/wps_pbc",      "text/html",                process_wps_pbc },
                                     { "/wps_pin",      "text/html",                process_wps_pin },
                                     { "/wps_go",       "text/html",                process_wps_go },
                                     /* Add more pages here */
                                     { NULL, NULL, NULL }
                                   };



/******************************************************
 *             Static variables
 ******************************************************/

/**
 * HTML data for main Appliance web page
 */
static const char top_web_page_top[] =
    "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN " "http://www.w3.org/TR/html4/strict.dtd\">\n"
    "<html>\n"
    "  <head>\n"
    "    <title>WICED Appliance Config</title>\n"
    "    <style><!-- .normal { background-color: #ffffff; } .highlight { background-color: #8f0000;  } --></style>\n"
    "  </head>\n"
    "  <body style=\"font-family:verdana;\" >\n"
    "    <h2 align=\"center\"><img src=\"brcmlogo.jpg\"/> <span style=\"color:#ff0000\">Broadcom</span> WICED Appliance Configuration</h2><hr>\n"
    "    <p align=\"center\">Select configuration method:</p>\n"
    "    <table border=0 cellpadding=20 align=\"center\" style=\"border-collapse:collapse\">\n"
#ifdef APPLIANCE_ENABLE_WPS
    "      <tr onclick=\"document.location.href='wps_pbc'\" onmouseover=\"this.className='highlight'\" onmouseout=\"this.className='normal'\"><td><img src=\"wps_icon.png\" style=\"vertical-align:middle\" /></td><td><p>Push button setup</p></td></tr>\n"
#else /* ifdef APPLIANCE_ENABLE_WPS */
        "      <tr style=\"background-color:#888888;opacity: 0.4;filter:alpha(opacity=40);zoom:1;\"><td><img src=\"wps_icon.png\" style=\"vertical-align:middle\" /></td><td><p>Push button setup (Disabled due to compilation settings)</p></td></tr>\n"
#endif /* ifdef APPLIANCE_ENABLE_WPS */
    "      <tr onclick=\"document.location.href='scanjoin'\" onmouseover=\"this.className='highlight'\" onmouseout=\"this.className='normal'\"><td align=\"right\"><img src=\"scan_icon.png\" style=\"vertical-align:middle\" /></td><td><p>Scan and select network</p></td></tr>\n"
#ifdef APPLIANCE_ENABLE_WPS
    "      <tr onclick=\"document.location.href='wps_pin'\" onmouseover=\"this.className='highlight'\" onmouseout=\"this.className='normal'\"><td><img src=\"wps_icon.png\" style=\"vertical-align:middle\" /></td><td><p>PIN entry setup</p></td></tr>\n"
#else /* ifdef APPLIANCE_ENABLE_WPS */
    "      <tr style=\"background-color:#888888;opacity: 0.4;filter:alpha(opacity=40);zoom:1;\"><td><img src=\"wps_icon.png\" style=\"vertical-align:middle\" /></td><td><p>PIN entry setup (Disabled due to compilation settings)</p></td></tr>\n"
#endif /* ifdef APPLIANCE_ENABLE_WPS */
    "    </table>\n"
    "  </body>\n"
    "</html>\n";



static const char wps_pbc_page[] =
    "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN " "http://www.w3.org/TR/html4/strict.dtd\">\n"
    "<html>\n"
    "  <head>\n"
    "    <title>WICED Appliance WPS-PBC Config</title>\n"
    "    <script src=\"ajax.js\" type=\"text/javascript\" ></script>\n"
    "    <script type=\"text/javascript\">\n"
    "      function do_conn( )\n"
    "      {\n"
    "         do_ajax( '/wps_go', null, null, null, null, null, null );\n"
    "         document.getElementById('dim').style.display = \"block\";\n"
    "         document.getElementById('conndisp').innerHTML = \"Appliance Started<br/>Web server and access point stopped<br/>See UART for further information\"\n"
    "      }\n"
    "    </script>\n"
    "  </head>\n"
    "  <body style=\"font-family:verdana;\" >\n"
    "    <h2 align=\"center\"><img src=\"brcmlogo.jpg\"/> <span style=\"color:#ff0000\">Broadcom</span> WICED Appliance Configuration</h2><br/><br/>\n"
    "    <h2 align=\"center\"><img src=\"wps_icon.png\" style=\"vertical-align:middle\" />Push button setup</h2><hr>\n"
    "    <table border=0 cellpadding=20 align=\"center\"><tr><td>1) Press button on your router or access point (look for this symbol: <img src=\"wps_icon.png\" height=30 style=\"vertical-align:middle\" />)</td></tr>\n"
    "    <tr><td><form><label>2) Click this button : </label><input type=\"button\" onclick=\"do_conn()\" value=\"GO!\" style=\"height: 1.5em; width: 5em;font-size: larger; background-color: #cfffcf;\" /></form></td></tr></table>\n"
    "    <div id=\"dim\" style=\"display: none; background: #000;position: fixed; left: 0; top: 0;width: 100%; height: 100%;opacity: .80;z-index: 9999;text-align: center\">\n"
    "      <h2 id=\"conndisp\" style=\"color:#ff0000;position:relative;top:50%\"></h2>\n"
    "    </div>\n"
    "  <br/><br/><br/><br/><br/><br/><br/><a href=\"/\">Back to setup mode selection</a>\n"
    "  </body>\n"
    "</html>\n";

static const char wps_pin_page[] =
    "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN " "http://www.w3.org/TR/html4/strict.dtd\">\n"
    "<html>\n"
    "  <head>\n"
    "    <title>WICED Appliance WPS-PIN Config</title>\n"
    "    <script src=\"ajax.js\" type=\"text/javascript\" ></script>\n"
    "    <script type=\"text/javascript\">\n"
    "      function do_conn( )\n"
    "      {\n"
    "         s = '" PIN_FIELD_NAME "='+encodeURIComponent(document.getElementById('" PIN_FIELD_NAME "').value);\n"
    "         do_ajax( '/wps_go?' + s, null, null, null, null, null, null );\n"
    "         document.getElementById('dim').style.display = \"block\";\n"
    "         document.getElementById('conndisp').innerHTML = \"Appliance Started<br/>Web server and access point stopped<br/>See UART for further information\"\n"
    "      }\n"
    "    </script>\n"
    "  </head>\n"
    "  <body style=\"font-family:verdana;\" >\n"
    "    <h2 align=\"center\"><img src=\"brcmlogo.jpg\"/> <span style=\"color:#ff0000\">Broadcom</span> WICED Appliance Configuration</h2><br/><br/>\n"
    "    <h2 align=\"center\"><img src=\"wps_icon.png\" style=\"vertical-align:middle\" />PIN entry setup</h2><hr>\n"
    "    <table border=0 cellpadding=20 align=\"center\"><tr><td>1) Set the PIN and enable WPS in your router or access point (using it's web browser interface)</td></tr>\n"
    "      <tr><td><form action=\"/wps_go\" method=\"get\"><label>2) Enter PIN here : </label><input type=\"text\" id=\"" PIN_FIELD_NAME "\" name=\"" PIN_FIELD_NAME "\" maxlength=8 /><label> and click this button : </label><input type=\"button\" onclick=\"do_conn()\" value=\"GO!\" style=\"height: 1.5em; width: 5em;font-size: larger; background-color: #cfffcf;\" /></form></td></tr></table>\n"
    "    <div id=\"dim\" style=\"display: none; background: #000;position: fixed; left: 0; top: 0;width: 100%; height: 100%;opacity: .80;z-index: 9999;text-align: center\">\n"
    "      <h2 id=\"conndisp\" style=\"color:#ff0000;position:relative;top:50%\"></h2>\n"
    "    </div>\n"
    "  <br/><br/><br/><br/><br/><br/><br/><a href=\"/\">Back to setup mode selection</a>\n"
    "  </body>\n"
    "</html>\n";


/**
 * Scan results are retrieved via AJAX
 * This is the HTML data for the header of the scan results
 */
static const char scan_results_top[] =
    "<p>Please enter password and join a network:</p>\n"
    "<form>\n"
    "<p><label>Password: </label><input type=\"text\" id=\"" PASSPHRASE_FIELD_NAME "\"name=\"" PASSPHRASE_FIELD_NAME "\" maxlength=64 /></p>\n"
    "<table>\n"
    "<tr><th>Network Name</th><th>Signal</th><th> </th></tr>\n";

/**
 * This is the HTML data for the footer of the scan results
 */
static const char scan_results_bottom[] = "</table></form>\n";



static const char scan_page_outer[] =
    "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN " "http://www.w3.org/TR/html4/strict.dtd\">\n"
    "<html>\n"
    "  <head>\n"
    "    <title>WICED Appliance Scan-Join Config</title>\n"
    "    <script src=\"ajax.js\" type=\"text/javascript\" ></script>\n"
    "    <script type=\"text/javascript\">\n"
    "      function do_conn( ssid, sec, chan, bssid )\n"
    "      {\n"
    "         s = '" SSID_FIELD_NAME "='+encodeURIComponent(ssid)+'&" SECURITY_FIELD_NAME "='+sec+'&" CHANNEL_FIELD_NAME "='+chan+'&" BSSID_FIELD_NAME "='+bssid+'&" PASSPHRASE_FIELD_NAME "='+encodeURIComponent(document.getElementById('" PASSPHRASE_FIELD_NAME "').value);\n"
    "         document.getElementById('dim').style.display = \"block\";\n"
    "         document.getElementById('conndisp').innerHTML = \"Appliance Started<br/>Web server and access point stopped<br/>See UART for further information\"\n"
    "         do_ajax( 'connect?' + s, null, null, null, null, null, null );\n"
    "      }\n"
    "    </script>\n"
    "  </head>\n"
    "  <body style=\"font-family:verdana;\" onLoad=\"do_ajax( 'scan', 'scanres', 'Starting Scan...', 'Scanning...', null, 'Error Occurred', null)\">\n"
    "    <h2><img src=\"/brcmlogo.jpg\"/><span style=\"color:#ff0000\"> Broadcom</span> WICED Appliance Scan-Join Configuration</h2><hr>\n"
    "    <div id=\"scanres\"></div>\n"
    "    \n"
    "    <div id=\"dim\" style=\"display: none; background: #000;position: fixed; left: 0; top: 0;width: 100%; height: 100%;opacity: .80;z-index: 9999;text-align: center\">\n"
    "       <h2 id=\"conndisp\" style=\"color:#ff0000;position:relative;top:50%\"></h2>\n"
    "       </div>\n"
    "  </body>\n"
    "</html>\n"
    "";

/**
 * The AJAX script which retrieves the scan results
 */
static const char ajax_script[] =
    "function do_ajax( ajax_url, elem_id, start_text, inprog_text, endfunc, failtext, failfunc )\n"
    "{\n"
    "  req = null;\n"
    "  if (elem_id) document.getElementById(elem_id).innerHTML = start_text;\n"
    "  if (window.XMLHttpRequest)\n"
    "  {\n"
    "    req = new XMLHttpRequest();\n"
    "    if (req.overrideMimeType)\n"
    "    {\n"
    "      req.overrideMimeType('text/xml');\n"
    "    }\n"
    "  }\n"
    "  else if (window.ActiveXObject)\n"
    "  {\n"
    "    try {\n"
    "      req = new ActiveXObject(\"Msxml2.XMLHTTP\");\n"
    "    } catch (e)\n"
    "    {\n"
    "      try {\n"
    "          req = new ActiveXObject(\"Microsoft.XMLHTTP\");\n"
    "      } catch (e) {}\n"
    "    }\n"
    "  }\n"
    "  try {"
    "    req.onprogress = function( e )\n"
    "    {\n"
    "        if (elem_id) document.getElementById(elem_id).innerHTML = req.responseText;\n"
    "    }\n"
    "  } catch (e) { if (elem_id) document.getElementById(elem_id).innerHTML = inprog_text; }\n"
    "  req.onreadystatechange = function()\n"
    "  {\n"
    "    if(req.readyState == 4)\n"
    "    {\n"
    "      if(req.status == 200)\n"
    "      {\n"
    "        if (elem_id) document.getElementById(elem_id).innerHTML  = req.responseText;\n"
    "        if (endfunc) endfunc(req, elem_id);\n"
    "      }\n"
    "      else\n"
    "      {\n"
    "        if (elem_id && failtext) document.getElementById(elem_id).innerHTML = failtext;\n"
    "        if (failfunc) failfunc(req, elem_id);\n"
    "      }\n"
    "    }\n"
    "  };\n"
    "  req.open(\"GET\", ajax_url, true);\n"
    "  req.send(null);\n"
    "}\n";


static wiced_mac_t             bssid_list[MAX_SCAN_RESULTS]; /* List of BSSID (AP MAC addresses) that have been scanned */
static wiced_scan_result_t     result_buff[CIRCULAR_RESULT_BUFF_SIZE];
static uint16_t                result_buff_write_pos = 0;
static uint16_t                result_buff_read_pos  = 0;

/******************************************************
 *             Static Functions
 ******************************************************/

/**
 * URL handler for serving main Appliance web page
 *
 * Simply sends the static web page data
 *
 * @param  socket  : a handle for the TCP socket over which the data will be sent
 * @param  params     : a byte array containing any parameters included in the URL
 * @param  params_len : size of the params byte array in bytes
 */
static int process_top( void * socket, char * params, int params_len )
{
    send_web_data( socket, (unsigned char*) top_web_page_top, sizeof(top_web_page_top) - 1 ); /* minus one to avoid copying terminating null */

    return 0;
}

/**
 * URL handler for serving AJAX script file
 *
 * Simply sends the static script data
 *
 * @param  socket  : a handle for the TCP socket over which the data will be sent
 * @param  params     : a byte array containing any parameters included in the URL
 * @param  params_len : size of the params byte array in bytes
 */
static int process_ajax( void * socket, char * params, int params_len )
{
    send_web_data( socket, (unsigned char*) ajax_script, sizeof( ajax_script ) - 1 ); /* minus one to avoid copying terminating null */

    return 0;
}

/**
 * URL handler for serving the icon file
 *
 * Simply sends the static icon data
 *
 * @param  socket  : a handle for the TCP socket over which the data will be sent
 * @param  params     : a byte array containing any parameters included in the URL
 * @param  params_len : size of the params byte array in bytes
 */
int process_favicon( void * socket, char * params, int params_len )
{
    send_web_data( socket, (unsigned char*) favicon, favicon_size );
    return 0;
}

/**
 * URL handler for serving the broadcom logo
 *
 * Simply sends the static jpeg data
 *
 * @param  socket  : a handle for the TCP socket over which the data will be sent
 * @param  params     : a byte array containing any parameters included in the URL
 * @param  params_len : size of the params byte array in bytes
 */
int process_brcmlogo( void * socket, char * params, int params_len )
{
    send_web_data( socket, (unsigned char*) brcmlogo, brcmlogo_size );
    return 0;
}


static int process_scanjoin( void * socket, char * params, int params_len )
{
    send_web_data( socket, (unsigned char*) scan_page_outer, sizeof(scan_page_outer) );
    return 0;
}

static int process_scan_icon( void * socket, char * params, int params_len )
{
    send_web_data( socket, (unsigned char*) scan_icon, sizeof(scan_icon) );
    return 0;
}


static int process_wps_icon( void * socket, char * params, int params_len )
{
    send_web_data( socket, (unsigned char*) wps_icon, sizeof(wps_icon) );
    return 0;
}

static int process_wps_pbc( void * socket, char * params, int params_len )
{
    send_web_data( socket, (unsigned char*) wps_pbc_page, sizeof(wps_pbc_page) );
    return 0;
}

static int process_wps_pin( void * socket, char * params, int params_len )
{
    send_web_data( socket, (unsigned char*) wps_pin_page, sizeof(wps_pin_page) );
    return 0;
}


/**
 * URL handler for serving scan results web page
 *
 * Sends the static header data, then constructs table row for each scan result,
 * before sending the static footer data.
 *
 * @param  socket  : a handle for the TCP socket over which the data will be sent
 * @param  params     : a byte array containing any parameters included in the URL
 * @param  params_len : size of the params byte array in bytes
 */
static int process_scan( void * socket, char * params, int params_len )
{
    char recordbuff[500];
    char * recordbuffptr;
    wiced_scan_result_t * update_result_ptr = (wiced_scan_result_t *) result_buff;
    wiced_scan_result_t * read_result_ptr;
    scan_cb_t scan_cb_data;

    send_web_data( socket, (unsigned char*) scan_results_top, sizeof( scan_results_top ) - 1 ); /* minus one to avoid copying terminating null */

    /* Clear list of BSSID addresses already parsed */
    memset( &bssid_list, 0, sizeof( bssid_list ) );

    scan_cb_data.scan_done = 0;
    result_buff_read_pos = 0;
    result_buff_write_pos = 0;
    scan_cb_data.result_buff = &result_buff[0];

    /* Wait for scan to complete */
    host_rtos_init_semaphore( &scan_cb_data.result_avail_sema );

    /* Start Scan */
    if ( WICED_SUCCESS != wiced_wifi_scan( WICED_SCAN_TYPE_ACTIVE, WICED_BSS_TYPE_ANY, NULL, NULL, NULL, NULL, scan_results_handler, (wiced_scan_result_t **) &update_result_ptr, &scan_cb_data ) )
    {
        WPRINT_APP_ERROR(("Error starting scan\r\n"));
        send_web_data( socket, (unsigned char*) scan_results_bottom, sizeof( scan_results_bottom ) - 1 ); /* minus one to avoid copying terminating null */
        host_rtos_deinit_semaphore( &scan_cb_data.result_avail_sema );
        return 0;
    }

    /* loop until scan results are completed */
    while ( host_rtos_get_semaphore( &scan_cb_data.result_avail_sema, NEVER_TIMEOUT, WICED_FALSE ) == WICED_SUCCESS )
    {
        int k;

        read_result_ptr = &result_buff[result_buff_read_pos];
        if ( scan_cb_data.scan_done == 1 )
        {
            /* Scan completed */
            break;
        }

            /* Reset the outgoing html buffer */
            recordbuffptr = recordbuff;

            /* Print SSID into buffer */
            recordbuffptr += sprintf( recordbuffptr, "<tr><td>" );
            for ( k = 0; k < read_result_ptr->SSID.len; k++ )
            {
                recordbuffptr += sprintf( recordbuffptr, "%c", read_result_ptr->SSID.val[k] );
            }

            /* Print a signal strength indication into buffer */
            recordbuffptr += sprintf( recordbuffptr, "</td><td>%s</td><td style=\"text-align:right;\">",
                                                      (read_result_ptr->signal_strength <= RSSI_VERY_POOR)?RSSI_VERY_POOR_STR:
                                                      (read_result_ptr->signal_strength <= RSSI_POOR     )?RSSI_POOR_STR:
                                                      (read_result_ptr->signal_strength <= RSSI_GOOD     )?RSSI_GOOD_STR:
                                                      (read_result_ptr->signal_strength <= RSSI_VERY_GOOD)?RSSI_VERY_GOOD_STR:RSSI_EXCELLENT_STR );

            /* Print the join button into the buffer */
            recordbuffptr += sprintf( recordbuffptr, "</td><td><input type=\"button\" value=\"Join\" onclick=\"do_conn( '" );
            for ( k = 0; k < read_result_ptr->SSID.len; k++ )
            {
                recordbuffptr += sprintf( recordbuffptr, "%c", read_result_ptr->SSID.val[k] );
            }
            recordbuffptr += sprintf( recordbuffptr, "', %d, %d, '%02X%02X%02X%02X%02X%02X' )\"/></td></tr>\n", read_result_ptr->security, read_result_ptr->channel, read_result_ptr->BSSID.octet[0], read_result_ptr->BSSID.octet[1], read_result_ptr->BSSID.octet[2], read_result_ptr->BSSID.octet[3], read_result_ptr->BSSID.octet[4], read_result_ptr->BSSID.octet[5] );

            /* Send the row buffer */
            send_web_data( socket, (unsigned char*) recordbuff, (int) ( recordbuffptr - recordbuff ) );

        result_buff_read_pos++;
        if ( result_buff_read_pos >= CIRCULAR_RESULT_BUFF_SIZE )
        {
            result_buff_read_pos = 0;
        }
    }

    /* delete semaphore */
    host_rtos_deinit_semaphore( &scan_cb_data.result_avail_sema );


    /* Send the static footer HTML data */
    send_web_data( socket, (unsigned char*) scan_results_bottom, sizeof( scan_results_bottom ) - 1 ); /* minus one to avoid copying terminating null */

    return 0;
}




/**
 *  Scan result callback
 *  Called whenever a scan result is available
 *
 *  @param result_ptr : pointer to pointer for location where result is stored. The inner pointer
 *                      can be updated to cause the next result to be put in a new location.
 *  @param user_data : unused
 */

static void scan_results_handler( wiced_scan_result_t ** result_ptr, void * user_data )
{
    scan_cb_t* scan_cb_data = (scan_cb_t*) user_data;

    if ( result_ptr == NULL )
    {
        /* finished */
        scan_cb_data->scan_done = 1;
        host_rtos_set_semaphore( &scan_cb_data->result_avail_sema, WICED_FALSE );
        return;
    }

    /* Check the list of BSSID values which have already been printed */
    wiced_mac_t * tmp_mac = bssid_list;
    while ( ( tmp_mac < bssid_list + ( sizeof(bssid_list) / sizeof(wiced_mac_t) ) ) &&
            !NULL_MAC( tmp_mac->octet ) )
    {
        if ( CMP_MAC( tmp_mac->octet, (*result_ptr)->BSSID.octet ) )
        {
            /* already seen this BSSID */
            return;
        }
        tmp_mac++;
    }
    /* New BSSID - add it to the list */
    memcpy( &tmp_mac->octet, ( *result_ptr )->BSSID.octet, sizeof(wiced_mac_t) );

    /* Increment the write location for the next scan result */

    result_buff_write_pos++;
    if ( result_buff_write_pos >= CIRCULAR_RESULT_BUFF_SIZE )
    {
        result_buff_write_pos = 0;
    }

    *result_ptr =&scan_cb_data->result_buff[result_buff_write_pos];


    /* signal other thread */
    host_rtos_set_semaphore( &scan_cb_data->result_avail_sema, WICED_FALSE );


    wiced_assert( "Circular result buffer overflow", result_buff_write_pos != result_buff_read_pos );
}


static int process_wps_go( void* socket, char * params, int params_len )
{
    /* client has signalled to start client mode via WPS. */

    /* Check if parameter is SSID */
    if ( ( strlen( PIN_FIELD_NAME ) + 1 < params_len ) &&
         ( 0 == strncmp( params, PIN_FIELD_NAME "=", strlen( PIN_FIELD_NAME ) + 1 ) ) )
    {
        appliance_config.config_type = CONFIG_WPS_PIN;

        params += strlen( PIN_FIELD_NAME ) + 1;
        int pinlen = 0;
        /* find length of pin */
        while ( ( params[pinlen] != '&'  ) &&
                ( params[pinlen] != '\n' ) &&
                ( params[pinlen] != ' '  ) &&
                ( params_len > 0 ) )
        {
            pinlen++;
            params_len--;
        }
        memcpy( &appliance_config.vals.wps_pin.pin, params, pinlen );
        appliance_config.vals.wps_pin.pin[pinlen] = '\x00';
    }
    else
    {
        appliance_config.config_type = CONFIG_WPS_PBC;
    }

    /* return 1 to shut down web server */
    return 1;
}


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
static int process_connect( void* socket, char * params, int params_len )
{
    /* client has signalled to start appliance mode. */

    decode_connect_params( params, params_len );

    /* return 1 to shut down web server */
    return 1;
}

/**
 * Reads the connection parameters from a URL string.
 *
 * Parses a URL parameter string to retrieve the fields for connection parameters
 *
 * @param params - the string containing the URL parameters
 * @param params_len - the length of the string containing the URL parameters
 */
static int decode_connect_params( char * params, int params_len )
{
    char* ssid = NULL;
    char* bssid = NULL;
    char* security = NULL;
    char* passphrase = NULL;
    char* channel = NULL;
    unsigned int i;

    /* Cycle through string */
    while ( ( *params != '\n' ) && ( *params != ' ' ) && ( params_len > 0 ) )
    {
        /* Check if parameter is SSID */
        if ( 0 == strncmp( params, SSID_FIELD_NAME "=", strlen( SSID_FIELD_NAME ) + 1 ) )
        {
            ssid = params + strlen( SSID_FIELD_NAME ) + 1;
        }

        /* Check if parameter is BSSID */
        if ( 0 == strncmp( params, BSSID_FIELD_NAME "=", strlen( BSSID_FIELD_NAME ) + 1 ) )
        {
            bssid = params + strlen( BSSID_FIELD_NAME ) + 1;
        }

        /* Check if parameter is Security type */
        if ( 0 == strncmp( params, SECURITY_FIELD_NAME "=", strlen( SECURITY_FIELD_NAME ) + 1 ) )
        {
            security = params + strlen( SECURITY_FIELD_NAME ) + 1;
        }

        /* Check if parameter is pass phrase */
        if ( 0 == strncmp( params, PASSPHRASE_FIELD_NAME "=", strlen( PASSPHRASE_FIELD_NAME ) + 1 ) )
        {
            passphrase = params + strlen( PASSPHRASE_FIELD_NAME ) + 1;
        }

        /* Check if parameter is pass phrase */
        if ( 0 == strncmp( params, CHANNEL_FIELD_NAME "=", strlen( CHANNEL_FIELD_NAME ) + 1 ) )
        {
            channel = params + strlen( CHANNEL_FIELD_NAME ) + 1;
        }

        /* Scan ahead to the next parameter or the end of the parameter list */
        while ( ( *params != '&' ) && ( *params != '\n' ) && ( *params != ' ' ) && ( params_len > 0 ) )
        {
            params++;
            params_len--;
        }

        /* Skip over the "&" which joins parameters if found */
        if ( *params == '&' )
        {
            *params = '\x00'; /* change ampersand to null termination of parameter */
            params++;
        }
    }
    *params = '\x00'; /* change last character to null termination of parameter */

    if ( ( ssid     == NULL ) || ( ssid[0]     == '\x00' ) ||
         ( channel  == NULL ) || ( channel[0]  == '\x00' ) ||
         ( bssid    == NULL ) || ( bssid[0]    == '\x00' ) ||
         ( security == NULL ) || ( security[0] == '\x00' ) )
    {
        /* Invalid entry */
        return -1;
    }

    /* URL decode the strings */
    url_decode( ssid );
    url_decode( bssid );
    if ( passphrase != NULL )
    {
        url_decode( passphrase );
    }
    url_decode( security );
    url_decode( channel );

    /* Copy the ssid into the config buffer */
    appliance_config.config_type = CONFIG_SCANJOIN;
    strcpy( (char*) appliance_config.vals.scanjoin.scanresult.SSID.val, ssid );
    appliance_config.vals.scanjoin.scanresult.SSID.len = strlen( ssid );

    /* Decode the hex value of the BSSID into the config buffer */
    for ( i = 0; i < sizeof(wiced_mac_t); i++ )
    {
        uint8_t msb = bssid[i * 2    ];
        uint8_t lsb = bssid[i * 2 + 1];
        appliance_config.vals.scanjoin.scanresult.BSSID.octet[i] = ( ( msb >= 'a' ) ? msb - 'a' + 10 : ( msb >= 'A' ) ? msb - 'A' + 10 : msb - '0' ) << 4;
        appliance_config.vals.scanjoin.scanresult.BSSID.octet[i] +=  ( lsb >= 'a' ) ? lsb - 'a' + 10 : ( lsb >= 'A' ) ? lsb - 'A' + 10 : lsb - '0';
    }

    /* Copy the passphrase into the config buffer */
    if ( passphrase != NULL )
    {
        strcpy( appliance_config.vals.scanjoin.passphrase, passphrase );
        appliance_config.vals.scanjoin.passphrase_len = strlen( passphrase );
    }

    /* Copy the security and channel into the config buffer */
    appliance_config.vals.scanjoin.scanresult.security = (wiced_security_t) atoi( security );
    appliance_config.vals.scanjoin.scanresult.channel = atoi( channel );

    return 0;
}

/**
 * Decode the URL encoded characters in a given string
 *
 * Characters such as space, percent, ampersand, question mark etc are encoded
 * in urls via a hex code :  %xx
 * This function modifies a given string in-situ to decode these characters
 *
 * @param str - the string containing encoded characters to be modified
 */
static void url_decode( char * str )
{
    int read_pos = 0;
    int write_pos = 0;

    /* Cycle through input string */
    while ( str[read_pos] != '\x00' )
    {
        /* Check for percent with 2 characters of room after it. */
        if ( ( str[read_pos] == '%' ) &&
             ( str[read_pos + 1] != '\x00' ) &&
             ( str[read_pos + 2] != '\x00' ) )
        {
            /* Encoded value found - decode it and write it at the current location */
            unsigned char upper_nibble = str[read_pos + 1];
            unsigned char lower_nibble = str[read_pos + 2];
            str[write_pos] = ( ( upper_nibble >= 'a' ) ? upper_nibble - 'a' + 10 : ( upper_nibble >= 'A' ) ? upper_nibble - 'A' + 10 : upper_nibble - '0' ) << 4;
            str[write_pos] +=  ( lower_nibble >= 'a' ) ? lower_nibble - 'a' + 10 : ( lower_nibble >= 'A' ) ? lower_nibble - 'A' + 10 : lower_nibble - '0';

            /* Skip the read location over the rest of the encoded character */
            read_pos += 2;
        }
        else
        {
            /* no encoded value at this location - just write the character as is */
            str[write_pos] = str[read_pos];
        }

        /* Advance read and write locations to the next character */
        write_pos++;
        read_pos++;
    }

    /* add terminating null */
    str[write_pos] = '\x00';

}
































