/*
 * Copyright 2013, Broadcom Corporation
 * All Rights Reserved.
 *
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 */
#include "wiced_wifi.h"
#include "string.h"
#include "wwd_debug.h"
#include "../console.h"
#include "wwd_assert.h"
#include "wwd_network.h"
#include "stdlib.h"
#include "wwd_management.h"
#include "internal/SDPCM.h"
#include "wifi.h"
#include "internal/wwd_internal.h"
#include "Network/wwd_buffer_interface.h"
#include "wiced_management.h"
#include "dhcp_server.h"
#include "../platform/platform.h"
#include "wwd_crypto.h"
#include "wiced.h"
#include "wiced_security.h"
#ifdef CONSOLE_ENABLE_WPS
#include "wiced_wps.h"
#include "wwd_events.h"
#include "wps_common.h"
#endif


#define MAX_SSID_LEN 32
#define MAX_PASSPHRASE_LEN 64
#define A_SHA_DIGEST_LEN 20
#define DOT11_PMK_LEN 32


//static wiced_result_t print_wifi_config_dct( void );
wiced_result_t scan_result_handler( wiced_scan_handler_result_t* malloced_scan_result );
void F( const char *password, const unsigned char *ssid, int ssidlength, int iterations, int count, unsigned char *output);
int PasswordHash ( const char *password, const unsigned char *ssid, int ssidlength, unsigned char *output );
void dump_bytes(const uint8_t* bptr, uint32_t len);
void hex_bytes_to_chars( char* cptr, const uint8_t* bptr, uint32_t blen );

static char last_joined_ssid[32] = "";
static char last_started_ssid[32] = "";
static char last_soft_ap_passphrase[MAX_PASSPHRASE_LEN+1] = "";
static int record_count;
static wiced_semaphore_t scan_semaphore;

#ifdef CONSOLE_ENABLE_WPS
extern wps_agent_t* workspace;
extern wiced_wps_device_detail_t device_details;
#endif



static const wiced_ip_setting_t ap_ip_settings =
{
    INITIALISER_IPV4_ADDRESS( .ip_address, MAKE_IPV4_ADDRESS( 192,168,  0,  1 ) ),
    INITIALISER_IPV4_ADDRESS( .netmask,    MAKE_IPV4_ADDRESS( 255,255,255,  0 ) ),
    INITIALISER_IPV4_ADDRESS( .gateway,    MAKE_IPV4_ADDRESS( 192,168,  0,  1 ) ),
};


/*
 * Due to the potential large size of DCT data, the following variables
 * are intentionally declared as global variables (not local) to prevent
 * them from blowing the stack.
 */
static platform_dct_wifi_config_t wifi_config_dct_local;


/*!
 ******************************************************************************
 * Joins an access point specified by the provided arguments
 *
 * @return  0 for success, otherwise error
 */

int join( int argc, char* argv[] )
{
    char* ssid = argv[1];
    wiced_security_t auth_type = str_to_authtype(argv[2]);
    uint8_t* security_key;
    uint8_t key_length;

    if (argc > 7)
    {
        return ERR_TOO_MANY_ARGS;
    }

    if (argc > 4 && argc != 7)
    {
        return ERR_INSUFFICENT_ARGS;
    }

    if ( auth_type == WICED_SECURITY_UNKNOWN )
    {
        WPRINT_APP_INFO(( "Error: Invalid security type\r\n" ));
        return ERR_UNKNOWN;
    }

    if ( auth_type == WICED_SECURITY_WEP_PSK )
    {
        int a;
        uint8_t          wep_key_buffer[64];
        wiced_wep_key_t* temp_wep_key = (wiced_wep_key_t*)wep_key_buffer;
        char temp_string[3];
        temp_string[2] = 0;
        key_length = strlen(argv[3])/2;

        /* Setup WEP key 0 */
        temp_wep_key[0].index = 0;
        temp_wep_key[0].length = key_length;
        for (a = 0; a < temp_wep_key[0].length; ++a)
        {
            memcpy(temp_string, &argv[3][a*2], 2);
            temp_wep_key[0].data[a] = hex_str_to_int(temp_string);
        }

        /* Setup WEP keys 1 to 3 */
        memcpy(wep_key_buffer + 1*(2 + key_length), temp_wep_key, (2 + key_length));
        memcpy(wep_key_buffer + 2*(2 + key_length), temp_wep_key, (2 + key_length));
        memcpy(wep_key_buffer + 3*(2 + key_length), temp_wep_key, (2 + key_length));
        wep_key_buffer[1*(2 + key_length)] = 1;
        wep_key_buffer[2*(2 + key_length)] = 2;
        wep_key_buffer[3*(2 + key_length)] = 3;

        security_key = wep_key_buffer;
        key_length = 4*(2 + key_length);
    }
    else if ( ( auth_type != WICED_SECURITY_OPEN ) && ( argc < 4 ) )
    {
        WPRINT_APP_INFO(("Error: Missing security key\r\n" ));
        return ERR_UNKNOWN;
    }
    else
    {
        security_key = (uint8_t*)argv[3];
        key_length = strlen((char*)security_key);
    }

    if ( argc == 7 )
    {
        return wifi_join( ssid, auth_type, (uint8_t*) security_key, key_length, argv[4], argv[5], argv[6]);
    }
    else
    {
        return wifi_join( ssid, auth_type, (uint8_t*) security_key, key_length, NULL, NULL, NULL );
    }
}

