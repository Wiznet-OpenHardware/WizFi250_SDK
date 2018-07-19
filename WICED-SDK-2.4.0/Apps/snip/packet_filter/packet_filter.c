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
 * Ethernet Packet Filter Application
 *
 * This application demonstrates how to make the WLAN chip automatically
 * filter ethernet packets.

 * This feature enables the host processor to go to sleep or perform
 * other tasks without the need to process incoming ethernet packets
 * that an application may not be interested in receiving
 *
 * Features demonstrated
 *  - Adding packet filters
 *  - Enabling/Disabling packet filters
 *  - Listing installed packet filters
 *  - Removing packet filters
 *
 * Application Instructions
 *   1. Modify the CLIENT_AP_SSID/CLIENT_AP_PASSPHRASE Wi-Fi credentials
 *      in the wifi_config_dct.h header file to match your Wi-Fi access point
 *   2. Connect a PC terminal to the serial port of the WICED Eval board,
 *      then build and download the application as described in the WICED
 *      Quick Start Guide
 *
 *   Once the device has joined a Wi-Fi network and received an IP address, try sending
 *   ICMP pings to the device from a remote computer connected to the same network. The
 *   filter list printed to the console will increment the Filter ID 101 Packet Matched
 *   for each ping packet received.
 *
 * Discussion
 *   When a packet filter(s) is installed, incoming packets received by the WLAN chip are
 *   run through the pre-installed filter(s). Filter criteria are added using the 'add'
 *   API function. If the WLAN chip receives a packet that matches one of the currently
 *   installed filters, the host MCU is notified, and the packet is forwarded to the MCU.
 *   Packets that do not match any of the installed filters are dropped by the WLAN chip.
 *
 *   If there are no packet filters installed, all received ethernet packets are passed from
 *   the WLAN chip to the host MCU
 *
 *   For full documentation and a list of packet filter API functions, please read the
 *   packet filter API documentation contained in the file <WICED-SDK>/doc/API.html
 *
 * References
 *   http://en.wikipedia.org/wiki/EtherType
 *
 */

#include "wiced.h"
#include "wifi_config_dct.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/
#define MAX_PATTERNS_PER_FILTER         1  /* The packet filter API currently only supports 1 pattern per filter */
#define FILTER_ID_ETHERTYPE           100  /* User-defined packet filter ID */
#define FILTER_ID_SPECIFIC_IP_ADDRESS 101  /* User-defined packet filter ID */

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

static wiced_result_t print_packet_filter_list ( void );
static wiced_result_t print_packet_filter_stats( uint8_t filter_id );

/******************************************************
 *               Variable Definitions
 ******************************************************/

/*
 * Example 1 : Filter for forwarding packets with EtherType[1] = 0x08xx
 *             ie. IPv4 packets (0x0800), ARP packets (0x0806), Wake-on-LAN packets (0x0842) etc.
 *   - offset       = 12 (EtherType field)
 *   - bitmask size = 2 bytes
 *   - bitmask      = 0xff00 (mask out lower byte, don't want to match on this byte)
 *   - pattern      = 0x0800
 *   - rule         = positive matching
 */

static const wiced_packet_filter_pattern_t filter_ethertype_pattern[1] =
{
    [0] =
    {
        .offset    = FILTER_OFFSET_ETH_HEADER_ETHERTYPE,
        .mask_size = 2,
        .mask      = (uint8_t*)"\xff\x00",
        .pattern   = (uint8_t*)"\x08\x00",
    }
};

static const wiced_packet_filter_settings_t filter_ethertype =
{
    .rule          = WICED_PACKET_FILTER_RULE_POSITIVE_MATCHING,
    .pattern_count = MAX_PATTERNS_PER_FILTER,
    .pattern_list  = (wiced_packet_filter_pattern_t*)filter_ethertype_pattern,
};


static uint32_t my_ipv4_address;

/*
 * Example 2 : Filter definition for packets to a specific IP address
 *   - offset       = 30          (offset relative to the start of the packet)
 *   - bitmask size = 4 bytes
 *   - bitmask      = 0xffffffff  (don't mask out any bits)
 *   - pattern      =
 *   - rule         = positive matching
 */
