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
 *
 * OTA Upgrade Example Application
 *
 * Please read the text at the top of the <WICED-SDK>/Apps/snip/ota_fr
 * application for information about the OTA Upgrade Example Application
 *
 * The ota_fr application demonstrates the OTA upgrade process.
 * The OTA upgrade example application in this directory can be
 * modified or replaced to suit your OTA upgrade requirements.
 *
 */


#include "tx_api.h"
#include "nx_api.h"
#include "netx_applications/dhcp/nxd_dhcp_client.h"
#include "netx_applications/dns/nxd_dns.h"
#include "wwd_wifi.h"
#include "wwd_management.h"
#include "Network/wwd_buffer_interface.h"
#include "Network/wwd_network_constants.h"
#include "Platform/wwd_platform_interface.h"
#include "wwd_network.h"
#include "web_server.h"
#include "wwd_debug.h"
#include "wwd_assert.h"
#include "bootloader_app.h"
#include "watchdog.h"

/******************************************************
 *        Configurable Constants
 ******************************************************/

#if 1 //MikeJ 20130318 - Change default Regulatory Domain from AU to US
#define COUNTRY                  WICED_COUNTRY_UNITED_STATES
#define AP_CHANNEL              (1)
#define AP_SSID_START           "WIZFI_OTA_"
#else
#define COUNTRY                  WICED_COUNTRY_AUSTRALIA
#define AP_CHANNEL              (1)
#define AP_SSID_START           "WICED_OTA_"
#endif
#define AP_PASS                 "12345678"
#define AP_SEC                  WICED_SECURITY_OPEN
#define AP_IP_ADDR              MAKE_IPV4_ADDRESS( 192, 168,   0,  1 )
#define AP_NETMASK              MAKE_IPV4_ADDRESS( 255, 255,   0,  0 )

#define JOIN_TIMEOUT             (10000)   /** timeout for joining the wireless network in milliseconds  = 10 seconds */
#define APP_STACK_SIZE           (4280)
#define APP_TX_BUFFER_POOL_SIZE  NUM_BUFFERS_POOL_SIZE(6)
#define APP_RX_BUFFER_POOL_SIZE  NUM_BUFFERS_POOL_SIZE(6)
#define IP_STACK_SIZE            (2048)
#define ARP_CACHE_SIZE           (3*52)

//#define DEBUG_PRINT(x)  printf x
#define DEBUG_PRINT(x)
#define DEBUG_ERROR(x) wiced_assert(x,0!=0)

/******************************************************
 * @cond       Macros
 ******************************************************/

#define MAKE_IPV4_ADDRESS   IP_ADDRESS

#define NUM_BUFFERS_POOL_SIZE(x)       ((WICED_LINK_MTU+sizeof(NX_PACKET)+1)*(x))

static const char* hexlist = "0123456789ABCDEF";

#define NIBBLE_HEX_VAL( x )  hexlist[(int)((x) & 0xF)]


/** @endcond */


/******************************************************
 *             Static Variables
 ******************************************************/

NX_IP          ip_handle;
NX_DNS         dns_handle;
NX_PACKET_POOL nx_pools[2]; /* 0=TX, 1=RX */

static TX_THREAD      app_main_thread_handle;
static char           app_main_thread_stack [APP_STACK_SIZE];
static char           ip_stack              [IP_STACK_SIZE];
static char           arp_cache             [ARP_CACHE_SIZE];
static uint32_t       tx_buffer_pool_memory [(APP_TX_BUFFER_POOL_SIZE)>>2];
static uint32_t       rx_buffer_pool_memory [(APP_RX_BUFFER_POOL_SIZE)>>2];
static TX_TIMER       watchdog_kick_timer;

extern const url_list_elem_t url_list[];
extern int app_main( void );

/******************************************************
 *             Static Function Prototypes
 ******************************************************/

static void app_main_thread( ULONG thread_input );
extern void start_dhcp_server( NX_IP* local_addr );
extern void quit_dhcp_server( void );
extern void start_dns_server( NX_IP* local_addr );
extern void quit_dns_server( void );


static VOID kick_watchdog( ULONG arg )
{
    watchdog_kick( );
}


/**
 * Main web server app thread
 *
 * Initialises NetX, Wiced, joins a wireless network,
 * creates a listening server socket then serves pages as they are requested
 *
 * @param thread_input : Unused parameter - required to match thread prototype
 */