int wifi_join(char* ssid, wiced_security_t auth_type, uint8_t* key, uint16_t key_length, char* ip, char* netmask, char* gateway)
{
    wiced_network_config_t network_config;
    wiced_ip_setting_t* ip_settings = NULL;
    wiced_ip_setting_t static_ip_settings;

    if (wiced_wifi_is_ready_to_transceive(WICED_STA_INTERFACE) == WICED_SUCCESS)
    {
        return ERR_CMD_OK;
    }

    // Read config
    wiced_dct_read_wifi_config_section( &wifi_config_dct_local );
//    print_wifi_config_dct();

    // Modify config
    strncpy((char*)&wifi_config_dct_local.stored_ap_list[0].details.SSID.val, ssid, MAX_SSID_LEN);
    wifi_config_dct_local.stored_ap_list[0].details.security = auth_type;
    memcpy((char*)wifi_config_dct_local.stored_ap_list[0].security_key, (char*)key, MAX_PASSPHRASE_LEN);
    wifi_config_dct_local.stored_ap_list[0].security_key_length = key_length;

    // Write config
    wiced_dct_write_wifi_config_section( (const platform_dct_wifi_config_t*)&wifi_config_dct_local );
//    print_wifi_config_dct();

    /* Tell the network stack to setup it's interface */
    if (ip == NULL )
    {
        network_config = WICED_USE_EXTERNAL_DHCP_SERVER;
    }
    else
    {
        network_config = WICED_USE_STATIC_IP;
        static_ip_settings.ip_address.ip.v4 = htonl(str_to_ip(ip));
        static_ip_settings.netmask.ip.v4 = htonl(str_to_ip(netmask));
        static_ip_settings.gateway.ip.v4 = htonl(str_to_ip(gateway));
        ip_settings = &static_ip_settings;
    }

    if ( wiced_network_up( WICED_STA_INTERFACE, network_config, ip_settings ) != WICED_SUCCESS )
    {
        if (auth_type == WICED_SECURITY_WEP_PSK ) // Now try shared instead of open authentication
        {
            wiced_dct_read_wifi_config_section( &wifi_config_dct_local );
            wifi_config_dct_local.stored_ap_list[0].details.security = WICED_SECURITY_WEP_SHARED;
            wiced_dct_write_wifi_config_section( (const platform_dct_wifi_config_t*)&wifi_config_dct_local );
            printf("WEP with open authentication failed, trying WEP with shared authentication...\r\n");
            if ( wiced_network_up( WICED_STA_INTERFACE, network_config, ip_settings ) != WICED_SUCCESS ) // Restore old value
            {
                printf("trying shared wep\r\n");
                wiced_dct_read_wifi_config_section( &wifi_config_dct_local );
                wifi_config_dct_local.stored_ap_list[0].details.security = WICED_SECURITY_WEP_PSK;
                wiced_dct_write_wifi_config_section( (const platform_dct_wifi_config_t*)&wifi_config_dct_local );
            }
            else
            {
                return ERR_CMD_OK;
            }
        }

        return ERR_UNKNOWN;
    }

    return ERR_CMD_OK;
}


/**
 *  Scan result callback
 *  Called whenever a scan result is available
 *
 *  @param result_ptr : pointer to pointer for location where result is stored. The inner pointer
 *                      can be updated to cause the next result to be put in a new location.
 *  @param user_data : unused
 */
wiced_result_t scan_result_handler( wiced_scan_handler_result_t* malloced_scan_result )
{
    malloc_transfer_to_curr_thread( malloced_scan_result );

    if (malloced_scan_result->scan_complete != WICED_TRUE)
    {
        wiced_scan_result_t* record = &malloced_scan_result->ap_details;

        wiced_assert( "error", ( record->bss_type == WICED_BSS_TYPE_INFRASTRUCTURE ) || ( record->bss_type == WICED_BSS_TYPE_ADHOC ) );

        record->SSID.val[record->SSID.len] = 0; /* Ensure it's null terminated */

        WPRINT_APP_INFO( ( "%3d ", record_count ) );
        print_scan_result( record );

        ++record_count;
    }
    else
    {
        wiced_rtos_set_semaphore(&scan_semaphore);
    }

    free( malloced_scan_result );

    return WICED_SUCCESS;
}


/*!
 ******************************************************************************
 * Scans for access points and prints out results
 *
 * @return  0 for success, otherwise error
 */

int scan( int argc, char* argv[] )
{
    record_count = 0;
    WPRINT_APP_INFO( ( "Waiting for scan results...\r\n" ) );

    WPRINT_APP_INFO( ("  # Type  BSSID             RSSI  Rate Chan Security    SSID\r\n" ) );
    WPRINT_APP_INFO( ("----------------------------------------------------------------------------------------------\r\n" ) );

    /* Initialise the semaphore that will tell us when the scan is complete */
    wiced_rtos_init_semaphore(&scan_semaphore);

    wiced_wifi_scan_networks(scan_result_handler, NULL );

    /* Wait until scan is complete */
    wiced_rtos_get_semaphore(&scan_semaphore, WICED_WAIT_FOREVER);

    wiced_rtos_deinit_semaphore(&scan_semaphore);

    /* Done! */
    WPRINT_APP_INFO( ( "\r\nEnd of scan results\r\n" ) );

    return ERR_CMD_OK;
}

