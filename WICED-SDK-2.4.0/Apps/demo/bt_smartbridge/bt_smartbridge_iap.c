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
 * Please read the header comments in bt_smartbridge.c for
 * information about this file
 *
 */

#include "iap_server.h"
#include "wiced.h"

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

static void iap_event_handler( wiced_iap_event_t event, void *arg );

/******************************************************
 *               Variables Definitions
 ******************************************************/

static wiced_semaphore_t config_event;

/******************************************************
 *               Function Definitions
 ******************************************************/

wiced_result_t bt_smartbridge_run_iap( void )
{
    wiced_iap_options_t iap_options =
    {
        .bluetooth_service_name = "WICED iAP Device",
        .bluetooth_vendor_id    = 0xAC05,
        .bluetooth_product_id   = 0x22BE,
        .auth_location          = AUTH_CHIP_LOCATION_EXTERNAL,
        .firmware_major_version = 1,
        .firmware_minor_version = 0,
        .firmware_revision      = 0,
        .hardware_major_version = 1,
        .hardware_minor_version = 0,
        .hardware_revision      = 0,
        .name                   = "WICED+",
        .model_number           = "A1234",
        .manufacturer           = "Broadcom",
        .public_iap_protocol_1  = "com.example.a",
        .bundle_seed_id         = "AA1A11A11A"
    };

    /* Configure iAP */
    wiced_configure_iap( &iap_options );

    /* Intialize the config_event semaphore */
    wiced_rtos_init_semaphore( &config_event );

    /* Start iAP and signal completion on config_event */
    wiced_iap_server_start( iap_event_handler, NULL );

    /* Wait until configuration has completed */
    wiced_rtos_get_semaphore( &config_event, WICED_WAIT_FOREVER );

    /* Shutdown iAP because configuration is complete */
    wiced_iap_server_stop( );

    return WICED_SUCCESS;
}

/*
 * iAP event handler
 */
static void iap_event_handler( wiced_iap_event_t event, void *arg )
{
    switch( event )
    {
        case IAP_EVENT_INITIALIZED:

            WPRINT_APP_INFO(( "%s: initialized\r\n", __func__ ));
            break;

        case IAP_EVENT_DEVICE_CONNECTED:

            WPRINT_APP_INFO(( "%s: device connected\r\n", __func__ ));
            break;

        case IAP_EVENT_CONFIGURED:

            WPRINT_APP_INFO(( "%s: successfully configured\r\n", __func__ ));

            // Signal the application thread

            wiced_rtos_set_semaphore( &config_event );
            break;

        case IAP_EVENT_CONFIGURATION_ERROR:

            WPRINT_APP_INFO(( "%s: configuration error\r\n", __func__ ));
            break;

        default:

            WPRINT_APP_ERROR(( "%s: unknown iAP status\r\n", __func__ ));
            break;

    }
}
