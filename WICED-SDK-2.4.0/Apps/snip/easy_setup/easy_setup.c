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
 * Wi-Fi Easy Setup Demonstration Application (BETA VERSION)
 *
 * Features demonstrated
 *  - Wi-Fi Easy Setup
 *  - ICMP ping
 *
 * Wi-Fi Easy Setup Demonstration
 *   Wi-Fi Easy Setup provides a seamless way to setup your WICED Wi-Fi Device.
 *   Easy Setup provides various setup methods that may be run in parallel, these
 *   methods include
 *     - softAP  : A softAP + webserver is used to setup the device
 *     - WPS     : Wi-Fi Protected Setup via push button or PIN
 *     - MFi iAP : Wi-Fi setup via Bluetooth using the Apple MFi iAP library
 *     - Cooee   : A Broadcom patented broadcast setup mechanism (currently in BETA)
 *                 Read the WICED Easy Setup - Cooee Application Note in the Doc
 *                 directory for further information
 *
 * General Notes
 *   1. This application currently ONLY demonstrates how to use Cooee (BETA VERSION)
 *      The application will be expanded to demonstrate how to use various
 *      setup methods concurrently in an upcoming SDK release
 *
 *   2. Cooee is currently in BETA and has the following limitations that will be
 *      fixed in the next release  ...
 *      (a) The AP must use WPA2-AES security (all security types supported in final release)
 *      (b) The AP must be on a known (by this app) channel (channel scanning supported in final release)
 *
 * Notes to run the demo
 *   1. Set the #define ACCESS_POINT_CHANNEL to match the channel of your AP.
 *   2. Make sure your AP is configured to use WPA2-AES security (NOT WPA2 Mixed mode!)
 *   3. Run the Cooee application located in the <WICED-SDK>/Apps/snip/easy_setup/cooee_setup_client
 *      directory (if the application is run with no arguments, usage information is displayed).
 *      If your AP does not forward multicast packets, you may need to set your PC
 *      WLAN adapter to use a restricted Wi-Fi mode: 2.4GHz / 802.11b/g/n / 1x1 / 20MHz channel
 *      This restriction is necessary to match the WLAN standard supported by
 *      the Broadcom BCM43362 Wi-Fi chip in the WICED device
 *   4. When the app receives Cooee setup info from the setup client, the app
 *      joins the AP and tries to send regular ICMP pings to the IP address provided
 *      by the Cooee setup client
 *
 *  ---------------------------------------------------------------------------------------
 *  *** WARNING *** WARNING *** WARNING *** WARNING *** WARNING *** WARNING *** WARNING ***
 *  ---------------------------------------------------------------------------------------
 *    If you plan to use Cooee in your application, it is essential you read and understand
 *    the Wi-Fi Easy Setup - Cooee Application Note provided in the <WICED-SDK>/Doc directory.
 *    Cooee has a number of practical restrictions and incorrect use may introduce security
 *    vulnerabilities to the configuration process!
 *  ---------------------------------------------------------------------------------------
 *  *** WARNING *** WARNING *** WARNING *** WARNING *** WARNING *** WARNING *** WARNING ***
 *  ---------------------------------------------------------------------------------------
 *
 */

#include <stdlib.h>
#include "wiced.h"
#include "tlv.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

#define INVALID_IP_VERSION      -1
#define ACCESS_POINT_CHANNEL    (11)

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

static void cooee_callback(uint8_t* cooee_data, uint16_t data_length);
static wiced_result_t send_ping( wiced_ip_address_t ping_target_ip );

/******************************************************
 *               Variable Definitions
 ******************************************************/

wiced_ip_address_t ping_target_ip;
static wiced_semaphore_t cooee_complete;

/******************************************************
 *               Function Definitions
 ******************************************************/