/*
 * From IEEE 802.11-2012
 * F(P, S, c, i) = U1 xor U2 xor ... Uc
 * U1 = PRF(P, S || Int(i))
 * U2 = PRF(P, U1)
 * Uc = PRF(P, Uc-1)
*/
void F( const char *password, const unsigned char *ssid, int ssidlength, int iterations, int count, unsigned char *output)
{
    unsigned char digest[36], digest1[A_SHA_DIGEST_LEN];
    int i, j;

//    for (i = 0; i < strlen(password); i++)
//    {
//        assert((password[i] >= 32) && (password[i] <= 126));
//    }

    /* U1 = PRF(P, S || int(i)) */
    memcpy(digest, ssid, ssidlength);
    digest[ssidlength] = (unsigned char)((count>>24) & 0xff);
    digest[ssidlength+1] = (unsigned char)((count>>16) & 0xff);
    digest[ssidlength+2] = (unsigned char)((count>>8) & 0xff);
    digest[ssidlength+3] = (unsigned char)(count & 0xff);

    //hmac_sha1(digest, ssidlength+4, (unsigned char*) password, (int) strlen(password), digest, digest1);
    sha1_hmac( (unsigned char*) password, (int) strlen(password), digest, ssidlength + 4, digest1 );

    /* output = U1 */
    memcpy(output, digest1, A_SHA_DIGEST_LEN);
    for (i = 1; i < iterations; i++)
    {
        /* Un = PRF(P, Un-1) */
        //hmac_sha1(digest1, A_SHA_DIGEST_LEN, (unsigned char*) password, (int) strlen(password), digest);
        sha1_hmac( (unsigned char*) password, (int) strlen(password), digest1, A_SHA_DIGEST_LEN, digest );
        memcpy(digest1, digest, A_SHA_DIGEST_LEN);

        /* output = output xor Un */
        for (j = 0; j < A_SHA_DIGEST_LEN; j++)
        {
            output[j] ^= digest[j];
        }
    }
}

/*
 * From IEEE 802.11-2012
 * password - ascii string up to 63 characters in length
 * ssid - octet string up to 32 octets
 * ssidlength - length of ssid in octets
 * output must be 40 octets in length and outputs 256 bits of key
*/
int PasswordHash ( const char *password, const unsigned char *ssid, int ssidlength, unsigned char *output )
{
    if ((strlen(password) > 63) || (ssidlength > 32))
    {
        return 0;
    }
    F(password, ssid, ssidlength, 4096, 1, output);
    F(password, ssid, ssidlength, 4096, 2, &output[A_SHA_DIGEST_LEN]);

    return 1;
}

void hex_bytes_to_chars( char* cptr, const uint8_t* bptr, uint32_t blen )
{
    int i,j;
    uint8_t temp;

    i = 0;
    j = 0;
    while( i < blen )
    {
        // Convert first nibble of byte to a hex character
        temp = bptr[i] / 16;
        if ( temp < 10 )
        {
            cptr[j] = temp + '0';
        }
        else
        {
            cptr[j] = (temp - 10) + 'A';
        }
        // Convert second nibble of byte to a hex character
        temp = bptr[i] % 16;
        if ( temp < 10 )
        {
            cptr[j+1] = temp + '0';
        }
        else
        {
            cptr[j+1] = (temp - 10) + 'A';
        }
        i++;
        j+=2;
    }
}


/*!
 ******************************************************************************
 * Starts a soft AP as specified by the provided arguments
 *
 * @return  0 for success, otherwise error
 */

