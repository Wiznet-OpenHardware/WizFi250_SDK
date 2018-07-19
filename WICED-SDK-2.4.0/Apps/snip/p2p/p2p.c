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
 * Wi-Fi Direct (P2P) Application
 *
 * Features demonstrated
 *  - Wi-Fi Direct
 *
 * This application snippet demonstrates usage of Wi-Fi Direct.
 * Wi-Fi Direct is still under development, the following
 * instructions apply to the current implementation and are
 * guaranteed to change for the next release!
 *
 * Application Instructions
 *   1. Connect a PC terminal to the serial port of the WICED Eval board,
 *      then build and download the application as described in the WICED
 *      Quick Start Guide
 *   2. Find a p2p capable device e.g. Android tablet
 *   3. Open the Wi-Fi settings and enable Wi-Fi Direct
 *   4. Tap on the device 'WICED-P2P'
 *
 *  The WICED device will attempt to connect with the
 *  P2P device. Progress information is printed to the
 *  console output.
 *
 */

#include "wiced.h"
#include "wiced_p2p.h"
#include "p2p_host_interface.h"
#include "wwd_wifi.h"
#include "internal/SDPCM.h"

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

/******************************************************
 *               Variable Definitions
 ******************************************************/

static const besl_p2p_device_detail_t device_details =
{
    .wps_device_details =
    {
        .device_name     = "WICED Device",
        .manufacturer    = "Broadcom",
        .model_name      = "BCM943362",
        .model_number    = "WD-1.0",
        .serial_number   = "12345670",
        .device_category = WICED_WPS_DEVICE_COMPUTER,
        .sub_category    = 7,
        .config_methods  = WPS_CONFIG_PUSH_BUTTON | WPS_CONFIG_VIRTUAL_PUSH_BUTTON | WPS_CONFIG_VIRTUAL_DISPLAY_PIN,
    },
    .group_owner_intent = 1,
    .ap_ssid_suffix     = "wiced!",
    .device_name        = "WICED-P2P",
};

static p2p_workspace_t workspace;

/******************************************************
 *               Function Definitions
 ******************************************************/

void application_start( )
{
    wiced_init();

    besl_p2p_init ( &workspace, &device_details );
    besl_p2p_start( &workspace );
}
