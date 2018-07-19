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
 * DCT Read/Write Application
 *
 * This application demonstrates how to use the WICED DCT API
 * to access the WICED DCT.
 *
 * Q. What is the DCT?
 * A. The Device Configuration Table (DCT) holds configuration
 *    information for the device
 *
 * The DCT is comprised of the following sections which are
 * defined in <WICED-SDK>/Wiced/Platform/include/platform_dct.h:
 *   - Header              : WICED internal device status
 *   - Manufacturing Info  : Factory configuration information
 *   - Security            : SSL/TLS certificates and keys
 *   - Wi-Fi Configuration : Wi-Fi softAP & client credentials
 *   - Application         : Application specific variables
 *
 * Application Instructions
 *   Connect a PC terminal to the serial port of the WICED Eval board,
 *   then build and download the application as described in the WICED
 *   Quick Start Guide
 *
 *   After download, the app prints the contents of the
 *   Manufacturing Info, Security, Wi-Fi Config and
 *   Applications Sections of the DCT to the UART.
 *
 *   It then uses the DCT read & write API to read and modify several
 *   DCT variables, including the MAC address stored in the DCT
 *   and an application specific variable, string_var.
 *
 */

#include "wiced.h"
#include "dct_read_write_dct.h"

/******************************************************
 *                      Macros
 ******************************************************/

/******************************************************
 *                    Constants
 ******************************************************/

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

static wiced_result_t print_security_dct    ( void );
static wiced_result_t print_mfg_info_dct    ( void );
static wiced_result_t print_wifi_config_dct ( void );
static wiced_result_t print_app_dct         ( void );
static wiced_result_t print_mac_address     ( const char* prefix, wiced_mac_t* mac );

/******************************************************
 *               Variable Definitions
 ******************************************************/
static const configuration_entry_t app_config[] =
{
    {"uint8_var ", DCT_OFFSET(dct_read_write_app_dct_t, uint8_var ),  4,  CONFIG_UINT32_DATA },
    {"uint32_var", DCT_OFFSET(dct_read_write_app_dct_t, uint32_var),  4,  CONFIG_UINT8_DATA },
    {"string_var", DCT_OFFSET(dct_read_write_app_dct_t, string_var), 50,  CONFIG_STRING_DATA },
    {0,0,0,0}
};

static const char*       modified_string_var  = "I've been modified!";
static const wiced_mac_t modified_mac_address =
{
    .octet = { 0x00, 0x12, 0xFE, 0xED, 0xBE, 0xAD }
};

/*
 * Due to the potential large size of DCT data, the following variables
 * are intentionally declared as global variables (not local) to prevent
 * them from blowing the stack.
 */
static dct_read_write_app_dct_t   app_dct_local;
static platform_dct_wifi_config_t wifi_config_dct_local;

/******************************************************
 *               Function Definitions
 ******************************************************/

void application_start( )
{
    wiced_mac_t original_mac_address;
    char original_string_var[50] = { 0 };

    /* Initialise the device */
    wiced_init( );

    /* Configure the device */
    //wiced_configure_device( app_config );  /* Config bypassed in local makefile and wifi_config_dct.h */

    WPRINT_APP_INFO( ( "\r\nReading DCT Contents ...\r\n" ) );

    /* Read & print all DCT sections */
    print_mfg_info_dct();
    print_security_dct();
    print_wifi_config_dct();
    print_app_dct();

    WPRINT_APP_INFO( ( "\r\n\r\n----------------------------------------------------------------\r\n\r\n") );

    /**************************************************************************
     * This section demonstrates how to modify Wi-Fi config section
     **************************************************************************/
    WPRINT_APP_INFO( ( "Modifying MAC address in Wi-Fi Config Section\r\n" ) );

    /* Print original MAC addresses */
    original_mac_address = ((platform_dct_wifi_config_t*)wiced_dct_get_wifi_config_section())->mac_address;
    print_mac_address( "Original mac_address: ", (wiced_mac_t*)&original_mac_address );

    /* Modify the MAC address */
    wiced_dct_read_wifi_config_section( &wifi_config_dct_local );
    wifi_config_dct_local.mac_address = modified_mac_address;
    wiced_dct_write_wifi_config_section( (const platform_dct_wifi_config_t*)&wifi_config_dct_local );

    /* Print modified MAC addresses */
    print_mac_address( "Modified mac_address: ", (wiced_mac_t*)&((platform_dct_wifi_config_t*)wiced_dct_get_wifi_config_section())->mac_address );

    WPRINT_APP_INFO( ( "\r\n\r\n----------------------------------------------------------------\r\n\r\n") );

    /**************************************************************************
     * This section demonstrates how to modify App section
     **************************************************************************/
    WPRINT_APP_INFO( ( "Modifying string_var in App Section \r\n" ) );

    /* Print original string_var */
    strcpy( original_string_var, ( (dct_read_write_app_dct_t*)wiced_dct_get_app_section( ) )->string_var );
    WPRINT_APP_INFO( ( "Original string_var: %s\r\n", original_string_var ) );

    /* Modify string_var */
    wiced_dct_read_app_section( (void*)&app_dct_local, sizeof( app_dct_local ) );
    strcpy( app_dct_local.string_var, modified_string_var );
    wiced_dct_write_app_section( &app_dct_local, sizeof( app_dct_local ) );

    /* Print modified string_var */
    WPRINT_APP_INFO( ( "Modified string_var: %s\r\n", ((dct_read_write_app_dct_t*)wiced_dct_get_app_section())->string_var ) );

    WPRINT_APP_INFO( ( "\r\n\r\n" ) );

    /**************************************************************************
     * Restore original data so the app still works for the next boot-up
     **************************************************************************/
    wifi_config_dct_local.mac_address = original_mac_address;
    wiced_dct_write_wifi_config_section( (const platform_dct_wifi_config_t*)&wifi_config_dct_local );

    strcpy( app_dct_local.string_var, original_string_var );
    wiced_dct_write_app_section( &app_dct_local, sizeof( app_dct_local ) );
}