void application_start( )
{

    wiced_init( );

    wiced_rtos_init_semaphore( &cooee_complete );

    wiced_wifi_set_channel( ACCESS_POINT_CHANNEL );

    WPRINT_APP_INFO( ("\r\n"
    		          "------------------------------\r\n"
    		          "Wi-Fi Easy Setup - Cooee Demonstration\r\n\r\n"
    		          "Listening for setup information ... \r\n"
    		          "If the app is stuck here, check the following:\r\n"
                      "  1. Is your access point on channel %d?\r\n"
                      "  2. Is your AP configured for WPA2 AES security?\r\n"
                      "  3. Do the AP SSID & Password credentials provided to \r\n"
                      "     the Cooee PC application match your AP?\r\n", ACCESS_POINT_CHANNEL) );

    // wiced_easy_setup_start_softap( );  /* Implementation and demo TBD */
    // wiced_easy_setup_start_wps( );     /* Implementation and demo TBD */
    // wiced_easy_setup_start_iap( );     /* Implementation and demo TBD */
    wiced_easy_setup_start_cooee( cooee_callback );

    /* Wait for Cooee to complete */
    wiced_rtos_get_semaphore( &cooee_complete, WICED_WAIT_FOREVER );

    wiced_network_up(WICED_STA_INTERFACE, WICED_USE_EXTERNAL_DHCP_SERVER, NULL);

    if (ping_target_ip.version == INVALID_IP_VERSION)
    {
    	WPRINT_APP_INFO(("\r\nCooee setup client provided invalid IP address, exiting.\r\n") );
    }
    else
    {
        WPRINT_APP_INFO(("\r\nPinging IP address provided by Cooee Setup Client ...\r\n") );
		while (1)
		{
			/* Send an ICMP ping to the gateway */
			send_ping( ping_target_ip );

			/* Sleep for a while */
			wiced_rtos_delay_milliseconds( 1 * SECONDS );
		}
    }

}

static void cooee_callback(uint8_t* cooee_data, uint16_t data_length)
{
    tlv8_data_t* tlv = (tlv8_data_t*) cooee_data;
    while (data_length >= (2 + tlv->length) )
    {
        switch (tlv->type)
        {
            case WICED_COOEE_IP_ADDRESS:
                if ( tlv->length == 4 )
                {
                    WPRINT_APP_INFO(("Cooee setup client provided IPv4 address: %d.%d.%d.%d\r\n", tlv->data[0], tlv->data[1], tlv->data[2], tlv->data[3] ));
                    ping_target_ip.version = WICED_IPV4;
                    ping_target_ip.ip.v4 = MAKE_IPV4_ADDRESS(tlv->data[0], tlv->data[1], tlv->data[2], tlv->data[3]);
                }
                else if ( tlv->length == 6 )
                {
                    ping_target_ip.version = WICED_IPV6;
                    WPRINT_APP_INFO(("Cooee setup client provided IPv6 address: %02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X\r\n",
                    		                                                    tlv->data[0], tlv->data[1], tlv->data[ 2], tlv->data[ 3], tlv->data[ 4], tlv->data[ 5], tlv->data[ 6], tlv->data[ 7],
                    		                                                    tlv->data[8], tlv->data[9], tlv->data[10], tlv->data[11], tlv->data[12], tlv->data[13], tlv->data[14], tlv->data[15]));
                    memcpy(ping_target_ip.ip.v6, tlv->data, 16);
                }
                else
                {
                    WPRINT_APP_INFO(("Cooee: Invalid IP address received\r\n"));
                	ping_target_ip.version = INVALID_IP_VERSION;
                }
                break;

            default:
                WPRINT_APP_INFO(("Cooee: Payload included TLV %d\r\n", tlv->type));
                break;
        }

        data_length -= (2 + tlv->length);
        cooee_data  += (2 + tlv->length);
        tlv = (tlv8_data_t*) cooee_data;
    }
    wiced_rtos_set_semaphore( &cooee_complete );
}


static wiced_result_t send_ping( wiced_ip_address_t ping_target_ip )
{
    const uint32_t     ping_timeout = 1000;
    uint32_t           elapsed_ms;
    wiced_result_t     status;

    wiced_ip_get_gateway_address( WICED_STA_INTERFACE, &ping_target_ip );
    status = wiced_ping( WICED_STA_INTERFACE, &ping_target_ip, ping_timeout, &elapsed_ms );

    if ( status == WICED_SUCCESS )
    {
    	WPRINT_APP_INFO(( "Ping Reply %lums\r\n", elapsed_ms ));
    }
    else if ( status == WICED_TIMEOUT )
    {
    	WPRINT_APP_INFO(( "Ping timeout\r\n" ));
    }
    else
    {
    	WPRINT_APP_INFO(( "Ping error\r\n" ));
    }

    return WICED_SUCCESS;
}