int start_ap( int argc, char* argv[] )
{
    char* ssid = argv[1];
    wiced_security_t auth_type = str_to_authtype(argv[2]);
    char* security_key = argv[3];
    uint8_t channel = atoi(argv[4]);
    wiced_result_t result;
    uint8_t pmk[DOT11_PMK_LEN + 8]; // PMK storage must be 40 octets in length for use in various functions

    if ( wiced_wifi_is_ready_to_transceive( WICED_AP_INTERFACE ) == WICED_SUCCESS )
    {
        WPRINT_APP_INFO(( "Error: AP already started\r\n" ));
        return ERR_UNKNOWN;
    }

    if ( auth_type != WICED_SECURITY_WPA2_AES_PSK && auth_type != WICED_SECURITY_OPEN && auth_type != WICED_SECURITY_WPA2_MIXED_PSK )
    {
        WPRINT_APP_INFO(( "Error: Invalid security type\r\n" ));
        return ERR_UNKNOWN;
    }

    if ( auth_type == WICED_SECURITY_OPEN )
    {
        char c = 0;

        WPRINT_APP_INFO(( "Open without any encryption?\r\n" ));
        while (1)
        {
            c = getchar();
            if ( c == 'y' )
            {
                break;
            }
            if ( c == 'n' )
            {
                return ERR_CMD_OK;
            }
            WPRINT_APP_INFO(( "y or n\r\n" ));
        }
    }

    if ( argc == 6 )
    {
        if ( memcmp( argv[5], "wps", sizeof("wps") ) != 0 )
        {
            return ERR_UNKNOWN;
        }
    }

    memset( &wifi_config_dct_local, 0, sizeof( platform_dct_wifi_config_t ) );
    // Read config
    wiced_dct_read_wifi_config_section( &wifi_config_dct_local );
    //print_wifi_config_dct();

    strncpy(last_soft_ap_passphrase, security_key, MAX_PASSPHRASE_LEN+1);

    // Modify config
    if (strlen(security_key) < MAX_PASSPHRASE_LEN)
    {
        memset(pmk, 0, DOT11_PMK_LEN);
        if (PasswordHash ( security_key, (unsigned char *)ssid, strlen(ssid), (unsigned char*)pmk ))
        {
            //WPRINT_APP_INFO(( "PMK: "));
            //dump_bytes(pmk, DOT11_PMK_LEN);
            wifi_config_dct_local.soft_ap_settings.security_key_length = MAX_PASSPHRASE_LEN;
            hex_bytes_to_chars( (char*)&wifi_config_dct_local.soft_ap_settings.security_key, pmk, DOT11_PMK_LEN );
        }
    }
    else
    {
        wifi_config_dct_local.soft_ap_settings.security_key_length = strlen(security_key);
        strncpy((char*)&wifi_config_dct_local.soft_ap_settings.security_key, security_key, MAX_PASSPHRASE_LEN);
    }
    strncpy((char*)&wifi_config_dct_local.soft_ap_settings.SSID.val, ssid, MAX_SSID_LEN);
    wifi_config_dct_local.soft_ap_settings.security = auth_type;
    wifi_config_dct_local.soft_ap_settings.channel = channel;

    // Write config
    wiced_dct_write_wifi_config_section( (const platform_dct_wifi_config_t*)&wifi_config_dct_local );
//    print_wifi_config_dct();


    if ( ( result = wiced_network_up( WICED_AP_INTERFACE, WICED_USE_INTERNAL_DHCP_SERVER, &ap_ip_settings ) ) != WICED_SUCCESS )
    {
        printf("Error starting AP %u\r\n", (unsigned int)result);
        return result;
    }
#ifdef CONSOLE_ENABLE_WPS
    if ( argc == 6 )
    {
        if ( memcmp( argv[5], "wps", sizeof("wps") ) == 0 )
        {
            if ( workspace == NULL )
            {
                workspace = calloc_named("wps", 1, sizeof(wps_agent_t));
                if ( workspace == NULL )
                {
                    WPRINT_APP_INFO(("Error calloc wps\r\n"));
                    stop_ap(0, NULL);
                    return WICED_NOMEM;
                }
            }

            if ( ( result = besl_wps_init( workspace, (besl_wps_device_detail_t*) &device_details, WPS_REGISTRAR_AGENT, WICED_AP_INTERFACE ) ) != WICED_SUCCESS )
            {
                WPRINT_APP_INFO(("Error besl init %u\r\n", (unsigned int)result));
                stop_ap(0, NULL);
                return result;
            }
            if ( ( result = besl_wps_management_set_event_handler( workspace, WICED_TRUE ) ) != WICED_SUCCESS )
            {
                WPRINT_APP_INFO(("Error besl setting event handler %u\r\n", (unsigned int)result));
                stop_ap(0, NULL);
                return result;
            }
        }
    }
#endif
    strncpy(last_started_ssid, ssid, MAX_SSID_LEN);
    return ERR_CMD_OK;
}

/*!
 ******************************************************************************
 * Stops a running soft AP
 *
 * @return  0 for success, otherwise error
 */

int stop_ap( int argc, char* argv[] )
{
#ifdef CONSOLE_ENABLE_WPS
    if (workspace != NULL)
    {
        besl_wps_management_set_event_handler( workspace, WICED_FALSE );
        besl_wps_deinit( workspace );
        free( workspace );
        workspace = NULL;
    }
#endif
    return wiced_network_down( WICED_AP_INTERFACE );
}

int test_ap( int argc, char* argv[] )
{
    int i;
    int iterations;

    if (  argc < 6 )
    {
        return ERR_UNKNOWN;
    }
    iterations = atoi(argv[argc - 1]);
    printf("Iterations: %d\r\n", iterations);
    for (i = 0; i < iterations; i++ )
    {
        printf( "Iteration %d\r\n", i);
        start_ap( argc-1, argv );
        stop_ap( 0, NULL );
    }
    wiced_mac_t mac;
    if ( wiced_wifi_get_mac_address( &mac ) == WICED_SUCCESS )
    {
        WPRINT_APP_INFO(("Test Pass (MAC address is: %02X:%02X:%02X:%02X:%02X:%02X)\r\n", mac.octet[0], mac.octet[1], mac.octet[2], mac.octet[3], mac.octet[4], mac.octet[5]));
    }
    else
    {
        WPRINT_APP_INFO(("Test Fail\r\n"));
    }
    return ERR_CMD_OK;
}

