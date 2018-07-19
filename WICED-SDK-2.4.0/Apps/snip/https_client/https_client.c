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
 * HTTPS Client Application
 *
 * This application snippet demonstrates how to use the WICED
 * DNS (Domain Name Service) & HTTPS API
 *
 * Features demonstrated
 *  - Wi-Fi client mode
 *  - DNS lookup
 *  - Secure HTTPS client connection
 *
 * Application Instructions
 * 1. Modify the CLIENT_AP_SSID/CLIENT_AP_PASSPHRASE Wi-Fi credentials
 *    in the wifi_config_dct.h header file to match your Wi-Fi access point
 * 2. Connect a PC terminal to the serial port of the WICED Eval board,
 *    then build and download the application as described in the WICED
 *    Quick Start Guide
 *
 * After the download completes, the application :
 *  - Connects to the Wi-Fi network specified
 *  - Resolves the http://google.com IP address using a DNS lookup
 *  - Downloads https://google.com using a secure HTTPS connection
 *  - Prints the downloaded HTML to the UART
 *
 */

#include <stdlib.h>
#include "wiced.h"
#include "http.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

#define BUFFER_LENGTH     (2048)

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

/******************************************************
 *               Variable Definitions
 ******************************************************/

uint8_t buffer[BUFFER_LENGTH];

/******************************************************
 *               Function Definitions
 ******************************************************/

void application_start( )
{
    wiced_ip_address_t ip_address;

    wiced_init( );
    wiced_network_up(WICED_STA_INTERFACE, WICED_USE_EXTERNAL_DHCP_SERVER, NULL);

    /* Configure the device */
    //wiced_configure_device( app_config );  /* Config bypassed in local makefile and wifi_config_dct.h */

    WPRINT_APP_INFO( ( "Resolving IP address of HTTPS server\r\n" ) );
    wiced_hostname_lookup("www.google.com", &ip_address, 10000);
    WPRINT_APP_INFO( ( "Server is at %u.%u.%u.%u\r\n", (uint8_t)(GET_IPV4_ADDRESS(ip_address) >> 24),
    		                                           (uint8_t)(GET_IPV4_ADDRESS(ip_address) >> 16),
    		                                           (uint8_t)(GET_IPV4_ADDRESS(ip_address) >> 8),
    		                                           (uint8_t)(GET_IPV4_ADDRESS(ip_address) >> 0) ) );

    WPRINT_APP_INFO( ( "Getting '/'...\r\n" ) );
    wiced_https_get(&ip_address, SIMPLE_GET_REQUEST, buffer, BUFFER_LENGTH);
    WPRINT_APP_INFO( ( "Server returned\r\n%s", buffer ) );

    wiced_deinit();
}
