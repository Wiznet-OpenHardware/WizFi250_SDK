/*
 * Copyright 2013, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */

#include "NetX/compat.h"
#include "tx_api.h"
#include "tx_thread.h"
#include "nx_api.h"
#include "wiced_management.h"
#include "wwd_network.h"
#include "wiced_wifi.h"
#include "wwd_debug.h"
#include "wwd_assert.h"
#include "Network/wwd_buffer_interface.h"
#include "RTOS/wwd_rtos_interface.h"
#include "wiced_network.h"
#include "console.h"
#include "wiced.h"

/******************************************************
 *        Settable Constants
 ******************************************************/
#define NUM_INTERFACES          (2)


/******************************************************
 * @cond       Macros
 ******************************************************/

/** @endcond */


/******************************************************
 *             Static Variables
 ******************************************************/

/******************************************************
 *             Global Variables
 ******************************************************/

extern NX_IP                 wiced_ip_handle[NUM_INTERFACES]; /* 0=STA, 1=AP */
NX_IP*                       ip_handle_default = &wiced_ip_handle[0];
extern NX_PACKET_POOL        wiced_packet_pools[2]; /* 0=TX, 1=RX */

/******************************************************
 *             Prototypes
 ******************************************************/
extern int app_main( void );


/**
 *  @param thread_input : Unused parameter - required to match thread prototype
 */
void application_start( void )
{
    /* Initialise the device */
    wiced_init( );

    /* Run the main application function */
    app_main( );
}

int set_ip( int argc, char* argv[] )
{
    ULONG ipaddr, netmask, gw;
    if ( argc < 4 )
    {
        return ERR_UNKNOWN;
    }
    ipaddr  = htonl(str_to_ip(argv[1]));
    netmask = htonl(str_to_ip(argv[2]));
    gw      = htonl(str_to_ip(argv[3]));
    nx_ip_interface_address_set( &wiced_ip_handle[WICED_STA_INTERFACE], 0, ipaddr, netmask );
    nx_ip_gateway_address_set( &wiced_ip_handle[WICED_STA_INTERFACE], gw );
    return ERR_CMD_OK;
}

void network_print_status( char* sta_ssid, char* ap_ssid )
{
    uint8_t interface;
    for ( interface = 0; interface <= 1; interface++ )
    {
        if ( wiced_wifi_is_ready_to_transceive( (wiced_interface_t) interface ) == WICED_SUCCESS )
        {
            if ( interface == WICED_STA_INTERFACE )
            {
                WPRINT_APP_INFO( ( "STA Interface\r\n"));
                WPRINT_APP_INFO( ( "   SSID       : %s\r\n", sta_ssid ) );
            }
            else
            {
                WPRINT_APP_INFO( ( "AP Interface\r\n"));
                WPRINT_APP_INFO( ( "   SSID       : %s\r\n", ap_ssid ) );

            }

            if ( wiced_ip_handle[interface].nx_ip_driver_link_up )
            {
                WPRINT_APP_INFO( ( "   IP Addr    : %u.%u.%u.%u\r\n", (unsigned char) ( ( wiced_ip_handle[interface].nx_ip_address >> 24 ) & 0xff ), (unsigned char) ( ( wiced_ip_handle[interface].nx_ip_address >> 16 ) & 0xff ), (unsigned char) ( ( wiced_ip_handle[interface].nx_ip_address >> 8 ) & 0xff ), (unsigned char) ( ( wiced_ip_handle[interface].nx_ip_address >> 0 ) & 0xff ) ) );
                WPRINT_APP_INFO( ( "   Gateway    : %u.%u.%u.%u\r\n", (unsigned char) ( ( wiced_ip_handle[interface].nx_ip_gateway_address >> 24 ) & 0xff ), (unsigned char) ( ( wiced_ip_handle[interface].nx_ip_gateway_address >> 16 ) & 0xff ), (unsigned char) ( ( wiced_ip_handle[interface].nx_ip_gateway_address >> 8 ) & 0xff ), (unsigned char) ( ( wiced_ip_handle[interface].nx_ip_gateway_address >> 0 ) & 0xff ) ) );
                WPRINT_APP_INFO( ( "   Netmask    : %u.%u.%u.%u\r\n", (unsigned char) ( ( wiced_ip_handle[interface].nx_ip_network_mask >> 24 ) & 0xff ), (unsigned char) ( ( wiced_ip_handle[interface].nx_ip_network_mask >> 16 ) & 0xff ), (unsigned char) ( ( wiced_ip_handle[interface].nx_ip_network_mask >> 8 ) & 0xff ), (unsigned char) ( ( wiced_ip_handle[interface].nx_ip_network_mask >> 0 ) & 0xff ) ) );
            }
        }
        else
        {
            if ( interface == WICED_STA_INTERFACE )
                WPRINT_APP_INFO( ( "STA Interface : Down\r\n" ) );
            else
                WPRINT_APP_INFO( ( "AP Interface  : Down\r\n" ) );
        }
    }
}

uint32_t host_get_time( void )
{
    return host_rtos_get_time();
}