int test_join( int argc, char* argv[] )
{
    int i;
    int iterations;
    uint32_t join_fails = 0, leave_fails = 0;

    if (  argc < 5 )
    {
        return ERR_UNKNOWN;
    }
    iterations = atoi(argv[argc - 1]);

    for (i = 0; i < iterations; i++ )
    {
        printf( "%d ", i);
        if ( join( argc-1, argv ) != ERR_CMD_OK)
        {
            ++join_fails;
        }
        if ( leave( 0, NULL ) != ERR_CMD_OK )
        {
            ++leave_fails;
        }
    }

    WPRINT_APP_INFO(("Join failures: %u\r\n", (unsigned int)join_fails));
    WPRINT_APP_INFO(("Leave failures: %u\r\n", (unsigned int)leave_fails));

    return ERR_CMD_OK;
}

int test_credentials( int argc, char* argv[] )
{
    wiced_result_t result;
    wiced_scan_result_t ap;

    memset(&ap, 0, sizeof(ap));

    ap.SSID.len = strlen(argv[1]);
    memcpy(ap.SSID.val, argv[1], ap.SSID.len);
    str_to_mac(argv[2], &ap.BSSID);
    ap.channel = atoi(argv[3]);
    ap.security = str_to_authtype(argv[4]);
    result = wiced_wifi_test_credentials(&ap, (uint8_t*)argv[5], strlen(argv[5]));

    if (result == WICED_SUCCESS)
    {
        WPRINT_APP_INFO(("Credentials are good\r\n"));
    }
    else
    {
        WPRINT_APP_INFO(("Credentials are bad\r\n"));
    }

    return ERR_CMD_OK;
}

int get_soft_ap_credentials( int argc, char* argv[] )
{
    wiced_security_t sec;

    if ( wiced_wifi_is_ready_to_transceive( WICED_AP_INTERFACE ) != WICED_SUCCESS )
    {
        WPRINT_APP_INFO(("Use start_ap command to bring up AP interface first\r\n"));
        return ERR_CMD_OK;
    }

    // Read config to get internal AP settings
    wiced_dct_read_wifi_config_section( &wifi_config_dct_local );
    WPRINT_APP_INFO(("SSID : %s\r\n", (char*)&wifi_config_dct_local.soft_ap_settings.SSID.val));
    sec = wifi_config_dct_local.soft_ap_settings.security;
    WPRINT_APP_INFO( ( "Security : %s\r\n", ( sec == WICED_SECURITY_OPEN )           ? "Open" :
                                            ( sec == WICED_SECURITY_WEP_PSK )        ? "WEP" :
                                            ( sec == WICED_SECURITY_WPA_TKIP_PSK )   ? "WPA TKIP" :
                                            ( sec == WICED_SECURITY_WPA_AES_PSK )    ? "WPA AES" :
                                            ( sec == WICED_SECURITY_WPA2_AES_PSK )   ? "WPA2 AES" :
                                            ( sec == WICED_SECURITY_WPA2_TKIP_PSK )  ? "WPA2 TKIP" :
                                            ( sec == WICED_SECURITY_WPA2_MIXED_PSK ) ? "WPA2 Mixed" :
                                            "Unknown" ) );
    WPRINT_APP_INFO(("Passphrase : %s\r\n", last_soft_ap_passphrase));

    // Copy config into the credential to be used by WPS
//    strncpy((char*)&credential.ssid.val, (char*)&wifi_config_dct_local.soft_ap_settings.SSID.val, MAX_SSID_LEN);
//    credential.ssid.len = strlen((char*)&credential.ssid.val);
//    credential.security = wifi_config_dct_local.soft_ap_settings.security;
//    memcpy((char*)&credential.passphrase, (char*)&wifi_config_dct_local.soft_ap_settings.security_key, MAX_PASSPHRASE_LEN);
//    credential.passphrase_length = wifi_config_dct_local.soft_ap_settings.security_key_length;

    return ERR_CMD_OK;
}

int get_pmk( int argc, char* argv[] )
{
    char pmk[64];

    if ( wiced_wifi_get_pmk( argv[1], strlen( argv[1] ), pmk ) == WICED_SUCCESS )
    {
        WPRINT_APP_INFO( ("%s\r\n", pmk) );
        return ERR_CMD_OK;
    }
    else
    {
        return ERR_UNKNOWN;
    }
}

int get_counters( int argc, char* argv[] )
{
    wiced_buffer_t buffer;
    wiced_buffer_t response;

    wiced_get_iovar_buffer(&buffer, sizeof(wl_cnt_t), "counters");

    if (wiced_send_iovar(SDPCM_GET, buffer, &response, SDPCM_STA_INTERFACE) == WICED_SUCCESS)
    {
        wl_cnt_t* data = (wl_cnt_t*)host_buffer_get_current_piece_data_pointer(response);
        WPRINT_APP_INFO(("rx1mbps=%u\r\n",(unsigned int)data->rx1mbps));

        host_buffer_release(response, WICED_NETWORK_RX);
    }
    else
    {
        return ERR_UNKNOWN;
    }
    return ERR_CMD_OK;

}