static wiced_packet_filter_pattern_t filter_specific_ip_address_pattern[1] =
{
    [0] =
    {
        .offset    = FILTER_OFFSET_IPV4_HEADER_DESTINATION_ADDR,
        .mask_size = 4,
        .mask      = (uint8_t*)"\xff\xff\xff\xff",  /* Network byte order updated */
        .pattern   = (uint8_t*)&my_ipv4_address,    /* at run-time by the app     */
    }
};

static const wiced_packet_filter_settings_t filter_specific_ip_address =
{
    .rule          = WICED_PACKET_FILTER_RULE_POSITIVE_MATCHING,
    .pattern_count = MAX_PATTERNS_PER_FILTER,
    .pattern_list  = (wiced_packet_filter_pattern_t*)filter_specific_ip_address_pattern,
};



/******************************************************
 *               Function Definitions
 ******************************************************/

void application_start( )
{
    wiced_ip_address_t ip_address;
    uint32_t           i;

    /* Initialise the device */
    wiced_init( );

    /* Bring up the network on the STA interface */
    wiced_network_up( WICED_STA_INTERFACE, WICED_USE_EXTERNAL_DHCP_SERVER, NULL );

    /* Prepare IPv4 address for specific IP address packet filter used below */
    wiced_ip_get_ipv4_address( WICED_STA_INTERFACE, &ip_address );
    my_ipv4_address = htonl( ip_address.ip.v4 );  /* Swap IP address from host to network byte order */


    /* Setup WLAN to forward filtered packets to the MCU */
    wiced_wifi_set_packet_filter_mode( WICED_PACKET_FILTER_MODE_FORWARD );


    /* Add and enable a packet filter for EtherType packets */
    if ( wiced_wifi_add_packet_filter( FILTER_ID_ETHERTYPE, &filter_ethertype ) != WICED_SUCCESS )
        WPRINT_APP_INFO( ( "ERROR: enabling filter ID: %u\r\n", (unsigned int)FILTER_ID_ETHERTYPE) );
    if ( wiced_wifi_enable_packet_filter( FILTER_ID_ETHERTYPE ) != WICED_SUCCESS )
        WPRINT_APP_INFO( ( "ERROR: enabling filter ID: %u\r\n", (unsigned int)FILTER_ID_ETHERTYPE) );


    /* Add and enable a filter for our IPv4 address */
    if ( wiced_wifi_add_packet_filter( FILTER_ID_SPECIFIC_IP_ADDRESS, &filter_specific_ip_address ) != WICED_SUCCESS )
        WPRINT_APP_INFO( ( "ERROR: enabling filter ID: %u\r\n", (unsigned int)FILTER_ID_SPECIFIC_IP_ADDRESS) );
    if ( wiced_wifi_enable_packet_filter( FILTER_ID_SPECIFIC_IP_ADDRESS ) != WICED_SUCCESS )
        WPRINT_APP_INFO( ( "ERROR: enabling filter ID: %u\r\n", (unsigned int)FILTER_ID_SPECIFIC_IP_ADDRESS) );


    /* Print a list of installed filters */
    if ( print_packet_filter_list() != WICED_SUCCESS )
        WPRINT_APP_INFO( ( "ERROR: Listing packet filters\r\n") );


    /* Print filter statistics for 30 seconds */
    for ( i = 0; i < 30; i++ )
    {
        /* Print statistics for installed filters */
    	WPRINT_APP_INFO(("\r\n\r\nFilter Statistics ------------\r\n\r\n"));
        if ( print_packet_filter_stats( FILTER_ID_ETHERTYPE ) != WICED_SUCCESS )
            WPRINT_APP_INFO( ( "ERROR: Getting filter stats\r\n") );
        if ( print_packet_filter_stats( FILTER_ID_SPECIFIC_IP_ADDRESS ) != WICED_SUCCESS )
            WPRINT_APP_INFO( ( "ERROR: Getting filter stats\r\n") );

        wiced_rtos_delay_milliseconds( 1000 );
    }


    /* Uninstall EtherType packet filter that was installed previously */
    if ( wiced_wifi_disable_packet_filter( FILTER_ID_ETHERTYPE ) != WICED_SUCCESS )
        WPRINT_APP_INFO( ( "ERROR: disabling filter ID: %u\r\n", (unsigned int)FILTER_ID_ETHERTYPE) );
    if ( wiced_wifi_remove_packet_filter( FILTER_ID_ETHERTYPE ) != WICED_SUCCESS )
        WPRINT_APP_INFO( ( "ERROR: removing filter ID: %u\r\n", (unsigned int)FILTER_ID_ETHERTYPE) );


    /* Uninstall IPv4 packet filter that was installed previously */
    if ( wiced_wifi_disable_packet_filter( FILTER_ID_SPECIFIC_IP_ADDRESS ) != WICED_SUCCESS )
        WPRINT_APP_INFO( ("ERROR: disabling filter ID: %u\r\n", (unsigned int)FILTER_ID_SPECIFIC_IP_ADDRESS) );
    if ( wiced_wifi_remove_packet_filter( FILTER_ID_SPECIFIC_IP_ADDRESS ) != WICED_SUCCESS )
        WPRINT_APP_INFO( ("ERROR: removing filter ID: %u\r\n", (unsigned int)FILTER_ID_SPECIFIC_IP_ADDRESS) );

}