static wiced_result_t print_security_dct( void )
{
    platform_dct_security_t const* dct_security = wiced_dct_get_security_section( );

    WPRINT_APP_INFO( ( "\r\n----------------------------------------------------------------\r\n\r\n") );

    /* Security Section */
    WPRINT_APP_INFO( ( "Security Section \r\n") );
    WPRINT_APP_INFO( ( "    Certificate : \r\n%s \r\n", dct_security->certificate ) );
    WPRINT_APP_INFO( ( "    Private Key : \r\n%s \r\n", dct_security->private_key ) );

    return WICED_SUCCESS;
}


static wiced_result_t print_mfg_info_dct( void )
{
    platform_dct_mfg_info_t const* dct_mfg_info = wiced_dct_get_mfg_info_section( );

    WPRINT_APP_INFO( ( "\r\n----------------------------------------------------------------\r\n\r\n") );

    /* Manufacturing Info Section */
    WPRINT_APP_INFO( ( "Manufacturing Info Section \r\n") );
    WPRINT_APP_INFO( ( "     manufacturer          : %s \r\n", dct_mfg_info->manufacturer ) );
    WPRINT_APP_INFO( ( "     product_name          : %s \r\n", dct_mfg_info->product_name ) );
    WPRINT_APP_INFO( ( "     BOM_name              : %s \r\n", dct_mfg_info->BOM_name ) );
    WPRINT_APP_INFO( ( "     BOM_rev               : %s \r\n", dct_mfg_info->BOM_rev ) );
    WPRINT_APP_INFO( ( "     serial_number         : %s \r\n", dct_mfg_info->serial_number ) );
    WPRINT_APP_INFO( ( "     manufacture_date_time : %s \r\n", dct_mfg_info->manufacture_date_time ) );
    WPRINT_APP_INFO( ( "     manufacture_location  : %s \r\n", dct_mfg_info->manufacture_location ) );
    WPRINT_APP_INFO( ( "     bootloader_version    : %s \r\n", dct_mfg_info->bootloader_version ) );

    return WICED_SUCCESS;
}


static wiced_result_t print_wifi_config_dct( void )
{
    platform_dct_wifi_config_t const* dct_wifi_config = wiced_dct_get_wifi_config_section( );

    WPRINT_APP_INFO( ( "\r\n----------------------------------------------------------------\r\n\r\n") );

    /* Wi-Fi Config Section */
    WPRINT_APP_INFO( ( "Wi-Fi Config Section \r\n") );
    WPRINT_APP_INFO( ( "    device_configured               : %d \r\n", dct_wifi_config->device_configured ) );
    WPRINT_APP_INFO( ( "    stored_ap_list[0]  (SSID)       : %s \r\n", dct_wifi_config->stored_ap_list[0].details.SSID.val ) );
    WPRINT_APP_INFO( ( "    stored_ap_list[0]  (Passphrase) : %s \r\n", dct_wifi_config->stored_ap_list[0].security_key ) );
    WPRINT_APP_INFO( ( "    soft_ap_settings   (SSID)       : %s \r\n", dct_wifi_config->soft_ap_settings.SSID.val ) );
    WPRINT_APP_INFO( ( "    soft_ap_settings   (Passphrase) : %s \r\n", dct_wifi_config->soft_ap_settings.security_key ) );
    WPRINT_APP_INFO( ( "    config_ap_settings (SSID)       : %s \r\n", dct_wifi_config->config_ap_settings.SSID.val ) );
    WPRINT_APP_INFO( ( "    config_ap_settings (Passphrase) : %s \r\n", dct_wifi_config->config_ap_settings.security_key ) );
    WPRINT_APP_INFO( ( "    country_code                    : %c%c%d \r\n", ((dct_wifi_config->country_code) >>  0) & 0xff,
                                                                            ((dct_wifi_config->country_code) >>  8) & 0xff,
                                                                            ((dct_wifi_config->country_code) >> 16) & 0xff));
    print_mac_address( "    DCT mac_address                 :", (wiced_mac_t*)&dct_wifi_config->mac_address );

    return WICED_SUCCESS;
}


static wiced_result_t print_app_dct( void )
{
    dct_read_write_app_dct_t const* dct_app = wiced_dct_get_app_section( );

    WPRINT_APP_INFO( ( "\r\n----------------------------------------------------------------\r\n\r\n") );

    /* Application Section */
    WPRINT_APP_INFO( ( "Application Section\r\n") );
    WPRINT_APP_INFO( ( "    uint8_var               : %u \r\n", (unsigned int)((dct_read_write_app_dct_t*)dct_app)->uint8_var  ) );
    WPRINT_APP_INFO( ( "    uint32_var              : %u \r\n", (unsigned int)((dct_read_write_app_dct_t*)dct_app)->uint32_var ) );
    WPRINT_APP_INFO( ( "    string_var              : %s \r\n", (char*)((dct_read_write_app_dct_t*)dct_app)->string_var ) );

    return WICED_SUCCESS;
}


static wiced_result_t print_mac_address( const char* prefix, wiced_mac_t* mac )
{
    WPRINT_APP_INFO( ( "%s %02X:%02X:%02X:%02X:%02X:%02X\r\n", prefix, mac->octet[0],
                                                                       mac->octet[1],
                                                                       mac->octet[2],
                                                                       mac->octet[3],
                                                                       mac->octet[4],
                                                                       mac->octet[5] ) );

    return WICED_SUCCESS;
}