int get_ap_info( int argc, char* argv[] )
{
    wl_bss_info_t ap_info;
    wiced_security_t sec;

    if ( wiced_wifi_get_ap_info( &ap_info, &sec ) == WICED_SUCCESS )
    {
        WPRINT_APP_INFO( ("SSID  : %s\r\n", (char*)ap_info.SSID ) );
        WPRINT_APP_INFO( ("BSSID : %02X:%02X:%02X:%02X:%02X:%02X\r\n", ap_info.BSSID.octet[0], ap_info.BSSID.octet[1], ap_info.BSSID.octet[2], ap_info.BSSID.octet[3], ap_info.BSSID.octet[4], ap_info.BSSID.octet[5]) );
        WPRINT_APP_INFO( ("RSSI  : %d\r\n", ap_info.RSSI) );
        WPRINT_APP_INFO( ("SNR   : %d\r\n", ap_info.SNR) );
        WPRINT_APP_INFO( ("Beacon period : %u\r\n", ap_info.beacon_period) );
        WPRINT_APP_INFO( ( "Security : %s\r\n", ( sec == WICED_SECURITY_OPEN )           ? "Open" :
                                                ( sec == WICED_SECURITY_WEP_PSK )        ? "WEP" :
                                                ( sec == WICED_SECURITY_WPA_TKIP_PSK )   ? "WPA TKIP" :
                                                ( sec == WICED_SECURITY_WPA_AES_PSK )    ? "WPA AES" :
                                                ( sec == WICED_SECURITY_WPA2_AES_PSK )   ? "WPA2 AES" :
                                                ( sec == WICED_SECURITY_WPA2_TKIP_PSK )  ? "WPA2 TKIP" :
                                                ( sec == WICED_SECURITY_WPA2_MIXED_PSK ) ? "WPA2 Mixed" :
                                                "Unknown" ) );
    }
    else
    {
        return ERR_UNKNOWN;
    }
    return ERR_CMD_OK;
}


/*!
 ******************************************************************************
 * Leaves an associated access point
 *
 * @return  0 for success, otherwise error
 */

int leave( int argc, char* argv[] )
{
    return wiced_network_down( WICED_STA_INTERFACE );
}


/*!
 ******************************************************************************
 * Prints the device MAC address
 *
 * @return  0 for success, otherwise error
 */

int get_mac_addr( int argc, char* argv[] )
{
    wiced_buffer_t buffer;
    wiced_buffer_t response;
    wiced_mac_t mac;
    sdpcm_interface_t interface = SDPCM_STA_INTERFACE;

    memset(&mac, 0, sizeof( wiced_mac_t));

    if ( wiced_get_iovar_buffer( &buffer, sizeof(wiced_mac_t), IOVAR_STR_CUR_ETHERADDR ) == NULL )
    {
        return ERR_UNKNOWN;
    }

    if (argc == 2 && argv[1][0] == '1')
    {
        interface = SDPCM_AP_INTERFACE;
    }

    if ( wiced_send_iovar( SDPCM_GET, buffer, &response, interface ) == WICED_SUCCESS )
    {
        memcpy( mac.octet, host_buffer_get_current_piece_data_pointer( response ), sizeof(wiced_mac_t) );
        host_buffer_release( response, WICED_NETWORK_RX );
    }
    WPRINT_APP_INFO(("MAC address is: %02X:%02X:%02X:%02X:%02X:%02X\r\n", mac.octet[0], mac.octet[1], mac.octet[2], mac.octet[3], mac.octet[4], mac.octet[5]));
    return ERR_CMD_OK;
}

/*!
 ******************************************************************************
 * Enables or disables power save mode as specified by the arguments
 *
 * @return  0 for success, otherwise error
 */

int wifi_powersave( int argc, char* argv[] )
{
    int a = atoi( argv[1] );

    switch( a )
    {
        case 0:
        {
            if ( wiced_wifi_disable_powersave( ) != WICED_SUCCESS )
            {
                WPRINT_APP_INFO( ("Failed to disable Wi-Fi powersave\r\n") );
            }
            break;
        }

        case 1:
        {
            if ( wiced_wifi_enable_powersave( ) != WICED_SUCCESS )
            {
                WPRINT_APP_INFO( ("Failed to enable Wi-Fi powersave\r\n") );
            }
            break;
        }

        case 2:
        {
            uint8_t return_to_sleep_delay_ms = (uint8_t) atoi( argv[ 2 ] );

            if ( wiced_wifi_enable_powersave_with_throughput( return_to_sleep_delay_ms ) != WICED_SUCCESS )
            {
                WPRINT_APP_INFO( ("Failed to enable Wi-Fi powersave with throughput\r\n") );
            }
            break;
        }

        default:
            return ERR_UNKNOWN_CMD;

    }

    return ERR_CMD_OK;
}

/*!
 ******************************************************************************
 * Sets the transmit power as specified in arguments in dBm
 *
 * @return  0 for success, otherwise error
 */

int set_tx_power( int argc, char* argv[] )
{
    int dbm = atoi( argv[1] );
    return (wiced_wifi_set_tx_power(dbm) != WICED_SUCCESS );
}

/*!
 ******************************************************************************
 * Gets the current transmit power in dBm
 *
 * @return  0 for success, otherwise error
 */

int get_tx_power( int argc, char* argv[] )
{
    wiced_result_t result;
    uint8_t dbm;

    if ( WICED_SUCCESS != ( result = wiced_wifi_get_tx_power( &dbm ) ) )
    {
        return result;
    }

    WPRINT_APP_INFO(("Transmit Power : %ddBm\r\n", dbm ));

    return result;
}

/*!
 ******************************************************************************
 * Prints the latest RSSI value
 *
 * @return  0 for success, otherwise error
 */