static wiced_result_t print_packet_filter_list( void )
{
    wiced_result_t         result;
    wiced_packet_filter_t* filter_list;
    uint32_t               filter_count;
    uint32_t               a;

    result = wiced_wifi_get_enabled_packet_filter_list( &filter_count, &filter_list );

    if ( result != WICED_SUCCESS )
    {
        return result;
    }

    WPRINT_APP_INFO(("\r\n\r\nFilter List ------------------\r\n\r\n"));

    for ( a = 0; a < filter_count; a++ )
    {
        uint8_t* mask;
        uint8_t* pattern;
        uint32_t b;

        WPRINT_APP_INFO( ("Filter ID      : %u \r\n", (unsigned int)filter_list[a].id) );
        WPRINT_APP_INFO( ("Filter Rule    : %s \r\n", ( filter_list[a].settings.rule == WICED_PACKET_FILTER_RULE_POSITIVE_MATCHING ) ? "Positive Matching" : "Negative Matching") );
        WPRINT_APP_INFO( ("Pattern Offset : %u \r\n", (unsigned int)filter_list[a].settings.pattern_list[0].offset) );
        WPRINT_APP_INFO( ("Pattern Size   : %u \r\n", (unsigned int)filter_list[a].settings.pattern_list[0].mask_size) );

        mask = (uint8_t*)filter_list[a].settings.pattern_list[0].mask;
        b    = filter_list[a].settings.pattern_list[0].mask_size;

        WPRINT_APP_INFO( ("Filter Mask    : ") );

        while ( b > 0 )
        {
            WPRINT_APP_INFO( ("%02x ", *mask) );
            mask++;
            b--;
        }

        WPRINT_APP_INFO(("\r\n"));

        pattern = (uint8_t*)filter_list[a].settings.pattern_list[0].pattern;
        b       = filter_list[a].settings.pattern_list[0].mask_size;

        WPRINT_APP_INFO( ("Filter Pattern : ") );

        while ( b > 0 )
        {
            WPRINT_APP_INFO( ("%02x ", *pattern) );
            pattern++;
            b--;
        }

        WPRINT_APP_INFO( ("\r\n\r\n") );
    }

    return wiced_wifi_delete_packet_filter_list( filter_list );
}


static wiced_result_t print_packet_filter_stats( uint8_t filter_id )
{
    wiced_result_t              status;
    wiced_packet_filter_stats_t stats;

    status = wiced_wifi_get_packet_filter_stats( filter_id, &stats );

    if ( status == WICED_TIMEOUT )
    {
        WPRINT_APP_INFO(( "Timeout: Getting Packet Filter Statistics\r\n" ));
        return status;
    }

    WPRINT_APP_INFO( ("Filter ID         : %u \r\n", (unsigned int)filter_id) );
    WPRINT_APP_INFO( ("Packets Matched   : %u \r\n", (unsigned int)stats.num_pkts_matched) );
    WPRINT_APP_INFO( ("Packets Forwarded : %u \r\n", (unsigned int)stats.num_pkts_forwarded) );
    WPRINT_APP_INFO( ("Packets Discarded : %u \r\n", (unsigned int)stats.num_pkts_discarded) );
    WPRINT_APP_INFO( ("\r\n") );

    return status;
}