static void app_main_thread( ULONG thread_input )
{
    volatile UINT status;
    wiced_result_t result;

    tx_timer_create( &watchdog_kick_timer, "wdk", &kick_watchdog, 0, SYSTICK_FREQUENCY, SYSTICK_FREQUENCY, TX_AUTO_ACTIVATE);

    DEBUG_PRINT( ( "\r\nPlatform " PLATFORM " initialised\r\n" ) );
    DEBUG_PRINT( ( "Started ThreadX " ThreadX_VERSION "\r\n" ) );

    /* Initialize the NetX system.  */
    DEBUG_PRINT( ( "Initialising NetX" NetX_VERSION "\r\n" ) );
    nx_system_initialize( );

    /* Create packet pools for transmit and receive */
    DEBUG_PRINT( ( "Creating Packet pools\r\n" ) );
    if ( NX_SUCCESS != nx_packet_pool_create( &nx_pools[0], "", WICED_LINK_MTU, tx_buffer_pool_memory, APP_TX_BUFFER_POOL_SIZE ) )
    {
        DEBUG_PRINT(("Couldn't create TX packet pool\r\n"));
        return;
    }
    if ( NX_SUCCESS != nx_packet_pool_create( &nx_pools[1], "", WICED_LINK_MTU, rx_buffer_pool_memory, APP_RX_BUFFER_POOL_SIZE ) )
    {
        DEBUG_PRINT(("Couldn't create RX packet pool\r\n"));
        return;
    }

    char ap_ssid[33];
    wiced_mac_t my_mac;

    /* Initialise Wiced */
    DEBUG_PRINT(("Starting WICED v" WICED_VERSION "\r\n"));
    while ( WICED_SUCCESS != ( result = wiced_management_init( COUNTRY, &nx_pools ) ) )
    {
        DEBUG_PRINT(("Error %d while starting WICED!\r\n", result));
    }

    /* Create the SSID */
    wiced_wifi_get_mac_address( &my_mac );

    strcpy( ap_ssid, AP_SSID_START );

    int i;
    int pos = sizeof(AP_SSID_START)-1;
    for(i = 0; i < 6; i++)
    {
        ap_ssid[pos+i*2]   = NIBBLE_HEX_VAL( ( my_mac.octet[i] >> 4 ) );
        ap_ssid[pos+i*2+1] = NIBBLE_HEX_VAL( ( my_mac.octet[i] >> 0 ) );
    }
    ap_ssid[pos+i*2] = '\x00';
//    sprintf( ap_ssid, AP_SSID_START "%02X%02X%02X%02X%02X%02X", my_mac.octet[0], my_mac.octet[1], my_mac.octet[2], my_mac.octet[3], my_mac.octet[4], my_mac.octet[5] );

    DEBUG_PRINT(("Starting Access Point: SSID: %s\r\n", ap_ssid ));

    /* Start the access point */
    wiced_wifi_start_ap( ap_ssid, AP_SEC, (uint8_t*) AP_PASS, sizeof( AP_PASS ) - 1, AP_CHANNEL );

    /* Create an IP instance. */

    /* Enable the network interface  */
    if ( NX_SUCCESS != ( status = nx_ip_create( &ip_handle, "NetX IP", AP_IP_ADDR, AP_NETMASK, &nx_pools[0], wiced_ap_netx_duo_driver_entry, ip_stack, IP_STACK_SIZE, 2 ) ) )
    {
        DEBUG_ERROR("Failed to create IP\r\n");
        return;
    }

    if ( NX_SUCCESS != ( status = nx_arp_enable( &ip_handle, (void *) arp_cache, ARP_CACHE_SIZE ) ) )
    {
        DEBUG_ERROR("Failed to enable ARP\r\n");
        nx_ip_delete( &ip_handle );
        return;
    }

    if ( NX_SUCCESS != ( status = nx_tcp_enable( &ip_handle ) ) )
    {
        DEBUG_ERROR("Failed to enable TCP\r\n");
        nx_ip_delete( &ip_handle );
        return;
    }

    if ( NX_SUCCESS != ( status = nx_udp_enable( &ip_handle ) ) )
    {
        DEBUG_ERROR("Failed to enable UDP\r\n");
        nx_ip_delete( &ip_handle );
        return;
    }

    if ( NX_SUCCESS != ( status = nx_icmp_enable( &ip_handle ) ) )
    {
        DEBUG_ERROR("Failed to enable ICMP\r\n");
        nx_ip_delete( &ip_handle );
        return;
    }

    ULONG ip_address, network_mask;
    nx_ip_address_get( &ip_handle, &ip_address, &network_mask );
    DEBUG_PRINT(("Network ready IP: %u.%u.%u.%u\r\n",(unsigned char)((ip_address >> 24)& 0xff), (unsigned char)((ip_address >> 16)& 0xff), (unsigned char)((ip_address >> 8)& 0xff), (unsigned char)((ip_address >> 0)& 0xff) ));

    /* Create DNS & DHCP server */
    start_dns_server( &ip_handle );
    start_dhcp_server( &ip_handle );

#if 0	// kaizen 20130619 ID1077 - Delete code in order to do not erase the existing app in OTA Mode
    /* Erase the existing app */
    bootloader_api->platform_set_app_valid_bit( APP_INVALID );
    platform_set_bootloader_led( 1 );
    bootloader_api->platform_erase_app( );
    platform_set_bootloader_led( 0 );
#endif

    // kaizen 20160912 For OTA
    platform_on_ota_mode();

    /* Run the web server */
    run_webserver( (void*) &ip_handle, url_list );


    host_rtos_delay_milliseconds(2000);

    bootloader_api->platform_reboot( );

/* TODO: cleanup after exit */

}

/**
 *  Main function - starts ThreadX
 *  Called from the crt0 _start function
 *
 */
int main( )
{
    /* Enter the ThreadX kernel.  */
    tx_kernel_enter( );
    return 0;
}

/**
 *  Application Define function - creates and starts the application thread
 *  Called by ThreadX whilst it is initialising
 *
 *  @param first_unused_memory: unused parameter - required to match prototype
 */

void tx_application_define( void *first_unused_memory )
{

    /* Create the application thread.  */
    tx_thread_create( &app_main_thread_handle, "app thread", app_main_thread, 0, app_main_thread_stack, APP_STACK_SIZE, 4, 4, TX_NO_TIME_SLICE, TX_AUTO_START );

}