int get_rssi( int argc, char* argv[] )
{
    int32_t rssi;
    wiced_wifi_get_rssi( &rssi );
    WPRINT_APP_INFO(("RSSI is %d\r\n", (int)rssi));
    return ERR_CMD_OK;
}


/*!
 ******************************************************************************
 * Returns the status of the Wi-Fi interface
 *
 * @return  0 for success, otherwise error
 */

int status( int argc, char* argv[] )
{
    wiced_mac_t mac;
    wiced_wifi_get_mac_address( &mac );
    WPRINT_APP_INFO(("WICED Version : " WICED_VERSION "\r\n"));
    WPRINT_APP_INFO(("Platform      : " PLATFORM "\r\n"));
    WPRINT_APP_INFO(("MAC Address   : %02X:%02X:%02X:%02X:%02X:%02X\r\n", mac.octet[0],mac.octet[1],mac.octet[2],mac.octet[3],mac.octet[4],mac.octet[5]));
    network_print_status(last_joined_ssid, last_started_ssid);
    return ERR_CMD_OK;
}

int antenna( int argc, char* argv[] )
{
    uint32_t value = str_to_int( argv[1] );
    if ( ( value == WICED_ANTENNA_1 ) || ( value == WICED_ANTENNA_2 ) || ( value == WICED_ANTENNA_AUTO ) )
    {
        if ( wiced_wifi_select_antenna( (wiced_antenna_t) value ) == WICED_SUCCESS )
        {
            return ERR_CMD_OK;
        }
    }
    return ERR_UNKNOWN;
}

int ant_sel( int argc, char* argv[] )
{
    wiced_buffer_t buffer;
    uint32_t value = hex_str_to_int(argv[1]);
    wlc_antselcfg_t* sel = (wlc_antselcfg_t*)wiced_get_iovar_buffer(&buffer, sizeof(wlc_antselcfg_t), "nphy_antsel");
    sel->ant_config[0] = value;
    sel->ant_config[1] = value;
    sel->ant_config[2] = value;
    sel->ant_config[3] = value;
    sel->num_antcfg = 0;
    if (wiced_send_iovar(SDPCM_SET, buffer, NULL, SDPCM_STA_INTERFACE) == WICED_SUCCESS)
    {
        return ERR_CMD_OK;
    }
    else
    {
        return ERR_UNKNOWN;
    }
}

int antdiv( int argc, char* argv[] )
{
    wiced_buffer_t buffer;
    *(uint32_t*)(wlc_antselcfg_t*)wiced_get_ioctl_buffer(&buffer, sizeof(uint32_t)) = str_to_int(argv[1]);
    if (wiced_send_ioctl(SDPCM_SET, WLC_SET_ANTDIV, buffer, NULL, SDPCM_STA_INTERFACE) == WICED_SUCCESS)
    {
        return ERR_CMD_OK;
    }
    else
    {
        return ERR_UNKNOWN;
    }
}

int txant( int argc, char* argv[] )
{
    wiced_buffer_t buffer;
    *(uint32_t*)(wlc_antselcfg_t*)wiced_get_ioctl_buffer(&buffer, sizeof(uint32_t)) = str_to_int(argv[1]);
    if (wiced_send_ioctl(SDPCM_SET, WLC_SET_TXANT, buffer, NULL, SDPCM_STA_INTERFACE) == WICED_SUCCESS)
    {
        return ERR_CMD_OK;
    }
    else
    {
        return ERR_UNKNOWN;
    }
}

int ucantdiv( int argc, char* argv[] )
{
    wiced_buffer_t buffer;
    *(uint32_t*)(wlc_antselcfg_t*)wiced_get_ioctl_buffer(&buffer, sizeof(uint32_t)) = str_to_int(argv[1]);
    if (wiced_send_ioctl(SDPCM_SET, WLC_SET_UCANTDIV, buffer, NULL, SDPCM_STA_INTERFACE) == WICED_SUCCESS)
    {
        return ERR_CMD_OK;
    }
    else
    {
        return ERR_UNKNOWN;
    }
}

int get_country( int argc, char* argv[] )
{
    // Get country information and print the abbreviation
    wl_country_t cspec;
    wiced_buffer_t buffer;
    wiced_buffer_t response;

    wl_country_t* temp = (wl_country_t*)wiced_get_iovar_buffer( &buffer, sizeof( wl_country_t ), "country" );
    memset( temp, 0, sizeof(wl_country_t) );
    wiced_result_t result = wiced_send_iovar( SDPCM_GET, buffer, &response, SDPCM_STA_INTERFACE );

    if (result == WICED_SUCCESS)
    {
        memcpy( (char *)&cspec, (char *)host_buffer_get_current_piece_data_pointer( response ), sizeof(wl_country_t) );
        host_buffer_release(response, WICED_NETWORK_RX);
        char* c = (char*)&(cspec.country_abbrev);
        WPRINT_APP_INFO(( "Country is %s\r\n", c ));

    }
    else
    {
        WPRINT_APP_INFO(("country iovar not supported, trying ioctl\r\n"));
        temp = (wl_country_t*)wiced_get_ioctl_buffer(&response, sizeof( wl_country_t ));
        memset( temp, 0, sizeof( wl_country_t ) );
        result = wiced_send_ioctl( SDPCM_GET, WLC_GET_COUNTRY, buffer, &response, SDPCM_STA_INTERFACE );
        if (result == WICED_SUCCESS)
        {
            memcpy( (char *)&cspec, (char *)host_buffer_get_current_piece_data_pointer( response ), sizeof(wl_country_t) );
            host_buffer_release(response, WICED_NETWORK_RX);
        }
    }

    return result;
}

int set_data_rate( int argc, char* argv[] )
{
    wiced_buffer_t buffer;
    uint32_t*          data;

    data = wiced_get_iovar_buffer( &buffer, (uint16_t) 4, "bg_rate" );

    if ( data == NULL )
    {
        return ERR_UNKNOWN;
    }

    /* Set data to 2 * <rate> as legacy rate unit is in 0.5Mbps */
    *data = (uint32_t)(2 * atof( argv[1] ));

    if ( wiced_send_iovar( SDPCM_SET, buffer, 0, SDPCM_STA_INTERFACE ) != WICED_SUCCESS )
    {
        return ERR_UNKNOWN;
    }

    return ERR_CMD_OK;
}

int get_data_rate( int argc, char* argv[] )
{
    wiced_buffer_t buffer;
    wiced_buffer_t response;
    uint32_t*          data;

    data = (uint32_t*) wiced_get_iovar_buffer( &buffer, (uint16_t) 4, "bg_rate" );

    if ( data == NULL )
    {
        return ERR_UNKNOWN;
    }

    if ( wiced_send_iovar( SDPCM_GET, buffer, &response, SDPCM_STA_INTERFACE ) != WICED_SUCCESS )
    {
        return ERR_UNKNOWN;
    }

    data = (uint32_t*) host_buffer_get_current_piece_data_pointer( response );

    /* 5.5 Mbps */
    if ( *data == 11 )
    {
        WPRINT_APP_INFO(( "data rate: 5.5 Mbps\n\r" ));
    }
    else
    {
        WPRINT_APP_INFO(( "data rate: %d Mbps\n\r", (int)(*data / 2) ));

    }

    host_buffer_release( response, WICED_NETWORK_RX );
    return ERR_CMD_OK;
}

/*!
 ******************************************************************************
 * Interface to the wiced_wifi_get_random() function. Prints result
 *
 * @return  0 for success, otherwise error
 */
int get_random( int argc, char* argv[] )
{
    uint16_t random;
    if ( wiced_wifi_get_random( &random ) == WICED_SUCCESS )
    {
        WPRINT_APP_INFO(("Random number is %d\r\n", (int)random));
        return ERR_CMD_OK;
    }

    return ERR_UNKNOWN;
}

//static wiced_result_t print_wifi_config_dct( void )
//{
//    platform_dct_wifi_config_t const* dct_wifi_config = wiced_dct_get_wifi_config_section( );
//
//    WPRINT_APP_INFO( ( "\r\n----------------------------------------------------------------\r\n\r\n") );
//
//    /* Wi-Fi Config Section */
//    WPRINT_APP_INFO( ( "Wi-Fi Config Section \r\n") );
//    WPRINT_APP_INFO( ( "    device_configured               : %d \r\n", dct_wifi_config->device_configured ) );
//    WPRINT_APP_INFO( ( "    stored_ap_list[0]  (SSID)       : %s \r\n", dct_wifi_config->stored_ap_list[0].details.SSID.val ) );
//    WPRINT_APP_INFO( ( "    stored_ap_list[0]  (Passphrase) : %s \r\n", dct_wifi_config->stored_ap_list[0].security_key ) );
//    WPRINT_APP_INFO( ( "    soft_ap_settings   (SSID)       : %s \r\n", dct_wifi_config->soft_ap_settings.SSID.val ) );
//    WPRINT_APP_INFO( ( "    soft_ap_settings   (Passphrase) : %s \r\n", dct_wifi_config->soft_ap_settings.security_key ) );
//    WPRINT_APP_INFO( ( "    config_ap_settings (SSID)       : %s \r\n", dct_wifi_config->config_ap_settings.SSID.val ) );
//    WPRINT_APP_INFO( ( "    config_ap_settings (Passphrase) : %s \r\n", dct_wifi_config->config_ap_settings.security_key ) );
//    WPRINT_APP_INFO( ( "    country_code                    : %c%c%d \r\n", ((dct_wifi_config->country_code) >>  0) & 0xff,
//                                                                            ((dct_wifi_config->country_code) >>  8) & 0xff,
//                                                                            ((dct_wifi_config->country_code) >> 16) & 0xff));
//    //print_mac_address( "    DCT mac_address                 :", (wiced_mac_t*)&dct_wifi_config->mac_address );
//
//    return WICED_SUCCESS;
//}

void dump_bytes(const uint8_t* bptr, uint32_t len)
{
    int i = 0;

    for (i = 0; i < len; )
    {
        if ((i & 0x0f) == 0)
        {
            WPRINT_APP_INFO( ( "\r\n" ) );
        }
        else if ((i & 0x07) == 0)
        {
            WPRINT_APP_INFO( (" ") );
        }
        WPRINT_APP_INFO( ( "%02x ", bptr[i++] ) );
    }
    WPRINT_APP_INFO( ( "\r\n" ) );
}
